#include "gtest/gtest.h"

#include <../ATFGeneratorApp/src/AutomaticTransferFunction/TransferFunction.h>

TEST(TransferFunctionTest, Creation)
{
  ASSERT_THROW(TransferFunction(NULL), std::exception_ptr);

  TransferFunction* tf = NULL;
  ASSERT_NO_THROW(tf = new TransferFunction("TransferFunction"));
  delete tf;
}

TEST(TransferFunctionTest, WithoutInit)
{
  TransferFunction* tf = NULL;
  ASSERT_NO_THROW(tf = new TransferFunction("TransferFunction"));
  ASSERT_THROW(tf->Generate(), std::exception_ptr);
  ASSERT_NO_THROW(tf->SetClosestBoundaryDistances(NULL, NULL, NULL, 127));
  ASSERT_THROW(tf->Generate(), std::exception_ptr);
  delete tf;
}

TEST(TransferFunctionTest, NoBoundaryDistances)
{
  TransferFunction* tf = NULL;
  ASSERT_NO_THROW(tf = new TransferFunction("TransferFunction"));
  ASSERT_THROW(tf->SetClosestBoundaryDistances(NULL, NULL, NULL, 0), std::length_error);
  ASSERT_THROW(tf->SetClosestBoundaryDistances(NULL, NULL, NULL, 257), std::length_error);
  delete tf;
}

TEST(TransferFunctionTest, NoColor)
{
  TransferFunction* tf = NULL;
  ASSERT_NO_THROW(tf = new TransferFunction("TransferFunction"));

  unsigned char v[2] = {100, 200};
  float d[2] = {0.2, 0.4};
  float s[2] = {0.3, 0.3};

  ASSERT_NO_THROW(tf->SetClosestBoundaryDistances(v, d, s, 2));
  ASSERT_THROW(tf->Generate(), std::domain_error);
  delete tf;
}

TEST(TransferFunctionTest, Correct)
{
  TransferFunction* tf = NULL;
  ASSERT_NO_THROW(tf = new TransferFunction("TransferFunction"));

  unsigned char v[2] = { 100, 200 };
  float d[2] = { 0.2, 0.4 };
  float s[2] = { 0.3, 0.3 };

  ASSERT_NO_THROW(tf->SetClosestBoundaryDistances(v, d, s, 2));

  unsigned char r = 50;
  unsigned char g = 100;
  unsigned char b = 150;
  unsigned char value = 200;
  ASSERT_NO_THROW(tf->SetValueColor(value, r, g, b));
  ASSERT_NO_THROW(tf->Generate());
  
  char * path = tf->GetPath();
  ASSERT_TRUE(path != NULL);

  FILE* fp = NULL;
  int error = fopen_s(&fp, path, "r");
  ASSERT_EQ(error, 0);
  ASSERT_TRUE(fp != NULL);

  int match = 0;
  char buffer[80];

  match = fscanf_s(fp, "%s", buffer);
  ASSERT_EQ(match, 1);
  ASSERT_STREQ(buffer, "linear");

  match = fscanf_s(fp, "%s", buffer);
  ASSERT_EQ(match, 1);
  ASSERT_STREQ(buffer, "0");

  match = fscanf_s(fp, "%s", buffer);
  ASSERT_EQ(match, 1);
  ASSERT_STREQ(buffer, "1");

  float rr, rg, rb;
  unsigned char rvalue;
  match = fscanf_s(fp, "%f %f %f %d", &rr, &rg, &rb, &rvalue);
  ASSERT_EQ(match, 4);
  ASSERT_NEAR(r / 255.0f, rr, 0.000001);
  ASSERT_NEAR(g / 255.0f, rg, 0.000001);
  ASSERT_NEAR(b / 255.0f, rb, 0.000001);
  ASSERT_EQ(value, rvalue);

  match = fscanf_s(fp, "%s", buffer);
  ASSERT_EQ(match, 1);
  ASSERT_STREQ(buffer, "2");

  int op, val;

  match = fscanf_s(fp, "%d %d", &op, &val);
  ASSERT_EQ(match, 2);
  ASSERT_GE(op, 0);
  ASSERT_LE(op, 1);
  ASSERT_EQ(val, 100);

  match = fscanf_s(fp, "%d %d", &op, &val);
  ASSERT_EQ(match, 2);
  ASSERT_GE(op, 0);
  ASSERT_LE(op, 1);
  ASSERT_EQ(val, 200);

  delete tf;
}