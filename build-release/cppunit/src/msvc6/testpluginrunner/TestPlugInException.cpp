// //////////////////////////////////////////////////////////////////////////
// Implementation file TestPlugInException.cpp for class TestPlugInException
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/06/23
// //////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TestPlugInException.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



TestPlugInException::TestPlugInException( std::string message, 
                                          Cause cause ) : 
    runtime_error( message ),
    m_cause( cause )
{
}


TestPlugInException::TestPlugInException( const TestPlugInException &copy ) : 
    runtime_error( copy )
{
}


TestPlugInException::~TestPlugInException()
{
}


TestPlugInException &
TestPlugInException::operator =( const TestPlugInException &copy )
{
  runtime_error::operator =( copy );
  m_cause = copy.m_cause;
  return *this;
}


TestPlugInException::Cause 
TestPlugInException::getCause() const
{
  return m_cause;
}
