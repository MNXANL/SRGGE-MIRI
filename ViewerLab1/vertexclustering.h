#ifndef VERTEXCLUSTERING_H
#define VERTEXCLUSTERING_H

#include "glwidget.h"
#include "mesh_io.h"
#include "./triangle_mesh.h"


#include <fstream>
#include <iostream>
#include <memory>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <limits>
#include <eigen3/Eigen/Geometry>


class VertexClustering
{
public:
    int buildCluster( std::vector<float>& vtx, std::vector<int>& faces, std::vector<float>& normals,
                      Eigen::Vector3f& min, Eigen::Vector3f& max, std::string method );

    void calcQMatrices( std::vector<float>& vtx, std::vector<float>& norm );
    void getNewNormals(const std::vector<float> &newVtx, const std::vector<int> &newFaces, std::vector<float> &newNormals  );


    std::vector < std::vector< float > > vtxPerLOD;
    std::vector < std::vector< int > >   facesPerLOD;
    std::vector < std::vector< float > > normPerLOD;
private:
    int MAX_LOD;

    std::vector < std::vector< float > > Grid;
    std::vector< Eigen::Matrix4f > QMatrixPerVert;
};

#endif // VERTEXCLUSTERING_H
