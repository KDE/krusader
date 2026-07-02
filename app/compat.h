/*
    SPDX-FileCopyrightText: 2019-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _COMPAT_H_
#define _COMPAT_H_

/**
 * QComboBox::activated(const QString &text) was made obsoleted in QT 5.15 in
 * favor of QComboBox::textActivated(const QString &text)
 *
 * https://doc.qt.io/qt-5.15/qcombobox-obsolete.html#activated-1
 *
 * This can be removed when the qt minimum version required will be >= 5.15
 */
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#define QCOMBOBOX_ACTIVATED textActivated
#else
#define QCOMBOBOX_ACTIVATED activated
#endif

/**
 * QComboBox::highlighted(const QString &text) was made obsoleted in QT 5.15 in
 * favor of QComboBox::textHighlighted(const QString &text)
 *
 * https://doc.qt.io/qt-5.15/qcombobox-obsolete.html#highlighted-1
 *
 * This can be removed when the qt minimum version required will be >= 5.15
 */
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#define QCOMBOBOX_HIGHLIGHTED textHighlighted
#else
#define QCOMBOBOX_HIGHLIGHTED highlighted
#endif

#endif
