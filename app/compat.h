/*
    SPDX-FileCopyrightText: 2019-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _COMPAT_H_
#define _COMPAT_H_

#include <kio_version.h>

#include <karchive_version.h>
#include <kcompletion_version.h>

/**
 * KActionMenu::setDelayed(bool) was made deprecated since 5.77 in
 * favor of KActionMenu::setPopupMode(QToolButton::ToolButtonPopupMode)
 *
 * This can be removed when the frameworks minimum version required will be >= 5.77
 */
#if KIO_VERSION >= QT_VERSION_CHECK(5, 77, 0)
#define KACTIONMENU_SETDELAYED setPopupMode(QToolButton::InstantPopup)
#else
#define KACTIONMENU_SETDELAYED setDelayed(false)
#endif

/**
 * KLineEdit::returnPressed has been deprecated since 5.81
 * in favor of KLineEdit::returnKeyPressed.
 *
 * This can be removed when the frameworks minimum version required will be >= 5.81
 */
#if KCOMPLETION_VERSION >= QT_VERSION_CHECK(5, 81, 0)
#define KLINEEDIT_RETURNKEYPRESSED returnKeyPressed
#else
#define KLINEEDIT_RETURNKEYPRESSED returnPressed
#endif

/**
 * KFilterDev::compressionTypeForMimeType has been deprecated since 5.85
 * in favor of KLineEdit::returnKeyPressed.
 *
 * This can be removed when the frameworks minimum version required will be >= 5.85
 */
#if KARCHIVE_VERSION >= QT_VERSION_CHECK(5, 85, 0)
#define COMPRESSIONTYPEFORMIMETYPE KCompressionDevice::compressionTypeForMimeType
#else
#define COMPRESSIONTYPEFORMIMETYPE KFilterDev::compressionTypeForMimeType
#endif

/**
 * QButtonGroup::buttonClicked(int id) was made obsoleted in QT 5.15 in
 * favor of QButtonGroup::idClicked(int id)
 *
 * https://doc.qt.io/qt-5.15/qbuttongroup-obsolete.html#buttonClicked-1
 *
 * This can be removed when the qt minimum version required will be >= 5.15
 */
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#define QBUTTONGROUP_BUTTONCLICKED idClicked
#else
#define QBUTTONGROUP_BUTTONCLICKED buttonClicked
#endif

/**
 * QResource::isCompressed() was made obsoleted in QT 5.15 in
 * favor of QResource::Compression QResource::compressionAlgorithm()
 *
 * https://doc.qt.io/qt-5.15/qresource-obsolete.html#isCompressed
 *
 * This can be removed when the qt minimum version required will be >= 5.15
 */
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#define QRESOURCE_ISCOMPRESSED(A) ((A).compressionAlgorithm() != QResource::NoCompression)
#else
#define QRESOURCE_ISCOMPRESSED(A) (A).isCompressed()
#endif

/**
 * QString::split(QChar sep, QString::SplitBehavior behavior, Qt::CaseSensitivity cs = Qt::CaseSensitive)
 * was made obsoleted in QT 5.15 in favor of the namespaced Qt::endl
 *
 * https://doc.qt.io/qt-5.15/qstring-obsolete.html#split-2
 *
 * This can be removed when the qt minimum version required will be >= 5.15
 */
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#define SKIP_EMPTY_PARTS Qt::SkipEmptyParts
#else
#define SKIP_EMPTY_PARTS QString::SkipEmptyParts
#endif

/**
 * QTextSteam::endl() was made obsoleted in QT 5.15 in
 * favor of the namespaced Qt::endl
 *
 * https://doc.qt.io/qt-5.15/qtextstream-obsolete.html#endl
 *
 * This can be removed when the qt minimum version required will be >= 5.15
 */
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#define QT_ENDL Qt::endl
#else
#define QT_ENDL endl
#endif

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
