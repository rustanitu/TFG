#include "Viewer.h"
#include "AutomaticTransferFunction\RAWFile.h"

int main(int argc, char **argv)
{
	//RAWFile file("C:\\Users\\Rustam\\Projetos\\TFG\\Modelos\\VolumeModels\\GeneralModels\\Sphere100", 255, 255, 255);
	//if (file.Open())
	//	file.Close();
	//return 0;
	Viewer::Instance ()->InitAndStart (argc, argv);
	Viewer::DestroyInstance ();
	return 0;
}