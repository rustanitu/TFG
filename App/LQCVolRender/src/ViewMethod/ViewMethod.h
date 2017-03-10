/*!
\file ViewMethod.h
\brief Template class for view methods.
\author Leonardo Quatrin Campagnolo
*/

#ifndef TEMPLATE_VIEWMETHOD_H
#define TEMPLATE_VIEWMETHOD_H

#include <iup.h>
#include <iupgl.h>
#include <string>

/*! enum with the ViewMethod types available
\ingroup views
*/
enum VRVIEWS
{
	GLSL2P,
	IAS,
	EQUIDISTANT_GLSL,
	ADAPTIVE_GLSL,
	CPU,
	ERN2D,
	ERN,
	_LastToIter,
};

/*!
\ingroup views
*/

class ViewMethod
{
public:
	/*! Constructor.
	\param type VRVIEWS type of the ViewMethod.
	*/
	ViewMethod (VRVIEWS type) { m_type = type; m_outdated = true; m_built = false; }

	/*! Destructor.*/
	~ViewMethod () {}

	virtual char* GetViewMethodName () { return "ViewMethod"; }

	/*! Action callback of the view method. Called when drawing canvas.
	\param cnv_renderer the iup canvas handle.
	\return return of callback (generally IUP_DEFAULT).
	*/
	virtual int Idle_Action_CB (Ihandle* cnv_renderer) = 0;
	
	/*! Keyboard callback of the view method. Called when a keyboard key is down or up pressed.
	\param ih iup handle.
	\param c the key being pressed.
	\param press if the key is down(0) or up(1)
	\return return of callback (generally IUP_DEFAULT).
	*/
	virtual int Keyboard_CB (Ihandle *ih, int c, int press) = 0;

	/*! Mouse Button callback of the view method. Called when a mouse button is down or up pressed.
	\param ih identifies the elemente that activated the event (iup documentation).
	\param button identifies the activated mouse button (iup documentation).
	\param pressed boolean that indicates the state of the button: down(0) or up(1) (iup documentation).
	\param x X position in the canvas where the event has occurred, in pixels (iup documentation).
	\param y Y position in the canvas where the event has occurred, in pixels (iup documentation).
	\param status status of the mouse buttons and some keyboard keys at the moment the event is generated, based on macros (iup documentation).
	\return return of callback (generally IUP_DEFAULT).
	*/
	virtual int Button_CB (Ihandle* ih, int button, int pressed, int x, int y, char* status) = 0;

	/*! Generated when the mouse moves.
	\param ih identifies the elemente that activated the event (iup documentation).
	\param x X position in the canvas where the event has occurred, in pixels (iup documentation).
	\param y Y position in the canvas where the event has occurred, in pixels (iup documentation).
	\param status status of the mouse buttons and some keyboard keys at the moment the event is generated, based on macros (iup documentation).
	\return return of callback (generally IUP_DEFAULT).
	*/
	virtual int Motion_CB (Ihandle *ih, int x, int y, char *status) = 0;

	virtual int Wheel_CB(Ihandle *ih, float delta, int x, int y, char *status) { return IUP_DEFAULT; }

	/*! Generated when the canvas is resized.
	\param ih identifies the elemente that activated the event (iup documentation).
	\param width width param.
	\param height height param.
	\return return of callback (generally IUP_DEFAULT).
	*/
	virtual int Resize_CB (Ihandle *ih, int width, int height) { return IUP_DEFAULT; }

	virtual void SaveCameraState (std::string filename) {}
	virtual void LoadCameraState (std::string filename) {}

	/*! Set the type of this ViewMethod (based on VRVIEWS enumerator)
	\param type a VRVIEWS type
	*/
	void SetType (VRVIEWS type) { m_type = type; }

	/*! Get the type of this ViewMethod (based on VRVIEWS enumerator)
	\return the type of the ViewMethod
	*/
	VRVIEWS GetType () { return m_type; }

	/*! Just mark if the ViewMethod needs an update check of what volume and transfer function is being rendered.*/
	void MarkOutdated () { m_outdated = true; }
	
	/*! Set if the ViewMethod need to be redisplayed or not.
	\param redisp boolean to redisplay value.
	*/
	virtual void SetRedisplay (bool redisp) {}

	/*! Create all iup interface.*/
	virtual void CreateIupUserInterface () {}
	/*! Update all iup interface.*/
	virtual void UpdateIupUserInterface () {}
	
	/*! Get the gui of the ViewMethod.
	\return the vbox identifier element with the gui elements.
	*/
	Ihandle* GetIupUserInterface () { return m_iup_vbox_allinterface; }
	
	/*! Set GUI elemente of the ViewMethod.
	\param iupuserinterface the iup gui interface (usually a vbox).
	*/
	void SetIupUserInterface (Ihandle* iupuserinterface) { m_iup_vbox_allinterface = iupuserinterface; }
	
	/*! Active the GUI.*/
	void ActiveInterface () { IupSetAttribute (m_iup_vbox_allinterface, "ACTIVE", "YES"); }
	/*! Disactive the GUI.*/
	void DisactiveInterface () { IupSetAttribute (m_iup_vbox_allinterface, "ACTIVE", "NO"); }

	/*! Reset the Camera of the ViewMethod.*/
	virtual void ResetCamera () {}

	/*! Just Build the renderer of the ViewMethod.*/
	virtual void BuildViewer () { m_built = true; }

	/*! Just Build the renderer of the ViewMethod.*/
	virtual void CleanViewer () { m_built = false; }

	virtual void ReloadTransferFunction(){}

	virtual void CleanVolumeTexture() {}
	virtual void CleanTransferFunctionTexture() {}

protected:
	/*! If the ViewMethod needs a update check for volume and transfer function being rendered.*/
	bool m_outdated;
	bool m_built;
	
	/*! the gui to be added to the common gui.*/
	Ihandle* m_iup_vbox_allinterface;

private:
	/*! the type of the ViewMethod, based on the enum VRVIEWS.*/
	VRVIEWS m_type;
};

#endif