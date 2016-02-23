// //////////////////////////////////////////////////////////////////////////
// Header file TestRunnerModel.h for class TestRunnerModel
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/04/26
// //////////////////////////////////////////////////////////////////////////
#ifndef TESTRUNNERMODEL_H
#define TESTRUNNERMODEL_H

#include <deque>
#include <cppunit/Test.h>


/*! \class TestRunnerModel
 * \brief This class represents a model for the test runner.
 */
class TestRunnerModel
{
public:
  static const CString settingKey;
  static const CString settingMainDialogKey;
  static const CString settingBrowseDialogKey;

  struct Settings
  {
    bool autorunOnLaunch;
    int  col_1; // 1st column width in list view
    int  col_2; // 2nd column width in list view
    int  col_3; // 3rd column width in list view
    int  col_4; // 4th column width in list view  
  };

  typedef std::deque<CPPUNIT_NS::Test *> History;

  /*! Constructs a TestRunnerModel object.
   */
  TestRunnerModel( CPPUNIT_NS::Test *rootTest );

  /*! Destructor.
   */
  virtual ~TestRunnerModel();

  virtual void setRootTest( CPPUNIT_NS::Test *rootTest );

  void loadSettings(Settings & s);
  void saveSettings(const Settings & s);

  const History &history() const;
  void selectHistoryTest( CPPUNIT_NS::Test *test );
  CPPUNIT_NS::Test *selectedTest() const;

  CPPUNIT_NS::Test *rootTest();

protected:
  void loadHistory();
  CString loadHistoryEntry( int idx );
  CPPUNIT_NS::Test *findTestByName( CString name ) const;
  CPPUNIT_NS::Test *findTestByNameFor( const CString &name, 
                                    CPPUNIT_NS::Test *test ) const;

  void saveHistoryEntry( int idx, 
                         CString testName );

  CString getHistoryEntryName( int idx ) const;

  static std::string toAnsiString( const CString &text );

private:
  /// Prevents the use of the copy constructor.
  TestRunnerModel( const TestRunnerModel &copy );

  /// Prevents the use of the copy operator.
  TestRunnerModel &operator =( const TestRunnerModel &copy );

protected:
  History m_history;

  CPPUNIT_NS::Test *m_rootTest;
};



// Inlines methods for TestRunnerModel:
// ------------------------------------



#endif  // TESTRUNNERMODEL_H
