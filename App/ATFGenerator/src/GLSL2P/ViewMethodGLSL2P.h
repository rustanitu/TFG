/*!
\file GLSL2PViewMethod.h
\brief GLSL2P ViewMethod class.
\author Leonardo Quatrin Campagnolo
*/

#ifndef GLSL2PVIEWMETHOD_H
#define GLSL2PVIEWMETHOD_H

#include "../Viewer.h"
#include "../ViewMethod/GLSLViewMethod.h"

#include "UserInterfaceGLSL2P.h"
#include "RendererGLSL2P.h"

class ViewMethodGLSL2P : public GLSLViewMethod
{
public:
  /*! Constructor*/
  ViewMethodGLSL2P ();
  /*! Destructor*/
  ~ViewMethodGLSL2P ();

  virtual int Idle_Action_CB (Ihandle* cnv_renderer);
  virtual int Keyboard_CB (Ihandle *ih, int c, int press);
  virtual int Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status);
  virtual int Motion_CB (Ihandle *ih, int x, int y, char *status);
  virtual int Resize_CB (Ihandle *ih, int width, int height);
  virtual void CreateIupUserInterface ();
  virtual void UpdateIupUserInterface ();
  virtual void ResetCamera ();
  
  virtual void SaveCameraState (std::string filename);
  virtual void LoadCameraState (std::string filename);

  virtual void BuildViewer ();
  virtual void CleanViewer ();

  virtual void CleanVolumeTexture()
  {
    delete m_renderer.m_glsl_volume;
    m_renderer.m_glsl_volume = NULL;
  }

  virtual void CleanTransferFunctionTexture()
  {
    delete m_renderer.m_glsl_transfer_function;
    m_renderer.m_glsl_transfer_function = NULL;
  }

  virtual void SetRedisplay (bool vredisplay);
  
  /*! Set if the renderer will automatic rotate the volume by adding rotations.*/
  void SetIterate (bool iter);
  /*! Get if the renderer is rotating the volume.*/
  bool GetIterate (); 

  void ReloadTransferFunction();
  
  /*! Set the automatic redisplay of the renderer.
  */
  void SetAutoRedisplay (bool autored);

  void ResetShaders (std::string shadername);
  void ApplyInputSliceSizes ();

  void SetSliceSizes (int ix, int lx, int iy, int ly, int iz, int lz);

  int GetInitXSlice ();
  int GetLastXSlice ();
  int GetInitYSlice ();
  int GetLastYSlice ();
  int GetInitZSlice ();
  int GetLastZSlice ();
  void SetInitXSlice (int value);
  void SetLastXSlice (int value);
  void SetInitYSlice (int value);
  void SetLastYSlice (int value);
  void SetInitZSlice (int value);
  void SetLastZSlice (int value);

  void AutoModeling ();
  void ApplyModeling (float xw, float yh, float zd);

  void SetCubeWidth (float w);
  void SetCubeHeight (float h);
  void SetCubeDepth (float z);
  float GetCubeWidth();
  float GetCubeHeight();
  float GetCubeDepth ();

  void SetGUICubeWidth (float w);
  void SetGUICubeHeight (float h);
  void SetGUICubeDepth (float z);
  float GetGUICubeWidth ();
  float GetGUICubeHeight ();
  float GetGUICubeDepth ();

  void UseDoublePrecision (bool usedp);

private:
  std::string m_volumename;
  std::string m_trasnferfunctionname;

  bool m_redisplay;

  bool m_mousepressed;
  int m_motion_x, m_motion_y;

  RendererGLSL2P m_renderer;
  UserInterfaceGLSL2P m_gui;
};

#endif