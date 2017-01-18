#include <volrend/Reader.h>

#include <cstdio>
#include <stdio.h>
#include <fstream>
#include <string>

#include <lqc/File/RAWLoader.h>

#include <volrend/TransferFunction1D.h>
#include <volrend/TransferFunctions_tf.h>
#include <volrend/TransferFunctionGroups.h>
#include <volrend/TransferFunctionGersa.h>
#include "../../ATFGenerator/src/TankAPI/Tank.h"

namespace vr
{
	ScalarField* ReadFromVolMod(std::string filepath)
	{
		ScalarField* ret = NULL;

		int found = filepath.find_last_of('.');
		std::string extension = filepath.substr(found + 1);

		printf("------------Reading Volume Model------------\n");
		if ( extension.compare("vol") == 0 )
			ret = ReadVolFile(filepath);
    else if (extension.compare("den") == 0)
      ret = ReadDenFile(filepath);
		else if ( extension.compare("ele") == 0 || extension.compare("txt") == 0 )
			ret = ReadEleFile(filepath);
		else if ( extension.compare("node") == 0 )
			ret = ReadNodeFile(filepath);
		else if ( extension.compare("raw") == 0 )
			ret = ReadRawFile(filepath);
		else if ( extension.compare("med") == 0 )
			ret = ReadMedFile(filepath);
		else if ( extension.compare("gmdl") == 0 )
		{
			printf(" File: %s\n", filepath.c_str());
			Tank* tank = new Tank();
			if ( tank->Read(filepath.c_str()) )
			{
				ret = tank;
			}
		}
		else
			printf("Extension not found\n");
		printf("--------------------------------------------\n");

		return ret;
	}

	Volume* ReadVolFile(std::string filename)
	{
		Volume* ret = NULL;

		printf("Started  -> Read Volume From .vol File\n");
		printf("  - File .vol Path: %s\n", filename.c_str());

		std::string fileifstream = "";

		fileifstream.append(filename);

		std::ifstream file(fileifstream.c_str());
		if ( file.is_open() )
		{
			std::string folder;
			std::getline(file, folder);
			std::cout << "  - Folder: " << folder << std::endl;

			std::string fname;
			std::getline(file, fname);
			std::cout << "  - File name: " << fname << std::endl;

			size_t size;
			file >> size;
			std::cout << "  - Variable Size: " << size << std::endl;

			int width, height, depth;
			file >> width >> height >> depth;
			std::cout << "  - Volume Size: [" << width << ", " << height << ", " << depth << "]" << std::endl;

			file.close();

			int found = fileifstream.find_last_of('\\');
			std::string filevol = fileifstream.substr(0, found + 1);
			filevol.append(fname);
			std::cout << "  - File Volume Path: " << filevol << std::endl;

			float* scalar_values = NULL;
			if ( folder.compare("volvis_raw") == 0 )
				scalar_values = ReadVolvisRaw(filevol, size, width, height, depth);
			else if ( folder.compare("ern_txt") == 0 )
				scalar_values = ReadErnTXT(filevol, width, height, depth);
			else if ( folder.compare("synthetic_model") == 0 )
				scalar_values = ReadSyntheticModelTXT(filevol, width, height, depth);

			ret = new Volume(width, height, depth, scalar_values);
			ret->SetName(fname);

			printf("lqc: Finished -> Read Volume From .vol File\n");
		}
		else
			printf("lqc: Finished -> Error on opening .vol file\n");

		return ret;
	}

  void swap_buffer(short* buff, int size)
  {
    int c = size / 8;
    int n = size - c * 8;
    char tmp0, tmp1, tmp2, tmp3;
    char *buf = (char *)buff;

    for (; c > 0; c--) {
      tmp0 = buf[0]; buf[0] = buf[1]; buf[1] = tmp0;
      tmp1 = buf[2]; buf[2] = buf[3]; buf[3] = tmp1;
      tmp2 = buf[4]; buf[4] = buf[5]; buf[5] = tmp2;
      tmp3 = buf[6]; buf[6] = buf[7]; buf[7] = tmp3;
      buf += 8;
    }
    for (; n > 0; n -= 2) {
      tmp0 = buf[0]; buf[0] = buf[1]; buf[1] = tmp0;
      buf += 2;
    }
  }

  void swap_buffer(int* buff, int size)
  {
    int c = size / 8;
    int n = size - c * 8;
    char tmp0, tmp1, tmp2, tmp3;
    char *buf = (char *)buff;

    for (; c > 0; c--) {
      tmp0 = buf[0]; buf[0] = buf[3]; buf[3] = tmp0;
      tmp1 = buf[1]; buf[1] = buf[2]; buf[2] = tmp1;
      tmp2 = buf[4]; buf[4] = buf[7]; buf[7] = tmp2;
      tmp3 = buf[5]; buf[5] = buf[6]; buf[6] = tmp3;
      buf += 8;
    }
    for (; n > 0; n -= 4) {
      tmp0 = buf[0]; buf[0] = buf[3]; buf[3] = tmp0;
      tmp1 = buf[1]; buf[1] = buf[2]; buf[2] = tmp1;
      buf += 4;
    }
  }

  Volume* ReadDenFile(std::string filename)
  {
    Volume* ret = NULL;

    printf("Started  -> Read Volume From .den File\n");
    printf("  - File .den Path: %s\n", filename.c_str());

    FILE* file = NULL;
    if (fopen_s(&file, filename.c_str(), "r") == 0) {
      short map_version;
      fread_s(&map_version, sizeof(short), sizeof(short), 1, file);

      bool need_swap = false;
      if (map_version == 256)
        need_swap = true;

      short trash[24];
      fread_s(trash, 24 * sizeof(short), sizeof(short), 24, file);
      if (need_swap)
        swap_buffer(trash, 24 * sizeof(short));

      short dimensions[3];
      fread_s(dimensions, 3 * sizeof(short), sizeof(short), 3, file);
      if (need_swap)
        swap_buffer(dimensions, 3 * sizeof(short));

      short warps;
      fread_s(&warps, sizeof(short), sizeof(short), 1, file);
      if (need_swap)
        swap_buffer(&warps, sizeof(short));

      int size;
      fread_s(&size, sizeof(int), sizeof(int), 1, file);
      if (need_swap)
        swap_buffer(&size, sizeof(int));

      unsigned char* data = new unsigned char[size];
      fread_s(data, size * sizeof(unsigned char), sizeof(unsigned char), size, file);

      fclose(file);

      ret = new Volume(dimensions[0], dimensions[1], dimensions[2], data);
      ret->SetName(filename.c_str());

      printf("lqc: Finished -> Read Volume From .den File\n");
    }
    else
      printf("lqc: Finished -> Error on opening .den file\n");

    return ret;
  }

	float* ReadErnTXT(std::string volfilename, int w, int h, int d)
	{
		float* scalar_values = NULL;
		scalar_values = new float[w*h*d];

		for ( int i = 0; i < w*h*d; i++ )
			scalar_values[i] = 0.0f;

		std::ifstream file(volfilename.c_str());
		if ( file.is_open() )
		{
			double x, y, z = 0, value;
			while ( file >> x >> y >> z >> value )
				scalar_values[(int) x + ((int) y * w) + ((int) z * w * h)] = (float) value;
			file.close();
		}
		return scalar_values;
	}

	float* ReadSyntheticModelTXT(std::string volfilename, int w, int h, int d)
	{
		float* scalar_values = NULL;
		FILE* file = NULL;
		int stderror = fopen_s(&file, volfilename.c_str(), "rb");
		if ( stderror == 0 )
		{
			int size = w*h*d;
			scalar_values = new float[size];

			char trash[100];
			fscanf_s(file, "%s", trash, sizeof(trash));
			fscanf_s(file, "%s", trash, sizeof(trash));
			fscanf_s(file, "%s", trash, sizeof(trash));
			fscanf_s(file, "%s", trash, sizeof(trash));
			fscanf_s(file, "%s", trash, sizeof(trash));
			fscanf_s(file, "%s", trash, sizeof(trash));

			float value;
			for ( int i = 0; i < size; ++i )
			{
				fscanf_s(file, "%f", &value);
				scalar_values[i] = value;
			}
		}
		return scalar_values;
	}

	float* ReadVolvisRaw(std::string volfilename, size_t bytes_per_value, int w, int h, int d)
	{
		float* scalar_values = NULL;
		lqc::IRAWLoader rawLoader = lqc::IRAWLoader(volfilename, bytes_per_value, w*h*d, bytes_per_value);

		//GLushort
		if ( bytes_per_value == sizeof(unsigned short) )
		{
			scalar_values = new float[w*h*d];
			unsigned short *b = new unsigned short[w*h*d];
			memcpy(b, rawLoader.GetData(), sizeof(unsigned short)*w*h*d);

			for ( int i = 0; i < (int) (w*h*d); i++ )
				scalar_values[i] = (float) b[i];
			delete[] b;
		}
		//GLubyte
		else if ( bytes_per_value == sizeof(unsigned char) )
		{
			scalar_values = new float[w*h*d];
			unsigned char *b = new unsigned char[w*h*d];
			memcpy(b, rawLoader.GetData(), sizeof(unsigned char)*w*h*d);

			for ( int i = 0; i < (int) (w*h*d); i++ )
				scalar_values[i] = (float) b[i];
			delete[] b;
		}

		return scalar_values;
	}

	Volume* ReadEleFile(std::string filepath)
	{
		int w = 4740, h = 3540, d = 5;
		Volume* ret = NULL;

		printf("Started  -> Read Volume From .ele File\n");
		printf("  - File .ele Path: %s\n", filepath.c_str());

		std::string fileifstream = "";
		fileifstream.append(filepath);

		std::ifstream file_ele(fileifstream.c_str());
		if ( file_ele.is_open() )
		{
			file_ele >> w >> h;
			float* scalar_values = NULL;
			scalar_values = new float[w*h*d];

			for ( int i = 0; i < w*h*d; i++ )
				scalar_values[i] = 0.0f;

			int x, y, value; int z = 0;
			while ( file_ele >> x >> y >> value )
			{
				int i = (int) (x) +((int) (y) * w) + ((int) z * w * h);
				scalar_values[i] = (float) value;
			}

			ret = new Volume(w, h, d, scalar_values);
			ret->SetName(filepath);

			file_ele.close();
			delete[] scalar_values;
			printf("Finished -> Read Volume From .ele File\n");
		}
		else
			printf("Finished -> Error on opening .ele file\n");

		return ret;
	}

	Volume* ReadNodeFile(std::string filepath)
	{
		Volume* ret = NULL;

		printf("Started  -> Read Volume From .node File\n");
		printf("  - File .node Path: %s\n", filepath.c_str());

		std::ifstream file(filepath.c_str());
		if ( file.is_open() )
		{
			int num_of_vertex, number_of_coordinates;
			file >> num_of_vertex >> number_of_coordinates;

			float* scalar_values = new float[num_of_vertex];

			for ( int i = 0; i < num_of_vertex; i++ )
			{
				int index;
				double x, y, z, scalar;
				file >> index >> x >> y >> z >> scalar;
			}

			delete[] scalar_values;

			file.close();
			printf("Finished -> Read Volume From .node File\n");
		}
		else
			printf("Finished -> Error on opening .node file\n");

		return ret;
	}

	Volume* ReadRawFile(std::string filepath)
	{
		Volume* ret = NULL;

		printf("Started  -> Read Volume From .raw File\n");
		printf("  - File .raw Path: %s\n", filepath.c_str());

		std::ifstream file(filepath.c_str());
		if ( file.is_open() )
		{
			int foundinit = filepath.find_last_of('\\');
			std::string filename = filepath.substr(foundinit + 1);
			printf("  - File .raw: %s\n", filename.c_str());

			int foundfp = filename.find_last_of('.');
			filename = filename.substr(0, foundfp);

			int foundsizes = filename.find_last_of('.');
			std::string t_filesizes = filename.substr(foundsizes + 1,
																								filename.size() - foundsizes);

			filename = filename.substr(0, filename.find_last_of('.'));

			int foundbytesize = filename.find_last_of('.');
			std::string t_filebytesize = filename.substr(foundbytesize + 1,
																									 filename.size() - foundbytesize);

			int fw, fh, fd;
			int bytesize;

			// Read the Volume Sizes
			int foundd = t_filesizes.find_last_of('x');
			fd = atoi(t_filesizes.substr(foundd + 1,
				t_filesizes.size() - foundd).c_str());

			t_filesizes = t_filesizes.substr(0, t_filesizes.find_last_of('x'));

			int foundh = t_filesizes.find_last_of('x');
			fh = atoi(t_filesizes.substr(foundh + 1,
				t_filesizes.size() - foundh).c_str());

			t_filesizes = t_filesizes.substr(0, t_filesizes.find_last_of('x'));

			int foundw = t_filesizes.find_last_of('x');
			fw = atoi(t_filesizes.substr(foundw + 1,
				t_filesizes.size() - foundw).c_str());
			// Byte Size
			bytesize = atoi(t_filebytesize.c_str());

			float* scalar_values = ReadVolvisRaw(filepath, (size_t) bytesize, fw, fh, fd);
			ret = new Volume(fw, fh, fd, scalar_values);
			ret->SetName(filepath);

			printf("  - Volume Name     : %s\n", filepath.c_str());
			printf("  - Volume Size     : [%d, %d, %d]\n", fw, fh, fd);
			printf("  - Volume Byte Size: %d\n", bytesize);

			file.close();
			printf("Finished -> Read Volume From .raw File\n");
		}
		else
			printf("Finished -> Error on opening .raw file\n");

		return ret;
	}

	Volume* ReadMedFile(std::string filepath)
	{
		Volume* ret = NULL;

		printf("Started  -> Read Volume From .med File\n");
		printf("  - File .med Path: %s\n", filepath.c_str());

		std::ifstream file(filepath.c_str());
		if ( file.is_open() )
		{
			int foundinit = filepath.find_last_of('\\');
			std::string filename = filepath.substr(foundinit + 1);
			printf("  - File .med: %s\n", filename.c_str());

			int foundfp = filename.find_last_of('.');
			filename = filename.substr(0, foundfp);

			int foundsizes = filename.find_last_of('.');
			std::string type = filename.substr(foundsizes + 1, filename.size() - foundsizes);

			filename = filename.substr(0, filename.find_last_of('.'));

			int fsize, frows, fcolumns;
			file >> fsize >> frows >> fcolumns;

			printf("  - Volume Name     : %s\n", filename.c_str());
			printf("  - Volume Entries  : %d\n", fsize);
			printf("  - Volume Size     : [%d, %d]\n", frows, fcolumns);

			float* scalar_values = new float[frows * fcolumns];
			for ( int t = 0; t < frows * fcolumns; t++ )
				scalar_values[t] = 0;

			int i = 0;
			int j = 0;
			int isovalue = 0;
			for ( int k = 0; k < fsize; k++ )
			{
				file >> j >> i >> isovalue;
				int idx = i + (j * frows);
				printf("[%d, %d] = %d\n", i, j, isovalue);
				scalar_values[idx] = isovalue;
			}

			ret = new Volume(frows, fcolumns, 1, scalar_values);

			file.close();
			printf("Finished -> Read Volume From .med File\n");
		}
		else
			printf("Finished -> Error on opening .med file\n");

		return ret;
	}

	TransferFunction* ReadTransferFunction(std::string file)
	{
		TransferFunction* ret = NULL;

		int found = file.find_last_of('.');
		std::string extension = file.substr(found + 1);

		printf("---------Reading Transfer Function----------\n");
		printf(" - File: %s\n", file.c_str());

		if ( extension.compare("tf1d") == 0 )
			ret = ReadTransferFunction_tf1d(file);
		else if ( extension.compare("tf") == 0 )
			ret = ReadTransferFunction_tf(file);
		else if ( extension.compare("tfg1d") == 0 )
			ret = ReadTransferFunction_tfg1d(file);
		else if ( extension.compare("tfgersa") == 0 )
			ret = ReadTransferFunction_tfgersa(file);

		printf("--------------------------------------------\n");
		return ret;
	}

	TransferFunction* ReadTransferFunction_tf1d(std::string file)
	{
		std::ifstream myfile(file);
		if ( myfile.is_open() )
		{
			std::string interpolation;
			std::getline(myfile, interpolation);
			printf(" - Interpolation: %s\n", interpolation.c_str());

			TransferFunction1D* tf = new TransferFunction1D();
			int init;
			myfile >> init;

			if ( init == 1 )
			{

			}
			else
			{
				int cpt_rgb_size;
				myfile >> cpt_rgb_size;
				double r, g, b, a;
				int isovalue;
				for ( int i = 0; i < cpt_rgb_size; i++ )
				{
					myfile >> r >> g >> b >> isovalue;
					tf->AddRGBControlPoint(TransferControlPoint(r, g, b, isovalue));
				}

				int cpt_alpha_size;
				myfile >> cpt_alpha_size;
				for ( int i = 0; i < cpt_alpha_size; i++ )
				{
					myfile >> a >> isovalue;
					tf->AddAlphaControlPoint(TransferControlPoint(a, isovalue));
				}
			}
			myfile.close();


			if ( interpolation.compare("linear") == 0 )
				tf->m_interpolation_type = TFInterpolationType::LINEAR;
			else if ( interpolation.compare("cubic") == 0 )
				tf->m_interpolation_type = TFInterpolationType::CUBIC;

			int foundname = file.find_last_of('\\');
			std::string tfname = file.substr(foundname + 1);
			tf->SetName(file);

			return tf;
		}
		return NULL;
	}

	TransferFunction* ReadTransferFunction_tf(std::string file)
	{
		TransferFunction_tf* transfer_functions = new TransferFunction_tf();

		FILE* fp = fopen(file.c_str(), "r");
		if ( !fp )
		{
			fprintf(stderr, "Error: cannot read transfer function file.\n");
			exit(-1);
		}

		//Read transfer function file.
		//Skip the lines that starts with #
		int numbpoints = 0;
		char line[256];
		int interpmode = -1;
		bool uniform = true;

		fscanf(fp, " %s", &line);
		while ( strcmp(&line[0], "#") == 0 )
			fscanf(fp, " %s", &line);

		if ( strcmp(line, "constant") == 0 || strcmp(line, "const") == 0 )
			transfer_functions->SetInterpolationMode(TransferFunction_tf::Interpolation::CONSTANT);
		else if ( strcmp(line, "discrete") == 0 )
			transfer_functions->SetInterpolationMode(TransferFunction_tf::Interpolation::DISCRETE);
		else if ( strcmp(line, "linear") == 0 )
			transfer_functions->SetInterpolationMode(TransferFunction_tf::Interpolation::LINEAR);
		else if ( strcmp(line, "non-uniform") == 0 || strcmp(line, "nonuniform") == 0 || strcmp(line, "nu") == 0 )
		{
			transfer_functions->SetInterpolationMode(TransferFunction_tf::Interpolation::LINEAR);
			uniform = false;
		}
		else if ( strcmp(line, "iso") == 0 )
		{
			transfer_functions->SetInterpolationMode(TransferFunction_tf::Interpolation::DISCRETE);
			uniform = false;
		}
		else
		{
			fprintf(stderr, "Error: cannot read transfer function file.\n");
			exit(-1);
		}

		fscanf(fp, " %s", &line);
		numbpoints = atoi(line);

		for ( int i = 0; i < numbpoints; i++ )
		{
			long double scalar;
			long double color[4];

			if ( uniform )
			{
				fscanf(fp, "%f %f %f %f", &color[0], &color[1], &color[2], &color[3]);
				scalar = (float) (i / ((double) numbpoints - 1.0));
			}
			else
				fscanf(fp, "%f %f %f %f %f", &scalar, &color[0], &color[1], &color[2], &color[3]);

			printf ("Control Point: %f %f %f %f %f\n", color[0], color[1], color[2], color[3], scalar);
			transfer_functions->AddControlPoint(color[0], color[1], color[2], color[3], scalar);
		}

		fclose(fp);

		transfer_functions->SetName(file);

		transfer_functions->CalculateLimits();
		transfer_functions->ScaleTo(0.0, 255.0);

		return transfer_functions;
	}

	TransferFunction* ReadTransferFunction_tfg1d(std::string file)
	{
		std::ifstream myfile(file);
		if ( myfile.is_open() )
		{
			TransferFunctionGroups* transfer_functions = new TransferFunctionGroups();

			std::string interpolation;
			std::getline(myfile, interpolation);
			printf(" - Interpolation: %s\n", interpolation.c_str());

			int vmin, vmax;
			myfile >> vmin >> vmax;
			transfer_functions->SetLimits(vmin, vmax);
			printf(" - Limits: [%d, %d]\n", vmin, vmax);

			int groups_size;
			myfile >> groups_size;
			printf(" - Number of Groups: %d\n", groups_size);
			double r, g, b, a;
			int isovalue;
			for ( int i = 0; i < groups_size; i++ )
			{
				myfile >> r >> g >> b >> a >> isovalue;
				transfer_functions->AddGroup(isovalue, lqc::Vector4d(r, g, b, a));
			}

			myfile.close();

			int foundname = file.find_last_of('\\');
			std::string tfname = file.substr(foundname + 1);
			transfer_functions->SetName(file);
			return transfer_functions;
		}
		return NULL;
	}

	TransferFunction* ReadTransferFunction_tfgersa(std::string file)
	{
		std::ifstream myfile(file);
		if ( myfile.is_open() )
		{
			TransferFunctionGersa* tf = new TransferFunctionGersa();
			std::string interpolation;
			std::getline(myfile, interpolation);
			printf(" - Interpolation: %s\n", interpolation.c_str());

			int init;
			myfile >> init;

			if ( init == 1 )
			{

			}
			else
			{
				int cpt_rgb_size;
				myfile >> cpt_rgb_size;
				double r, g, b, a;
				int isovalue;
				for ( int i = 0; i < cpt_rgb_size; i++ )
				{
					myfile >> r >> g >> b >> isovalue;
					tf->AddRGBControlPoint(TransferControlPoint(r, g, b, isovalue));
				}

				int cpt_alpha_size;
				myfile >> cpt_alpha_size;
				for ( int i = 0; i < cpt_alpha_size; i++ )
				{
					myfile >> a;
					tf->AddAlphaControlPoint(TransferControlPoint(a / 255.0, i));
				}
			}

			myfile.close();

			if ( interpolation.compare("linear") == 0 )
				tf->m_interpolation_type = TFInterpolationType::LINEAR;
			else if ( interpolation.compare("cubic") == 0 )
				tf->m_interpolation_type = TFInterpolationType::CUBIC;

			int foundname = file.find_last_of('\\');
			std::string tfname = file.substr(foundname + 1);
			tf->SetName(file);

			return tf;
		}
		return NULL;
	}
}
