/***************************************************************************
                         lister.cpp  -  description
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "lister.h"

#include <QtCore/QFile>
#include <QtCore/QRect>
#include <QtCore/QDate>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QtGui/QClipboard>
#include <QtGui/QSpacerItem>
#include <QtGui/QProgressBar>
#include <QtGui/QKeyEvent>
#include <QtGui/QScrollBar>
#include <QtGui/QHBoxLayout>
#include <QtGui/QMenu>
#include <QtGui/QPrintDialog>
#include <QtGui/QPrinter>

#include <kuiserverjobtracker.h>
#include <KColorScheme>
#include <KTemporaryFile>
#include <KMessageBox>
#include <KActionCollection>
#include <KInputDialog>
#include <KFileDialog>
#include <KLocale>
#include <KGlobalSettings>
#include <KCharsets>
#include <KIO/Job>
#include <KIO/CopyJob>
#include <KIO/JobUiDelegate>

#include "../krusader.h"
#include "../GUI/krremoteencodingmenu.h"

#define  SEARCH_CACHE_CHARS 100000
#define  SEARCH_MAX_ROW_LEN 4000
#define  CONTROL_CHAR       752
#define  CACHE_SIZE         100000

ListerTextArea::ListerTextArea(Lister *lister, QWidget *parent) : KTextEdit(parent), _lister(lister),
        _sizeX(-1), _sizeY(-1), _cursorAnchorPos(-1), _inSliderOp(false), _inCursorUpdate(false), _hexMode(false)
{
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(slotCursorPositionChanged()));
    _tabWidth = 4;
    setWordWrapMode(QTextOption::NoWrap);
    setLineWrapMode(QTextEdit::NoWrap);
}

void ListerTextArea::reset()
{
    _screenStartPos = 0;
    _cursorPos = 0;
    _cursorAnchorPos = -1;
    _cursorAtFirstColumn = true;
    calculateText();
}

void ListerTextArea::sizeChanged()
{
    if (_cursorAnchorPos > _lister->fileSize())
        _cursorAnchorPos = -1;
    if (_cursorPos > _lister->fileSize())
        _cursorPos = _lister->fileSize();

    redrawTextArea(true);
}

void ListerTextArea::resizeEvent(QResizeEvent * event)
{
    KTextEdit::resizeEvent(event);
    redrawTextArea();
}

void ListerTextArea::calculateText(bool forcedUpdate)
{
    QFontMetrics fm(font());

    int fontHeight = fm.height();
    if (fontHeight < 1)
        fontHeight = 1;

    QRect crect = viewport()->contentsRect();
    int windowHeight = crect.height();

    int sizeY = windowHeight / fontHeight;
    _pageSize = sizeY;

    QList<qint64> rowStarts;

    int fontWidth = fm.width("W");
    if (fontWidth < 1)
        fontWidth = 1;
    int windowWidth = crect.width() - fontWidth / 2;
    if (windowWidth < 1)
        windowWidth = 0;

    setTabStopWidth(fontWidth * _tabWidth);

    int sizeX = windowWidth / fontWidth;

    _sizeChanged = (_sizeY != sizeY) || (_sizeX != sizeX) || forcedUpdate;
    _sizeY = sizeY;
    _sizeX = sizeX;

    QStringList list = readLines(_screenStartPos, _screenEndPos, _sizeY, &rowStarts);

    if (_sizeChanged) {
        _averagePageSize = _screenEndPos - _screenStartPos;
        setUpScrollBar();
    }

    rowStarts << _screenEndPos;

    QStringList listRemn = readLines(_screenEndPos, _screenEndPos, 1);
    list << listRemn;

    if (list != _rowContent) {
        setPlainText(list.join("\n"));
        _rowContent = list;
        _rowStarts = rowStarts;
    }
}

qint64 ListerTextArea::textToFilePosition(int x, int y, bool &isfirst)
{
    isfirst = (x == 0);
    if (y >= _rowStarts.count())
        return -1;
    qint64 rowStart = _rowStarts[ y ];
    if (x == 0)
        return rowStart;

    if (_hexMode) {
        qint64 pos = rowStart + _lister->hexPositionToIndex(_sizeX, x);
        if (pos > _lister->fileSize())
            pos = _lister->fileSize();
        return pos;
    }

    // we can't use fromUnicode because of the invalid encoded chars
    int maxBytes = 2 * _sizeX * MAX_CHAR_LENGTH;
    char * cache = _lister->cacheRef(rowStart, maxBytes);
    QByteArray cachedBuffer(cache, maxBytes);

    QTextStream stream(&cachedBuffer);
    stream.setCodec(codec());
    stream.read(x);
    return rowStart + stream.pos();
}

void ListerTextArea::fileToTextPosition(qint64 p, bool isfirst, int &x, int &y)
{
    if (p < _screenStartPos || p > _screenEndPos || _rowStarts.count() < 1) {
        x = -1;
        y = (p > _screenEndPos) ? -2 : -1;
    } else {
        y = 0;
        while (y < _rowStarts.count() && _rowStarts[ y ] <= p)
            y++;
        y--;
        if (y < 0) {
            x = y = -1;
            return;
        }

        qint64 rowStart = _rowStarts[ y ];
        if (_hexMode) {
            x = _lister->hexIndexToPosition(_sizeX, (int)(p - rowStart));
            return;
        }

        int maxBytes = 2 * _sizeX * MAX_CHAR_LENGTH;
        x = 0;
        if (rowStart >= p) {
            if ((rowStart == p) && !isfirst && y > 0) {
                qint64 previousRow = _rowStarts[ y - 1 ];
                char * cache = _lister->cacheRef(previousRow, maxBytes);
                QByteArray cachedBuffer(cache, p - previousRow);

                QTextStream stream(&cachedBuffer);
                stream.setCodec(codec());
                stream.read(_rowContent[ y - 1].length());
                if (previousRow + stream.pos() == p) {
                    y--;
                    x = _rowContent[ y ].length();
                }
            }
            return;
        }

        char * cache = _lister->cacheRef(rowStart, maxBytes);
        QByteArray cachedBuffer(cache, p - rowStart);

        QString res = codec()->toUnicode(cachedBuffer);
        x = res.length();
    }
}

void ListerTextArea::getCursorPosition(int &x, int &y)
{
    getScreenPosition(textCursor().position(), x, y);
}

void ListerTextArea::getScreenPosition(int position, int &x, int &y)
{
    x = position;
    y = 0;
    for (int i = 0; i < _rowContent.count(); i++) {
        int rowLen = _rowContent[ i ].length() + 1;
        if (x >= rowLen) {
            x -= rowLen;
            y++;
        } else
            break;
    }
}

void ListerTextArea::setCursorPosition(int x, int y, int anchorX, int anchorY)
{
    _inCursorUpdate = true;
    if (x == -1 || y < 0) {
        setCursorWidth(0);
        if (anchorY == -1) {
            _inCursorUpdate = false;
            return;
        }

        if (y == -2) {
            y = _sizeY;
            x = (_rowContent.count() > _sizeY) ? _rowContent[ _sizeY ].length() : 0;
        } else
            x = y = 0;
    } else
        setCursorWidth(2);

    QTextCursor::MoveMode mode = QTextCursor::MoveAnchor;
    if (anchorX != -1 && anchorY != -1) {
        if (anchorY >= _sizeY) {
            moveCursor(QTextCursor::End, mode);
            moveCursor(QTextCursor::StartOfLine, mode);
        } else {
            moveCursor(QTextCursor::Start, mode);
            for (int i = 0; i < anchorY; i++)
                moveCursor(QTextCursor::Down, mode);
        }

        if (_rowContent.count() > anchorY && anchorX > _rowContent[ anchorY ].length())
            anchorX = _rowContent[ anchorY ].length();

        for (int j = 0; j < anchorX; j++)
            moveCursor(QTextCursor::Right, mode);

        mode = QTextCursor::KeepAnchor;
    }

    if (y >= _sizeY) {
        moveCursor(QTextCursor::End, mode);
        moveCursor(QTextCursor::StartOfLine, mode);
    } else {
        moveCursor(QTextCursor::Start, mode);
        for (int i = 0; i < y; i++)
            moveCursor(QTextCursor::Down, mode);
    }

    if (_rowContent.count() > y && x > _rowContent[ y ].length())
        x = _rowContent[ y ].length();

    for (int j = 0; j < x; j++)
        moveCursor(QTextCursor::Right, mode);

    _inCursorUpdate = false;
}

qint64 ListerTextArea::getCursorPosition(bool &isfirst)
{
    if (cursorWidth() == 0) {
        isfirst = _cursorAtFirstColumn;
        return _cursorPos;
    }

    int x, y;
    getCursorPosition(x, y);
    return textToFilePosition(x, y, isfirst);
}

void ListerTextArea::setCursorPosition(qint64 p, bool isfirst)
{
    _cursorPos = p;
    int x, y;
    fileToTextPosition(p, isfirst, x, y);

    int anchorX = -1, anchorY = -1;
    if (_cursorAnchorPos != -1 && _cursorAnchorPos != p) {
        int anchPos = _cursorAnchorPos;
        bool anchorBelow = false, anchorAbove = false;
        if (anchPos < _screenStartPos) {
            anchPos = _screenStartPos;
            anchorY = -2;
            anchorAbove = true;
        }
        if (anchPos > _screenEndPos) {
            anchPos = _screenEndPos;
            anchorY = -3;
            anchorBelow = true;
        }

        fileToTextPosition(anchPos, isfirst, anchorX, anchorY);
        if (anchorAbove && _hexMode)
            anchorX = 0;
        if (anchorBelow && _hexMode && _rowContent.count() > 0)
            anchorX = _rowContent[ 0 ].length();
    }
    setCursorPosition(x, y, anchorX, anchorY);
}

void ListerTextArea::slotCursorPositionChanged()
{
    if (_inCursorUpdate)
        return;
    int cursorX, cursorY;
    getCursorPosition(cursorX, cursorY);
    _cursorAtFirstColumn = (cursorX == 0);
    _cursorPos = textToFilePosition(cursorX, cursorY, _cursorAtFirstColumn);
    setCursorWidth(2);
    //fprintf( stderr, "Cursor pos: %d %d %Ld\n", cursorX, cursorY, _cursorPos );
}

QString ListerTextArea::readSection(qint64 p1, qint64 p2)
{
    if (p1 == p2)
        return QString();

    if (p1 > p2) {
        qint64 tmp = p1;
        p1 = p2;
        p2 = tmp;
    }

    QString section;
    qint64 pos = p1;

    if (_hexMode) {
        while (p1 != p2) {
            QStringList list = _lister->readHexLines(p1, p2, _sizeX, 1);
            if (list.count() == 0)
                break;
            if (!section.isEmpty())
                section += QChar('\n');
            section += list[ 0 ];
        }
        return section;
    }

    QTextCodec * textCodec = codec();
    QTextDecoder * decoder = textCodec->makeDecoder();

    do {
        int maxBytes = _sizeX * _sizeY * MAX_CHAR_LENGTH;
        if (maxBytes > (p2 - pos))
            maxBytes = (int)(p2 - pos);
        char * cache = _lister->cacheRef(pos, maxBytes);
        if (cache == 0 || maxBytes == 0)
            break;
        section += decoder->toUnicode(cache, maxBytes);
        pos += maxBytes;
    } while (pos < p2);

    delete decoder;
    return section;
}

QStringList ListerTextArea::readLines(qint64 filePos, qint64 &endPos, int lines, QList<qint64> * locs)
{
    endPos = filePos;
    QStringList list;

    if (_hexMode) {
        endPos = _lister->fileSize();
        if (filePos >= endPos)
            return list;
        qint64 startPos = filePos;
        int bytes = _lister->hexBytesPerLine(_sizeX);
        filePos = startPos = ((startPos) / bytes) * bytes;
        list = _lister->readHexLines(filePos, endPos, _sizeX, lines);
        endPos = filePos;
        if (locs) {
            for (int i = 0; i != list.count(); i++) {
                (*locs) << startPos;
                startPos += bytes;
            }
        }
        return list;
    }

    int maxBytes = _sizeX * _sizeY * MAX_CHAR_LENGTH;
    char * cache = _lister->cacheRef(filePos, maxBytes);
    if (cache == 0 || maxBytes == 0)
        return list;

    QTextCodec * textCodec = codec();
    QTextDecoder * decoder = textCodec->makeDecoder();

    int cnt = 0;
    int y = 0;
    QString row = "";
    int effLength = 0;
    if (locs)
        (*locs) << filePos;
    bool isLastLongLine = false;
    while (cnt < maxBytes && y < lines) {
        int lastCnt = cnt;
        QString chr = decoder->toUnicode(cache + (cnt++), 1);
        if (!chr.isEmpty()) {
            if ((chr[ 0 ] < 32) && (chr[ 0 ] != '\n') && (chr[ 0 ] != '\t'))
                chr = QChar(CONTROL_CHAR);
            if (chr == "\n") {
                if (!isLastLongLine) {
                    list << row;
                    effLength = 0;
                    row = "";
                    y++;
                    if (locs)
                        (*locs) << filePos + cnt;
                }
                isLastLongLine = false;
            } else {
                isLastLongLine = false;
                if (chr == "\t") {
                    effLength += _tabWidth - (effLength % _tabWidth) - 1;
                    if (effLength > _sizeX) {
                        list << row;
                        effLength = 0;
                        row = "";
                        y++;
                        if (locs)
                            (*locs) << filePos + lastCnt;
                    }
                }
                row += chr;
                effLength++;
                if (effLength >= _sizeX) {
                    list << row;
                    effLength = 0;
                    row = "";
                    y++;
                    if (locs)
                        (*locs) << filePos + cnt;
                    isLastLongLine = true;
                }
            }
        }
    }

    if (y < lines)
        list << row;

    if (locs) {
        while (locs->count() > lines) {
            locs->removeLast();
        }
    }

    endPos = filePos + cnt;

    delete decoder;
    return list;
}

QTextCodec * ListerTextArea::codec()
{
    QString cs = _lister->characterSet();
    if (cs.isEmpty())
        return QTextCodec::codecForLocale();
    else
        return KGlobal::charsets()->codecForName(cs);
}

void ListerTextArea::setUpScrollBar()
{
    if (_averagePageSize == _lister->fileSize()) {
        _lister->scrollBar()->setPageStep(0);
        _lister->scrollBar()->setMaximum(0);
        _lister->scrollBar()->hide();
        _lastPageStartPos = 0;
    } else {
        int maxPage = MAX_CHAR_LENGTH * _sizeX * _sizeY;
        qint64 pageStartPos = _lister->fileSize() - maxPage;
        qint64 endPos;
        if (pageStartPos < 0)
            pageStartPos = 0;
        QStringList list = readLines(pageStartPos, endPos, maxPage);
        if (list.count() <= _sizeY) {
            _lastPageStartPos = 0;
        } else {
            readLines(pageStartPos, _lastPageStartPos, list.count() - _sizeY);
        }

        int maximum = (_lastPageStartPos > SLIDER_MAX) ? SLIDER_MAX : _lastPageStartPos;
        int pageSize = (_lastPageStartPos > SLIDER_MAX) ? SLIDER_MAX * _averagePageSize / _lastPageStartPos : _averagePageSize;
        if (pageSize == 0)
            pageSize++;

        _lister->scrollBar()->setPageStep(pageSize);
        _lister->scrollBar()->setMaximum(maximum);
        _lister->scrollBar()->show();
    }
}

void ListerTextArea::keyPressEvent(QKeyEvent * ke)
{
    if (Krusader::actCopy->shortcut().contains(QKeySequence(ke->key() | ke->modifiers()))) {
        copySelectedToClipboard();
        ke->accept();
        return;
    }

    if (ke->modifiers() == Qt::NoModifier || ke->modifiers() == Qt::ShiftModifier) {
        qint64 newAnchor = -1;
        if (ke->modifiers() == Qt::ShiftModifier) {
            newAnchor = _cursorAnchorPos;
            if (_cursorAnchorPos == -1)
                newAnchor = _cursorPos;
        }

        switch (ke->key()) {
        case Qt::Key_F3:
            ke->accept();
            if (ke->modifiers() == Qt::ShiftModifier)
                _lister->searchPrev();
            else
                _lister->searchNext();
            return;
        case Qt::Key_Home:
        case Qt::Key_End:
            _cursorAnchorPos = newAnchor;
            break;
        case Qt::Key_Left: {
            _cursorAnchorPos = newAnchor;
            ensureVisibleCursor();
            int x, y;
            getCursorPosition(x, y);
            if (y == 0 && x == 0)
                slotActionTriggered(QAbstractSlider::SliderSingleStepSub);
        }
        break;
        case Qt::Key_Right: {
            _cursorAnchorPos = newAnchor;
            ensureVisibleCursor();
            if (textCursor().position() == toPlainText().length())
                slotActionTriggered(QAbstractSlider::SliderSingleStepAdd);
        }
        break;
        case Qt::Key_Up: {
            _cursorAnchorPos = newAnchor;
            ensureVisibleCursor();
            int x, y;
            getCursorPosition(x, y);
            if (y == 0)
                slotActionTriggered(QAbstractSlider::SliderSingleStepSub);
        }
        break;
        case Qt::Key_Down: {
            _cursorAnchorPos = newAnchor;
            ensureVisibleCursor();
            int x, y;
            getCursorPosition(x, y);
            if (y >= _sizeY)
                slotActionTriggered(QAbstractSlider::SliderSingleStepAdd);
        }
        break;
        case Qt::Key_PageDown: {
            _cursorAnchorPos = newAnchor;
            ensureVisibleCursor();
            ke->accept();
            int x, y;
            getCursorPosition(x, y);
            slotActionTriggered(QAbstractSlider::SliderPageStepAdd);
            y += _sizeY - _skippedLines;
            if (y > _rowContent.count()) {
                y = _rowContent.count();
                if (y > 0)
                    x = _rowContent[ y - 1 ].length();
                else
                    x = 0;
            }
            _cursorPos = textToFilePosition(x, y, _cursorAtFirstColumn);
            setCursorPosition(_cursorPos, _cursorAtFirstColumn);
        }
        return;
        case Qt::Key_PageUp: {
            _cursorAnchorPos = newAnchor;
            ensureVisibleCursor();
            ke->accept();
            int x, y;
            getCursorPosition(x, y);
            slotActionTriggered(QAbstractSlider::SliderPageStepSub);
            y -= _sizeY - _skippedLines;
            if (y < 0) {
                y = 0;
                x = 0;
            }
            _cursorPos = textToFilePosition(x, y, _cursorAtFirstColumn);
            setCursorPosition(_cursorPos, _cursorAtFirstColumn);
        }
        return;
        }
    }
    if (ke->modifiers() == Qt::ControlModifier) {
        switch (ke->key()) {
        case Qt::Key_G:
            ke->accept();
            _lister->jumpToPosition();
            return;
        case Qt::Key_F:
            ke->accept();
            _lister->enableSearch(true);
            return;
        case Qt::Key_Home:
            _cursorAnchorPos = -1;
            ke->accept();
            slotActionTriggered(QAbstractSlider::SliderToMinimum);
            setCursorPosition((qint64)0, true);
            return;
        case Qt::Key_A:
        case Qt::Key_End: {
            _cursorAnchorPos = (ke->key() == Qt::Key_A) ? 0 : -1;
            ke->accept();
            slotActionTriggered(QAbstractSlider::SliderToMaximum);
            qint64 endPos = _lister->fileSize();
            setCursorPosition(endPos, true);
            return;
        }
        case Qt::Key_Down:
            ke->accept();
            slotActionTriggered(QAbstractSlider::SliderSingleStepAdd);
            return;
        case Qt::Key_Up:
            ke->accept();
            slotActionTriggered(QAbstractSlider::SliderSingleStepSub);
            return;
        case Qt::Key_PageDown:
            ke->accept();
            slotActionTriggered(QAbstractSlider::SliderPageStepAdd);
            return;
        case Qt::Key_PageUp:
            ke->accept();
            slotActionTriggered(QAbstractSlider::SliderPageStepSub);
            return;
        }
    }
    int oldAnchor = textCursor().anchor();
    KTextEdit::keyPressEvent(ke);
    handleAnchorChange(oldAnchor);
}

void ListerTextArea::mousePressEvent(QMouseEvent * e)
{
    _cursorAnchorPos = -1;
    int oldAnchor = textCursor().anchor();
    KTextEdit::mousePressEvent(e);
    handleAnchorChange(oldAnchor);
}

void ListerTextArea::mouseDoubleClickEvent(QMouseEvent * e)
{
    _cursorAnchorPos = -1;
    int oldAnchor = textCursor().anchor();
    KTextEdit::mouseDoubleClickEvent(e);
    handleAnchorChange(oldAnchor);
}

void ListerTextArea::mouseMoveEvent(QMouseEvent * e)
{
    if (e->pos().y() < 0) {
        slotActionTriggered(QAbstractSlider::SliderSingleStepSub);
    } else if (e->pos().y() > height()) {
        slotActionTriggered(QAbstractSlider::SliderSingleStepAdd);
    }
    if (_cursorAnchorPos == -1)
        _cursorAnchorPos = _cursorPos;
    KTextEdit::mouseMoveEvent(e);
    if (_cursorAnchorPos == _cursorPos)
        _cursorAnchorPos = -1;
}

void ListerTextArea::wheelEvent(QWheelEvent * e)
{
    int delta = e->delta();
    if (delta) {
        if (delta > 0) {
            e->accept();
            while (delta > 0) {
                slotActionTriggered(QAbstractSlider::SliderSingleStepSub);
                slotActionTriggered(QAbstractSlider::SliderSingleStepSub);
                slotActionTriggered(QAbstractSlider::SliderSingleStepSub);
                delta -= 120;
            }
        } else {
            e->accept();
            while (delta < 0) {
                slotActionTriggered(QAbstractSlider::SliderSingleStepAdd);
                slotActionTriggered(QAbstractSlider::SliderSingleStepAdd);
                slotActionTriggered(QAbstractSlider::SliderSingleStepAdd);
                delta += 120;
            }
        }
    }
}

void ListerTextArea::slotActionTriggered(int action)
{
    switch (action) {
    case QAbstractSlider::SliderSingleStepAdd: {
        qint64 endPos;
        readLines(_screenStartPos, endPos, 1);
        if (endPos <= _lastPageStartPos) {
            _screenStartPos = endPos;
        }
    }
    break;
    case QAbstractSlider::SliderSingleStepSub: {
        if (_screenStartPos == 0)
            break;

        if (_hexMode) {
            int bytesPerRow = _lister->hexBytesPerLine(_sizeX);
            _screenStartPos = (_screenStartPos / bytesPerRow) * bytesPerRow;
            _screenStartPos -= bytesPerRow;
            if (_screenStartPos < 0)
                _screenStartPos = 0;
        } else {
            int maxSize = _sizeX * _sizeY * MAX_CHAR_LENGTH;
            QByteArray encodedEnter = codec()->fromUnicode(QString("\n"));

            qint64 readPos = _screenStartPos - maxSize;
            if (readPos < 0)
                readPos = 0;
            maxSize = _screenStartPos - readPos;

            char * cache = _lister->cacheRef(readPos, maxSize);
            QByteArray backBuffer(cache, maxSize);

            int from = maxSize;
            while (from > 0) {
                from--;
                from = backBuffer.lastIndexOf(encodedEnter, from);
                if (from == -1)
                    break;
                int backRef = from - 20;
                if (backRef < 0)
                    backRef = 0;
                int size = from - backRef + encodedEnter.size();
                QString decoded = codec()->toUnicode(cache + backRef, size);
                if (decoded.endsWith(QLatin1String("\n"))) {
                    if (from < (maxSize - encodedEnter.size())) {
                        from += encodedEnter.size();
                        break;
                    }
                }
            }

            if (from == -1)
                from = 0;
            readPos += from;

            qint64 previousPos = readPos;

            while (readPos < _screenStartPos) {
                previousPos = readPos;
                readLines(readPos, readPos, 1);
            }

            _screenStartPos = previousPos;
        }
    }
    break;
    case QAbstractSlider::SliderPageStepAdd: {
        _skippedLines = 0;

        qint64 endPos;
        for (int i = 0; i < _sizeY; i++) {
            readLines(_screenStartPos, endPos, 1);
            if (endPos <= _lastPageStartPos) {
                _screenStartPos = endPos;
                _skippedLines++;
            } else {
                break;
            }
        }
    }
    break;
    case QAbstractSlider::SliderPageStepSub: {
        _skippedLines = 0;

        if (_screenStartPos == 0)
            break;

        if (_hexMode) {
            int bytesPerRow = _lister->hexBytesPerLine(_sizeX);
            _screenStartPos = (_screenStartPos / bytesPerRow) * bytesPerRow;
            _screenStartPos -= _sizeY * bytesPerRow;
            if (_screenStartPos < 0)
                _screenStartPos = 0;
        } else {
            int maxSize = 2 * _sizeX * _sizeY * MAX_CHAR_LENGTH;
            QByteArray encodedEnter = codec()->fromUnicode(QString("\n"));

            qint64 readPos = _screenStartPos - maxSize;
            if (readPos < 0)
                readPos = 0;
            maxSize = _screenStartPos - readPos;

            char * cache = _lister->cacheRef(readPos, maxSize);
            QByteArray backBuffer(cache, maxSize);

            int sizeY = _sizeY + 1;
            int origSizeY = sizeY;
            int from = maxSize;
            int lastEnter = maxSize;

        repeat:     while (from > 0) {
                from--;
                from = backBuffer.lastIndexOf(encodedEnter, from);
                if (from == -1)
                    break;
                int backRef = from - 20;
                if (backRef < 0)
                    backRef = 0;
                int size = from - backRef + encodedEnter.size();
                QString decoded = codec()->toUnicode(cache + backRef, size);
                if (decoded.endsWith(QLatin1String("\n"))) {
                    if (from < (maxSize - encodedEnter.size())) {
                        int arrayStart = from + encodedEnter.size();
                        decoded = codec()->toUnicode(cache + arrayStart, lastEnter - arrayStart);
                        sizeY -= ((decoded.length() / (_sizeX + 1)) + 1);
                        if (sizeY < 0) {
                            from = arrayStart;
                            break;
                        }
                    }
                    lastEnter = from;
                }
            }

            if (from == -1)
                from = 0;
            qint64 searchPos = readPos + from;

            QList<qint64> locs;
            while (searchPos < _screenStartPos) {
                locs << searchPos;
                readLines(searchPos, searchPos, 1);
            }

            if (locs.count() >= _sizeY)
                _screenStartPos = locs[ locs.count() - _sizeY ];
            else if (from != 0) {
                origSizeY += locs.count() + 1;
                sizeY = origSizeY;
                goto repeat;
            } else if (readPos == 0)
                _screenStartPos = 0;
        }
    }
    break;
    case QAbstractSlider::SliderToMinimum:
        _screenStartPos = 0;
        break;
    case QAbstractSlider::SliderToMaximum:
        _screenStartPos = _lastPageStartPos;
        break;
    case QAbstractSlider::SliderMove: {
        if (_inSliderOp)  // self created call?
            return;
        qint64 pos = _lister->scrollBar()->sliderPosition();

        if (pos == SLIDER_MAX) {
            _screenStartPos = _lastPageStartPos;
            break;
        } else if (pos == 0) {
            _screenStartPos = 0;
            break;
        }

        if (_lastPageStartPos > SLIDER_MAX)
            pos = _lastPageStartPos * pos / SLIDER_MAX;

        if (pos != 0) {
            if (_hexMode) {
                int bytesPerRow = _lister->hexBytesPerLine(_sizeX);
                pos = (pos / bytesPerRow) * bytesPerRow;
            } else {
                int maxSize = _sizeX * _sizeY * MAX_CHAR_LENGTH;
                qint64 readPos = pos - maxSize;
                if (readPos < 0)
                    readPos = 0;
                qint64 previousPos = readPos;

                while (readPos <= pos) {
                    previousPos = readPos;
                    readLines(readPos, readPos, 1);
                }

                pos = previousPos;
            }
        }

        _screenStartPos = pos;
    }
    break;
    case QAbstractSlider::SliderNoAction:
        break;
    };

    _inSliderOp = true;
    int value = (_lastPageStartPos > SLIDER_MAX) ? SLIDER_MAX * _screenStartPos / _lastPageStartPos : _screenStartPos;
    _lister->scrollBar()->setSliderPosition(value);
    _inSliderOp = false;

    redrawTextArea();
}

void ListerTextArea::redrawTextArea(bool forcedUpdate)
{
    bool isfirst;
    qint64 pos = getCursorPosition(isfirst);
    calculateText(forcedUpdate);
    setCursorPosition(pos, isfirst);
}

void ListerTextArea::ensureVisibleCursor()
{
    if (_cursorPos < _screenStartPos || _cursorPos > _screenEndPos) {
        int delta = _sizeY / 2;
        if (delta == 0)
            delta++;

        qint64 newScreenStart = _cursorPos;
        while (delta) {
            int maxSize = _sizeX * MAX_CHAR_LENGTH;
            qint64 readPos = newScreenStart - maxSize;
            if (readPos < 0)
                readPos = 0;

            qint64 previousPos = readPos;

            while (readPos < newScreenStart) {
                previousPos = readPos;
                readLines(readPos, readPos, 1);
                if (readPos == previousPos)
                    break;
            }

            newScreenStart = previousPos;
            delta--;
        }
        if (newScreenStart > _lastPageStartPos)
            newScreenStart = _lastPageStartPos;

        _screenStartPos = newScreenStart;
        slotActionTriggered(QAbstractSlider::SliderNoAction);
    }
}

void ListerTextArea::setAnchorAndCursor(qint64 anchor, qint64 cursor)
{
    _cursorAnchorPos = anchor;
    setCursorPosition(cursor, false);
    ensureVisibleCursor();
}

void ListerTextArea::copySelectedToClipboard()
{
    if (_cursorAnchorPos != -1 && _cursorAnchorPos != _cursorPos) {
        QString selection = readSection(_cursorAnchorPos, _cursorPos);
        QApplication::clipboard()->setText(selection);
    }
}

void ListerTextArea::handleAnchorChange(int oldAnchor)
{
    int cursor = textCursor().position();
    int anchor = textCursor().anchor();

    if (anchor == cursor) {
        _cursorAnchorPos = -1;
    } else {
        if (oldAnchor != anchor) {
            int x, y;
            bool isfirst;
            getScreenPosition(anchor, x, y);
            _cursorAnchorPos = textToFilePosition(x, y, isfirst);
        }
    }
}

void ListerTextArea::setHexMode(bool hexMode)
{
    bool isfirst;
    qint64 pos = getCursorPosition(isfirst);
    _hexMode = hexMode;
    _screenStartPos = 0;
    calculateText(true);
    setCursorPosition(pos, isfirst);
    ensureVisibleCursor();
}

ListerBrowserExtension::ListerBrowserExtension(Lister * lister) : KParts::BrowserExtension(lister)
{
    _lister = lister;

    emit enableAction("copy", true);
    emit enableAction("print", true);
}

void ListerBrowserExtension::copy()
{
    _lister->textArea()->copySelectedToClipboard();
}

void ListerBrowserExtension::print()
{
    _lister->print();
}

class ListerEncodingMenu : public KrRemoteEncodingMenu
{
public:
    ListerEncodingMenu(Lister *lister, const QString &text, const QString &icon, KActionCollection *parent) :
            KrRemoteEncodingMenu(text, icon, parent), _lister(lister) {
    }

protected:
    virtual QString currentCharacterSet() {
        return _lister->characterSet();
    }

    virtual void chooseDefault() {
        _lister->setCharacterSet(QString());
    }

    virtual void chooseEncoding(QString encodingName) {
        QString charset = KGlobal::charsets()->encodingForName(encodingName);
        _lister->setCharacterSet(charset);
    }

    Lister * _lister;
};

Lister::Lister(QWidget *parent) : KParts::ReadOnlyPart(parent), _searchInProgress(false), _cache(0), _active(false), _searchLastFailedPosition(-1),
        _searchProgressCounter(0), _tempFile(0), _downloading(false)
{
    setXMLFile("krusaderlisterui.rc");

    _actionSaveSelected = new KAction(KIcon("document-save"), i18n("Save selection..."), this);
    connect(_actionSaveSelected, SIGNAL(triggered(bool)), SLOT(saveSelected()));
    actionCollection()->addAction("save_selected", _actionSaveSelected);

    _actionSaveAs = new KAction(KIcon("document-save-as"), i18n("Save as..."), this);
    connect(_actionSaveAs, SIGNAL(triggered(bool)), SLOT(saveAs()));
    actionCollection()->addAction("save_as", _actionSaveAs);

    _actionPrint = new KAction(KIcon("document-print"), i18n("Print..."), this);
    _actionPrint->setShortcut(Qt::CTRL + Qt::Key_P);
    connect(_actionPrint, SIGNAL(triggered(bool)), SLOT(print()));
    actionCollection()->addAction("print", _actionPrint);

    _actionSearch = new KAction(KIcon("system-search"), i18n("Search"), this);
    _actionSearch->setShortcut(Qt::CTRL + Qt::Key_F);
    connect(_actionSearch, SIGNAL(triggered(bool)), SLOT(searchAction()));
    actionCollection()->addAction("search", _actionSearch);

    _actionSearchNext = new KAction(KIcon("go-down"), i18n("Search next"), this);
    _actionSearchNext->setShortcut(Qt::Key_F3);
    connect(_actionSearchNext, SIGNAL(triggered(bool)), SLOT(searchNext()));
    actionCollection()->addAction("search_next", _actionSearchNext);

    _actionSearchPrev = new KAction(KIcon("go-up"), i18n("Search previous"), this);
    _actionSearchPrev->setShortcut(Qt::SHIFT + Qt::Key_F3);
    connect(_actionSearchPrev, SIGNAL(triggered(bool)), SLOT(searchPrev()));
    actionCollection()->addAction("search_prev", _actionSearchPrev);

    _actionJumpToPosition = new KAction(KIcon("go-jump"), i18n("Jump to position"), this);
    _actionJumpToPosition->setShortcut(Qt::CTRL + Qt::Key_G);
    connect(_actionJumpToPosition, SIGNAL(triggered(bool)), SLOT(jumpToPosition()));
    actionCollection()->addAction("jump_to_position", _actionJumpToPosition);

    _actionHexMode = new KAction(KIcon("document-preview"), i18n("Hex mode"), this);
    _actionHexMode->setShortcut(Qt::CTRL + Qt::Key_H);
    connect(_actionHexMode, SIGNAL(triggered(bool)), SLOT(toggleHexMode()));
    actionCollection()->addAction("hex_mode", _actionHexMode);

    _actionEncoding = new ListerEncodingMenu(this, i18n("Select charset"), "character-set", actionCollection());

    QWidget * widget = new QWidget(parent);
    widget->setFocusPolicy(Qt::StrongFocus);
    QGridLayout *grid = new QGridLayout(widget);
    _textArea = new ListerTextArea(this, widget);
    _textArea->setFont(KGlobalSettings::fixedFont());
    _textArea->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    _textArea->setCursorWidth(2);
    _textArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _textArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    widget->setFocusProxy(_textArea);
    grid->addWidget(_textArea, 0, 0);
    _scrollBar = new QScrollBar(Qt::Vertical, widget);
    grid->addWidget(_scrollBar, 0, 1);
    _scrollBar->hide();

    QWidget * statusWidget = new QWidget(widget);
    QHBoxLayout *hbox = new QHBoxLayout(statusWidget);

    _listerLabel = new QLabel(i18n("Lister:"), statusWidget);
    hbox->addWidget(_listerLabel);
    _searchProgressBar = new QProgressBar(statusWidget);
    _searchProgressBar->setMinimum(0);
    _searchProgressBar->setMaximum(1000);
    _searchProgressBar->setValue(0);
    _searchProgressBar->hide();
    hbox->addWidget(_searchProgressBar);

    _searchStopButton = new QToolButton(statusWidget);
    _searchStopButton->setIcon(KIcon("process-stop"));
    _searchStopButton->setToolTip(i18n("Stop search"));
    _searchStopButton->hide();
    connect(_searchStopButton, SIGNAL(clicked()), this, SLOT(searchDelete()));
    hbox->addWidget(_searchStopButton);

    _searchLineEdit = new KLineEdit(statusWidget);
    _searchLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    _originalBackground = _searchLineEdit->palette().color(QPalette::Base);
    _originalForeground = _searchLineEdit->palette().color(QPalette::Text);

    connect(_searchLineEdit, SIGNAL(returnPressed()), this, SLOT(searchNext()));
    connect(_searchLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(searchTextChanged()));

    hbox->addWidget(_searchLineEdit);
    _searchNextButton = new QPushButton(KIcon("go-down"), i18n("Next"), statusWidget);
    _searchNextButton->setToolTip(i18n("Jump to next match"));
    connect(_searchNextButton, SIGNAL(clicked()), this, SLOT(searchNext()));
    hbox->addWidget(_searchNextButton);
    _searchPrevButton = new QPushButton(KIcon("go-up"), i18n("Previous"), statusWidget);
    _searchPrevButton->setToolTip(i18n("Jump to previous match"));
    connect(_searchPrevButton, SIGNAL(clicked()), this, SLOT(searchPrev()));
    hbox->addWidget(_searchPrevButton);
    _searchOptions = new QPushButton(i18n("Options"), statusWidget);
    _searchOptions->setToolTip(i18n("Modify search behavior"));
    QMenu * menu = new QMenu();
    _fromCursorAction = menu->addAction(i18n("From cursor"));
    _fromCursorAction->setCheckable(true);
    _fromCursorAction->setChecked(true);
    _caseSensitiveAction = menu->addAction(i18n("Case sensitive"));
    _caseSensitiveAction->setCheckable(true);
    _matchWholeWordsOnlyAction = menu->addAction(i18n("Match whole words only"));
    _matchWholeWordsOnlyAction->setCheckable(true);
    _regExpAction = menu->addAction(i18n("RegExp"));
    _regExpAction->setCheckable(true);
    _hexAction = menu->addAction(i18n("Hexadecimal"));
    _hexAction->setCheckable(true);
    _searchOptions->setMenu(menu);

    hbox->addWidget(_searchOptions);

    QSpacerItem* cbSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hbox->addItem(cbSpacer);

    _statusLabel = new QLabel(statusWidget);
    hbox->addWidget(_statusLabel);

    grid->addWidget(statusWidget, 1, 0, 1, 2);
    setWidget(widget);

    connect(_scrollBar, SIGNAL(actionTriggered(int)), _textArea, SLOT(slotActionTriggered(int)));
    connect(&_updateTimer, SIGNAL(timeout()), this, SLOT(slotUpdate()));

    _updateTimer.setSingleShot(false);

    new ListerBrowserExtension(this);
    enableSearch(false);
}

Lister::~Lister()
{
    if (_cache != 0) {
        delete []_cache;
        _cache = 0;
    }
    if (_tempFile != 0) {
        delete _tempFile;
        _tempFile = 0;
    }
}

bool Lister::openUrl(const KUrl &listerUrl)
{
    _downloading = false;
    setUrl(listerUrl);

    if (_tempFile) {
        delete _tempFile;
        _tempFile = 0;
    }
    _fileSize = 0;

    if (listerUrl.isLocalFile()) {
        _filePath = listerUrl.path();
        if (!QFile::exists(_filePath))
            return false;
        _fileSize = getFileSize();
    } else {
        _tempFile = new KTemporaryFile();
        _tempFile->setSuffix(listerUrl.fileName());
        _tempFile->open();

        _filePath = _tempFile->fileName();

        KIO::Job * downloadJob = KIO::get(listerUrl, KIO::NoReload, KIO::HideProgressInfo);

        connect(downloadJob, SIGNAL(data(KIO::Job *, const QByteArray &)),
                this, SLOT(slotFileDataReceived(KIO::Job *, const QByteArray &)));
        connect(downloadJob, SIGNAL(result(KJob*)),
                this, SLOT(slotFileFinished(KJob *)));
        _downloading = false;
    }
    if (_cache) {
        delete []_cache;
        _cache = 0;
    }
    _textArea->reset();
    emit started(0);
    emit setWindowCaption(listerUrl.prettyUrl());
    emit completed();
    return true;
}

void Lister::slotFileDataReceived(KIO::Job *, const QByteArray &array)
{
    if (array.size() != 0)
        _tempFile->write(array);
}

void Lister::slotFileFinished(KJob *job)
{
    _tempFile->flush();
    if (job->error()) {   /* any error occurred? */
        KIO::TransferJob *kioJob = (KIO::TransferJob *)job;
        KMessageBox::error(_textArea, i18n("Error reading file %1!", kioJob->url().pathOrUrl()));
    }
    _downloading = false;
}


char * Lister::cacheRef(qint64 filePos, int &size)
{
    if (filePos >= _fileSize)
        return 0;
    if (_fileSize - filePos < size)
        size = _fileSize - filePos;
    if ((_cache != 0) && (filePos >= _cachePos) && (filePos + size <= _cachePos + _cacheSize))
        return _cache + (filePos - _cachePos);

    int cacheSize = CACHE_SIZE;
    int negativeOffset = CACHE_SIZE * 2 / 5;
    qint64 cachePos = filePos - negativeOffset;
    if (cachePos < 0)
        cachePos = 0;

    QFile myfile(_filePath);
    if (!myfile.open(QIODevice::ReadOnly)) {
        myfile.close();
        return 0;
    }

    if (!myfile.seek(cachePos)) {
        myfile.close();
        return 0;
    }

    char * newCache = new char [ cacheSize ];

    qint64 bytes = myfile.read(newCache, cacheSize);
    if (bytes == -1) {
        delete []newCache;
        myfile.close();
        return 0;
    }
    if (_cache)
        delete []_cache;

    _cache = newCache;
    _cacheSize = bytes;
    _cachePos = cachePos;
    int newSize = _cacheSize - (filePos - _cachePos);
    if (newSize < size)
        size = newSize;

    return _cache + (filePos - _cachePos);
}

qint64 Lister::getFileSize()
{
    return QFile(_filePath).size();
}

void Lister::guiActivateEvent(KParts::GUIActivateEvent * event)
{
    if (event->activated()) {
        _active = true;
        _updateTimer.setInterval(150);
        _updateTimer.start();
        slotUpdate();
    } else {
        enableSearch(false);
        _active = false;
        _updateTimer.stop();
    }
    KParts::ReadOnlyPart::guiActivateEvent(event);
}

void Lister::slotUpdate()
{
    qint64 oldSize = _fileSize;
    _fileSize = getFileSize();
    if (oldSize != _fileSize)
        _textArea->sizeChanged();

    int cursorX = 0, cursorY = 0;
    _textArea->getCursorPosition(cursorX, cursorY);
    bool isfirst = false;
    qint64 cursor = _textArea->getCursorPosition(isfirst);

    int percent = (_fileSize == 0) ? 0 : (int)((201 * cursor) / _fileSize / 2);

    QString status = i18n("Column: %1, Position: %2 (%3, %4%)")
                     .arg(cursorX, 3, 10, QChar(' '))
                     .arg(cursor).arg(_fileSize).arg(percent);
    _statusLabel->setText(status);

    if (_searchProgressCounter)
        _searchProgressCounter--;
}

void Lister::enableSearch(bool enable)
{
    if (enable) {
        _listerLabel->setText(i18n("Search:"));
        _searchLineEdit->show();
        _searchNextButton->show();
        _searchPrevButton->show();
        _searchOptions->show();
        _searchLineEdit->setFocus();
    } else {
        _listerLabel->setText(i18n("Lister:"));
        _searchLineEdit->hide();
        _searchNextButton->hide();
        _searchPrevButton->hide();
        _searchOptions->hide();
    }
}

void Lister::searchNext()
{
    search(true);
}

void Lister::searchPrev()
{
    search(false);
}

void Lister::search(bool forward, bool restart)
{
    _restartFromBeginning = restart;
    if (_searchInProgress || _searchLineEdit->text().isEmpty())
        return;
    if (_searchLineEdit->isHidden())
        enableSearch(true);

    _searchPosition = forward ? 0 : _fileSize;
    if (_fromCursorAction->isChecked()) {
        bool isfirst;
        qint64 cursor = _textArea->getCursorPosition(isfirst);
        if (cursor != 0 && !forward)
            cursor--;
        if (_searchLastFailedPosition == -1 || _searchLastFailedPosition != cursor)
            _searchPosition = cursor;
    }
    bool caseSensitive = _caseSensitiveAction->isChecked();
    bool matchWholeWord = _matchWholeWordsOnlyAction->isChecked();
    bool regExp = _regExpAction->isChecked();
    bool hex = _hexAction->isChecked();

    if (hex) {
        QString hexcontent = _searchLineEdit->text();
        hexcontent.remove(QLatin1String("0x"));
        hexcontent.remove(' ');
        hexcontent.remove('\t');
        hexcontent.remove('\n');
        hexcontent.remove('\r');

        _searchHexQuery = QByteArray();

        if (hexcontent.length() & 1) {
            setColor(false, false);
            return;
        }

        while (!hexcontent.isEmpty()) {
            QString hexData = hexcontent.left(2);
            hexcontent = hexcontent.mid(2);
            bool ok = true;
            int c = hexData.toUInt(&ok, 16);
            if (!ok) {
                setColor(false, false);
                return;
            }
            _searchHexQuery.push_back((char) c);
        }
    } else {
        _searchQuery.setContent(_searchLineEdit->text(), caseSensitive, matchWholeWord, false, _textArea->codec()->name(), regExp);
    }
    _searchIsForward = forward;
    _searchHexadecimal = hex;

    QTimer::singleShot(0, this, SLOT(slotSearchMore()));
    _searchInProgress = true;
    _searchProgressCounter = 3;

    enableActions(false);
}

void Lister::enableActions(bool state)
{
    _actionSearch->setEnabled(state);
    _actionSearchNext->setEnabled(state);
    _actionSearchPrev->setEnabled(state);
    _actionJumpToPosition->setEnabled(state);
}

void Lister::slotSearchMore()
{
    if (!_searchInProgress)
        return;

    updateProgressBar();
    if (!_searchIsForward)
        _searchPosition--;

    if (_searchPosition < 0 || _searchPosition >= _fileSize) {
        if (_restartFromBeginning)
            resetSearchPosition();
        else {
            searchFailed();
            return;
        }
    }

    int maxCacheSize = SEARCH_CACHE_CHARS;
    qint64 searchPos = _searchPosition;
    bool setPosition = true;
    if (!_searchIsForward) {
        qint64 origSearchPos = _searchPosition;
        searchPos -= maxCacheSize;
        if (searchPos <= 0) {
            searchPos = 0;
            _searchPosition = 0;
            setPosition = false;
        }
        qint64 diff = origSearchPos - searchPos;
        if (diff < maxCacheSize)
            maxCacheSize = diff;
    }

    char * cache = cacheRef(searchPos, maxCacheSize);
    if (cache == 0 || maxCacheSize == 0) {
        searchFailed();
        return;
    }

    qint64 foundAnchor = -1;
    qint64 foundCursor = -1;
    int cnt = 0;

    if (_searchHexadecimal) {
        QByteArray cacheItems(cache, maxCacheSize);
        int ndx = _searchIsForward ? cacheItems.indexOf(_searchHexQuery) : cacheItems.lastIndexOf(_searchHexQuery);
        if (maxCacheSize > _searchHexQuery.length()) {
            if (_searchIsForward) {
                _searchPosition = searchPos + maxCacheSize;
                if ((_searchPosition < _fileSize) && (maxCacheSize > _searchHexQuery.length()))
                    _searchPosition -= _searchHexQuery.length();
                cnt = _searchPosition - searchPos;
            } else {
                if (_searchPosition > 0)
                    _searchPosition += _searchHexQuery.length();
            }
        }
        if (ndx != -1) {
            foundAnchor = searchPos + ndx;
            foundCursor = foundAnchor + _searchHexQuery.length();
        }
    } else {
        QTextCodec * textCodec = _textArea->codec();
        QTextDecoder * decoder = textCodec->makeDecoder();

        int rowStart = 0;

        QString row = "";

        while (cnt < maxCacheSize) {
            QString chr = decoder->toUnicode(cache + (cnt++), 1);
            if (!chr.isEmpty() || cnt >= maxCacheSize) {
                if (chr != "\n")
                    row += chr;

                if (chr == "\n" || row.length() >= SEARCH_MAX_ROW_LEN || cnt >= maxCacheSize) {
                    if (setPosition) {
                        _searchPosition = searchPos + cnt;
                        if (!_searchIsForward) {
                            _searchPosition ++;
                            setPosition = false;
                        }
                    }

                    if (_searchQuery.checkLine(row, !_searchIsForward)) {
                        QByteArray cachedBuffer(cache + rowStart, maxCacheSize - rowStart);

                        QTextStream stream(&cachedBuffer);
                        stream.setCodec(textCodec);

                        stream.read(_searchQuery.matchIndex());
                        foundAnchor = searchPos + rowStart + stream.pos();

                        stream.read(_searchQuery.matchLength());
                        foundCursor = searchPos + rowStart + stream.pos();

                        if (_searchIsForward)
                            break;
                    }

                    row = "";
                    rowStart = cnt;
                }
            }
        }

        delete decoder;
    }

    if (foundAnchor != -1 && foundCursor != -1) {
        _textArea->setAnchorAndCursor(foundAnchor, foundCursor);
        searchSucceeded();
        return;
    }

    if (_searchIsForward && searchPos + cnt >= _fileSize) {
        if (_restartFromBeginning)
            resetSearchPosition();
        else {
            searchFailed();
            return;
        }
    } else if (_searchPosition <= 0 || _searchPosition >= _fileSize) {
        if (_restartFromBeginning)
            resetSearchPosition();
        else {
            searchFailed();
            return;
        }
    }

    QTimer::singleShot(0, this, SLOT(slotSearchMore()));
}

void Lister::resetSearchPosition()
{
    _restartFromBeginning = false;
    _searchPosition = _searchIsForward ? 0 : _fileSize - 1;
}

void Lister::searchSucceeded()
{
    _searchInProgress = false;
    setColor(true, false);
    hideProgressBar();
    _searchLastFailedPosition = -1;

    enableActions(true);
}

void Lister::searchFailed()
{
    _searchInProgress = false;
    setColor(false, false);
    hideProgressBar();
    bool isfirst;
    _searchLastFailedPosition = _textArea->getCursorPosition(isfirst);
    if (!_searchIsForward)
        _searchLastFailedPosition--;

    enableActions(true);
}

void Lister::searchDelete()
{
    _searchInProgress = false;
    setColor(false, true);
    hideProgressBar();
    _searchLastFailedPosition = -1;

    enableActions(true);
}

void Lister::searchTextChanged()
{
    searchDelete();
    if (_fileSize < 0x10000) { // autosearch files less than 64k
        if (!_searchLineEdit->text().isEmpty()) {
            bool isfirst;
            qint64 anchor = _textArea->getCursorAnchor();
            qint64 cursor = _textArea->getCursorPosition(isfirst);
            if (cursor > anchor && anchor != -1) {
                _textArea->setCursorPosition(anchor, true);
            }
            search(true, true);
        }
    }
}

void Lister::setColor(bool match, bool restore)
{
    QColor  fore, back;

    if (!restore) {
        KConfigGroup gc(krConfig, "Colors");

        QString foreground, background;

        if (match) {
            foreground = "Quicksearch Match Foreground";
            background = "Quicksearch Match Background";
            fore = Qt::black;
            back = QColor(192, 255, 192);
        } else {
            foreground = "Quicksearch Non-match Foreground";
            background = "Quicksearch Non-match Background";
            fore = Qt::black;
            back = QColor(255, 192, 192);
        }

        if (gc.readEntry(foreground, QString()) == "KDE default")
            fore = KColorScheme(QPalette::Active, KColorScheme::View).foreground().color();
        else if (!gc.readEntry(foreground, QString()).isEmpty())
            fore = gc.readEntry(foreground, fore);

        if (gc.readEntry(background, QString()) == "KDE default")
            back = KColorScheme(QPalette::Active, KColorScheme::View).background().color();
        else if (!gc.readEntry(background, QString()).isEmpty())
            back = gc.readEntry(background, back);
    } else {
        back = _originalBackground;
        fore = _originalForeground;
    }

    QPalette pal = _searchLineEdit->palette();
    pal.setColor(QPalette::Base, back);
    pal.setColor(QPalette::Text, fore);
    _searchLineEdit->setPalette(pal);
}

void Lister::hideProgressBar()
{
    if (!_searchProgressBar->isHidden()) {
        _searchProgressBar->hide();
        _searchStopButton->hide();
        _searchLineEdit->show();
        _searchNextButton->show();
        _searchPrevButton->show();
        _searchOptions->show();
        _listerLabel->setText(i18n("Search:"));
    }
}

void Lister::updateProgressBar()
{
    if (_searchProgressCounter)
        return;

    if (_searchProgressBar->isHidden()) {
        _searchProgressBar->show();
        _searchStopButton->show();
        _searchOptions->hide();
        _searchLineEdit->hide();
        _searchNextButton->hide();
        _searchPrevButton->hide();
        _listerLabel->setText(i18n("Search position:"));
    }

    qint64 pcnt = (_fileSize == 0) ? 1000 : (2001 * _searchPosition) / _fileSize / 2;
    int pctInt = (int)pcnt;
    if (_searchProgressBar->value() != pctInt)
        _searchProgressBar->setValue(pctInt);
}

void Lister::jumpToPosition()
{
    bool ok = true;
    QString res = KInputDialog::getText(i18n("Jump to position"), i18n("Text position:"), "0",
                                        &ok, _textArea);
    if (!ok)
        return;

    res = res.trimmed();
    qint64 pos = -1;
    if (res.startsWith(QLatin1String("0x"))) {
        res = res.mid(2);
        bool ok;
        qulonglong upos = res.toULongLong(&ok, 16);
        if (!ok) {
            KMessageBox::error(_textArea, i18n("Invalid number!"), i18n("Jump to position"));
            return;
        }
        pos = (qint64)upos;
    } else {
        bool ok;
        qulonglong upos = res.toULongLong(&ok);
        if (!ok) {
            KMessageBox::error(_textArea, i18n("Invalid number!"), i18n("Jump to position"));
            return;
        }
        pos = (qint64)upos;
    }

    if (pos < 0 || pos > _fileSize) {
        KMessageBox::error(_textArea, i18n("Number out of range!"), i18n("Jump to position"));
        return;
    }

    _textArea->deleteAnchor();
    _textArea->setCursorPosition(pos, true);
    _textArea->ensureVisibleCursor();
}

void Lister::saveAs()
{
    KUrl url = KFileDialog::getSaveUrl(KUrl(), QString(), _textArea, i18n("Lister"));
    if (url.isEmpty())
        return;
    KUrl sourceUrl;
    if (!_downloading)
        sourceUrl = KUrl(_filePath);
    else
        sourceUrl = this->url();

    KUrl::List urlList;
    urlList << sourceUrl;

    KIO::Job *job = KIO::copy(urlList, url);
    job->setUiDelegate(new KIO::JobUiDelegate());
    KIO::getJobTracker()->registerJob(job);
    job->ui()->setAutoErrorHandlingEnabled(true);
}

void Lister::saveSelected()
{
    bool isfirst;
    qint64 start = _textArea->getCursorAnchor();
    qint64 end = _textArea->getCursorPosition(isfirst);
    if (start == -1 || start == end) {
        KMessageBox::error(_textArea, i18n("Nothing is selected!"), i18n("Save selection..."));
        return;
    }
    if (start > end) {
        _savePosition = end;
        _saveEnd = start;
    } else {
        _savePosition = start;
        _saveEnd = end;
    }

    KUrl url = KFileDialog::getSaveUrl(KUrl(), QString(), _textArea, i18n("Lister"));
    if (url.isEmpty())
        return;

    KIO::Job *saveJob = KIO::put(url, -1, KIO::Overwrite);
    connect(saveJob, SIGNAL(dataReq(KIO::Job *, QByteArray &)),
            this, SLOT(slotDataSend(KIO::Job *, QByteArray &)));
    connect(saveJob, SIGNAL(result(KJob*)),
            this, SLOT(slotSendFinished(KJob *)));

    saveJob->setUiDelegate(new KIO::JobUiDelegate());
    KIO::getJobTracker()->registerJob(saveJob);
    saveJob->ui()->setAutoErrorHandlingEnabled(true);

    _actionSaveSelected->setEnabled(false);
}

void Lister::slotDataSend(KIO::Job *, QByteArray &array)
{
    if (_savePosition >= _saveEnd) {
        array = QByteArray();
        return;
    }
    qint64 max = _saveEnd - _savePosition;
    if (max > 1000)
        max = 1000;
    int maxBytes = (int)max;
    char * cache = cacheRef(_savePosition, maxBytes);
    _savePosition += maxBytes;

    array = QByteArray(cache, maxBytes);
}

void Lister::slotSendFinished(KJob *)
{
    _actionSaveSelected->setEnabled(true);
}

void Lister::setCharacterSet(QString set)
{
    _characterSet = set;
    _textArea->redrawTextArea(true);
}

void Lister::print()
{
    bool isfirst;
    qint64 anchor = _textArea->getCursorAnchor();
    qint64 cursor = _textArea->getCursorPosition(isfirst);
    bool hasSelection = (anchor != -1 && anchor != cursor);

    QString docName = url().fileName();
    QPrinter printer;
    printer.setDocName(docName);

    QPointer<QPrintDialog> printDialog = new QPrintDialog(&printer, _textArea);

    if (hasSelection)
        printDialog->addEnabledOption(QAbstractPrintDialog::PrintSelection);

    if (printDialog->exec()) {
        if (printer.pageOrder() == QPrinter::LastPageFirst) {
            switch (KMessageBox::warningContinueCancel(_textArea,
                    i18n("Reverse printing is not supported. Continue with normal printing?"))) {
            case KMessageBox::Continue :
                break;
            default:
                return;
            }
        }
        QPainter painter;
        painter.begin(&printer);

        QDate date = QDate::currentDate();
        QString dateString = date.toString(Qt::SystemLocaleShortDate);

        QRect pageRect = printer.pageRect();
        QRect drawingRect(0, 0, pageRect.width(), pageRect.height());

        QFont normalFont = KGlobalSettings::generalFont();
        QFont fixedFont  = KGlobalSettings::fixedFont();

        QFontMetrics fmNormal(normalFont);
        int normalFontHeight = fmNormal.height();

        QFontMetrics fmFixed(fixedFont);
        int fixedFontHeight = fmFixed.height();
        int fixedFontWidth = fmFixed.width("W");
        if (fixedFontHeight <= 0)
            fixedFontHeight = 1;
        if (fixedFontWidth <= 0)
            fixedFontWidth = 1;

        int effPageSize = drawingRect.height() - normalFontHeight - 1;
        int rowsPerPage = effPageSize / fixedFontHeight;
        if (rowsPerPage <= 0)
            rowsPerPage = 1;
        int columnsPerPage = drawingRect.width() / fixedFontWidth;
        if (columnsPerPage <= 0)
            columnsPerPage = 1;

        bool firstPage = true;

        qint64 startPos = 0;
        qint64 endPos = _fileSize;
        if (printer.printRange() == QPrinter::Selection) {
            if (anchor > cursor)
                startPos = cursor, endPos = anchor;
            else
                startPos = anchor, endPos = cursor;
        }

        for (int page = 1;; ++page) {
            QStringList rows = readLines(startPos, endPos, columnsPerPage, rowsPerPage);

            bool visible = true;
            if (printer.fromPage() && page < printer.fromPage())
                visible = false;
            if (printer.toPage() && printer.toPage() >= printer.fromPage() && page > printer.toPage())
                visible = false;

            if (visible) {
                if (!firstPage)
                    printer.newPage();
                firstPage = false;
                // Use the painter to draw on the page.
                painter.setFont(normalFont);

                painter.drawText(drawingRect, Qt::AlignLeft, dateString);
                painter.drawText(drawingRect, Qt::AlignHCenter, docName);
                painter.drawText(drawingRect, Qt::AlignRight, QString("%1").arg(page));

                painter.drawLine(0, normalFontHeight, drawingRect.width(), normalFontHeight);

                painter.setFont(fixedFont);
                int yoffset = normalFontHeight + 1;
                foreach(const QString &row, rows) {
                    painter.drawText(0, yoffset + fixedFontHeight, row);
                    yoffset += fixedFontHeight;
                }
            }
            if (startPos >= endPos)
                break;
        }
    }

    delete printDialog;
}

QStringList Lister::readLines(qint64 &filePos, qint64 endPos, int columns, int lines)
{
    if (_textArea->hexMode())
        return readHexLines(filePos, endPos, columns, lines);
    QStringList list;
    int maxBytes = columns * lines * MAX_CHAR_LENGTH;
    if (maxBytes > (endPos - filePos))
        maxBytes = (int)(endPos - filePos);
    if (maxBytes <= 0)
        return list;
    char * cache = cacheRef(filePos, maxBytes);
    if (cache == 0 || maxBytes == 0)
        return list;

    QTextCodec * textCodec = _textArea->codec();
    QTextDecoder * decoder = textCodec->makeDecoder();

    int cnt = 0;
    int y = 0;
    QString row = "";
    bool isLastLongLine = false;
    while (cnt < maxBytes && y < lines) {
        QString chr = decoder->toUnicode(cache + (cnt++), 1);
        if (!chr.isEmpty()) {
            if ((chr[ 0 ] < 32) && (chr[ 0 ] != '\n') && (chr[ 0 ] != '\t'))
                chr = QChar(' ');
            if (chr == "\n") {
                if (!isLastLongLine) {
                    list << row;
                    row = "";
                    y++;
                }
                isLastLongLine = false;
            } else {
                isLastLongLine = false;
                if (chr == "\t") {
                    int tabLength = _textArea->tabWidth() - (row.length() % _textArea->tabWidth());
                    if (row.length() + tabLength > columns) {
                        list << row;
                        row = "";
                        y++;
                    }
                    row += QString(tabLength, QChar(' '));
                } else
                    row += chr;

                if (row.length() >= columns) {
                    list << row;
                    row = "";
                    y++;
                    isLastLongLine = true;
                }
            }
        }
    }

    if (y < lines)
        list << row;

    filePos += cnt;

    delete decoder;
    return list;
}

int Lister::hexPositionDigits()
{
    int positionDigits = 0;
    qint64 checker = _fileSize;
    while (checker) {
        positionDigits++;
        checker /= 16;
    }
    if (positionDigits < 8)
        positionDigits = 8;
    return positionDigits;
}

int Lister::hexBytesPerLine(int columns)
{
    int positionDigits = hexPositionDigits();
    int bytesPerRow = 8;
    if (columns >= positionDigits + 5 + 64)
        bytesPerRow = 16;
    if (columns >= positionDigits + 5 + 128)
        bytesPerRow = 32;

    return bytesPerRow;
}

QStringList Lister::readHexLines(qint64 &filePos, qint64 endPos, int columns, int lines)
{
    int positionDigits = hexPositionDigits();
    int bytesPerRow = hexBytesPerLine(columns);

    QStringList list;

    qint64 choppedPos = (filePos / bytesPerRow) * bytesPerRow;
    int maxBytes = bytesPerRow * lines;
    if (maxBytes > (endPos - choppedPos))
        maxBytes = (int)(endPos - choppedPos);
    if (maxBytes <= 0)
        return list;

    char * cache = cacheRef(choppedPos, maxBytes);

    if (cache == 0 || maxBytes == 0)
        return list;

    int cnt = 0;
    for (int l = 0; l < lines; l++) {
        if (filePos >= endPos)
            break;
        qint64 printPos = (filePos / bytesPerRow) * bytesPerRow;
        QString pos;
        pos.setNum(printPos, 16);
        while (pos.length() < positionDigits)
            pos = QString("0") + pos;
        pos = QString("0x") + pos;
        pos += QString(": ");

        QString charData;

        for (int i = 0; i != bytesPerRow; i++, cnt++) {
            qint64 currentPos = printPos + i;
            if (currentPos < filePos || currentPos >= endPos) {
                pos += QString("   ");
                charData += QString(" ");
            } else {
                char c = cache[ cnt ];
                int charCode = (int)c;
                if (charCode < 0)
                    charCode += 256;
                QString hex;
                hex.setNum(charCode, 16);
                if (hex.length() < 2)
                    hex = QString("0") + hex;
                pos += hex + QString(" ");
                if (c < 32 || c >= 128)
                    c = '.';
                charData += QChar(c);
            }
        }

        pos += QString(" ") + charData;
        list << pos;
        filePos = printPos + bytesPerRow;
    }

    if (filePos > endPos)
        filePos = endPos;

    return list;
}

int Lister::hexIndexToPosition(int columns, int index)
{
    int positionDigits = hexPositionDigits();
    int bytesPerRow = hexBytesPerLine(columns);

    if (index >= bytesPerRow)
        index = bytesPerRow;

    return positionDigits + 4 + (3*index);
}

int Lister::hexPositionToIndex(int columns, int position)
{
    int positionDigits = hexPositionDigits();
    int bytesPerRow = hexBytesPerLine(columns);

    position -= 4 + positionDigits;
    if (position <= 0)
        return 0;

    position /= 3;
    if (position >= bytesPerRow)
        return bytesPerRow;
    return position;
}

void Lister::toggleHexMode()
{
    setHexMode(!_textArea->hexMode());
}

void Lister::setHexMode(bool mode)
{
    if (mode) {
        _textArea->setHexMode(true);
        _actionHexMode->setText(i18n("Text mode"));
    } else {
        _textArea->setHexMode(false);
        _actionHexMode->setText(i18n("Hex mode"));
    }
}
