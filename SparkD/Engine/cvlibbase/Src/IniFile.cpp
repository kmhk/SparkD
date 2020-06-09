/*! 
 * \file IniFile.h
 * \ingroup 
 * \brief 
 * \author
 */

#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include "IniFile.h"

static char szLF[3] = {0x0d, 0x0a, 0x00};

namespace cvlib
{

IniFile::Key::Key ()
{
}

IniFile::Key::Key (const Key& key)
{
	names.append(key.names);
	values.append(key.values);
	comments.append(key.comments);
}

IniFile::Key::~Key()
{
	names.removeAll();
	values.removeAll();
	comments.removeAll();
}

IniFile::IniFile( String iniPath)
{
	path( iniPath);
	m_fCaseInsensitive = true;
}

bool IniFile::readFile()
{
	// Normally you would use ifstream, but the SGI CC compiler has
	// a few bugs with ifstream. So ... fstream used.
	FILE* pFile;
	char szTemp1[MAX_VALUEDATA];
	String	szLine;
	String   szKeyname, szValuename, szValue;

	pFile = fopen( m_szPath.str(), "rb");
	if ( pFile == NULL)
		return false;
 
	bool fSuccess = true;
	while( fgets( szTemp1, MAX_VALUEDATA, pFile))
	{
		szLine =szTemp1;
		// To be compatible with Win32, check for existence of '\r'.
		// Win32 files have the '\r' and Unix files don't at the end of a szLine.
		// Note that the '\r' will be written to INI files from
		// Unix so that the created INI file can be read under Win32
		// without change.
		if ( szLine[szLine.length() - 1] == '\r')
			szLine = szLine.mid( 0, szLine.length() - 1);
		else if ( szLine[szLine.length() - 1] == '\n')
			szLine = szLine.mid( 0, szLine.length() - 2);
    
		if ( szLine.length())
		{
			// parsing by state transition model
			int i;
			for (i = 0; i < szLine.length();)
			{
				if (szLine[i] == ' ' || szLine[i] == '\t')
					i ++;
				else
					break;
			}
			szLine = szLine.right(szLine.length() - i);

			int nStrLen = szLine.length();
			if (nStrLen == 0)
				goto END;
			i = 0;
			char ch;
			ch = szLine[i];
			if (ch == '[')
			{
				i ++;
				goto A;
			}
			else if (ch == '#' || ch == ';')
			{
				if ( !m_Names.getSize())
					headerComment( szLine.mid(i + 1));
				else
					keyComment( szKeyname, szLine.mid(i + 1));

				goto END;
			}
			else
			{
				{
					String szTemp = szLine.mid(i);
					//int nRight = szTemp.findOneOf(" \t;#[]'\\,.<>?:\"{}=!@$%^&*()_+-=/*-+`~");
					int nRight = szTemp.findOneOf(" \t;#[]'\\,.<>?:\"{}=!@$%^&*()+-=/*-+`~");
					if (nRight != -1 && 0 < nRight)
					{
						szValuename = szLine.mid( 0, nRight);
						i += nRight;
					}
					else 
					{
						szTemp.empty();
						goto ERR;
					}
				}
				goto D;
			}

A:
			if (i < nStrLen)
				ch = szLine[i];
			else
				goto ERR;
			if (ch == '\t' || ch == ' ')
			{
				i ++;
				goto A;
			}
			
			{
				String szTemp = szLine.mid(i);
//				int nRight = szTemp.findOneOf(" \t;#[]'\\,.<>?:\"{}=!@$%^&*()_+-=/*-+`~");
				int nRight = szTemp.findOneOf(" \t;#[]'\\,.<>?:\"{}=!@$%^&*()+-=/*-+`~");
				if (nRight != -1 && 0 < nRight)
				{
					szKeyname = szTemp.mid(0, nRight);
					addKeyName( szKeyname);
					i += nRight;
				}
				else
				{
					szTemp.empty();
					goto ERR;
				}
			}
B:
			if (i < nStrLen)
				ch = szLine[i];
			else
				goto ERR;
			if (ch == '\t' || ch == ' ')
			{
				i ++;
				goto B;
			}
			else if (ch == ']')
				i ++;
C:
			if (i < nStrLen)
				ch = szLine[i];
			else
				goto END;
			if (ch == '\t' || ch == ' ')
			{
				i ++;
				goto C;
			}
			else
				goto ERR;
D:
			if (i < nStrLen)
				ch = szLine[i];
			else
				goto ERR;
			if (ch == '\t' || ch == ' ')
			{
				i ++;
				goto D;
			}
			else if (ch == '=')
				i ++;
E:
			if (i < nStrLen)
			{
				ch = szLine[i];
				if (ch == '\t' || ch == ' ')
				{
					i ++;
					goto E;
				}
			}
			else if (i > nStrLen)
				goto ERR;

			// RËÏ±¨
			{
				String szTemp = szLine.mid(i);
				int nRight = szTemp.findOneOf(" \t;#");
				if (nRight != -1 && 0 < nRight)
				{
					szValue = szTemp.mid(0, nRight);
					setValue( szKeyname, szValuename, szValue);
					i += nRight;
				}
				else
				{
					setValue( szKeyname, szValuename, szTemp);
					i = szLine.length();
				}
			}
F:
			if (i < nStrLen)
				ch = szLine[i];
			else
				goto END;
			if (ch == '\t' || ch == ' ')
			{
				i ++;
				goto F;
			}
			else
			{
				goto ERR;
			}

ERR:
			fSuccess = false;
			break;

END:
			;
		}
	}

	fclose(pFile);
	if (fSuccess == false)
	{
		m_Keys.removeAll();
		m_Names.removeAll();
		m_Comments.removeAll();
		m_szPath.empty();
		return false;
	}
	if ( m_Names.getSize())
		return true;
	return false;
}

bool IniFile::writeFile() const
{
	int nCommentID, nKeyID, nValueID;
	// Normally you would use ofstream, but the SGI CC compiler has
	// a few bugs with ofstream. So ... fstream used.
	FILE* pFile;

	pFile = fopen( m_szPath, "wb");
	if ( pFile == NULL)
		return false;

	// write header m_Comments.
	for ( nCommentID = 0; nCommentID < m_Comments.getSize(); ++nCommentID)
		fprintf (pFile, ";%s%s", (const char*)m_Comments[nCommentID], szLF);
	if ( m_Comments.getSize())
		fprintf (pFile, "%s", szLF);

	// write m_Keys and values.
	for ( nKeyID = 0; nKeyID < m_Keys.getSize(); ++nKeyID)
	{
		fprintf (pFile, "[%s]%s", (const char*)m_Names[nKeyID], szLF);
		// Comments.
		for ( nCommentID = 0; nCommentID < ((Key*)m_Keys[nKeyID])->comments.getSize(); ++nCommentID)
			fprintf (pFile, ";%s%s", (const char*)((Key*)m_Keys[nKeyID])->comments[nCommentID], szLF);
		// Values.
		for ( nValueID = 0; nValueID < ((Key*)m_Keys[nKeyID])->names.getSize(); ++nValueID)
			fprintf (pFile, "%s=%s%s", (const char*)((Key*)m_Keys[nKeyID])->names[nValueID], (const char*)((Key*)m_Keys[nKeyID])->values[nValueID], szLF);
		fprintf(pFile, "%s", szLF);
	}
	fclose(pFile);

	return true;
}

long IniFile::findKey( String keyname) const
{
	for ( int keyID = 0; keyID < m_Names.getSize(); ++keyID)
		if ( checkCase( m_Names[keyID]) == checkCase( keyname))
			return long(keyID);
	return noID;
}

long IniFile::findValue( unsigned const keyID, String valuename) const
{
	if ( !m_Keys.getSize() || keyID >= (unsigned)m_Keys.getSize())
		return noID;

	for ( int valueID = 0; valueID < ((Key*)m_Keys[keyID])->names.getSize(); ++valueID)
	{
		StringArray& allnames = ((Key*)m_Keys[keyID])->names;
		if ( checkCase( allnames[valueID]) == checkCase( valuename))
			return long(valueID);
	}
	return noID;
}

unsigned IniFile::addKeyName( String keyname)
{
	m_Names.add( keyname);
	m_Keys.setSize (m_Keys.getSize() + 1);
	m_Keys[m_Keys.getSize()-1]=new Key;
	return m_Names.getSize() - 1;
}

String IniFile::keyName( unsigned const keyID) const
{
	if ( keyID < (unsigned)m_Names.getSize())
		return m_Names[keyID];
	else
		return String("");
}

unsigned IniFile::numValues( unsigned const keyID) const
{
	if ( keyID < (unsigned)m_Keys.getSize())
		return ((Key*)m_Keys[keyID])->names.getSize();
	return 0;
}

unsigned IniFile::numValues( String keyname) const
{
	long keyID = findKey( keyname);
	if ( keyID == noID)
		return 0;
	return (unsigned)((Key*)m_Keys[(int)keyID])->names.getSize();
}

String IniFile::valueName( unsigned const keyID, unsigned const valueID) const
{
	if ( keyID < (unsigned)m_Keys.getSize() && valueID < (unsigned)((Key*)m_Keys[keyID])->names.getSize())
		return ((Key*)m_Keys[keyID])->names[valueID];
	return String("");
}

String IniFile::valueName( String keyname, unsigned const valueID) const
{
	long keyID = findKey( keyname);
	if ( keyID == noID)
		return String("");
	return valueName( (unsigned)keyID, valueID);
}

bool IniFile::setValue( unsigned const keyID, unsigned const valueID, String szValue)
{
	if ( keyID < (unsigned)m_Keys.getSize() && valueID < (unsigned)((Key*)m_Keys[keyID])->names.getSize())
		((Key*)m_Keys[keyID])->values[valueID] = szValue;

	return false;
}

bool IniFile::setValue( String keyname, String valuename, String value, bool const create)
{
	long keyID = findKey( keyname);
	if ( keyID == noID)
	{
		if ( create)
			keyID = long( addKeyName( keyname));
		else
			return false;
	}

	long valueID = findValue( unsigned(keyID), valuename);
	if ( valueID == noID) 
	{
		if ( !create)
			return false;
		((Key*)m_Keys[(int)keyID])->names.add( valuename);
		((Key*)m_Keys[(int)keyID])->values.add( value);
	} 
	else
		((Key*)m_Keys[(int)keyID])->values[(int)valueID] = value;

	return true;
}

bool IniFile::setValueI( String keyname, String valuename, int const value, bool const /*create*/)
{
	char svalue[MAX_VALUEDATA];

	sprintf( svalue, "%d", value);
	return setValue( keyname, valuename, String(svalue));
}

bool IniFile::setValueF( String keyname, String valuename, double const value, bool const /*create*/)
{
	char svalue[MAX_VALUEDATA];

	sprintf( svalue, "%f", value);
	return setValue( keyname, valuename, String(svalue));
}

bool IniFile::setValueV( String keyname, String valuename, char *format, ...)
{
	va_list args;
	char value[MAX_VALUEDATA];

	va_start( args, format);
	vsprintf( value, format, args);
	va_end( args);
	return setValue( keyname, valuename, String(value));
}

String IniFile::getValue( unsigned const keyID, unsigned const valueID, String defValue) const
{
	if ( keyID < (unsigned)m_Keys.getSize() && valueID < (unsigned)((Key*)m_Keys[keyID])->names.getSize())
		return ((Key*)m_Keys[keyID])->values[valueID];
	return defValue;
}

String IniFile::getValue( String keyname, String valuename, String defValue) const
{
	long keyID = findKey( keyname);
	if ( keyID == noID)
		return defValue;

	long valueID = findValue( unsigned(keyID), valuename);
	if ( valueID == noID)
		return defValue;

	return ((Key*)m_Keys[(int)keyID])->values[(int)valueID];
}

int IniFile::getValueI(String keyname, String valuename, int const defValue) const
{
	char svalue[MAX_VALUEDATA];

	sprintf( svalue, "%d", defValue);
	return atoi( getValue( keyname, valuename, String(svalue)).str()); 
}

double IniFile::getValueF(String keyname, String valuename, double const defValue) const
{
	char svalue[MAX_VALUEDATA];

	sprintf( svalue, "%f", defValue);
	return atof( getValue( keyname, valuename, String(svalue)).str()); 
}

// 16 variables may be a bit of over kill, but hey, it's only code.
unsigned IniFile::getValueV( String keyname, String valuename, char *format,
			      void *v1, void *v2, void *v3, void *v4,
  			      void *v5, void *v6, void *v7, void *v8,
  			      void *v9, void *v10, void *v11, void *v12,
  			      void *v13, void *v14, void *v15, void *v16)
{
	String   value;
	// va_list  args;
	unsigned nVals;

	String str = getValue( keyname, valuename);
	value = str;
	if ( !value.length())
		return false;
	// Why is there not vsscanf() function. Linux man pages say that there is
	// but no compiler I've seen has it defined. Bummer!
	//
	// va_start( args, format);
	// nVals = vsscanf( value.c_str(), format, args);
	// va_end( args);

	nVals = sscanf( value.str(), format,
		v1, v2, v3, v4, v5, v6, v7, v8,
		v9, v10, v11, v12, v13, v14, v15, v16);

	return nVals;
}

bool IniFile::deleteValue( String keyname, String valuename)
{
	long keyID = findKey( keyname);
	if ( keyID == noID)
		return false;

	long valueID = findValue( unsigned(keyID), valuename);
	if ( valueID == noID)
		return false;

  // This looks strange, but is neccessary.
// 	vector<String>::iterator npos = ((Key*)m_Keys[keyID])->names.begin() + valueID;
// 	vector<String>::iterator vpos = ((Key*)m_Keys[keyID])->values.begin() + valueID;
	((Key*)m_Keys[(int)keyID])->names.removeAt((int)valueID);
	((Key*)m_Keys[(int)keyID])->values.removeAt((int)valueID);

  return true;
}

bool IniFile::deleteKey( String keyname)
{
	int keyID = (int)findKey( keyname);
	if ( keyID == noID)
		return false;

	// Now hopefully this destroys the vector lists within m_Keys.
	// Looking at <vector> source, this should be the case using the destructor.
	// If not, I may have to do it explicitly. Memory leak check should tell.
	// memleak_test.cpp shows that the following not required.
	//((Key*)m_Keys[keyID])->m_Names.clear();
	//((Key*)m_Keys[keyID])->values.clear();

	m_Names.removeAt(keyID);
	delete (Key*)m_Keys[keyID];
	m_Keys.removeAt(keyID);

  return true;
}

void IniFile::erase()
{
  // This loop not needed. The vector<> destructor seems to do
  // all the work itself. memleak_test.cpp shows this.
  //for ( unsigned i = 0; i < m_Keys.getSize(); ++i) {
  //  m_Keys[i].m_Names.clear();
  //  m_Keys[i].values.clear();
  //}
	m_Names.removeAll();
	m_Comments.removeAll();
	for (int i=0; i<m_Keys.getSize(); i++)
		delete ((Key*)m_Keys[i]);
	m_Keys.removeAll();
}

void IniFile::headerComment( String szComment)
{
	m_Comments.add(szComment);
}

String IniFile::headerComment( unsigned const commentID) const
{
	if ( commentID < (unsigned)m_Comments.getSize())
		return m_Comments[commentID];
	return String("");
}

bool IniFile::deleteHeaderComment( unsigned nCommentID)
{
	if ( nCommentID < (unsigned)m_Comments.getSize())
	{
		m_Comments.removeAt(nCommentID);
		return true;
	}
	return false;
}

unsigned IniFile::numKeyComments( unsigned const keyID) const
{
	if ( keyID < (unsigned)m_Keys.getSize())
		return ((Key*)m_Keys[keyID])->comments.getSize();
	return 0;
}

unsigned IniFile::numKeyComments( String keyname) const
{
	int keyID = (int)findKey( keyname);
	if ( keyID == noID)
		return 0;
	return ((Key*)m_Keys[keyID])->comments.getSize();
}

bool IniFile::keyComment( unsigned const keyID, String comment)
{
	if ( keyID < (unsigned)m_Keys.getSize())
	{
		((Key*)m_Keys[keyID])->comments.add( comment);
		return true;
	}
	return false;
}

bool IniFile::keyComment( String keyname, String comment)
{
	long keyID = findKey( keyname);
	if ( keyID == noID)
		return false;
	return keyComment( unsigned(keyID), comment);
}

String IniFile::keyComment( unsigned const keyID, unsigned const commentID) const
{
	if ( keyID < (unsigned)m_Keys.getSize() && commentID < (unsigned)((Key*)m_Keys[keyID])->comments.getSize())
		return ((Key*)m_Keys[keyID])->comments[commentID];
	return String("");
}

String IniFile::keyComment( String keyname, unsigned const commentID) const
{
	long keyID = findKey( keyname);
	if ( keyID == noID)
		return String("");
	return keyComment( unsigned(keyID), commentID);
}

bool IniFile::deleteKeyComment( unsigned const keyID, unsigned const nCommentID)
{
	if ( keyID < (unsigned)m_Keys.getSize() && nCommentID < (unsigned)((Key*)m_Keys[keyID])->comments.getSize())
	{
		((Key*)m_Keys[keyID])->comments.removeAt( nCommentID);
		return true;
	}
	return false;
}

bool IniFile::deleteKeyComment( String keyname, unsigned const commentID)
{
	long keyID = findKey( keyname);
	if ( keyID == noID)
		return false;
	return deleteKeyComment( unsigned(keyID), commentID);
}

bool IniFile::deleteKeyComments( unsigned const keyID)
{
	if ( keyID < (unsigned)m_Keys.getSize())
	{
		((Key*)m_Keys[keyID])->comments.removeAll();
		return true;
	}
	return false;
}

bool IniFile::deleteKeyComments( String keyname)
{
	long keyID = findKey( keyname);
	if ( keyID == noID)
		return false;
	return deleteKeyComments( unsigned(keyID));
}

String IniFile::checkCase( String s) const
{
	if ( m_fCaseInsensitive)
		s.makeLower();
	return s;
}

/**
  @memo     Constructor.

  @doc      Constructor. Opens the file.
  
  @param    filename	The file to open.
    
  @return   Nothing.
  
*/
PropsReader::PropsReader( const String &filename ) : CR(0x0a), LF(0x0d), COMMENT_CHAR('#') 
{
	fh = fopen( filename, "rb" );
}


/**
  @memo     Destructor.

  @doc      Destructor. Closes the file.
    
  @return   Nothing.
  
*/
PropsReader::~PropsReader() 
{
	if (fh) fclose(fh);
}


/**
  @memo     Increments the file pointer beyond any white space.

  @doc      Increments the file pointer beyond any white space.
    
  @return   Nothing.
  
*/
void PropsReader::SkipWhiteSpace() 
{
    int ch; 
	do { 
        ch=fgetc( fh ); 
    } while( ch==' ' || ch=='\t' || ch==CR || ch==LF );
    ungetc( ch, fh );
}


/**
  @memo     Increments the file pointer to the start of the next line.

  @doc      Increments the file pointer to the start of the next line.
    
  @return   Nothing.
  
*/
void PropsReader::SkipRestOfLine() 
{
	int ch; 
	do { 
        ch=fgetc( fh ); 
    } while( ch!=EOF && ch!=CR && ch!=LF );
    ungetc( ch, fh );

	SkipWhiteSpace();
}


/**
  @memo     Returns true if more white space is present.

  @doc      Returns true if more white space is present on the current line.
    
  @return   Nothing.
  
*/
bool PropsReader::moreNonWhiteSpaceOnLine() 
{
    char buf[256];
	int ch, n_gets = 0; 
    bool non_white = false;

	do {
      ch=fgetc( fh ); 
      buf[n_gets++] = (char)ch;
      if ( ch!='\t' && ch!=' ' && ch!=CR && ch!=LF && ch!=EOF) {  non_white = true; break; }
    
    } while( ch!=EOF && ch!=CR && ch!=LF );

    for(int i=0;i<n_gets;i++) ungetc( buf[--n_gets], fh );	

    return non_white;
}


/**
  @memo     Increments the file pointer beyond any comments.

  @doc      Increments the file pointer beyond any comments.
    
  @return   Nothing.
  
*/
void PropsReader::SkipComments() 
{
	int ch;
	
    ch = getc( fh );        
	if ( ch==COMMENT_CHAR ) {
			
		SkipRestOfLine();                        
        SkipComments();
	} else {
		ungetc( ch, fh );
	}
}

TokenFile::TokenFile()
{
	m_CurString.empty(); // make string info empty
	m_Delimiter = " ,\n\t";
	strtok((char *)(const char*)m_CurString, m_Delimiter);
}

TokenFile::~TokenFile()
{
	
}

#pragma warning (push)
#pragma warning (disable : 4706)
bool TokenFile::nextToken(String& tok)
{
	char *tpos;
	bool rval;
	
	if ((tpos = strtok(NULL,m_Delimiter)) == NULL) // is there any valid data left in string ?
	{
		m_CurString.empty();
		while ((rval = this->readString(m_CurString)) && m_CurString.isEmpty()) {}
		if (!rval) return false; // we reached end of file
		
		tpos = strtok((char *)(const  char*)m_CurString,m_Delimiter);
		if (tpos == NULL) return false;
	}
	
	tok = tpos;
	return true;
}
#pragma warning (pop)

void TokenFile::writeString(const char* lpsz)
{
	write (lpsz, strlen(lpsz)+1, 1);
}

char* TokenFile::readString(char* lpsz, int nMax)
{
	int i, ch;
	
	/* read in single line from "stdin": */
	for( i = 0; (i < nMax-1) &&  ((ch = getC()) != EOF) && (ch != '\n'); i++ )
		lpsz[i] = (char)ch;
	
	/* Terminate string with null character: */
	lpsz[i] = '\0';
	return lpsz;
}

bool TokenFile::readString(String& rString)
{
	int nMax = 10000;
	char* lpsz = (char*)malloc(nMax);
	int i, ch;
	
	/* read in single line from "stdin": */
	for( i = 0; (i < nMax-1) &&  ((ch = getC()) != EOF) && (ch != '\n'); i++ )
		lpsz[i] = (char)ch;
	if (eof())
		return false;
	/* Terminate string with null character: */
	lpsz[i] = '\0';
	rString = lpsz;
	free(lpsz);
	return true;
}

}
