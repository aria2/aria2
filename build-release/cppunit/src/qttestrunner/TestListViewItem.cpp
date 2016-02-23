// //////////////////////////////////////////////////////////////////////////
// Implementation file TestListViewItem.cpp for class TestListViewItem
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/09/19
// //////////////////////////////////////////////////////////////////////////

#include "TestListViewItem.h"


TestListViewItem::TestListViewItem( CPPUNIT_NS::Test *test,
                                    QListViewItem *parent ) : 
    QListViewItem( parent ),
    _test( test )
{
}


TestListViewItem::~TestListViewItem()
{
}


CPPUNIT_NS::Test *
TestListViewItem::test() const
{
  return _test;
}
