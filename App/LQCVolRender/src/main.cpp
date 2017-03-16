#include "Viewer.h"

#include <atfg\VolWriter.h>

#include <atfg\TransferFunction.h>
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

  PredictionMap map(256, 256);
  if (!map.Init())
    return 0;
  //map.SetValue(1.0f, 64, 128);
  //map.SetValue(0.6f, 64, 129);
  //map.SetValue(0.4f, 64, 60);
  //map.SetValue(1.0f, 64, 64);
  //map.SetValue(0.2f, 130, 68);
  //map.SetValue(0.5f, 128 + 64, 64 + 32);
  //map.SetValue(0.7f, 128 - 64, 32 + 128);
  //map.SetValue(1.0f, 64, 125);
  //map.SetValue(0.5f, 64 + 128, 128);
  //map.SetValue(0.0f, 64 + 128 - 32, 128);
  //map.Interpolate();


  map.SetValue(1, 64, 128);
  map.SetValue(2, 64, 129);
  map.SetValue(3, 64, 60);
  map.SetValue(4, 64, 64);
  map.SetValue(5, 130, 68);
  map.SetValue(6, 128 + 64, 64 + 32);
  map.SetValue(7, 128 - 64, 32 + 128);
  map.SetValue(8, 64, 125);
  map.SetValue(9, 64 + 128, 128);
  map.SetValue(10, 64 + 128 - 32, 128);


  std::forward_list<PMCell*> defined_cells;
  std::forward_list<PMCell*> undefined_cells;
  int count = 1;
  for (int i = 0; i < MAX_V; ++i)
  {
    for (int j = 0; j < MAX_V; ++j)
    {
      if (map.IsDefined(i, j))
        defined_cells.push_front(map.GetCell(i, j));
      else
        undefined_cells.push_front(map.GetCell(i, j));
    }
  }
  //map.PredictWithRBF(defined_cells, undefined_cells);
  map.PredictWithInverseDistanceWeighting(defined_cells, undefined_cells, 1.8f);

  double* data = new double[MAX_V*MAX_V];
  for (int i = 0; i < MAX_V; ++i)
  {
    for (int j = 0; j < MAX_V; ++j)
    {
#if 0
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
  //IupSetAttribute(plot, "AXS_ZMIN", "-1.1");
  IupSetAttribute(plot, "AXS_ZMIN", "-20");
  IupSetAttribute(plot, "AXS_ZAUTOMAX", "NO");
  IupSetAttribute(plot, "AXS_ZMAX", "40.1");
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