#include "krbookmarkbutton.h"
#include "../krusader.h"
#include <qpixmap.h>
#include <kiconloader.h>
#include <klocale.h>

KrBookmarkButton::KrBookmarkButton(QWidget *parent): QToolButton(parent) {
	QPixmap icon = krLoader->loadIcon("bookmark", KIcon::Toolbar, 16);
	setFixedSize(icon.width() + 4, icon.height() + 4);
	setPixmap(icon);
	setTextLabel(i18n("BookMan II"), true);
	setPopupDelay(10); // 0.01 seconds press
	setAcceptDrops(false);
}

