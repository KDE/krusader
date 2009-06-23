/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2000 Rafi Yanai <yanai@users.sourceforge.net>               *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef KICONS_H
#define KICONS_H

#include <QtGui/QIcon>
#include <QtGui/QPixmap>
#include <kiconloader.h>
#include <kicontheme.h>

// can be used everywhere - EXCEPT in KFileList related calls, loads the icon according to
// KIconLoader::Desktop settings and resizes it to a smaller size. If this is used to toolbuttons,
// the the icon is resized again to fit into the toolbutton or menu.
// IMPORTANT: this SHOULD NOT BE USED for actions. If creating an action, just state the file-name
// of the icon to allow automatic resizing when needed.
#define LOADICON(W, X) QIcon(krLoader->loadIcon(X,KIconLoader::Desktop)).pixmap(W->style()->pixelMetric(QStyle::PM_SmallIconSize),QIcon::Normal)

// used only for calls within the kfilelist framework, handles icon sizes
QPixmap FL_LOADICON(QString name);

extern  const char * no_xpm[];
extern  const char * yes_xpm[];
extern  const char * link_xpm[];
extern  const char * black_xpm[];
extern  const char * yellow_xpm[];
extern  const char * green_xpm[];
extern  const char * red_xpm[];
extern  const char * white_xpm[];
extern  const char * blue_xpm[];

#endif
