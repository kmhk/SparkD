
/*!
 * \file	TemplateFactory.h
 * \ingroup base
 * \brief	
 * \author	
 */

#pragma once

#include "Template.h"
#include "cvlibbaseDef.h"
#include "StringArray.h"
#include "PtrArray.h"

namespace cvlib
{

	template<class TYPE>
	class CVLIB_DECLSPEC FactoryTemplate
	{
	public:
		FactoryTemplate() { m_nActiveIdx = -1; m_nDefaultNum = 0; }
		virtual ~FactoryTemplate() { release(); }

		virtual void	init() {}
		virtual void	release() {};
		//	virtual	int		find (TYPE* pPrototype){return -1;}

		int				find(const String& szID) const {
			String temp = szID;
			int npos = szID.find('_', 0);
			if (npos != -1)
				temp = szID.left(npos);
			int i;
			for (i = 0; i < m_PrototypeNames.getSize(); i++)
			{
				if (temp == m_PrototypeNames[i])
					break;
			}
			if (i == m_PrototypeNames.getSize())return -1;
			return i;
		}
		TYPE*			get(const String& szID) { return getPrototype(find(szID)); }
		void			setCurSel(int nIdx) { if (nIdx < 0 || nIdx >= m_PrototypeNames.getSize()) return; m_nActiveIdx = nIdx; }
		int				setCurSel() const { return m_nActiveIdx; }
		String			getName(int nIdx) const {
			if (nIdx < 0 || nIdx >= m_PrototypeNames.getSize()) return "";
			else return m_PrototypeNames[nIdx];
		}
		String			getCurrName() const {
			if (m_nActiveIdx < 0 || m_nActiveIdx >= m_PrototypeNames.getSize()) return "";
			else return m_PrototypeNames[m_nActiveIdx];
		}
		int				count() const { return m_Prototypes.getSize(); }

		void			add(TYPE* pPrototype, const String& szID) {
			m_PrototypeNames.add(szID);
			m_Prototypes.add(pPrototype);
		}
		TYPE*			getPrototype(int nIdx) {
			if (nIdx < 0 || nIdx >= m_Prototypes.getSize()) return NULL;
			else return (TYPE*)m_Prototypes[nIdx];
		}
		TYPE*			getCurrPrototype() {
			if (m_nActiveIdx < 0 || m_nActiveIdx >= m_Prototypes.getSize()) return NULL;
			else return (TYPE*)m_Prototypes[m_nActiveIdx];
		}
		int toFile(const String& sz) const
		{
			FILE* pfile = fopen(sz, "w");
			if (pfile)
			{
				for (int i = 0; i < m_PrototypeNames.getSize(); i++)
					fprintf(pfile, "%s\n", (const char*)m_PrototypeNames[i]);
				fclose(pfile);
			}
			return 1;
		}
	protected:
		StringArray		m_PrototypeNames;
		int				m_nDefaultNum;
		int				m_nActiveIdx;
		PtrArray		m_Prototypes;
	};

}