/*
    SPDX-FileCopyrightText: 2009 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LISTER_H
#define LISTER_H

// QtCore
#include <QList>
#include <QMutex>
#include <QTextCodec>
#include <QTimer>
// QtGui
#include <QColor>
// QtWidgets
#include <QShortcut>
#include <QWidget>

#include <KLineEdit>
#include <KParts/NavigationExtension>
#include <KParts/Part>
#include <KTextEdit>

#include "../FileSystem/krquery.h"

#define SLIDER_MAX 10000
#define MAX_CHAR_LENGTH 4

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
    void reset();
    void calculateText(const bool forcedUpdate = false);
    void redrawTextArea(const bool forcedUpdate = false);

    qint64 textToFilePositionOnScreen(const int x, const int y, bool &isfirst);
    void fileToTextPositionOnScreen(const qint64 p, const bool isfirst, int &x, int &y);

    int tabWidth()
    {
        return _tabWidth;
    }
    bool hexMode()
    {
        return _hexMode;
    }
    void setHexMode(const bool hexMode);

    void copySelectedToClipboard();
    QString getSelectedText();
    void clearSelection();

    void getCursorPosition(int &x, int &y);
    qint64 getCursorPosition(bool &isfirst);
    qint64 getCursorAnchor()
    {
        return _cursorAnchorPos;
    }
    void setCursorPositionInDocument(const qint64 p, const bool isfirst);
    void ensureVisibleCursor();
    void deleteAnchor()
    {
        _cursorAnchorPos = -1;
    }

    void setAnchorAndCursor(const qint64 anchor, const qint64 cursor);
    void sizeChanged();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *event) override;

    QStringList readLines(const qint64 filePos, qint64 &endPos, const int lines, QList<qint64> *locs = nullptr);
    QString readSection(qint64 p1, qint64 p2);
    void setUpScrollBar();
    void setCursorPositionOnScreen(const int x, const int y, const int anchorX = -1, const int anchorY = -1);
    void handleAnchorChange(const int oldAnchor);
    void performAnchorChange(const int anchor);
    void getScreenPosition(const int position, int &x, int &y);

public slots:
    void slotActionTriggered(const int action);

protected slots:
    void slotCursorPositionChanged();
    void zoomIn(const int range = 1);
    void zoomOut(const int range = 1);

protected:
    Lister *_lister;

    qint64 _screenStartPos = 0;
    qint64 _screenEndPos = 0;
    qint64 _averagePageSize = 0;

    qint64 _lastPageStartPos = 0;

    int _sizeX = -1;
    int _sizeY = -1;
    int _pageSize = 0;

    int _tabWidth = 4;

    bool _sizeChanged = false;

    QStringList _rowContent;
    QList<qint64> _rowStarts;

    qint64 _cursorPos = -1;
    bool _cursorAtFirstColumn = true;
    bool _skipCursorChangedListener = false;

    qint64 _cursorAnchorPos = -1;

    int _skippedLines = 0;

    bool _inSliderOp = false;
    bool _hexMode = false;
    bool _redrawing = false;

    QMutex _cursorBlinkMutex;
    QTimer _blinkTimer;
};

class ListerBrowserExtension : public KParts::NavigationExtension
{
    Q_OBJECT

public:
    explicit ListerBrowserExtension(Lister *lister);

public slots:
    void copy();
    void print();

protected:
    Lister *_lister;
};

class ListerPane : public QWidget
{
    Q_OBJECT

public:
    ListerPane(Lister *lister, QWidget *parent);

protected:
    bool event(QEvent *event) override;

protected:
    bool handleCloseEvent(QEvent *e);
    Lister *_lister;
};

class Lister : public KParts::ReadOnlyPart
{
    Q_OBJECT

public:
    explicit Lister(QWidget *parent);

    QScrollBar *scrollBar()
    {
        return _scrollBar;
    }
    ListerTextArea *textArea()
    {
        return _textArea;
    }

    inline qint64 fileSize()
    {
        return _fileSize;
    }
    QByteArray cacheChunk(const qint64 filePos, const qint64 maxSize);

    bool isSearchEnabled();
    void enableSearch(const bool);
    void enableActions(const bool);

    QString characterSet()
    {
        return _characterSet;
    }
    QTextCodec *codec()
    {
        return _codec;
    }
    void setCharacterSet(const QString &set);
    void setHexMode(const bool);

    QStringList readHexLines(qint64 &filePos, const qint64 endPos, const int columns, const int lines);
    int hexBytesPerLine(const int columns);
    int hexPositionDigits();
    int hexIndexToPosition(const int columns, const int index);
    int hexPositionToIndex(const int columns, const int position);

public slots:
    void searchAction()
    {
        enableSearch(true);
    }
    void searchNext();
    void searchPrev();
    void searchDelete();
    void jumpToPosition();
    void saveAs();
    void saveSelected();
    void print();
    void toggleHexMode();
    void slotUpdate();

protected slots:
    void slotSearchMore();

    void searchSucceeded();
    void searchFailed();
    void searchTextChanged();

    void slotDataSend(KIO::Job *, QByteArray &);
    void slotSendFinished(KJob *);

protected:
    bool openUrl(const QUrl &url) override;
    bool closeUrl() override
    {
        return true;
    }
    bool openFile() override
    {
        return true;
    }
    void guiActivateEvent(KParts::GUIActivateEvent *event) override;
    void setColor(const bool match, const bool restore);
    void hideProgressBar();
    void updateProgressBar();
    void resetSearchPosition();

    qint64 getFileSize();
    void search(const bool forward, const bool restart = false);
    QStringList readLines(qint64 &filePos, const qint64 endPos, const int columns, const int lines);

    QTimer _searchUpdateTimer;
    ListerTextArea *_textArea;
    QScrollBar *_scrollBar;
    QLabel *_listerLabel;
    KLineEdit *_searchLineEdit;
    QProgressBar *_searchProgressBar;
    QToolButton *_searchStopButton;
    QPushButton *_searchNextButton;
    QPushButton *_searchPrevButton;
    bool _searchInProgress = false;
    bool _searchHexadecimal = false;
    QPushButton *_searchOptions;
    QLabel *_statusLabel;

    QAction *_fromCursorAction;
    QAction *_caseSensitiveAction;
    QAction *_matchWholeWordsOnlyAction;
    QAction *_regExpAction;
    QAction *_hexAction;

    QAction *_actionSaveSelected;
    QAction *_actionSaveAs;
    QAction *_actionPrint;
    QAction *_actionSearch;
    QAction *_actionSearchNext;
    QAction *_actionSearchPrev;
    QAction *_actionJumpToPosition;
    QAction *_actionHexMode;

    QString _filePath;
    qint64 _fileSize = 0;

    QByteArray _cache;
    qint64 _cachePos = 0;

    KrQuery _searchQuery;
    QByteArray _searchHexQuery;
    qint64 _searchPosition = 0;
    bool _searchIsForward = true;
    qint64 _searchLastFailedPosition = -1;
    int _searchProgressCounter = 0;

    QColor _originalBackground;
    QColor _originalForeground;

    QString _characterSet;
    QTextCodec *_codec = QTextCodec::codecForLocale();

    QTemporaryFile *_tempFile = nullptr;

    bool _downloading = false;
    bool _restartFromBeginning = false;
    QTimer _downloadUpdateTimer;

    qint64 _savePosition = 0;
    qint64 _saveEnd = 0;
};

#endif // __LISTER_H__
