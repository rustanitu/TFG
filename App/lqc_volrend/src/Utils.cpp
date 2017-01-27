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

  gl::GLTexture3D* GenerateRGBATexture(ScalarField* vol, TransferFunction* TF1D)
	{
		if (!TF1D && !vol && !vol->Validate()) return NULL;

		int width  = vol->GetWidth();
		int height = vol->GetHeight();
		int depth  = vol->GetDepth();

		gl::GLTexture3D* tex3d_rgba = new gl::GLTexture3D(width, height, depth);
		tex3d_rgba->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		int set = 1;

		lqc::Vector4f* value_rgba = new lqc::Vector4f[width * height * depth];
		for (int i = 0; i < width*height*depth; i++)
		{
			lqc::Vector4d v = TF1D->Get(vol->GetValue(i));
			value_rgba[i] = lqc::Vector4f(v.x, v.y, v.z, v.w);
		}

		tex3d_rgba->SetData(value_rgba, GL_RGBA, GL_RGBA, GL_FLOAT);

		delete[] value_rgba;

		return tex3d_rgba;
	}

	gl::GLTexture3D* GenerateGradientTexture (ScalarField* vol,
																						int gradient_sample_size,
																						int filter_nxnxn,
																						bool normalized_gradient,
																						int init_x,
																						int init_y,
																						int init_z,
																						int last_x,
																						int last_y,
																						int last_z)
	{
		int width = vol->GetWidth();
		int height = vol->GetHeight();
		int depth = vol->GetDepth();

		//1
		//Generation of gradients
		int n = gradient_sample_size;
		lqc::Vector3f* gradients = new lqc::Vector3f[width * height * depth];
		lqc::Vector3f s1, s2;
		int index = 0;
		for (UINT32 z = 0; z < depth; z++)
		{
			for (UINT32 y = 0; y < height; y++)
			{
				for (UINT32 x = 0; x < width; x++)
				{
					s1.x = vol->GetValue(x - n, y, z);
					s2.x = vol->GetValue(x + n, y, z);
					s1.y = vol->GetValue(x, y - n, z);
					s2.y = vol->GetValue(x, y + n, z);
					s1.z = vol->GetValue(x, y, z - n);
					s2.z = vol->GetValue(x, y, z + n);

					lqc::Vector3f s2s1 = (s2 - s1);

					if (normalized_gradient)
					{
						s2s1 = lqc::Vector3f::Normalize(s2s1);
					}
					else
					{
						s2s1.x = s2s1.x / 2.0f * (float)gradient_sample_size;
						s2s1.y = s2s1.y / 2.0f * (float)gradient_sample_size;
						s2s1.z = s2s1.z / 2.0f * (float)gradient_sample_size;
					}

					gradients[index] = s2s1;

					if (lqc::IsNaN(gradients[index].x))
						gradients[index] = lqc::Vector3f::Zero();

					index++;
				}
			}
		}

		//2
		//Filtering
		n = filter_nxnxn;
		index = 0;
		if (n > 0)
		{
			for (int z = 0; z < depth; z++)
			{
				for (int y = 0; y < height; y++)
				{
					for (int x = 0; x < width; x++)
					{
						int fn = (n - 1) / 2;

						lqc::Vector3f average = lqc::Vector3f::Zero();
						int num = 0;
						for (int k = z - fn; k <= z + fn; k++)
						{
							for (int j = y - fn; j <= y + fn; j++)
							{
								for (int i = x - fn; i <= x + fn; i++)
								{
									if (!vol->IsOutOfBoundary(i, j, k))
									{
										average += gradients[x + (y * width) + (z * width * height)];
										num++;
									}
								}
							}
						}

						average = average / (float)num;
						if (average.x != 0.0f && average.y != 0.0f && average.z != 0.0f)
							average.Normalize();

						gradients[index++] = average;
					}
				}
			}
		}

		//3
		//Set the content of the gradient texture
		if (init_x == -1 && last_x == -1
			&& init_y == -1 && last_y == -1
			&& init_z == -1 && last_z == -1)
		{
			init_x = 0;
			init_y = 0;
			init_z = 0;

			last_x = vol->GetWidth();
			last_y = vol->GetHeight();
			last_z = vol->GetDepth();
		}

		int size_x = abs(last_x - init_x);
		int size_y = abs(last_y - init_y);
		int size_z = abs(last_z - init_z);
		lqc::Vector3f* gradients_values = new lqc::Vector3f[size_x*size_y*size_z];

		for (int k = 0; k < size_z; k++)
			for (int j = 0; j < size_y; j++)
				for (int i = 0; i < size_x; i++)
					gradients_values[i + (j * size_x) + (k * size_x * size_y)] = gradients[(i + init_x) + ((j + init_y) * width) + ((k + init_z) * width * height)];

		//4
		//Creating Texture
		gl::GLTexture3D* tex3d_gradient = new gl::GLTexture3D(size_x, size_y, size_z);
		tex3d_gradient->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		tex3d_gradient->SetData((GLvoid*)gradients_values, GL_RGB32F, GL_RGB, GL_FLOAT);

		delete[] gradients_values;
		delete[] gradients;

		return tex3d_gradient;
	}
}