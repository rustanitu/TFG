#include "Viewer.h"

int main(int argc, char **argv)
{
	Viewer::Instance ()->InitAndStart (argc, argv);
	Viewer::DestroyInstance ();
	return 0;
}