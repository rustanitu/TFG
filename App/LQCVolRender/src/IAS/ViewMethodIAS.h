/*!
\file IASViewMethod.h
\brief IAS ViewMethod class.
\author Leonardo Quatrin Campagnolo
*/

#ifndef IASVIEWMETHOD_H
#define IASVIEWMETHOD_H

#include "../Viewer.h"
#include "../ViewMethod/GLSLViewMethod.h"

#include "UserInterfaceIAS.h"
#include "RendererIAS.h"

class ViewMethodIAS : public GLSLViewMethod
{
public:
  /*! Constructor
  */
  ViewMethodIAS ();
  /*! Destructor
  */
  ~ViewMethodIAS ();

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

  virtual void SetRedisplay (bool vredisplay);

  /*! Set the use of gradient texture.
  */
  void SetUseGradient (int state);
  /*! Get the state of use of gradient texture.
  */
  int GetUseGradient ();
  
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

  void ChangeCurrentShaderProgram (int shader_id);
  void UseDoublePrecision (bool usedoubleprec);

  void ReloadTransferFunction();

private:
  std::string m_volumename;
  std::string m_trasnferfunctionname;

  bool m_redisplay;

  bool m_mousepressed;
  int m_motion_x, m_motion_y;

  RendererIAS m_renderer;
  UserInterfaceIAS m_gui;
};

#endif