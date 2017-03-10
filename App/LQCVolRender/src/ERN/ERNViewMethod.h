/*!
\file ERNViewMethod.h
\brief ERN ViewMethod class.
\author Leonardo Quatrin Campagnolo
*/

#ifndef ERNVIEWMETHOD_H
#define ERNVIEWMETHOD_H

#include "../Viewer.h"
#include "../ViewMethod/ViewMethod.h"

#include "ERNUserInterface.h"
#include "ERNRenderer.h"

class ERNViewMethod : public ViewMethod
{
public:
  /*! Constructor
  */
  ERNViewMethod ();
  /*! Destructor
  */
  ~ERNViewMethod ();

  virtual int Idle_Action_CB (Ihandle* cnv_renderer);
  virtual int Keyboard_CB (Ihandle *ih, int c, int press);
  virtual int Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status);
  virtual int Motion_CB (Ihandle *ih, int x, int y, char *status);
  virtual int Resize_CB (Ihandle *ih, int width, int height);
  virtual void CreateIupUserInterface ();
  virtual void UpdateIupUserInterface ();
  virtual void ResetCamera ();

  virtual void BuildViewer ();
  virtual void CleanViewer ();

  virtual void SetRedisplay (bool vredisplay);

  /*! Set render mode of the FrameBuffer Color attachments.
  */
  void SetRenderMode (int rendermode);

  /*! Set number of samples per pixel that pass through the volume.
  */
  void SetSamples (int samples);
  /*! Return the number of samples.
  */
  int GetSamples ();

  /*! Set the use of gradient texture.
  */
  void SetUseGradient (int state);
  /*! Get the state of use of gradient texture.
  */
  int GetUseGradient ();

  /*! Set if the renderer will automatic rotate the volume by adding rotations.
  */
  void SetIterate (bool iter);
  /*! Get if the renderer is rotating the volume.
  */
  bool GetIterate ();

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
  float GetCubeWidth ();
  float GetCubeHeight ();
  float GetCubeDepth ();

  void SetGUICubeWidth (float w);
  void SetGUICubeHeight (float h);
  void SetGUICubeDepth (float z);
  float GetGUICubeWidth ();
  float GetGUICubeHeight ();
  float GetGUICubeDepth ();

  void ReloadTransferFunction();

private:
  std::string m_volumename;
  std::string m_trasnferfunctionname;

  bool m_redisplay;
  bool m_auto_redisplay;

  bool m_mousepressed;
  int m_motion_x, m_motion_y;

  ERNRenderer m_renderer;
  ERNUserInterface m_gui;
};

#endif