//Link http://www.cs.virginia.edu/~gfx/pubs/antimony/
//Credits to Daniel Dunbar & Greg Humphreys

#include <lqc/Sampling/PoissonDisk.h>
#include <lqc/Sampling/ScallopedRegion.h>
#include <lqc/Sampling/WeightedDiscretePDF.h>
#include <map>

#ifndef PDGridMaxSamplePerCell
#define PDGridMaxSamplePerCell 9
#endif

namespace lqc
{
  PoissonDiskSampler::PoissonDiskSampler (float radius, bool isTiled, bool use_grid)
    : m_radius (radius), m_is_tiled (isTiled), m_use_grid (use_grid), SamplerType ("NULL")
  {
    if (m_use_grid)
    {
      //Utilizando esse valor, a máxima quantidade de amostras
      // é igual à 9.
      //Número de células em x e y.
      //Quantidade de células = m_gridSize*m_gridSize
      // 2.0, pois o grid vai de -1 a 1
      // 4.0 * r, pois dessa forma, conseguimos colocar 9
      //círculos em cada célula
      m_gridSize = (int)ceil (2.0 / (4.0*m_radius));
      if (m_gridSize < 2) m_gridSize = 2;

      //comprimento e altura de cada célula
      m_gridCellSize = 2.0f / m_gridSize;

      m_grid = new int[m_gridSize*m_gridSize][PDGridMaxSamplePerCell];

      for (int y = 0; y < m_gridSize; y++)
      for (int x = 0; x < m_gridSize; x++)
      for (int k = 0; k < PDGridMaxSamplePerCell; k++)
        m_grid[y*m_gridSize + x][k] = -1;
    }
    else
    {
      m_gridSize = 0;
      m_gridCellSize = 0;
      m_grid = 0;
    }
  }

  PoissonDiskSampler::~PoissonDiskSampler ()
  {
    delete[] m_grid;
    m_points.clear ();
  }

  std::vector<Vector2f> PoissonDiskSampler::GetPoints ()
  {
    return m_points;
  }

  void PoissonDiskSampler::GetGridXY (Vector2f v, int *gx_out, int *gy_out)
  {
    int gx = *gx_out = (int)floor (.5*(v.x + 1)*m_gridSize);
    int gy = *gy_out = (int)floor (.5*(v.y + 1)*m_gridSize);
    if (gx < 0 || gx >= m_gridSize || gy < 0 || gy >= m_gridSize) {
      printf ("Internal error, point outside grid was generated, ignoring.\n");
    }
  }

  Vector2f PoissonDiskSampler::GetTiled (Vector2f v)
  {
    float x = v.x, y = v.y;

    if (m_is_tiled) {
      if (x<-1) x += 2;
      else if (x>1) x -= 2;

      if (y<-1) y += 2;
      else if (y>1) y -= 2;
    }

    return Vector2f (x, y);
  }

  Vector2f PoissonDiskSampler::RandomPoint ()
  {
    return Vector2f ((2.0f * frand ()) - 1.0f,
      (2.0f * frand ()) - 1.0f);
  }

  void PoissonDiskSampler::AddPoint (Vector2f pt)
  {
    int i, gx, gy, *cell;

    m_points.push_back (pt);

    if (m_grid) {
      GetGridXY (pt, &gx, &gy);
      cell = m_grid[gy*m_gridSize + gx];
      for (i = 0; i < PDGridMaxSamplePerCell; i++) {
        if (cell[i] == -1){
          cell[i] = (int)m_points.size () - 1;
          break;
        }
      }
      if (i == PDGridMaxSamplePerCell)
      {
        printf ("Internal error, overflowed max points per grid cell. Exiting.\n");
        exit (1);
      }
    }
  }

  bool PoissonDiskSampler::PointInDomain (Vector2f p)
  {
    return -1 <= p.x && -1 <= p.y && 1 >= p.x && 1 >= p.y;
  }

  void PoissonDiskSampler::ResetPoints ()
  {
    m_points.clear ();
  }

  int PoissonDiskSampler::FindNeighbors (Vector2f pt, float distance)
  {
    if (!m_grid) {
      printf ("Internal error, sampler cannot search without grid.\n");
      return -1;
    }

    float distanceSqrd = distance*distance;
    int i, j, k, gx, gy, N = (int)ceil (distance / m_gridCellSize);
    //Multiplica por 2
    if (N > (m_gridSize >> 1)) N = m_gridSize >> 1;

    m_neighbors.clear ();
    GetGridXY (pt, &gx, &gy);
    //Verificar as vizinhanças ao redor e
    // testar os pontos que estão nessas células
    for (j = -N; j <= N; j++) {
      for (i = -N; i <= N; i++) {
        int cx = (gx + i + m_gridSize) % m_gridSize;
        int cy = (gy + j + m_gridSize) % m_gridSize;
        int *cell = m_grid[cy*m_gridSize + cx];

        for (k = 0; k < PDGridMaxSamplePerCell; k++) {
          if (cell[k] == -1) {
            break;
          }
          else {
            //Verifica se algum ponto perto está quebrando a regra de distância
            // mínima
            if (SquareDistance (GetTiled (m_points[cell[k]] - pt)) <= distanceSqrd)
              m_neighbors.push_back (cell[k]);
          }
        }
      }
    }
    return (int)m_neighbors.size ();
  }

  float PoissonDiskSampler::FindClosestNeighbor (Vector2f pt, float distance)
  {
    if (!m_grid) {
      printf ("Internal error, sampler cannot search without grid.\n");
      exit (1);
    }

    float closestSqrd = distance*distance;
    int i, j, k, gx, gy, N = (int)ceil (distance / m_gridCellSize);
    if (N > (m_gridSize >> 1)) N = m_gridSize >> 1;

    GetGridXY (pt, &gx, &gy);
    for (j = -N; j <= N; j++) {
      for (i = -N; i <= N; i++) {
        int cx = (gx + i + m_gridSize) % m_gridSize;
        int cy = (gy + j + m_gridSize) % m_gridSize;
        int *cell = m_grid[cy*m_gridSize + cx];

        for (k = 0; k < PDGridMaxSamplePerCell; k++) {
          if (cell[k] == -1) {
            break;
          }
          else {
            float d = SquareDistance (GetTiled (m_points[cell[k]] - pt));

            if (d < closestSqrd)
              closestSqrd = d;
          }
        }
      }
    }

    return sqrt (closestSqrd);
  }

  void PoissonDiskSampler::FindNeighborRanges (int index, RangeList *rl)
  {
    if (!m_grid) {
      printf ("Internal error, sampler cannot search without grid.\n");
      exit (1);
    }

    Vector2f &candidate = m_points[index];
    //no máximo, para ter uma interseção de círculos
    // a distância deve ser 4*raio
    float rangeSqrd = 4 * 4 * m_radius*m_radius;
    int i, j, k, gx, gy, N = (int)ceil (4 * m_radius / m_gridCellSize);
    if (N > (m_gridSize >> 1)) N = m_gridSize >> 1;

    GetGridXY (candidate, &gx, &gy);

    int xSide = (candidate.x - (-1 + gx*m_gridCellSize)) > m_gridCellSize*.5;
    int ySide = (candidate.y - (-1 + gy*m_gridCellSize)) > m_gridCellSize*.5;
    int iy = 1;
    for (j = -N; j <= N; j++) {
      int ix = 1;

      if (j == 0) iy = ySide;
      else if (j == 1) iy = 0;

      for (i = -N; i <= N; i++) {
        if (i == 0) ix = xSide;
        else if (i == 1) ix = 0;

        // offset to closest cell point
        float dx = candidate.x - (-1 + (gx + i + ix)*m_gridCellSize);
        float dy = candidate.y - (-1 + (gy + j + iy)*m_gridCellSize);

        if (dx*dx + dy*dy < rangeSqrd) {
          int cx = (gx + i + m_gridSize) % m_gridSize;
          int cy = (gy + j + m_gridSize) % m_gridSize;
          int *cell = m_grid[cy*m_gridSize + cx];

          for (k = 0; k < PDGridMaxSamplePerCell; k++) {
            if (cell[k] == -1) {
              break;
            }
            else if (cell[k] != index) {
              Vector2f &pt = m_points[cell[k]];
              Vector2f v = GetTiled (pt - candidate);
              float distSqrd = v.x*v.x + v.y*v.y;

              //encontrar os ângulos das interseções entre 2 círculos
              if (distSqrd < rangeSqrd) {
                float dist = sqrt (distSqrd);
                float angle = atan2 (v.y, v.x);
                float theta = acos (.25f*dist / m_radius);

                rl->subtract (angle - theta, angle + theta);
              }
            }
          }
        }
      }
    }
  }

  ///////////
  //Methods//
  ///////////
  DartThrowing::DartThrowing (float radius, bool isTiled, int minMaxThrows, int maxThrowsMult)
    : PoissonDiskSampler (radius, isTiled),
    m_minMaxThrows (minMaxThrows),
    m_maxThrowsMult (maxThrowsMult)
  {
    SamplerType = "DartThrowing";
  }

  void DartThrowing::Do ()
  {
    while (Step ());
  }

  bool DartThrowing::Step ()
  {
    int i, N = (int)m_points.size ()*m_maxThrowsMult;
    if (N < m_minMaxThrows) N = m_minMaxThrows;

    for (i = 0; i < N; i++)
    {
      Vector2f pt = RandomPoint ();

      FindNeighbors (pt, 2 * m_radius);

      if (!m_neighbors.size ())
      {
        AddPoint (pt);
        break;
      }
    }

    if (i >= N)
      return false;
    return true;
  }

  BestCandidate::BestCandidate (float radius, bool isTiled, int multiplier)
    : PoissonDiskSampler (radius, isTiled),
    m_multiplier (multiplier),
    m_N ((int)(.7 / (radius*radius)))
  {
    m_i_stepaux = 0;
    { SamplerType = "BestCandidate"; }
  }

  void BestCandidate::Do ()
  {
    m_i_stepaux = 0;
    while (Step ());
  }

  bool BestCandidate::Step ()
  {
    if (m_i_stepaux < m_N)
    {
      int i = m_i_stepaux;
      Vector2f best (0, 0);
      float bestDistance = 0;
      int count = 1 + (int)m_points.size ()*m_multiplier;

      for (int j = 0; j<count; j++) {
        Vector2f pt = RandomPoint ();
        float closest = 2;

        closest = FindClosestNeighbor (pt, 4 * m_radius);
        if (j == 0 || closest>bestDistance) {
          bestDistance = closest;
          best = pt;
        }
      }
      AddPoint (best);
      m_i_stepaux++;
      return true;
    }
    else
    {
      return false;
    }
  }

  PureSampler::PureSampler (float radius) : PoissonDiskSampler (radius, true) {}
  void PureSampler::Do ()
  {
    //gera um primeiro ponto e cria a região em volta do ponto
    Vector2f pt = RandomPoint();
    ScallopedRegion *rgn = new ScallopedRegion (pt, m_radius*2.f, m_radius*4.f);

    //inicializa um map com as regiões de cada ponto
    std::map<int, ScallopedRegion*> regions;
    
    //inicializa a árvore de probabilidades
    WeightedDiscretePDFi regionsPDF;

    //Adiciona o ponto no conjunto
    AddPoint (pt);
    //mapeia a região para a scalloped region recém gerada
    regions[(int)m_points.size () - 1] = rgn;
    //insere a região na árvore binária de pesos
    //prob = rgn->area / area_total
    regionsPDF.insert ((int)m_points.size () - 1, rgn->area);
  
    //Enquanto ainda houver alguma região 
    while (regions.size ())
    {
      //gera um float [0, 1] e escolhe uma das regiões
      // para gerar uma nova amostra
      int idx = regionsPDF.choose (frand ());

      //Gera uma amostra na região escolhida aleatoriamente
      pt = GetTiled (((*regions.find (idx)).second)->Sample ());
      
      //Gera a nova região do novo ponto que será adicionado
      rgn = new ScallopedRegion (pt, m_radius * 2, m_radius * 4);

      //Verifica a vizinhança em volta do ponto
      FindNeighbors (pt, m_radius * 8);

      //Passando por toda vizinhança encontrada
		  for (std::vector<int>::const_iterator it = m_neighbors.begin(); it != m_neighbors.end(); it++)
      {
        //Pega o índice de um vizinho
  			int nIdx = *it;
			  //Captura seu correspondente ponto
        Vector2f n = m_points[nIdx];
        //Realiza a subtração do anel do ponto vizinho
        rgn->SubtractDisk(pt+GetTiled(n-pt), m_radius*4);
      
			  //Pega a região do vizinho (caso este tenha uma região com área > 0)
        std::map<int, ScallopedRegion*>::iterator entry = regions.find (nIdx);
        if (entry != regions.end())
        {
				  ScallopedRegion *nRgn = (*entry).second;
				  //Subtrai o disco 
          //(deve-se retirar o disco com raio 2r tendo como 
          // centro o novo ponto adicionado, pois este ponto 
          // invalida uma área 2r do domínio)
          nRgn->SubtractDisk(n+GetTiled(pt-n), m_radius*2);

          //caso após a subtração a região demarcada é nula,
          // esta deve ser deletada do conjunto de regiões
				  if (nRgn->IsEmpty())
          {
					  regions.erase(entry);
					  regionsPDF.remove(nIdx);
					  delete nRgn;
				  }
          //caso contrário, apenas atualiza a região
          else 
          {
					  regionsPDF.update(nIdx, nRgn->area);
				  }
			  }
		  }

      //Adiciona o ponto no conjunto
		  AddPoint(pt);

      //Se a região anelar do ponto adicionado não for nula
		  // Quer dizer que sua probabilidade p > 0
      if (!rgn->IsEmpty())
      {
        //adiciona o ponto na árvore de probabilidades
        // e adiciona no mapeamento de regiões
			  regions[(int) m_points.size()-1] = rgn;
			  regionsPDF.insert((int) m_points.size()-1, rgn->area);
		  }
      // caso contrário, p == 0, apenas é deletado
      else
      {
			  delete rgn;
		  }
    }
  }

  bool PureSampler::Step () { return false; }
  
  LinearPureSampler::LinearPureSampler (float radius) : PoissonDiskSampler (radius, true) {}
  void LinearPureSampler::Do () 
  {
    std::vector<int> candidates;

    //Gera um primeiro ponto
    AddPoint (RandomPoint ());
    //Adiciona à lista de candidatos
    candidates.push_back ((int)m_points.size () - 1);

    while (candidates.size ()) 
    {
      //seleciona aleatoriamente um candidato
      // e retira o mesmo da lista atual de candidato
      int c = rand() % candidates.size ();
      int index = candidates[c];
      Vector2f candidate = m_points[index];
      candidates[c] = candidates[candidates.size () - 1];
      candidates.pop_back ();

      //cria a região anelar desse candidato
      ScallopedRegion sr (candidate, m_radius * 2, m_radius * 4);
      //Acha os vizinhos do ponto candidato
      FindNeighbors (candidate, m_radius * 8);

      //Pra cada vizinho encontrado...
      for (std::vector<int>::const_iterator it = m_neighbors.begin (); it != m_neighbors.end (); it++)
      {
        int nIdx = *it;
        Vector2f n = m_points[nIdx];
        Vector2f nClose = candidate + GetTiled (n - candidate);

        //verifica se o viziho foi adicionado antes ou depois
        //Se foi antes
        //A área do vizinho que foi adicionado antes
        // (2r a 4r e o próprio círculo 2r) deve invalidar a área
        // do candidato cujo ponto foi adicionado depois
        if (nIdx < index)
        {
          sr.SubtractDisk (nClose, m_radius * 4);
        }
        //Se o ponto candidato foi adicionado antes do ponto vizinho pego,
        // deve-se subtrair apenas a área 2r que o vizinho invalida por padrão
        else 
        {
          sr.SubtractDisk (nClose, m_radius * 2);
        }
      }

      //Vai gerando amostras dentro da área anelar do candidato
      // até a área ser preenchida, ou seja, até não haver mais 
      // suficiente para gerar um novo ponto
      while (!sr.IsEmpty ())
      {
        Vector2f p = sr.Sample ();
        Vector2f pt = GetTiled (p);

        AddPoint (pt);
        candidates.push_back ((int)m_points.size () - 1);

        sr.SubtractDisk (p, m_radius * 2);
      }
    }
  }

  bool LinearPureSampler::Step () { return false; }

  BoundarySampler::BoundarySampler (float radius, bool isTiled)
    : PoissonDiskSampler (radius, isTiled), m_rl (NULL)
  {
    m_candidates = std::vector<int> ();
    m_candidates.clear ();
    SamplerType = "BoundarySampler";
  }

  BoundarySampler::~BoundarySampler ()
  {
    delete m_rl;
  }

  void BoundarySampler::Do ()
  {
    while (Step ());
    /*//Range inicial 0~0, ou seja, área = 0
    RangeList rl (0, 0);
    //vetor baseados nos índices dos pontos dentro do vetor m_points
    std::vector<int> candidates;

    //Adiciona o primeiro ponto
    AddPoint (RandomPoint ());
    //Adiciona este ponto como um novo candidato
    candidates.push_back ((int)m_points.size () - 1);

    //Enquanto ainda houver candidatos para que sejam adicionados pontos
    while (candidates.size ())
    {
    //randomiza um candidato dentre os pontos
    int c = rand () % candidates.size ();
    int index = candidates[c];

    //pega o ponto que foi selecionado
    Vector2f candidate = m_points[index];

    //retira esse candidato escolhido
    candidates[c] = candidates[candidates.size () - 1];
    candidates.pop_back ();

    //reseta o RangeList
    rl.reset (0, (float)PI * 2.0f);
    //Faz vários subtracts para gerar todos os ranges
    FindNeighborRanges (index, &rl);

    //Enquanto houver algum lugar para ser adicionado um ponto
    // na região 2*r do candidato selecionado
    while (rl.numRanges)
    {
    //Randomiza um ângulo pega um valor de ângulo
    RangeEntry &re = rl.ranges[rand () % rl.numRanges];
    float angle = re.min + (re.max - re.min)*frand ();

    //Calcula o ponto que deve ser adicionado
    Vector2f pt = GetTiled (Vector2f (candidate.x + cos (angle) * 2 * m_radius,
    candidate.y + sin (angle) * 2 * m_radius));

    AddPoint (pt);

    //adiciona este ponto adicionado como um novo candidato
    candidates.push_back ((int)m_points.size () - 1);

    //retira a parte do ângulo utilizada para adicionar este ponto
    rl.subtract (angle - (float)PI / 3.0f, angle + (float)PI / 3.0f);
    }
    }*/
  }

  bool BoundarySampler::Step ()
  {
    if (m_points.empty ())
    {
      //Range inicial 0~0, ou seja, área = 0
      m_rl = new RangeList (0, 0);
      //vetor baseados nos índices dos pontos dentro do vetor m_points
      m_candidates.clear ();

      //Adiciona o primeiro ponto
      AddPoint (RandomPoint ());
      //Adiciona este ponto como um novo candidato
      m_candidates.push_back ((int)m_points.size () - 1);
      m_step = 1;
      return true;
    }
    //Enquanto ainda houver candidatos para que sejam adicionados pontos
    if (m_candidates.size () && m_step == 1)
    {
      //randomiza um candidato dentre os pontos
      int c = rand () % m_candidates.size ();
      int index = m_candidates[c];

      //pega o ponto que foi selecionado
      m_current_candidate = m_points[index];

      //retira esse candidato escolhido
      m_candidates[c] = m_candidates[m_candidates.size () - 1];
      m_candidates.pop_back ();

      //reseta o RangeList
      m_rl->reset (0, (float)PI * 2.0f);
      //Faz vários subtracts para gerar todos os ranges
      FindNeighborRanges (index, m_rl);
      m_step = 2;
      return true;
    }
    else if (m_step == 2)
    {
      //Enquanto houver algum lugar para ser adicionado um ponto
      // na região 2*r do candidato selecionado
      while (m_rl->numRanges)
      {
        //Randomiza um ângulo pega um valor de ângulo
        RangeEntry &re = m_rl->ranges[rand () % m_rl->numRanges];
        float angle = re.min + (re.max - re.min)*frand ();

        //Calcula o ponto que deve ser adicionado
        Vector2f pt = GetTiled (Vector2f (m_current_candidate.x + cos (angle) * 2 * m_radius,
          m_current_candidate.y + sin (angle) * 2 * m_radius));

        AddPoint (pt);

        //adiciona este ponto adicionado como um novo candidato
        m_candidates.push_back ((int)m_points.size () - 1);

        //retira a parte do ângulo utilizada para adicionar este ponto
        // ângulo que será occludido
        m_rl->subtract (angle - (float)PI / 3.0f, angle + (float)PI / 3.0f);
      }
      m_step = 1;
      return true;
    }
    return false;
  }

  PenroseSampler::PenroseSampler (float radius) : PoissonDiskSampler (radius, true) {}
  void PenroseSampler::Do () {}
  bool PenroseSampler::Step () { return false; }
  UniformSampler::UniformSampler (float radius) : PoissonDiskSampler (radius, true) {}
  void UniformSampler::Do () {}
  bool UniformSampler::Step () { return false; }
}