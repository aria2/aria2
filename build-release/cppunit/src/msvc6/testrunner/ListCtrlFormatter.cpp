#include "StdAfx.h"
#include "ListCtrlFormatter.h"
#include <string>


ListCtrlFormatter::ListCtrlFormatter( CListCtrl &list ) :
    m_List( list ),
    m_nColNo( 0 )
{
}


ListCtrlFormatter::~ListCtrlFormatter()
{
}


void 
ListCtrlFormatter::AddColumn( CString strHeading,
                              int nWidth,
                              int nFormat,
                              int nSubItemNo )
{
  LVCOLUMN column;
  column.mask = LVCF_FMT | LVCF_SUBITEM  | LVCF_TEXT | LVCF_WIDTH;
  column.cx = nWidth;
  column.fmt = nFormat;
  column.pszText = strHeading.GetBuffer( strHeading.GetLength() );
  column.iSubItem = nSubItemNo == -1 ? m_nColNo : nSubItemNo;
  column.iImage = 0;
  column.iOrder = 0;
  VERIFY( m_List.InsertColumn( m_nColNo++, &column ) >= 0 );
  strHeading.ReleaseBuffer();
/*
  VERIFY( m_List.InsertColumn( m_nColNo++, 
                               strHeading, 
                               nFormat,
                               nWidth, 
                               nSubItemNo ) >= 0 );
*/
  }


void 
ListCtrlFormatter::AddColumn( const std::string &strHeading,
                              int nWidth,
                              int nFormat,
                              int nSubItemNo )
{
  AddColumn( CString( strHeading.c_str() ), nWidth, nFormat, nSubItemNo );
}


void 
ListCtrlFormatter::AddColumn( UINT nIdStringHeading,
                              int nWidth,
                              int nFormat,
                              int nSubItemNo )
{
  CString strHeading;
  VERIFY( strHeading.LoadString( nIdStringHeading ) );
  AddColumn( strHeading, nWidth, nFormat, nSubItemNo );
}


int
ListCtrlFormatter::GetNextColumnIndex() const
{
  return m_nColNo;
}
