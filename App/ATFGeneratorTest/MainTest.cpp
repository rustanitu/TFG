#include "gtest/gtest.h"

#include <volrend\Volume.h>
#include <volrend\Reader.h>
#include <../ATFGeneratorApp/src/AutomaticTransferFunction/ATFGenerator.h>

TEST(ATFGeneratorTest, UseWithoutInit)
{
  //ASSERT_DEATH_IF_SUPPORTED(ATFGenerator(NULL), "");

  //vr::Volume* vol = new vr::Volume(0,0,0);
  //ATFGenerator* atfg = new ATFGenerator(vol);

  //ASSERT_DEATH_IF_SUPPORTED(atfg->ExtractTransferFunction(), 1, "");

  //EXPECT_DEATH(atfg->GenerateGradientSummedHistogram(), "");
  //EXPECT_DEATH(atfg->GenerateHistogramSlice(0), "");
  //EXPECT_DEATH(atfg->GenerateHistogramSlices(), "");
  //EXPECT_DEATH(atfg->GenerateLaplacianSummedHistogram(), "");
  //EXPECT_DEATH(atfg->GetGradient(0, 0, 0), "");
  //EXPECT_DEATH(atfg->GetLaplacian(0, 0, 0), "");

  //delete atfg;
  //delete vol;
}

TEST(ATFGeneratorTest, InitTest)
{
  vr::Volume* vol = new vr::Volume(0, 0, 0);
  ATFGenerator* atfg = new ATFGenerator(vol);
  ASSERT_FALSE(atfg->Init());
  delete atfg;
  delete vol;

  vol = vr::ReadFromVolMod("..\\..\\Modelos\\VolumeModels\\Engine.1.256x256x128.raw");
  atfg = new ATFGenerator(vol);
  ASSERT_TRUE(atfg->Init());
  delete atfg;
  delete vol;
}