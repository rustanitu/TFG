#include "gtest/gtest.h"

#include <volrend\Volume.h>
#include <volrend\Reader.h>
#include <../ATFGeneratorApp/src/AutomaticTransferFunction/ATFGenerator.h>
#include <../ATFGeneratorApp/src/AutomaticTransferFunction/PGMFile.h>

const char* VOLUME_TEST_FILE = "..\\..\\Modelos\\VolumeModels\\Engine.1.256x256x128.raw";

TEST(ATFGeneratorTest, Creation)
{
  ASSERT_THROW(ATFGenerator(NULL), std::exception_ptr);

  vr::Volume* vol = new vr::Volume();
  ATFGenerator* atfg = NULL;

  ASSERT_NO_THROW(atfg = new ATFGenerator(vol));

  delete atfg;
  delete vol;
}

TEST(ATFGeneratorTest, WithoutInit)
{
  vr::Volume* vol = new vr::Volume();
  ATFGenerator* atfg = new ATFGenerator(vol);

  ASSERT_THROW(atfg->ExtractTransferFunction(), std::domain_error);
  ASSERT_THROW(atfg->GenerateGradientSummedHistogram(), std::domain_error);
  ASSERT_THROW(atfg->GenerateHistogramSlice(0), std::domain_error);
  ASSERT_THROW(atfg->GenerateHistogramSlices(), std::domain_error);
  ASSERT_THROW(atfg->GenerateLaplacianSummedHistogram(), std::domain_error);
  ASSERT_THROW(atfg->GetGradient(0, 0, 0), std::domain_error);
  ASSERT_THROW(atfg->GetLaplacian(0, 0, 0), std::domain_error);

  delete atfg;
  delete vol;
}

TEST(ATFGeneratorTest, Init)
{
  vr::Volume* vol = new vr::Volume(0, 0, 0);
  ATFGenerator* atfg = new ATFGenerator(vol);

  ASSERT_THROW(atfg->Init(), std::out_of_range);

  delete atfg;
  delete vol;
  
  vol = vr::ReadFromVolMod(VOLUME_TEST_FILE);
  atfg = new ATFGenerator(vol);

  ASSERT_TRUE(atfg->Init());

  delete atfg;
  delete vol;
}

TEST(ATFGeneratorTest, ExtractTransferFunction)
{
  vr::Volume* vol = vr::ReadFromVolMod(VOLUME_TEST_FILE);
  ATFGenerator* atfg = new ATFGenerator(vol);

  ASSERT_TRUE(atfg->Init());
  ASSERT_TRUE(atfg->ExtractTransferFunction());

  delete atfg;
  delete vol;
}

TEST(ATFGeneratorTest, GetTransferFunction)
{
  vr::Volume* vol = vr::ReadFromVolMod(VOLUME_TEST_FILE);
  ATFGenerator* atfg = new ATFGenerator(vol);

  ASSERT_TRUE(atfg->Init());
  ASSERT_EQ(atfg->GetTransferFunction(), (ITransferFunction*)NULL);
  ASSERT_TRUE(atfg->ExtractTransferFunction());
  ASSERT_NE(atfg->GetTransferFunction(), (ITransferFunction*)NULL);

  delete atfg;
  delete vol;
}

TEST(ATFGeneratorTest, BorderVolumeDerivatives)
{
  vr::Volume* vol = vr::ReadFromVolMod(VOLUME_TEST_FILE);
  ATFGenerator* atfg = new ATFGenerator(vol);

  ASSERT_TRUE(atfg->Init());

  int width = vol->GetWidth();
  int height = vol->GetHeight();
  int depth = vol->GetDepth();

  for (int x = 0; x < width; x++)
  {
    for (int y = 0; y < height; y++)
    {
      for (int z = 0; z < depth; z++)
      {
        if (x*y*z == 0 || x == width - 1 || y == height - 1 || z == depth - 1)
        {
          ASSERT_EQ(atfg->GetGradient(x, y, z), 0);
          ASSERT_EQ(atfg->GetLaplacian(x, y, z), 0);
        }
        else
        {
          float g = 0.0f;
          float gx = 0.0f;
          float gy = 0.0f;
          float gz = 0.0f;

          gx = 0.5f * (vol->GetValue(x + 1, y, z) - vol->GetValue(x - 1, y, z));
          gy = 0.5f * (vol->GetValue(x, y + 1, z) - vol->GetValue(x, y - 1, z));
          gz = 0.5f * (vol->GetValue(x, y, z + 1) - vol->GetValue(x, y, z - 1));
          g = sqrt(gx*gx + gy*gy + gz*gz);
          ASSERT_EQ(atfg->GetGradient(x, y, z), g);

          float l = 0.0f;
          float lx = 0.0f;
          float ly = 0.0f;
          float lz = 0.0f;

          float v = 2 * vol->GetValue(x, y, z);
          lx = vol->GetValue(x + 1, y, z) - v + vol->GetValue(x - 1, y, z);
          ly = vol->GetValue(x, y + 1, z) - v + vol->GetValue(x, y - 1, z);
          lz = vol->GetValue(x, y, z + 1) - v + vol->GetValue(x, y, z - 1);
          l = lx + ly + lz;
          ASSERT_EQ(atfg->GetLaplacian(x, y, z), l);
        }
      }
    }
  }

  delete atfg;
  delete vol;
}

TEST(ATFGeneratorTest, GradientHistogram)
{
  vr::Volume* vol = vr::ReadFromVolMod(VOLUME_TEST_FILE);
  ATFGenerator* atfg = new ATFGenerator(vol);

  const char* filename = "Gradient Summed Histogram.pgm";
  PGMFile* file = new PGMFile("Gradient Summed Histogram", 1, 1);
  ASSERT_TRUE(file->Open());
  file->WriteByte(127);
  file->Close();

  FILE* fp = NULL;
  int erro = fopen_s(&fp, filename, "rb");
  ASSERT_EQ(erro, 0);
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fclose(fp);

  ASSERT_TRUE(atfg->Init());
  ASSERT_NO_THROW(atfg->GenerateGradientSummedHistogram());

  fp = NULL;
  erro = fopen_s(&fp, filename, "rb");
  ASSERT_EQ(erro, 0);
  fseek(fp, 0, SEEK_END);
  long hist_size = ftell(fp);
  fclose(fp);

  ASSERT_GT(hist_size, size);

  delete atfg;
  delete vol;
}

TEST(ATFGeneratorTest, LaplacianHistogram)
{
  vr::Volume* vol = vr::ReadFromVolMod(VOLUME_TEST_FILE);
  ATFGenerator* atfg = new ATFGenerator(vol);

  const char* filename = "Laplacian Summed Histogram.pgm";
  PGMFile* file = new PGMFile("Laplacian Summed Histogram", 1, 1);
  ASSERT_TRUE(file->Open());
  file->WriteByte(127);
  file->Close();

  FILE* fp = NULL;
  int erro = fopen_s(&fp, filename, "rb");
  ASSERT_EQ(erro, 0);
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fclose(fp);

  ASSERT_TRUE(atfg->Init());
  ASSERT_NO_THROW(atfg->GenerateLaplacianSummedHistogram());

  fp = NULL;
  erro = fopen_s(&fp, filename, "rb");
  ASSERT_EQ(erro, 0);
  fseek(fp, 0, SEEK_END);
  long hist_size = ftell(fp);
  fclose(fp);

  ASSERT_GT(hist_size, size);

  delete atfg;
  delete vol;
}

TEST(ATFGeneratorTest, HistogramSlice)
{
  vr::Volume* vol = vr::ReadFromVolMod(VOLUME_TEST_FILE);
  ATFGenerator* atfg = new ATFGenerator(vol);

  const char* filename = "Histogram Slice 0.pgm";
  PGMFile* file = new PGMFile("Histogram Slice 0", 1, 1);
  ASSERT_TRUE(file->Open());
  file->WriteByte(127);
  file->Close();

  FILE* fp = NULL;
  int erro = fopen_s(&fp, filename, "rb");
  ASSERT_EQ(erro, 0);
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fclose(fp);

  ASSERT_TRUE(atfg->Init());
  ASSERT_NO_THROW(atfg->GenerateHistogramSlice(0));

  fp = NULL;
  erro = fopen_s(&fp, filename, "rb");
  ASSERT_EQ(erro, 0);
  fseek(fp, 0, SEEK_END);
  long hist_size = ftell(fp);
  fclose(fp);

  ASSERT_GT(hist_size, size);

  ASSERT_THROW(atfg->GenerateHistogramSlice(256), std::domain_error);

  delete atfg;
  delete vol;
}