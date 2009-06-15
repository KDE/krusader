/***************************************************************************
                          lister.h  -  description
                             -------------------
    copyright            : (C) 2009 + by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
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

#ifndef LISTER_H
#define LISTER_H

#include <QWidget>
#include <QtGui/QTextEdit>
#include <QtCore/QTimer>
#include <QList>
#include <QColor>

#include <kparts/part.h>
#include <kparts/browserextension.h>

#include "../VFS/krquery.h"

#define  LISTER_CACHE_FACTOR 3
#define  SLIDER_MAX          10000
#define  MAX_CHAR_LENGTH     4

class QTextEdit;
class Lister;
class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;
class QProgressBar;
class KTemporaryFile;
class KAction;

class ListerTextArea : public QTextEdit
{
    Q_OBJECT

public:
    ListerTextArea(Lister *lister, QWidget *parent);
    void           reset();
    void           calculateText(bool forcedUpdate = false);
    void           redrawTextArea(bool forcedUpdate = false);

    qint64         textToFilePosition(int x, int y, bool &isfirst);
    void           fileToTextPosition(qint64 p, bool isfirst, int &x, int &y);

    QTextCodec   * codec();

    void           copySelectedToClipboard();

    void           getCursorPosition(int &x, int &y);
    qint64         getCursorPosition(bool &isfirst);
    void           setCursorPosition(qint64 p, bool isfirst);
    void           ensureVisibleCursor();
    void           deleteAnchor() {
        _cursorAnchorPos = -1;
    }

    void           setAnchorAndCursor(qint64 anchor, qint64 cursor);
    void           sizeChanged();

protected:
    virtual void   resizeEvent(QResizeEvent * event);
    virtual void   keyPressEvent(QKeyEvent * e);
    virtual void   mousePressEvent(QMouseEvent * e);
    virtual void   mouseDoubleClickEvent(QMouseEvent * e);
    virtual void   mouseMoveEvent(QMouseEvent * e);
    virtual void   wheelEvent(QWheelEvent * event);

    QStringList    readLines(qint64 filePos, qint64 &endPos, int lines, QList<qint64> * locs = 0);
    QString        readSection(qint64 p1, qint64 p2);
    void           setUpScrollBar();
    void           setCursorPosition(int x, int y, int anchorX = -1, int anchorY = -1);
    void           handleAnchorChange(int oldAnchor);
    void           getScreenPosition(int position, int &x, int &y);

protected slots:
    void           slotActionTriggered(int action);
    void           slotCursorPositionChanged();

protected:
    Lister        *_lister;

    qint64         _screenStartPos;
    qint64         _screenEndPos;
    qint64         _averagePageSize;

    qint64         _lastPageStartPos;

    int            _sizeX;
    int            _sizeY;
    int            _pageSize;

    int            _tabWidth;

    bool           _sizeChanged;

    QStringList    _rowContent;
    QList<qint64>  _rowStarts;

    qint64         _cursorPos;
    bool           _cursorAtFirstColumn;

    qint64         _cursorAnchorPos;

    int            _skippedLines;

    bool           _inSliderOp;
    bool           _inCursorUpdate;
};

class ListerBrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT

public:
    ListerBrowserExtension(Lister * lister);

public slots:
    void copy();

protected:
    Lister   *_lister;
};


class Lister : public KParts::ReadOnlyPart
{
    Q_OBJECT

public:
    Lister(QWidget *parent);
    ~Lister();

    QScrollBar     *scrollBar() {
        return _scrollBar;
    }
    ListerTextArea *textArea() {
        return _textArea;
    }

    inline qint64   fileSize() {
        return _fileSize;
    }
    char *          cacheRef(qint64 filePos, int &size);

    void            enableSearch(bool);
    void            enableActions(bool);

public slots:
    void            searchAction() {
        enableSearch(true);
    }
    void            searchNext();
    void            searchPrev();
    void            jumpToPosition();

protected slots:
    void            slotUpdate();
    void            slotSearchMore();

    void            searchSucceeded();
    void            searchFailed();
    void            searchDelete();

    void            slotFileDataReceived(KIO::Job *, const QByteArray &);
    void            slotFileFinished(KJob *);

protected:
    virtual bool    openUrl(const KUrl &url);
    virtual bool    closeUrl() {
        return true;
    }
    virtual bool    openFile() {
        return true;
    }
    virtual void    guiActivateEvent(KParts::GUIActivateEvent * event);
    void            setColor(bool match, bool restore);
    void            hideProgressBar();
    void            updateProgressBar();

    qint64          getFileSize();
    void            search(bool forward);

    QTimer          _updateTimer;
    ListerTextArea *_textArea;
    QScrollBar     *_scrollBar;
    QLabel         *_listerLabel;
    QLineEdit      *_searchLineEdit;
    QProgressBar   *_searchProgressBar;
    QToolButton    *_searchStopButton;
    QPushButton    *_searchNextButton;
    QPushButton    *_searchPrevButton;
    bool            _searchInProgress;
    QPushButton    *_searchOptions;
    QLabel         *_statusLabel;

    QAction        *_fromCursorAction;
    QAction        *_caseSensitiveAction;
    QAction        *_matchWholeWordsOnlyAction;
    QAction        *_regExpAction;

    KAction        *_actionSearch;
    KAction        *_actionSearchNext;
    KAction        *_actionSearchPrev;
    KAction        *_actionJumpToPosition;

    QString         _filePath;
    qint64          _fileSize;

    char           *_cache;
    int             _cacheSize;
    qint64          _cachePos;

    bool            _active;

    KRQuery         _searchQuery;
    qint64          _searchPosition;
    bool            _searchIsForward;
    qint64          _searchLastFailedPosition;
    int             _searchProgressCounter;

    QColor          _originalBackground;
    QColor          _originalForeground;

    KTemporaryFile *_tempFile;
};

#endif // __LISTER_H__
