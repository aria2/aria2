#include "StdAfx.h"
#include "ListCtrlSetter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



ListCtrlSetter::ListCtrlSetter( CListCtrl &list ) :
    m_List( list ),
    m_nLineNo( -1 )
{
}


ListCtrlSetter::~ListCtrlSetter()
{
}


void 
ListCtrlSetter::modifyLine( int nLineNo )
{
  editLine( nLineNo, nLineNo >= m_List.GetItemCount() );
}


void 
ListCtrlSetter::addLine()
{
  editLine( m_List.GetItemCount(), true );
}


void 
ListCtrlSetter::insertLine( int nLineNo )
{
  editLine( nLineNo, true );
}


void 
ListCtrlSetter::editLine( int nLineNo, 
                          bool bInsertLine )
{
  m_nLineNo = nLineNo;
  m_bInsertLine = bInsertLine;
  m_nNextSubItem = 0;
}


void 
ListCtrlSetter::addSubItem( const CString &strText )
{
  doAddSubItem( LVIF_TEXT, strText, 0 );
}


void 
ListCtrlSetter::addSubItem( const CString &strText, 
                            void *lParam )
{
  doAddSubItem( LVIF_TEXT | LVIF_PARAM, strText, 0, lParam );
}


void 
ListCtrlSetter::addSubItem( const CString &strText,
                            int nImage )
{
  doAddSubItem( LVIF_TEXT | LVIF_IMAGE, strText, nImage );
}


void 
ListCtrlSetter::addSubItem( const CString &strText, 
                            void *lParam,
                            int nImage )
{
  doAddSubItem( LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM, strText, 0, lParam );
}


void 
ListCtrlSetter::doAddSubItem( UINT nMask, 
                              CString strText, 
                              int nImage, 
                              void *lParam )
{
  int textLength = strText.GetLength();

  LVITEM item;
  item.mask = nMask;
  item.pszText = strText.GetBuffer( textLength );
  item.cchTextMax = textLength;
  item.iImage = nImage;
  item.lParam = (LPARAM)lParam;
  item.iItem = m_nLineNo;
  item.iSubItem = m_nNextSubItem++;
  if ( m_nNextSubItem == 1  &&
       m_bInsertLine )      // First item & new line
  {
    m_nLineNo = m_List.InsertItem( &item );
    VERIFY( m_nLineNo >= 0 );
  }
  else
  {
    VERIFY( m_List.SetItem( &item ) );
  }

  strText.ReleaseBuffer();
}


int 
ListCtrlSetter::getLineNo() const
{
  return m_nLineNo;
}
