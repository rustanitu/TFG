/*!
\file ERN2DRenderer.h
\brief ERN2D renderer class.
\author Leonardo Quatrin Campagnolo
*/

#ifndef ERN2DRENDERER_H
#define ERN2DRENDERER_H

#include <GL/glew.h>
#include <ctime>

#include <volrend/ScalarField.h>
#include <volrend/TransferFunction.h>

#include <math/Matrix4.h>
#include <math/MUtils.h>

#include <glutils/GLShader.h>
#include <glutils/GLCamera.h>
#include <glutils/GLArrayObject.h>
#include <glutils/GLBufferObject.h>

#include <iup.h>
#include <iupgl.h>

#include <glutils/GLTexture2D.h>
#include <glutils/GLCamera.h>
#include <glutils/GLStackMatrix.h>

class GBuffer;

class ERN2DRenderer
{
public:
  /*! Constructor*/
  ERN2DRenderer ();
  /*! Destructor*/
  ~ERN2DRenderer ();

  /*! Initialize the Renderer objects.*/
  void CreateScene (int CurrentWidth, int CurrentHeight, vr::ScalarField* volume, gl::GLTexture* tf, bool resetslices = true);

  /*! Render a frame of the scene.*/
  bool Render (int Width, int Height);

  void MouseButtonCB (int button, int pressed, int x, int y, char* status);
  void MouseMotionCB (int x, int y, char* status);

  void Resize (int Width, int Height);

  /*! Destroy all the objects of Renderer.*/
  void Destroy ();

  void BindShaderUniforms ();
  void ResetGeometry ();

  /*! Generate the textures from volume.
  \param volume ScalarField pointer used to generate the 3D textures.
  */
  void ReloadVolume (vr::ScalarField* volume, vr::TransferFunction* tfunction, bool resetslicesizes = true);

  void SetXRotation (float radius);
  float GetXRotation ();
  void SetYRotation (float radius);
  float GetYRotation ();

  void ReloadShaders ();
  void ResetShaders (std::string shadername);

  int GetInitXSlice ();
  int GetLastXSlice ();
  int GetInitYSlice ();
  int GetLastYSlice ();
  int GetInitZSlice ();
  int GetLastZSlice ();

  void SetSliceSizes (int ix, int lx, int iy, int ly, int iz, int lz);

  void SetInitXSlice (int value);
  void SetLastXSlice (int value);
  void SetInitYSlice (int value);
  void SetLastYSlice (int value);
  void SetInitZSlice (int value);
  void SetLastZSlice (int value);

  void AutoModeling (vr::ScalarField* volume);
  void ApplyModeling (float xw, float yh, float zd);

  void SetCubeWidth (float w);
  void SetCubeHeight (float h);
  void SetCubeDepth (float z);
  float GetCubeWidth ();
  float GetCubeHeight ();
  float GetCubeDepth ();

  gl::GLTexture2D* m_glsl_preintegrated_texture;
private:
  void RenderQuad ();
  void CreateQuad ();
  void DestroyQuad ();

  /*! Struct to store the position and color of vertices to bind in a vertex buffer.
  \note used in CreateCube() function*/
  typedef struct Vertex
  {
    float Position[3];
    float Color[3];
  } Vertex;

  int interaction_mode;
  int motion_x;
  int motion_y;

  /*! quad render vao.*/
  gl::GLArrayObject* m_qvao;
  /*! quad render vbo.*/
  gl::GLBufferObject* m_qvbo;
  /*! quad render ibo.*/
  gl::GLBufferObject* m_qibo;

  gl::GLShader* m_shader_quadpass;

  /*! Store the framebuffer with specific color attachments.*/
  GBuffer *m_glfbo;


  unsigned m_FrameCount;
  clock_t m_LastTime;
  
  float m_cube_width;
  float m_cube_height;
  float m_cube_depth;

  float m_tex_width;
  float m_tex_height;
  float m_tex_depth;

  int m_init_slice_x;
  int m_last_slice_x;
  int m_init_slice_y;
  int m_last_slice_y;
  int m_init_slice_z;
  int m_last_slice_z;

  int canvas_width;
  int canvas_height;

  gl::StackMatrix stack_matrix;
  lqc::Camera_glm cam;
  glm::mat4 model;
};


#endif