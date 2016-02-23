// //////////////////////////////////////////////////////////////////////////
// Header file TestFailureListViewItem.h for class TestFailureListViewItem
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/20
// //////////////////////////////////////////////////////////////////////////
#ifndef TESTFAILURELISTVIEWITEM_H
#define TESTFAILURELISTVIEWITEM_H

#include <qlistview.h>
class TestFailureInfo;


/*! \class TestFailureListViewItem
 * \brief This class represents a test failure item.
 */
class TestFailureListViewItem : public QListViewItem
{
public:
  /*! Constructs a TestFailureListViewItem object.
   */
  TestFailureListViewItem( TestFailureInfo *failure,
                           QListView *parent );

  /*! Destructor.
   */
  virtual ~TestFailureListViewItem();

  TestFailureInfo *failure();

private:
  /// Prevents the use of the copy constructor.
  TestFailureListViewItem( const TestFailureListViewItem &copy );

  /// Prevents the use of the copy operator.
  void operator =( const TestFailureListViewItem &copy );

private:
  TestFailureInfo *_failure;
};



// Inlines methods for TestFailureListViewItem:
// --------------------------------------------



#endif  // TESTFAILURELISTVIEWITEM_H
