#include "Viewer.h"

#include "TankAPI/Tank.h"

int main(int argc, char **argv)
{
	Tank* tank = new Tank();
	if ( tank->Read("../../Modelos/TankModels/pituba.gmdl") )
	{
		ATFGenerator* atfg = new ATFGenerator(tank);
		if ( atfg->Init() )
		{
			if ( atfg->ExtractTransferFunction() )
			{
				printf("Sucesso ao extrair a funcao de transferencia!\n");
			}
			else
			{
				printf("Falha ao extrair a funcao de transferencia!\n");
			}
		}
		else
		{
			printf("Erro ao inicializar o ATFG!\n");
		}
	}
	else
	{
		printf("Erro ao abrir o reservatorio!\n");
	}

	//Viewer::Instance ()->InitAndStart (argc, argv);
	//Viewer::DestroyInstance ();
	return 0;
}