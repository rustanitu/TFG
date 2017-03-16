/// Cluster.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "Cluster.h"
#include <unordered_set>

Cluster::Cluster(const int& base)
: m_base(base), m_size(0)
{
}

void Cluster::Insert(PMCell* cell)
{
  if (Has(cell))
  {
    //printf("Valor ja definido para a celula (%d, %d)\n", cell->GetX(), cell->GetY());
    return;
  }

  m_set.insert(cell);
  m_list.push_front(cell);
  m_size++;
}

Cluster* Cluster::Merge(Cluster** cluster1, Cluster** cluster2)
{
  if (*cluster1 == NULL)
  {
    delete *cluster1;
    *cluster1 = NULL;
    return *cluster2;
  }
  
  if (*cluster2 == NULL)
  {
    delete *cluster2;
    *cluster2 = NULL;
    return *cluster1;
  }

  Cluster* receiver = *cluster1;
  Cluster* giver = *cluster2;
  if (giver->m_size > receiver->m_size)
  {
    Cluster* aux = receiver;
    receiver = giver;
    giver = aux;
  }

  receiver->m_list.splice_after(receiver->m_list.before_begin(), giver->m_list);
  receiver->m_set.insert(giver->m_set.begin(), giver->m_set.end());
  receiver->m_size += giver->m_size;

  giver->m_set.clear();
  giver->m_size = 0;
  
  if (receiver == *cluster1)
  {
    delete *cluster2;
    *cluster2 = NULL;
  }
  else
  {
    delete *cluster1;
    *cluster1 = NULL;
  }

  return receiver;
}

bool Cluster::Has(const int& i, const int& j)
{
  if (GetCell(i, j) != NULL)
    return true;

  return false;
}

bool Cluster::Has(PMCell* cell)
{
  auto it = m_set.find(cell);
  if (it != m_set.end())
    return true;

  return false;
}

bool Cluster::IsEmpty()
{
  return m_set.empty();
}

const std::unordered_set<PMCell*, PMCellHash, PMCellComparator>& Cluster::GetCells() const
{
  return m_set;
}

std::forward_list<PMCell*>& Cluster::GetCellsList()
{
  return m_list;
}

double Cluster::GetValue(const int& i, const int& j)
{
  return GetCell(i, j)->GetValue();
}

bool Cluster::IsDefined(const int& i, const int& j)
{
  return GetCell(i, j)->IsDefined();
}

PMCell* Cluster::GetCell(const int& i, const int& j)
{
  PMCell* dummy_cell = new PMCell(i, j, m_base);
  auto it = m_set.find(dummy_cell);
  delete dummy_cell;
  if (it != m_set.end())
  {
    return *it;
  }

  return NULL;
}