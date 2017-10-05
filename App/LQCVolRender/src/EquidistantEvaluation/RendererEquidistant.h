/*!
\file IASRenderer.h
\brief IAS renderer class.
\author Leonardo Quatrin Campagnolo
*/

#ifndef RGLSL_RENDERER_H
#define RGLSL_RENDERER_H

#include <atfg/Utils.h>
#include "../gbuffer.h"
#include <atfg/Volume.h>
#include <atfg/TransferFunction.h>
#include <glutils/GLUtils.h>
#include <math/MUtils.h>
#include <lqc/lqcdefines.h>

#include <glutils/GLShader.h>
#include <glutils/GLArrayObject.h>
#include <glutils/GLBufferObject.h>
#include <glutils/GLCamera.h>
#include <glutils/GLStackMatrix.h>

class RendererEquidistant
{
public:
  gl::GLTexture3D* m_glsl_volume;
  gl::GLTexture* m_glsl_transfer_function;

public:
  /*! Constructor*/
  RendererEquidistant ();
  /*! Destructor*/
  ~RendererEquidistant ();

  /*! Destroy all the objects of Renderer.*/
  void Destroy ();

  /*! Initialize the Renderer objects.*/
  void CreateScene (int CurrentWidth, int CurrentHeight, vr::Volume* volume, gl::GLTexture* tf, bool resetslices = true);
 
  /*! Render a frame of the scene.*/
  bool Render (int Width, int Height);
  virtual void Resize (int Width, int Height);
  void MouseButtonCb (int b, int p, int x, int y, char* st);
  void MouseMotionCb (int x, int y, char *status);

  void ReloadVolume (vr::Volume* volume);
  void ReloadTransferFunction (vr::TransferFunction* tfunction);
  void ResetModelMatrix ();
  void AutoResizeGeometry (vr::Volume* volume, int Width, int Height, bool stop_bef_shad_operations = false);

  void SetStepDistance (float stp);
  
  bool mousepressed;
  int motion_x, motion_y;
  int interaction_mode;

  /*! Struct to store the position and color of vertices to bind in a vertex buffer.
  \note used in CreateCube() function*/
  typedef struct Vertex
  {
    float Position[3];
    float Color[3];
  } Vertex;


private:
  void DrawFirstPass ();
  virtual void DrawSecondPass ();

  void CreateFirstPass ();
  virtual void CreateSecondPass ();
  void CreateBuffers ();

  void DestroyFirstPass ();
  void DestroySecondPass ();
  void DestroyBuffers ();

  gl::GLShader* shader_fpass;
  gl::GLShader* shader_spass;
  gl::GLArrayObject*  vao;
  gl::GLBufferObject* vbo;
  gl::GLBufferObject* ibo;

  gl::GLArrayObject*  svao;
  gl::GLBufferObject* svbo;
  gl::GLBufferObject* sibo;

  float m_geo_cubewidth;
  float m_geo_cubeheight;
  float m_geo_cubedepth;

  /*! Store the framebuffer with specific color attachments.*/
  GBuffer *m_glfbo;

  int canvas_width;
  int canvas_height;

  gl::StackMatrix stack_matrix;
  lqc::Camera_glm cam;
  glm::mat4 model;
};


#endif