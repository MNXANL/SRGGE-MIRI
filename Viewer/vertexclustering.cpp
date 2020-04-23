#include "vertexclustering.h"

#include "./mesh_io.h"
#include "./triangle_mesh.h"
using namespace std;


// Get all Q[v] matrices, for each v in vertices
void VertexClustering::calcQMatrices( std::vector<float>& vtx, std::vector<float>& norm ) {
    int SIZE = vtx.size() / 3;
    QMatrixPerVert.resize( 0 );
    for (int i = 0; i < SIZE; ++i) {
        Eigen::Vector3f myVec, myNorm;
        myVec = Eigen::Vector3f( vtx[3*i + 0], vtx[3*i + 1], vtx[3*i + 2] );
        myNorm = Eigen::Vector3f( norm[3*i + 0], norm[3*i + 1], norm[3*i + 2] );
        float myDotProd = myNorm.dot( myVec );

        Eigen::Vector4f Pv( norm[3*i + 0], norm[3*i + 1], norm[3*i + 2], -myDotProd );
        QMatrixPerVert.push_back( Pv * Pv.transpose() );
    }
}

// Get the new normals for a new set of vertices and faces
void VertexClustering::getNewNormals(const std::vector<float> &newVtx, const std::vector<int> &newFaces,   std::vector<float> &newNormals  ) {
    std::vector< float > faceNormals( newFaces.size(), 0.0f );
    //std::vector< float > newNormals( newVtx.size(), 0.0f);


    //for each face grabb vtx 3-by-3, get the normal per face
    for (int i = 0; i < (int) newFaces.size(); i+=3) {
        // get vertices from face
        Eigen::Vector3f vtx1( newVtx[ 3*newFaces[i+0] + 0 ], newVtx[ 3*newFaces[i+0] + 1 ], newVtx[ 3*newFaces[i+0] + 2 ] );
        Eigen::Vector3f vtx2( newVtx[ 3*newFaces[i+1] + 0 ], newVtx[ 3*newFaces[i+1] + 1 ], newVtx[ 3*newFaces[i+1] + 2 ] );
        Eigen::Vector3f vtx3( newVtx[ 3*newFaces[i+2] + 0 ], newVtx[ 3*newFaces[i+2] + 1 ], newVtx[ 3*newFaces[i+2] + 2 ] );

        Eigen::Vector3f tri[3] = {vtx1, vtx2, vtx3};

        //get the face's normal via cross-product
        Eigen::Vector3f vtx1to2 = vtx2 - vtx1;
        Eigen::Vector3f vtx1to3 = vtx3 - vtx1;
        Eigen::Vector3f nrml = vtx1to2.cross( vtx1to3 );

        double norma = nrml.norm();
        if ( norma >= 1e-9 ) {
            //Sum normalized normal to faceNormals
            nrml.normalize();
            faceNormals[i + 0] = nrml[0];
            faceNormals[i + 1] = nrml[1];
            faceNormals[i + 2] = nrml[2];
        }
        //else  skip, already 0!

        // Angle per each triangle vertex
        for (int j = 0 ; j < 3 ; ++j) {
            // rotation: pivot per iteration is [ j -> j1 -> j2 ]

            //int j0 = (j+0) % 3;
            int j1 = (j+1) % 3;
            int j2 = (j+2) % 3;

            //get its normal
            vtx1to2 = tri[ j1 ] - tri[ j ];
            vtx1to3 = tri[ j2 ] - tri[ j ];

            float dotProd = vtx1to2.dot( vtx1to3 );
            dotProd = -(dotProd < 0.0f) * dotProd;
            double phi = acos( dotProd / (vtx1to3.norm()*vtx1to2.norm()) );

            //Sum faceNormals to the new normals w/ respective angle ponderation
            newNormals[ 3*newFaces[ i+j ] + 0 ] += phi*faceNormals[ i + 0 ];
            newNormals[ 3*newFaces[ i+j ] + 1 ] += phi*faceNormals[ i + 1 ];
            newNormals[ 3*newFaces[ i+j ] + 2 ] += phi*faceNormals[ i + 2 ];

        }
    }

    // Finally, normalize every single norm
    for (int i = 0; i < (int) newNormals.size(); i+=3) {
        Eigen::Vector3f nrml( newNormals[i + 0], newNormals[ i + 1 ], newNormals[ i + 2 ] );

        double norma = nrml.norm();
        if ( norma >= 1e-9 ) {
            //Sum normalized normal to newNormals
            nrml.normalize();
            newNormals[ i + 0 ] = nrml[0];
            newNormals[ i + 1 ] = nrml[1];
            newNormals[ i + 2 ] = nrml[2];
        }
    }
    //return newNormals;
}


bool vtxLEQ( Eigen::Vector3f& A, Eigen::Vector3f& B ) {
    bool X = A[0] <= B[0];
    bool Y = A[1] <= B[1];
    bool Z = A[2] <= B[2];
    return (X and Y and Z);
}





int VertexClustering::buildCluster( std::vector<float>& vtx, std::vector<int>& faces, std::vector<float>& normals,
                                     Eigen::Vector3f& min, Eigen::Vector3f& max, std::string method )
{
    //Set up data structures
    MAX_LOD = 8;

    vtxPerLOD.resize(MAX_LOD);
    facesPerLOD.resize(MAX_LOD);
    normPerLOD.resize(MAX_LOD);

    std::cout << MAX_LOD << std::endl;
    std::cout << vtxPerLOD.size() << std::endl;
    std::cout << facesPerLOD.size() << std::endl;
    std::cout << normPerLOD.size() << std::endl;


    for (int i = 0 ; i < MAX_LOD ; ++i) {
        vtxPerLOD[i] = vtx ;
        facesPerLOD[i] = faces ;
        normPerLOD[i] = normals ;
    }
    // Use actual mesh values as LOD 0 (max detail)
    /*vtxPerLOD[0] = vtx ;
    facesPerLOD[0] = faces ;
    normPerLOD[0] = normals ;*/

    // Dimensions
    float xDim = (max[0] - min[0]);    float yDim = (max[1] - min[1]);    float zDim = (max[2] - min[2]);
    int NumVertices = vtx.size() / 3;
    int NumFaces = faces.size() / 3;

    if (method == "Error Quadrics" )
        calcQMatrices(vtx, normals);


    std::vector< std::vector<float> > Grid( MAX_LOD*MAX_LOD*MAX_LOD );
    std::vector<float> newVtx(0);
    std::vector<int>   newFaces(0);
    std::vector< float > newNormals(0);
    std::vector< int > gridCellVtx_To_Index( NumVertices, -1 );



    // For each level of detail 2 to MAX_LOD+1 ...
    for (int LOD = 2; LOD <= MAX_LOD; ++LOD ){
        int level3D = LOD*LOD*LOD; // LOD ^ 3

        // Create new structures for this LOD
        newVtx.resize(0);
        newFaces.resize(0);
        std::fill(gridCellVtx_To_Index.begin(), gridCellVtx_To_Index.end(), -1);  // clean array


        // Set 3D uniform grid size and dimensions per cell
        Grid.resize( level3D );

        float gridCellX = xDim / LOD;
        float gridCellY = yDim / LOD;
        float gridCellZ = zDim / LOD;

        // For each vertex...
        for (int idx = 0 ; idx < NumVertices ; ++idx) {
            // Place index into grid; if index==LOD subtract one
            int vtX = (int) ( (vtx[3*idx + 0] - min(0)) / gridCellX );      vtX -= (vtX == LOD);
            int vtY = (int) ( (vtx[3*idx + 1] - min(1)) / gridCellY );      vtY -= (vtY == LOD);
            int vtZ = (int) ( (vtx[3*idx + 2] - min(2)) / gridCellZ );      vtZ -= (vtZ == LOD);

            // Indexing = x + y*lvl + z*(lvl**2)
            int gridPos = vtX + LOD*(vtY + LOD*vtZ) ;
            Grid[ gridPos ].push_back( idx );
        }



        if ( method == "Error Quadrics" ) { // ERROR QUADRICS
            for (int i = 0 ; i < level3D ; ++i) {
                // Get vertex median:
                int GridSize = Grid[ i ].size();
                Eigen::Matrix4f Qmat = Eigen::Matrix4f::Zero();
                Eigen::Matrix4f Qinv = Eigen::Matrix4f::Zero();
                if ( GridSize > 0 ) {
                    bool isQInvertible = false;
                    // sequence of pair contractions in the cluster
                    for (int j = 0 ; j < GridSize; ++j) {
                        Qmat += QMatrixPerVert[ Grid[i][j] ];
                        gridCellVtx_To_Index[ Grid[i][j] ] = i;
                    }

                    Qmat(3,0) = 0.0f; Qmat(3,1) = 0.0f; Qmat(3,2) = 0.0f; Qmat(3,3) = 1.0f;
                    Qmat.computeInverseWithCheck( Qinv, isQInvertible, 0.1 );
                    if ( isQInvertible ) {

                        Eigen::Vector4f Vvec = Qinv * Eigen::Vector4f(0.0f, 0.0f, 0.0f, 1.0f );
                        Eigen::Vector3f myV ( Vvec[0], Vvec[1], Vvec[2] );

                        std::cout << i << ", " << myV[0] << ", " << myV[1] << ", " << myV[2] << std::endl;
                        if ( vtxLEQ( min, myV )  and vtxLEQ( myV, max )  ){
                            // Add those vertices to the newVtx vector
                            newVtx.push_back( myV[ 0 ] );
                            newVtx.push_back( myV[ 1 ] );
                            newVtx.push_back( myV[ 2 ] );
                        }
                    }

                }
            }
        }
        else if ( method == "Shape-Preserving" ) { // SHAPE-PRESERVING ALGORITHM
            for (int i = 0 ; i < level3D ; ++i) {
                // Get vertex median:
                int GridSize = Grid[ i ].size();
                Eigen::Matrix4f Qmat = Eigen::Matrix4f::Zero();
                Eigen::Matrix4f Qinv = Eigen::Matrix4f::Zero();
                Eigen::Vector4f Vvec = Eigen::Vector4f::Zero();
                if ( GridSize > 0 ) {
                    bool isQInverted = false;
                    for (int j = 0 ; j < GridSize; ++j) {
                        Qmat += QMatrixPerVert[ Grid[i][j] ];
                        gridCellVtx_To_Index[ Grid[i][j] ] = i;
                    }

                    Qmat(3,0) = 0.0f; Qmat(3,1) = 0.0f; Qmat(3,2) = 0.0f; Qmat(3,3) = 1.0f;
                    Qmat.computeInverseWithCheck( Qinv, isQInverted, 0.1 );
                    if ( isQInverted ) {

                        Eigen::Vector4f myV = Qinv * Eigen::Vector4f(0.0f, 0.0f, 0.0f, 1.0f );

                        std::cout << i << ", " << myV[0] << ", " << myV[1] << ", " << myV[2] << std::endl;
                        // Add those vertices to the newVtx vector
                        newVtx.push_back( myV[ 0 ] );
                        newVtx.push_back( myV[ 1 ] );
                        newVtx.push_back( myV[ 2 ] );

                    }
                }
            }
        }
        else { //if ( method == "Median" )

            // For each position in the grid...
            for (int i = 0 ; i < level3D ; ++i) {
                int GridSize = Grid[ i ].size();
                if ( GridSize > 0 ) {
                    // Get vertex median:
                    int myFace = Grid[ i ][ GridSize >> 1 ];

                    // Add those vertices to the newVtx vector
                    newVtx.push_back( vtx[ 3*myFace + 0 ] );
                    newVtx.push_back( vtx[ 3*myFace + 1 ] );
                    newVtx.push_back( vtx[ 3*myFace + 2 ] );

                    for (int j = 0 ; j < GridSize; ++j) {
                        gridCellVtx_To_Index[ Grid[i][j] ] = i;
                    }
                }
            }
        }
        vtxPerLOD[LOD - 1]   = newVtx;

        std::set< std::pair< int, int > > newGridEdges;

        // Get the faces from each vtx
        for (int myFace = 0 ; myFace < newVtx.size() ; ++myFace) {
        /* for (int myFace = 0 ; myFace < NumFaces ; ++myFace) {
            // Get face idx:
            int fc1 = gridCellVtx_To_Index[ faces[ 3*myFace + 0 ] ];
            int fc2 = gridCellVtx_To_Index[ faces[ 3*myFace + 1 ] ];
            int fc3 = gridCellVtx_To_Index[ faces[ 3*myFace + 2 ] ];*/

            int fc1 = gridCellVtx_To_Index[ 3*myFace + 0 ];
            int fc2 = gridCellVtx_To_Index[ 3*myFace + 1 ];
            int fc3 = gridCellVtx_To_Index[ 3*myFace + 2 ];


            // If each face coord belongs to a different cell (i.e. no matching cells) FIX ME
            if ( fc1 != fc2  and  fc1 != fc3  and  fc2 != fc3 ) {
                std::cout << " --> IDX VTX[" << myFace << "] = " << fc1 << ", \t" << fc2  << ", \t" << fc3 << std::endl;

            /*if((fc1 + LOD == fc2 and fc1 + 1 == fc3) or
               (fc1 + LOD == fc2 and fc2 + 1 == fc3) or
               (fc1 + LOD*LOD == fc2 and fc2 + 1 == fc3) or
               (fc1 + LOD*LOD == fc2 and fc2 + 1 == fc3) ){*/

                int x = 0;
                int y = 0;
                int z = fc3 % level3D;
                int pos[3][3];
                //     X                        Y                                       Z       coordinates
                pos[0][0] = fc1 / (LOD*LOD);      pos[0][1] = std::max(fc1, pos[0][0]) / LOD;       pos[0][2] = std::max(fc1, pos[0][0]) % LOD;
                pos[1][0] = fc2 / (LOD*LOD);      pos[1][1] = std::max(fc2, pos[1][0]) / LOD;       pos[1][2] = std::max(fc2, pos[1][0]) % LOD;
                pos[2][0] = fc3 / (LOD*LOD);      pos[2][1] = std::max(fc3, pos[2][0]) / LOD;       pos[2][2] = std::max(fc3, pos[2][0]) % LOD;

                std::cout << "Pos fc1: " << pos[0][0] << ", " << pos[0][1] << ", " << pos[0][2] << "\n";
                std::cout << "Pos fc2: " << pos[1][0] << ", " << pos[1][1] << ", " << pos[1][2] << "\n";
                std::cout << "Pos fc3: " << pos[2][0] << ", " << pos[2][1] << ", " << pos[2][2] << "\n";

                // Add those faces to the newFaces vector
                newFaces.push_back( fc1 );
                newFaces.push_back( fc2 );
                newFaces.push_back( fc3 );
            }
        }


        facesPerLOD[LOD - 1] = newFaces;

        newNormals.resize( vtx.size() );
        //getNewNormals( newVtx, newFaces, newNormals );
        getNewNormals( vtxPerLOD[LOD - 1], facesPerLOD[LOD - 1], newNormals );

        normPerLOD[LOD - 1]  = newNormals;

        std::cout << "Calc for DivPerCell = " << LOD-1 << std::endl;
        std::cout << "-----------------------" << std::endl;
        std::cout << "  Vtxs size:  " << newVtx.size() << std::endl;
        std::cout << "  Faces size: " << newFaces.size() << std::endl;
        std::cout << "  Normals size: " << newVtx.size() << std::endl;

        // Insert new LOD into array
        /*
        vtxPerLOD[LOD - 1]   = newVtx;
        facesPerLOD[LOD - 1] = newFaces;
        normPerLOD[LOD - 1]  = newNormals;
        */


        std::cout << "-----------------------" << std::endl;
    }


    std::cout << "-----------------------" << std::endl;
    std::cout << "Vtxs LoD size:    " << vtxPerLOD.size() << std::endl;
    std::cout << "Faces LoD size:   " << facesPerLOD.size() << std::endl;
    std::cout << "Normals LoD size: " << normPerLOD.size() << std::endl;

    return MAX_LOD;
}


