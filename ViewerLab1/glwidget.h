// Author: Marc Comino 2020

#ifndef GLWIDGET_H_
#define GLWIDGET_H_

#include <GL/glew.h>
#include <QGLWidget>
#include <QImage>
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QString>

#include <memory>

#include "./camera.h"
#include "./triangle_mesh.h"

class GLWidget : public QGLWidget {
  Q_OBJECT

 public:
  explicit GLWidget(QWidget *parent = nullptr);
  ~GLWidget();

  /**
   * @brief LoadModel Loads a PLY model at the filename path into the mesh_ data
   * structure.
   * @param filename Path to the PLY model.
   * @return Whether it was able to load the model.
   */
  bool LoadModel(const QString &filename);

 protected:
  /**
   * @brief initializeGL Initializes OpenGL variables and loads, compiles and
   * links shaders.
   */
  void initializeGL();

  /**
   * @brief resizeGL Resizes the viewport.
   * @param w New viewport width.
   * @param h New viewport height.
   */
  void resizeGL(int w, int h);

  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void keyPressEvent(QKeyEvent *event);

 private:
  /**
   * @brief program_ The phong shader program.
   */
  std::unique_ptr<QOpenGLShaderProgram> phong_program_;

  /**
   * @brief camera_ Class that computes the multiple camera transform matrices.
   */
  data_visualization::Camera camera_;

  /**
   * @brief mesh_ Data structure representing a triangle mesh.
   */
  std::unique_ptr<data_representation::TriangleMesh> mesh_;

  /**
   * @brief VAO Vertex Array Object id.
   */
  std::vector< GLuint > VAO;

  /**
   * @brief vbo_v_id Vertex Buffer id for vertices.
   */
  std::vector< GLuint > vbo_v_id;
  /**
  * @brief vbo_n_id Vertex Buffer id for normals.
  */
  std::vector< GLuint > vbo_n_id;

  /**
  * @brief faces_id Vertex Buffer id for faces.
  */
  std::vector< GLuint > faces_id;


  /**
  * @brief myLod Current selected level of detail.
  */
  int myLod;

  /**
  * @brief num_instances Number of instances of the model.
  */
  GLuint num_instances;

  /**
  * @brief num_instances Number of instances of the model.
  */
  std::string my_method;


  /**
  * @brief file Model filename.
  */
  std::string file;

  /**
  * @brief num_instances Number of instances of the model.
  */
  float dist_offset;

  /**
   * @brief initialized_ Whether the widget has finished initializations.
   */
  bool initialized_;

  /**
   * @brief width_ Viewport current width.
   */
  float width_;

  /**
   * @brief height_ Viewport current height.
   */
  float height_;


 protected slots:
  /**
   * @brief paintGL Function that handles rendering the scene.
   */
  void paintGL();

  /**
   * @brief SetNumInstances Sets the instances of the model to be rendered.
   */
  void SetNumInstances(int numInst);

  /**
   * @brief SetDistOffset Sets the distance between all instanced models.
   */
  void SetDistanceOffset(double offset);

  /**
   * @brief SetLevelOfDetail Sets the level of detail of the model.
   */
  void SetLevelOfDetail(int lod);

  /**
   * @brief SetMethod Sets the vertex-selection method for the clustering
   */
  void SetMethod(QString method);



 signals:
  /**
   * @brief SetFaces Signal that updates the interface label "Faces".
   */
  void SetFaces(QString);

  /**
   * @brief SetFaces Signal that updates the interface label "Vertices".
   */
  void SetVertices(QString);

  /**
   * @brief SetFaces Signal that updates the interface label "Framerate".
   */
  void SetFramerate(QString);

};

#endif  //  GLWIDGET_H_
