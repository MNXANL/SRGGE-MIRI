#include "vertexclustering.h"

#include "./mesh_io.h"
#include "./triangle_mesh.h"

using namespace std;

#define MAX_LOD 10


void VertexClustering::buildCluster( std::vector<float>& vtx, std::vector<int>& faces, std::vector<float>& norm,
                                     Eigen::Vector3f& min, Eigen::Vector3f& max )
{
    //Clear all previous data structures
    vtxPerLOD.clear();
    facesPerLOD.clear();
    normPerLOD.clear();

    // Use actual mesh values as LOD 0 (max detail)
    vtxPerLOD.push_back( vtx );
    facesPerLOD.push_back( faces );
    normPerLOD.push_back( norm );

    // Dimensions
    float xDim = (max[0] - min[0]);
    float yDim = (max[1] - min[1]);
    float zDim = (max[2] - min[2]);
    int NumVertices = vtx.size() / 3;
    int NumFaces = faces.size() / 3;

    // For each level of detail 4 to 1...
    for (int LOD = MAX_LOD ; LOD > 0; --LOD){
        int level3D = LOD*LOD*LOD; // LOD ^ 3

        // Create new structures for this LOD
        std::vector<float> newVtx(0);
        std::vector<int>   newFaces(0);
        //std::vector<float> newNorm(0); Declared below

        // Set 3D uniform grid size and dimensions per cell
        std::vector< std::vector<float> > Grid( level3D, std::vector<float>() );
        float gridCellX = xDim / LOD;
        float gridCellY = yDim / LOD;
        float gridCellZ = zDim / LOD;

        // For each vertex...
        for (int idx = 0 ; idx < NumVertices ; ++idx) {
            // Place index into grid; if index==LOD subtract one (rightmost edge of last cell per coordinate)
            int vtX = (int) ( (vtx[3*idx + 0] - min(0)) / gridCellX );      vtX -= (vtX == LOD);
            int vtY = (int) ( (vtx[3*idx + 1] - min(1)) / gridCellY );      vtY -= (vtY == LOD);
            int vtZ = (int) ( (vtx[3*idx + 2] - min(2)) / gridCellZ );      vtZ -= (vtZ == LOD);

            // Indexing = x + y*lvl + z*(lvl**2)
            int gridPos = vtX + LOD*(vtY + LOD*vtZ) ;
            Grid[ gridPos ].push_back( idx );
        }

        std::map< int, std::vector< int > > VF;
        std::vector< float > gridCellVtx_To_Index( NumVertices );

        // For each position in the grid...
        for (int i = 0 ; i < level3D ; ++i) {
            // Get vertex mean:
            int GridSize = Grid[ i ].size();
            if ( GridSize > 0 ) {
                int myFace = GridSize >> 1; // div 2

                // Add those vertices to the newVtx vector
                newVtx.push_back( vtx[ 3*myFace + 0 ] );
                newVtx.push_back( vtx[ 3*myFace + 1 ] );
                newVtx.push_back( vtx[ 3*myFace + 2 ] );

                for (int j = 0 ; j < GridSize; ++j) {
                    gridCellVtx_To_Index[ Grid[i][j] ] = i;
                }
            }
        }

        // Get the faces from each vtx (or viceversa TBClarified!!!)
        for (int myFace = 0 ; myFace < NumFaces ; ++myFace) {
            // Get face idx:
            int fc1 = gridCellVtx_To_Index[ faces[ 3*myFace + 0 ] ];
            int fc2 = gridCellVtx_To_Index[ faces[ 3*myFace + 1 ] ];
            int fc3 = gridCellVtx_To_Index[ faces[ 3*myFace + 2 ] ];

            // If each face coord belongs to different cell (i.e. no matching cells)
            if ( fc1 != fc2  and  fc1 != fc3  and  fc2 != fc3 ) {
                // Add those faces to the newFaces vector
                newFaces.push_back( fc1 );
                newFaces.push_back( fc2 );
                newFaces.push_back( fc3 );
            }
        }

        // Calc normals

        std::vector< float > faceNormals( newFaces.size() );
        std::vector<float> newNorm( newVtx.size() );

        //for each face grabbing vtx 3-by-3, get the normal per face
        for (int i = 0; i < (int) newFaces.size(); i+=3) {
            // get vertices from face
            Eigen::Vector3f vtx1( newVtx[ 3*faces[i+0] + 0 ], newVtx[ 3*faces[i+0] + 1 ], newVtx[ 3*faces[i+0] + 2 ] );
            Eigen::Vector3f vtx2( newVtx[ 3*faces[i+1] + 0 ], newVtx[ 3*faces[i+1] + 1 ], newVtx[ 3*faces[i+1] + 2 ] );
            Eigen::Vector3f vtx3( newVtx[ 3*faces[i+2] + 0 ], newVtx[ 3*faces[i+2] + 1 ], newVtx[ 3*faces[i+2] + 2 ] );

            //get its normal
            Eigen::Vector3f vtx1to2 = vtx2 - vtx1;
            Eigen::Vector3f vtx2to3 = vtx3 - vtx2;
            Eigen::Vector3f nrml = vtx1to2.cross( vtx2to3 );
            nrml.normalize();
            if ( nrml.norm() < 1e-9 )
                nrml = Eigen::Vector3f( 0.0, 0.0, 0.0 );

            //Sum normals to faceNormals
            faceNormals[i + 0] = nrml[0];
            faceNormals[i + 1] = nrml[1];
            faceNormals[i + 2] = nrml[2];
        }

        std::cout << "Vtxs size:  " << newVtx.size() << std::endl;
        std::cout << "Faces size: " << newFaces.size() << std::endl;
        std::cout << "Normals size: " << newFaces.size() << std::endl;

        // Insert new LOD into array
        vtxPerLOD.push_back( newVtx );
        facesPerLOD.push_back( newFaces );
        normPerLOD.push_back( newNorm );
    }


    //for (auto i : boundBox)
    //    std::cout << "BoundBox: " << i(0) << ", " << i(1) << ", " << i(2) << std::endl;

    // STEP 2 : split global bounding box
}
