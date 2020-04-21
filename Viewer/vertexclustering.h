#ifndef VERTEXCLUSTERING_H
#define VERTEXCLUSTERING_H

#include "glwidget.h"
#include "mesh_io.h"
#include "./triangle_mesh.h"


#include <fstream>
#include <iostream>
#include <memory>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <limits>
#include <eigen3/Eigen/Geometry>


class VertexClustering
{
public:
    VertexClustering() {}
    void buildCluster( std::vector<float>& vtx, std::vector<int>& faces, std::vector<float>& norm,
                      Eigen::Vector3f& min, Eigen::Vector3f& max );

    std::vector < std::vector< float > > vtxPerLOD;
    std::vector < std::vector< int > >   facesPerLOD;
    std::vector < std::vector< float > > normPerLOD;
};

#endif // VERTEXCLUSTERING_H
