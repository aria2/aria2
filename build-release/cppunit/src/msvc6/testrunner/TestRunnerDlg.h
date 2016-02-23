#if !defined(AFX_TESTRUNNERDLG_H)
#define AFX_TESTRUNNERDLG_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TestRunnerDlg.h : header file
//

/* Refer to MSDN documentation:
   mk:@MSITStore:h:\DevStudio\MSDN\98VSa\1036\vcmfc.chm::/html/_mfcnotes_tn033.htm#_mfcnotes_how_to_write_an_mfc_extension_dll
   to know how to write and use MFC extension DLL
   Can be found in the index with "mfc extension"
   =>
   Using:
   - your application must link  Multithreaded MFC DLL
   - memory allocation is done using the same heap
   - you must define the symbol _AFX_DLL
   Building:
   - you must define the symbol _AFX_DLL and _AFX_EXT
 */

// Define the folowing symbol to subclass TestRunnerDlg
#ifndef CPPUNIT_SUBCLASSING_TESTRUNNERDLG_BUILD
#include "resource.h"
#else
#define IDD_DIALOG_TESTRUNNER 0
#endif

#include <vector>
#include <cppunit/TestSuite.h>
#include <cppunit/Exception.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestListener.h>
#include <cppunit/TestResultCollector.h>

#include "ActiveTest.h"
#include "MsDevCallerListCtrl.h"
#include "TestRunnerModel.h"
#include "DynamicWindow/cdxCDynamicDialog.h"

class ProgressBar;
class TestRunnerModel;


/////////////////////////////////////////////////////////////////////////////
// TestRunnerDlg dialog

class TestRunnerDlg : public cdxCDynamicDialog,
                      public CPPUNIT_NS::TestListener
{
public:
  TestRunnerDlg( TestRunnerModel *model,
                int nDialogResourceId,
                CWnd* pParent = NULL);
  TestRunnerDlg( TestRunnerModel *model,
                const TCHAR* szDialogResourceId = NULL,
                CWnd* pParent = NULL);
  virtual ~TestRunnerDlg();

  // overrided from TestListener;
  void startTest( CPPUNIT_NS::Test *test );
  void addFailure( const CPPUNIT_NS::TestFailure &failure );
  void endTest( CPPUNIT_NS::Test *test );

  // IDD is not use, it is just there for the wizard.
  //{{AFX_DATA(TestRunnerDlg)
	CEdit	m_details;
  MsDevCallerListCtrl m_listCtrl;
  CButton m_buttonClose;
  CButton m_buttonStop;
  CButton m_buttonRun;
  CButton m_buttonBrowse;
  BOOL m_bAutorunAtStartup;
	//}}AFX_DATA

  //{{AFX_VIRTUAL(TestRunnerDlg)
public:
  virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

protected:

  //{{AFX_MSG(TestRunnerDlg)
  virtual BOOL OnInitDialog();
  afx_msg void OnRun();
  afx_msg void OnStop();
  virtual void OnOK();
  afx_msg void OnBrowseTest();
  afx_msg void OnQuitApplication();
  afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelectedFailureChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelectTestInHistoryCombo();
	//}}AFX_MSG
  DECLARE_MESSAGE_MAP()

  typedef std::vector<CPPUNIT_NS::Test *> Tests;
  ProgressBar *m_testsProgress;
  CPPUNIT_NS::Test *m_selectedTest;
  ActiveTest *m_activeTest;
  CPPUNIT_NS::TestResult *m_testObserver;
  CPPUNIT_NS::TestResultCollector *m_result;
  int m_testsRun;
  int m_errors;
  int m_failures;
  DWORD m_testStartTime;
  DWORD m_testEndTime;
  static const CString ms_cppunitKey;
  HACCEL m_hAccelerator;
  bool m_bIsRunning;
  TestRunnerModel *m_model;
  CImageList m_errorListBitmap;
  CFont m_fixedSizeFont;

  enum ErrorTypeBitmaps
  {
    errorTypeFailure =0,
    errorTypeError
  };

  void addListEntry( const CPPUNIT_NS::TestFailure &failure );
  void beIdle();
  void beRunning();
  void beRunDisabled();
  void reset();
  void freeState();
  void updateCountsDisplay();
  void setupHistoryCombo();
  CPPUNIT_NS::Test *findTestByName( std::string name ) const;
  CPPUNIT_NS::Test *findTestByNameFor( const std::string &name, 
                                    CPPUNIT_NS::Test *test ) const;
  void addNewTestToHistory( CPPUNIT_NS::Test *test );
  void addTestToHistoryCombo( CPPUNIT_NS::Test *test, 
                              int idx =-1 );
  void removeTestFromHistory( CPPUNIT_NS::Test *test );
  CComboBox *getHistoryCombo();
  void updateSelectedItem();
  void saveHistory();
  void loadSettings();
  void saveSettings();
  TestRunnerModel &model();
  void updateHistoryCombo();
  void displayFailureDetailsFor( unsigned int failureIndex );

  CRect getItemWindowRect( unsigned int itemId );
  CRect getItemClientRect( unsigned int itemId );

  //CRect getDialogBounds();

  virtual void initializeLayout();
  void updateListColumnSize();
  void initializeFixedSizeFont();


private:
  TestRunnerModel::Settings m_settings;

  /// do all initialization, that is usually done in the constructor, so that the
  /// code is not duplicated in the two constructors
  void TestRunnerDlg::init(TestRunnerModel *model);
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTRUNNERDLG_H)
