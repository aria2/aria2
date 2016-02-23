// //////////////////////////////////////////////////////////////////////////
// Header file TestPlugInException.h for class TestPlugInException
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/06/23
// //////////////////////////////////////////////////////////////////////////
#ifndef TESTPLUGINEXCEPTION_H
#define TESTPLUGINEXCEPTION_H

#include <stdexcept>
#include <string>


/*! \class TestPlugInException
 * \brief This class represents a failure of using the test plug-in.
 */
class TestPlugInException : public std::runtime_error
{
public:
  enum Cause
  {
    failedToLoadDll =0,
    failedToCopyDll,
    failedToGetInterfaceFunction,
    failedToMakeTest
  };

  /*! Constructs a TestPlugInException object.
   */
  TestPlugInException( std::string message, 
                       Cause cause );

  /*! Copy constructor.
   * @param copy Object to copy.
   */
  TestPlugInException( const TestPlugInException &copy );

  /*! Destructor.
   */
  virtual ~TestPlugInException();

  /*! Copy operator.
   * @param copy Object to copy.
   * @return Reference on this object.
   */
  TestPlugInException &operator =( const TestPlugInException &copy );

  Cause getCause() const;

private:
  Cause m_cause;
};



// Inlines methods for TestPlugInException:
// ----------------------------------------



#endif  // TESTPLUGINEXCEPTION_H
