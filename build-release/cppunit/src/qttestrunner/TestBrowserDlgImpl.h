#ifndef TESTBROWSER_H
#define TESTBROWSER_H

#include <cppunit/Test.h>
#include "testbrowserdlg.h"

class QListViewItem;

class TestBrowser : public TestBrowserBase
{ 
    Q_OBJECT

public:
  TestBrowser( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
  ~TestBrowser();

  void setRootTest( CPPUNIT_NS::Test *rootTest );

  CPPUNIT_NS::Test *selectedTest();

protected slots:
  void accept();

private:
  void insertItemFor( CPPUNIT_NS::Test *test,
                      QListViewItem *parentItem );

private:
  CPPUNIT_NS::Test *_selectedTest;
};

#endif // TESTBROWSER_H
