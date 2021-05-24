/*
    SPDX-FileCopyrightText: 2019-2020 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _COMPAT_H_
#define _COMPAT_H_

#include <kio_version.h>

#if KIO_VERSION >= QT_VERSION_CHECK(5, 48, 0)
    #define UDS_ENTRY_INSERT(A, B) UDSEntry::fastInsert((A), (B));
#else
    #define UDS_ENTRY_INSERT(A, B) UDSEntry::insert((A), (B));
#endif

/**
 * QLineEdit::selectionLength() is not present in QT < 5.10
 * 
 * This can be removed when the qt minimum version required will be >= 5.10
 */
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    #define QLINEEDIT_SELECTIONLENGTH selectionLength()
#else
    #define QLINEEDIT_SELECTIONLENGTH selectedText().length()
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
 * This can be removed when the qt minimum version required will be >= 5.13
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
 * This can be removed when the qt minimum version required will be >= 5.14
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
 * This can be removed when the qt minimum version required will be >= 5.14
 */
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    #define QCOMBOBOX_HIGHLIGHTED textHighlighted
#else
    #define QCOMBOBOX_HIGHLIGHTED highlighted
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

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    #define SET_TAB_STOP_DISTANCE(X) setTabStopDistance(X)
#else
    #define SET_TAB_STOP_DISTANCE(X) setTabStopWidth(X)
#endif

#endif
