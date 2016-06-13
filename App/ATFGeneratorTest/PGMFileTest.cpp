#include "gtest/gtest.h"

#include <../ATFGeneratorApp/src/AutomaticTransferFunction/PGMFile.h>

TEST(PGMFileTest, Creation)
{
  ASSERT_THROW(PGMFile(NULL, 0, 0), std::exception_ptr);
  ASSERT_NO_THROW(PGMFile("tmpfile", 0, 0));
}

TEST(PGMFileTest, Opening)
{
  PGMFile* file = NULL;
  ASSERT_NO_THROW(file = new PGMFile("tmpfile", 0, 0));
  ASSERT_TRUE(file->Open());
  ASSERT_THROW(file->Open(), std::domain_error);
  ASSERT_NO_THROW(file->Close());
  ASSERT_TRUE(file->Open());
  ASSERT_NO_THROW(file->Close());
  delete file;
}

TEST(PGMFileTest, WriteRead)
{
  PGMFile* file = NULL;
  ASSERT_NO_THROW(file = new PGMFile("tmpfile", 10, 10));
  ASSERT_TRUE(file->Open());

  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      ASSERT_NO_THROW(file->WriteByte(i + j));
    }
    ASSERT_NO_THROW(file->WriteEndLine());
  }

  ASSERT_NO_THROW(file->Close());
  delete file;

  FILE* fp = NULL;
  int error = fopen_s(&fp, "tmpfile.pgm", "r");
  ASSERT_EQ(error, 0);
  
  char code[2];
  int match = fscanf_s(fp, "%s", code);
  ASSERT_EQ(match, 1);
  ASSERT_STREQ(code, PGM_CODE);
  int width, height;
  match = fscanf_s(fp, "%d %d \n", &width, &height);
  ASSERT_EQ(match, 2);
  ASSERT_EQ(width, 10);
  ASSERT_EQ(height, 10);
  int max;
  match = fscanf_s(fp, "%d \n", &max);
  ASSERT_EQ(match, 1);
  ASSERT_EQ(max, UCHAR_MAX);

  int byte;
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      match = fscanf_s(fp, "%d ", &byte);
      ASSERT_EQ(match, 1);
      ASSERT_EQ(byte, i + j);
    }
    fscanf_s(fp, "\n");
  }

  fclose(fp);
}