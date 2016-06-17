/*!
\file IASViewMethod.h
\brief IAS ViewMethod class.
\author Leonardo Quatrin Campagnolo
*/

#ifndef RGLSL_VIEWMETHOD_H
#define RGLSL_VIEWMETHOD_H

#include "../Viewer.h"
#include "../ViewMethod/GLSLViewMethod.h"

#include "UserInterfaceEquidistant.h"
#include "RendererEquidistant.h"

class ViewMethodEquidistant : public GLSLViewMethod
{
public:
  /*! Constructor
  */
  ViewMethodEquidistant ();
  /*! Destructor
  */
  ~ViewMethodEquidistant ();

  virtual char* GetViewMethodName ();

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

private:
  std::string m_volumename;
  std::string m_trasnferfunctionname;

  bool m_mousepressed;
  int m_motion_x, m_motion_y;

  RendererEquidistant m_renderer;
  UserInterfaceEquidistant m_gui;
};

#endif