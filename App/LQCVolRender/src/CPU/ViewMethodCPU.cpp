#include "ViewMethodCPU.h"

#include "../Viewer.h"
#include "../CPUTimer.h"

#include <glutils/GLShader.h>

ViewMethodCPU::ViewMethodCPU ()
: ViewMethod (VRVIEWS::CPU)
{
	m_volumename = "";
	m_trasnferfunctionname = "";

	m_redisplay = true;
	m_show_redisplay_informations = true;
	
	m_canvaswidth = -1;
	m_canvasheight = -1;

	MarkOutdated ();
}

ViewMethodCPU::~ViewMethodCPU ()
{
}

int ViewMethodCPU::Idle_Action_CB (Ihandle* cnv_renderer)
{
	IupGLMakeCurrent (cnv_renderer);
	if (m_redisplay)
	{
		if (m_outdated)
		{
			if (Viewer::Instance ()->m_volumename.compare (m_volumename) != 0 ||
					Viewer::Instance ()->m_tf_name.compare (m_trasnferfunctionname) != 0)
			{
				m_volumename = Viewer::Instance ()->m_volumename;
				m_trasnferfunctionname = Viewer::Instance ()->m_tf_name;
			}
			m_viewer.SetModelVolumeAndTransferFunction ((vr::Volume*)Viewer::Instance ()->m_volume, Viewer::Instance ()->m_transfer_function);
			if (m_canvaswidth != Viewer::Instance ()->GetCanvasWidth () || m_canvasheight != Viewer::Instance ()->GetCanvasHeight ())
				Resize (Viewer::Instance ()->GetCanvasWidth (), Viewer::Instance ()->GetCanvasHeight ());
			m_outdated = false;
		}

		if (m_show_redisplay_informations)
		{
			#ifdef CLEAR_CONSOLE_VIEWERVR
				#ifdef WIN32
					if (system ("CLS")) system ("clear");
				#endif
			#endif
			printf ("---------------------CPU ViewMethod Redisplay---------------------------\n");
			printf ("CPU ViewMethod Redisplay Settings:\n");
			printf (" - Riemann Integral:\n");
			printf ("   -> Interval Distance for Riemman Method Set to : %lf\n", m_viewer.m_voleval.m_distance_intervals);
			printf ("\n");
			printf (" - Adaptive Simpson Integral:\n");
			printf ("   -> Using[Outer, Inner] Integral Errors : [%lf, %lf]\n", m_viewer.m_voleval.m_integral_error, m_viewer.m_voleval.m_error_inner_integral);
			printf ("   -> Interval Distance for Adaptive Method Set to : %lf\n", m_viewer.m_voleval.m_adaptive_distance_intervals);
			printf ("\n");
		}

		if (m_viewer.IsRenderErrorTest ())
			m_viewer.m_voleval.BeginErrorTestStructures (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);

		CPUTimer timer_clock;
		timer_clock.start ();
		m_viewer.Render (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
		timer_clock.stop ();

		m_gui.SetTimeElapsed (timer_clock.getCPUCurrSecs());
		m_gui.SetStatistics (m_viewer.m_voleval.m_statistics.n_gets_volume_values,
												 m_viewer.m_voleval.m_statistics.n_gets_tf_values,
												 0);

		printf ("CPU Redraw Done:\n  - Time Elapsed: %Lf seconds\n", timer_clock.getCPUCurrSecs ());
		if (m_viewer.IsRenderErrorTest ())
		{
			printf ("\n");
			m_viewer.PrintErrorTestResults ();
		}
		printf ("------------------------------------------------------------------------\n");

		m_redisplay = false;
		IupGLSwapBuffers (cnv_renderer);
	}
	return IUP_DEFAULT;
}

int ViewMethodCPU::Keyboard_CB (Ihandle *ih, int c, int press)
{
	if (press == 0)
	{
		switch (c)
		{
		case 'w':
			m_viewer.m_camera.Keyboard_cb (c, 0, 0);
			break;
		case 'a':
			m_viewer.m_camera.Keyboard_cb (c, 0, 0);
			break;
		case 's':
			m_viewer.m_camera.Keyboard_cb (c, 0, 0);
			break;
		case 'd':
			m_viewer.m_camera.Keyboard_cb (c, 0, 0);
			break;
		case 'e':
			m_viewer.m_camera.Keyboard_cb (c, 0, 0);
			break;
		case 'q':
			m_viewer.m_camera.Keyboard_cb (c, 0, 0);
			break;
		case K_C:
		case K_c:
			if (m_viewer.m_num_of_threads == 2)
				m_viewer.m_num_of_threads = 4;
			else if (m_viewer.m_num_of_threads == 4)
				m_viewer.m_num_of_threads = 2;
			printf ("Set Number of Threads equal to %d\n", m_viewer.m_num_of_threads);
			break;
		case K_V:
		case K_v:
			RecalculateAABBSizes ();
			ResetCamera ();
			break;
		case K_B:
		case K_b:
			m_viewer.m_camera.SetMovementSpeed (m_viewer.m_camera.GetMovementSpeed () - 1.0f);
			break;
		case K_N:
		case K_n:
			m_viewer.m_camera.SetMovementSpeed (m_viewer.m_camera.GetMovementSpeed () + 1.0f);
			break;
		case K_Y:
		case K_y:
			std::cout << "Set the new integral error value: ";
			{
				double ierror;
				std::cin >> ierror;
				SetIntegralError (ierror);
				std::cout << "Integral Error set to: " << ierror << "." << std::endl;
			}
			break;
		case K_U:
		case K_u:
			std::cout << "Set the new inner integral error value: ";
			{
				double ierror;
				std::cin >> ierror;
				SetInnerIntegralError (ierror);
				std::cout << "Inner Integral Error set to: " << ierror << "." << std::endl;
			}
			break;
		//case K_H:
		//case K_h:
		//  std::cout << "Set the new interval distance evaluation: ";
		//  {
		//    double idistance;
		//    std::cin >> idistance;
		//    SetIntervalDistance (idistance);
		//    std::cout << "interval distance evaluation set to: " << idistance << "." << std::endl;
		//  }
		//  break;
		//case K_K:
		//case K_k:
		//  std::cout << "Set the new interval distance: ";
		//  {
		//    double distintervals;
		//    std::cin >> distintervals;
		//    m_viewer.m_voleval.m_distance_intervals = distintervals;
		//    std::cout << "Opacity Correction set to: " << distintervals << "." << std::endl;
		//  }
		//  break;
		case K_R:
		case K_r:
			m_redisplay = true;
			break;
		case K_M:
		case K_m:
			m_viewer.ResetCamera (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
			break;
		case K_ESC:
			exit (EXIT_SUCCESS);
			break;
		case K_UP:
			m_viewer.m_camera.RotateX (-.1f);
			break;
		case K_DOWN:
			m_viewer.m_camera.RotateX (.1f);
			break;
		case K_LEFT:
			m_viewer.m_camera.RotateY (.1f);
			break;
		case K_RIGHT:
			m_viewer.m_camera.RotateY (-.1f);
			break;
		case K_Z:
		case K_z:
			std::cout << "Test: \n";
			{
				int X, Y;
				std::cout << "Pixel X: ";
				std::cin >> X;
				std::cout << "Pixel Y: ";
				std::cin >> Y;

				Ihandle* cnv = Viewer::Instance ()->GetCanvasRenderer ();
				IupGLMakeCurrent (cnv);
				m_viewer.TraceToTestErrorRay (X, Y);
				IupGLSwapBuffers (cnv);

				std::cout << "Teste realizado" << std::endl;
				return IUP_DEFAULT;
			}
			break;
		case K_X:
		case K_x:
			std::cout << "Atualizar variáveis do método de avaliação: \n";
			{
				int method;
				std::cout << "Escolher método para atualizar: ";
				std::cin >> method;


				return IUP_DEFAULT;
			}
			break;
		case K_O:
		case K_o:

			std::cout << "Tempo/Erro:\n";
			{
				int method;
				std::cout << "Escolher teste: ";
				std::cin >> method;
				switch (method)
				{
				case 1:
					ErrorTests ();
					break;
				case 2:
					StepChangeSimpsonTest ();
					break;
				case 7:
					break;
				default:
					break;
				}
			}
			return IUP_DEFAULT;
			break;
		case K_H:
		case K_h:
			std::cout << "Histogramas:\n";
			{
				int method;
				std::cout << "Escolher Histograma: ";
				std::cin >> method;
				switch (method)
				{
				case 1:
					HistogramGeneration ();
					break;
				case 2:
					TimeGeneration ();
					break;
				case 3:
					HistogramGeneration_minmax ();
					break;
				case 4:
					RiemannComparison ();
					break;
				default:
					break;
				}
			}
			return IUP_DEFAULT;
			break;
		case K_P:
		case K_p:
			Viewer::Instance ()->SaveSnapshot ("cpu_screenshot.png");
			return IUP_DEFAULT;
			break;
		case K_SP:
			printf ("Determinar passo máximo e minimo:\n");
			printf ("Maximo:");
			std::cin >> m_viewer.m_voleval.m_max_h_step;
			printf ("Minimo:");
			std::cin >> m_viewer.m_voleval.m_min_h_step;

			return IUP_DEFAULT;
			break;
		default:
			break;
		}
		m_redisplay = true;
	}
	return IUP_DEFAULT;
}

int ViewMethodCPU::Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
	if (button == 51 && pressed == 0)
	{
		Ihandle* cnv = Viewer::Instance()->GetCanvasRenderer ();
		IupGLMakeCurrent (cnv);
		m_viewer.TraceToTestErrorRay (x, m_canvasheight - y);
		IupGLSwapBuffers (cnv);
	}
	return IUP_DEFAULT;
}

int ViewMethodCPU::Motion_CB (Ihandle *ih, int x, int y, char *status)
{
	return IUP_DEFAULT;
}

int ViewMethodCPU::Resize_CB (Ihandle *ih, int width, int height)
{
	Resize (width, height);
	return IUP_DEFAULT;
}

void ViewMethodCPU::CreateIupUserInterface ()
{
	m_gui.CreateIupInterface ();
}

void ViewMethodCPU::UpdateIupUserInterface ()
{
	m_gui.SetUsingPThreads (m_viewer.m_pthread_on);
	m_gui.SetNumberOfThreads (GetNumberOfThreads());
	
	m_gui.SetTimeElapsed (0.0);
	m_gui.SetStatistics (m_viewer.m_voleval.m_statistics.n_gets_volume_values,
											 m_viewer.m_voleval.m_statistics.n_gets_tf_values,
											 0);
	m_gui.SetErrorValue (GetIntegralError());

	m_gui.SetSamples (m_viewer.m_voleval.m_samples);
	m_gui.SetAdaptiveSamples (m_viewer.m_voleval.m_adaptive_samples);
}

void ViewMethodCPU::ReloadTransferFunction()
{
  //m_renderer.ReloadTransferFunction(Viewer::Instance()->m_transfer_function);
  //UpdateIupUserInterface();
}

void ViewMethodCPU::ResetCamera ()
{
	m_viewer.ResetCamera (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
	m_redisplay = true;
}

void ViewMethodCPU::BuildViewer ()
{
	if (!m_built)
	{
		m_viewer.ResetAABB ((vr::Volume*)Viewer::Instance()->m_volume);

		Resize (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);

		SetAdaptiveSamples (4);
		SetSamples (64);

		m_viewer.m_voleval.m_integral_error = GetMaxError ();

		SetIntegralError (1.0);

		printf ("ViewMethodCPU Built\n");
		m_built = true;
	}
}

void ViewMethodCPU::CleanViewer ()
{
	if (m_built)
	{

		printf ("ViewMethodCPU Cleaned\n");
		m_built = false;
	}
}

void ViewMethodCPU::SetPThread (bool state)
{
	m_viewer.m_pthread_on = state;
}

bool ViewMethodCPU::IsUsingPThread ()
{
	return m_viewer.m_pthread_on;
}

void ViewMethodCPU::SetCurrentMethod (int item)
{
	m_viewer.m_voleval.m_current_method = item;
}

void ViewMethodCPU::SetSamples (int samples)
{
	if (samples >= 1)
		m_viewer.m_voleval.m_samples = samples;
}

int ViewMethodCPU::GetSamples ()
{
	return m_viewer.m_voleval.m_samples;
}

void ViewMethodCPU::SetAdaptiveSamples (int adap_samples)
{
	if (adap_samples >= 1)
		m_viewer.m_voleval.m_adaptive_samples = adap_samples;
}

int ViewMethodCPU::GetAdaptiveSamples ()
{
	return m_viewer.m_voleval.m_adaptive_samples;
}

void ViewMethodCPU::SetIntegralError (double error)
{
	m_viewer.m_voleval.m_integral_error = error;
	m_gui.SetErrorValue (error);
}

double ViewMethodCPU::GetIntegralError ()
{
	return m_viewer.m_voleval.m_integral_error;
}

void ViewMethodCPU::SetInnerIntegralError (double error)
{
	m_viewer.m_voleval.m_error_inner_integral = error;
}

double ViewMethodCPU::GetInnerIntegralError ()
{
	return m_viewer.m_voleval.m_error_inner_integral;
}

void ViewMethodCPU::SetIntervalDistance (double distance)
{
	m_viewer.m_voleval.m_adaptive_distance_intervals = distance;
}

double ViewMethodCPU::GetIntervalDistance ()
{
	return m_viewer.m_voleval.m_adaptive_distance_intervals;
}

double ViewMethodCPU::GetMinError ()
{
	return m_gui.GetMinError ();
}

double ViewMethodCPU::GetMaxError ()
{
	return m_gui.GetMaxError ();
}

void ViewMethodCPU::Resize (int width, int height)
{
	m_canvaswidth = width;
	m_canvasheight = height;

	if (m_viewer.m_pixels)
		delete[] m_viewer.m_pixels;
	m_viewer.m_pixels = new float[width*height * 4];
	m_viewer.ResetCamera (width, height);
}

int ViewMethodCPU::GetNumberOfThreads ()
{
	return m_viewer.m_num_of_threads;
}

void ViewMethodCPU::SetNumberOfThreads (int nthreads)
{
	if (nthreads == 2 || nthreads == 4)
		m_viewer.m_num_of_threads = nthreads;
}

void ViewMethodCPU::SetUseAlphaTruncation (bool state)
{
	m_viewer.m_voleval.m_alpha_truncation = state;
}

bool ViewMethodCPU::IsUsingAlphaTruncation ()
{
	return m_viewer.m_voleval.m_alpha_truncation;
}

VolumeEvaluator* ViewMethodCPU::GetVolumeEvaluator ()
{
	return &m_viewer.m_voleval;
}

void ViewMethodCPU::RecalculateAABBSizes ()
{
	float w = (float)Viewer::Instance ()->m_volume->GetWidth ();
	float h = (float)Viewer::Instance ()->m_volume->GetHeight ();
	float d = (float)Viewer::Instance ()->m_volume->GetDepth ();
	
	float x, y, z;
	
	if (m_viewer.m_scaletomaxanchorvalue)
	{
		float max = std::max (w, std::max (h, d));

		x = ((w / max) * max) / 2.0f;
		y = ((h / max) * max) / 2.0f;
		z = ((d / max) * max) / 2.0f;
	}
	else
	{
		x = w / 2.0f;
		y = h / 2.0f;
		z = d / 2.0f;
	}

  m_viewer.SetAABB((vr::Volume*)Viewer::Instance()->m_volume, -x, -y, -z, x, y, z);
}

lqc::Vector4f pixelerror;
err_type* gt_pixels;
void ViewMethodCPU::RenderErrorTest (CPUTimer* timeclock, lqc::Vector4f* accumerror, lqc::Vector4f* maxerror)
{
	int wt = Viewer::Instance ()->m_CurrentWidth;
	(*timeclock).start ();
	m_viewer.Render (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
	(*timeclock).stop ();
	for (int x = 0; x < Viewer::Instance ()->m_CurrentWidth; x++)
	{
		for (int y = 0; y < Viewer::Instance ()->m_CurrentHeight; y++)
		{
			pixelerror.x = fabs (gt_pixels[x * 4 + y*wt * 4] - m_viewer.m_pixels[x * 4 + y*wt * 4]);
			pixelerror.y = fabs (gt_pixels[x * 4 + y*wt * 4 + 1] - m_viewer.m_pixels[x * 4 + y*wt * 4 + 1]);
			pixelerror.z = fabs (gt_pixels[x * 4 + y*wt * 4 + 2] - m_viewer.m_pixels[x * 4 + y*wt * 4 + 2]);
			pixelerror.w = fabs (gt_pixels[x * 4 + y*wt * 4 + 3] - m_viewer.m_pixels[x * 4 + y*wt * 4 + 3]);

			(*accumerror).x += pixelerror.x;
			(*accumerror).y += pixelerror.y;
			(*accumerror).z += pixelerror.z;
			(*accumerror).w += pixelerror.w;

			(*maxerror).x = MAX ((*maxerror).x, pixelerror.x);
			(*maxerror).y = MAX ((*maxerror).y, pixelerror.y);
			(*maxerror).z = MAX ((*maxerror).z, pixelerror.z);
			(*maxerror).w = MAX ((*maxerror).w, pixelerror.w);
		}
	}
	IupGLSwapBuffers (Viewer::Instance ()->GetCanvasRenderer ());
}

void ViewMethodCPU::ErrorTests ()
{
	std::cout << "Iniciar bateria de testes: (Referencia - Riemann 0.01)\n";
	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	//Variáveis:
	CPUTimer R___0_010_timeclock;
	
	CPUTimer R___1_000_timeclock;
	lqc::Vector4f R___1_000_accumerror (0), R___1_000_maxerror (0);
	CPUTimer R___0_500_timeclock;
	lqc::Vector4f R___0_500_accumerror (0), R___0_500_maxerror (0);
	CPUTimer R___0_100_timeclock;
	lqc::Vector4f R___0_100_accumerror (0), R___0_100_maxerror (0);
	CPUTimer R___0_050_timeclock;
	lqc::Vector4f R___0_050_accumerror (0), R___0_050_maxerror (0);


	CPUTimer Sh__0_100_timeclock;
	lqc::Vector4f Sh__0_100_accumerror (0), Sh__0_100_maxerror (0);
	CPUTimer Sh__0_010_timeclock;
	lqc::Vector4f Sh__0_010_accumerror (0), Sh__0_010_maxerror (0);

	CPUTimer Se__0_100_timeclock;
	lqc::Vector4f Se__0_100_accumerror (0), Se__0_100_maxerror (0);
	CPUTimer Se__0_010_timeclock;
	lqc::Vector4f Se__0_010_accumerror (0), Se__0_010_maxerror (0);
	CPUTimer Se__0_001_timeclock;
	lqc::Vector4f Se__0_001_accumerror (0), Se__0_001_maxerror (0);
	
	CPUTimer Sep_0_100_timeclock;
	lqc::Vector4f Sep_0_100_accumerror (0), Sep_0_100_maxerror (0);
	CPUTimer Sep_0_010_timeclock;
	lqc::Vector4f Sep_0_010_accumerror (0), Sep_0_010_maxerror (0);
	CPUTimer Sep_0_001_timeclock;
	lqc::Vector4f Sep_0_001_accumerror (0), Sep_0_001_maxerror (0);

	CPUTimer Seq_0_100_timeclock;
	lqc::Vector4f Seq_0_100_accumerror (0), Seq_0_100_maxerror (0);
	CPUTimer Seq_0_010_timeclock;
	lqc::Vector4f Seq_0_010_accumerror (0), Seq_0_010_maxerror (0);
	CPUTimer Seq_0_005_timeclock;
	lqc::Vector4f Seq_0_005_accumerror (0), Seq_0_005_maxerror (0);
	CPUTimer Seq_0_001_timeclock;
	lqc::Vector4f Seq_0_001_accumerror (0), Seq_0_001_maxerror (0);
	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////

	int width = Viewer::Instance ()->m_CurrentWidth;
	int height = Viewer::Instance ()->m_CurrentHeight;
	IupGLMakeCurrent (Viewer::Instance ()->GetCanvasRenderer ());
	gt_pixels = new float[Viewer::Instance ()->m_CurrentWidth * Viewer::Instance ()->m_CurrentHeight * 4];

	int curmethod = m_viewer.m_voleval.m_current_method;
	m_viewer.m_voleval.m_current_method = VolumeEvaluator::C_RIEMANN_SUMMATION_RIGHT_HAND;
	m_viewer.m_voleval.m_distance_intervals = 0.01;
	R___0_010_timeclock.start ();
	m_viewer.Render (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
	R___0_010_timeclock.stop ();
	int wt = Viewer::Instance ()->m_CurrentWidth;
	for (int x = 0; x < Viewer::Instance ()->m_CurrentWidth; x++)
	{
		for (int y = 0; y < Viewer::Instance ()->m_CurrentHeight; y++)
		{
			gt_pixels[x * 4 + y*wt * 4] = m_viewer.m_pixels[x * 4 + y*wt * 4];
			gt_pixels[x * 4 + y*wt * 4 + 1] = m_viewer.m_pixels[x * 4 + y*wt * 4 + 1];
			gt_pixels[x * 4 + y*wt * 4 + 2] = m_viewer.m_pixels[x * 4 + y*wt * 4 + 2];
			gt_pixels[x * 4 + y*wt * 4 + 3] = m_viewer.m_pixels[x * 4 + y*wt * 4 + 3];
		}
	}
	IupGLSwapBuffers (Viewer::Instance ()->GetCanvasRenderer ());
	Viewer::Instance ()->SaveSnapshot ("ET_Result_R_0_010.png");
	printf ("R 0.01 Completado\n");
	
	m_viewer.m_voleval.m_distance_intervals = 1.0;
	RenderErrorTest (&R___1_000_timeclock, &R___1_000_accumerror, &R___1_000_maxerror);
	Viewer::Instance ()->SaveSnapshot ("ET_Result_R_1_000.png");
	GenerateErrorMap ("ErrorMap_RGB_R_1_000.png", "ErrorMap_RGBA_R_1_000.png", gt_pixels, width, height);
	printf ("R 1.00 Completado\n");

	m_viewer.m_voleval.m_distance_intervals = 0.5;
	RenderErrorTest (&R___0_500_timeclock, &R___0_500_accumerror, &R___0_500_maxerror);
	Viewer::Instance ()->SaveSnapshot ("ET_Result_R_0_500.png");
	GenerateErrorMap ("ErrorMap_RGB_R_0_500.png", "ErrorMap_RGBA_R_0_500.png", gt_pixels, width, height);
	printf ("R 0.50 Completado\n");

	m_viewer.m_voleval.m_distance_intervals = 0.1;
	RenderErrorTest (&R___0_100_timeclock, &R___0_100_accumerror, &R___0_100_maxerror);
	Viewer::Instance ()->SaveSnapshot ("ET_Result_R_0_100.png");
	GenerateErrorMap ("ErrorMap_RGB_R_0_100.png", "ErrorMap_RGBA_R_0_100.png", gt_pixels, width, height);
	printf ("R 0.10 Completado\n");

	m_viewer.m_voleval.m_distance_intervals = 0.05;
	RenderErrorTest (&R___0_050_timeclock, &R___0_050_accumerror, &R___0_050_maxerror);
	Viewer::Instance ()->SaveSnapshot ("ET_Result_R_0_050.png");
	GenerateErrorMap ("ErrorMap_RGB_R_0_050.png", "ErrorMap_RGBA_R_0_050.png", gt_pixels, width, height);
	printf ("R 0.05 Completado\n");

	m_viewer.m_voleval.m_current_method = VolumeEvaluator::A_SIMPSON_HALF_INTEGRATION;
	SetIntegralError (0.1);
	RenderErrorTest (&Sh__0_100_timeclock, &Sh__0_100_accumerror, &Sh__0_100_maxerror);
	Viewer::Instance ()->SaveSnapshot ("ET_Result_Sh_0_100.png");
	GenerateErrorMap ("ErrorMap_RGB_Sh_0_100.png", "ErrorMap_RGBA_Sh_0_100.png", gt_pixels, width, height);
	printf ("Sh 0.10 Completado\n");

	SetIntegralError (0.01);
	RenderErrorTest (&Sh__0_010_timeclock, &Sh__0_010_accumerror, &Sh__0_010_maxerror);
	Viewer::Instance ()->SaveSnapshot ("ET_Result_Sh_0_010.png");
	GenerateErrorMap ("ErrorMap_RGB_Sh_0_010.png", "ErrorMap_RGBA_Sh_0_010.png", gt_pixels, width, height);
	printf ("Sh 0.01 Completado\n");

	//SIMPSON QUEUE TESTES
	m_viewer.m_voleval.m_current_method = VolumeEvaluator::A_SIMPSON_HALF_QUEUE;
	SetIntegralError (0.1);
	RenderErrorTest (&Seq_0_100_timeclock, &Seq_0_100_accumerror, &Seq_0_100_maxerror);
	Viewer::Instance ()->SaveSnapshot ("ET_Result_Seq_0_100.png");
	GenerateErrorMap ("ErrorMap_RGB_Seq_0_100.png", "ErrorMap_RGBA_Seq_0_100.png", gt_pixels, width, height);
	printf ("Seq 0.10 Completado\n");

	SetIntegralError (0.01);
	RenderErrorTest (&Seq_0_010_timeclock, &Seq_0_010_accumerror, &Seq_0_010_maxerror);
	Viewer::Instance ()->SaveSnapshot ("ET_Result_Seq_0_010.png");
	GenerateErrorMap ("ErrorMap_RGB_Seq_0_010.png", "ErrorMap_RGBA_Seq_0_010.png", gt_pixels, width, height);
	printf ("Seq 0.01 Completado\n");

	SetIntegralError (0.005);
	RenderErrorTest (&Seq_0_005_timeclock, &Seq_0_005_accumerror, &Seq_0_005_maxerror);
	Viewer::Instance ()->SaveSnapshot ("ET_Result_Seq_0_005.png");
	GenerateErrorMap ("ErrorMap_RGB_Seq_0_005.png", "ErrorMap_RGBA_Seq_0_005.png", gt_pixels, width, height);
	printf ("Seq 0.005 Completado\n");

	SetIntegralError (0.001);
	RenderErrorTest (&Seq_0_001_timeclock, &Seq_0_001_accumerror, &Seq_0_001_maxerror);
	Viewer::Instance ()->SaveSnapshot ("ET_Result_Seq_0_001.png");
	GenerateErrorMap ("ErrorMap_RGB_Seq_0_001.png", "ErrorMap_RGBA_Seq_0_001.png", gt_pixels, width, height);
	printf ("Seq 0.001 Completado\n");

	printf ("-----------------------------------------------------------\n");
	printf ("\nTempo de Riemann 0.01 (Referencia): %.6lf\n\n", R___0_010_timeclock.getCPUTotalSecs ());
	printf ("Lista dos Métodos:\n");
	printf ("Riemann 1.0\n");
	printf ("Riemann 0.5\n");
	printf ("Riemann 0.1\n");
	printf ("Riemann 0.05\n");
	printf ("Simpson Half 0.1\n");
	printf ("Simpson Half 0.01\n");
	printf ("Simpson Queue 0.1\n");
	printf ("Simpson Queue 0.01\n");
	printf ("Simpson Queue 0.005\n");
	printf ("Simpson Queue 0.001\n");
	printf ("-----------------------------------------------------------\n");
	printf ("Tabela de Tempo:\n");
	printf ("%.6lf\n", R___1_000_timeclock.getCPUTotalSecs ());
	printf ("%.6lf\n", R___0_500_timeclock.getCPUTotalSecs ());
	printf ("%.6lf\n", R___0_100_timeclock.getCPUTotalSecs ());
	printf ("%.6lf\n", R___0_050_timeclock.getCPUTotalSecs ());
	printf ("%.6lf\n", Sh__0_100_timeclock.getCPUTotalSecs ());
	printf ("%.6lf\n", Sh__0_010_timeclock.getCPUTotalSecs ());
	printf ("%.6lf\n", Seq_0_100_timeclock.getCPUTotalSecs ());
	printf ("%.6lf\n", Seq_0_010_timeclock.getCPUTotalSecs ());
	printf ("%.6lf\n", Seq_0_005_timeclock.getCPUTotalSecs ());
	printf ("%.6lf\n", Seq_0_001_timeclock.getCPUTotalSecs ());
	printf ("-----------------------------------------------------------\n");
	printf ("Tabela de Erro Acumulado:\n");
	printf ("Canal R:\n");
	printf ("%.6f\n", R___1_000_accumerror.x);
	printf ("%.6f\n", R___0_500_accumerror.x);
	printf ("%.6f\n", R___0_100_accumerror.x);
	printf ("%.6f\n", R___0_050_accumerror.x);
	printf ("%.6f\n", Sh__0_100_accumerror.x);
	printf ("%.6f\n", Sh__0_010_accumerror.x);
	printf ("%.6f\n", Seq_0_100_accumerror.x);
	printf ("%.6f\n", Seq_0_010_accumerror.x);
	printf ("%.6f\n", Seq_0_005_accumerror.x);
	printf ("%.6f\n", Seq_0_001_accumerror.x);
	printf ("Canal G:\n");
	printf ("%.6f\n", R___1_000_accumerror.y);
	printf ("%.6f\n", R___0_500_accumerror.y);
	printf ("%.6f\n", R___0_100_accumerror.y);
	printf ("%.6f\n", R___0_050_accumerror.y);
	printf ("%.6f\n", Sh__0_100_accumerror.y);
	printf ("%.6f\n", Sh__0_010_accumerror.y);
	printf ("%.6f\n", Seq_0_100_accumerror.y);
	printf ("%.6f\n", Seq_0_010_accumerror.y);
	printf ("%.6f\n", Seq_0_005_accumerror.y);
	printf ("%.6f\n", Seq_0_001_accumerror.y);
	printf ("Canal B:\n");
	printf ("%.6f\n", R___1_000_accumerror.z);
	printf ("%.6f\n", R___0_500_accumerror.z);
	printf ("%.6f\n", R___0_100_accumerror.z);
	printf ("%.6f\n", R___0_050_accumerror.z);
	printf ("%.6f\n", Sh__0_100_accumerror.z);
	printf ("%.6f\n", Sh__0_010_accumerror.z);
	printf ("%.6f\n", Seq_0_100_accumerror.z);
	printf ("%.6f\n", Seq_0_010_accumerror.z);
	printf ("%.6f\n", Seq_0_005_accumerror.z);
	printf ("%.6f\n", Seq_0_001_accumerror.z);
	printf ("Canal A:\n");
	printf ("%.6f\n", R___1_000_accumerror.w);
	printf ("%.6f\n", R___0_500_accumerror.w);
	printf ("%.6f\n", R___0_100_accumerror.w);
	printf ("%.6f\n", R___0_050_accumerror.w);
	printf ("%.6f\n", Sh__0_100_accumerror.w);
	printf ("%.6f\n", Sh__0_010_accumerror.w);
	printf ("%.6f\n", Seq_0_100_accumerror.w);
	printf ("%.6f\n", Seq_0_010_accumerror.w);
	printf ("%.6f\n", Seq_0_005_accumerror.w);
	printf ("%.6f\n", Seq_0_001_accumerror.w);

	printf ("-----------------------------------------------------------\n");
	printf ("Tabela de Erro Maximo:\n");
	printf ("Canal R:\n");
	printf ("%.6f\n", R___1_000_maxerror.x);
	printf ("%.6f\n", R___0_500_maxerror.x);
	printf ("%.6f\n", R___0_100_maxerror.x);
	printf ("%.6f\n", R___0_050_maxerror.x);
	printf ("%.6f\n", Sh__0_100_maxerror.x);
	printf ("%.6f\n", Sh__0_010_maxerror.x);
	printf ("%.6f\n", Seq_0_100_maxerror.x);
	printf ("%.6f\n", Seq_0_010_maxerror.x);
	printf ("%.6f\n", Seq_0_005_maxerror.x);
	printf ("%.6f\n", Seq_0_001_maxerror.x);
	printf ("Canal G:\n");
	printf ("%.6f\n", R___1_000_maxerror.y);
	printf ("%.6f\n", R___0_500_maxerror.y);
	printf ("%.6f\n", R___0_100_maxerror.y);
	printf ("%.6f\n", R___0_050_maxerror.y);
	printf ("%.6f\n", Sh__0_100_maxerror.y);
	printf ("%.6f\n", Sh__0_010_maxerror.y);
	printf ("%.6f\n", Seq_0_100_maxerror.y);
	printf ("%.6f\n", Seq_0_010_maxerror.y);
	printf ("%.6f\n", Seq_0_005_maxerror.y);
	printf ("%.6f\n", Seq_0_001_maxerror.y);
	printf ("Canal B:\n");
	printf ("%.6f\n", R___1_000_maxerror.z);
	printf ("%.6f\n", R___0_500_maxerror.z);
	printf ("%.6f\n", R___0_100_maxerror.z);
	printf ("%.6f\n", R___0_050_maxerror.z);
	printf ("%.6f\n", Sh__0_100_maxerror.z);
	printf ("%.6f\n", Sh__0_010_maxerror.z);
	printf ("%.6f\n", Seq_0_100_maxerror.z);
	printf ("%.6f\n", Seq_0_010_maxerror.z);
	printf ("%.6f\n", Seq_0_005_maxerror.z);
	printf ("%.6f\n", Seq_0_001_maxerror.z);
	printf ("Canal A:\n");
	printf ("%.6f\n", R___1_000_maxerror.w);
	printf ("%.6f\n", R___0_500_maxerror.w);
	printf ("%.6f\n", R___0_100_maxerror.w);
	printf ("%.6f\n", R___0_050_maxerror.w);
	printf ("%.6f\n", Sh__0_100_maxerror.w);
	printf ("%.6f\n", Sh__0_010_maxerror.w);
	printf ("%.6f\n", Seq_0_100_maxerror.w);
	printf ("%.6f\n", Seq_0_010_maxerror.w);
	printf ("%.6f\n", Seq_0_005_maxerror.w);
	printf ("%.6f\n", Seq_0_001_maxerror.w);
	printf ("-----------------------------------------------------------\n");
	printf ("\n");

	m_viewer.m_voleval.m_current_method = curmethod;

	delete[] gt_pixels;
	gt_pixels = NULL;

	std::cout << "Bateria de testes finalizada: " << std::endl;
}

void ViewMethodCPU::SaveCameraState (std::string filename)
{
	m_viewer.SaveInitRtCameraState (filename);
}

void ViewMethodCPU::LoadCameraState (std::string filename)
{
	m_viewer.LoadInitRtCameraState (filename);
}

void ViewMethodCPU::MapErrorTests ()
{
	std::cout << "Generating error map" << std::endl;
	Ihandle* cnv = Viewer::Instance ()->GetCanvasRenderer ();
	IupGLMakeCurrent (cnv);
	float* gt_pixels = new float[Viewer::Instance ()->m_CurrentWidth * Viewer::Instance ()->m_CurrentHeight * 4];
	int wt = Viewer::Instance ()->m_CurrentWidth;
	for (int x = 0; x < Viewer::Instance ()->m_CurrentWidth; x++)
	{
		for (int y = 0; y < Viewer::Instance ()->m_CurrentHeight; y++)
		{
			gt_pixels[x * 4 + y*wt * 4] = m_viewer.m_pixels[x * 4 + y*wt * 4];
			gt_pixels[x * 4 + y*wt * 4 + 1] = m_viewer.m_pixels[x * 4 + y*wt * 4 + 1];
			gt_pixels[x * 4 + y*wt * 4 + 2] = m_viewer.m_pixels[x * 4 + y*wt * 4 + 2];
			gt_pixels[x * 4 + y*wt * 4 + 3] = m_viewer.m_pixels[x * 4 + y*wt * 4 + 3];
		}
	}

	int curmethod = m_viewer.m_voleval.m_current_method;
	m_viewer.m_voleval.m_current_method = VolumeEvaluator::C_RIEMANN_SUMMATION_RIGHT_HAND;

	m_viewer.m_voleval.m_distance_intervals = 0.01;
	m_viewer.Render (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);

	//m_viewer.PrintPixelsDifference (gt_pixels, Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);

	m_viewer.m_voleval.m_current_method = curmethod;
	std::cout << "Error map generated" << std::endl;
	IupGLSwapBuffers (cnv);
	Viewer::Instance ()->SaveSnapshot ("cpu_generatederrormap.png");
}

void ViewMethodCPU::GenerateErrorMap (char* filename, char* filename_alpha, err_type* gt_pixels, int width, int height)
{
	Ihandle* cnv = Viewer::Instance ()->GetCanvasRenderer ();
	IupGLMakeCurrent (cnv);
 // m_viewer.PrintPixelsDifference (gt_pixels, width, height);
	IupGLSwapBuffers (cnv);
	Viewer::Instance ()->SaveSnapshot (filename_alpha);
	{
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				int i = x * 4 + y*width * 4;
				m_viewer.m_pixels[i + 3] = 1;
			}
		}
		m_viewer.Display ();
	}
	IupGLSwapBuffers (cnv);
	Viewer::Instance ()->SaveSnapshot (filename);
}

void ViewMethodCPU::HistogramGeneration ()
{
	std::cout << "Geracao de histogramas" << std::endl;
	int curmethod = m_viewer.m_voleval.m_current_method;
	int distintervals = m_viewer.m_voleval.m_distance_intervals;
	maxerror[0] = 0; maxerror[1] = 0; maxerror[2] = 0; maxerror[3] = 0;
	raystraced = 0;
	IupGLMakeCurrent (Viewer::Instance ()->GetCanvasRenderer ());

	gt_pixels = RenderGroundTruth ();
	
	m_viewer.m_voleval.m_current_method = VolumeEvaluator::C_RIEMANN_SUMMATION_RIGHT_HAND;
	//RIEMANN
	m_viewer.m_voleval.m_distance_intervals = 0.5;
	err_type* r05_error = GetErrorFromVolEvalMethod (gt_pixels, "Riemann 0.5", maxerror);
	m_viewer.m_voleval.m_distance_intervals = 0.4;
	err_type* r04_error = GetErrorFromVolEvalMethod (gt_pixels, "Riemann 0.4", maxerror);
	m_viewer.m_voleval.m_distance_intervals = 0.3;
	err_type* r03_error = GetErrorFromVolEvalMethod (gt_pixels, "Riemann 0.3", maxerror);
	m_viewer.m_voleval.m_distance_intervals = 0.2;
	err_type* r02_error = GetErrorFromVolEvalMethod (gt_pixels, "Riemann 0.2", maxerror);
	m_viewer.m_voleval.m_distance_intervals = 0.1;
	err_type* r01_error = GetErrorFromVolEvalMethod (gt_pixels, "Riemann 0.1", maxerror);

	m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_COUPLED;
	SetIntegralError (0.01);
	err_type* opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Coupled 0.010", maxerror);
	SetIntegralError (0.005);
	err_type* opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Coupled 0.005", maxerror);
	SetIntegralError (0.001);
	err_type* opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Coupled 0.001", maxerror);

	m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_DECOUPLED;
	SetIntegralError (0.01);
	err_type* sep_opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Decoupled 0.010", maxerror);
	SetIntegralError (0.005);
	err_type* sep_opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Decoupled 0.005", maxerror);
	SetIntegralError (0.001);
	err_type* sep_opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Decoupled 0.001", maxerror);

	GenerateExtrapolatedError ("0010_Riemann_05.txt", r05_error   , 0.010);
	GenerateExtrapolatedError ("0010_Riemann_04.txt", r04_error   , 0.010);
	GenerateExtrapolatedError ("0010_Riemann_03.txt", r03_error   , 0.010);
	GenerateExtrapolatedError ("0010_Riemann_02.txt", r02_error   , 0.010);
	GenerateExtrapolatedError ("0010_Riemann_01.txt", r01_error   , 0.010);
	GenerateExtrapolatedError ("0010_Coupled.txt"   , opt_0010    , 0.010);
	GenerateExtrapolatedError ("0010_Decoupled.txt" , sep_opt_0010, 0.010);
	
	GenerateExtrapolatedError ("0005_Riemann_05.txt", r05_error   , 0.005);
	GenerateExtrapolatedError ("0005_Riemann_04.txt", r04_error   , 0.005);
	GenerateExtrapolatedError ("0005_Riemann_03.txt", r03_error   , 0.005);
	GenerateExtrapolatedError ("0005_Riemann_02.txt", r02_error   , 0.005);
	GenerateExtrapolatedError ("0005_Riemann_01.txt", r01_error   , 0.005);
	GenerateExtrapolatedError ("0005_Coupled.txt"   , opt_0005    , 0.005);
	GenerateExtrapolatedError ("0005_Decoupled.txt" , sep_opt_0005, 0.005);

	GenerateExtrapolatedError ("0001_Riemann_05.txt", r05_error   , 0.001);
	GenerateExtrapolatedError ("0001_Riemann_04.txt", r04_error   , 0.001);
	GenerateExtrapolatedError ("0001_Riemann_03.txt", r03_error   , 0.001);
	GenerateExtrapolatedError ("0001_Riemann_02.txt", r02_error   , 0.001);
	GenerateExtrapolatedError ("0001_Riemann_01.txt", r01_error   , 0.001);
	GenerateExtrapolatedError ("0001_Coupled.txt"   , opt_0001    , 0.001);
	GenerateExtrapolatedError ("0001_Decoupled.txt" , sep_opt_0001, 0.001);

	delete[] gt_pixels;        
	gt_pixels = NULL;             
	delete[] r05_error;
	delete[] r04_error;
	delete[] r03_error;
	delete[] r02_error;
	delete[] r01_error;

	delete[] opt_0010;
	delete[] opt_0005;
	delete[] opt_0001;

	delete[] sep_opt_0010;
	delete[] sep_opt_0005;
	delete[] sep_opt_0001;

	std::cout << "DONE" << std::endl;

	m_viewer.m_voleval.m_current_method = curmethod;
	m_viewer.m_voleval.m_distance_intervals = distintervals;
}

void ViewMethodCPU::GenerateHistogramError (std::string filename, err_type* p_errors)
{
	std::fstream histogram;
	histogram.open (filename, std::ios::in | std::ios::out | std::ios::app);
	int wt = Viewer::Instance ()->m_CurrentWidth;

	int histogram_a[26] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int histogram_r[26] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int histogram_g[26] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int histogram_b[26] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	
	for (int x = 0; x < Viewer::Instance ()->m_CurrentWidth; x++)
	{
		for (int y = 0; y < Viewer::Instance ()->m_CurrentHeight; y++)
		{
			int index = x * 4 + y*wt * 4;
			double errorbound_r = 0.0;
			double errorbound_g = 0.0;
			double errorbound_b = 0.0;
			double errorbound_a = 0.0;
			
			for (int t = 0; t < 26; t++, errorbound_r += hist_error_value_step[0])
				if (p_errors[index + 0] <= errorbound_r || (t + 1) == 26)
				{
					histogram_r[t]++;
					break;
				}
			for (int t = 0; t < 26; t++, errorbound_g += hist_error_value_step[1])
				if (p_errors[index + 1] <= errorbound_g || (t + 1) == 26)
				{
					histogram_g[t]++;
					break;
				}
			for (int t = 0; t < 26; t++, errorbound_b += hist_error_value_step[2])
				if (p_errors[index + 2] <= errorbound_b || (t + 1) == 26)
				{
					histogram_b[t]++;
					break;
				}
			for (int t = 0; t < 26; t++, errorbound_a += hist_error_value_step[3])
				if (p_errors[index + 3] <= errorbound_a || (t + 1) == 26)
				{
					histogram_a[t]++;
					break;
				}
		}
	}
	
	histogram << histogram_r[0] - raystraced << "\t" << histogram_g[0] - raystraced << "\t" << histogram_b[0] - raystraced << "\t" << histogram_a[0] - raystraced << "\n";
	for (int t = 1; t < 26; t++)
		histogram << histogram_r[t] << "\t" << histogram_g[t] << "\t" << histogram_b[t] << "\t" << histogram_a[t] << "\n";

	histogram.close ();
}

void ViewMethodCPU::StepChangeSimpsonTest ()
{
	std::cout << "Erro e tempo dos algoritmo de Simpson com h maximo sendo mudado" << std::endl;
	int curmethod = m_viewer.m_voleval.m_current_method;
	int distintervals = m_viewer.m_voleval.m_distance_intervals;
	
	maxerror[0] = 0; maxerror[1] = 0; maxerror[2] = 0; maxerror[3] = 0;
	raystraced = 0;

	IupGLMakeCurrent (Viewer::Instance ()->GetCanvasRenderer ());

	gt_pixels = RenderGroundTruth ();

	m_viewer.m_voleval.m_current_method = VolumeEvaluator::A_SIMPSON_HALF_QUEUE_ITERATION;
	SetIntegralError (0.01);
	m_viewer.m_voleval.m_min_h_step = 0.04;

	//SIMPSON HALF_QUEUE 0.1
	m_viewer.m_voleval.m_max_h_step = 10.0;
	err_type* shq001_error = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half ITERATION 0.01 10", maxerror);
	//SIMPSON HALF_QUEUE 0.08
	m_viewer.m_voleval.m_max_h_step = 8.0;
	err_type* shq001_error8 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half ITERATION 0.01 8", maxerror);
	//SIMPSON HALF_QUEUE 0.06
	m_viewer.m_voleval.m_max_h_step = 6.0;
	err_type* shq001_error6 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half ITERATION 0.01 6", maxerror);
	//SIMPSON HALF_QUEUE 0.04
	m_viewer.m_voleval.m_max_h_step = 4.0;
	err_type* shq001_error4 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half ITERATION 0.01 4", maxerror);

	hist_error_value_step[0] = maxerror[0] / 20.0;
	hist_error_value_step[1] = maxerror[1] / 20.0;
	hist_error_value_step[2] = maxerror[2] / 20.0;
	hist_error_value_step[3] = maxerror[3] / 20.0;

	GenerateHistogramError ("0_h001_S_0_1.txt", shq001_error);
	GenerateHistogramError ("1_h001_S_0_08.txt", shq001_error8);
	GenerateHistogramError ("3_h001_S_0_06.txt", shq001_error6);
	GenerateHistogramError ("4_h001_S_0_04.txt", shq001_error4);
	std::cout << "Rays not analyzed: " << raystraced << std::endl;
	std::cout << "max_error:\n" << maxerror[0] << "\n" << maxerror[1] << "\n" << maxerror[2] << "\n" << maxerror[3] << std::endl;
	std::cout << "error_steps:\n" << hist_error_value_step[0] << "\n" << hist_error_value_step[1] << "\n" << hist_error_value_step[2] << "\n" << hist_error_value_step[3] << std::endl;

	delete[] gt_pixels;
	gt_pixels = NULL;
	delete[] shq001_error;
	delete[] shq001_error8;
	delete[] shq001_error6;
	delete[] shq001_error4;

	std::cout << "DONE" << std::endl;

	m_viewer.m_voleval.m_current_method = curmethod;
	m_viewer.m_voleval.m_distance_intervals = distintervals;
	m_viewer.m_voleval.m_min_h_step = 0.01;
	m_viewer.m_voleval.m_max_h_step = 10.0;
}

err_type* ViewMethodCPU::RenderGroundTruth ()
{
	int curmethod = m_viewer.m_voleval.m_current_method;
	int distanceinterval = m_viewer.m_voleval.m_distance_intervals;

	m_viewer.m_voleval.m_current_method = VolumeEvaluator::C_RIEMANN_SUMMATION_RIGHT_HAND;
	m_viewer.m_voleval.m_distance_intervals = 0.01;
	
	CPUTimer r001_timer;
	r001_timer.start ();
	m_viewer.Render (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
	r001_timer.stop ();
	
	raystraced = m_viewer.rays_traced;

	int wt = Viewer::Instance ()->m_CurrentWidth;
	err_type* groundtruthpixels = new err_type[Viewer::Instance ()->m_CurrentWidth * Viewer::Instance ()->m_CurrentHeight * 4];
	for (int x = 0; x < Viewer::Instance ()->m_CurrentWidth; x++)
		for (int y = 0; y < Viewer::Instance ()->m_CurrentHeight; y++)
			for (int i = 0; i < 4; i++)
				groundtruthpixels[x * 4 + y*wt * 4 + i] = (err_type)m_viewer.m_pixels[x * 4 + y*wt * 4 + i];

	std::cout << r001_timer.getCPUTotalSecs () << "\tGround truth time (Riemann 0.01)" << std::endl;

	m_viewer.m_voleval.m_current_method = curmethod;
	m_viewer.m_voleval.m_distance_intervals = distanceinterval;

	return groundtruthpixels;
}

err_type* ViewMethodCPU::GetErrorFromVolEvalMethod(err_type* GroundTruth, std::string feedback_message, err_type* maxerror)
{
	err_type* method_error = new err_type[Viewer::Instance()->m_CurrentWidth * Viewer::Instance()->m_CurrentHeight * 4];

	CPUTimer method_timer;
	method_timer.start();
	m_viewer.Render(Viewer::Instance()->m_CurrentWidth, Viewer::Instance()->m_CurrentHeight);
	method_timer.stop();

	int wt = Viewer::Instance()->m_CurrentWidth;
	for (int x = 0; x < Viewer::Instance()->m_CurrentWidth; x++)
		for (int y = 0; y < Viewer::Instance()->m_CurrentHeight; y++)
		{
			int index = x * 4 + y*wt * 4;
			method_error[index + 0] = (err_type)m_viewer.m_pixels[index + 0];
			method_error[index + 1] = (err_type)m_viewer.m_pixels[index + 1];
			method_error[index + 2] = (err_type)m_viewer.m_pixels[index + 2];
			method_error[index + 3] = (err_type)m_viewer.m_pixels[index + 3];

			maxerror[0] = std::max(maxerror[0], method_error[index + 0]);
			maxerror[1] = std::max(maxerror[1], method_error[index + 1]);
			maxerror[2] = std::max(maxerror[2], method_error[index + 2]);
			maxerror[3] = std::max(maxerror[3], method_error[index + 3]);
		}
	std::cout << method_timer.getCPUTotalSecs() << '\t' << feedback_message << std::endl;

	return method_error;
}

err_type* ViewMethodCPU::GetErrorFromVolEvalMethod2 (err_type* GroundTruth, std::string feedback_message, err_type* maxerror)
{
	err_type* method_error = new err_type[Viewer::Instance ()->m_CurrentWidth * Viewer::Instance ()->m_CurrentHeight * 4];

	CPUTimer method_timer;
	method_timer.start ();
	m_viewer.Render (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
	method_timer.stop ();

	int wt = Viewer::Instance ()->m_CurrentWidth;
	for (int x = 0; x < Viewer::Instance ()->m_CurrentWidth; x++)
		for (int y = 0; y < Viewer::Instance ()->m_CurrentHeight; y++)
		{
			int index = x * 4 + y*wt * 4;
			method_error[index + 0] = (err_type)m_viewer.m_pixels[index + 0];
			method_error[index + 1] = (err_type)m_viewer.m_pixels[index + 1];
			method_error[index + 2] = (err_type)m_viewer.m_pixels[index + 2];
			method_error[index + 3] = (err_type)m_viewer.m_pixels[index + 3];

			maxerror[0] = std::max (maxerror[0], method_error[index + 0]);
			maxerror[1] = std::max (maxerror[1], method_error[index + 1]);
			maxerror[2] = std::max (maxerror[2], method_error[index + 2]);
			maxerror[3] = std::max (maxerror[3], method_error[index + 3]);
		}
	std::cout << method_timer.getCPUTotalSecs () << '\t' << feedback_message << std::endl;

	return method_error;
}

void ViewMethodCPU::GetTimeFromVolEvalMethod (std::string feedback_message)
{
	CPUTimer method_timer;
	method_timer.start ();
	m_viewer.Render (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
	method_timer.stop ();
	std::cout << method_timer.getCPUTotalSecs () << "\t" << feedback_message << std::endl;
}

void ViewMethodCPU::GenerateHistogramErrorControlled (std::string filename, err_type* p_errors)
{
	std::fstream histogram;
	histogram.open (filename, std::ios::in | std::ios::out | std::ios::app);
	int wt = Viewer::Instance ()->m_CurrentWidth;
	
	//0.001 | 0.00390625 | 0.005 | 0.01 | 0.02 | 0.03 | 0.04 | 0.05 | 0.1 | > 0.1
	double error_h[11] = { 0.0, 0.001, 0.00390625, 0.005, 0.01, 0.02, 0.03, 0.04, 0.05, 0.1, 1};
	int size = 11;

	int histogram_a[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int histogram_r[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int histogram_g[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int histogram_b[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	for (int x = 0; x < Viewer::Instance ()->m_CurrentWidth; x++)
	{
		for (int y = 0; y < Viewer::Instance ()->m_CurrentHeight; y++)
		{
			int index = x * 4 + y*wt * 4;

			for (int t = 0; t < size; t++)
				if (p_errors[index + 0] <= error_h[t] || (t + 1) == size)
				{
					histogram_r[t]++;
					break;
				}
			for (int t = 0; t < size; t++)
				if (p_errors[index + 1] <= error_h[t] || (t + 1) == size)
				{
					histogram_g[t]++;
					break;
				}
			for (int t = 0; t < size; t++)
				if (p_errors[index + 2] <= error_h[t] || (t + 1) == size)
				{
					histogram_b[t]++;
					break;
				}
			for (int t = 0; t < size; t++)
				if (p_errors[index + 3] <= error_h[t] || (t + 1) == size)
				{
					histogram_a[t]++;
					break;
				}
		}
	}

	histogram << histogram_r[0] - raystraced << "\t" << histogram_g[0] - raystraced << "\t" << histogram_b[0] - raystraced << "\t" << histogram_a[0] - raystraced << "\n";
	for (int t = 1; t < size; t++)
		histogram << histogram_r[t] << "\t" << histogram_g[t] << "\t" << histogram_b[t] << "\t" << histogram_a[t] << "\n";

	histogram.close ();
}

void ViewMethodCPU::GenerateExtrapolatedError2(err_type* GroundTruth, char* filename, err_type* p_colors, float error)
{

	err_type* img = new err_type[Viewer::Instance()->m_CurrentWidth * Viewer::Instance()->m_CurrentHeight * 4];
	for (int x = 0; x < Viewer::Instance()->m_CurrentWidth*Viewer::Instance()->m_CurrentHeight; x++)
		img[x] = p_colors[x];

	int wt = Viewer::Instance()->m_CurrentWidth;
	for (int x = 0; x < Viewer::Instance()->m_CurrentWidth; x++)
	{
		for (int y = 0; y < Viewer::Instance()->m_CurrentHeight; y++)
		{
			int index = x * 4 + y*wt * 4;

			if (fabs(p_colors[index + 0] - GroundTruth[index + 0]) > error ||
				fabs(p_colors[index + 1] - GroundTruth[index + 1]) > error ||
				fabs(p_colors[index + 2] - GroundTruth[index + 2]) > error ||
				fabs(p_colors[index + 3] - GroundTruth[index + 3]) > error)
			{
		m_viewer.m_pixels[index + 1] = 0.0;
				m_viewer.m_pixels[index] = m_viewer.m_pixels[index + 2] = m_viewer.m_pixels[index + 3] = 1.0;
			}
			else
			{
				m_viewer.m_pixels[index] = p_colors[index];
				m_viewer.m_pixels[index + 1] = p_colors[index + 1];
				m_viewer.m_pixels[index + 2] = p_colors[index + 2];
				m_viewer.m_pixels[index + 3] = p_colors[index + 3];
			}

		}
	}

	m_viewer.Display();

	IupGLSwapBuffers(Viewer::Instance()->GetCanvasRenderer());
	Viewer::Instance()->SaveSnapshot(filename);

	delete[] img;
}

void ViewMethodCPU::GenerateExtrapolatedError (std::string filename, err_type* p_errors, float error)
{
	std::fstream histogram;
	histogram.open (filename, std::ios::in | std::ios::out | std::ios::app);
	int wt = Viewer::Instance ()->m_CurrentWidth;
	
	int r = 0, g = 0, b = 0, a = 0;

	for (int x = 0; x < Viewer::Instance ()->m_CurrentWidth; x++)
	{
		for (int y = 0; y < Viewer::Instance ()->m_CurrentHeight; y++)
		{
			int index = x * 4 + y*wt * 4;

			if (p_errors[index + 0] > error)
				r++;
			if (p_errors[index + 1] > error)
				g++;
			if (p_errors[index + 2] > error)
				b++;
			if (p_errors[index + 3] > error)
				a++;
		}
	}

	histogram << raystraced << "\n";
	histogram 
		<< r << " & " 
		<< g << " & " 
		<< b << " & "
		<< a << "\n";

	histogram 
		<< 100.0 * ((double)r / (double)raystraced) << " & "
		<< 100.0 * ((double)g / (double)raystraced) << " & "
		<< 100.0 * ((double)b / (double)raystraced) << " & "
		<< 100.0 * ((double)a / (double)raystraced) << "\n";
	histogram.close ();
}

void ViewMethodCPU::RunMethod (std::string feedback_message)
{
	CPUTimer method_timer;
	method_timer.start ();
	m_viewer.Render (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
	method_timer.stop ();
	std::cout << method_timer.getCPUTotalSecs () << '\t' << feedback_message << std::endl;
}

void ViewMethodCPU::TimeGeneration ()
{
	std::cout << "Geracao de tempo" << std::endl;
	int curmethod = m_viewer.m_voleval.m_current_method;
	int distintervals = m_viewer.m_voleval.m_distance_intervals;
	IupGLMakeCurrent (Viewer::Instance ()->GetCanvasRenderer ());

	m_viewer.m_voleval.m_current_method = VolumeEvaluator::C_RIEMANN_SUMMATION_RIGHT_HAND;
	m_viewer.m_voleval.m_distance_intervals = 0.5;
	RunMethod ("Riemann 0.5");
	m_viewer.m_voleval.m_distance_intervals = 0.4;
	RunMethod ("Riemann 0.4");
	m_viewer.m_voleval.m_distance_intervals = 0.3;
	RunMethod ("Riemann 0.3");
	m_viewer.m_voleval.m_distance_intervals = 0.2;
	RunMethod ("Riemann 0.2");
	m_viewer.m_voleval.m_distance_intervals = 0.1;
	RunMethod ("Riemann 0.1");

	m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_COUPLED;
	SetIntegralError (0.01);
	RunMethod ("Simpson Half Iterate 0.010");
	SetIntegralError (0.005);
	RunMethod ("Simpson Half Iterate 0.005");
	SetIntegralError (0.001);
	RunMethod ("Simpson Half Iterate 0.001");

	m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_DECOUPLED;
	SetIntegralError (0.01);
	RunMethod ("Simpson Half Iterate Separated Optmimized 0.010");
	SetIntegralError (0.005);
	RunMethod ("Simpson Half Iterate Separated Optmimized 0.005");
	SetIntegralError (0.001);
	RunMethod ("Simpson Half Iterate Separated Optmimized 0.001");

	std::cout << "DONE" << std::endl;

	m_viewer.m_voleval.m_current_method = curmethod;
	m_viewer.m_voleval.m_distance_intervals = distintervals;
}

void ViewMethodCPU::HistogramGeneration_minmax ()
{
	std::cout << "Geracao de histogramas" << std::endl;
	int curmethod = m_viewer.m_voleval.m_current_method;
	int distintervals = m_viewer.m_voleval.m_distance_intervals;
	maxerror[0] = 0; maxerror[1] = 0; maxerror[2] = 0; maxerror[3] = 0;
	raystraced = 0;
	IupGLMakeCurrent (Viewer::Instance ()->GetCanvasRenderer ());


	std::ifstream loadfile;
	loadfile.open ("groundtruth.txt");
	if (loadfile.is_open ())
	{
		gt_pixels = new err_type[Viewer::Instance ()->m_CurrentWidth * Viewer::Instance ()->m_CurrentHeight * 4];

		loadfile >> raystraced;
		int wt = Viewer::Instance ()->m_CurrentWidth;
		for (int x = 0; x < Viewer::Instance ()->m_CurrentWidth; x++)
			for (int y = 0; y < Viewer::Instance ()->m_CurrentHeight; y++)
			{
				int index = x * 4 + y*wt * 4;

				loadfile >> gt_pixels[index];
				loadfile >> gt_pixels[index + 1];
				loadfile >> gt_pixels[index + 2];
				loadfile >> gt_pixels[index + 3];
			}
		loadfile.close ();
	}
	else
	{
		gt_pixels = RenderGroundTruth ();
		std::ofstream state_file;
		state_file.open ("groundtruth.txt");

		state_file << raystraced << "\n";
		int wt = Viewer::Instance ()->m_CurrentWidth;
		for (int x = 0; x < Viewer::Instance ()->m_CurrentWidth; x++)
			for (int y = 0; y < Viewer::Instance ()->m_CurrentHeight; y++)
			{
				int index = x * 4 + y*wt * 4;
				state_file << gt_pixels[index] << " " << gt_pixels[index + 1] << " " << 
					gt_pixels[index + 2] << " " << gt_pixels[index + 3] << '\n';
			}
		
		state_file.close ();
	}

	m_viewer.m_voleval.m_current_method = VolumeEvaluator::C_RIEMANN_SUMMATION_RIGHT_HAND;
	//RIEMANN
	m_viewer.m_voleval.m_distance_intervals = 0.1;
	err_type* r01_error = GetErrorFromVolEvalMethod (gt_pixels, "Riemann 0.1", maxerror);
	GenerateExtrapolatedError ("0010_Riemann_01.txt", r01_error, 0.010);
	GenerateExtrapolatedError ("0005_Riemann_01.txt", r01_error, 0.005);
	GenerateExtrapolatedError ("0001_Riemann_01.txt", r01_error, 0.001);
	delete[] r01_error;

	m_viewer.m_voleval.m_distance_intervals = 0.2;
	err_type* r02_error = GetErrorFromVolEvalMethod (gt_pixels, "Riemann 0.2", maxerror);
	GenerateExtrapolatedError ("0010_Riemann_02.txt", r02_error, 0.010);
	GenerateExtrapolatedError ("0005_Riemann_02.txt", r02_error, 0.005);
	GenerateExtrapolatedError ("0001_Riemann_02.txt", r02_error, 0.001);
	delete[] r02_error;

	m_viewer.m_voleval.m_distance_intervals = 0.3;
	err_type* r03_error = GetErrorFromVolEvalMethod (gt_pixels, "Riemann 0.3", maxerror);
	GenerateExtrapolatedError ("0010_Riemann_03.txt", r03_error, 0.010);
	GenerateExtrapolatedError ("0005_Riemann_03.txt", r03_error, 0.005);
	GenerateExtrapolatedError ("0001_Riemann_03.txt", r03_error, 0.001);
	delete[] r03_error;

	m_viewer.m_voleval.m_distance_intervals = 0.4;
	err_type* r04_error = GetErrorFromVolEvalMethod (gt_pixels, "Riemann 0.4", maxerror);
	GenerateExtrapolatedError ("0010_Riemann_04.txt", r04_error, 0.010);
	GenerateExtrapolatedError ("0005_Riemann_04.txt", r04_error, 0.005);
	GenerateExtrapolatedError ("0001_Riemann_04.txt", r04_error, 0.001);
	delete[] r04_error;

	m_viewer.m_voleval.m_distance_intervals = 0.5;
	err_type* r05_error = GetErrorFromVolEvalMethod (gt_pixels, "Riemann 0.5", maxerror);
	GenerateExtrapolatedError ("0010_Riemann_05.txt", r05_error, 0.010);
	GenerateExtrapolatedError ("0005_Riemann_05.txt", r05_error, 0.005);
	GenerateExtrapolatedError ("0001_Riemann_05.txt", r05_error, 0.001);
	delete[] r05_error;

	////////////////
	// [0.05, 2.0] //
	////////////////
	{
		printf ("[0.05, 2.0]\n");
		m_viewer.m_voleval.m_min_h_step = 0.05;
		m_viewer.m_voleval.m_max_h_step = 2.0;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_COUPLED;
		SetIntegralError (0.01);
		err_type* opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.010", maxerror);
		GenerateExtrapolatedError ("0010_OptimizedAdaptive_005_20.txt", opt_0010, 0.010);
		delete[] opt_0010;

		SetIntegralError (0.005);
		err_type* opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.005", maxerror);
		GenerateExtrapolatedError ("0005_OptimizedAdaptive_005_20.txt", opt_0005, 0.005);
		delete[] opt_0005;

		SetIntegralError (0.001);
		err_type* opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.001", maxerror);
		GenerateExtrapolatedError ("0001_OptimizedAdaptive_005_20.txt", opt_0001, 0.001);
		delete[] opt_0001;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_DECOUPLED;
		SetIntegralError (0.01);
		err_type* sep_opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.010", maxerror);
		GenerateExtrapolatedError ("0010_SeparatedAdaptiveOptmized_005_20.txt", sep_opt_0010, 0.010);
		delete[] sep_opt_0010;

		SetIntegralError (0.005);
		err_type* sep_opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.005", maxerror);
		GenerateExtrapolatedError ("0005_SeparatedAdaptiveOptmized_005_20.txt", sep_opt_0005, 0.005);
		delete[] sep_opt_0005;

		SetIntegralError (0.001);
		err_type* sep_opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.001", maxerror);
		GenerateExtrapolatedError ("0001_SeparatedAdaptiveOptmized_005_20.txt", sep_opt_0001, 0.001);
		delete[] sep_opt_0001;
	}
	////////////////
	// [0.05, 2.0] //
	////////////////

	////////////////
	// [0.1, 2.0] //
	////////////////
	{
		printf ("[0.1, 2.0]\n");
		m_viewer.m_voleval.m_min_h_step = 0.1;
		m_viewer.m_voleval.m_max_h_step = 2.0;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_COUPLED;
		SetIntegralError (0.01);
		err_type* opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.010", maxerror);
		GenerateExtrapolatedError ("0010_OptimizedAdaptive_01_20.txt", opt_0010, 0.010);
		delete[] opt_0010;

		SetIntegralError (0.005);
		err_type* opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.005", maxerror);
		GenerateExtrapolatedError ("0005_OptimizedAdaptive_01_20.txt", opt_0005, 0.005);
		delete[] opt_0005;

		SetIntegralError (0.001);
		err_type* opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.001", maxerror);
		GenerateExtrapolatedError ("0001_OptimizedAdaptive_01_20.txt", opt_0001, 0.001);
		delete[] opt_0001;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_DECOUPLED;
		SetIntegralError (0.01);
		err_type* sep_opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.010", maxerror);
		GenerateExtrapolatedError ("0010_SeparatedAdaptiveOptmized_01_20.txt", sep_opt_0010, 0.010);
		delete[] sep_opt_0010;

		SetIntegralError (0.005);
		err_type* sep_opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.005", maxerror);
		GenerateExtrapolatedError ("0005_SeparatedAdaptiveOptmized_01_20.txt", sep_opt_0005, 0.005);
		delete[] sep_opt_0005;

		SetIntegralError (0.001);
		err_type* sep_opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.001", maxerror);
		GenerateExtrapolatedError ("0001_SeparatedAdaptiveOptmized_01_20.txt", sep_opt_0001, 0.001);
		delete[] sep_opt_0001;
	}
	////////////////
	// [0.1, 2.0] //
	////////////////


	////////////////
	// [0.1, 4.0] //
	////////////////
	{
		printf ("[0.1, 4.0]\n");
		m_viewer.m_voleval.m_min_h_step = 0.1;
		m_viewer.m_voleval.m_max_h_step = 4.0;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_COUPLED;
		SetIntegralError (0.01);
		err_type* opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.010", maxerror);
		GenerateExtrapolatedError ("0010_OptimizedAdaptive_01_40.txt", opt_0010, 0.010);
		delete[] opt_0010;

		SetIntegralError (0.005);
		err_type* opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.005", maxerror);
		GenerateExtrapolatedError ("0005_OptimizedAdaptive_01_40.txt", opt_0005, 0.005);
		delete[] opt_0005;

		SetIntegralError (0.001);
		err_type* opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.001", maxerror);
		GenerateExtrapolatedError ("0001_OptimizedAdaptive_01_40.txt", opt_0001, 0.001);
		delete[] opt_0001;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_DECOUPLED;
		SetIntegralError (0.01);
		err_type* sep_opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.010", maxerror);
		GenerateExtrapolatedError ("0010_SeparatedAdaptiveOptmized_01_40.txt", sep_opt_0010, 0.010);
		delete[] sep_opt_0010;

		SetIntegralError (0.005);
		err_type* sep_opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.005", maxerror);
		GenerateExtrapolatedError ("0005_SeparatedAdaptiveOptmized_01_40.txt", sep_opt_0005, 0.005);
		delete[] sep_opt_0005;

		SetIntegralError (0.001);
		err_type* sep_opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.001", maxerror);
		GenerateExtrapolatedError ("0001_SeparatedAdaptiveOptmized_01_40.txt", sep_opt_0001, 0.001);
		delete[] sep_opt_0001;
	}
	////////////////
	// [0.1, 4.0] //
	////////////////

	////////////////
	// [0.1, 8.0] //
	////////////////
	{
		printf ("[0.1, 8.0]\n");
		m_viewer.m_voleval.m_min_h_step = 0.1;
		m_viewer.m_voleval.m_max_h_step = 8.0;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_COUPLED;
		SetIntegralError (0.01);
		err_type* opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.010", maxerror);
		GenerateExtrapolatedError ("0010_OptimizedAdaptive_01_80.txt", opt_0010, 0.010);
		delete[] opt_0010;

		SetIntegralError (0.005);
		err_type* opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.005", maxerror);
		GenerateExtrapolatedError ("0005_OptimizedAdaptive_01_80.txt", opt_0005, 0.005);
		delete[] opt_0005;

		SetIntegralError (0.001);
		err_type* opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.001", maxerror);
		GenerateExtrapolatedError ("0001_OptimizedAdaptive_01_80.txt", opt_0001, 0.001);
		delete[] opt_0001;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_DECOUPLED;
		SetIntegralError (0.01);
		err_type* sep_opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.010", maxerror);
		GenerateExtrapolatedError ("0010_SeparatedAdaptiveOptmized_01_80.txt", sep_opt_0010, 0.010);
		delete[] sep_opt_0010;

		SetIntegralError (0.005);
		err_type* sep_opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.005", maxerror);
		GenerateExtrapolatedError ("0005_SeparatedAdaptiveOptmized_01_80.txt", sep_opt_0005, 0.005);
		delete[] sep_opt_0005;

		SetIntegralError (0.001);
		err_type* sep_opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.001", maxerror);
		GenerateExtrapolatedError ("0001_SeparatedAdaptiveOptmized_01_80.txt", sep_opt_0001, 0.001);
		delete[] sep_opt_0001;
	}
	////////////////
	// [0.1, 8.0] //
	////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////
	// [0.2, 2.0] //
	////////////////
	{
		printf ("[0.2, 2.0]\n");
		m_viewer.m_voleval.m_min_h_step = 0.2;
		m_viewer.m_voleval.m_max_h_step = 2.0;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_COUPLED;
		SetIntegralError (0.01);
		err_type* opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.010", maxerror);
		GenerateExtrapolatedError ("0010_OptimizedAdaptive_02_20.txt", opt_0010, 0.010);
		delete[] opt_0010;

		SetIntegralError (0.005);
		err_type* opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.005", maxerror);
		GenerateExtrapolatedError ("0005_OptimizedAdaptive_02_20.txt", opt_0005, 0.005);
		delete[] opt_0005;

		SetIntegralError (0.001);
		err_type* opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.001", maxerror);
		GenerateExtrapolatedError ("0001_OptimizedAdaptive_02_20.txt", opt_0001, 0.001);
		delete[] opt_0001;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_DECOUPLED;
		SetIntegralError (0.01);
		err_type* sep_opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.010", maxerror);
		GenerateExtrapolatedError ("0010_SeparatedAdaptiveOptmized_02_20.txt", sep_opt_0010, 0.010);
		delete[] sep_opt_0010;

		SetIntegralError (0.005);
		err_type* sep_opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.005", maxerror);
		GenerateExtrapolatedError ("0005_SeparatedAdaptiveOptmized_02_20.txt", sep_opt_0005, 0.005);
		delete[] sep_opt_0005;

		SetIntegralError (0.001);
		err_type* sep_opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.001", maxerror);
		GenerateExtrapolatedError ("0001_SeparatedAdaptiveOptmized_02_20.txt", sep_opt_0001, 0.001);
		delete[] sep_opt_0001;
	}
	////////////////
	// [0.2, 2.0] //
	////////////////


	////////////////
	// [0.2, 4.0] //
	////////////////
	{
		printf ("[0.2, 4.0]\n");
		m_viewer.m_voleval.m_min_h_step = 0.2;
		m_viewer.m_voleval.m_max_h_step = 4.0;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_COUPLED;
		SetIntegralError (0.01);
		err_type* opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.010", maxerror);
		GenerateExtrapolatedError ("0010_OptimizedAdaptive_02_40.txt", opt_0010, 0.010);
		delete[] opt_0010;

		SetIntegralError (0.005);
		err_type* opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.005", maxerror);
		GenerateExtrapolatedError ("0005_OptimizedAdaptive_02_40.txt", opt_0005, 0.005);
		delete[] opt_0005;

		SetIntegralError (0.001);
		err_type* opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.001", maxerror);
		GenerateExtrapolatedError ("0001_OptimizedAdaptive_02_40.txt", opt_0001, 0.001);
		delete[] opt_0001;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_DECOUPLED;
		SetIntegralError (0.01);
		err_type* sep_opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.010", maxerror);
		GenerateExtrapolatedError ("0010_SeparatedAdaptiveOptmized_02_40.txt", sep_opt_0010, 0.010);
		delete[] sep_opt_0010;

		SetIntegralError (0.005);
		err_type* sep_opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.005", maxerror);
		GenerateExtrapolatedError ("0005_SeparatedAdaptiveOptmized_02_40.txt", sep_opt_0005, 0.005);
		delete[] sep_opt_0005;

		SetIntegralError (0.001);
		err_type* sep_opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.001", maxerror);
		GenerateExtrapolatedError ("0001_SeparatedAdaptiveOptmized_02_40.txt", sep_opt_0001, 0.001);
		delete[] sep_opt_0001;
	}
	////////////////
	// [0.2, 4.0] //
	////////////////

	////////////////
	// [0.2, 8.0] //
	////////////////
	{
		printf ("[0.2, 8.0]\n");
		m_viewer.m_voleval.m_min_h_step = 0.2;
		m_viewer.m_voleval.m_max_h_step = 8.0;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_COUPLED;
		SetIntegralError (0.01);
		err_type* opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.010", maxerror);
		GenerateExtrapolatedError ("0010_OptimizedAdaptive_02_80.txt", opt_0010, 0.010);
		delete[] opt_0010;

		SetIntegralError (0.005);
		err_type* opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.005", maxerror);
		GenerateExtrapolatedError ("0005_OptimizedAdaptive_02_80.txt", opt_0005, 0.005);
		delete[] opt_0005;

		SetIntegralError (0.001);
		err_type* opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.001", maxerror);
		GenerateExtrapolatedError ("0001_OptimizedAdaptive_02_80.txt", opt_0001, 0.001);
		delete[] opt_0001;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_DECOUPLED;
		SetIntegralError (0.01);
		err_type* sep_opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.010", maxerror);
		GenerateExtrapolatedError ("0010_SeparatedAdaptiveOptmized_02_80.txt", sep_opt_0010, 0.010);
		delete[] sep_opt_0010;

		SetIntegralError (0.005);
		err_type* sep_opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.005", maxerror);
		GenerateExtrapolatedError ("0005_SeparatedAdaptiveOptmized_02_80.txt", sep_opt_0005, 0.005);
		delete[] sep_opt_0005;

		SetIntegralError (0.001);
		err_type* sep_opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.001", maxerror);
		GenerateExtrapolatedError ("0001_SeparatedAdaptiveOptmized_02_80.txt", sep_opt_0001, 0.001);
		delete[] sep_opt_0001;
	}
	////////////////
	// [0.2, 8.0] //
	////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	////////////////
	// [0.4, 2.0] //
	////////////////
	{
		printf ("[0.4, 2.0]\n");
		m_viewer.m_voleval.m_min_h_step = 0.4;
		m_viewer.m_voleval.m_max_h_step = 2.0;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_COUPLED;
		SetIntegralError (0.01);
		err_type* opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.010", maxerror);
		GenerateExtrapolatedError ("0010_OptimizedAdaptive_04_20.txt", opt_0010, 0.010);
		delete[] opt_0010;

		SetIntegralError (0.005);
		err_type* opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.005", maxerror);
		GenerateExtrapolatedError ("0005_OptimizedAdaptive_04_20.txt", opt_0005, 0.005);
		delete[] opt_0005;

		SetIntegralError (0.001);
		err_type* opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.001", maxerror);
		GenerateExtrapolatedError ("0001_OptimizedAdaptive_04_20.txt", opt_0001, 0.001);
		delete[] opt_0001;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_DECOUPLED;
		SetIntegralError (0.01);
		err_type* sep_opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.010", maxerror);
		GenerateExtrapolatedError ("0010_SeparatedAdaptiveOptmized_04_20.txt", sep_opt_0010, 0.010);
		delete[] sep_opt_0010;

		SetIntegralError (0.005);
		err_type* sep_opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.005", maxerror);
		GenerateExtrapolatedError ("0005_SeparatedAdaptiveOptmized_04_20.txt", sep_opt_0005, 0.005);
		delete[] sep_opt_0005;

		SetIntegralError (0.001);
		err_type* sep_opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.001", maxerror);
		GenerateExtrapolatedError ("0001_SeparatedAdaptiveOptmized_04_20.txt", sep_opt_0001, 0.001);
		delete[] sep_opt_0001;
	}
	////////////////
	// [0.4, 2.0] //
	////////////////


	////////////////
	// [0.4, 4.0] //
	////////////////
	{
		printf ("[0.4, 4.0]\n");
		m_viewer.m_voleval.m_min_h_step = 0.4;
		m_viewer.m_voleval.m_max_h_step = 4.0;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_COUPLED;
		SetIntegralError (0.01);
		err_type* opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.010", maxerror);
		GenerateExtrapolatedError ("0010_OptimizedAdaptive_04_40.txt", opt_0010, 0.010);
		delete[] opt_0010;

		SetIntegralError (0.005);
		err_type* opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.005", maxerror);
		GenerateExtrapolatedError ("0005_OptimizedAdaptive_04_40.txt", opt_0005, 0.005);
		delete[] opt_0005;

		SetIntegralError (0.001);
		err_type* opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.001", maxerror);
		GenerateExtrapolatedError ("0001_OptimizedAdaptive_04_40.txt", opt_0001, 0.001);
		delete[] opt_0001;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_DECOUPLED;
		SetIntegralError (0.01);
		err_type* sep_opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.010", maxerror);
		GenerateExtrapolatedError ("0010_SeparatedAdaptiveOptmized_04_40.txt", sep_opt_0010, 0.010);
		delete[] sep_opt_0010;

		SetIntegralError (0.005);
		err_type* sep_opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.005", maxerror);
		GenerateExtrapolatedError ("0005_SeparatedAdaptiveOptmized_04_40.txt", sep_opt_0005, 0.005);
		delete[] sep_opt_0005;

		SetIntegralError (0.001);
		err_type* sep_opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.001", maxerror);
		GenerateExtrapolatedError ("0001_SeparatedAdaptiveOptmized_04_40.txt", sep_opt_0001, 0.001);
		delete[] sep_opt_0001;
	}
	////////////////
	// [0.4, 4.0] //
	////////////////

	////////////////
	// [0.4, 8.0] //
	////////////////
	{
		printf ("[0.4, 8.0]\n");
		m_viewer.m_voleval.m_min_h_step = 0.4;
		m_viewer.m_voleval.m_max_h_step = 8.0;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_COUPLED;
		SetIntegralError (0.01);
		err_type* opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.010", maxerror);
		GenerateExtrapolatedError ("0010_OptimizedAdaptive_04_80.txt", opt_0010, 0.010);
		delete[] opt_0010;

		SetIntegralError (0.005);
		err_type* opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.005", maxerror);
		GenerateExtrapolatedError ("0005_OptimizedAdaptive_04_80.txt", opt_0005, 0.005);
		delete[] opt_0005;

		SetIntegralError (0.001);
		err_type* opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate 0.001", maxerror);
		GenerateExtrapolatedError ("0001_OptimizedAdaptive_04_80.txt", opt_0001, 0.001);
		delete[] opt_0001;

		m_viewer.m_voleval.m_current_method = VolumeEvaluator::VOLEVAL_ADAPTIVE_SIMPSON_DECOUPLED;
		SetIntegralError (0.01);
		err_type* sep_opt_0010 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.010", maxerror);
		GenerateExtrapolatedError ("0010_SeparatedAdaptiveOptmized_04_80.txt", sep_opt_0010, 0.010);
		delete[] sep_opt_0010;

		SetIntegralError (0.005);
		err_type* sep_opt_0005 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.005", maxerror);
		GenerateExtrapolatedError ("0005_SeparatedAdaptiveOptmized_04_80.txt", sep_opt_0005, 0.005);
		delete[] sep_opt_0005;

		SetIntegralError (0.001);
		err_type* sep_opt_0001 = GetErrorFromVolEvalMethod (gt_pixels, "Simpson Half Iterate Separated Optmimized 0.001", maxerror);
		GenerateExtrapolatedError ("0001_SeparatedAdaptiveOptmized_04_80.txt", sep_opt_0001, 0.001);
		delete[] sep_opt_0001;
	}
	////////////////
	// [0.4, 8.0] //
	////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	delete[] gt_pixels;
	gt_pixels = NULL;

	std::cout << "DONE" << std::endl;

	m_viewer.m_voleval.m_current_method = curmethod;
	m_viewer.m_voleval.m_distance_intervals = distintervals;
}

void ViewMethodCPU::RiemannComparison ()
{
	double threshold = 1.0 / 512.0;
	std::cout << "Comparação de Riemann" << std::endl;
	int curmethod = m_viewer.m_voleval.m_current_method;
	int distintervals = m_viewer.m_voleval.m_distance_intervals;
	maxerror[0] = 0; maxerror[1] = 0; maxerror[2] = 0; maxerror[3] = 0;
	raystraced = 0;
	IupGLMakeCurrent (Viewer::Instance ()->GetCanvasRenderer ());

	m_viewer.m_voleval.m_current_method = VolumeEvaluator::C_RIEMANN_SUMMATION_RIGHT_HAND;
	//RIEMANN
	m_viewer.m_voleval.m_distance_intervals = 0.5;
	float* r05 = new float[Viewer::Instance ()->m_CurrentWidth * Viewer::Instance ()->m_CurrentHeight * 4];
	m_viewer.Render (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
	int wt = Viewer::Instance ()->m_CurrentWidth;
	for (int x = 0; x < Viewer::Instance ()->m_CurrentWidth; x++) {
		for (int y = 0; y < Viewer::Instance ()->m_CurrentHeight; y++) {
			int index = x * 4 + y*wt * 4;
			r05[index + 0] = m_viewer.m_pixels[index + 0];
			r05[index + 1] = m_viewer.m_pixels[index + 1];
			r05[index + 2] = m_viewer.m_pixels[index + 2];
			r05[index + 3] = m_viewer.m_pixels[index + 3];
		}
	}
	std::cout << "Riemann 0.5000" << std::endl;

	m_viewer.m_voleval.m_distance_intervals = 0.4;
	float* r04 = new float[Viewer::Instance ()->m_CurrentWidth * Viewer::Instance ()->m_CurrentHeight * 4];
	err_type* r04000_error = RenderAndComparison (r05, "Riemann 0.4000", r04);
	GenerateExtrapolatedError ("Riemann_04000.txt", r04000_error, threshold);

	m_viewer.m_voleval.m_distance_intervals = 0.3;
	float* r03 = new float[Viewer::Instance ()->m_CurrentWidth * Viewer::Instance ()->m_CurrentHeight * 4];
	err_type* r03000_error = RenderAndComparison (r04, "Riemann 0.3000", r03);
	GenerateExtrapolatedError ("Riemann_03000.txt", r03000_error, threshold);

	m_viewer.m_voleval.m_distance_intervals = 0.2;
	float* r02 = new float[Viewer::Instance ()->m_CurrentWidth * Viewer::Instance ()->m_CurrentHeight * 4];
	err_type* r02000_error = RenderAndComparison (r03, "Riemann 0.2000", r02);
	GenerateExtrapolatedError ("Riemann_02000.txt", r02000_error, threshold);
	
	m_viewer.m_voleval.m_distance_intervals = 0.1;
	float* r01 = new float[Viewer::Instance ()->m_CurrentWidth * Viewer::Instance ()->m_CurrentHeight * 4];
	err_type* r01000_error = RenderAndComparison (r02, "Riemann 0.1000", r01);
	GenerateExtrapolatedError ("Riemann_01000.txt", r01000_error, threshold);
	
	m_viewer.m_voleval.m_distance_intervals = 0.01;
	float* r001 = new float[Viewer::Instance ()->m_CurrentWidth * Viewer::Instance ()->m_CurrentHeight * 4];
	err_type* r00100_error = RenderAndComparison (r01, "Riemann 0.0100", r001);
	GenerateExtrapolatedError ("Riemann_00100.txt", r00100_error, threshold);

	m_viewer.m_voleval.m_distance_intervals = 0.005;
	float* r0005 = new float[Viewer::Instance ()->m_CurrentWidth * Viewer::Instance ()->m_CurrentHeight * 4];
	err_type* r00050_error = RenderAndComparison (r001, "Riemann 0.0050", r0005);
	GenerateExtrapolatedError ("Riemann_00050.txt", r00050_error, threshold);

	m_viewer.m_voleval.m_distance_intervals = 0.001;
	float* r0001 = new float[Viewer::Instance ()->m_CurrentWidth * Viewer::Instance ()->m_CurrentHeight * 4];
	err_type* r00010_error = RenderAndComparison (r0005, "Riemann 0.0010", r0001);
	GenerateExtrapolatedError ("Riemann_00010.txt", r00010_error, threshold);
	
	m_viewer.m_voleval.m_distance_intervals = 0.0005;
	float* r00005 = new float[Viewer::Instance ()->m_CurrentWidth * Viewer::Instance ()->m_CurrentHeight * 4];
	err_type* r00005_error = RenderAndComparison (r0001, "Riemann 0.0005", r00005);
	GenerateExtrapolatedError ("Riemann_00005.txt", r00005_error, threshold);

	m_viewer.m_voleval.m_distance_intervals = 0.0001;
	float* r00001 = new float[Viewer::Instance ()->m_CurrentWidth * Viewer::Instance ()->m_CurrentHeight * 4];
	err_type* r00001_error = RenderAndComparison (r00005, "Riemann 0.0001", r00001);
	GenerateExtrapolatedError ("Riemann_00001.txt", r00001_error, threshold);

	delete[] r05;
	delete[] r04;
	delete[] r03;
	delete[] r02;
	delete[] r01;
	delete[] r001; 
	delete[] r0005;
	delete[] r0001;
	delete[] r00005;
	delete[] r00001;
	
	delete[] r04000_error;
	delete[] r03000_error;
	delete[] r02000_error;
	delete[] r01000_error;
	delete[] r00100_error;
	delete[] r00050_error;
	delete[] r00010_error;
	delete[] r00005_error;
	delete[] r00001_error;

	m_viewer.m_voleval.m_current_method = curmethod;
	m_viewer.m_voleval.m_distance_intervals = distintervals;
}

err_type* ViewMethodCPU::RenderAndComparison (err_type* GroundTruth, std::string feedback_message, float* pixelcolor)
{
	err_type* method_error = new err_type[Viewer::Instance ()->m_CurrentWidth * Viewer::Instance ()->m_CurrentHeight * 4];

	m_viewer.Render (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);

	int wt = Viewer::Instance ()->m_CurrentWidth;
	for (int x = 0; x < Viewer::Instance ()->m_CurrentWidth; x++) {
		for (int y = 0; y < Viewer::Instance ()->m_CurrentHeight; y++) {
			int index = x * 4 + y*wt * 4;
			pixelcolor[index + 0] = m_viewer.m_pixels[index + 0];
			pixelcolor[index + 1] = m_viewer.m_pixels[index + 1];
			pixelcolor[index + 2] = m_viewer.m_pixels[index + 2];
			pixelcolor[index + 3] = m_viewer.m_pixels[index + 3];

			method_error[index + 0] = fabs ((err_type)m_viewer.m_pixels[index + 0] - GroundTruth[index + 0]);
			method_error[index + 1] = fabs ((err_type)m_viewer.m_pixels[index + 1] - GroundTruth[index + 1]);
			method_error[index + 2] = fabs ((err_type)m_viewer.m_pixels[index + 2] - GroundTruth[index + 2]);
			method_error[index + 3] = fabs ((err_type)m_viewer.m_pixels[index + 3] - GroundTruth[index + 3]);
		}
	}
	std::cout << feedback_message << std::endl;

	return method_error;
}