#include "krbookmarkbutton.h"
#include "krbookmarkhandler.h"
#include "../krusader.h"
#include <qpixmap.h>
#include <kiconloader.h>
#include <kaction.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kdebug.h>

KrBookmarkButton::KrBookmarkButton(QWidget *parent): QToolButton(parent) {
	QPixmap icon = krLoader->loadIcon("bookmark", KIcon::Toolbar, 16);
	setFixedSize(icon.width() + 4, icon.height() + 4);
	setPixmap(icon);
	setTextLabel(i18n("BookMan II"), true);
	setPopupDelay(10); // 0.01 seconds press
	setAcceptDrops(false);

	acmBookmarks = new KActionMenu(i18n("Bookmarks"), "bookmark", 0, 0);
	acmBookmarks->setDelayed(false);
	acmBookmarks->popupMenu()->setKeyboardShortcutsEnabled(true);
	acmBookmarks->popupMenu()->setKeyboardShortcutsExecute(true);

	setPopup(acmBookmarks->popupMenu());
	connect(this, SIGNAL(pressed()), this, SLOT(populate()));
}

void KrBookmarkButton::populate() {
	krBookMan->populate(static_cast<KPopupMenu*>(popup()));
}

void KrBookmarkButton::openPopup() {
	populate();
	popup()->exec(mapToGlobal(QPoint(0, height())));
  kdWarning() << "here" << endl;
  //emit pressed();
 
}
