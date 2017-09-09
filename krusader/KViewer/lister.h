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

// QtCore
#include <QList>
#include <QTimer>
#include <QTextCodec>
#include <QMutex>
// QtGui
#include <QColor>
// QtWidgets
#include <QWidget>
#include <QShortcut>

#include <KCompletion/KLineEdit>
#include <KParts/BrowserExtension>
#include <KParts/Part>
#include <KTextWidgets/KTextEdit>

#include "../FileSystem/krquery.h"

#define  SLIDER_MAX          10000
#define  MAX_CHAR_LENGTH     4

class Lister;
class QLabel;
class QProgressBar;
class QPushButton;
class QToolButton;
class QAction;
class QTemporaryFile;
class ListerEncodingMenu;

class ListerTextArea : public KTextEdit
{
    Q_OBJECT

public:
    ListerTextArea(Lister *lister, QWidget *parent);
    void           reset();
    void           calculateText(const bool forcedUpdate = false);
    void           redrawTextArea(const bool forcedUpdate = false);

    qint64         textToFilePositionOnScreen(const int x, const int y, bool &isfirst);
    void           fileToTextPositionOnScreen(const qint64 p, const bool isfirst, int &x, int &y);

    int            tabWidth() {
        return _tabWidth;
    }
    bool           hexMode() {
        return _hexMode;
    }
    void           setHexMode(const bool hexMode);

    void           copySelectedToClipboard();
    QString        getSelectedText();
    void           clearSelection();

    void           getCursorPosition(int &x, int &y);
    qint64         getCursorPosition(bool &isfirst);
    qint64         getCursorAnchor() {
        return _cursorAnchorPos;
    }
    void           setCursorPositionInDocument(const qint64 p, const bool isfirst);
    void           ensureVisibleCursor();
    void           deleteAnchor() {
        _cursorAnchorPos = -1;
    }

    void           setAnchorAndCursor(const qint64 anchor, const qint64 cursor);
    void           sizeChanged();

protected:
    virtual void   resizeEvent(QResizeEvent * event) Q_DECL_OVERRIDE;
    virtual void   keyPressEvent(QKeyEvent * e) Q_DECL_OVERRIDE;
    virtual void   mousePressEvent(QMouseEvent * e) Q_DECL_OVERRIDE;
    virtual void   mouseDoubleClickEvent(QMouseEvent * e) Q_DECL_OVERRIDE;
    virtual void   mouseMoveEvent(QMouseEvent * e) Q_DECL_OVERRIDE;
    virtual void   wheelEvent(QWheelEvent * event) Q_DECL_OVERRIDE;

    QStringList    readLines(const qint64 filePos, qint64 &endPos, const int lines, QList<qint64> * locs = 0);
    QString        readSection(qint64 p1, qint64 p2);
    void           setUpScrollBar();
    void           setCursorPositionOnScreen(const int x, const int y, const int anchorX = -1, const int anchorY = -1);
    void           handleAnchorChange(const int oldAnchor);
    void           performAnchorChange(const int anchor);
    void           getScreenPosition(const int position, int &x, int &y);

protected slots:
    void           slotActionTriggered(const int action);
    void           slotCursorPositionChanged();
    void           zoomIn(const int range = 1);
    void           zoomOut(const int range = 1);

protected:
    Lister        *_lister;

    qint64         _screenStartPos = 0;
    qint64         _screenEndPos = 0;
    qint64         _averagePageSize = 0;

    qint64         _lastPageStartPos = 0;

    int            _sizeX = -1;
    int            _sizeY = -1;
    int            _pageSize = 0;

    int            _tabWidth = 4;

    bool           _sizeChanged = false;

    QStringList    _rowContent;
    QList<qint64>  _rowStarts;

    qint64         _cursorPos = -1;
    bool           _cursorAtFirstColumn = true;
    bool           _skipCursorChangedListener = false;

    qint64         _cursorAnchorPos = -1;

    int            _skippedLines = 0;

    bool           _inSliderOp = false;
    bool           _hexMode = false;
    bool           _redrawing = false;

    QMutex         _cursorBlinkMutex;
    QTimer         _blinkTimer;
};

class ListerBrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT

public:
    explicit ListerBrowserExtension(Lister * lister);

public slots:
    void copy();
    void print();

protected:
    Lister   *_lister;
};


class ListerPane : public QWidget
{
    Q_OBJECT

public:
    ListerPane(Lister *lister, QWidget *parent);

protected:
    virtual bool   event(QEvent *event) Q_DECL_OVERRIDE;

protected:
    bool     handleCloseEvent(QEvent *e);
    Lister        *_lister;
};


class Lister : public KParts::ReadOnlyPart
{
    Q_OBJECT

public:
    explicit Lister(QWidget *parent);

    QScrollBar     *scrollBar() {
        return _scrollBar;
    }
    ListerTextArea *textArea() {
        return _textArea;
    }

    inline qint64   fileSize() {
        return _fileSize;
    }
    QByteArray      cacheChunk(const qint64 filePos, const int maxSize);

    bool            isSearchEnabled();
    void            enableSearch(const bool);
    void            enableActions(const bool);

    QString         characterSet() { return _characterSet; }
    QTextCodec     *codec() { return _codec; }
    void            setCharacterSet(const QString set);
    void            setHexMode(const bool);

    QStringList     readHexLines(qint64 &filePos, const qint64 endPos, const int columns, const int lines);
    int             hexBytesPerLine(const int columns);
    int             hexPositionDigits();
    int             hexIndexToPosition(const int columns, const int index);
    int             hexPositionToIndex(const int columns, const int position);


public slots:
    void            searchAction() {
        enableSearch(true);
    }
    void            searchNext();
    void            searchPrev();
    void            searchDelete();
    void            jumpToPosition();
    void            saveAs();
    void            saveSelected();
    void            print();
    void            toggleHexMode();
    void            slotUpdate();

protected slots:
    void            slotSearchMore();

    void            searchSucceeded();
    void            searchFailed();
    void            searchTextChanged();

    void            slotDataSend(KIO::Job *, QByteArray &);
    void            slotSendFinished(KJob *);

protected:
    virtual bool    openUrl(const QUrl &url) Q_DECL_OVERRIDE;
    virtual bool    closeUrl() Q_DECL_OVERRIDE {
        return true;
    }
    virtual bool    openFile() Q_DECL_OVERRIDE {
        return true;
    }
    virtual void    guiActivateEvent(KParts::GUIActivateEvent * event) Q_DECL_OVERRIDE;
    void            setColor(const bool match, const bool restore);
    void            hideProgressBar();
    void            updateProgressBar();
    void            resetSearchPosition();

    qint64          getFileSize();
    void            search(const bool forward, const bool restart = false);
    QStringList     readLines(qint64 &filePos, const qint64 endPos, const int columns, const int lines);

    QTimer          _searchUpdateTimer;
    ListerTextArea *_textArea;
    QScrollBar     *_scrollBar;
    QLabel         *_listerLabel;
    KLineEdit      *_searchLineEdit;
    QProgressBar   *_searchProgressBar;
    QToolButton    *_searchStopButton;
    QPushButton    *_searchNextButton;
    QPushButton    *_searchPrevButton;
    bool            _searchInProgress = false;
    bool            _searchHexadecimal = false;
    QPushButton    *_searchOptions;
    QLabel         *_statusLabel;

    QAction        *_fromCursorAction;
    QAction        *_caseSensitiveAction;
    QAction        *_matchWholeWordsOnlyAction;
    QAction        *_regExpAction;
    QAction        *_hexAction;

    QAction        *_actionSaveSelected;
    QAction        *_actionSaveAs;
    QAction        *_actionPrint;
    QAction        *_actionSearch;
    QAction        *_actionSearchNext;
    QAction        *_actionSearchPrev;
    QAction        *_actionJumpToPosition;
    QAction        *_actionHexMode;

    QString         _filePath;
    qint64          _fileSize = 0;

    QByteArray      _cache;
    qint64          _cachePos = 0;

    KRQuery         _searchQuery;
    QByteArray      _searchHexQuery;
    qint64          _searchPosition = 0;
    bool            _searchIsForward = true;
    qint64          _searchLastFailedPosition = -1;
    int             _searchProgressCounter = 0;

    QColor          _originalBackground;
    QColor          _originalForeground;

    QString         _characterSet;
    QTextCodec     *_codec = QTextCodec::codecForLocale();

    QTemporaryFile *_tempFile = 0;

    bool            _downloading = false;
    bool            _restartFromBeginning = false;
    QTimer          _downloadUpdateTimer;

    qint64          _savePosition = 0;
    qint64          _saveEnd = 0;
};

#endif // __LISTER_H__
