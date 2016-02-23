#ifndef LISTCTRLFORMATTER_H
#define LISTCTRLFORMATTER_H

#include <string>

/*! \brief Helper to setup ListCtrl columns format.
 */
class ListCtrlFormatter
{
public:
  /*!
   * Constructor.
   * \param list List control to setup.
   */
  ListCtrlFormatter( CListCtrl &list );

  /*!
   * Destructeur.
   */
  virtual ~ListCtrlFormatter();

  /*! Adds a column to the list control.
   * \param strHeading Column title.
   * \param nWidth Column width in pixel. (-1 if not defined).
   * \param nFormat Text alignment (LVCFMT_LEFT, LVCFMT_RIGHT, LVCFMT_CENTER).
   * \param nSubItemNo Index of the sub-item associates to the column.
   */
  void AddColumn( const std::string &strHeading,
                  int nWidth =-1,
                  int nFormat = LVCFMT_LEFT,
                  int nSubItemNo =-1 );

  /*! Adds a column to the list control.
   * \param strHeading Column title.
   * \param nWidth Column width in pixel. (-1 if not defined).
   * \param nFormat Text alignment (LVCFMT_LEFT, LVCFMT_RIGHT, LVCFMT_CENTER).
   * \param nSubItemNo Index of the sub-item associates to the column.
   */
  void AddColumn( CString strHeading,
                  int nWidth =-1,
                  int nFormat = LVCFMT_LEFT,
                  int nSubItemNo =-1 );

  /*! Adds a column to the list control.
   * \param nIdStringHeading Resource ID of the column title string (IDS_xxx).
   * \param nWidth Column width in pixel. (-1 if not defined).
   * \param nFormat Text alignment (LVCFMT_LEFT, LVCFMT_RIGHT, LVCFMT_CENTER).
   * \param nSubItemNo Index of the sub-item associates to the column.
   */
  void AddColumn( UINT nIdStringHeading,
                  int nWidth =-1,
                  int nFormat = LVCFMT_LEFT,
                  int nSubItemNo =-1 );

  /*! Gets the sub item index of the next column.
   * \return Sub item index of the next column, starting with 0.
   */
  int GetNextColumnIndex() const;

private:
  /*! Associated list control.
   */
  CListCtrl &m_List;

  /*! Next column number.
   */
  int m_nColNo;
};


#endif //WILLISTCTRLFORMATTER_H
