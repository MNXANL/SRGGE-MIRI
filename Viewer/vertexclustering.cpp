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
// similar to Mesh_IO::ComputeFaceNormals except with some optimizations
void VertexClustering::getNewNormals(const std::vector<float> &newVtx, const std::vector<int> &newFaces,   std::vector<float> &newNormals  ) {
    std::vector< float > faceNormals( newFaces.size(), 0.0f );
    //std::vector< float > newNormals( newVtx.size(), 0.0f);


    //for each face grab vtx 3-by-3, get the normal per face
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
    MAX_LOD = 10;
    int STEP = 2;
    int RATIO = 2;

    vtxPerLOD.resize((MAX_LOD / STEP) + 2);
    facesPerLOD.resize((MAX_LOD / STEP) + 2);
    normPerLOD.resize((MAX_LOD / STEP) + 2);


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
    std::vector< int > numVtx_To_cellGrid( NumVertices, -1 );


    std::cout << "There are " << (MAX_LOD / STEP) + 1 << " items to add\n" ;
    int NumLods = 0; //1

    newVtx.resize(0);
    newFaces.resize(0);
    newNormals.resize(0);
/*
    // Use void mesh values as LOD 0 ("no" detail)
    vtxPerLOD[0]   = newVtx ;
    facesPerLOD[0] = newFaces ;
    normPerLOD[0]  = newNormals ;
*/

    // For each level of detail 2 to MAX_LOD+1 ...
    for (int LOD = 2; LOD <= RATIO*MAX_LOD; LOD += RATIO*STEP )
    {
        int level3D = LOD*LOD*LOD; // LOD ^ 3

        // Create new structures for this LOD
        newVtx.resize(0);
        newFaces.resize(0);
        std::fill(numVtx_To_cellGrid.begin(), numVtx_To_cellGrid.end(), -1);  // clean array

        float gridCellX = xDim / LOD;
        float gridCellY = yDim / LOD;
        float gridCellZ = zDim / LOD;

        // For each vertex...

        if ( method == "Shape-Preserving" ) {
            // Set 3D uniform grid size w/normal sign and dimensions per cell
            //
            // Normal 0 = {-X, -Y, -Z}, 1 = {X, -Y, -Z}, .., 7 = {+X, +Y, -+Z}
            Grid.clear( );
            Grid.resize( 8 * level3D );
            for (int idx = 0 ; idx < NumVertices ; ++idx) {
                // Place index into grid; if index==LOD subtract one
                int vtX = (int) ( (vtx[3*idx + 0] - min(0)) / gridCellX );      vtX -= (vtX == LOD);
                int vtY = (int) ( (vtx[3*idx + 1] - min(1)) / gridCellY );      vtY -= (vtY == LOD);
                int vtZ = (int) ( (vtx[3*idx + 2] - min(2)) / gridCellZ );      vtZ -= (vtZ == LOD);

                // Indexing = x + y*lvl + z*(lvl**2) = x + lvl( y + lvl(z) )
                int gridPos = vtX + LOD*(vtY + LOD*vtZ) ;

                int sign = (0.0f <= normals[3*idx + 0]) + 2*(0.0f <= normals[3*idx + 1]) + 4*(0.0f <= normals[3*idx + 2]);

                Grid[ 8*gridPos + sign ].push_back( idx ); //8*i + j, i= lvl, j=norm dir BROKEN :(
            }
        }
        else {
            // Set 3D uniform grid size and dimensions per cell
            Grid.clear( );
            Grid.resize( level3D );
            for (int idx = 0 ; idx < NumVertices ; ++idx) {
                // Place index into grid; if index==LOD subtract one
                int vtX = (int) ( (vtx[3*idx + 0] - min(0)) / gridCellX );      vtX -= (vtX == LOD);
                int vtY = (int) ( (vtx[3*idx + 1] - min(1)) / gridCellY );      vtY -= (vtY == LOD);
                int vtZ = (int) ( (vtx[3*idx + 2] - min(2)) / gridCellZ );      vtZ -= (vtZ == LOD);

                // Indexing = x + y*lvl + z*(lvl**2) = x + lvl( y + lvl(z) )
                int gridPos = vtX + LOD*(vtY + LOD*vtZ) ;
                Grid[ gridPos ].push_back( idx );
            }
        }


        int NumSkips = 0;
        if ( method == "Error Quadrics" ) { // ERROR QUADRICS
            for (int i = 0 ; i < level3D ; ++i) {
                // Get vertex median:
                int GridSize = Grid[ i ].size();
                Eigen::Matrix4f Qmat = Eigen::Matrix4f::Zero();
                Eigen::Matrix4f Qinv = Eigen::Matrix4f::Zero();
                if ( GridSize > 0 ) {
                    bool isQInvertible = false;
                    // sequence of cell contractions in the grid
                    for (int j = 0 ; j < GridSize; ++j) {
                        int myFace = Grid[ i ][ j ];
                        Qmat += QMatrixPerVert[ myFace ];
                        numVtx_To_cellGrid[ Grid[i][j] ] = i - NumSkips; // cell[ #vtx ] = i;
                    }
                    Qmat(3,0) = 0.0f; Qmat(3,1) = 0.0f; Qmat(3,2) = 0.0f; Qmat(3,3) = 1.0f;
                    Qmat.computeInverseWithCheck( Qinv, isQInvertible, 0.1 );
                    if ( isQInvertible ) {
                        Eigen::Vector4f Vvec = Qinv * Eigen::Vector4f(0.0f, 0.0f, 0.0f, 1.0f );
                        Eigen::Vector3f minCell( min[0] + gridCellX*0.0  +  gridCellX* (i % LOD),
                            min[1] + gridCellY*0.0  +  gridCellY* ((i / LOD)% LOD),
                            min[2] + gridCellZ*0.0  +  gridCellZ* (i / (LOD*LOD)) );
                        Eigen::Vector3f maxCell( min[0] + gridCellX*1.0  +  gridCellX* (i % LOD),
                            min[1] + gridCellY*1.0  +  gridCellY* ((i / LOD)% LOD),
                            min[2] + gridCellZ*1.0  +  gridCellZ* (i / (LOD*LOD)) );

                        Eigen::Vector3f myV ( Vvec[0], Vvec[1], Vvec[2] );
                        if ( vtxLEQ( min, myV )  and vtxLEQ( myV, max )  ){
                        //if ( vtxLEQ( minCell, myV )  and vtxLEQ( myV, maxCell )  ){
                            // Add those vertices to the newVtx vector
                            newVtx.push_back( myV[ 0 ] );
                            newVtx.push_back( myV[ 1 ] );
                            newVtx.push_back( myV[ 2 ] );
                        }else {
                            float sumX, sumY, sumZ; sumZ = sumY = sumX = 0.0f;
                            for (int j = 0 ; j < GridSize; ++j) {
                                // vtx mean
                                int myFace = Grid[ i ][ j ];
                                sumX += vtx[ 3*myFace + 0];
                                sumY += vtx[ 3*myFace + 1 ];
                                sumZ += vtx[ 3*myFace + 2 ];
                                numVtx_To_cellGrid[ Grid[i][j] ] = i - NumSkips; // cell[ #vtx ] = i;
                            }
                            newVtx.push_back( sumX / GridSize );
                            newVtx.push_back( sumY / GridSize );
                            newVtx.push_back( sumZ / GridSize );
                        }
                    }
                    else {
                        float sumX, sumY, sumZ; sumZ = sumY = sumX = 0.0f;
                        for (int j = 0 ; j < GridSize; ++j) {
                            // vtx mean
                            int myFace = Grid[ i ][ j ];
                            sumX += vtx[ 3*myFace + 0];
                            sumY += vtx[ 3*myFace + 1 ];
                            sumZ += vtx[ 3*myFace + 2 ];
                            numVtx_To_cellGrid[ Grid[i][j] ] = i - NumSkips; // cell[ #vtx ] = i;
                        }
                        newVtx.push_back( sumX / GridSize );
                        newVtx.push_back( sumY / GridSize );
                        newVtx.push_back( sumZ / GridSize );
                    }
                }
                else {++NumSkips;}
            }
        }
        else if ( method == "Shape-Preserving" ) {
            // for each cell...
            for (int i = 0 ; i < level3D ; ++i) {
                // ... and for each normal direction...
                int sumItems = 0;
                for (int j = 0 ; j < 8 ; ++j) {
                    int GridSize = Grid[ 8*i + j ].size();
                    sumItems += GridSize;
                    if ( GridSize > 0 ) {
                        // Get vertex mean:
                        float sumX, sumY, sumZ;
                        sumZ = sumY = sumX = 0.0f;

                        for (int k = 0 ; k < GridSize; ++k) {
                            int myFace = Grid[ 8*i + j ][ k ];
                            sumX += vtx[ 3*myFace + 0 ];
                            sumY += vtx[ 3*myFace + 1 ];
                            sumZ += vtx[ 3*myFace + 2 ];

                            // link old vtx with new by describing which cell in the grid has it
                            numVtx_To_cellGrid[ Grid[8*i + j][k] ] = 8*i + j - NumSkips; // cell[ #vtx ] = i;
                        }

                        // Add those vertices to the newVtx vector
                        newVtx.push_back( sumX / GridSize );
                        newVtx.push_back( sumY / GridSize );
                        newVtx.push_back( sumZ / GridSize );

                    }
                    else ++NumSkips;
                }
            }
        }
        else if ( method == "Voxelize" ) {
            // TEST: centerpoint of each uniform grid square
            // For each position in the grid...
            for (int i = 0 ; i < level3D ; ++i) {
                int GridSize = Grid[ i ].size();
                if ( GridSize > 0 ) {
                    for (int j = 0 ; j < GridSize; ++j) {
                        // link old vtx with new by describing which cell has it
                        numVtx_To_cellGrid[ Grid[i][j] ] = i-NumSkips; // cell[ #vtx ] = i;
                    }
                    newVtx.push_back( min[0] + gridCellX*0.5  +  gridCellX* (i % LOD) );
                    newVtx.push_back( min[1] + gridCellY*0.5  +  gridCellY*((i / LOD)%LOD) );
                    newVtx.push_back( min[2] + gridCellZ*0.5  +  gridCellZ* (i / (LOD*LOD)) );
                }
                else {
                    ++NumSkips;
                }
            }
        }
        else /*if ( method == "Mean" )*/ {
            // For each position in the grid...
            for (int i = 0 ; i < level3D ; ++i) {
                int GridSize = Grid[ i ].size();
                if ( GridSize > 0 ) {
                    // Get vertex mean:
                    float sumX, sumY, sumZ;
                    sumZ = sumY = sumX = 0.0f;

                    for (int j = 0 ; j < GridSize; ++j) {
                        int myFace = Grid[ i ][ j ];
                        sumX += vtx[ 3*myFace + 0 ];
                        sumY += vtx[ 3*myFace + 1 ];
                        sumZ += vtx[ 3*myFace + 2 ];

                        // link old vtx with new by describing which cell in the grid has it
                        numVtx_To_cellGrid[ Grid[i][j] ] = i - NumSkips; // cell[ #vtx ] = i;
                    }

                    // Add those vertices to the newVtx vector
                    newVtx.push_back( sumX / GridSize );
                    newVtx.push_back( sumY / GridSize );
                    newVtx.push_back( sumZ / GridSize );

                } else {
                    ++NumSkips;
                }
            }
        }

        vtxPerLOD[NumLods]   = newVtx;
        // Get the vtxs from each face
        for (int i = 0 ; i < NumFaces ; ++i) {
            int vtx1 = faces[ 3*i + 0 ];
            int vtx2 = faces[ 3*i + 1 ];
            int vtx3 = faces[ 3*i + 2 ];

            // Get for each vtx its corresponding cell in the grid:
            int idx1 = numVtx_To_cellGrid[ vtx1 ];
            int idx2 = numVtx_To_cellGrid[ vtx2 ];
            int idx3 = numVtx_To_cellGrid[ vtx3 ];

            // If all vtxs belong to a different cell (i.e. no matching cells) and were marked
            if ( idx1 != idx2 and idx2 != idx3 and idx1 != idx3 and idx1 >= 0 and idx2 >= 0 and idx3 >= 0) {
                //Add those faces to the newFaces vector
                newFaces.push_back( idx1 );
                newFaces.push_back( idx2 );
                newFaces.push_back( idx3 );

            }
        }
        facesPerLOD[NumLods] = newFaces;

        newNormals.resize( vtx.size() );
        getNewNormals( vtxPerLOD[NumLods], facesPerLOD[NumLods], newNormals );
        normPerLOD[NumLods]  = newNormals;

        ++NumLods;
        std::cout << NumLods << "th item is the next iteration to add...\n";


        if (LOD == 2) LOD = RATIO*2;
    }

    std::cout << "Ended with " << NumLods << " items\n";
    //Finally add full detail LOD at the end
    vtxPerLOD[NumLods]   = vtx;
    facesPerLOD[NumLods] = faces;
    normPerLOD[NumLods]  = normals;


    return NumLods + 2;
}


