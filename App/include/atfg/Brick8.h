/// Brick8.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef BRICK8_H
#define BRICK8_H

#include "Tank.h"

#include <stdlib.h>
#include <float.h>
#include <fstream>

struct Node
{
  int refcount;
  double value;
  glm::vec3 pos;
  Node() : refcount(0), value(0) {}
};

class Brick8
{
public:
  Brick8(Tank* tank);
  ~Brick8();

  void WriteNeutralFile(const char* file);

private:
  void Write(const char* str);

  void Write(const int& content);

  void Write(const float& content);

  //void Write(const double& content);
  
  void Write(const glm::vec3& content);

  void FinishLine();

private:
  Node* m_nodes;
  Tank* m_tank;
  std::ofstream* m_file;
};

#endif