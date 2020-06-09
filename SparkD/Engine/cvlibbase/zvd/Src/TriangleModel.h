#pragma once

#include "Template.h"
#include "XFile.h"

namespace cvlib {

	struct Triangle
	{
		int n1;
		int n2;
		int n3;
	};

	class CVLIB_DECLSPEC TriangleModel
	{
	public:
		TriangleModel();
		virtual ~TriangleModel();

		struct SEdge
		{
			int nFrom;
			int nTo;
		} *m_pEdges;

		int		m_nNum;
	public:
		class CVLIB_DECLSPEC TriangleList : public Vector<Triangle*>
		{
		public:
			int findTriangle(int n1, int n2, int n3)
			{
				int i;
				for (i = 0; i < getSize(); i++)
				{
					Triangle* pTriangle = (Triangle*)getAt(i);
					if (pTriangle->n1 == n1 && pTriangle->n2 == n2 && pTriangle->n3 == n3)
						break;
					if (pTriangle->n1 == n1 && pTriangle->n2 == n3 && pTriangle->n3 == n2)
						break;
					if (pTriangle->n1 == n2 && pTriangle->n2 == n1 && pTriangle->n3 == n3)
						break;
					if (pTriangle->n1 == n2 && pTriangle->n2 == n3 && pTriangle->n3 == n1)
						break;
					if (pTriangle->n1 == n3 && pTriangle->n2 == n1 && pTriangle->n3 == n2)
						break;
					if (pTriangle->n1 == n3 && pTriangle->n2 == n2 && pTriangle->n3 == n1)
						break;
				}
				if (i == getSize())
					return 0;
				else
					return 1;
			}
			void removeAllTriangle()
			{
				for (int i = 0; i < getSize(); i++)
				{
					delete (Triangle*)getAt(i);
				}
				removeAll();
			}
		};
		TriangleList	m_TriangleList;

		int		load(FILE* pFile);
		int		Save(FILE* pFile);
		int		create(int nNum);
		void	release();

		void	generateTriangles();
		void	adjacentTriangles(int nVertex, TriangleList* pList);

		int		loadTriangleList(FILE* pFile);
		int		loadTriangleList(XFile* pFile);
		int		saveTriangleList(FILE* pFile);

	protected:
		int		findEdge(int nFrom, int nTo);
	};

}
