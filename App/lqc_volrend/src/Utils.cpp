#include <volrend/Utils.h>
#include <math/MUtils.h>
#include <iostream>

namespace vr
{
	gl::GLTexture3D* GenerateRTexture (ScalarField* vol,
																		 int init_x,
																		 int init_y,
																		 int init_z,
																		 int last_x,
																		 int last_y,
																		 int last_z)
	{
		if (!vol && !vol->Validate()) return NULL;

		int size_x = abs(last_x - init_x);
		int size_y = abs(last_y - init_y);
		int size_z = abs(last_z - init_z);
		printf("size_x: %d, size_y: %d, size_z: %d\n", size_x, size_y, size_z);
		
		int size = size_x*size_y*size_z;
		GLfloat* scalar_values = new GLfloat[size];

		int max;
		UINT32 x, y, z;
		for (UINT32 k = 0; k < size_z; k++)
			for (UINT32 j = 0; j < size_y; j++)
				for (UINT32 i = 0; i < size_x; i++)
				{
					x = i + init_x;
					y = j + init_y;
					z = k + init_z;
					int id = vol->GetId(x, y, z);
					scalar_values[id] = ((vol->GetValue(x, y, z) - vol->GetMinValue()) / (vol->GetMaxValue() - vol->GetMinValue()));
					assert(scalar_values[id] >= 0.0f && scalar_values[id] <= 1.0f);
				}

		gl::GLTexture3D* tex3d_r = new gl::GLTexture3D(size_x, size_y, size_z);
		tex3d_r->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
		tex3d_r->SetData(scalar_values, GL_RED, GL_RED, GL_FLOAT);
		gl::ExitOnGLError("ERROR: After SetData");

		delete[] scalar_values;

		return tex3d_r;
	}

	gl::GLTexture3D* GenerateRGTexture(ATFGenerator* atfg,
		int init_x,
		int init_y,
		int init_z,
		int last_x,
		int last_y,
		int last_z)
	{
		if (!atfg)
			return NULL;

		vr::ScalarField* vol = atfg->GetScalarField();
		if (!vol && !vol->Validate()) return NULL;

		int size_x = abs(last_x - init_x);
		int size_y = abs(last_y - init_y);
		int size_z = abs(last_z - init_z);
		printf("size_x: %d, size_y: %d, size_z: %d\n", size_x, size_y, size_z);

		int size = size_x*size_y*size_z;
		GLfloat* scalar_values = new GLfloat[size * 2];

		int max;
		UINT32 x, y, z;
		int c = 0;
		for (UINT32 k = 0; k < size_z; k++)
			for (UINT32 j = 0; j < size_y; j++)
				for (UINT32 i = 0; i < size_x; i++) {
					x = i + init_x;
					y = j + init_y;
					z = k + init_z;
					scalar_values[c++] = ((vol->GetValue(x, y, z) - vol->GetMinValue()) / (vol->GetMaxValue() - vol->GetMinValue()));
					scalar_values[c++] = ((atfg->GetGradient(x, y, z) - vol->GetMinGradient()) / (vol->GetMaxGradient() - vol->GetMinGradient()));
				}

		gl::GLTexture3D* tex3d_r = new gl::GLTexture3D(size_x, size_y, size_z);
		tex3d_r->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
		tex3d_r->SetData(scalar_values, GL_RG32F, GL_RG, GL_FLOAT);
		gl::ExitOnGLError("ERROR: After SetData");

		delete[] scalar_values;

		return tex3d_r;
	}

	gl::GLTexture3D* GenerateR8UITexture(ScalarField* vol,
		int init_x,
		int init_y,
		int init_z,
		int last_x,
		int last_y,
		int last_z)
	{
		if (!vol && !vol->Validate()) return NULL;

		int size_x = abs(last_x - init_x);
		int size_y = abs(last_y - init_y);
		int size_z = abs(last_z - init_z);
		printf("size_x: %d, size_y: %d, size_z: %d\n", size_x, size_y, size_z);

		int size = size_x*size_y*size_z;
		GLfloat* active_values = new GLfloat[size];

		int max;
		UINT32 x, y, z;
		for (UINT32 k = 0; k < size_z; k++)
			for (UINT32 j = 0; j < size_y; j++)
				for (UINT32 i = 0; i < size_x; i++) {
					x = i + init_x;
					y = j + init_y;
					z = k + init_z;
					int id = vol->GetId(x, y, z);
					active_values[id] = vol->IsActive(x, y, z) ? 1 : 0;
				}

		gl::GLTexture3D* tex3d_r = new gl::GLTexture3D(size_x, size_y, size_z);
		tex3d_r->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
		tex3d_r->SetData(active_values, GL_RED, GL_RED, GL_FLOAT);
		gl::ExitOnGLError("ERROR: After SetData");

		delete[] active_values;

		return tex3d_r;
	}
}