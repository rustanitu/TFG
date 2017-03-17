/// ClusterSet.cpp
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#include "ClusterSet.h"

#include <armadillo>
#include <vector>
#include <forward_list>

ClusterSet::ClusterSet(const int& width, const int& height)
: m_width(width), m_height(height)
{
}

std::forward_list<Cluster>& ClusterSet::KMeans(const int& k,
  std::forward_list<PMCell*>& defined_cells,
  std::forward_list<PMCell*>& undefined_cells)
{
	m_clusters.clear();
	srand(time(NULL));

	int div = k + 2;
	int xinc = m_width / div;
	int yinc = m_height / div;
	for ( int i = 1; i <= k; ++i )
	{
		int x = xinc * i;
		for ( int j = 1; j <= k; ++j )
		{
			int y = yinc * j;
			Cluster c(x, y);
			m_clusters.push_front(c);
		}
	}

	bool repeat = true;
	while ( repeat )
	{
		repeat = false;

    AssignDefinedCellsToClusters(defined_cells);

		for ( auto cluster_it = m_clusters.begin(); cluster_it != m_clusters.end(); ++cluster_it )
		{
			int cx = cluster_it->GetX();
			int cy = cluster_it->GetY();

			cluster_it->ReCenter();

			int x = cluster_it->GetX();
			int y = cluster_it->GetY();
			if ( x != cx || y != cy )
				repeat = true;
		}

		if ( repeat )
		{
			for ( auto cluster_it = m_clusters.begin(); cluster_it != m_clusters.end(); ++cluster_it )
			{
        cluster_it->RemoveDefinedCells(defined_cells);
			}
		}
	}

  AssignUndefinedCellsToClusters(undefined_cells);

	return m_clusters;
}

void ClusterSet::AssignDefinedCellsToClusters(std::forward_list<PMCell*>& cells)
{
	auto cell_it_before = cells.before_begin();
	auto cell_it = cells.begin();
	while ( !cells.empty() )
	{
    PMCell* cell = *cell_it;
		int x = cell->GetX();
		int y = cell->GetY();

		float mindist = FLT_MAX;
		auto mincluster_it = m_clusters.end();

		for ( auto cluster_it = m_clusters.begin(); cluster_it != m_clusters.end(); ++cluster_it )
		{
			int cx = cluster_it->GetX();
			int cy = cluster_it->GetY();

			float dist = (x - cx) * (x - cx) + (y - cy) * (y - cy);
			if ( dist < mindist )
			{
				mindist = dist;
				mincluster_it = cluster_it;
			}
		}

		mincluster_it->InsertDefinedCells(cells, cell_it_before);

		cell_it_before = cells.before_begin();
		cell_it = cells.begin();
	}
}

void ClusterSet::AssignUndefinedCellsToClusters(std::forward_list<PMCell*>& cells)
{
  auto cell_it_before = cells.before_begin();
  auto cell_it = cells.begin();
  while (!cells.empty())
  {
    PMCell* cell = *cell_it;
    int x = cell->GetX();
    int y = cell->GetY();

    float mindist = FLT_MAX;
    auto mincluster_it = m_clusters.end();

    for (auto cluster_it = m_clusters.begin(); cluster_it != m_clusters.end(); ++cluster_it)
    {
      int cx = cluster_it->GetX();
      int cy = cluster_it->GetY();

      float dist = (x - cx) * (x - cx) + (y - cy) * (y - cy);
      if (dist < mindist)
      {
        mindist = dist;
        mincluster_it = cluster_it;
      }
    }

    mincluster_it->InsertUndefinedCells(cells, cell_it_before);

    cell_it_before = cells.before_begin();
    cell_it = cells.begin();
  }
}