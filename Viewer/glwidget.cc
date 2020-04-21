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

const char kPhongVertexShaderFile[] = "../shaders/phong.vert";
const char kPhongFragmentShaderFile[] = "../shaders/phong.frag";

const int kVertexAttributeIdx = 0;
const int kNormalAttributeIdx = 1;

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



GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent), initialized_(false), width_(0.0), height_(0.0), num_instances(1), dist_offset(1.0), myLod(0)
{
  setFocusPolicy(Qt::StrongFocus);
  iniTime = time( NULL );
  VAO = std::vector< GLuint >(10);
  vbo_v_id = std::vector< GLuint >(10);
  vbo_n_id = std::vector< GLuint >(10);
  faces_id = std::vector< GLuint >(10);
}

GLWidget::~GLWidget() {}

bool GLWidget::LoadModel(const QString &filename) {
  std::string file = filename.toUtf8().constData();
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
    // static VertexClustering LOD;
    LOD.buildCluster( mesh_->vertices_, mesh_->faces_, mesh_->normals_,   mesh_->min_, mesh_->max_ );

    // TODO(students): Create / Initialize buffers.
    for (int i = 0; i < 10; ++i) {
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

   /* ORIGINAL  VAO/VBO GENERATION
    // Initialize VBO for vertices
    glGenBuffers(1, &vbo_v_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_v_id);
    glBufferData(GL_ARRAY_BUFFER, mesh_->vertices_.size() * sizeof(float), &mesh_->vertices_[0], GL_STATIC_DRAW);
    glVertexAttribPointer(kVertexAttributeIdx, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(kVertexAttributeIdx);


    // Initialize VBO for normals
    glGenBuffers(1, &vbo_n_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_n_id);
    glBufferData(GL_ARRAY_BUFFER, mesh_->normals_.size() * sizeof(float), &mesh_->normals_[0], GL_STATIC_DRAW);
    glVertexAttribPointer(kNormalAttributeIdx, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(kNormalAttributeIdx);

    glBindVertexArray(0);

    // Initialize VBO for faces
    glGenBuffers(1, &faces_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_->faces_.size() * sizeof(int), &mesh_->faces_[0], GL_STATIC_DRAW);

    // END.*/

    emit SetFaces(QString(std::to_string(LOD.facesPerLOD[0].size() / 3).c_str()));
    emit SetVertices(
        QString(std::to_string(LOD.vtxPerLOD[0].size() / 3).c_str()));
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

  LoadModel("../models/sphere.ply");
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
      GLint projection_location, view_location, model_location, normal_matrix_location, numinst_location, offset_location;

      phong_program_->bind();
      projection_location = phong_program_->uniformLocation("projection");
      view_location = phong_program_->uniformLocation("view");
      model_location = phong_program_->uniformLocation("model");
      normal_matrix_location = phong_program_->uniformLocation("normal_matrix");
      numinst_location = phong_program_->uniformLocation("num_instances");
      offset_location = phong_program_->uniformLocation("offset");

      glUniformMatrix4fv(projection_location, 1, GL_FALSE, projection.data());
      glUniformMatrix4fv(view_location, 1, GL_FALSE, view.data());
      glUniformMatrix4fv(model_location, 1, GL_FALSE, model.data());
      glUniformMatrix3fv(normal_matrix_location, 1, GL_FALSE, normal.data());
      glUniform1i(numinst_location, num_instances );
      glUniform1f(offset_location, dist_offset );

      // TODO(students): Implement model rendering.
      glBindVertexArray(VAO[ myLod ]);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_id[ myLod ]);

      //glDrawElements(GL_TRIANGLES, mesh_->faces_.size(), GL_UNSIGNED_INT, 0);
      glDrawElementsInstanced(GL_TRIANGLES, LOD.facesPerLOD[ myLod ].size(), GL_UNSIGNED_INT, 0, num_instances * num_instances);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      std::cout << "LOD = " << myLod << std::endl;

      emit SetFaces(    QString(std::to_string(LOD.facesPerLOD[ myLod ].size() / 3).c_str()) );
      emit SetVertices( QString(std::to_string(LOD.vtxPerLOD[ myLod ].size()   / 3).c_str()) );
      // END.
    }
    // Framerate counter
    ++FPS;
    endTime = time( NULL );
    if ( endTime > iniTime ) {
        emit SetFramerate(QString( std::to_string( FPS / (endTime - iniTime) ).c_str() ));
        iniTime = endTime;
        FPS = 0;
    }

    model = camera_.SetIdentity();
  }
}

void GLWidget::SetNumInstances(int numInst) {
    num_instances = numInst;
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
