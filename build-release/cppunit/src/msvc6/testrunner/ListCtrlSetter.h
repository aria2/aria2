#ifndef LISTCTRLSETTER_H
#define LISTCTRLSETTER_H

#include <string>


/*! \brief Helper to set up the content of a list control.
 */
class ListCtrlSetter
{
public:
  /*!
   * Constructor.
   * \param list List control to fill.
   */
  ListCtrlSetter( CListCtrl &list );

  /*!
   * Destructor.
   */
  virtual ~ListCtrlSetter();


  /*! Modifies the specified line.
   * \param nLineNo number of the line to modify.
   */
  void modifyLine( int nLineNo );

  /*! Adds a new line at the end of the list.
   */
  void addLine();

  /*! Insert a new line in the list.
   * \param nLineNo Line number before the new line is insert.
   */
  void insertLine( int nLineNo );


  void addSubItem( const CString &strText );

  void addSubItem( const CString &strText, void *lParam );

  void addSubItem( const CString &strText, int nImage );

  void addSubItem( const CString &strText, void *lParam, int nImage );

  /*! Gets the number of the line being modified.
   * \return Number of the line being modified.
   */
  int getLineNo() const;

private:
  /*! Edit a line.
   * \param nLineNo Number of the line to edit.
   * \param bInsertLine \c true if the line is inserted, \c false if it is modified.
   */
  void editLine( int nLineNo, 
                 bool bInsertLine );

  /*! Add a sub-item.
   * \param nMask Mask LV_IF... to set.
   * \param strText Sub-item Text.
   * \param nImage Image number.
   * \param lParam Item data pointer.
   */
  void doAddSubItem( UINT nMask, 
                     CString strText, 
                     int nImage, 
                     void *lParam =NULL );

private:
  /*! List control which content is being set up.
   */
  CListCtrl &m_List;

  /*! Current line number (line being edited).
   */
  int m_nLineNo;

  /*! Line should be inserted ?
   */
  bool m_bInsertLine;

  /*! Next sub-item number.
   */
  int m_nNextSubItem;
};


#endif //LISTCTRLSETTER_H
