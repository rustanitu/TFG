/*!
\file GLSLUserInterface.h
\brief GLSL ViewMethod GUI.
\author Leonardo Quatrin Campagnolo
*/

#ifndef GLSLUSERINTERFACE_H
#define GLSLUSERINTERFACE_H

#include <iup.h>
#include <iupgl.h>

class GLSLUserInterface
{
public:
  /*! Iup callback to change the color attachments that will be drew.
  */
  static int ListCB_VisModelChange (Ihandle* ih, char *text, int item, int state);
  /*! Iup callback when the number of samples is changed by the spinbox.
  */
  static int SpinCB_GLSLSamples (Ihandle* ih, int increment);
  /*! Iup callback to set the use or not use of gradient texture.
  */
  static int ToggleCB_GLSLUSEGRADIENT (Ihandle* ih, int state);
  /*! Iup callback to automatically rotate the volume.
  */
  static int ToggleCB_GLSLITERATE (Ihandle* ih, int state);

  static int ListCB_GLSLComposition (Ihandle* ih, char *text, int item, int state);

  static int SpinCB_InitSlicesX (Ihandle* ih, int increment);
  static int SpinCB_LastSlicesX (Ihandle* ih, int increment);
  static int SpinCB_InitSlicesY (Ihandle* ih, int increment);
  static int SpinCB_LastSlicesY (Ihandle* ih, int increment);
  static int SpinCB_InitSlicesZ (Ihandle* ih, int increment);
  static int SpinCB_LastSlicesZ (Ihandle* ih, int increment);

  static int ButtonCB_ReloadTextures (Ihandle* ih);

  static int ButtonCB_AutoModeling (Ihandle* ih);

public:
  /*! Constructor*/
  GLSLUserInterface ();
  /*! Destructor*/
  ~GLSLUserInterface ();

  /*! Generate all iup elements of the gui.*/
  void CreateIupInterface ();

  /*! Set the value of the iup_list_vismodes.*/
  void SetVisualizationModel (int render_mode);
  /*! Set the value of iup_spinbox_samples.*/
  void SetNumberOfSamples (int samples);
  /*! Set the value of the iup_toggle_usegradient.*/
  void SetGradientToggle (int gtoggle);
  /*! Set the value of the iup_toggle_iterate.*/
  void SetIterateToggle (bool itoggle);

  void SetInitXSlice (int initx);
  void SetLastXSlice (int lastx);
  void SetInitYSlice (int inity);
  void SetLastYSlice (int lasty);
  void SetInitZSlice (int initz);
  void SetLastZSlice (int lastz);

  void UpdateCubeSizes ();

  float m_gui_cubewidth;
  float m_gui_cubeheight;
  float m_gui_cubedepth;

private:

  Ihandle* m_iup_text_cubesizes;

  /*! List with the types for visualize the framebuffer color attachments.*/
  Ihandle* m_iup_list_vismodes;
  /*! Just a label to name the spinbox.*/
  Ihandle* m_iup_label_samples;
  /*! Spinbox with the number of samples being used per pixel.*/
  Ihandle* m_iup_spinbox_samples;

  Ihandle* m_iup_hbox_samples;

  /*! Toggle to control the use of the gradient texture.*/
  Ihandle* m_iup_toggle_usegradient;
  /*! Toggle to control the automatic rotation of the volume.*/
  Ihandle* m_iup_toggle_iterate;

  Ihandle* m_iup_label_rendertype;
  Ihandle* m_iup_list_rendertype;

  Ihandle* m_iup_label_slicesx;
  Ihandle* m_iup_spin_initslicex;
  Ihandle* m_iup_spin_lastslicex;
  Ihandle* m_iup_hbox_slicex;

  Ihandle* m_iup_label_slicesy;
  Ihandle* m_iup_spin_initslicey;
  Ihandle* m_iup_spin_lastslicey;
  Ihandle* m_iup_hbox_slicey;

  Ihandle* m_iup_label_slicesz;
  Ihandle* m_iup_spin_initslicez;
  Ihandle* m_iup_spin_lastslicez;
  Ihandle* m_iup_hbox_slicez;

  Ihandle* m_iup_button_slicesizes;

  Ihandle* m_iup_label_volumesizes;
  Ihandle* m_iup_text_volumesizes;
  Ihandle* m_iup_button_volumesizes;
  Ihandle* m_iup_hbox_volumesizes;
};


#endif