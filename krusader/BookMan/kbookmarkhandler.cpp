/**
    This file is mostly copied from kio/kfile.
    No author was mentioned in the original.

    Fri Mar 29 01:26:26 CET 2002
    Copyright 2002, kfile authors and Anders Lund <anders@alweb.dk>

*/

#include <kbookmarkimporter.h>
#include <kpopupmenu.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <kdiroperator.h>
#include <kaction.h>

#include "kbookmarkhandler.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "../Panel/listpanel.h"
#include "kbookmarkhandler.moc"

#include "addbookmarkdlg.h"
#include <kmessagebox.h>
#include <kdebug.h>


KBookmarkHandler::KBookmarkHandler( QWidget* parent, KPopupMenu* kpopupmenu )
    : QObject( parent ),
      KBookmarkOwner(),
      mParent( parent ),
      m_menu( kpopupmenu )
{
  if (!m_menu)
    m_menu = new KPopupMenu( parent, "bookmark menu" );

  // Let's find our bookmark file
  QString file = locate( "data", "krusader/krbookmarks.xml" );
  if ( file.isEmpty() ) {
    file = locateLocal( "data", "krusader/krbookmarks.xml" );
  }

  KBookmarkManager *manager = KBookmarkManager::managerForFile( file, false);
  manager->setUpdate( true );
  manager->setShowNSBookmarks( false );

  m_bookmarkMenu = new KBookmarkMenu( manager, this, m_menu, 0, true );

  newBookmarkName = "";
  newBookmarkUrl = "";
}

KBookmarkHandler::~KBookmarkHandler()
{
    // delete m_bookmarkMenu; ###
}

void KBookmarkHandler::openBookmarkURL(const KURL& url)
{
  emit openUrl( url );
}

bool KBookmarkHandler::openAddDialog()
{
  AddBookmarkDlg *addDialog = new AddBookmarkDlg();
  // set current URL as default
  QString activePath = krApp->mainView->activePanel->getPath();
  addDialog->setUrl(activePath);
  if (addDialog->exec() == QDialog::Accepted) {
    newBookmarkName = addDialog->getName().stripWhiteSpace();
    newBookmarkUrl = addDialog->getUrl().stripWhiteSpace();
    if (newBookmarkUrl == "") {
      newBookmarkUrl = activePath;
    }
    if (newBookmarkName == "") {
      newBookmarkName = newBookmarkUrl;
    }
    return true;
  }
  else {
    return false;
  }
}

QString KBookmarkHandler::currentURL()
{
  if (openAddDialog()) {
    return newBookmarkUrl;
  }
  else {
    return 0;
  }
}

QString KBookmarkHandler::currentTitle()
{
  return newBookmarkName;
}
