#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qmultilineedit.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <cppunit/Exception.h>
#include "TestRunnerDlgImpl.h"
#include "TestBrowserDlgImpl.h"
#include "MostRecentTests.h"
#include "TestRunnerModel.h"
#include "TestFailureListViewItem.h"


/* 
 *  Constructs a TestRunnerDlg which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
TestRunnerDlg::TestRunnerDlg( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : TestRunnerDlgBase( parent, name, modal, fl ),
    _model( NULL ),
    _recentTests( new MostRecentTests() )
{
}


/*  
 *  Destroys the object and frees any allocated resources
 */
TestRunnerDlg::~TestRunnerDlg()
{
  delete _model;
  delete _recentTests;
}


void 
TestRunnerDlg::setModel( TestRunnerModel *model,
                         bool autorunTest )
{
  delete _model;

  _model = model;

  // update combo when recent list change
  connect( _recentTests, SIGNAL( listChanged() ),
           SLOT( refreshRecentTests() ) );

  // make selected test in combo the "most recent"
  connect( _comboTest, SIGNAL( activated(int) ),
           _recentTests, SLOT( selectTestToRun(int) ) );

  // refresh the test report counters when a test is selected
  connect( _recentTests, SIGNAL( testToRunChanged(CPPUNIT_NS::Test *) ),
           _model, SLOT( resetTestReportCounterFor(CPPUNIT_NS::Test *) ) );

  // refresh progress bar
  connect( _model, SIGNAL( numberOfTestCaseChanged(int) ),
           _progressRun, SLOT( setTotalSteps(int) ) );
  connect( _model, SIGNAL( numberOfTestCaseRunChanged(int) ),
           _progressRun, SLOT( setProgress(int) ) );
  
  // refresh test report counters
  connect( _model, SIGNAL( numberOfTestCaseChanged( int ) ),
           SLOT( setNumberOfTestCase( int ) ) );
  connect( _model, SIGNAL( numberOfTestCaseRunChanged( int ) ),
           SLOT( setNumberOfTestCaseRun( int ) ) );
  connect( _model, SIGNAL( numberOfTestCaseFailureChanged( int ) ),
           SLOT( setNumberOfTestCaseFailure( int ) ) );

  // clear failure list
  connect( _model, SIGNAL( failuresCleared() ),
           SLOT( clearTestFailureList() ) );

  // clear failure detail list
  connect( _model, SIGNAL( failuresCleared() ),
           SLOT( clearFailureDetail() ) );

  // add failure to failure list
  connect( _model, SIGNAL( failureAdded(TestFailureInfo *) ),
           SLOT( reportFailure(TestFailureInfo*) ) );

  // show detail on failure selection
  connect( _listFailures, SIGNAL( selectionChanged(QListViewItem*) ),
           SLOT( showFailureDetailAt(QListViewItem*) ) );

  // disable button when running test
  connect( _model, SIGNAL( testRunStarted( CPPUNIT_NS::Test *, CPPUNIT_NS::TestResult *) ),
           SLOT( beRunningTest() ) );

  // enable button when finished running test
  connect( _model, SIGNAL( testRunFinished() ),
           SLOT( beCanRunTest() ) );

  _recentTests->setTestToRun( model->rootTest() );
  beCanRunTest();

  if ( autorunTest )
    runTest();
}


void 
TestRunnerDlg::browseForTest()
{
  TestBrowser *dlg = new TestBrowser( this,
                                      "Test Browser",
                                      TRUE );
  dlg->setRootTest( _model->rootTest() );

  if ( dlg->exec() )
    _recentTests->setTestToRun( dlg->selectedTest() );

  delete dlg;
}


void 
TestRunnerDlg::runTest()
{
  CPPUNIT_NS::Test *testToRun = _recentTests->testToRun();
  if ( testToRun == NULL )
    return;
  _model->runTest( testToRun );
}


void 
TestRunnerDlg::stopTest()
{
  _model->stopRunningTest();
  if ( _model->isTestRunning() )
    beStoppingTest();
}


void 
TestRunnerDlg::clearTestFailureList()
{
  _listFailures->clear();
}


void 
TestRunnerDlg::refreshRecentTests()
{
  _comboTest->clear();
  for ( int index =0; index < _recentTests->testCount(); ++index )
    _comboTest->insertItem( _recentTests->testNameAt( index ) );
}


void 
TestRunnerDlg::setNumberOfTestCase( int numberOfTestCase )
{
  _labelTestCaseCount->setText( QString::number( numberOfTestCase ) );
}


void 
TestRunnerDlg::setNumberOfTestCaseRun( int numberOfRun )
{
  _labelTestRunCount->setText( QString::number( numberOfRun ) );
}


void 
TestRunnerDlg::setNumberOfTestCaseFailure( int numberOfFailure )
{
  _labelFailureCount->setText( QString::number( numberOfFailure ) );
}


void 
TestRunnerDlg::reportFailure( TestFailureInfo *failure )
{
  QListViewItem *item = new TestFailureListViewItem( failure, 
                                                     _listFailures );
  item->setText( indexType, 
                 failure->isError() ? tr("Error") : tr("Failure") );
  std::string failedtestName = failure->failedTestName().c_str();
  item->setText( indexTestName, QString::fromLatin1( failedtestName.c_str() ) );

  CPPUNIT_NS::Exception *thrownException = failure->thrownException();
//2.0  item->setText( indexMessage, thrownException->what() );
  item->setText( indexMessage, QString(thrownException->what()).stripWhiteSpace() );
  item->setText( indexFilename, failure->sourceLine().fileName().c_str() );
  item->setText( indexLineNumber,
                 QString::number( failure->sourceLine().lineNumber() ) );

  _listFailures->insertItem( item );
  _listFailures->triggerUpdate();

  if ( _listFailures->childCount() == 1 )
    _listFailures->setSelected( item, TRUE );
}


void 
TestRunnerDlg::showFailureDetailAt( QListViewItem *selection )
{
  TestFailureInfo *failure = ((TestFailureListViewItem*)selection)->failure();
  
  QString title = tr("Failure detail for: ");
  title += QString::fromLatin1( failure->failedTestName().c_str() );
  _groupFailureDetail->setTitle( title );

  QString location( failure->sourceLine().fileName().c_str() );
  location += " (" + 
              QString::number( failure->sourceLine().lineNumber() ) +
              ")";
  _labelFailureLocation->setText( location );

  _editFailureMessage->setText( failure->thrownException()->what() );
}


void 
TestRunnerDlg::clearFailureDetail()
{
  _groupFailureDetail->setTitle( tr("Failure detail for:...") );
  _labelFailureLocation->setText( QString::null );
  _editFailureMessage->setText( QString::null );
}


void 
TestRunnerDlg::beCanRunTest()
{
  _buttonRunTest->setEnabled( true );
  _buttonBrowse->setEnabled( true );
  _comboTest->setEnabled( true );
  _buttonStop->setDisabled( true );
  _buttonStop->setText( tr("Stop") );
  _buttonClose->setEnabled( true );
}


void 
TestRunnerDlg::beRunningTest()
{
  _buttonRunTest->setDisabled( true );
  _buttonBrowse->setDisabled( true );
  _comboTest->setDisabled( true );
  _buttonStop->setEnabled( true );
  _buttonStop->setText( tr("Stop") );
  _buttonClose->setDisabled( true );
}


void 
TestRunnerDlg::beStoppingTest()
{
  _buttonStop->setDisabled( true );
  _buttonStop->setText( tr("Stopping") );
}
