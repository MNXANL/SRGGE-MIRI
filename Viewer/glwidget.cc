// Author: Marc Comino 2020

#include "glwidget.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "./mesh_io.h"
#include "./triangle_mesh.h"
#include "./vertexclustering.h"

using namespace std;

namespace {

const double kFieldOfView = 60;
const double kZNear = 0.01;
const double kZFar = 100;

int totalFrames;

const char kPhongVertexShaderFile[] = "../shaders/phong.vert";
const char kPhongFragmentShaderFile[] = "../shaders/phong.frag";

const int kVertexAttributeIdx = 0;
const int kNormalAttributeIdx = 1;

const GLfloat quadVertices[] = { // 3D positions of the quads in NDC
// Vtx coord in XYZ      UV texture pos
 -1.0f, -1.0f,  0.0f,     0.0f,  0.0f,
 -1.0f,  1.0f,  0.0f,     0.0f,  1.0f,
  1.0f, -1.0f,  0.0f,     1.0f,  0.0f,
  1.0f,  1.0f,  0.0f,     1.0f,  1.0f
};


const GLubyte quadIndices[] = { // indices per quad
  //0, 1, 2,      3, 2, 1
    1, 0, 2,      2, 3, 1
};




#define MAX_TRI_PER_FRAME 1000000

/*** FRAMERATE ***/
int FPS = 0;
time_t iniTime;
time_t endTime;

bool ReadFile(const std::string filename, std::string *shader_source) {
  std::ifstream infile(filename.c_str());

  if (!infile.is_open() || !infile.good()) {
    std::cerr << "Error " + filename + " not found." << std::endl;
    return false;
  }

  std::stringstream stream;
  stream << infile.rdbuf();
  infile.close();

  *shader_source = stream.str();
  return true;
}


bool LoadProgram(const std::string &vertex, const std::string &fragment, QOpenGLShaderProgram *program) {
  std::string vertex_shader, fragment_shader;
  bool res = ReadFile(vertex, &vertex_shader) && ReadFile(fragment, &fragment_shader);

  if (res) {
    program->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                     vertex_shader.c_str());
    program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                     fragment_shader.c_str());
    program->bindAttributeLocation("vertex", kVertexAttributeIdx);
    program->bindAttributeLocation("normal", kNormalAttributeIdx);
    program->link();
  }

  return res;
}

}  // namespace


// Vertex clustering LoD manager.
static VertexClustering LOD;

//std::vector< std::vector < std::vector <int> > > modelInstanceLOD( 3, std::vector<int>( 20, std::vector<int>(20) ) );
//std::vector< std::vector < std::vector <int> > > modelFrameLOD(    3, std::vector<int>( 20, std::vector<int>(20) ) );
std::vector < std::vector < int > > modelInstanceLOD( 50, std::vector<int>(50) );
std::vector < std::vector < int > > modelFrameLOD(    50, std::vector<int>(50) );


GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent), initialized_(false), width_(0.0), height_(0.0),
      triSum_(0), num_instances(1), dist_offset(1.0), myLod(0), hyst_( false ),
      my_method( "Mean" ), file("../models/sphere.ply")
{
  setFocusPolicy(Qt::StrongFocus);
  iniTime = time( NULL );
  VAO = std::vector< GLuint >(10);
  vbo_v_id = std::vector< GLuint >(10);
  vbo_n_id = std::vector< GLuint >(10);
  faces_id = std::vector< GLuint >(10);


}

GLWidget::~GLWidget() {}

bool GLWidget::LoadModel(const QString &filename, int slot) {
  /*std::string*/ file = filename.toUtf8().constData();
  size_t pos = file.find_last_of(".");
  std::string type = file.substr(pos + 1);

  std::unique_ptr<data_representation::TriangleMesh> mesh =
      std::make_unique<data_representation::TriangleMesh>();

  bool res = false;
  if (type.compare("ply") == 0) {
    res = data_representation::ReadFromPly(file, mesh.get());
  }

  if (res) {
    mesh_.reset(mesh.release());
    camera_.UpdateModel(mesh_->min_, mesh_->max_);

    int N = LOD.buildCluster( mesh_->vertices_, mesh_->faces_, mesh_->normals_,   mesh_->min_, mesh_->max_, my_method );

    // TODO(students): Create / Initialize buffers.
    for (int i = 0; i < N; ++i) {
        // Create & bind empty VAO
        glGenVertexArrays(1, &VAO[i]);
        glBindVertexArray(VAO[i]);

        // Initialize VBO for vertices
        glGenBuffers(1, &vbo_v_id[i]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_v_id[i]);
        glBufferData(GL_ARRAY_BUFFER, LOD.vtxPerLOD[i].size() * sizeof(float), &LOD.vtxPerLOD[i][0], GL_STATIC_DRAW);
        glVertexAttribPointer(kVertexAttributeIdx, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(kVertexAttributeIdx);


        // Initialize VBO for normals
        glGenBuffers(1, &vbo_n_id[i]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_n_id[i]);
        glBufferData(GL_ARRAY_BUFFER, LOD.normPerLOD[i].size() * sizeof(float), &LOD.normPerLOD[i][0], GL_STATIC_DRAW);
        glVertexAttribPointer(kNormalAttributeIdx, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(kNormalAttributeIdx);

        glBindVertexArray(0);

        // Initialize VBO for faces
        glGenBuffers(1, &faces_id[i]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_id[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, LOD.facesPerLOD[i].size() * sizeof(int), &LOD.facesPerLOD[i][0], GL_STATIC_DRAW);

    }
    // END.

    /*modelInstanceLOD.clear();
    modelInstanceLOD.push_back( std::vector< int > ( num_instances ) );
    for (int i = 0; i < num_instances; ++i) {
        modelInstanceLOD[i].push_back( num_instances );
        for (int j = 0; j < num_instances; ++j) {
            modelInstanceLOD[i][j] = -1;
        }
    }*/


    return true;
  }

  return false;
}

void GLWidget::initializeGL() {
  glewInit();

  glEnable(GL_NORMALIZE);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);

  phong_program_ = std::make_unique<QOpenGLShaderProgram>();

  bool res = LoadProgram(kPhongVertexShaderFile, kPhongFragmentShaderFile,
                         phong_program_.get());
  if (!res) exit(0);

  LoadModel("../models/sphere.ply", 0);

  totalFrames = 0;
  initialized_ = true;
}

void GLWidget::resizeGL(int w, int h) {
  if (h == 0) h = 1;
  width_ = w;
  height_ = h;

  camera_.SetViewport(0, 0, w, h);
  camera_.SetProjection(kFieldOfView, kZNear, kZFar);
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    camera_.StartRotating(event->x(), event->y());
  }
  if (event->button() == Qt::RightButton) {
    camera_.StartZooming(event->x(), event->y());
  }
  updateGL();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
  camera_.SetRotationX(event->y());
  camera_.SetRotationY(event->x());
  camera_.SafeZoom(event->y());
  updateGL();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    camera_.StopRotating(event->x(), event->y());
  }
  if (event->button() == Qt::RightButton) {
    camera_.StopZooming(event->x(), event->y());
  }
  updateGL();
}

void GLWidget::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Up) camera_.Zoom(-1);
  if (event->key() == Qt::Key_Down) camera_.Zoom(1);

  if (event->key() == Qt::Key_Left) camera_.Rotate(-1);
  if (event->key() == Qt::Key_Right) camera_.Rotate(1);

  if (event->key() == Qt::Key_W) camera_.Zoom(-1);
  if (event->key() == Qt::Key_S) camera_.Zoom(1);

  if (event->key() == Qt::Key_A) camera_.Rotate(-1);
  if (event->key() == Qt::Key_D) camera_.Rotate(1);

  if (event->key() == Qt::Key_I) camera_.Zoom(-1);
  if (event->key() == Qt::Key_J) camera_.Zoom(1);

  if (event->key() == Qt::Key_K) camera_.Rotate(-1);
  if (event->key() == Qt::Key_L) camera_.Rotate(1);

  if (event->key() == Qt::Key_R) {
    phong_program_.reset();
    phong_program_ = std::make_unique<QOpenGLShaderProgram>();
    LoadProgram(kPhongVertexShaderFile, kPhongFragmentShaderFile,
                phong_program_.get());
  }

  updateGL();
}

void GLWidget::paintGL() {
  glClearColor(0.55f, 0.63f, 0.77f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (initialized_) {
    camera_.SetViewport();

    Eigen::Matrix4f projection = camera_.SetProjection();
    Eigen::Matrix4f view = camera_.SetView();
    Eigen::Matrix4f model = camera_.SetModel();

    Eigen::Matrix4f t = view * model;
    Eigen::Matrix3f normal;
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j) normal(i, j) = t(i, j);

    normal = normal.inverse().transpose();

    if (mesh_ != nullptr) {
      GLint projection_location, view_location, model_location, normal_matrix_location,
            numinst_location, offset_location, mesh_position, instance_position;

    calculateLevelPerModelInstance( hyst_, model, view, totalFrames );

    triSum_ = 0;
    int vtxSum = 0;


    for ( int i = 0 ; i < num_instances ; ++i) {
        for ( int j = 0 ; j < num_instances ; ++j) {
            phong_program_->bind();
            projection_location = phong_program_->uniformLocation("projection");
            view_location = phong_program_->uniformLocation("view");
            model_location = phong_program_->uniformLocation("model");
            normal_matrix_location = phong_program_->uniformLocation("normal_matrix");
            numinst_location = phong_program_->uniformLocation("num_instances");
            offset_location = phong_program_->uniformLocation("offset");

            mesh_position = phong_program_->uniformLocation("meshPosition");
            instance_position = phong_program_->uniformLocation("myInstance");


            glUniformMatrix4fv(projection_location, 1, GL_FALSE, projection.data());
            glUniformMatrix4fv(view_location, 1, GL_FALSE, view.data());
            glUniformMatrix4fv(model_location, 1, GL_FALSE, model.data());
            glUniformMatrix3fv(normal_matrix_location, 1, GL_FALSE, normal.data());
            glUniform1i(numinst_location, num_instances );
            glUniform1f(offset_location, dist_offset );

            glUniform3f(mesh_position, 0.0, 0.0, 0.0 );
            glUniform1i(instance_position, num_instances*i + j );

            // myLod = modelInstanceLOD[0][i][j];
            myLod = modelInstanceLOD[i][j];

            //  model rendering.
            glBindVertexArray(VAO[ myLod ]);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_id[ myLod ]);

            glDrawElements(GL_TRIANGLES, LOD.facesPerLOD[ myLod ].size(), GL_UNSIGNED_INT, 0);
            //glDrawElementsInstanced(GL_TRIANGLES, LOD.facesPerLOD[ myLod ].size(), GL_UNSIGNED_INT, 0, num_instances * num_instances);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            triSum_ += ( LOD.facesPerLOD[ myLod ].size() / 3 );
            vtxSum  += ( LOD.vtxPerLOD[ myLod ].size()   / 3 );

       }
     }


      emit SetFaces(    QString( std::to_string( triSum_ ).c_str() ) );
      emit SetVertices( QString( std::to_string( vtxSum ).c_str() ) );
      // END.
    }
    
    // Framerate counter
    ++FPS;
    ++totalFrames;
    endTime = time( NULL );
    if ( endTime > iniTime ) {
        emit SetFramerate(QString( std::to_string( FPS / (endTime - iniTime) ).c_str() ));
        iniTime = endTime;
        FPS = 0;
    }

    model = camera_.SetIdentity();

    /* prevent overflows
    if ( totalFrames >= ( 2 << 30 ) ) {
        totalFrames = 0;
        for (int i = 0; i < 20; ++i ){
            for (int j = 0; j < 20; ++j ){
                modelFrameLOD[i][j] = 0;
            }
        }
    }
    */
  }
}





int GLWidget::getContribution( Eigen::Matrix4f& model, Eigen::Matrix4f& view, int& i, int& j, int OFFSET ) {
    // Get diagonal
    Eigen::Vector3f diagVec = mesh_->max_ - mesh_->min_ ;
    float d = diagVec.norm();


    float Xcoord = mesh_->max_[0];
    //Get viewpoint distance
    Eigen::Affine3f tV( Eigen::Translation3f( Eigen::Vector3f(
       2*Xcoord*float( i ) - ( num_instances / ( 2*Xcoord ) ),
       0,
       2*Xcoord*float( j ) - ( num_instances / ( 2*Xcoord ) )
    )));
    Eigen::Matrix4f mtx = tV.matrix();
    mtx = model * mtx;

    Eigen::Vector3f diagCtr = diagVec / 2;
    Eigen::Vector4f viewCtr (diagCtr[0], diagCtr[1], diagCtr[2], 1.0f); // center coordinates in Vec4 ....
                    viewCtr = view * mtx * viewCtr;                     // ... into view position
    Eigen::Vector3f Ctr( viewCtr[0], viewCtr[1], viewCtr[2] );

    float bigD = Ctr.norm();


    // Geometry size is implicit
    int L = modelInstanceLOD[i][j] + OFFSET;

    float myContribution = d / ( float( 2 << L ) * bigD );
          myContribution = d / ( pow( 2 , L ) * bigD );
    return myContribution;
}



std::pair<int, int> GLWidget::getMinPosition( Eigen::Matrix4f& model, Eigen::Matrix4f& view ) {
    std::pair<int, int> myPair{ -1, -1 };
    int MIN_COST = 10;

    //for (int i = 0; i < num_instances; ++i) {
    //    for (int j = 0; j < num_instances; ++j) {
    for (int i = num_instances - 1; i >= 0; --i) {
        for (int j = num_instances - 1; j >= 0; --j) {
            int LOD = modelInstanceLOD[i][j];
            if (0 <= LOD and LOD <= 4) {
                int myCost = getContribution( model, view, i, j, 1 ) - getContribution( model, view, i, j, 0 );
                if ( myPair.first == -1 ) {
                    myPair.first = i; myPair.second = j;
                    MIN_COST = myCost;
                }
                else {
                    if ( myCost < MIN_COST ) {
                        MIN_COST = myCost;
                        myPair.first = i; myPair.second = j;
                    }
                }
            }
        }
    }
    return myPair;
}



std::pair<int, int> GLWidget::getMaxPosition( Eigen::Matrix4f& model, Eigen::Matrix4f& view ) {
    std::pair<int, int> myPair{ -1, -1 };
    int MAX_COST = -10;
    for (int i = num_instances - 1; i >= 0; --i) {
        for (int j = num_instances - 1; j >= 0; --j) {
            int LOD = modelInstanceLOD[i][j];
            if (1 <= LOD and LOD <= 5) {
                int myCost = - getContribution( model, view, i, j, 0 ) + getContribution( model, view, i, j, -1 );
                if ( myPair.first == -1 ) {
                    myPair.first = i; myPair.second = j;

                    MAX_COST = myCost;
                }
                else {
                    if ( MAX_COST < myCost ) {
                        MAX_COST = myCost;
                        myPair.first = i; myPair.second = j;
                    }
                }
            }
        }
    }
    return myPair;
}

void GLWidget::calculateLevelPerModelInstance( bool hyst, Eigen::Matrix4f& model, Eigen::Matrix4f& view, int myFrame ) {

    std::pair<int, int> Pos;
    if ( MAX_TRI_PER_FRAME <= triSum_ ) // we must INCREASE some maximum LOD position (less poly)
        Pos = getMaxPosition( model, view );
    else if ( triSum_ <= MAX_TRI_PER_FRAME )// we could DECREASE some minimum LOD position (more poly)
        Pos = getMinPosition( model, view );
    else return;

    if ( Pos.first < 0 or Pos.second < 0) return;


    if ( not hyst ) {
        if ( MAX_TRI_PER_FRAME <= triSum_ and 0 < modelInstanceLOD[ Pos.first ][ Pos.second ]  ) {
            triSum_ -= LOD.facesPerLOD[ modelInstanceLOD[ Pos.first ][ Pos.second ] ].size() / 3; // subtract old LOD data
            --modelInstanceLOD[ Pos.first ][ Pos.second ];
            triSum_ += LOD.facesPerLOD[ modelInstanceLOD[ Pos.first ][ Pos.second ] ].size() / 3; // sum new LOD data
        }
        else if (  triSum_ < MAX_TRI_PER_FRAME and modelInstanceLOD[ Pos.first ][ Pos.second ] < 5 ) {
            triSum_ -= LOD.facesPerLOD[ modelInstanceLOD[ Pos.first ][ Pos.second ] ].size() / 3; // subtract old LOD data
            ++modelInstanceLOD[ Pos.first ][ Pos.second ];
            triSum_ += LOD.facesPerLOD[ modelInstanceLOD[ Pos.first ][ Pos.second ] ].size() / 3; // sum new LOD data
        }
    }
    else {
        if ( MAX_TRI_PER_FRAME <= triSum_ and 0 < modelInstanceLOD[ Pos.first ][ Pos.second ] and myFrame - modelFrameLOD[ Pos.first ][ Pos.second ] >= 15 ) {
            triSum_ -= LOD.facesPerLOD[ modelInstanceLOD[ Pos.first ][ Pos.second ] ].size() / 3; // subtract old LOD data
            --modelInstanceLOD[ Pos.first ][ Pos.second ];
            triSum_ += LOD.facesPerLOD[ modelInstanceLOD[ Pos.first ][ Pos.second ] ].size() / 3; // sum new LOD data

            modelFrameLOD[ Pos.first ][ Pos.second ] = myFrame;    // hysteriesis for MAX

        }
        else if ( triSum_ < MAX_TRI_PER_FRAME and modelInstanceLOD[ Pos.first ][ Pos.second ] < 5 and myFrame - modelFrameLOD[ Pos.first ][ Pos.second ] >= 15 ) {
            triSum_ -= LOD.facesPerLOD[ modelInstanceLOD[ Pos.first ][ Pos.second ] ].size() / 3; // subtract old LOD data
            ++modelInstanceLOD[ Pos.first ][ Pos.second ];
            triSum_ += LOD.facesPerLOD[ modelInstanceLOD[ Pos.first ][ Pos.second ] ].size() / 3; // sum new LOD data

            modelFrameLOD[ Pos.first ][ Pos.second ] = myFrame;    // hysteriesis for MIN
        }
    }

}






void GLWidget::SetNumInstances(int numInst) {
    num_instances = numInst;
    for (int i = num_instances; i < 50; ++i) {
        for (int j = num_instances; j < 50; ++j) {
            modelInstanceLOD[i][j] = 0;
            modelFrameLOD[i][j]    = 0;
        }
    }

    updateGL();
}

void GLWidget::SetDistanceOffset(double offset){
    dist_offset = (float) offset;
    updateGL();
}

void GLWidget::SetLevelOfDetail(int lod) {
    myLod = lod;
    updateGL();
}


void GLWidget::SetMethod(QString method) {
    my_method = method.toUtf8().constData();
    LoadModel( QString::fromUtf8(file.c_str()) );
    updateGL();
}



void GLWidget::SetHysteriesis(bool checked) {
    hyst_ = checked;
    updateGL();
}


