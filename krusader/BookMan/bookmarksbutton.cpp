/***************************************************************************
                          bookmarksbutton.cpp  -  description
                             -------------------
    begin                : Ne jún 8 2003
    copyright            : (C) 2003 by Ján Hala?a
    email                : xhalasa@fi.muni.cz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "bookmarksbutton.h"

#include <kaction.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

BookmarksButton::BookmarksButton(QWidget *parent) : QToolButton(parent)
{
  KIconLoader *iconLoader = new KIconLoader();
  QPixmap icon = iconLoader->loadIcon("bookmark", KIcon::Toolbar, 16);

  setFixedSize(icon.width() + 4, icon.height() + 4);
  setPixmap(icon);
  setTextLabel(i18n("Open your bookmarks"), true);
  setPopupDelay(10); // 0.01 seconds press
  setAcceptDrops(false);

  KActionMenu *acmBookmarks = new KActionMenu(i18n("Bookmarks"), "bookmark", 0, "bookmarks");
  acmBookmarks->setDelayed(false);

  bookmarkHandler = new KBookmarkHandler(this, acmBookmarks->popupMenu());
  QObject::connect(bookmarkHandler, SIGNAL(openUrl(const QString&)), this, SIGNAL(openUrl(const QString&)));
  setPopup(acmBookmarks->popupMenu());
}

BookmarksButton::~BookmarksButton(){
}
