// TestRunnerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mmsystem.h"
#include "TestRunnerApp.h"
#include "TestRunnerDlg.h"
#include "Resource.h"
#include "ActiveTest.h"
#include "ProgressBar.h"
#include "TreeHierarchyDlg.h"
#include "ListCtrlFormatter.h"
#include "ListCtrlSetter.h"
#include "MfcSynchronizationObject.h"
#include "ResourceLoaders.h"
#include <cppunit/TestFailure.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/* Notes:
 - code duplication between OnOK() and OnQuitApplication()
 - the threading need to be rewrite, so that GUI update occures in the original
 thread, not in the thread that is running the tests. This slow down the time
 needed to run the test much...
 */


/////////////////////////////////////////////////////////////////////////////
// TestRunnerDlg dialog

const CString TestRunnerDlg::ms_cppunitKey( "CppUnit" );


TestRunnerDlg::TestRunnerDlg( TestRunnerModel *model,
                              int nDialogResourceId,
                              CWnd* pParent )
    : cdxCDynamicDialog( nDialogResourceId, pParent )
{
  ASSERT(0); // this constructor should not be used because of possible resource problems
             // => use the constructor with the string parameter
  init(model);
}

TestRunnerDlg::TestRunnerDlg( TestRunnerModel *model,
                              const TCHAR* szDialogResourceId,
                              CWnd* pParent )
    : cdxCDynamicDialog( szDialogResourceId == NULL ? 
                                _T("CPP_UNIT_TEST_RUNNER_IDD_DIALOG_TESTRUNNER")
                              : szDialogResourceId, 
                         pParent)
{
  init(model);
}

void
TestRunnerDlg::init(TestRunnerModel *model)
{
  m_model = model;

  //{{AFX_DATA_INIT(TestRunnerDlg)
    m_bAutorunAtStartup = FALSE;
  //}}AFX_DATA_INIT

  m_testsProgress     = 0;
  m_selectedTest      = 0;
  m_bAutorunAtStartup = true;
  m_bIsRunning = false;
  m_activeTest = 0;
  m_result = 0;
  m_testObserver = 0;

  ModifyFlags( flSWPCopyBits, 0 );      // anti-flickering option for resizing
}

void 
TestRunnerDlg::DoDataExchange(CDataExchange* pDX)
{
  cdxCDynamicDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(TestRunnerDlg)
    DDX_Control(pDX, IDC_DETAILS, m_details);
    DDX_Control(pDX, IDC_LIST, m_listCtrl);
    DDX_Control(pDX, IDOK, m_buttonClose);
    DDX_Control(pDX, ID_STOP, m_buttonStop);
    DDX_Control(pDX, ID_RUN, m_buttonRun);
    DDX_Control(pDX, IDC_BROWSE_TEST, m_buttonBrowse);
    DDX_Check(pDX, IDC_CHECK_AUTORUN, m_bAutorunAtStartup);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(TestRunnerDlg, cdxCDynamicDialog)
  //{{AFX_MSG_MAP(TestRunnerDlg)
  ON_BN_CLICKED(ID_RUN, OnRun)
  ON_BN_CLICKED(ID_STOP, OnStop)
  ON_BN_CLICKED(IDC_BROWSE_TEST, OnBrowseTest)
  ON_COMMAND(ID_QUIT_APPLICATION, OnQuitApplication)
  ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnSelectedFailureChange)
	ON_CBN_SELCHANGE(IDC_COMBO_TEST, OnSelectTestInHistoryCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TestRunnerDlg message handlers

BOOL 
TestRunnerDlg::OnInitDialog() 
{
  cdxCDynamicDialog::OnInitDialog();

#ifdef CPPUNIT_SUBCLASSING_TESTRUNNERDLG_BUILD
  m_hAccelerator = ::LoadAccelerators( AfxGetResourceHandle(),
#else
  m_hAccelerator = ::LoadAccelerators( g_testRunnerResource,
#endif
                                       MAKEINTRESOURCE( IDR_ACCELERATOR_TEST_RUNNER ) );
// It always fails!!! I don't understand why. Complain about not finding the resource name!
  ASSERT( m_hAccelerator !=NULL );
  
  CComboBox   *comboBox = (CComboBox *)GetDlgItem (IDC_COMBO_TEST);

  ASSERT (comboBox);

  VERIFY( m_errorListBitmap.Create( _T("CPP_UNIT_TEST_RUNNER_IDB_ERROR_TYPE"), 
                                    16, 1, 
                                    RGB( 255,0,255 ) ) );

  m_testsProgress = new ProgressBar();
  m_testsProgress->Create( NULL, NULL, WS_CHILD, CRect(), this, 0 );
  m_testsProgress->ShowWindow( SW_SHOW );
  m_testsProgress->MoveWindow( getItemClientRect( IDC_STATIC_PROGRESS_BAR ) );

  initializeLayout();
  loadSettings();
  initializeFixedSizeFont();
  m_details.SetFont( &m_fixedSizeFont );  // Does not work. Need to investigate...
      
  m_listCtrl.SetImageList( &m_errorListBitmap, LVSIL_SMALL );
  m_listCtrl.SetExtendedStyle( m_listCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT );

  int total_col_1_4 = m_settings.col_1 + m_settings.col_2 + 
		      m_settings.col_3 + m_settings.col_4;

  CRect listBounds;
  m_listCtrl.GetClientRect(&listBounds);
  int col_5_width = listBounds.Width() - total_col_1_4; // 5th column = rest of listview space
  ListCtrlFormatter formatter( m_listCtrl );
  formatter.AddColumn( loadCString(IDS_ERRORLIST_TYPE), m_settings.col_1, LVCFMT_LEFT, 0 );
  formatter.AddColumn( loadCString(IDS_ERRORLIST_NAME), m_settings.col_2, LVCFMT_LEFT, 1 );
  formatter.AddColumn( loadCString(IDS_ERRORLIST_FAILED_CONDITION), m_settings.col_3, LVCFMT_LEFT, 2 );
  m_listCtrl.setLineNumberSubItem( formatter.GetNextColumnIndex() );
  formatter.AddColumn( loadCString(IDS_ERRORLIST_LINE_NUMBER), m_settings.col_4, LVCFMT_LEFT, 3 );
  m_listCtrl.setFileNameSubItem( formatter.GetNextColumnIndex() );
  formatter.AddColumn( loadCString(IDS_ERRORLIST_FILE_NAME), col_5_width, LVCFMT_LEFT, 4 );

  reset ();
  updateHistoryCombo();
  UpdateData( FALSE );

  updateListColumnSize();

  m_buttonRun.SetFocus();

  if ( m_bAutorunAtStartup )
    OnRun();
  
  return FALSE;  // return TRUE unless you set the focus to a control
                 // EXCEPTION: OCX Property Pages should return FALSE
}


TestRunnerDlg::~TestRunnerDlg()
{ 
  freeState();
  delete m_testsProgress;
}


void 
TestRunnerDlg::OnRun() 
{
  if ( m_bIsRunning )
    return;

  m_selectedTest = m_model->selectedTest();

  if ( m_selectedTest == 0 )
    return;

  freeState(); 
  reset();

  beRunning();

  int numberOfTests = m_selectedTest->countTestCases();

  m_testsProgress->start( numberOfTests );

  
  m_result = new CPPUNIT_NS::TestResultCollector( new MfcSynchronizationObject() );
  m_testObserver = new CPPUNIT_NS::TestResult( new MfcSynchronizationObject() );
  m_testObserver->addListener( m_result );
  m_testObserver->addListener( this );
  m_activeTest = new ActiveTest( m_selectedTest );

  m_testStartTime = timeGetTime();

  m_activeTest->run( m_testObserver );

  m_testEndTime = timeGetTime();
}


void 
TestRunnerDlg::addListEntry( const CPPUNIT_NS::TestFailure &failure )
{
  CListCtrl *listCtrl = (CListCtrl *)GetDlgItem (IDC_LIST);
  int currentEntry = m_result->testErrors() + 
                     m_result->testFailures() -1;

  ErrorTypeBitmaps errorType;
  if ( failure.isError() )
    errorType = errorTypeError;
  else
    errorType = errorTypeFailure;

  ListCtrlSetter setter( *listCtrl );
  setter.insertLine( currentEntry );
  setter.addSubItem( failure.isError() ? _T("Error") : _T("Failure"), errorType );

  // Set test name
  setter.addSubItem( failure.failedTestName().c_str(), errorType );

  // Set the asserted text
  CString message( failure.thrownException()->message().shortDescription().c_str() );
  message.Replace( '\n', ' ' );   // should only print the short description there,
  setter.addSubItem( message );   // and dump the detail on an edit control when clicked.

  // Set the line number
  if ( failure.sourceLine().isValid() )
  {
    CString lineNumber;
    lineNumber.Format( _T("%ld"), failure.sourceLine().lineNumber() );
    setter.addSubItem( lineNumber );
  }
  else
    setter.addSubItem( _T("") );

  // Set the file name
  setter.addSubItem( failure.sourceLine().fileName().c_str() );

  if ( !listCtrl->GetFirstSelectedItemPosition() )
  {
    // Select first entry => display details of first entry.
    listCtrl->SetItemState( currentEntry, LVIS_SELECTED, LVIS_SELECTED );
    listCtrl->SetFocus();   // Does not work ?!?
  }

  listCtrl->RedrawItems( currentEntry, currentEntry );
  listCtrl->UpdateWindow();
}


void 
TestRunnerDlg::startTest( CPPUNIT_NS::Test *test )
{
  CWnd *runningTestCaseLabel = GetDlgItem(IDC_RUNNING_TEST_CASE_LABEL);
  if ( runningTestCaseLabel )
    runningTestCaseLabel->SetWindowText( CString( test->getName().c_str() ) );
}


void 
TestRunnerDlg::addFailure( const CPPUNIT_NS::TestFailure &failure )
{
  addListEntry( failure );
  if ( failure.isError() )
    m_errors++;
  else
    m_failures++;

  updateCountsDisplay();
}


void 
TestRunnerDlg::endTest( CPPUNIT_NS::Test *test )
{
  if ( m_selectedTest == 0 )
    return;

  m_testsRun++;
  updateCountsDisplay();
  m_testsProgress->step( m_failures == 0  &&  m_errors == 0 );

  m_testEndTime = timeGetTime();

  updateCountsDisplay();

  if ( m_testsRun >= m_selectedTest->countTestCases() )
    beIdle ();
}


void 
TestRunnerDlg::beRunning()
{
  m_bIsRunning = true;
  m_buttonRun.EnableWindow( FALSE );
  m_buttonClose.EnableWindow( FALSE );
  m_buttonBrowse.EnableWindow( FALSE );

//    m_buttonRun.SetButtonStyle( m_buttonRun.GetButtonStyle() & ~BS_DEFPUSHBUTTON );
//    m_buttonStop.SetButtonStyle( m_buttonStop.GetButtonStyle() | BS_DEFPUSHBUTTON );
}


void 
TestRunnerDlg::beIdle()
{
  m_bIsRunning = false;
  m_buttonRun.EnableWindow( TRUE );
  m_buttonBrowse.EnableWindow( TRUE );
  m_buttonClose.EnableWindow( TRUE );

  m_buttonRun.SetButtonStyle( m_buttonRun.GetButtonStyle() | BS_DEFPUSHBUTTON );
//    m_buttonStop.SetButtonStyle( m_buttonStop.GetButtonStyle() & ~BS_DEFPUSHBUTTON );
}


void 
TestRunnerDlg::beRunDisabled()
{
  m_bIsRunning = false;
  m_buttonRun.EnableWindow( FALSE );
  m_buttonBrowse.EnableWindow( FALSE );
  m_buttonStop.EnableWindow( FALSE );
  m_buttonClose.EnableWindow( TRUE );

//    m_buttonRun.SetButtonStyle( m_buttonRun.GetButtonStyle() | BS_DEFPUSHBUTTON );
//    m_buttonStop.SetButtonStyle( m_buttonStop.GetButtonStyle() & ~BS_DEFPUSHBUTTON );
}


void 
TestRunnerDlg::freeState()
{
  delete m_activeTest;
  delete m_result;
  delete m_testObserver;
  m_activeTest = 0;
  m_result = 0;
  m_testObserver = 0;
}


void 
TestRunnerDlg::reset()
{
  m_testsRun = 0;
  m_errors = 0;
  m_failures = 0;
  m_testEndTime = m_testStartTime;

  updateCountsDisplay();

  freeState();
  CListCtrl *listCtrl = (CListCtrl *)GetDlgItem (IDC_LIST);

  listCtrl->DeleteAllItems();
  m_testsProgress->reset();
  displayFailureDetailsFor( -1 );
}


void 
TestRunnerDlg::updateCountsDisplay()
{
  CStatic *statTestsRun = (CStatic *)GetDlgItem( IDC_STATIC_RUNS );
  CStatic *statErrors = (CStatic *)GetDlgItem( IDC_STATIC_ERRORS );
  CStatic *statFailures = (CStatic *)GetDlgItem( IDC_STATIC_FAILURES );
  CEdit *editTime = (CEdit *)GetDlgItem( IDC_EDIT_TIME );

  CString argumentString;

  argumentString.Format( _T("%d"), m_testsRun );
  statTestsRun->SetWindowText (argumentString);

  argumentString.Format( _T("%d"), m_errors );
  statErrors->SetWindowText( argumentString );

  argumentString.Format( _T("%d"), m_failures );
  statFailures->SetWindowText( argumentString );

  argumentString.Format( _T("Execution time: %3.3lf seconds"), 
                         (m_testEndTime - m_testStartTime) / 1000.0 );
  editTime->SetWindowText( argumentString );
}


void 
TestRunnerDlg::OnStop() 
{
  if ( m_testObserver )
    m_testObserver->stop ();

  beIdle ();
}


void 
TestRunnerDlg::OnOK() 
{
  if ( m_testObserver )
    m_testObserver->stop ();

  UpdateData();
  saveSettings();

  cdxCDynamicDialog::OnOK ();
}


void 
TestRunnerDlg::OnSelectTestInHistoryCombo() 
{
  unsigned int currentSelection = getHistoryCombo()->GetCurSel ();

  if ( currentSelection >= 0  &&
       currentSelection < m_model->history().size() )
  {
    CPPUNIT_NS::Test *selectedTest = m_model->history()[currentSelection];
    m_model->selectHistoryTest( selectedTest );
    updateHistoryCombo();
    beIdle();
  }
  else
    beRunDisabled();
}


void
TestRunnerDlg::updateHistoryCombo()
{
  getHistoryCombo()->LockWindowUpdate();

  getHistoryCombo()->ResetContent();

  const TestRunnerModel::History &history = m_model->history();
  for ( TestRunnerModel::History::const_iterator it = history.begin(); 
        it != history.end(); 
        ++it )
  {
    CPPUNIT_NS::Test *test = *it;
    getHistoryCombo()->AddString( CString(test->getName().c_str()) );
  }

  if ( history.size() > 0 )
  {
    getHistoryCombo()->SetCurSel( 0 );
    beIdle();
  }
  else
  {
    beRunDisabled();
    m_buttonBrowse.EnableWindow( TRUE );
	}

  getHistoryCombo()->UnlockWindowUpdate();
}


void 
TestRunnerDlg::OnBrowseTest() 
{
  TreeHierarchyDlg dlg;
  dlg.setRootTest( m_model->rootTest() );
  if ( dlg.DoModal() == IDOK )
  {
    m_model->selectHistoryTest( dlg.getSelectedTest() );
    updateHistoryCombo();
  }
}


BOOL 
TestRunnerDlg::PreTranslateMessage(MSG* pMsg) 
{
  if ( ::TranslateAccelerator( m_hWnd,
                               m_hAccelerator,
                               pMsg ) )
  {
    return TRUE;
  }
  return cdxCDynamicDialog::PreTranslateMessage(pMsg);
}


CComboBox *
TestRunnerDlg::getHistoryCombo()
{
  CComboBox   *comboBox = (CComboBox *)GetDlgItem (IDC_COMBO_TEST);
  ASSERT (comboBox);
  return comboBox;
}


void
TestRunnerDlg::loadSettings()
{
  m_model->loadSettings(m_settings);
  RestoreWindowPosition( TestRunnerModel::settingKey, 
                         TestRunnerModel::settingMainDialogKey );


  m_bAutorunAtStartup = m_settings.autorunOnLaunch;
}


void
TestRunnerDlg::saveSettings()
{
  m_settings.autorunOnLaunch = ( m_bAutorunAtStartup != 0 );
  StoreWindowPosition( TestRunnerModel::settingKey, 
                       TestRunnerModel::settingMainDialogKey );
  
  m_settings.col_1 = m_listCtrl.GetColumnWidth(0);
  m_settings.col_2 = m_listCtrl.GetColumnWidth(1);
  m_settings.col_3 = m_listCtrl.GetColumnWidth(2);
  m_settings.col_4 = m_listCtrl.GetColumnWidth(3);

  m_model->saveSettings(m_settings);
}


void 
TestRunnerDlg::OnQuitApplication() 
{
  if ( m_testObserver )
    m_testObserver->stop();

  UpdateData();
  saveSettings();
  
  CWinApp *app = AfxGetApp();
  ASSERT( app != NULL );
  app->PostThreadMessage( WM_QUIT, 0, 0 );
}


TestRunnerModel &
TestRunnerDlg::model()
{
  ASSERT( m_model != NULL );
  return *m_model;
}


void 
TestRunnerDlg::OnClose() 
{
	OnOK();
}


CRect 
TestRunnerDlg::getItemWindowRect( unsigned int itemId )
{
  CWnd * pItem = GetDlgItem( itemId );
  CRect rect;
  if ( pItem )
    pItem->GetWindowRect( &rect );
  return rect;
}


CRect 
TestRunnerDlg::getItemClientRect( unsigned int itemId )
{
  CRect rect = getItemWindowRect( itemId );
  if ( !rect.IsRectNull() )
  {
    CPoint clientTopLeft = rect.TopLeft();
    ScreenToClient( &clientTopLeft );
    rect = CRect( clientTopLeft, rect.Size() );
  }

  return rect;
}


void 
TestRunnerDlg::initializeLayout()
{
  // see DynamicWindow/doc for documentation
  const int listGrowthRatio = 30;
  AddSzXControl( IDC_COMBO_TEST, mdResize );
  AddSzXControl( IDC_BROWSE_TEST, mdRepos );
  AddSzXControl( IDC_RUNNING_TEST_CASE_LABEL, mdResize );
  AddSzXControl( ID_RUN, mdRepos );
  AddSzXControl( *m_testsProgress, mdResize );
  AddSzXControl( IDC_CHECK_AUTORUN, mdRepos );
  AddSzControl( IDC_LIST, 0, 0, 100, listGrowthRatio );
  AddSzXControl( ID_STOP, mdRepos );
  AddSzXControl( IDOK, mdRepos );
  AddSzYControl( IDC_STATIC_DETAILS, listGrowthRatio, listGrowthRatio );
  AddSzControl( IDC_DETAILS, 0, listGrowthRatio, 100, 100 );
  AddSzControl( IDC_EDIT_TIME, mdResize, mdRepos );
}


void 
TestRunnerDlg::OnSize( UINT nType, int cx, int cy ) 
{
	cdxCDynamicDialog::OnSize(nType, cx, cy);
	updateListColumnSize();
}


void 
TestRunnerDlg::updateListColumnSize()
{
  if ( !m_listCtrl.GetSafeHwnd() )
    return;

  // resize to fit last column
  CRect listBounds = getItemClientRect( IDC_LIST );
  
  int width_1_4 = 0;
  for (int i = 0; i < 4; ++i)
    width_1_4 += m_listCtrl.GetColumnWidth( i );
  
  // the 4 offset is so no horiz scroll bar will appear
  m_listCtrl.SetColumnWidth(4, listBounds.Width() - width_1_4 - 4); 
}


void 
TestRunnerDlg::OnSelectedFailureChange( NMHDR* pNMHDR, 
                                        LRESULT* pResult )
{
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW*)pNMHDR;

  if ( (pNMListView->uNewState & LVIS_SELECTED) != 0 )  // item selected
    displayFailureDetailsFor( pNMListView->iItem );
	
	*pResult = 0;
}


void 
TestRunnerDlg::displayFailureDetailsFor( unsigned int failureIndex )
{
  CString details;
  if ( m_result  &&  failureIndex < m_result->failures().size() )
    details = m_result->failures()[ failureIndex ]->thrownException()->what();

  details.Replace( _T("\n"), _T("\r\n") );

  m_details.SetWindowText( details );
}


void 
TestRunnerDlg::initializeFixedSizeFont()
{
  LOGFONT font;
  GetFont()->GetLogFont( &font );
  font.lfPitchAndFamily = FIXED_PITCH | //VARIABLE_PITCH
                          (font.lfPitchAndFamily & ~15);   // font family
  m_fixedSizeFont.CreateFontIndirect( &font );
}
