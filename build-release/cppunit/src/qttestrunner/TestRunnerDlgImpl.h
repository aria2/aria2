#ifndef TESTRUNNERDLG_H
#define TESTRUNNERDLG_H

#include "testrunnerdlg.h"
#include <cppunit/Test.h>
class TestRunnerModel;
class MostRecentTests;
class TestFailureInfo;
class QListViewItem;

class TestRunnerDlg : public TestRunnerDlgBase
{ 
  Q_OBJECT

public:
  TestRunnerDlg( QWidget* parent = 0, 
                 const char* name = 0, 
                 bool modal = FALSE, 
                 WFlags fl = 0 );
  ~TestRunnerDlg();

  void setModel( TestRunnerModel *model,
                 bool autorunTest );

public slots:
  void refreshRecentTests();

protected slots:
  void browseForTest();
  void runTest();
  void stopTest();
  void setNumberOfTestCase( int numberOfTestCase );
  void setNumberOfTestCaseRun( int numberOfRun );
  void setNumberOfTestCaseFailure( int numberOfFailure );
  void clearTestFailureList();
  void clearFailureDetail();
  void reportFailure( TestFailureInfo *failure );
  void showFailureDetailAt( QListViewItem *selection );
  void beCanRunTest();
  void beRunningTest();
  void beStoppingTest();

private:

  enum Columns
  {
    indexType =0,
    indexTestName,
    indexMessage,
    indexFilename,
    indexLineNumber
  };

  TestRunnerModel *_model;
  MostRecentTests *_recentTests;
};

#endif // TESTRUNNERDLG_H
