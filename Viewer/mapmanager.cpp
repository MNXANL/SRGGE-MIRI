#include "mapmanager.h"

mapManager::mapManager()
{

}


void mapManager::loadMap( const std::string filename ) {
/*

      std::ifstream fin;

      fin.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
      if (!fin.is_open() || !fin.good()) return false;

      int vertices = 0, faces = 0;



      fin->getline(line, 100);
      if (strncmp(line, "txt", 3) != 0) return false;

      *vertices = 0;
      fin->getline(line, 100);
      while (strncmp(line, "end_header", 10) != 0) {
        if (strncmp(line, "element vertex", 14) == 0) *vertices = atoi(&line[15]);
        fin->getline(line, 100);
        if (strncmp(line, "element face", 12) == 0) *faces = atoi(&line[13]);
      }


      if (!ReadPlyHeader(&fin, &vertices, &faces)) {
        fin.close();
        return false;
      }

      mesh->vertices_.resize(static_cast<size_t>(vertices) * 3);
      ReadPlyVertices(&fin, mesh);

      mesh->faces_.resize(static_cast<size_t>(faces) * 3);
      ReadPlyFaces(&fin, mesh);

      fin.close();

      ComputeVertexNormals(mesh->vertices_, mesh->faces_, &mesh->normals_);
      ComputeBoundingBox(mesh->vertices_, mesh);

      return true;

      */
}
