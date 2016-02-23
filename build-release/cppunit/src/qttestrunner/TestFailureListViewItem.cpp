// //////////////////////////////////////////////////////////////////////////
// Implementation file TestFailureListViewItem.cpp for class TestFailureListViewItem
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/20
// //////////////////////////////////////////////////////////////////////////

#include "TestFailureListViewItem.h"


TestFailureListViewItem::TestFailureListViewItem(
                           TestFailureInfo *failure,
                           QListView *parent ) : 
    QListViewItem( parent ),
    _failure( failure )
{
	setMultiLinesEnabled (true);
}


TestFailureListViewItem::~TestFailureListViewItem()
{
}


TestFailureInfo *
TestFailureListViewItem::failure()
{
  return _failure;
}
