#include "Viewer.h"

#include <volrend\VolWriter.h>

#include <volrend\TransferFunction.h>
#include <iup_mglplot.h>

#if 1
int main(int argc, char **argv)
{
  IupOpen(&argc, &argv);
  IupPlotOpen();
  IupMglPlotOpen();
  IupGLCanvasOpen();

  Ihandle* plot = IupMglPlot();
  Ihandle* diag = IupDialog(IupVbox(plot, NULL));
  IupSetAttribute(diag, "SIZE", "HALFxFULL");

  PredictionMap<double, DoubleCell> map(256, 256);
  if (!map.Init())
    return 0;
  map.SetValue(1.0f, 64, 128);
  map.SetValue(1.0f, 64, 60);
  map.SetValue(1.0f, 64, 64);
  map.SetValue(1.0f, 130, 68);
  map.SetValue(1.0f, 128 + 64, 64 + 32);
  map.SetValue(1.0f, 128 - 64, 32 + 128);
  map.SetValue(1.0f, 64, 125);
  map.SetValue(1.0f, 64 + 128, 128);
  map.SetValue(1.0f, 64 + 128 - 32, 128);
  map.Interpolate();

  double* data = new double[MAX_V*MAX_V];
  for (int i = 0; i < MAX_V; ++i)
  {
    for (int j = 0; j < MAX_V; ++j)
    {
#if 1
      if (!map.IsDefined(i, j))
      {
        data[i + MAX_V*j] = -1.0f;
        continue;
      }
#endif
      double x = map.GetValue(i, j);
			double a = vr::TransferFunction::CenteredGaussianFunction(x, 1.0f, 1.0f, 0.0f);
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
  IupSetAttribute(plot, "AXS_ZMIN", "-1.1");
  IupSetAttribute(plot, "AXS_ZAUTOMAX", "NO");
  IupSetAttribute(plot, "AXS_ZMAX", "1.1");
  IupSetAttribute(plot, "AXS_XLABEL", "Scalar Value");
  IupSetAttribute(plot, "AXS_YLABEL", "Gradient");
  IupSetAttribute(plot, "AXS_ZLABEL", "Alpha");
  //IupSetAttribute(plot, "COLORSCHEME", "kw");
  IupSetAttribute(plot, "COLORSCHEME", "kkby");
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