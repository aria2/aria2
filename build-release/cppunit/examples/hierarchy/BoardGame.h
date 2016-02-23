#ifndef __BOARDGAME_H__
#define __BOARDGAME_H__

/** Example class to show hierarchy testing.
 * 
 *  Shamelessly ripped and adapted from
 *  <a href="http://c2.com/cgi/wiki?ClassHierarchyTestingInCppUnit">
 *  ClassHierarchyTestingInCppUnit</a>
 *  
 */
class BoardGame {
  public:
    /// expected to return true
    virtual bool reset();
    virtual ~BoardGame();
};

#endif
