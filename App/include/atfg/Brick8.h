/// Brick8.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef BRICK8_H
#define BRICK8_H

#include "ScalarField.h"

#include <stdlib.h>
#include <float.h>

struct Node
{
  int refcount;
  double value;
  Node() : refcount(0), value(DBL_MAX) {}
};

class Brick8
{
public:
  Brick8(vr::ScalarField* scalarfield);
  ~Brick8();

  int GetId(int x, int y, int z);
  void Assign(double val, int x, int y, int z);

  void WriteNeutralFile();

private:
  int m_width;
  int m_height;
  int m_depth;
  Node* m_nodes;
};

#endif