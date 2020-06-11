/*!
 * \file	Object.h
 * \ingroup base
 * \brief	
 * \author	
 */

#pragma once

#include "String.hpp"

namespace cvlib 
{
	
/**
 * @brief	cvlib Library
 <pre> . </pre>
 */
class CVLIB_DECLSPEC Exception
{
public:
	// Construction and Destruction
	Exception ();
    virtual ~Exception() throw();

    Exception(const Exception& rhs) throw();
    Exception& operator=(const Exception& rhs) throw();
    virtual const char *What() const throw() {return errorMessage();}

	// @cmember
	// Constructor.
	Exception(const char *szMessage, long lErrorCode,   
		      const char *szFunctionName = 0, 
			  const char *szSourceFile = 0, 
			  int iLine = 0) throw();
	
	// @cmember
	// Get the function name.
	const char *functionName(void) const {return m_szFunctionName;}
	
	// @cmember
	// Get the error message.
	const char *errorMessage(void) const {return m_szMessage;}
	
	// @cmember
	// Get the source file.
	const char *sourceFile(void) const {return m_szSourceFile;}
	
	// @cmember
	// Get the line number.
	int sourceLine(void) const {return m_line;}
	
	// @cmember
	// Get the error code.
	long errorCode(void) const {return m_code;}
	
	// @cmember
	// Return a complete message.
	virtual const char *fullMessage(void);
	
	
	// @cmember
	// Since we don't have run-time type information, we have a virtual
	// ThrowLast() method to make sure that type information for
	// derived classes is not lost in try-catch blocks.
	virtual void throwLast(void) {	throw *this;}
	
	
	// @access:(IMPL) Protected members
protected:
    // @cmember:(IMPL)
	// Function name that the error occurred in.
	const char *m_szFunctionName;
	
	// @cmember:(IMPL)
	// Error message.
	const char *m_szMessage;
	
	// @cmember:(IMPL)
	// Source file that the error occurred in.
	const char *m_szSourceFile;
	
    // @cmember:(IMPL)
	// Full error message.
	String m_strFullMessage;
	
	// @cmember:(IMPL)
	// Line number that the error occurred on.
	int  m_line;
	
	// @cmember:(IMPL)
	// Error code.
	long m_code;
};

}