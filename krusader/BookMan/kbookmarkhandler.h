/**
    This file is mostly copied from kio/kfile.
    No author was mentioned in the original.

    Slightly modified by Anders Lund.

    Fri Mar 29 01:26:26 CET 2002
    Copyright 2002, kfile authors and Anders Lund <anders@alweb.dk>

    Thanks to the kfile folks;)
*/


#ifndef _KBOOKMARKHANDLER_H_
#define _KBOOKMARKHANDLER_H_

#include "kbookmarkmanager.h"
#include "kbookmarkmenu.h"

class KPopupMenu;
class KActionMenu;

class KBookmarkHandler : public QObject, public KBookmarkOwner
{
  Q_OBJECT

public:
  KBookmarkHandler( QWidget* parent, KPopupMenu *kpopupmenu=0 );
  ~KBookmarkHandler();

  /**
   * This function is called when the user selects a bookmark.
  */
  virtual void openBookmarkURL( const KURL& url );

  /**
   * This method is called whenever the user wants to add the
   * current location to the bookmarks list. It is called
   * after currentURL().
   *
   * @author Jan Halasa
   * @returns QString that will become the name of the bookmark.
  */
  virtual QString currentTitle();

  /**
   * This method is called whenever the user wants to add the
   * current location to the bookmarks list.
   * It is called before currentTitle().
   *
   * @author Jan Halasa
   * @returns QString that will become the URL of the bookmark.
  */
  virtual QString currentURL();

  KPopupMenu *menu() const { return m_menu; }

signals:
  void openUrl(const KURL& url );

private:
  /**
   * Opens the dialog for entering a name and an URL of the new bookmark.
   *
   * @author Jan Halasa
  */
  bool openAddDialog();

  QWidget *mParent;
  KPopupMenu *m_menu;
  KBookmarkMenu *m_bookmarkMenu;

  QString newBookmarkName;
  QString newBookmarkUrl;
};


#endif // _KBOOKMARKHANDLER_H_
