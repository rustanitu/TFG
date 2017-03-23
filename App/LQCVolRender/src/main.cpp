#include "Viewer.h"
#if 1

int main(int argc, char **argv)
{
  Viewer::Instance()->InitAndStart(argc, argv);
  Viewer::DestroyInstance();
  return 0;
}
#else
#include <atfg\TransferFunction.h>
#include <iup_mglplot.h>

void main0()
{
  PredictionMap map(3, 3);
  map.SetValue(0.8f, 0, 0);
  map.SetValue(0.724474f, 0, 1);
  map.SetValue(1.0685f, 0, 2);
  map.SetValue(0.5f, 1, 2);
  map.SetValue(0.2f, 2, 0);
  map.SetValue(0.284706f, 2, 2);
  map.Interpolate();

  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      std::cout << map.GetValue(i, j) << " ";
    }
    std::cout << std::endl;
  }
}

int main1(int argc, char **argv)
{
  IupOpen(&argc, &argv);
  IupPlotOpen();
  IupMglPlotOpen();
  IupGLCanvasOpen();

  Ihandle* plot = IupMglPlot();
  Ihandle* diag = IupDialog(IupVbox(plot, NULL));
  IupSetAttribute(diag, "SIZE", "HALFxFULL");

  PredictionMap map(256, 256);
  map.SetValue(1.0f, 64, 128);
  map.SetValue(0.6f, 64, 129);
  map.SetValue(0.4f, 64, 60);
  map.SetValue(1.0f, 64, 64);
  map.SetValue(0.2f, 130, 68);
  map.SetValue(0.5f, 128 + 64, 64 + 32);
  map.SetValue(0.7f, 128 - 64, 32 + 128);
  map.SetValue(1.0f, 64, 125);
  map.SetValue(0.5f, 64 + 128, 128);
  map.SetValue(0.0f, 64 + 128 - 32, 128);
  map.Interpolate();

  double* data = new double[MAX_V*MAX_V];
  for (int i = 0; i < MAX_V; ++i)
  {
    for (int j = 0; j < MAX_V; ++j)
    {
      data[i + MAX_V*j] = map.GetValue(i, j);
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
  IupSetAttribute(plot, "AXS_ZMIN", "-1.1");
  IupSetAttribute(plot, "AXS_ZAUTOMAX", "NO");
  IupSetAttribute(plot, "AXS_ZMAX", "1.1");
  IupSetAttribute(plot, "AXS_XLABEL", "Scalar Value");
  IupSetAttribute(plot, "AXS_YLABEL", "Gradient");
  IupSetAttribute(plot, "AXS_ZLABEL", "Alpha");
  //IupSetAttribute(plot, "COLORSCHEME", "kw");
  //IupSetAttribute(plot, "COLORSCHEME", "kkby");
  IupSetAttribute(plot, "ROTATE", "90:0:0");
  IupSetAttribute(plot, "REDRAW", NULL);

  delete[] data;


  IupShowXY(diag, IUP_RIGHT, IUP_TOP);
  IupMainLoop();
  return 0;
}
#endif