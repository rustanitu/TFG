#include "Viewer.h"
#include <volrend\VolWriter.h>

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
	Viewer::Instance ()->InitAndStart (argc, argv);
	Viewer::DestroyInstance ();
	return 0;
}