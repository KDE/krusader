/***************************************************************************
		    synchronizercolors.h  -  description
                             -------------------
    copyright            : (C) 2016 + by Krusader Krew
    web site             : http://www.krusader.org/
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SYNCHRONIZERCOLORS_H
#define SYNCHRONIZERCOLORS_H

#include <KGuiAddons/KColorUtils>

#define DECLARE_SYNCHRONIZER_BACKGROUND_DEFAULTS  QColor SYNCHRONIZER_BACKGROUND_DEFAULTS[] = { \
    QGuiApplication::palette().color(QPalette::Active, QPalette::Base), \
    QGuiApplication::palette().color(QPalette::Active, QPalette::Base), \
    QGuiApplication::palette().color(QPalette::Active, QPalette::Base), \
    QGuiApplication::palette().color(QPalette::Active, QPalette::Base), \
    KColorUtils::tint(QGuiApplication::palette().color(QPalette::Active, QPalette::Base), Qt::red, 0.5) \
}

#define DECLARE_SYNCHRONIZER_FOREGROUND_DEFAULTS  QColor SYNCHRONIZER_FOREGROUND_DEFAULTS[] = { \
    QGuiApplication::palette().color(QPalette::Active, QPalette::Text), \
    KColorUtils::tint(QGuiApplication::palette().color(QPalette::Active, QPalette::Text), Qt::red, 0.7), \
    KColorUtils::tint(QGuiApplication::palette().color(QPalette::Active, QPalette::Text), QColor(0, 115, 207), 0.7), \
    KColorUtils::tint(QGuiApplication::palette().color(QPalette::Active, QPalette::Text), Qt::green, 0.5), \
    QGuiApplication::palette().color(QPalette::Active, QPalette::Text) \
}

#endif /* __SYNCHRONIZERCOLORS_H__ */
