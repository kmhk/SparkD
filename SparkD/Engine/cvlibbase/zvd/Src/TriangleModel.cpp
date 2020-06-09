#include "TriangleModel.h"

namespace cvlib
{

	TriangleModel::TriangleModel()
	{
		m_pEdges = NULL;
		m_nNum = NULL;
	}

	TriangleModel::~TriangleModel()
	{
		release();
	}

	int	TriangleModel::load(FILE* pFile)
	{
		if (fread(&m_nNum, sizeof(m_nNum), 1, pFile) != 1)
			return 0;
		create(m_nNum);
		if (fread(m_pEdges, sizeof(m_pEdges[0]), m_nNum, pFile) != (size_t)m_nNum)
			return 0;
		generateTriangles();
		return 1;
	}

	int	TriangleModel::Save(FILE* pFile)
	{
		fwrite(&m_nNum, sizeof(m_nNum), 1, pFile);
		fwrite(m_pEdges, sizeof(m_pEdges[0]), m_nNum, pFile);
		return 1;
	}

	int	TriangleModel::create(int nNum)
	{
		m_nNum = nNum;
		m_pEdges = new SEdge[m_nNum];
		return 1;
	}

	void	TriangleModel::release()
	{
		if (m_pEdges)
			delete[]m_pEdges;
		m_pEdges = NULL;

		m_TriangleList.removeAllTriangle();
		m_nNum = 0;
	}

	void	TriangleModel::generateTriangles()
	{
		m_TriangleList.removeAllTriangle();

		int	nTotalNum = 0;
		int i;
		for (i = 0; i < m_nNum; i++)
		{
			nTotalNum = MAX(m_pEdges[i].nFrom, nTotalNum);
			nTotalNum = MAX(m_pEdges[i].nTo, nTotalNum);
		}
		nTotalNum++;

		int	nNeighborNum = 0;
		int* pnNeighborVertexs;
		pnNeighborVertexs = new int[m_nNum];
		for (int iVertex = 0; iVertex < nTotalNum; iVertex++)
		{
			nNeighborNum = 0;
			for (i = 0; i < m_nNum; i++)
			{
				if (m_pEdges[i].nFrom == iVertex)
				{
					int k;
					for (k = 0; k < nNeighborNum; k++)
					{
						if (pnNeighborVertexs[k] == m_pEdges[i].nTo)
							break;
					}
					if (k == nNeighborNum)
					{
						pnNeighborVertexs[nNeighborNum] = m_pEdges[i].nTo;
						nNeighborNum++;
					}
				}
				else if (m_pEdges[i].nTo == iVertex)
				{
					int k;
					for (k = 0; k < nNeighborNum; k++)
					{
						if (pnNeighborVertexs[k] == m_pEdges[i].nFrom)
							break;
					}
					if (k == nNeighborNum)
					{
						pnNeighborVertexs[nNeighborNum] = m_pEdges[i].nFrom;
						nNeighborNum++;
					}
				}
			}
			for (i = 0; i < nNeighborNum - 1; i++)
			{
				for (int k = i + 1; k < nNeighborNum; k++)
				{
					if (findEdge(pnNeighborVertexs[i], pnNeighborVertexs[k]))
					{
						int nFind = m_TriangleList.findTriangle(iVertex, pnNeighborVertexs[i], pnNeighborVertexs[k]);
						if (nFind == 0)
						{
							Triangle* pTriangle = new Triangle;
							pTriangle->n1 = iVertex;
							pTriangle->n2 = pnNeighborVertexs[i];
							pTriangle->n3 = pnNeighborVertexs[k];
							m_TriangleList.add(pTriangle);
						}
					}
				}
			}
		}
		delete[](pnNeighborVertexs);
	}

	int TriangleModel::findEdge(int nFrom, int nTo)
	{
		int i;
		for (i = 0; i < m_nNum; i++)
		{
			if (m_pEdges[i].nFrom == nFrom && m_pEdges[i].nTo == nTo)
				break;
			if (m_pEdges[i].nTo == nFrom && m_pEdges[i].nFrom == nTo)
				break;
		}
		if (i == m_nNum)
			return 0;
		else
			return 1;
	}

	void	TriangleModel::adjacentTriangles(int nVertex, TriangleList* pList)
	{
		pList->removeAll();
		for (int i = 0; i < m_TriangleList.getSize(); i++)
		{
			Triangle* pTriangle = (Triangle*)m_TriangleList.getAt(i);
			if (pTriangle->n1 == nVertex ||
				pTriangle->n2 == nVertex ||
				pTriangle->n3 == nVertex)
			{
				pList->add(pTriangle);
			}
		}
	}

	int	TriangleModel::loadTriangleList(FILE* pFile)
	{
		int nNum;
		fread(&nNum, sizeof(nNum), 1, pFile);
		for (int i = 0; i < nNum; i++)
		{
			Triangle* pTriangle = new Triangle;
			fread(pTriangle, sizeof(Triangle), 1, pFile);
			m_TriangleList.add(pTriangle);
		}
		m_pEdges = NULL;
		m_nNum = 0;
		return 1;
	}
	int	TriangleModel::loadTriangleList(XFile* pFile)
	{
		int nNum;
		pFile->read(&nNum, sizeof(nNum), 1);
		for (int i = 0; i < nNum; i++)
		{
			Triangle* pTriangle = new Triangle;
			pFile->read(pTriangle, sizeof(Triangle), 1);
			m_TriangleList.add(pTriangle);
		}
		m_pEdges = NULL;
		m_nNum = 0;
		return 1;
	}
	int	TriangleModel::saveTriangleList(FILE* pFile)
	{
		int nNum = m_TriangleList.getSize();
		fwrite(&nNum, sizeof(nNum), 1, pFile);
		for (int i = 0; i < nNum; i++)
		{
			Triangle* pTriangle = (Triangle*)m_TriangleList.getAt(i);
			fwrite(pTriangle, sizeof(Triangle), 1, pFile);
		}
		return 1;
	}


}
