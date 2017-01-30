#include "ViewMethodGLSL2P.h"

ViewMethodGLSL2P::ViewMethodGLSL2P ()
	: GLSLViewMethod(VRVIEWS::GLSL2P)
	, m_scale(1.0f)
	, m_button(-1)
{
	m_volumename = "";
	m_trasnferfunctionname = "";

	m_redisplay = true;
	m_mousepressed = false;
}

ViewMethodGLSL2P::~ViewMethodGLSL2P ()
{}

int ViewMethodGLSL2P::Idle_Action_CB (Ihandle* cnv_renderer)
{
	IupGLMakeCurrent (cnv_renderer);
	if (m_redisplay || m_auto_redisplay)
	{
		if (m_outdated)
		{
			Viewer::Instance()->UpdateATFG();
			if (!m_renderer.m_glsl_volume || Viewer::Instance()->m_volumename.compare(m_volumename) != 0)
			{
				m_volumename = Viewer::Instance ()->m_volumename;
				m_renderer.ReloadVolume (Viewer::Instance ()->m_atfg);
			}
			if (!m_renderer.m_glsl_transfer_function || Viewer::Instance ()->m_tf_name.compare (m_trasnferfunctionname) != 0)
			{
				m_trasnferfunctionname = Viewer::Instance ()->m_tf_name;
				m_renderer.ReloadTransferFunction (Viewer::Instance ()->m_transfer_function);
			}

			UpdateIupUserInterface ();
			m_outdated = false;
		}

		if (m_renderer.m_glsl_volume && m_renderer.m_glsl_transfer_function)
		{
			m_renderer.Render (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
			IupGLSwapBuffers (cnv_renderer);
		}
		m_redisplay = false;
	}
	return IUP_DEFAULT;
}

void ViewMethodGLSL2P::ReloadTransferFunction()
{
	m_renderer.ReloadTransferFunction(Viewer::Instance()->m_transfer_function);
	UpdateIupUserInterface();
}

int ViewMethodGLSL2P::Keyboard_CB (Ihandle *ih, int c, int press)
{
	if (press == 0)
	{
		switch (c)
		{
			case K_P:
			case K_p:
				Viewer::Instance ()->SaveSnapshot ("glsl2p_screenshot.png");
				return IUP_DEFAULT;
				break;
			case K_D:
			case K_d:
				{
					std::cout << "Set Step Distance: ";
					float stepdist;
					std::cin >> stepdist;
					m_renderer.SetStepDistance (stepdist);
				}
				break;
			case K_R:
			case K_r:
				m_redisplay = true;
				break;
			case K_T:
			case K_t:
				m_renderer.ReloadShaders ();
				break;
			case K_M:
			case K_m:
				m_renderer.SetYRotation (0.0f);
				m_renderer.SetXRotation (0.0f);
				m_renderer.SetYTranslation (0.0f);
				m_renderer.SetXTranslation (0.0f);
				m_renderer.SetScale (1.0f);
				m_redisplay = true;
				break;
			case K_SP:
				m_renderer.m_iterate = !m_renderer.m_iterate;
				m_auto_redisplay = m_renderer.m_iterate;
				m_gui.SetIterateToggle (m_renderer.m_iterate);
				break;
			case K_Z:
			case K_z:
				m_auto_redisplay = !m_auto_redisplay;
				break;
			case K_ESC:
				exit (EXIT_SUCCESS);
				break;
			default:
				break;
		}
	}
	return IUP_DEFAULT;
}

int ViewMethodGLSL2P::Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status)
{
	m_mousepressed = pressed == 1 ? true : false;
	if (m_mousepressed)
	{
		m_motion_x = x;
		m_motion_y = y;
		m_button = button;
	}
	return IUP_DEFAULT;
}

int ViewMethodGLSL2P::Motion_CB (Ihandle *ih, int x, int y, char *status)
{
	if (m_mousepressed)
	{
		int ydiff = m_motion_y - y;
		int xdiff = m_motion_x - x;
		if ( m_button == 49 )
		{
			if ( ydiff != 0 )//&& abs(ydiff) > abs(xdiff) )
			{
				if ( m_renderer.GetYRotation() >= 135 && m_renderer.GetYRotation() <= 225 )
					ydiff = -ydiff;
				m_renderer.SetXRotation(m_renderer.GetXRotation() + ydiff);
				m_motion_y = y;
				m_redisplay = true;
			}
			if ( xdiff != 0)
			{
				m_renderer.SetYRotation(m_renderer.GetYRotation() - xdiff);
				m_motion_x = x;
				m_redisplay = true;
			}
		}
		else if ( m_button == 50 )
		{
			if ( ydiff != 0 && abs(ydiff) > abs(xdiff) )
			{
				m_renderer.SetYTranslation(m_renderer.GetYTranslation() + ydiff / (float) m_renderer.GetScreenHeight());
				m_motion_y = y;
				m_redisplay = true;
			}
			else if ( xdiff != 0 )
			{
				m_renderer.SetXTranslation(m_renderer.GetXTranslation() - xdiff / (float) m_renderer.GetScreenWidth());
				m_motion_x = x;
				m_redisplay = true;
			}
		}
	}
	return IUP_DEFAULT;
}

int ViewMethodGLSL2P::Wheel_CB(Ihandle *ih, float delta, int x, int y, char *status)
{
	if ( delta < 0 )
		m_scale -= 0.1f;
	else
		m_scale += 0.1f;
	m_renderer.SetScale(m_scale);
	return IUP_DEFAULT;
}

int ViewMethodGLSL2P::Resize_CB (Ihandle *ih, int width, int height)
{
	m_renderer.Resize (width, height);
	MarkOutdated ();
	return IUP_DEFAULT;
}

void ViewMethodGLSL2P::CreateIupUserInterface ()
{
	m_gui.CreateIupInterface ();
}

void ViewMethodGLSL2P::UpdateIupUserInterface ()
{
	m_gui.SetIterateToggle (m_renderer.m_iterate);

	m_gui.SetInitXSlice (m_renderer.GetInitXSlice());
	m_gui.SetLastXSlice (m_renderer.GetLastXSlice ());
	m_gui.SetInitYSlice (m_renderer.GetInitYSlice ());
	m_gui.SetLastYSlice (m_renderer.GetLastYSlice ());
	m_gui.SetInitZSlice (m_renderer.GetInitZSlice ());
	m_gui.SetLastZSlice (m_renderer.GetLastZSlice ());

}

void ViewMethodGLSL2P::ResetCamera ()
{
	m_renderer.SetYRotation (0.0f);
	m_renderer.SetXRotation (0.0f);
	m_redisplay = true;
}

void ViewMethodGLSL2P::SaveCameraState (std::string filename)
{
	m_renderer.SaveInitCameraState (filename);
}

void ViewMethodGLSL2P::LoadCameraState (std::string filename)
{
	m_renderer.LoadInitCameraState (filename);
}

void ViewMethodGLSL2P::BuildViewer ()
{
	if (!m_built)
	{
		m_renderer.CreateScene (Viewer::Instance ()->m_CurrentWidth,
			Viewer::Instance ()->m_CurrentHeight,
			Viewer::Instance ()->m_volume);
		MarkOutdated ();
		
		printf ("ViewMethodGLSL2P Built\n");
		m_built = true;
	}
}

void ViewMethodGLSL2P::CleanViewer ()
{
	if (m_built)
	{
		m_renderer.Destroy ();

		printf ("ViewMethodGLSL2P Cleaned\n");
		m_built = false;
	}
}

void ViewMethodGLSL2P::SetIterate (bool iter)
{
	m_renderer.m_iterate = iter;
}

bool ViewMethodGLSL2P::GetIterate ()
{
	return m_renderer.m_iterate;
}

void ViewMethodGLSL2P::SetAutoRedisplay (bool autored)
{
	m_auto_redisplay = autored;
}

void ViewMethodGLSL2P::SetRedisplay (bool redisp)
{
	m_redisplay = redisp;
}

void ViewMethodGLSL2P::ResetShaders (std::string shadername)
{
	m_renderer.ResetShaders (shadername);
	
	m_renderer.Destroy ();

	m_renderer.CreateScene (
		Viewer::Instance ()->m_CurrentWidth,
		Viewer::Instance ()->m_CurrentHeight,
		Viewer::Instance ()->m_volume,
		false);
	MarkOutdated ();
}

void ViewMethodGLSL2P::ApplyInputSliceSizes ()
{
	m_renderer.ReloadVolume (Viewer::Instance()->m_atfg, false);
}

void ViewMethodGLSL2P::SetSliceSizes (int ix, int lx, int iy, int ly, int iz, int lz)
{
	m_renderer.SetSliceSizes (ix, lx, iy, ly, iz, lz);
}

int ViewMethodGLSL2P::GetInitXSlice ()
{
	return m_renderer.GetInitXSlice ();
}

int ViewMethodGLSL2P::GetLastXSlice ()
{
	return m_renderer.GetLastXSlice ();
}

int ViewMethodGLSL2P::GetInitYSlice ()
{
	return m_renderer.GetInitYSlice ();
}

int ViewMethodGLSL2P::GetLastYSlice ()
{
	return m_renderer.GetLastYSlice ();
}

int ViewMethodGLSL2P::GetInitZSlice ()
{
	return m_renderer.GetInitZSlice ();
}

int ViewMethodGLSL2P::GetLastZSlice ()
{
	return m_renderer.GetLastZSlice ();
}

void ViewMethodGLSL2P::SetInitXSlice (int value)
{
	m_renderer.SetInitXSlice (value);
	m_gui.SetInitXSlice (m_renderer.GetInitXSlice());
}

void ViewMethodGLSL2P::SetLastXSlice (int value)
{
	m_renderer.SetLastXSlice (value);
	m_gui.SetLastXSlice (m_renderer.GetLastXSlice ());
}

void ViewMethodGLSL2P::SetInitYSlice (int value)
{
	m_renderer.SetInitYSlice (value);
	m_gui.SetInitYSlice (m_renderer.GetInitYSlice ());
}

void ViewMethodGLSL2P::SetLastYSlice (int value)
{
	m_renderer.SetLastYSlice (value);
	m_gui.SetLastYSlice (m_renderer.GetLastYSlice ());
}

void ViewMethodGLSL2P::SetInitZSlice (int value)
{
	m_renderer.SetInitZSlice (value);
	m_gui.SetInitZSlice (m_renderer.GetInitZSlice ());
}

void ViewMethodGLSL2P::SetLastZSlice (int value)
{
	m_renderer.SetLastZSlice (value);
	m_gui.SetLastZSlice (m_renderer.GetLastZSlice ());
}

void ViewMethodGLSL2P::AutoModeling ()
{
	m_renderer.AutoModeling (Viewer::Instance()->m_volume);
	m_renderer.ResetGeometry ();
	m_renderer.Resize (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight);
}

void ViewMethodGLSL2P::ApplyModeling (float xw, float yh, float zd)
{
	m_renderer.ApplyModeling (xw, yh, zd);
}

void ViewMethodGLSL2P::SetCubeWidth (float w)
{
	m_renderer.SetCubeWidth (w);
}

void ViewMethodGLSL2P::SetCubeHeight (float h)
{
	m_renderer.SetCubeHeight (h);
}

void ViewMethodGLSL2P::SetCubeDepth (float z)
{
	m_renderer.SetCubeDepth (z);
}

float ViewMethodGLSL2P::GetCubeWidth ()
{
	return m_renderer.GetCubeWidth();
}

float ViewMethodGLSL2P::GetCubeHeight ()
{
	return m_renderer.GetCubeHeight ();
}

float ViewMethodGLSL2P::GetCubeDepth ()
{
	return m_renderer.GetCubeDepth ();
}

void ViewMethodGLSL2P::SetGUICubeWidth (float w)
{
	m_gui.m_gui_cubewidth = w;
	m_gui.UpdateCubeSizes ();
}

void ViewMethodGLSL2P::SetGUICubeHeight (float h)
{
	m_gui.m_gui_cubeheight = h;
	m_gui.UpdateCubeSizes ();
}

void ViewMethodGLSL2P::SetGUICubeDepth (float z)
{
	m_gui.m_gui_cubedepth = z;
	m_gui.UpdateCubeSizes ();
}

float ViewMethodGLSL2P::GetGUICubeWidth ()
{
	return m_gui.m_gui_cubewidth;
}

float ViewMethodGLSL2P::GetGUICubeHeight ()
{
	return m_gui.m_gui_cubeheight;
}

float ViewMethodGLSL2P::GetGUICubeDepth ()
{
	return m_gui.m_gui_cubedepth;
}

void ViewMethodGLSL2P::UseDoublePrecision (bool usedp)
{
	m_renderer.USE_DOUBLE_PRECISION = usedp;


}