/*! 
 * \file    IniFile.h
 * \ingroup base
 * \brief   ini.
 * \author 
 */

#pragma once

// C++ Includes
#include "StringArray.h"
#include "String.hpp"
#include "XFileDisk.h"
#include "PtrArray.h"

#define MAX_KEYNAME    128
#define MAX_VALUENAME  128
#define MAX_VALUEDATA 2048

namespace cvlib
{
	/**
	 * @brief ini
	 */
	class CVLIB_DECLSPEC IniFile
	{
	private:
		bool   m_fCaseInsensitive;
		String m_szPath;

		class CVLIB_DECLSPEC Key
		{
		public:
			Key();
			Key(const Key& key);
			virtual ~Key();

			StringArray	names;
			StringArray values;
			StringArray comments;
		};

		PtrArray	m_Keys;
		StringArray m_Names;
		StringArray m_Comments;

		String checkCase(String s) const;

	public:
		enum errors
		{
			noID = -1
		};

		IniFile(const String iniPath = "");
		virtual ~IniFile()
		{
			reset();
		}

		// Sets whether or not keynames and valuenames should be case sensitive.
		// The default is case insensitive.

		/**
		 * @brief   match case
		 *
		 * @return  void
		 *
		 */
		void caseSensitive()
		{
			m_fCaseInsensitive = false;
		}
		/**
		 * @brief   match case.
		 *
		 * @return  void
		 *
		 */
		void caseInsensitive()
		{
			m_fCaseInsensitive = true;
		}

		// Sets m_szPath of ini file to read and write from.
		/**
		 * @brief   ini.
		 *
		 * @param   newPath [in] :
		 * @return  void
		 *
		 */
		void path(const String newPath)
		{
			m_szPath = newPath;
		}

		/**
		 * @brief   ini.
		 *
		 * @return
		 *
		 */

		String path()
		{
			return m_szPath;
		}

		/**
		 * @brief   ini.
		 *
		 * @param   newPath [in] :
		 * @return  void
		 *
		 */
		void setPath(const String newPath)
		{
			path(newPath);
		}

		bool readFile();
		bool writeFile() const;

		// Deletes all stored ini data.
		void erase();
		void clear()
		{
			erase();
		}
		void reset()
		{
			erase();
		}
		long findKey(String keyname) const;
		long findValue(unsigned const keyID, String valuename) const;
		unsigned int numKeys() const
		{
			return m_Names.getSize();
		}
		unsigned int getNumKeys() const
		{
			return numKeys();
		}
		unsigned int addKeyName(String  keyname);
		String keyName(unsigned const keyID) const;
		String getKeyName(unsigned const keyID) const { return keyName(keyID); }
		unsigned int numValues(unsigned const keyID) const;
		unsigned int getNumValues(unsigned const keyID) const { return numValues(keyID); }
		unsigned int numValues(String keyname) const;
		unsigned int getNumValues(String keyname) const { return numValues(keyname); }

		// Returns value name by index for a given keyname or keyID.

		String valueName(unsigned const keyID, unsigned const valueID) const;
		String getValueName(unsigned const keyID, unsigned const valueID) const
		{
			return valueName(keyID, valueID);
		}
		String valueName(String keyname, unsigned const valueID) const;
		String getValueName(String keyname, unsigned const valueID) const
		{
			return valueName(keyname, valueID);
		}

		// Gets value of [keyname] valuename =.
		// Overloaded to return String, int, and double.
		// Returns defValue if Key/value not found.
		String getValue(unsigned const keyID, unsigned const valueID, String defValue = "") const;
		String getValue(String keyname, String valuename, String defValue = "") const;
		int    getValueI(String keyname, String valuename, int const defValue = 0) const;
		bool   getValueB(String keyname, String valuename, bool const defValue = false) const
		{
			int dd;
			if (defValue)
				dd = 1;
			else
				dd = 0;
			int res = getValueI(keyname, valuename, dd);
			if (res == 0)
				return false;
			else
				return true;
		}

		double   getValueF(String keyname, String valuename, double const defValue = 0.0) const;
		// This is a variable length formatted getValue routine. All these voids
		// are required because there is no vsscanf() like there is a vsprintf().
		// Only a maximum of 8 variable can be read.

		unsigned int getValueV(String keyname, String valuename, char *format,
			void *v1 = 0, void *v2 = 0, void *v3 = 0, void *v4 = 0,
			void *v5 = 0, void *v6 = 0, void *v7 = 0, void *v8 = 0,
			void *v9 = 0, void *v10 = 0, void *v11 = 0, void *v12 = 0,
			void *v13 = 0, void *v14 = 0, void *v15 = 0, void *v16 = 0);

		// Sets value of [keyname] valuename =.
		// Specify the optional paramter as false (0) if you do not want it to create
		// the Key if it doesn't exist. Returns true if data entered, false otherwise.
		// Overloaded to accept String, int, and double.
		bool setValue(unsigned const keyID, unsigned const valueID, String const value);
		bool setValue(String keyname, String valuename, String value, bool const create = true);
		bool setValueI(String keyname, String valuename, int const value, bool const create = true);
		bool setValueB(String keyname, String valuename, bool const value, bool const create = true)
		{
			return setValueI(keyname, valuename, int(value), create);
		}
		bool setValueF(String keyname, String valuename, double const value, bool const create = true);
		bool setValueV(String keyname, String valuename, char *format, ...);
		bool deleteValue(String keyname, String valuename);
		bool deleteKey(String keyname);

		// Header comment functions.
		// Header m_Comments are those m_Comments before the first Key.
		//
		// Number of header m_Comments.
		unsigned numHeaderComments()
		{
			return m_Comments.getSize();
		}
		void     headerComment(String comment);
		String   headerComment(unsigned const commentID) const;
		bool     deleteHeaderComment(unsigned commentID);
		void     deleteHeaderComments()
		{
			m_Comments.removeAll();
		}

		// Key comment functions.
		// Key m_Comments are those m_Comments within a Key. Any m_Comments
		// defined within value m_Names will be added to this list. Therefore,
		// these m_Comments will be moved to the top of the Key definition when
		// the IniFile::writeFile() is called.
		//
		unsigned int numKeyComments(unsigned const keyID) const;
		unsigned int numKeyComments(String keyname) const;
		bool     keyComment(unsigned const keyID, String comment);
		bool     keyComment(String keyname, String comment);
		String   keyComment(unsigned const keyID, unsigned const commentID) const;
		String   keyComment(String  keyname, unsigned const commentID) const;
		bool     deleteKeyComment(unsigned keyID, unsigned const commentID);
		bool     deleteKeyComment(String keyname, unsigned const commentID);
		bool     deleteKeyComments(unsigned const keyID);
		bool     deleteKeyComments(String keyname);
	};

	/**
		@memo     Simple lo-fi property reader.

		@doc      Simple lo-fi property reader. Used as naive parser/scanner
					for the .asf, acf files et cetera.
	*/
	class CVLIB_DECLSPEC PropsReader
	{
		FILE *fh;
		void SkipRestOfLine();
		void SkipComments();
		void SkipWhiteSpace();
		/*const*/ char CR;
		/*const*/ char LF;
		/*const*/ char COMMENT_CHAR;

	public:

		PropsReader(const String &filename);
		~PropsReader();
		bool moreNonWhiteSpaceOnLine();

		/// Skips whitespace and any commments preceeding the current file position.
		void sync() { SkipWhiteSpace(); SkipComments(); }

		/// Returns true if the file is valid.
		bool isValid() { return fh != NULL; }

		/// Current open file.
		FILE *FH() { return fh; }
	};

	class CVLIB_DECLSPEC TokenFile : public XFileDisk
	{
	public:
		virtual void writeString(const char* lpsz);
		virtual char* readString(char* lpsz, int nMax);
		virtual bool readString(String& rString);
		virtual bool nextToken(String& tok);
		TokenFile();
		virtual ~TokenFile();
	private:
		String m_CurString;
		String m_Delimiter;
	};

}