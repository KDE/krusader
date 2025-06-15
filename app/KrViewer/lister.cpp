/*
    SPDX-FileCopyrightText: 2009 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "lister.h"

// QtCore
#include <QDate>
#include <QFile>
#include <QRect>
#include <QTemporaryFile>
#include <QTextStream>
#include <QStringConverter>
// QtGui
#include <QClipboard>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QPainter>
// QtWidgets
#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QSpacerItem>
#include <QToolButton>
// QtPrintSupport
#include <QPrintDialog>
#include <QPrinter>

#include <KActionCollection>
#include <KCharsets>
#include <KIO/CopyJob>
#include <KIO/JobUiDelegate>
#include <KIO/JobUiDelegateFactory>
#include <KIO/TransferJob>
#include <KIO/JobTracker>
#include <KJobTrackerInterface>
#include <KLocalizedString>
#include <KMessageBox>
#include <KParts/GUIActivateEvent>
#include <KSharedConfig>

#include "../GUI/krremoteencodingmenu.h"
#include "../compat.h"
#include "../icon.h"
#include "../kractions.h"
#include "../krglobal.h"

#define SEARCH_CACHE_CHARS 100000
#define SEARCH_MAX_ROW_LEN 4000
#define CONTROL_CHAR 752
#define CACHE_SIZE 1048576 // cache size set to 1MiB

ListerTextArea::ListerTextArea(Lister *lister, QWidget *parent)
    : KTextEdit(parent)
    , _lister(lister)
{
    connect(this, &QTextEdit::cursorPositionChanged, this, &ListerTextArea::slotCursorPositionChanged);
    _tabWidth = 4;
    setWordWrapMode(QTextOption::NoWrap);
    setLineWrapMode(QTextEdit::NoWrap);

    // zoom shortcuts
    connect(new QShortcut(QKeySequence("Ctrl++"), this), &QShortcut::activated, this, [=]() {
        zoomIn();
    });
    connect(new QShortcut(QKeySequence("Ctrl+-"), this), &QShortcut::activated, this, [=]() {
        zoomOut();
    });

    // start cursor blinking
    connect(&_blinkTimer, &QTimer::timeout, this, [=] {
        if (!_cursorBlinkMutex.tryLock()) {
            return;
        }
        setCursorWidth(cursorWidth() == 0 ? 2 : 0);
        _cursorBlinkMutex.unlock();
    });
    _blinkTimer.start(500);
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

void ListerTextArea::resizeEvent(QResizeEvent *event)
{
    KTextEdit::resizeEvent(event);
    redrawTextArea();
}

void ListerTextArea::calculateText(const bool forcedUpdate)
{
    const QRect contentRect = viewport()->contentsRect();
    const QFontMetrics fm(font());

    const int fontHeight = std::max(fm.height(), 1);

    // This is quite accurate (although not perfect) way of getting
    // a single character width along with its surrounding space.
    const double fontWidth = (fm.horizontalAdvance("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW")
                              - fm.horizontalAdvance("W"))
        / 99.0;

    const int sizeY = contentRect.height() / fontHeight;
    _pageSize = sizeY;

    const int textViewportWidth = std::max(contentRect.width() - (int)fontWidth, 0);

    setTabStopDistance(fontWidth * _tabWidth);

    const int sizeX = static_cast<int>(textViewportWidth / fontWidth);

    _sizeChanged = (_sizeY != sizeY) || (_sizeX != sizeX) || forcedUpdate;
    _sizeY = sizeY;
    _sizeX = sizeX;

    QList<qint64> rowStarts;

    QStringList list = readLines(_screenStartPos, _screenEndPos, _sizeY, &rowStarts);

    if (_sizeChanged) {
        _averagePageSize = _screenEndPos - _screenStartPos;
        setUpScrollBar();
    }

    const QStringList listRemn = readLines(_screenEndPos, _screenEndPos, 1);
    list << listRemn;

    if (list != _rowContent) {
        _cursorBlinkMutex.lock();
        _blinkTimer.stop();
        setCursorWidth(0);

        setPlainText(list.join("\n"));

        if (_cursorAnchorPos == -1 || _cursorAnchorPos == _cursorPos) {
            clearSelection();
            _blinkTimer.start(500);
        }

        _cursorBlinkMutex.unlock();

        _rowContent = list;
        _rowStarts = rowStarts;
        if (_rowStarts.size() < _sizeY) {
            _rowStarts << _screenEndPos;
        }
    }
}

qint64 ListerTextArea::textToFilePositionOnScreen(const int x, const int y, bool &isfirst)
{
    isfirst = (x == 0);
    if (y >= _rowStarts.count()) {
        return 0;
    }
    const qint64 rowStart = _rowStarts[y];
    if (x == 0) {
        return rowStart;
    }

    if (_hexMode) {
        const qint64 pos = rowStart + _lister->hexPositionToIndex(_sizeX, x);
        if (pos > _lister->fileSize()) {
            return _lister->fileSize();
        }
        return pos;
    }

    // we can't use fromUnicode because of the invalid encoded chars
    const qsizetype maxBytes = 2 * _sizeX * MAX_CHAR_LENGTH;
    QByteArray chunk = _lister->cacheChunk(rowStart, maxBytes);

    QString s = _lister->codec()->toUnicode(chunk);
    QTextStream stream(&s);

    stream.read(x);
    return rowStart + stream.pos();
}

void ListerTextArea::fileToTextPositionOnScreen(const qint64 p, const bool isfirst, int &x, int &y)
{
    // check if cursor is outside of visible area
    if (p < _screenStartPos || p > _screenEndPos || _rowStarts.count() < 1) {
        x = -1;
        y = (p > _screenEndPos) ? -2 : -1;
        return;
    }

    // find row
    y = 0;
    while (y < _rowStarts.count() && _rowStarts[y] <= p) {
        y++;
    }
    y--;
    if (y < 0) {
        x = y = -1;
        return;
    }

    const qint64 rowStart = _rowStarts[y];
    if (_hexMode) {
        x = _lister->hexIndexToPosition(_sizeX, (int)(p - rowStart));
        return;
    }

    // find column
    const qsizetype maxBytes = 2 * _sizeX * MAX_CHAR_LENGTH;
    x = 0;
    if (rowStart >= p) {
        if ((rowStart == p) && !isfirst && y > 0) {
            const qint64 previousRow = _rowStarts[y - 1];
            const QByteArray chunk = _lister->cacheChunk(previousRow, maxBytes);
            QByteArray cachedBuffer = chunk.left(static_cast<int>(p - previousRow));
            QTextCodec *codec = _lister->codec();
            QString decoded = codec->toUnicode(cachedBuffer);
            QTextStream stream(&decoded);

            stream.read(_rowContent[y - 1].length());
            if (previousRow + stream.pos() == p) {
                y--;
                x = static_cast<int>(_rowContent[y].length());
            }
        }
        return;
    }

    const QByteArray chunk = _lister->cacheChunk(rowStart, maxBytes);
    const QByteArray cachedBuffer = chunk.left(static_cast<int>(p - rowStart));

    x = static_cast<int>(_lister->codec()->toUnicode(cachedBuffer).length());
}

void ListerTextArea::getCursorPosition(int &x, int &y)
{
    getScreenPosition(textCursor().position(), x, y);
}

void ListerTextArea::getScreenPosition(const int position, int &x, int &y)
{
    x = position;
    y = 0;
    for (const QString &row : std::as_const(_rowContent)) {
        const int rowLen = static_cast<int>(row.length()) + 1;
        if (x < rowLen) {
            return;
        }
        x -= rowLen;
        y++;
    }
}

void ListerTextArea::setCursorPositionOnScreen(const int x, const int y, const int anchorX, const int anchorY)
{
    setCursorWidth(0);

    int finalX = x;
    int finalY = y;

    if (finalX == -1 || finalY < 0) {
        if (anchorY == -1) {
            return;
        }

        if (finalY == -2) {
            finalY = _sizeY;
            finalX = (_rowContent.count() > _sizeY) ? static_cast<int>(_rowContent[_sizeY].length()) : 0;
        } else
            finalX = finalY = 0;
    }

    const int realSizeY = std::min(_sizeY + 1, static_cast<int>(_rowContent.count()));

    const auto setUpCursor = [&](const int cursorX, const int cursorY, const QTextCursor::MoveMode mode) -> bool {
        if (cursorY > realSizeY) {
            return false;
        }

        _skipCursorChangedListener = true;

        moveCursor(QTextCursor::Start, mode);
        for (int i = 0; i < cursorY; i++) {
            moveCursor(QTextCursor::Down, mode);
        }

        qsizetype finalCursorX = cursorX;
        if (_rowContent.count() > cursorY && finalCursorX > _rowContent[cursorY].length()) {
            finalCursorX = _rowContent[cursorY].length();
        }

        for (int i = 0; i < finalCursorX; i++) {
            moveCursor(QTextCursor::Right, mode);
        }

        _skipCursorChangedListener = false;

        return true;
    };

    QTextCursor::MoveMode mode = QTextCursor::MoveAnchor;

    // set cursor anchor
    if (anchorX != -1 && anchorY != -1) {
        const bool canContinue = setUpCursor(anchorX, anchorY, mode);
        if (!canContinue) {
            return;
        }

        mode = QTextCursor::KeepAnchor;
    }

    // set cursor position
    setUpCursor(finalX, finalY, mode);
}

qint64 ListerTextArea::getCursorPosition(bool &isfirst)
{
    if (cursorWidth() == 0) {
        isfirst = _cursorAtFirstColumn;
        return _cursorPos;
    }

    int x, y;
    getCursorPosition(x, y);
    return textToFilePositionOnScreen(x, y, isfirst);
}

void ListerTextArea::setCursorPositionInDocument(const qint64 p, const bool isfirst)
{
    _cursorPos = p;
    int x, y;
    fileToTextPositionOnScreen(p, isfirst, x, y);

    bool startBlinkTimer = _screenStartPos <= _cursorPos && _cursorPos <= _screenEndPos;
    int anchorX = -1, anchorY = -1;
    if (_cursorAnchorPos != -1 && _cursorAnchorPos != p) {
        qint64 anchPos = _cursorAnchorPos;
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

        fileToTextPositionOnScreen(anchPos, isfirst, anchorX, anchorY);

        if (_hexMode) {
            if (anchorAbove) {
                anchorX = 0;
            }
            if (anchorBelow && _rowContent.count() > 0) {
                anchorX = static_cast<int>(_rowContent[0].length());
            }
        }

        startBlinkTimer = startBlinkTimer && !anchorAbove && !anchorBelow;
    }
    if (startBlinkTimer) {
        _blinkTimer.start(500);
    }
    setCursorPositionOnScreen(x, y, anchorX, anchorY);
    _lister->slotUpdate();
}

void ListerTextArea::slotCursorPositionChanged()
{
    if (_skipCursorChangedListener) {
        return;
    }
    int cursorX, cursorY;
    getCursorPosition(cursorX, cursorY);
    _cursorAtFirstColumn = (cursorX == 0);
    _cursorPos = textToFilePositionOnScreen(cursorX, cursorY, _cursorAtFirstColumn);
    _lister->slotUpdate();
}

QString ListerTextArea::readSection(const qint64 p1, const qint64 p2)
{
    if (p1 == p2)
        return QString();

    qint64 sel1 = p1;
    qint64 sel2 = p2;
    if (sel1 > sel2) {
        std::swap(sel1, sel2);
    }

    QString section;

    if (_hexMode) {
        while (sel1 != sel2) {
            const QStringList list = _lister->readHexLines(sel1, sel2, _sizeX, 1);
            if (list.isEmpty()) {
                break;
            }
            if (!section.isEmpty()) {
                section += QChar('\n');
            }
            section += list.at(0);
        }
        return section;
    }

    qint64 pos = sel1;

    QScopedPointer<QTextDecoder> decoder(_lister->codec()->makeDecoder());

    do {
        const qint64 maxBytes = std::min(qint64(_sizeX) * _sizeY * MAX_CHAR_LENGTH, sel2 - pos);
        const QByteArray chunk = _lister->cacheChunk(pos, maxBytes);
        if (chunk.isEmpty())
            break;
        section += decoder->toUnicode(chunk);
        pos += chunk.size();
    } while (pos < sel2);

    return section;
}

QStringList ListerTextArea::readLines(qint64 filePos, qint64 &endPos, const int lines, QList<qint64> *locs)
{
    QStringList list;

    if (_hexMode) {
        endPos = _lister->fileSize();
        if (filePos >= endPos) {
            return list;
        }
        const int bytes = _lister->hexBytesPerLine(_sizeX);
        qint64 startPos = (filePos / bytes) * bytes;
        qint64 shiftPos = startPos;
        list = _lister->readHexLines(shiftPos, endPos, _sizeX, lines);
        endPos = shiftPos;
        if (locs) {
            for (int i = 0; i < list.count(); i++) {
                (*locs) << startPos;
                startPos += bytes;
            }
        }
        return list;
    }

    endPos = filePos;
    const qsizetype maxBytes = _sizeX * _sizeY * MAX_CHAR_LENGTH;
    const QByteArray chunk = _lister->cacheChunk(filePos, maxBytes);
    if (chunk.isEmpty())
        return list;

    int byteCounter = 0;
    QString row = "";
    int effLength = 0;
    if (locs)
        (*locs) << filePos;
    bool skipImmediateNewline = false;

    const auto performNewline = [&](qint64 nextRowStartOffset) {
        list << row;
        effLength = 0;
        row = "";
        if (locs) {
            (*locs) << (filePos + nextRowStartOffset);
        }
    };

    QScopedPointer<QTextDecoder> decoder(_lister->codec()->makeDecoder());

    while (byteCounter < chunk.size() && list.size() < lines) {
        const int lastCnt = byteCounter;
        QString chr = decoder->toUnicode(chunk.mid(byteCounter++, 1));
        if (chr.isEmpty()) {
            continue;
        }

        if ((chr[0] < QChar(32)) && (chr[0] != '\n') && (chr[0] != '\t')) {
            chr = QChar(CONTROL_CHAR);
        }

        if (chr == "\n") {
            if (!skipImmediateNewline) {
                performNewline(byteCounter);
            }
            skipImmediateNewline = false;
            continue;
        }

        skipImmediateNewline = false;

        if (chr == "\t") {
            effLength += _tabWidth - (effLength % _tabWidth) - 1;
            if (effLength > _sizeX) {
                performNewline(lastCnt);
            }
        }
        row += chr;
        effLength++;
        if (effLength >= _sizeX) {
            performNewline(byteCounter);
            skipImmediateNewline = true;
        }
    }

    if (list.size() < lines)
        list << row;

    endPos = filePos + byteCounter;

    return list;
}

void ListerTextArea::setUpScrollBar()
{
    if (_averagePageSize == _lister->fileSize()) {
        _lister->scrollBar()->setPageStep(0);
        _lister->scrollBar()->setMaximum(0);
        _lister->scrollBar()->hide();
        _lastPageStartPos = 0;
    } else {
        const int maxPage = MAX_CHAR_LENGTH * _sizeX * _sizeY;
        qint64 pageStartPos = _lister->fileSize() - maxPage;
        qint64 endPos;
        if (pageStartPos < 0)
            pageStartPos = 0;
        QStringList list = readLines(pageStartPos, endPos, maxPage);
        if (list.count() <= _sizeY) {
            _lastPageStartPos = 0;
        } else {
            readLines(pageStartPos, _lastPageStartPos, static_cast<int>(list.count()) - _sizeY);
        }

        const qint64 maximum = (_lastPageStartPos > SLIDER_MAX) ? SLIDER_MAX : _lastPageStartPos;
        qint64 pageSize = (_lastPageStartPos > SLIDER_MAX) ? SLIDER_MAX * _averagePageSize / _lastPageStartPos : _averagePageSize;
        if (pageSize == 0)
            pageSize++;

        _lister->scrollBar()->setPageStep(static_cast<int>(pageSize));
        _lister->scrollBar()->setMaximum(static_cast<int>(maximum));
        _lister->scrollBar()->show();
    }
}

void ListerTextArea::keyPressEvent(QKeyEvent *ke)
{
    if (KrGlobal::copyShortcut == QKeySequence(ke->key() | ke->modifiers())) {
        copySelectedToClipboard();
        ke->accept();
        return;
    }

    if (ke->modifiers() == Qt::NoModifier || ke->modifiers() & Qt::ShiftModifier) {
        qint64 newAnchor = -1;
        if (ke->modifiers() & Qt::ShiftModifier) {
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
        } break;
        case Qt::Key_Right: {
            _cursorAnchorPos = newAnchor;
            ensureVisibleCursor();
            if (textCursor().position() == toPlainText().length())
                slotActionTriggered(QAbstractSlider::SliderSingleStepAdd);
        } break;
        case Qt::Key_Up: {
            _cursorAnchorPos = newAnchor;
            ensureVisibleCursor();
            int x, y;
            getCursorPosition(x, y);
            if (y == 0)
                slotActionTriggered(QAbstractSlider::SliderSingleStepSub);
        } break;
        case Qt::Key_Down: {
            _cursorAnchorPos = newAnchor;
            ensureVisibleCursor();
            int x, y;
            getCursorPosition(x, y);
            if (y >= _sizeY - 1)
                slotActionTriggered(QAbstractSlider::SliderSingleStepAdd);
        } break;
        case Qt::Key_PageDown: {
            _cursorAnchorPos = newAnchor;
            ensureVisibleCursor();
            ke->accept();
            int x, y;
            getCursorPosition(x, y);
            slotActionTriggered(QAbstractSlider::SliderPageStepAdd);
            y += _sizeY - _skippedLines;
            if (y > _rowContent.count()) {
                y = static_cast<int>(_rowContent.count()) - 1;
                if (y > 0)
                    x = static_cast<int>(_rowContent[y - 1].length());
                else
                    x = 0;
            }
            _cursorPos = textToFilePositionOnScreen(x, y, _cursorAtFirstColumn);
            setCursorPositionInDocument(_cursorPos, _cursorAtFirstColumn);
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
            _cursorPos = textToFilePositionOnScreen(x, y, _cursorAtFirstColumn);
            setCursorPositionInDocument(_cursorPos, _cursorAtFirstColumn);
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
            setCursorPositionInDocument((qint64)0, true);
            return;
        case Qt::Key_A:
        case Qt::Key_End: {
            _cursorAnchorPos = (ke->key() == Qt::Key_A) ? 0 : -1;
            ke->accept();
            slotActionTriggered(QAbstractSlider::SliderToMaximum);
            const qint64 endPos = _lister->fileSize();
            setCursorPositionInDocument(endPos, false);
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
    const int oldAnchor = textCursor().anchor();
    KTextEdit::keyPressEvent(ke);
    handleAnchorChange(oldAnchor);
}

void ListerTextArea::mousePressEvent(QMouseEvent *e)
{
    KTextEdit::mousePressEvent(e);
    // do change anchor only when shift is not pressed
    if (!(QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)) {
        performAnchorChange(textCursor().anchor());
    }
}

void ListerTextArea::mouseDoubleClickEvent(QMouseEvent *e)
{
    _cursorAnchorPos = -1;
    const int oldAnchor = textCursor().anchor();
    KTextEdit::mouseDoubleClickEvent(e);
    handleAnchorChange(oldAnchor);
}

void ListerTextArea::mouseMoveEvent(QMouseEvent *e)
{
    if (e->pos().y() < 0) {
        slotActionTriggered(QAbstractSlider::SliderSingleStepSub);
    } else if (e->pos().y() > height()) {
        slotActionTriggered(QAbstractSlider::SliderSingleStepAdd);
    }
    KTextEdit::mouseMoveEvent(e);
}

void ListerTextArea::wheelEvent(QWheelEvent *e)
{
    int delta = e->angleDelta().y();
    if (delta) {
        // zooming
        if (e->modifiers() & Qt::ControlModifier) {
            e->accept();
            if (delta > 0) {
                zoomIn();
            } else {
                zoomOut();
            }
            return;
        }

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

        setCursorPositionInDocument(_cursorPos, false);
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
    } break;
    case QAbstractSlider::SliderSingleStepSub: {
        if (_screenStartPos == 0) {
            break;
        }

        if (_hexMode) {
            int bytesPerRow = _lister->hexBytesPerLine(_sizeX);
            _screenStartPos = (_screenStartPos / bytesPerRow) * bytesPerRow;
            _screenStartPos -= bytesPerRow;
            if (_screenStartPos < 0) {
                _screenStartPos = 0;
            }
            break;
        }

        qint64 maxSize = _sizeX * _sizeY * MAX_CHAR_LENGTH;
        const QByteArray encodedEnter = _lister->codec()->fromUnicode(QString("\n"));

        qint64 readPos = _screenStartPos - maxSize;
        if (readPos < 0) {
            readPos = 0;
        }
        maxSize = _screenStartPos - readPos;

        const QByteArray chunk = _lister->cacheChunk(readPos, maxSize);

        qsizetype from = chunk.size();
        while (from > 0) {
            from--;
            from = chunk.lastIndexOf(encodedEnter, from);
            if (from == -1) {
                from = 0;
                break;
            }
            const qsizetype backRef = std::max(from - 20, qsizetype(0));
            const qsizetype size = from - backRef + encodedEnter.size();
            const QString decoded = _lister->codec()->toUnicode(chunk.mid(backRef, size));
            if (decoded.endsWith(QLatin1String("\n"))) {
                if (from < (chunk.size() - encodedEnter.size())) {
                    from += encodedEnter.size();
                    break;
                }
            }
        }

        readPos += from;
        qint64 previousPos = readPos;
        while (readPos < _screenStartPos) {
            previousPos = readPos;
            readLines(readPos, readPos, 1);
        }
        _screenStartPos = previousPos;
    } break;
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
    } break;
    case QAbstractSlider::SliderPageStepSub: {
        _skippedLines = 0;

        if (_screenStartPos == 0) {
            break;
        }

        if (_hexMode) {
            const qsizetype bytesPerRow = _lister->hexBytesPerLine(_sizeX);
            _screenStartPos = (_screenStartPos / bytesPerRow) * bytesPerRow;
            _screenStartPos -= _sizeY * bytesPerRow;
            if (_screenStartPos < 0) {
                _screenStartPos = 0;
            }
            break;
        }

        // text lister mode
        qint64 maxSize = 2 * _sizeX * _sizeY * MAX_CHAR_LENGTH;
        const QByteArray encodedEnter = _lister->codec()->fromUnicode(QString("\n"));

        qint64 readPos = _screenStartPos - maxSize;
        if (readPos < 0)
            readPos = 0;
        maxSize = _screenStartPos - readPos;

        const QByteArray chunk = _lister->cacheChunk(readPos, maxSize);
        maxSize = chunk.size();

        qsizetype sizeY = _sizeY + 1;
        qsizetype origSizeY = sizeY;
        qint64 from = maxSize;
        qint64 lastEnter = maxSize;

        bool readNext = true;
        while (readNext) {
            readNext = false;
            while (from > 0) {
                from--;
                from = chunk.lastIndexOf(encodedEnter, static_cast<int>(from));
                if (from == -1) {
                    from = 0;
                    break;
                }
                const qint64 backRef = std::max(from - 20, 0LL);
                const qint64 size = from - backRef + static_cast<qint64>(encodedEnter.size());
                QString decoded = _lister->codec()->toUnicode(chunk.mid(static_cast<int>(backRef), static_cast<int>(size)));
                if (decoded.endsWith(QLatin1String("\n"))) {
                    if (from < (maxSize - encodedEnter.size())) {
                        qint64 arrayStart = from + encodedEnter.size();
                        decoded = _lister->codec()->toUnicode(chunk.mid(static_cast<int>(arrayStart), static_cast<int>(lastEnter - arrayStart)));
                        sizeY -= ((decoded.length() / (_sizeX + 1)) + 1);
                        if (sizeY < 0) {
                            from = arrayStart;
                            break;
                        }
                    }
                    lastEnter = from;
                }
            }

            qint64 searchPos = readPos + from;
            QList<qint64> locs;
            while (searchPos < _screenStartPos) {
                locs << searchPos;
                readLines(searchPos, searchPos, 1);
            }

            if (locs.count() >= _sizeY) {
                _screenStartPos = locs[locs.count() - _sizeY];
            } else if (from != 0) {
                origSizeY += locs.count() + 1;
                sizeY = origSizeY;
                readNext = true;
            } else if (readPos == 0) {
                _screenStartPos = 0;
            }
        }

    } break;
    case QAbstractSlider::SliderToMinimum:
        _screenStartPos = 0;
        break;
    case QAbstractSlider::SliderToMaximum:
        _screenStartPos = _lastPageStartPos;
        break;
    case QAbstractSlider::SliderMove: {
        if (_inSliderOp) // self created call?
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
                const int bytesPerRow = _lister->hexBytesPerLine(_sizeX);
                pos = (pos / bytesPerRow) * bytesPerRow;
            } else {
                const qsizetype maxSize = _sizeX * _sizeY * MAX_CHAR_LENGTH;
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
    } break;
    case QAbstractSlider::SliderNoAction:
        break;
    };

    _inSliderOp = true;
    const int value = static_cast<int>((_lastPageStartPos > SLIDER_MAX) ? SLIDER_MAX * _screenStartPos / _lastPageStartPos : _screenStartPos);
    _lister->scrollBar()->setSliderPosition(value);
    _inSliderOp = false;

    redrawTextArea();
}

void ListerTextArea::redrawTextArea(bool forcedUpdate)
{
    if (_redrawing) {
        return;
    }
    _redrawing = true;
    bool isfirst;
    const qint64 pos = getCursorPosition(isfirst);
    calculateText(forcedUpdate);
    setCursorPositionInDocument(pos, isfirst);
    _redrawing = false;
}

void ListerTextArea::ensureVisibleCursor()
{
    if (_screenStartPos <= _cursorPos && _cursorPos <= _screenEndPos) {
        return;
    }

    qsizetype delta = _sizeY / 2;
    if (delta == 0)
        delta++;

    qint64 newScreenStart = _cursorPos;
    while (delta) {
        const qsizetype maxSize = _sizeX * MAX_CHAR_LENGTH;
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
    if (newScreenStart > _lastPageStartPos) {
        newScreenStart = _lastPageStartPos;
    }

    _screenStartPos = newScreenStart;
    slotActionTriggered(QAbstractSlider::SliderNoAction);
}

void ListerTextArea::setAnchorAndCursor(qint64 anchor, qint64 cursor)
{
    _cursorPos = cursor;
    _cursorAnchorPos = anchor;
    ensureVisibleCursor();
    setCursorPositionInDocument(cursor, false);
}

QString ListerTextArea::getSelectedText()
{
    if (_cursorAnchorPos != -1 && _cursorAnchorPos != _cursorPos) {
        return readSection(_cursorAnchorPos, _cursorPos);
    }
    return QString();
}

void ListerTextArea::copySelectedToClipboard()
{
    const QString selection = getSelectedText();
    if (!selection.isEmpty()) {
        QApplication::clipboard()->setText(selection);
    }
}

void ListerTextArea::clearSelection()
{
    QTextCursor cursor = textCursor();
    cursor.clearSelection();
    setTextCursor(cursor);
    _cursorAnchorPos = -1;
}

void ListerTextArea::performAnchorChange(int anchor)
{
    int x, y;
    bool isfirst;
    getScreenPosition(anchor, x, y);
    _cursorAnchorPos = textToFilePositionOnScreen(x, y, isfirst);
}

void ListerTextArea::handleAnchorChange(int oldAnchor)
{
    const int anchor = textCursor().anchor();

    if (oldAnchor != anchor) {
        performAnchorChange(anchor);
    }
}

void ListerTextArea::setHexMode(bool hexMode)
{
    bool isfirst;
    const qint64 pos = getCursorPosition(isfirst);
    _hexMode = hexMode;
    _screenStartPos = 0;
    calculateText(true);
    setCursorPositionInDocument(pos, isfirst);
    ensureVisibleCursor();
}

void ListerTextArea::zoomIn(int range)
{
    KTextEdit::zoomIn(range);
    redrawTextArea();
}

void ListerTextArea::zoomOut(int range)
{
    KTextEdit::zoomOut(range);
    redrawTextArea();
}

ListerPane::ListerPane(Lister *lister, QWidget *parent)
    : QWidget(parent)
    , _lister(lister)
{
}

bool ListerPane::event(QEvent *e)
{
    const bool handled = ListerPane::handleCloseEvent(e);
    if (!handled) {
        return QWidget::event(e);
    }
    return true;
}

bool ListerPane::handleCloseEvent(QEvent *e)
{
    if (e->type() == QEvent::ShortcutOverride) {
        auto *ke = dynamic_cast<QKeyEvent *>(e);
        if (ke->key() == Qt::Key_Escape) {
            if (_lister->isSearchEnabled()) {
                _lister->searchDelete();
                _lister->enableSearch(false);
                ke->accept();
                return true;
            }
            if (!_lister->textArea()->getSelectedText().isEmpty()) {
                _lister->textArea()->clearSelection();
                ke->accept();
                return true;
            }
        }
    }
    return false;
}

ListerBrowserExtension::ListerBrowserExtension(Lister *lister)
    : KParts::NavigationExtension(lister)
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
    ListerEncodingMenu(Lister *lister, const QString &text, const QString &icon, KActionCollection *parent)
        : KrRemoteEncodingMenu(text, icon, parent)
        , _lister(lister)
    {
    }

protected:
    QString currentCharacterSet() override
    {
        return _lister->characterSet();
    }

    void chooseDefault() override
    {
        _lister->setCharacterSet(QString());
    }

    void chooseEncoding(QString encodingName) override
    {
        _lister->setCharacterSet(encodingName);
    }

    Lister *_lister;
};

Lister::Lister(QWidget *parent)
    : KParts::ReadOnlyPart(parent)
{
    setXMLFile("krusaderlisterui.rc");

    _actionSaveSelected = new QAction(Icon("document-save"), i18n("Save selection..."), this);
    connect(_actionSaveSelected, &QAction::triggered, this, &Lister::saveSelected);
    actionCollection()->addAction("save_selected", _actionSaveSelected);

    _actionSaveAs = new QAction(Icon("document-save-as"), i18n("Save as..."), this);
    connect(_actionSaveAs, &QAction::triggered, this, &Lister::saveAs);
    actionCollection()->addAction("save_as", _actionSaveAs);

    _actionPrint = new QAction(Icon("document-print"), i18n("Print..."), this);
    connect(_actionPrint, &QAction::triggered, this, &Lister::print);
    actionCollection()->addAction("print", _actionPrint);
    actionCollection()->setDefaultShortcut(_actionPrint, Qt::CTRL | Qt::Key_P);

    _actionSearch = new QAction(Icon("system-search"), i18n("Search"), this);
    connect(_actionSearch, &QAction::triggered, this, &Lister::searchAction);
    actionCollection()->addAction("search", _actionSearch);
    actionCollection()->setDefaultShortcut(_actionSearch, Qt::CTRL | Qt::Key_F);

    _actionSearchNext = new QAction(Icon("go-down"), i18n("Search next"), this);
    connect(_actionSearchNext, &QAction::triggered, this, &Lister::searchNext);
    actionCollection()->addAction("search_next", _actionSearchNext);
    actionCollection()->setDefaultShortcut(_actionSearchNext, Qt::Key_F3);

    _actionSearchPrev = new QAction(Icon("go-up"), i18n("Search previous"), this);
    connect(_actionSearchPrev, &QAction::triggered, this, &Lister::searchPrev);
    actionCollection()->addAction("search_prev", _actionSearchPrev);
    actionCollection()->setDefaultShortcut(_actionSearchPrev, Qt::SHIFT | Qt::Key_F3);

    _actionJumpToPosition = new QAction(Icon("go-jump"), i18n("Jump to position"), this);
    connect(_actionJumpToPosition, &QAction::triggered, this, &Lister::jumpToPosition);
    actionCollection()->addAction("jump_to_position", _actionJumpToPosition);
    actionCollection()->setDefaultShortcut(_actionJumpToPosition, Qt::CTRL | Qt::Key_G);

    _actionHexMode = new QAction(Icon("document-preview"), i18n("Hex mode"), this);
    connect(_actionHexMode, &QAction::triggered, this, &Lister::toggleHexMode);
    actionCollection()->addAction("hex_mode", _actionHexMode);
    actionCollection()->setDefaultShortcut(_actionHexMode, Qt::CTRL | Qt::Key_H);

    new ListerEncodingMenu(this, i18n("Select charset"), "character-set", actionCollection());

    QWidget *widget = new ListerPane(this, parent);
    widget->setFocusPolicy(Qt::StrongFocus);
    auto *grid = new QGridLayout(widget);
    _textArea = new ListerTextArea(this, widget);
    _textArea->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    _textArea->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    _textArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _textArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    widget->setFocusProxy(_textArea);
    grid->addWidget(_textArea, 0, 0);
    _scrollBar = new QScrollBar(Qt::Vertical, widget);
    grid->addWidget(_scrollBar, 0, 1);
    _scrollBar->hide();

    QWidget *statusWidget = new QWidget(widget);
    auto *hbox = new QHBoxLayout(statusWidget);

    _listerLabel = new QLabel(i18n("Lister:"), statusWidget);
    hbox->addWidget(_listerLabel);
    _searchProgressBar = new QProgressBar(statusWidget);
    _searchProgressBar->setMinimum(0);
    _searchProgressBar->setMaximum(1000);
    _searchProgressBar->setValue(0);
    _searchProgressBar->hide();
    hbox->addWidget(_searchProgressBar);

    _searchStopButton = new QToolButton(statusWidget);
    _searchStopButton->setIcon(Icon("process-stop"));
    _searchStopButton->setToolTip(i18n("Stop search"));
    _searchStopButton->hide();
    connect(_searchStopButton, &QToolButton::clicked, this, &Lister::searchDelete);
    hbox->addWidget(_searchStopButton);

    _searchLineEdit = new KLineEdit(statusWidget);
    _searchLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    _originalBackground = _searchLineEdit->palette().color(QPalette::Base);
    _originalForeground = _searchLineEdit->palette().color(QPalette::Text);

    connect(_searchLineEdit, &KLineEdit::KLINEEDIT_RETURNKEYPRESSED, this, &Lister::searchNext);
    connect(_searchLineEdit, &KLineEdit::textChanged, this, &Lister::searchTextChanged);

    hbox->addWidget(_searchLineEdit);
    _searchNextButton = new QPushButton(Icon("go-down"), i18n("Next"), statusWidget);
    _searchNextButton->setToolTip(i18n("Jump to next match"));
    connect(_searchNextButton, &QPushButton::clicked, this, &Lister::searchNext);
    hbox->addWidget(_searchNextButton);
    _searchPrevButton = new QPushButton(Icon("go-up"), i18n("Previous"), statusWidget);
    _searchPrevButton->setToolTip(i18n("Jump to previous match"));
    connect(_searchPrevButton, &QPushButton::clicked, this, &Lister::searchPrev);
    hbox->addWidget(_searchPrevButton);
    _searchOptions = new QPushButton(i18n("Options"), statusWidget);
    _searchOptions->setToolTip(i18n("Modify search behavior"));
    auto *menu = new QMenu();
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

    hbox->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    _statusLabel = new QLabel(statusWidget);
    hbox->addWidget(_statusLabel);

    grid->addWidget(statusWidget, 1, 0, 1, 2);
    setWidget(widget);

    connect(_scrollBar, &QScrollBar::actionTriggered, _textArea, &ListerTextArea::slotActionTriggered);
    connect(&_searchUpdateTimer, &QTimer::timeout, this, &Lister::slotUpdate);

    new ListerBrowserExtension(this);
    enableSearch(false);

    _tempFile = new QTemporaryFile(this);
    _tempFile->setFileTemplate(QDir::tempPath() + QLatin1String("/krusader_lister.XXXXXX"));
}

bool Lister::openUrl(const QUrl &listerUrl)
{
    _downloading = false;
    setUrl(listerUrl);

    _fileSize = 0;

    if (listerUrl.isLocalFile()) {
        _filePath = listerUrl.path();
        if (!QFile::exists(_filePath))
            return false;
        _fileSize = getFileSize();
    } else {
        if (_tempFile->isOpen()) {
            _tempFile->close();
        }
        _tempFile->open();

        _filePath = _tempFile->fileName();

        KIO::TransferJob *downloadJob = KIO::get(listerUrl, KIO::NoReload, KIO::HideProgressInfo);

        connect(downloadJob, &KIO::TransferJob::data, this, [=](KIO::Job *, QByteArray array) {
            if (array.size() != 0) {
                _tempFile->write(array);
            }
        });
        connect(downloadJob, &KIO::TransferJob::result, this, [=](KJob *job) {
            _tempFile->flush();
            if (job->error()) { /* any error occurred? */
                auto *kioJob = qobject_cast<KIO::TransferJob *>(job);
                KMessageBox::error(_textArea, i18n("Error reading file %1.", kioJob->url().toDisplayString(QUrl::PreferLocalFile)));
            }
            _downloading = false;
            _downloadUpdateTimer.stop();
            slotUpdate();
        });
        connect(&_downloadUpdateTimer, &QTimer::timeout, this, [&]() {
            slotUpdate();
        });
        _downloadUpdateTimer.start(500);
        _downloading = true;
    }

    // invalidate cache
    _cache.clear();

    _textArea->reset();
    emit started(nullptr);
    emit setWindowCaption(listerUrl.toDisplayString());
    emit completed();
    return true;
}

QByteArray Lister::cacheChunk(const qint64 filePos, const qint64 maxSize)
{
    if (filePos >= _fileSize) {
        return QByteArray();
    }

    qint64 size = maxSize;
    if (_fileSize - filePos < size) {
        size = _fileSize - filePos;
    }

    if (!_cache.isEmpty() && (filePos >= _cachePos) && (filePos + size <= _cachePos + _cache.size())) {
        return _cache.mid(static_cast<int>(filePos - _cachePos), static_cast<int>(size));
    }

    const int negativeOffset = CACHE_SIZE * 2 / 5;
    qint64 cachePos = filePos - negativeOffset;
    if (cachePos < 0)
        cachePos = 0;

    QFile sourceFile(_filePath);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }

    if (!sourceFile.seek(cachePos)) {
        return QByteArray();
    }

    const QByteArray bytes = sourceFile.read(CACHE_SIZE);
    if (bytes.isEmpty()) {
        return bytes;
    }

    _cache = bytes;
    _cachePos = cachePos;
    const qint64 cacheRefIndex = filePos - _cachePos;
    qint64 newSize = bytes.size() - cacheRefIndex;
    if (newSize < size)
        size = newSize;

    return _cache.mid(static_cast<int>(cacheRefIndex), static_cast<int>(size));
}

qint64 Lister::getFileSize()
{
    return QFile(_filePath).size();
}

void Lister::guiActivateEvent(KParts::GUIActivateEvent *event)
{
    if (event->activated()) {
        slotUpdate();
        _textArea->redrawTextArea(true);
    } else {
        enableSearch(false);
    }
    KParts::ReadOnlyPart::guiActivateEvent(event);
}

void Lister::slotUpdate()
{
    const qint64 oldSize = _fileSize;
    _fileSize = getFileSize();
    if (oldSize != _fileSize)
        _textArea->sizeChanged();

    int cursorX = 0, cursorY = 0;
    _textArea->getCursorPosition(cursorX, cursorY);
    bool isfirst = false;
    const qint64 cursor = _textArea->getCursorPosition(isfirst);

    const int percent = (_fileSize == 0) ? 0 : (int)((201 * cursor) / _fileSize / 2);

    const QString status = i18n("Column: %1, Position: %2 (%3, %4%)", cursorX, cursor, _fileSize, percent);
    _statusLabel->setText(status);

    if (_searchProgressCounter)
        _searchProgressCounter--;
}

bool Lister::isSearchEnabled()
{
    return !_searchLineEdit->isHidden() || !_searchProgressBar->isHidden();
}

void Lister::enableSearch(const bool enable)
{
    if (enable) {
        _listerLabel->setText(i18n("Search:"));
        _searchLineEdit->show();
        _searchNextButton->show();
        _searchPrevButton->show();
        _searchOptions->show();
        if (!_searchLineEdit->hasFocus()) {
            _searchLineEdit->setFocus();
            const QString selection = _textArea->getSelectedText();
            if (!selection.isEmpty()) {
                _searchLineEdit->setText(selection);
            }
            _searchLineEdit->selectAll();
        }
    } else {
        _listerLabel->setText(i18n("Lister:"));
        _searchLineEdit->hide();
        _searchNextButton->hide();
        _searchPrevButton->hide();
        _searchOptions->hide();
        _textArea->setFocus();
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

void Lister::search(const bool forward, const bool restart)
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
    const bool caseSensitive = _caseSensitiveAction->isChecked();
    const bool matchWholeWord = _matchWholeWordsOnlyAction->isChecked();
    const bool regExp = _regExpAction->isChecked();
    const bool hex = _hexAction->isChecked();

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
            const QString hexData = hexcontent.left(2);
            hexcontent = hexcontent.mid(2);
            bool ok = true;
            const int c = hexData.toUInt(&ok, 16);
            if (!ok) {
                setColor(false, false);
                return;
            }
            _searchHexQuery.push_back((char)c);
        }
    } else {
        _searchQuery.setContent(_searchLineEdit->text(), caseSensitive, matchWholeWord, codec()->name(), regExp);
    }
    _searchIsForward = forward;
    _searchHexadecimal = hex;

    QTimer::singleShot(0, this, &Lister::slotSearchMore);
    _searchInProgress = true;
    _searchProgressCounter = 3;

    enableActions(false);
}

void Lister::enableActions(const bool state)
{
    _actionSearch->setEnabled(state);
    _actionSearchNext->setEnabled(state);
    _actionSearchPrev->setEnabled(state);
    _actionJumpToPosition->setEnabled(state);
    if (state) {
        _searchUpdateTimer.stop();
    } else {
        slotUpdate();
    }
}

void Lister::slotSearchMore()
{
    if (!_searchInProgress)
        return;

    if (!_searchUpdateTimer.isActive()) {
        _searchUpdateTimer.start(200);
    }

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

    qint64 maxCacheSize = SEARCH_CACHE_CHARS;
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

    const QByteArray chunk = cacheChunk(searchPos, maxCacheSize);
    if (chunk.isEmpty()) {
        searchFailed();
        return;
    }

    const qint64 chunkSize = chunk.size();

    qint64 foundAnchor = -1;
    qint64 foundCursor = -1;
    qint64 byteCounter = 0;

    if (_searchHexadecimal) {
        const qsizetype ndx = _searchIsForward ? chunk.indexOf(_searchHexQuery) : chunk.lastIndexOf(_searchHexQuery);
        if (chunkSize > _searchHexQuery.length()) {
            if (_searchIsForward) {
                _searchPosition = searchPos + chunkSize;
                if ((_searchPosition < _fileSize) && (chunkSize > _searchHexQuery.length()))
                    _searchPosition -= _searchHexQuery.length();
                byteCounter = _searchPosition - searchPos;
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
        qint64 rowStart = 0;
        QString row = "";

        QScopedPointer<QTextDecoder> decoder(_codec->makeDecoder());

        while (byteCounter < chunkSize) {
            const QString chr = decoder->toUnicode(chunk.mid(static_cast<int>(byteCounter++), 1));
            if (chr.isEmpty() && byteCounter < chunkSize) {
                continue;
            }

            if (chr != "\n")
                row += chr;

            if (chr == "\n" || row.length() >= SEARCH_MAX_ROW_LEN || byteCounter >= chunkSize) {
                if (setPosition) {
                    _searchPosition = searchPos + byteCounter;
                    if (!_searchIsForward) {
                        _searchPosition++;
                        setPosition = false;
                    }
                }

                if (_searchQuery.checkLine(row, !_searchIsForward)) {
                    QByteArray cachedBuffer = chunk.mid(static_cast<int>(rowStart), static_cast<int>(chunkSize - rowStart));
                    QString decoded = _codec->toUnicode(cachedBuffer);
                    QTextStream stream(&decoded);

                    stream.read(_searchQuery.matchIndex());
                    foundAnchor = searchPos + rowStart + stream.pos();

                    stream.read(_searchQuery.matchLength());
                    foundCursor = searchPos + rowStart + stream.pos();

                    if (_searchIsForward)
                        break;
                }

                row = "";
                rowStart = byteCounter;
            }
        }
    }

    if (foundAnchor != -1 && foundCursor != -1) {
        _textArea->setAnchorAndCursor(foundAnchor, foundCursor);
        searchSucceeded();
        return;
    }

    if (_searchIsForward && searchPos + byteCounter >= _fileSize) {
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

    QTimer::singleShot(0, this, &Lister::slotSearchMore);
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
            const qint64 anchor = _textArea->getCursorAnchor();
            const qint64 cursor = _textArea->getCursorPosition(isfirst);
            if (cursor > anchor && anchor != -1) {
                _textArea->setCursorPositionInDocument(anchor, true);
            }
            search(true, true);
        }
    }
}

void Lister::setColor(const bool match, const bool restore)
{
    QColor fore, back;

    if (!restore) {
        const KConfigGroup gc(krConfig, "Colors");

        QString foreground, background;
        const QPalette p = QGuiApplication::palette();

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
            fore = p.color(QPalette::Active, QPalette::Text);
        else if (!gc.readEntry(foreground, QString()).isEmpty())
            fore = gc.readEntry(foreground, fore);

        if (gc.readEntry(background, QString()) == "KDE default")
            back = p.color(QPalette::Active, QPalette::Base);
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

        // otherwise focus is set to document tab
        _textArea->setFocus();
    }

    const qint64 pcnt = (_fileSize == 0) ? 1000 : (2001 * _searchPosition) / _fileSize / 2;
    const auto pctInt = (int)pcnt;
    if (_searchProgressBar->value() != pctInt)
        _searchProgressBar->setValue(pctInt);
}

void Lister::jumpToPosition()
{
    bool ok = true;
    QString res = QInputDialog::getText(_textArea, i18n("Jump to position"), i18n("Text position:"), QLineEdit::Normal, "0", &ok);
    if (!ok)
        return;

    res = res.trimmed();
    qint64 pos = -1;
    if (res.startsWith(QLatin1String("0x"))) {
        res = res.mid(2);
        bool ok;
        const qulonglong upos = res.toULongLong(&ok, 16);
        if (!ok) {
            KMessageBox::error(_textArea, i18n("Invalid number."), i18n("Jump to position"));
            return;
        }
        pos = (qint64)upos;
    } else {
        bool ok;
        const qulonglong upos = res.toULongLong(&ok);
        if (!ok) {
            KMessageBox::error(_textArea, i18n("Invalid number."), i18n("Jump to position"));
            return;
        }
        pos = (qint64)upos;
    }

    if (pos < 0 || pos > _fileSize) {
        KMessageBox::error(_textArea, i18n("Number out of range."), i18n("Jump to position"));
        return;
    }

    _textArea->deleteAnchor();
    _textArea->setCursorPositionInDocument(pos, true);
    _textArea->ensureVisibleCursor();
}

void Lister::saveAs()
{
    const QUrl url = QFileDialog::getSaveFileUrl(_textArea, i18n("Lister"));
    if (url.isEmpty())
        return;
    QUrl sourceUrl;
    if (!_downloading)
        sourceUrl = QUrl::fromLocalFile(_filePath);
    else
        sourceUrl = this->url();

    QList<QUrl> urlList;
    urlList << sourceUrl;

    KIO::Job *job = KIO::copy(urlList, url);
    job->setUiDelegate(KIO::createDefaultJobUiDelegate());
    KIO::getJobTracker()->registerJob(job);
    job->uiDelegate()->setAutoErrorHandlingEnabled(true);
}

void Lister::saveSelected()
{
    bool isfirst;
    const qint64 start = _textArea->getCursorAnchor();
    const qint64 end = _textArea->getCursorPosition(isfirst);
    if (start == -1 || start == end) {
        KMessageBox::error(_textArea, i18n("Nothing is selected."), i18n("Save selection..."));
        return;
    }
    if (start > end) {
        _savePosition = end;
        _saveEnd = start;
    } else {
        _savePosition = start;
        _saveEnd = end;
    }

    const QUrl url = QFileDialog::getSaveFileUrl(_textArea, i18n("Lister"));
    if (url.isEmpty())
        return;

    KIO::TransferJob *saveJob = KIO::put(url, -1, KIO::Overwrite);
    connect(saveJob, &KIO::TransferJob::dataReq, this, &Lister::slotDataSend);
    connect(saveJob, &KIO::TransferJob::result, this, &Lister::slotSendFinished);

    saveJob->setUiDelegate(KIO::createDefaultJobUiDelegate());
    KIO::getJobTracker()->registerJob(saveJob);
    saveJob->uiDelegate()->setAutoErrorHandlingEnabled(true);

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
    array = cacheChunk(_savePosition, (int)max);
    _savePosition += array.size();
}

void Lister::slotSendFinished(KJob *)
{
    _actionSaveSelected->setEnabled(true);
}

void Lister::setCharacterSet(const QString &set)
{
    _characterSet = set;
    if (_characterSet.isEmpty()) {
        _codec = QTextCodec::codecForLocale();
    } else {
        // Should move from this with KF6 migration
        _codec = QTextCodec::codecForName(_characterSet.toUtf8());
        Q_ASSERT(_codec != nullptr);
    }
    _textArea->redrawTextArea(true);
}

void Lister::print()
{
    bool isfirst;
    const qint64 anchor = _textArea->getCursorAnchor();
    const qint64 cursor = _textArea->getCursorPosition(isfirst);
    const bool hasSelection = (anchor != -1 && anchor != cursor);

    const QString docName = url().fileName();
    QPrinter printer;
    printer.setDocName(docName);

    QScopedPointer<QPrintDialog> printDialog(new QPrintDialog(&printer, _textArea));

    if (hasSelection) {
        printDialog->setOption(QAbstractPrintDialog::PrintSelection);
    }

    if (!printDialog->exec()) {
        return;
    }

    if (printer.pageOrder() == QPrinter::LastPageFirst) {
        switch (KMessageBox::warningContinueCancel(_textArea, i18n("Reverse printing is not supported. Continue with normal printing?"))) {
        case KMessageBox::Continue:
            break;
        default:
            return;
        }
    }
    QPainter painter;
    painter.begin(&printer);

    const QString dateString = QLocale().toString(QDate::currentDate(), QLocale::ShortFormat);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    const QRect pageRect = printer.pageLayout().paintRectPixels(printer.resolution());
#else
    const QRect pageRect = printer.pageRect();
#endif
    const QRect drawingRect(0, 0, pageRect.width(), pageRect.height());

    const QFont normalFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    const QFontMetrics fmNormal(normalFont);
    const int normalFontHeight = fmNormal.height();

    const QFontMetrics fmFixed(fixedFont);
    const int fixedFontHeight = std::max(fmFixed.height(), 1);
    const int fixedFontWidth = std::max(fmFixed.horizontalAdvance("W"), 1);

    const int effPageSize = drawingRect.height() - normalFontHeight - 1;
    const int rowsPerPage = std::max(effPageSize / fixedFontHeight, 1);
    const int columnsPerPage = std::max(drawingRect.width() / fixedFontWidth, 1);

    bool firstPage = true;

    qint64 startPos = 0;
    qint64 endPos = _fileSize;
    if (printer.printRange() == QPrinter::Selection) {
        if (anchor > cursor)
            startPos = cursor, endPos = anchor;
        else
            startPos = anchor, endPos = cursor;
    }

    int page = 0;
    while (startPos < endPos) {
        page++;

        QStringList rows = readLines(startPos, endPos, columnsPerPage, rowsPerPage);

        // print since set-up fromPage number
        if (printer.fromPage() && page < printer.fromPage()) {
            continue;
        }

        // print until set-up toPage number
        if (printer.toPage() && printer.toPage() >= printer.fromPage() && page > printer.toPage())
            break;

        if (!firstPage) {
            printer.newPage();
        }
        firstPage = false;
        // Use the painter to draw on the page.
        painter.setFont(normalFont);

        painter.drawText(drawingRect, Qt::AlignLeft, dateString);
        painter.drawText(drawingRect, Qt::AlignHCenter, docName);
        painter.drawText(drawingRect, Qt::AlignRight, QString("%1").arg(page));

        painter.drawLine(0, normalFontHeight, drawingRect.width(), normalFontHeight);

        painter.setFont(fixedFont);
        int yOffset = normalFontHeight + 1;
        for (const QString &row : std::as_const(rows)) {
            painter.drawText(0, yOffset + fixedFontHeight, row);
            yOffset += fixedFontHeight;
        }
    }
}

QStringList Lister::readLines(qint64 &filePos, const qint64 endPos, const int columns, const int lines)
{
    if (_textArea->hexMode()) {
        return readHexLines(filePos, endPos, columns, lines);
    }
    QStringList list;
    const int maxBytes = std::min(columns * lines * MAX_CHAR_LENGTH, (int)(endPos - filePos));
    if (maxBytes <= 0) {
        return list;
    }
    const QByteArray chunk = cacheChunk(filePos, maxBytes);
    if (chunk.isEmpty()) {
        return list;
    }

    QScopedPointer<QTextDecoder> decoder(_codec->makeDecoder());

    int byteCounter = 0;
    QString row = "";
    bool skipImmediateNewline = false;
    while (byteCounter < chunk.size() && list.size() < lines) {
        QString chr = decoder->toUnicode(chunk.mid(byteCounter++, 1));
        if (chr.isEmpty()) {
            continue;
        }

        // replace unreadable characters
        if ((chr[0] < QChar(32)) && (chr[0] != '\n') && (chr[0] != '\t')) {
            chr = QChar(' ');
        }

        // handle newline
        if (chr == "\n") {
            if (!skipImmediateNewline) {
                list << row;
                row = "";
            }
            skipImmediateNewline = false;
            continue;
        }

        skipImmediateNewline = false;

        // handle tab
        if (chr == "\t") {
            const qsizetype tabLength = _textArea->tabWidth() - (row.length() % _textArea->tabWidth());
            if (row.length() + tabLength > columns) {
                list << row;
                row = "";
            }
            row += QString(tabLength, QChar(' '));
        } else {
            // normal printable character
            row += chr;
        }

        if (row.length() >= columns) {
            list << row;
            row = "";
            skipImmediateNewline = true;
        }
    }

    if (list.size() < lines) {
        list << row;
    }

    filePos += byteCounter;

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
    if (positionDigits < 8) {
        return 8;
    }
    return positionDigits;
}

int Lister::hexBytesPerLine(const int columns)
{
    const int positionDigits = hexPositionDigits();
    if (columns >= positionDigits + 5 + 128) {
        return 32;
    }
    if (columns >= positionDigits + 5 + 64) {
        return 16;
    }
    return 8;
}

QStringList Lister::readHexLines(qint64 &filePos, const qint64 endPos, const int columns, const int lines)
{
    const int positionDigits = hexPositionDigits();
    const int bytesPerRow = hexBytesPerLine(columns);

    QStringList list;

    const qint64 choppedPos = (filePos / bytesPerRow) * bytesPerRow;
    const int maxBytes = std::min(bytesPerRow * lines, (int)(endPos - choppedPos));
    if (maxBytes <= 0)
        return list;

    const QByteArray chunk = cacheChunk(choppedPos, maxBytes);
    if (chunk.isEmpty())
        return list;

    int cnt = 0;
    for (int l = 0; l < lines; l++) {
        if (filePos >= endPos) {
            break;
        }

        const qint64 printPos = (filePos / bytesPerRow) * bytesPerRow;
        QString pos;
        pos.setNum(printPos, 16);
        while (pos.length() < positionDigits)
            pos = QString("0") + pos;
        pos = QString("0x") + pos;
        pos += QString(": ");

        QString charData;

        for (int i = 0; i != bytesPerRow; ++i, ++cnt) {
            const qint64 currentPos = printPos + i;
            if (currentPos < filePos || currentPos >= endPos) {
                pos += QString("   ");
                charData += QString(" ");
            } else {
                char c = chunk.at(cnt);
                auto charCode = (int)c;
                if (charCode < 0)
                    charCode += 256;
                QString hex;
                hex.setNum(charCode, 16);
                if (hex.length() < 2)
                    hex = QString("0") + hex;
                pos += hex + QString(" ");
                if (c < 32)
                    c = '.';
                charData += QChar(c);
            }
        }

        pos += QString(" ") + charData;
        list << pos;
        filePos = printPos + bytesPerRow;
    }

    if (filePos > endPos) {
        filePos = endPos;
    }

    return list;
}

int Lister::hexIndexToPosition(const int columns, const int index)
{
    const int positionDigits = hexPositionDigits();
    const int bytesPerRow = hexBytesPerLine(columns);
    const int finalIndex = std::min(index, bytesPerRow);

    return positionDigits + 4 + (3 * finalIndex);
}

int Lister::hexPositionToIndex(const int columns, const int position)
{
    const int positionDigits = hexPositionDigits();
    const int bytesPerRow = hexBytesPerLine(columns);

    int finalPosition = position;
    finalPosition -= 4 + positionDigits;
    if (finalPosition <= 0)
        return 0;

    finalPosition /= 3;
    if (finalPosition >= bytesPerRow)
        return bytesPerRow;
    return finalPosition;
}

void Lister::toggleHexMode()
{
    setHexMode(!_textArea->hexMode());
}

void Lister::setHexMode(const bool mode)
{
    if (mode) {
        _textArea->setHexMode(true);
        _actionHexMode->setText(i18n("Text mode"));
    } else {
        _textArea->setHexMode(false);
        _actionHexMode->setText(i18n("Hex mode"));
    }
}
