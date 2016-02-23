// TestPlugInRunnerDlg.h : header file
//

#if !defined(AFX_TESTPLUGINRUNNERDLG_H__AF6DB5BC_25E5_4459_8A54_9704298F64FF__INCLUDED_)
#define AFX_TESTPLUGINRUNNERDLG_H__AF6DB5BC_25E5_4459_8A54_9704298F64FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include <TestRunnerDlg.h>
#include <TestRunnerModel.h>
#include "TestPlugInRunnerModel.h"
#include <list>
#include <string>

/////////////////////////////////////////////////////////////////////////////
// TestPlugInRunnerDlg dialog

class TestPlugInRunnerDlg : public TestRunnerDlg
{
// Construction
public:
  TestPlugInRunnerDlg( TestPlugInRunnerModel *model,
                       CWnd* pParent = NULL);

// Dialog Data
  //{{AFX_DATA(TestPlugInRunnerDlg)
  //}}AFX_DATA

  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(TestPlugInRunnerDlg)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  HICON m_hIcon;

  // Generated message map functions
  //{{AFX_MSG(TestPlugInRunnerDlg)
  virtual BOOL OnInitDialog();
  afx_msg void OnPaint();
  afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void OnChooseDll();
  afx_msg void OnReloadDll();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP();

protected:
  virtual void initializeLayout();


private:
  TestPlugInRunnerModel &plugInModel();

  static std::list<std::string> getCommandLineArguments();
  void loadPluginIfNesseccary();
  void loadDll( std::string path );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTPLUGINRUNNERDLG_H__AF6DB5BC_25E5_4459_8A54_9704298F64FF__INCLUDED_)
