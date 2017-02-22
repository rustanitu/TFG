#include "Viewer.h"

#include <volrend\VolWriter.h>

#include <volrend\TransferFunction2D.h>
#include <iup_mglplot.h>

#if 0
int main(int argc, char **argv)
{
  IupOpen(&argc, &argv);
  IupPlotOpen();
  IupMglPlotOpen();
  IupGLCanvasOpen();

  Ihandle* plot = IupMglPlot();
  Ihandle* diag = IupDialog(IupVbox(plot, NULL));
  IupSetAttribute(diag, "SIZE", "HALFxFULL");

  PredictionMap<double, DoubleCell> map(1, 256);
  if (!map.Init())
    return 0;
  map.SetValue(-1.0f, 0, 64);
  map.SetValue(0.5f, 0, 64 + 32);
  map.SetValue(0.5f, 0, 32 + 128);
  map.SetValue(1.0f, 0, 64 + 128);
  map.PredictWithInverseDistanceWeighting(1.8);

  double* data = new double[MAX_V*MAX_V];
  for (int i = 0; i < MAX_V; ++i)
  {
    for (int j = 0; j < MAX_V; ++j)
    {
      double x = map.GetValue(0, i);
      data[i + MAX_V*j] = x;
    }
  }

  IupSetAttribute(plot, "CLEAR", "YES");
  int index = IupMglPlotNewDataSet(plot, 1);
  IupMglPlotSetData(plot, index, data, MAX_V, MAX_V, 1);
  IupSetAttribute(plot, "DS_MODE", "PLANAR_SURFACE");
  IupSetAttribute(plot, "LEGEND", "NO");
  //IupSetAttribute(plot, "OPENGL", "YES");
  IupSetAttribute(plot, "LIGHT", "NO");
  IupSetAttribute(plot, "AXS_ZAUTOMIN", "NO");
  IupSetAttribute(plot, "AXS_ZMIN", "-1.001");
  IupSetAttribute(plot, "AXS_ZAUTOMAX", "NO");
  IupSetAttribute(plot, "AXS_ZMAX", "1.001");
  IupSetAttribute(plot, "AXS_XLABEL", "Scalar Value");
  IupSetAttribute(plot, "AXS_YLABEL", "Gradient");
  IupSetAttribute(plot, "AXS_ZLABEL", "Alpha");
  //IupSetAttribute(plot, "COLORSCHEME", "kw");
  IupSetAttribute(plot, "ROTATE", "90:0:0");
  IupSetAttribute(plot, "REDRAW", NULL);

  delete[] data;
  

  IupShowXY(diag, IUP_RIGHT, IUP_TOP);
  IupMainLoop();
	return 0;
}
#else
int main(int argc, char **argv)
{
  /********************************
  VolWriter vol("DividedBox", 56);
  if (vol.Open())
  {
  vol.WriteLine();
  vol.Close();
  }
  //*******************************/
  Viewer::Instance()->InitAndStart(argc, argv);
  Viewer::DestroyInstance();
  return 0;
}
#endif