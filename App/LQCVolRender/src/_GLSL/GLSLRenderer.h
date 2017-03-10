/*!
\file GLSLRenderer.h
\brief GLSL renderer class.
\author Leonardo Quatrin Campagnolo
*/

#ifndef GLSLRENDERER_H
#define GLSLRENDERER_H

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <ctime>

#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>

#include <math/Matrix4.h>
#include <math/MUtils.h>

#include <glutils/GLShader.h>
#include <glutils/GLCamera.h>
#include <glutils/GLArrayObject.h>
#include <glutils/GLBufferObject.h>

#include <iup.h>
#include <iupgl.h>

class GBuffer;

class GLSLRenderer
{
public:
  /*! Constructor*/
  GLSLRenderer ();
  /*! Destructor*/
  ~GLSLRenderer ();

  /*! Initialize the Renderer objects.*/
  void CreateScene (int CurrentWidth, int CurrentHeight, vr::Volume* volume, lqc::GLTexture1D* tf, bool resetslices = true);
 
  /*! Render a frame of the scene.*/
  bool Render (int Width, int Height);
  
  void Resize (int Width, int Height);

  /*! Destroy all the objects of Renderer.*/
  void Destroy ();

  void ResetGeometry ();

  /*! Generate the textures from volume.
  \param volume Volume pointer used to generate the 3D textures.
  */
  void ReloadVolume (vr::Volume* volume, bool resetslicesizes = true);
  /*! Generate the texture from transfer function.
  \param tfunction Transfer Function pointer used to generate the 1D texture.
  */
  void ReloadTransferFunction (vr::TransferFunction* tfunction);

  /*! Set the use of the gradient texture.*/
  void ResetUseGradient (int use_gradient, vr::Volume* volume);
  /*! Set the number of samples.*/
  void ResetSamples (int samples);

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

  void AutoModeling (vr::Volume* volume);
  void ApplyModeling (float xw, float yh, float zd);

  void SetCubeWidth (float w);
  void SetCubeHeight (float h);
  void SetCubeDepth (float z);
  float GetCubeWidth ();
  float GetCubeHeight ();
  float GetCubeDepth ();

  /*! automatic rotation at y axis of volume.*/
  bool m_iterate;
  /*! how the framebuffer color attachments are rendered.*/
  int m_render;
  
  /*! value of a shader uniform variable, setting the use of the gradient texture.*/
  int m_glsl_use_gradient;
  /*! value of a shader uniform variable, setting the number of samples used.*/
  int m_glsl_samples;

  /*! Store the texture of the current volume.*/
  lqc::GLTexture3D* m_glsl_volume;
  /*! Store the gradient texture of the current volume.*/
  lqc::GLTexture3D* m_glsl_gradient;
  /*! Store the texture of the current transfer function.*/
  lqc::GLTexture1D* m_glsl_transfer_function;

private:
  /*! First pass of Rendering algorithm.
  \note The volume rendering algorithm implemented is based on two pass volume rendering
  */
  void FirstPass ();
  /*! Second pass of Rendering algorithm.
  \note The volume rendering algorithm implemented is based on two pass volume rendering
  */
  void SecondPass ();

  /*! Just create a cube and the shader used with it.*/
  void CreateCube ();
  void DestroyCube ();
    
  /*! Just create a quad and the shader used with it.*/
  void CreateQuad ();
  void DestroyQuad ();

  /*! Just draw a cube and store the front faces and back faces in the FrameBuffer.*/
  void DrawCube (void);

  /*! Struct to store the position and color of vertices to bind in a vertex buffer.
  \note used in CreateCube() function*/
  typedef struct Vertex
  {
    float Position[4];
    float Color[4];
  } Vertex;

  /*! Struct to store the position and Texture coordinate of vertices to bind in a vertex buffer.
  \note used in CreateQuad() function*/
  typedef struct QuadVertex
  {
    float Position[4];
    float TextCoord[2];
  } QuadVertex;

  /*! first pass vao.*/
  lqc::GLArrayObject* m_fvao;
  /*! first pass vbo.*/
  lqc::GLBufferObject* m_fvbo;
  /*! first pass ibo.*/
  lqc::GLBufferObject* m_fibo;

  /*! second pass vao.*/
  lqc::GLArrayObject* m_svao;
  /*! second pass vbo.*/
  lqc::GLBufferObject* m_svbo;
  /*! second pass ibo.*/
  lqc::GLBufferObject* m_sibo;

  std::string m_shader_name;

  /*! shader variables for the first pass.*/
  lqc::GLShader* m_shader_firstpass;
  /*! shader variables for the second pass.*/
  lqc::GLShader* m_shader_secondpass;

  /*! Store the framebuffer with specific color attachments.*/
  GBuffer *m_glfbo;

  /*! volume rotation at x axis.*/
  float m_x_rotation;
  /*! volume rotation at y axis.*/
  float m_y_rotation;

  unsigned m_FrameCount;
  clock_t m_LastTime;

  lqc::Matrix4f m_ProjectionMatrix, m_ViewMatrix, m_ModelMatrix;

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
};


#endif