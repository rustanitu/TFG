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
  ASSERT_NO_THROW(tf->SetValueColor(0,0,0,0));
  ASSERT_NO_THROW(tf->Generate());
  delete tf;
}