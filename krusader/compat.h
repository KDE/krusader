/*****************************************************************************
 * Copyright (C) 2019-2020 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#ifndef _COMPAT_H_
#define _COMPAT_H_

#include <kio_version.h>

#if KIO_VERSION >= QT_VERSION_CHECK(5, 48, 0)
    #define UDS_ENTRY_INSERT(A, B) UDSEntry::fastInsert((A), (B));
#else
    #define UDS_ENTRY_INSERT(A, B) UDSEntry::insert((A), (B));
#endif

/**
 * QFontMetrics::width(const QString&, int) was made obsoleted in QT 5.11 in
 * favor of QFontMetrics::horizontalAdvance(const QString &, int)
 *
 * https://doc.qt.io/archives/qt-5.11/qfontmetrics-obsolete.html#width
 *
 * This can be removed when the qt minimum version required will be >= 5.11
 */
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    #define QFONTMETRICS_WIDTH(A) horizontalAdvance(A)
#else
    #define QFONTMETRICS_WIDTH(A) width(A)
#endif

#endif
