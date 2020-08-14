#ifndef MAPMANAGER_H
#define MAPMANAGER_H


#include <mesh_io.h>
#include <assert.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>


class mapManager
{
public:
    mapManager();

    void loadMap( const std::string filename );

private:
    std::vector< std::string > map;
};

#endif // MAPMANAGER_H
