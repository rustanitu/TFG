/*!
\file GLSLRenderer.h
\brief GLSL renderer class.
\author Leonardo Quatrin Campagnolo
*/

#ifndef GLSL2P_RENDERER_H
#define GLSL2P_RENDERER_H

#include <GL/glew.h>
#include <ctime>

#include <glutils/GLTexture3D.h>
#include <volrend/ScalarField.h>
#include <volrend/TransferFunction.h>

#include <math/Matrix4.h>
#include <math/MUtils.h>

#include <glutils/GLShader.h>
#include <glutils/GLCamera.h>
#include <glutils/GLArrayObject.h>
#include <glutils/GLBufferObject.h>

#include <iup.h>
#include <iupgl.h>

class GBuffer;

class RendererGLSL2P
{
public:
	bool USE_DOUBLE_PRECISION;
public:
	/*! Constructor*/
	RendererGLSL2P ();
	/*! Destructor*/
	~RendererGLSL2P ();

	/*! Initialize the Renderer objects.*/
	void CreateScene (int CurrentWidth, int CurrentHeight, vr::ScalarField* volume, bool resetslices = true);
 
	/*! Render a frame of the scene.*/
	bool Render (int Width, int Height);
	
	void Resize (int Width, int Height);

	/*! Destroy all the objects of Renderer.*/
	void Destroy ();

	void BindShaderUniforms ();
	void ResetGeometry ();

	/*! Generate the textures from volume.
	\param volume ScalarField pointer used to generate the 3D textures.
	*/
	void ReloadVolume (vr::ScalarField* volume, bool resetslicesizes = true);
	/*! Generate the texture from transfer function.
	\param tfunction Transfer Function pointer used to generate the 1D texture.
	*/
	void ReloadTransferFunction (vr::TransferFunction* tfunction);
	void ReloadVisibleSet(int set);

	void SetXRotation (float radius);
	float GetXRotation ();
	void SetYRotation (float radius);
	float GetYRotation ();

	void ReloadShaders ();
	void ResetShaders (std::string shadername);

	int GetInitXSlice ();
	int GetLastXSlice ();
	int GetInitYSlice ();
	int GetLastYSlice ();
	int GetInitZSlice ();
	int GetLastZSlice ();
	
	void SetSliceSizes (int ix, int lx, int iy, int ly, int iz, int lz);

	void SetInitXSlice (int value);
	void SetLastXSlice (int value);
	void SetInitYSlice (int value);
	void SetLastYSlice (int value);
	void SetInitZSlice (int value);
	void SetLastZSlice (int value);

	void AutoModeling (vr::ScalarField* volume);
	void ApplyModeling (float xw, float yh, float zd);

	void SetCubeWidth (float w);
	void SetCubeHeight (float h);
	void SetCubeDepth (float z);
	float GetCubeWidth ();
	float GetCubeHeight ();
	float GetCubeDepth ();

	void SaveInitCameraState (std::string filename);
	void LoadInitCameraState (std::string filename);

	void SetStepDistance (float step_distance);

	/*! automatic rotation at y axis of volume.*/
	bool m_iterate;
	
	/*! Store the texture of the current volume.*/
	gl::GLTexture3D* m_glsl_volume;
	/*! Store the texture of the current transfer function.*/
	gl::GLTexture1D* m_glsl_transfer_function;

	gl::GLTexture3D* m_glsl_activetex;

private:
	/*! First pass of Rendering algorithm.
	\note The volume rendering algorithm implemented is based on two pass volume rendering
	*/
	void FirstPass ();
	/*! Second pass of Rendering algorithm.
	\note The volume rendering algorithm implemented is based on two pass volume rendering
	*/
	void SecondPass ();

	/*! Just create a cube and the shader used with it.*/
	void CreateFirstPass ();
	void DestroyCube ();
		
	/*! Just create a quad and the shader used with it.*/
	void DestroyQuad ();
	void CreateSecondPass ();

	/*! Just draw a cube and store the front faces and back faces in the FrameBuffer.*/
	void DrawCube (void);

	/*! Struct to store the position and color of vertices to bind in a vertex buffer.
	\note used in CreateCube() function*/
	typedef struct Vertex
	{
		float Position[3];
		float Color[3];
	} Vertex;

	/*! first pass vao.*/
	gl::GLArrayObject* m_fvao;
	/*! first pass vbo.*/
	gl::GLBufferObject* m_fvbo;
	/*! first pass ibo.*/
	gl::GLBufferObject* m_fibo;

	/*! second pass vao.*/
	gl::GLArrayObject* m_svao;
	/*! second pass vbo.*/
	gl::GLBufferObject* m_svbo;
	/*! second pass ibo.*/
	gl::GLBufferObject* m_sibo;

	/*! shader variables for the first pass.*/
	gl::GLShader* m_shader_firstpass;
	/*! shader variables for the second pass.*/
	gl::GLShader* m_shader_secondpass;

	/*! Store the framebuffer with specific color attachments.*/
	GBuffer *m_glfbo;

	/*! volume rotation at x axis.*/
	float m_x_rotation;
	/*! volume rotation at y axis.*/
	float m_y_rotation;

	unsigned m_FrameCount;
	clock_t m_LastTime;

	lqc::Matrix4f m_ProjectionMatrix, m_ViewMatrix, m_ModelMatrix;

	float m_cube_width;
	float m_cube_height;
	float m_cube_depth;

	float m_tex_width;
	float m_tex_height;
	float m_tex_depth;

	int m_init_slice_x;
	int m_last_slice_x;
	int m_init_slice_y;
	int m_last_slice_y;
	int m_init_slice_z;
	int m_last_slice_z;

	int m_sdr_width;
	int m_sdr_height;
};


#endif