/*!
\file Viewer.h
\brief Main class of the volume viewer.
\author Leonardo Quatrin Campagnolo
*/

#ifndef VIEWER_H
#define VIEWER_H

#include <GL/glew.h>

#include <vector>
#include <fstream>
#include <map>

#include "ViewerInterface.h"
#include "ViewMethod/ViewMethod.h"
#include "AutomaticTransferFunction\ATFGenerator.h"

#include <iup.h>
#include <iupgl.h>

#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>

#include <im.h>
#include <im_image.h>
/*! The class which has all the objects of the view
They're differente by the type of the model being acessed.
It's a Singleton class. So, can be acessed by:

%Example: \code
  Viewer* mviewer = Viewer::Instance();
  \endcode
*/

#define CLEAR_CONSOLE_VIEWERVR

class Viewer
{
public:
  /*! Returns the current instance of Viewer (lazy instantiation).
  */
  static Viewer *Instance ();

  /*! Verify if already exists an instance of the Viewer.
  \return exist or not exist (true or false)
  */
  static bool Exists ();

  /*! Just Destroy the instance of the singleton.
  */
  static void DestroyInstance ();

  void Init (int argc, char *argv[]);
  void Start ();
  
  void InitAndStart (int argc, char **argv);

  /*! Read all Transfer Functions from Resources folder.
  */
  void ReloadTransferFunction ();
  void SetTransferFunction (vr::TransferFunction* tf, std::string file);
  
  void SetVolumeModel (vr::Volume* vol, std::string file);

  /*! Build all viewers added to Viewer.
  */
  void BuildViewers ();
  void CleanViewers ();

  /*! Update the gui of the viewers connected to Viewer.
  */
  void UpdateIupUserInterfaces ();

  void GenerateVolHistogram ();

  /*! Current Volume Model being visualized.
  */
  std::string m_volumename;

  /*! Current Transfer Function being visualized.
  */
  std::string m_tf_name;

  /*! The current volume loaded to be viewed.*/
  vr::Volume* m_volume;
  /*! The current transfer function loeaded.*/
  vr::TransferFunction* m_transfer_function;
  std::string m_transfer_function_file;
  std::string m_volume_file;

  /*! Automatic Transfer Function Generator*/
  ATFGenerator* m_atfg;

  /*! The current ViewMethod being used.*/
  VRVIEWS m_current_view;

  /*! A vector with all the ViewMethods.*/
  std::map<VRVIEWS, ViewMethod*> m_viewmethods;

  int GetCanvasWidth ();
  int GetCanvasHeight ();

  /*! Current Width and height of canvas being rendered.*/
  int m_CurrentWidth, m_CurrentHeight;

  /*! Clear color*/
  lqc::Vector4f m_rendererClearColor;

  std::vector<std::string> m_vr_transfer_function;

  void SetCanvasSizes (int width, int height);
  int Redraw (Ihandle* handle, Ihandle* canvas);
  int Keyboard (Ihandle *ih, int c, int press);
  int MouseButton (Ihandle* ih, int button, int pressed, int x, int y, char* status);
  int MouseMotion (Ihandle *ih, int x, int y, char *status);

  void DisactiveInterfaces ();
  void MarkAllViewMethodsOutdated ();
  void CreateAllViewMethodIupInterfaces ();

  Ihandle* GetCanvasRenderer ();
  void UpdateAdInterface ();

  void SetClearColor (float r, float g, float b, float a);
  lqc::Vector4f GetClearColor ();

  void InitIup (int argc, char *argv[]);
  void InitGL (int argc, char *argv[]);

  void SaveViewerState ();
  void LoadViewerState ();

  bool FileDlg_VolumeModel ();
  bool FileDlg_TransferFunction ();

  bool SaveSnapshot (char* filename);

  static int SetBoundaryThickness(Ihandle* ih);
  static int SetGTresh(Ihandle* ih);
  static int SetMinHistogramValue(Ihandle* ih, int min);
  static int SetBoundary(Ihandle* ih, int boundary);
  
  ViewerInterface m_gui;
protected:
  /*! Constructor.*/
  Viewer ();
  /*! Destructor.*/
  virtual ~Viewer ();

private:
  /*! the pointer to the singleton instance.*/
  static Viewer* m_instance;

  int m_boundary;
  int m_boundary_thickness;
  float m_gtresh;
  float m_min_hist;
};

#endif