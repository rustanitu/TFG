/// Brick8.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "Brick8.h"

static int BRICK8_TO_TANK[8] = { 5, 7, 3, 1, 4, 6, 2, 0 };

Brick8::Brick8(Tank* tank)
: m_nodes(NULL)
, m_tank(tank)
, m_file(NULL)
{
  int size = tank->GetVerticesNum();
  m_nodes = new Node[size];
  glm::vec3* verts = tank->GetVertices();
  for (int i = 0; i < size; ++i)
  {
    m_nodes[i].pos = verts[i];
  }
}

Brick8::~Brick8()
{
  delete[] m_nodes;
}

//int Brick8::GetId(int x, int y, int z)
//{
//  return x + y;// *m_width + z * m_width * m_height;
//}
//
//bool Brick8::IsOutOfRange(int x, int y, int z)
//{
//  int id = GetId(x, y, z);
//  //if (id > -1 && id < m_width * m_height * m_depth)
//  //  return false;
//  return true;
//}
//
//void Brick8::Assign(Tank* tank, int x, int y, int z)
//{
//  x *= 2;
//  y *= 2;
//  z *= 2;
//
//  int id = tank->GetId(x, y, z);
//  double val = tank->GetValue(id);
//  const Cell& cell = tank->GetCell(id);
//  for (int i = 0; i < 8; ++i)
//  {
//    m_nodes[cell.GetIthVertexIndex(i)].value += val;
//    m_nodes[cell.GetIthVertexIndex(i)].refcount++;
//  }
//  m_nodes[GetId(x + 1, y, z)].value = val;
//  m_nodes[GetId(x + 1, y, z)].refcount++;
//
//  m_nodes[GetId(x + 1, y + 1, z)].value = val;
//  m_nodes[GetId(x + 1, y + 1, z)].refcount++;
//
//  m_nodes[GetId(x + 1, y + 1, z + 1)].value = val;
//  m_nodes[GetId(x + 1, y + 1, z + 1)].refcount++;
//  
//  m_nodes[GetId(x + 1, y, z + 1)].value = val;
//  m_nodes[GetId(x + 1, y, z + 1)].refcount++;
//  
//  m_nodes[GetId(x, y, z)].value = val;
//  m_nodes[GetId(x, y, z)].refcount++;
//  
//  m_nodes[GetId(x, y + 1, z)].value = val;
//  m_nodes[GetId(x, y + 1, z)].refcount++;
//  
//  m_nodes[GetId(x, y + 1, z + 1)].value = val;
//  m_nodes[GetId(x, y + 1, z + 1)].refcount++;
//  
//  m_nodes[GetId(x, y, z + 1)].value = val;
//  m_nodes[GetId(x, y, z + 1)].refcount++;
//}
//
//void Brick8::Assign(double val, int x, int y, int z)
//{
//  m_nodes[GetId(x + 1, y, z)].value = val;
//  m_nodes[GetId(x + 1, y, z)].refcount++;
//  m_nodes[GetId(x + 1, y + 1, z)].value = val;
//  m_nodes[GetId(x + 1, y + 1, z)].refcount++;
//  m_nodes[GetId(x + 1, y + 1, z + 1)].value = val;
//  m_nodes[GetId(x + 1, y + 1, z + 1)].refcount++;
//  m_nodes[GetId(x + 1, y, z + 1)].value = val;
//  m_nodes[GetId(x + 1, y, z + 1)].refcount++;
//  m_nodes[GetId(x, y, z)].value = val;
//  m_nodes[GetId(x, y, z)].refcount++;
//  m_nodes[GetId(x, y + 1, z)].value = val;
//  m_nodes[GetId(x, y + 1, z)].refcount++;
//  m_nodes[GetId(x, y + 1, z + 1)].value = val;
//  m_nodes[GetId(x, y + 1, z + 1)].refcount++;
//  m_nodes[GetId(x, y, z + 1)].value = val;
//  m_nodes[GetId(x, y, z + 1)].refcount++;
//}

void Brick8::Write(const char* str)
{
  *m_file << str << " ";
}

void Brick8::Write(const int& content)
{
  *m_file << content << " ";
}

void Brick8::Write(const float& content)
{
  *m_file << content << " ";
}

//void Brick8::Write(const double& content)
//{
//  *m_file << std::fixed << content << " ";
//}

void Brick8::Write(const glm::vec3& content)
{
  Write(content.x);
  Write(content.y);
  Write(content.z);
}

void Brick8::FinishLine()
{
  *m_file << std::endl;
}

void Brick8::WriteNeutralFile(const char* file)
{
  delete m_file;
  m_file = new std::ofstream(file, std::ofstream::binary);
  printf("Writing %s neutral file.\n", file);

  Write("%HEADER"); FinishLine();
  Write("'Tank'"); FinishLine();

  Write("%HEADER.AUTHOR"); FinishLine();
  Write("'ATFG'"); FinishLine();

  printf("Writing neutral file NODE section.\n");

  const int node_size = m_tank->GetVerticesNum();
  Write("%NODE"); FinishLine();
  Write(node_size); FinishLine();

  Write("%NODE.COORD"); FinishLine();
  Write(node_size); FinishLine();
  for (int i = 0; i < node_size; ++i)
  {
    Write(i + 1);
    Write(m_nodes[i].pos);
    FinishLine();
  }

  printf("Writing neutral file ELEMENT section.\n");

  const int elem_size = m_tank->GetCellsNum();
  Write("%ELEMENT"); FinishLine();
  Write(elem_size); FinishLine();

  Write("%ELEMENT.BRICK8"); FinishLine();
  Write(elem_size); FinishLine();
  for (int i = 0; i < elem_size; ++i)
  {
    Write(i + 1);
    Write(1);
    Write(1);
    const Cell& cell = m_tank->GetCell(i);
    for (int j = 0; j < 8; ++j)
    {
      const int id = cell.GetIthVertexIndex(BRICK8_TO_TANK[j]);
      Write(id + 1);
      m_nodes[id].value += cell.GetValue();
      m_nodes[id].grad += cell.GetGradient();
      m_nodes[id].refcount++;
    }
    FinishLine();
  }

  printf("Writing neutral file RESULT section.\n");

  Write("%RESULT"); FinishLine();
  Write(1); FinishLine();
  Write(1); Write("unique_case"); FinishLine();

  Write("%RESULT.CASE"); FinishLine();
  Write(1); Write(1); FinishLine();
  Write(1); Write("unique_step"); FinishLine();

  Write("%RESULT.CASE.STEP"); FinishLine();
  Write(1); FinishLine();

  Write("%RESULT.CASE.STEP.NODAL.SCALAR"); FinishLine();
  Write(2); FinishLine();
  Write("'Value'"); Write("'Gradient'"); FinishLine();

  Write("%RESULT.CASE.STEP.NODAL.SCALAR.DATA"); FinishLine();
  Write(node_size); FinishLine();
  for (int i = 0; i < node_size; ++i)
  {
    Write(i + 1);
    Write((float)m_nodes[i].value / m_nodes[i].refcount);
    Write(0);
    FinishLine();
  }
  
  Write("%END");

  m_file->close();
  printf("Finish writing neutral file.\n");
}