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

#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QtCore/QFile>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtGui/QClipboard>
#include <QtGui/QSpacerItem>
#include <QtGui/QProgressBar>
#include <QKeyEvent>
#include <QScrollBar>
#include <QtCore/QRect>
#include <QHBoxLayout>
#include <QMenu>
#include <KColorScheme>
#include <KTemporaryFile>
#include <KMessageBox>
#include <KActionCollection>

#include <klocale.h>
#include <kio/job.h>
#include <kglobalsettings.h>
#include "../krusader.h"

#define  SEARCH_CACHE_CHARS 100000
#define  SEARCH_MAX_ROW_LEN 4000

/* TODO: Implement jump to position */
/* TODO: Implement text codecs */
/* TODO: Implement save selected */
/* TODO: Implement right click */
/* TODO: Implement print */
/* TODO: Implement hex viewer */
/* TODO: Implement size checking for viewing text files */
/* TODO: Implement isFirst at fileToTextPosition */

ListerTextArea::ListerTextArea( Lister *lister, QWidget *parent ) : QTextEdit( parent ), _lister( lister ),
  _sizeX( -1 ), _sizeY( -1 ), _cursorAnchorPos( -1 ), _inSliderOp( false ), _inCursorUpdate( false )
{
  connect( this, SIGNAL( cursorPositionChanged() ), this, SLOT( slotCursorPositionChanged() ) );
  _tabWidth = 4;
  setWordWrapMode( QTextOption::NoWrap );
  setLineWrapMode( QTextEdit::NoWrap );
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
  if( _cursorAnchorPos > _lister->fileSize() )
    _cursorAnchorPos = -1;
  if( _cursorPos > _lister->fileSize() )
   _cursorPos = _lister->fileSize();

  redrawTextArea(true);
}

void ListerTextArea::resizeEvent ( QResizeEvent * event )
{
  QTextEdit::resizeEvent( event );
  redrawTextArea();
}

void ListerTextArea::calculateText( bool forcedUpdate )
{
  QFontMetrics fm( font() );

  int fontHeight = fm.height();
  if( fontHeight < 1 )
    fontHeight = 1;

  QRect crect = viewport()->contentsRect();
  int windowHeight = crect.height();

  int sizeY = windowHeight / fontHeight;
  _pageSize = sizeY;

  QList<qint64> rowStarts;

  int fontWidth = fm.width("W");
  if( fontWidth < 1 )
    fontWidth = 1;
  int windowWidth = crect.width() - fontWidth / 2;
  if( windowWidth < 1 )
    windowWidth = 0;

  setTabStopWidth( fontWidth * _tabWidth );

  int sizeX = windowWidth / fontWidth;

  _sizeChanged = (_sizeY != sizeY) || (_sizeX != sizeX) || forcedUpdate;
  _sizeY = sizeY;
  _sizeX = sizeX;

  QStringList list = readLines( _screenStartPos, _screenEndPos, _sizeY, &rowStarts );

  if( _sizeChanged )
  {
    _averagePageSize = _screenEndPos - _screenStartPos;
    setUpScrollBar();
  }

  rowStarts << _screenEndPos;

  QStringList listRemn = readLines( _screenEndPos, _screenEndPos, 1 );
    list << listRemn;

  if( list != _rowContent ) {
    setPlainText( list.join("\n") );
    _rowContent = list;
    _rowStarts = rowStarts;
  }
}

qint64 ListerTextArea::textToFilePosition( int x, int y, bool &isfirst )
{
  isfirst = (x == 0);
  if( y >= _rowStarts.count() )
    return -1;
  qint64 rowStart = _rowStarts[ y ];
  if( x == 0 )
    return rowStart;

  // we can't use fromUnicode because of the invalid encoded chars
  int maxBytes = 2 * _sizeX * MAX_CHAR_LENGTH;
  char * cache = _lister->cacheRef( rowStart, maxBytes );
  QByteArray cachedBuffer( cache, maxBytes );

  QTextStream stream( &cachedBuffer );
  stream.setCodec( codec() );
  stream.read( x );
  return rowStart + stream.pos();
}

void ListerTextArea::fileToTextPosition( qint64 p, bool /* isfirst */, int &x, int &y )
{
  if( p < _screenStartPos || p > _screenEndPos || _rowStarts.count() < 1 )
  {
    x = -1;
    y = ( p > _screenEndPos ) ? -2 : -1;
  } else {
    y = 0;
    while( y < _rowStarts.count() && _rowStarts[ y ] <= p )
      y++;
    y--;
    if( y < 0 )
    {
      x = y = -1;
      return;
    }

    int maxBytes = 2 * _sizeX * MAX_CHAR_LENGTH;
    qint64 rowStart = _rowStarts[ y ];
    x = 0;
    if( rowStart >= p )
    {
      return;
    }

    char * cache = _lister->cacheRef( rowStart, maxBytes );
    QByteArray cachedBuffer( cache, p - rowStart );

    QString res = codec()->toUnicode( cachedBuffer );
    x=res.length();
  }
}

void ListerTextArea::getCursorPosition( int &x, int &y )
{
  getScreenPosition( textCursor().position(), x, y );
}

void ListerTextArea::getScreenPosition( int position, int &x, int &y )
{
  x = position;
  y = 0;
  for( int i=0; i < _rowContent.count(); i++ )
  {
    int rowLen = _rowContent[ i ].length() + 1;
    if( x >= rowLen )
    {
      x -= rowLen;
      y++;
    } else
      break;
  }
}

void ListerTextArea::setCursorPosition( int x, int y, int anchorX, int anchorY )
{
  _inCursorUpdate = true;
  if( x == -1 || y < 0 )
  {
    setCursorWidth( 0 );
    if( anchorY == -1 )
    {
      _inCursorUpdate = false;
      return;
    }

    if( y == -2 )
    {
      y = _sizeY;
      x = ( _rowContent.count() > _sizeY ) ? _rowContent[ _sizeY ].length() : 0;
    }
    else
      x = y = 0;
  }
  else
    setCursorWidth( 2 );

  QTextCursor::MoveMode mode = QTextCursor::MoveAnchor;
  if( anchorX != -1 && anchorY != -1 )
  {
    if( anchorY >= _sizeY )
    {
      moveCursor( QTextCursor::End, mode );
      moveCursor( QTextCursor::StartOfLine, mode );
    }
    else
    {
      moveCursor( QTextCursor::Start, mode );
      for( int i=0; i < anchorY; i++ )
        moveCursor( QTextCursor::Down, mode );
    }

    if( _rowContent.count() > anchorY && anchorX > _rowContent[ anchorY ].length() )
      anchorX = _rowContent[ anchorY ].length();

    for( int j=0; j < anchorX; j++ )
      moveCursor( QTextCursor::Right, mode );

    mode = QTextCursor::KeepAnchor;
  }

  if( y >= _sizeY )
  {
    moveCursor( QTextCursor::End, mode );
    moveCursor( QTextCursor::StartOfLine, mode );
  }
  else
  {
    moveCursor( QTextCursor::Start, mode );
    for( int i=0; i < y; i++ )
      moveCursor( QTextCursor::Down, mode );
  }

  if( _rowContent.count() > y && x > _rowContent[ y ].length() )
    x = _rowContent[ y ].length();

  for( int j=0; j < x; j++ )
    moveCursor( QTextCursor::Right, mode );

  _inCursorUpdate = false;
}

qint64 ListerTextArea::getCursorPosition( bool &isfirst )
{
  if( cursorWidth() == 0 )
  {
    isfirst = _cursorAtFirstColumn;
    return _cursorPos;
  }

  int x, y;
  getCursorPosition( x, y );
  return textToFilePosition( x, y, isfirst );
}

void ListerTextArea::setCursorPosition( qint64 p, bool isfirst )
{
  _cursorPos = p;
  int x,y;
  fileToTextPosition( p, isfirst, x, y );

  int anchorX = -1, anchorY = -1;
  if( _cursorAnchorPos != -1 && _cursorAnchorPos != p )
  {
    int anchPos = _cursorAnchorPos;
    if( anchPos < _screenStartPos ) {
      anchPos = _screenStartPos;
      anchorY = -2;
    }
    if( anchPos > _screenEndPos ) {
      anchPos = _screenEndPos;
      anchorY = -3;
    }

    fileToTextPosition( anchPos, isfirst, anchorX, anchorY );
  }
  setCursorPosition( x, y, anchorX, anchorY );
}

void ListerTextArea::slotCursorPositionChanged()
{
  if( _inCursorUpdate )
    return;
  int cursorX, cursorY;
  getCursorPosition( cursorX, cursorY );
  _cursorAtFirstColumn = (cursorX == 0);
  _cursorPos = textToFilePosition( cursorX, cursorY, _cursorAtFirstColumn );
  setCursorWidth( 2 );
  //fprintf( stderr, "Cursor pos: %d %d %Ld\n", cursorX, cursorY, _cursorPos );
}

QString ListerTextArea::readSection( qint64 p1, qint64 p2 )
{
  if( p1 == p2 )
    return QString();

  if( p1 > p2 )
  {
    qint64 tmp = p1;
    p1 = p2;
    p2 = tmp;
  }

  QString section;
  qint64 pos = p1;

  QTextCodec * textCodec = codec();
  QTextDecoder * decoder = textCodec->makeDecoder();

  do {
    int maxBytes = _sizeX * _sizeY * MAX_CHAR_LENGTH;
    if( maxBytes > ( p2 - pos ) )
      maxBytes = (int)(p2 - pos);
    char * cache = _lister->cacheRef( pos, maxBytes );
    if( cache == 0 || maxBytes == 0 )
      break;
    section += decoder->toUnicode( cache, maxBytes );
    pos += maxBytes;
  }while( pos < p2 );

  delete decoder;
  return section;
}

QStringList ListerTextArea::readLines( qint64 filePos, qint64 &endPos, int lines, QList<qint64> * locs )
{
  endPos = filePos;
  QStringList list;
  int maxBytes = _sizeX * _sizeY * MAX_CHAR_LENGTH;
  char * cache = _lister->cacheRef( filePos, maxBytes );
  if( cache == 0 || maxBytes == 0 )
    return list;

  QTextCodec * textCodec = codec();
  QTextDecoder * decoder = textCodec->makeDecoder();

  int cnt = 0;
  int y=0;
  QString row = "";
  int effLength = 0;
  if( locs )
    (*locs)<< filePos;
  bool isLastLongLine = false;
  while( cnt < maxBytes && y < lines )
  {
    int lastCnt = cnt;
    QString chr = decoder->toUnicode( cache + (cnt++), 1 );
    if( !chr.isEmpty() )
    {
      if( chr == "\n" )
      {
        if( !isLastLongLine )
        {
          list << row;
          effLength = 0;
          row = "";
          y++;
          if( locs )
            (*locs)<< filePos + cnt;
        }
        isLastLongLine = false;
      } else {
        isLastLongLine = false;
        if( chr == "\t" )
        {
          effLength += _tabWidth - (effLength % _tabWidth ) - 1;
          if( effLength > _sizeX )
          {
            list << row;
            effLength = 0;
            row = "";
            y++;
            if( locs )
              (*locs)<< filePos + lastCnt;
          }
        }
        row += chr;
        effLength++;
        if( effLength >= _sizeX )
        {
          list << row;
          effLength = 0;
          row = "";
          y++;
          if( locs )
            (*locs)<< filePos + cnt;
          isLastLongLine = true;
        }
      }
    }
  }

  if( y < lines )
    list << row;

  if( locs )
  {
    while( locs->count() > lines )
    {
      locs->removeLast();
    }
  }

  endPos = filePos + cnt;

  delete decoder;
  return list;
}

QTextCodec * ListerTextArea::codec()
{
  return QTextCodec::codecForMib( 4 ); // TODO: implement text codecs
}

void ListerTextArea::setUpScrollBar()
{
  if( _averagePageSize == _lister->fileSize() )
  {
    _lister->scrollBar()->setPageStep( 0 );
    _lister->scrollBar()->setMaximum( 0 );
    _lister->scrollBar()->hide();
    _lastPageStartPos = 0;
  } else {
    int maxPage = MAX_CHAR_LENGTH * _sizeX * _sizeY;
    qint64 pageStartPos = _lister->fileSize() - maxPage;
    qint64 endPos;
    if( pageStartPos < 0 )
      pageStartPos = 0;
    QStringList list = readLines( pageStartPos, endPos, maxPage );
    if( list.count() <= _sizeY )
    {
      _lastPageStartPos = 0;
    } else {
      readLines( pageStartPos, _lastPageStartPos, list.count() - _sizeY );
    }

    int maximum = ( _lastPageStartPos > SLIDER_MAX ) ? SLIDER_MAX : _lastPageStartPos;
    int pageSize = ( _lastPageStartPos > SLIDER_MAX ) ? SLIDER_MAX * _averagePageSize / _lastPageStartPos : _averagePageSize;
    if( pageSize == 0 )
      pageSize++;

    _lister->scrollBar()->setPageStep( pageSize );
    _lister->scrollBar()->setMaximum( maximum );
    _lister->scrollBar()->show();
  }
}

void ListerTextArea::keyPressEvent( QKeyEvent * ke )
{
  if( Krusader::actCopy->shortcut().contains( QKeySequence( ke->key() | ke->modifiers() ) ) )
  {
    copySelectedToClipboard();
    ke->accept();
    return;
  }

  if( ke->modifiers() == Qt::NoModifier || ke->modifiers() == Qt::ShiftModifier )
  {
    qint64 newAnchor = -1;
    if( ke->modifiers() == Qt::ShiftModifier )
    {
      newAnchor = _cursorAnchorPos;
      if( _cursorAnchorPos == -1 )
        newAnchor = _cursorPos;
    }

    switch( ke->key() )
    {
    case Qt::Key_F3:
      ke->accept();
      if( ke->modifiers() == Qt::ShiftModifier )
        _lister->searchPrev();
      else
        _lister->searchNext();
      return;
    case Qt::Key_Home:
    case Qt::Key_End:
      _cursorAnchorPos = newAnchor;
      break;
    case Qt::Key_Left:
      {
        _cursorAnchorPos = newAnchor;
        ensureVisibleCursor();
        int x,y;
        getCursorPosition( x, y );
        if( y == 0 && x == 0 )
          slotActionTriggered( QAbstractSlider::SliderSingleStepSub );
      }
      break;
    case Qt::Key_Right:
      {
        _cursorAnchorPos = newAnchor;
        ensureVisibleCursor();
        if( textCursor().position() == toPlainText().length() )
          slotActionTriggered( QAbstractSlider::SliderSingleStepAdd );
      }
      break;
    case Qt::Key_Up:
      {
        _cursorAnchorPos = newAnchor;
        ensureVisibleCursor();
        int x,y;
        getCursorPosition( x, y );
        if( y == 0 )
          slotActionTriggered( QAbstractSlider::SliderSingleStepSub );
      }
      break;
    case Qt::Key_Down:
      {
        _cursorAnchorPos = newAnchor;
        ensureVisibleCursor();
        int x,y;
        getCursorPosition( x, y );
        if( y >= _sizeY )
          slotActionTriggered( QAbstractSlider::SliderSingleStepAdd );
      }
      break;
    case Qt::Key_PageDown:
      {
        _cursorAnchorPos = newAnchor;
        ensureVisibleCursor();
        ke->accept();
        int x,y;
        getCursorPosition( x, y );
        slotActionTriggered( QAbstractSlider::SliderPageStepAdd );
        y += _sizeY - _skippedLines;
        if( y > _rowContent.count() )
        {
          y = _rowContent.count();
          if( y > 0 )
            x = _rowContent[ y - 1 ].length();
          else
            x = 0;
        }
        _cursorPos = textToFilePosition( x, y, _cursorAtFirstColumn );
        setCursorPosition( _cursorPos, _cursorAtFirstColumn );
      }
      return;
    case Qt::Key_PageUp:
      {
        _cursorAnchorPos = newAnchor;
        ensureVisibleCursor();
        ke->accept();
        int x,y;
        getCursorPosition( x, y );
        slotActionTriggered( QAbstractSlider::SliderPageStepSub );
        y -= _sizeY - _skippedLines;
        if( y < 0 )
        {
          y = 0;
          x = 0;
        }
        _cursorPos = textToFilePosition( x, y, _cursorAtFirstColumn );
        setCursorPosition( _cursorPos, _cursorAtFirstColumn );
      }
      return;
    }
  }
  if( ke->modifiers() == Qt::ControlModifier )
  {
    switch( ke->key() )
    {
    case Qt::Key_F:
      ke->accept();
      _lister->enableSearch( true );
      return;
    case Qt::Key_Home:
      _cursorAnchorPos = -1;
      ke->accept();
      slotActionTriggered( QAbstractSlider::SliderToMinimum );
      setCursorPosition( (qint64)0, true );
      return;
    case Qt::Key_A:
    case Qt::Key_End:
      {
        _cursorAnchorPos =  ( ke->key() == Qt::Key_A ) ? 0 : -1;
        ke->accept();
        slotActionTriggered( QAbstractSlider::SliderToMaximum );
        qint64 endPos = _lister->fileSize();
        setCursorPosition( endPos, true );
        return;
      }
    case Qt::Key_Down:
      ke->accept();
      slotActionTriggered( QAbstractSlider::SliderSingleStepAdd );
      return;
    case Qt::Key_Up:
      ke->accept();
      slotActionTriggered( QAbstractSlider::SliderSingleStepSub );
      return;
    case Qt::Key_PageDown:
      ke->accept();
      slotActionTriggered( QAbstractSlider::SliderPageStepAdd );
      return;
    case Qt::Key_PageUp:
      ke->accept();
      slotActionTriggered( QAbstractSlider::SliderPageStepSub );
      return;
    }
  }
  int oldAnchor = textCursor().anchor();
  QTextEdit::keyPressEvent( ke );
  handleAnchorChange( oldAnchor );
}

void ListerTextArea::mousePressEvent( QMouseEvent * e )
{
  _cursorAnchorPos = -1;
  int oldAnchor = textCursor().anchor();
  QTextEdit::mousePressEvent( e );
  handleAnchorChange( oldAnchor );
}

void ListerTextArea::mouseDoubleClickEvent( QMouseEvent * e )
{
  _cursorAnchorPos = -1;
  int oldAnchor = textCursor().anchor();
  QTextEdit::mouseDoubleClickEvent( e );
  handleAnchorChange( oldAnchor );
}

void ListerTextArea::mouseMoveEvent( QMouseEvent * e )
{
  if( e->pos().y() < 0 )
  {
    slotActionTriggered( QAbstractSlider::SliderSingleStepSub );
  } else if( e->pos().y() > height() )
  {
    slotActionTriggered( QAbstractSlider::SliderSingleStepAdd );
  }
  if( _cursorAnchorPos == -1 )
    _cursorAnchorPos = _cursorPos;
  QTextEdit::mouseMoveEvent( e );
  if( _cursorAnchorPos == _cursorPos )
    _cursorAnchorPos = -1;
}

void ListerTextArea::wheelEvent ( QWheelEvent * e )
{
  int delta = e->delta();
  if( delta )
  {
    if( delta > 0 )
    {
      e->accept();
      while( delta > 0 )
      {
        slotActionTriggered( QAbstractSlider::SliderSingleStepSub );
        slotActionTriggered( QAbstractSlider::SliderSingleStepSub );
        slotActionTriggered( QAbstractSlider::SliderSingleStepSub );
        delta -= 120;
      }
    }
    else
    {
      e->accept();
      while( delta < 0 )
      {
        slotActionTriggered( QAbstractSlider::SliderSingleStepAdd );
        slotActionTriggered( QAbstractSlider::SliderSingleStepAdd );
        slotActionTriggered( QAbstractSlider::SliderSingleStepAdd );
        delta+=120;
      }
    }
  }
}

void ListerTextArea::slotActionTriggered ( int action )
{
  switch (action) {
  case QAbstractSlider::SliderSingleStepAdd:
    {
      qint64 endPos;
      readLines( _screenStartPos, endPos, 1 );
      if( endPos <= _lastPageStartPos )
      {
        _screenStartPos = endPos;
      }
    }
    break;
  case QAbstractSlider::SliderSingleStepSub:
    {
      if( _screenStartPos == 0 )
        break;

      int maxSize = _sizeX * _sizeY * MAX_CHAR_LENGTH;
      qint64 readPos = _screenStartPos - maxSize;
      if( readPos < 0 )
        readPos = 0;
      qint64 previousPos = readPos;

      while( readPos < _screenStartPos )
      {
        previousPos = readPos;
        readLines( readPos, readPos, 1 );
      }

      _screenStartPos = previousPos;
    }
    break;
  case QAbstractSlider::SliderPageStepAdd:
    {
      _skippedLines = 0;

      qint64 endPos;
      for( int i=0; i < _sizeY; i++ )
      {
        readLines( _screenStartPos, endPos, 1 );
        if( endPos <= _lastPageStartPos )
        {
          _screenStartPos = endPos;
          _skippedLines++;
        } else {
          break;
        }
      }
    }
    break;
  case QAbstractSlider::SliderPageStepSub:
    {
      _skippedLines = 0;

      if( _screenStartPos == 0 )
        break;

      int maxSize = 2 * _sizeX * _sizeY * MAX_CHAR_LENGTH;
      qint64 readPos = _screenStartPos - maxSize;
      if( readPos < 0 )
        readPos = 0;

      int numRows = _sizeY;
      if( numRows < 1 )
        numRows = 1;
      qint64 previousPoses[ numRows ];
      for( int i=0; i != numRows; i++ )
        previousPoses[ i ] = -1;

      int circularCounter = 0;
      while( readPos < _screenStartPos )
      {
        previousPoses[ circularCounter ]  = readPos;
        circularCounter = (circularCounter + 1) % numRows;
        readLines( readPos, readPos, 1 );
      }

      _skippedLines = _sizeY;
      while( previousPoses[ circularCounter ] == -1 )
      {
        circularCounter = (++circularCounter) % numRows;
        _skippedLines--;
      }

      _screenStartPos = previousPoses[ circularCounter ];
    }
    break;
  case QAbstractSlider::SliderToMinimum:
    _screenStartPos = 0;
    break;
  case QAbstractSlider::SliderToMaximum:
    _screenStartPos = _lastPageStartPos;
    break;
  case QAbstractSlider::SliderMove:
    {
      if( _inSliderOp ) // self created call?
        return;
      qint64 pos = _lister->scrollBar()->sliderPosition();

      if( pos == SLIDER_MAX )
      {
        _screenStartPos = _lastPageStartPos;
        break;
      }
      else if( pos == 0 )
      {
        _screenStartPos = 0;
        break;
      }

      if( _lastPageStartPos > SLIDER_MAX )
        pos = _lastPageStartPos * pos / SLIDER_MAX;

      if( pos != 0 )
      {
        int maxSize = _sizeX * _sizeY * MAX_CHAR_LENGTH;
        qint64 readPos = pos - maxSize;
        if( readPos < 0 )
          readPos = 0;
        qint64 previousPos = readPos;

        while( readPos <= pos )
        {
          previousPos = readPos;
          readLines( readPos, readPos, 1 );
        }
 
        pos = previousPos;
      }

      _screenStartPos = pos;
    }
    break;
  case QAbstractSlider::SliderNoAction:
    break;
  };

  _inSliderOp = true;
  int value = ( _lastPageStartPos > SLIDER_MAX ) ? SLIDER_MAX * _screenStartPos / _lastPageStartPos : _screenStartPos;
  _lister->scrollBar()->setSliderPosition( value );
  _inSliderOp = false;

  redrawTextArea();
}

void ListerTextArea::redrawTextArea( bool forcedUpdate )
{
  bool isfirst;
  qint64 pos = getCursorPosition( isfirst );
  calculateText( forcedUpdate );
  setCursorPosition( pos, isfirst );
}

void ListerTextArea::ensureVisibleCursor()
{
  if( _cursorPos < _screenStartPos || _cursorPos > _screenEndPos )
  {
    int delta = _sizeY / 2;
    if( delta == 0 )
      delta++;

    qint64 newScreenStart = _cursorPos;
    while( delta )
    {
      int maxSize = _sizeX * MAX_CHAR_LENGTH;
      qint64 readPos = newScreenStart - maxSize;
      if( readPos < 0 )
        readPos = 0;

      qint64 previousPos = readPos;

      while( readPos < newScreenStart )
      {
        previousPos = readPos;
        readLines( readPos, readPos, 1 );
      }

      newScreenStart = previousPos;
      delta--;
    }

    _screenStartPos = newScreenStart;
    slotActionTriggered( QAbstractSlider::SliderNoAction );
  }
}

void ListerTextArea::setAnchorAndCursor( qint64 anchor, qint64 cursor )
{
  _cursorAnchorPos = anchor;
  setCursorPosition( cursor, false );
  ensureVisibleCursor();
}

void ListerTextArea::copySelectedToClipboard()
{
  if( _cursorAnchorPos != -1 && _cursorAnchorPos != _cursorPos )
  {
    QString selection = readSection( _cursorAnchorPos, _cursorPos );
    QApplication::clipboard()->setText( selection );
  }
}

void ListerTextArea::handleAnchorChange( int oldAnchor )
{
  int cursor = textCursor().position();
  int anchor = textCursor().anchor();

  if( anchor == cursor )
  {
    _cursorAnchorPos = -1;
  } else {
    if( oldAnchor != anchor )
    {
      int x,y;
      bool isfirst;
      getScreenPosition( anchor, x, y );
      _cursorAnchorPos = textToFilePosition( x, y, isfirst );
    }
  }
}

ListerBrowserExtension::ListerBrowserExtension( Lister * lister ) : KParts::BrowserExtension( lister )
{
  _lister = lister;

  emit enableAction( "copy", true );
}

void ListerBrowserExtension::copy()
{
  _lister->textArea()->copySelectedToClipboard();
}

Lister::Lister( QWidget *parent ) : KParts::ReadOnlyPart( parent ), _searchInProgress( false ), _cache( 0 ), _active( false ), _searchLastFailedPosition( -1 ),
  _searchProgressCounter( 0 ), _tempFile( 0 )
{
  setXMLFile( "krusaderlisterui.rc" );

  _actionSearch = new KAction(KIcon("system-search"), i18n("Search"), this);
  _actionSearch->setShortcut( Qt::CTRL + Qt::Key_F );
  connect(_actionSearch, SIGNAL(triggered(bool)), SLOT(searchAction()));
  actionCollection()->addAction( "search", _actionSearch );

  _actionSearchNext = new KAction(KIcon("go-down"), i18n("Search next"), this);
  _actionSearchNext->setShortcut( Qt::Key_F3 );
  connect(_actionSearchNext, SIGNAL(triggered(bool)), SLOT(searchNext()));
  actionCollection()->addAction( "search_next", _actionSearchNext );

  _actionSearchPrev = new KAction(KIcon("go-up"), i18n("Search previous"), this);
  _actionSearchPrev->setShortcut( Qt::SHIFT + Qt::Key_F3 );
  connect(_actionSearchPrev, SIGNAL(triggered(bool)), SLOT(searchPrev()));
  actionCollection()->addAction( "search_prev", _actionSearchPrev );

  QWidget * widget = new QWidget( parent );
  widget->setFocusPolicy( Qt::StrongFocus );
  QGridLayout *grid = new QGridLayout( widget );
  _textArea = new ListerTextArea( this, widget );
  _textArea->setFont( KGlobalSettings::fixedFont() );
  _textArea->setTextInteractionFlags( Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  _textArea->setCursorWidth( 2 );
  _textArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  _textArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  widget->setFocusProxy( _textArea );
  grid->addWidget( _textArea, 0, 0 );
  _scrollBar = new QScrollBar( Qt::Vertical, widget );
  grid->addWidget( _scrollBar, 0, 1 );
  _scrollBar->hide();

  QWidget * statusWidget = new QWidget( widget );
  QHBoxLayout *hbox = new QHBoxLayout( statusWidget );

  _listerLabel = new QLabel( i18n( "Lister:" ), statusWidget );
  hbox->addWidget( _listerLabel );
  _searchProgressBar = new QProgressBar( statusWidget );
  _searchProgressBar->setMinimum( 0 );
  _searchProgressBar->setMaximum( 1000 );
  _searchProgressBar->setValue( 0 );
  _searchProgressBar->hide();
  hbox->addWidget( _searchProgressBar );

  _searchStopButton = new QToolButton( statusWidget );
  _searchStopButton->setIcon( KIcon("process-stop") );
  _searchStopButton->setToolTip( i18n( "Stop search" ) );
  _searchStopButton->hide();
  connect( _searchStopButton, SIGNAL( clicked() ), this, SLOT( searchDelete() ) );
  hbox->addWidget( _searchStopButton );

  _searchLineEdit = new QLineEdit( statusWidget );
  _searchLineEdit->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

  _originalBackground = _searchLineEdit->palette().color( QPalette::Base );
  _originalForeground = _searchLineEdit->palette().color( QPalette::Text );

  connect( _searchLineEdit, SIGNAL( returnPressed() ), this, SLOT( searchNext() ) );
  connect( _searchLineEdit, SIGNAL( textChanged ( const QString & ) ), this, SLOT( searchDelete() ) );

  hbox->addWidget( _searchLineEdit );
  _searchNextButton = new QPushButton( KIcon("go-down"), i18n( "Next" ), statusWidget );
  _searchNextButton->setToolTip( i18n( "Jump to next match" ) );
  connect( _searchNextButton, SIGNAL( clicked() ), this, SLOT( searchNext() ) );
  hbox->addWidget( _searchNextButton );
  _searchPrevButton = new QPushButton( KIcon("go-up"), i18n( "Previous" ), statusWidget );
  _searchPrevButton->setToolTip( i18n( "Jump to previous match" ) );
  connect( _searchPrevButton, SIGNAL( clicked() ), this, SLOT( searchPrev() ) );
  hbox->addWidget( _searchPrevButton );
  _searchOptions = new QPushButton( i18n( "Options" ), statusWidget );
  _searchOptions->setToolTip( i18n( "Modify search behavior" ) );
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
  _searchOptions->setMenu( menu );

  hbox->addWidget( _searchOptions );

  QSpacerItem* cbSpacer = new QSpacerItem ( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  hbox->addItem ( cbSpacer );

  _statusLabel = new QLabel( statusWidget );
  hbox->addWidget( _statusLabel );

  grid->addWidget( statusWidget, 1, 0, 1, 2 );
  setWidget( widget );

  connect( _scrollBar, SIGNAL( actionTriggered ( int ) ), _textArea, SLOT( slotActionTriggered( int ) ) );
  connect( &_updateTimer, SIGNAL(timeout()), this, SLOT(slotUpdate()));

  _updateTimer.setSingleShot( false );

  new ListerBrowserExtension( this );
  enableSearch( false );
}

Lister::~Lister()
{
  if( _cache != 0 )
  {
    delete []_cache;
    _cache = 0;
  }
  if( _tempFile != 0 )
  {
    delete _tempFile;
    _tempFile = 0;
  }
}

bool Lister::openUrl( const KUrl &listerUrl )
{
  setUrl( listerUrl );

  if( _tempFile )
  {
    delete _tempFile;
    _tempFile = 0;
  }
  _fileSize = 0;

  if( listerUrl.isLocalFile() )
  {
    _filePath = listerUrl.path();
    if( !QFile::exists( _filePath ) )
      return false;
    _fileSize = getFileSize();
  }
  else
  {
    _tempFile = new KTemporaryFile();
    _tempFile->setSuffix( listerUrl.fileName() );
    _tempFile->open();

    _filePath = _tempFile->fileName();

     KIO::Job * downloadJob = KIO::get( listerUrl, KIO::NoReload, KIO::HideProgressInfo );

     connect(downloadJob, SIGNAL(data(KIO::Job *, const QByteArray &)),
             this, SLOT(slotFileDataReceived(KIO::Job *, const QByteArray &)));
     connect(downloadJob, SIGNAL(result(KJob*)),
             this, SLOT(slotFileFinished(KJob *)));
  }
  if( _cache )
  {
    delete []_cache;
    _cache = 0;
  }
  _textArea->reset();
  emit started( 0 );
  emit setWindowCaption( listerUrl.prettyUrl() );
  emit completed();
  return true;
}

void Lister::slotFileDataReceived(KIO::Job *, const QByteArray &array)
{
  if( array.size() != 0 )
    _tempFile->write( array );
}

void Lister::slotFileFinished(KJob *job)
{
  _tempFile->flush();
  if( job->error() )    /* any error occurred? */
  {
    KIO::TransferJob *kioJob = (KIO::TransferJob *)job;
    KMessageBox::error(0, i18n("Error reading file %1!", kioJob->url().pathOrUrl() ) );
  }
}


char * Lister::cacheRef( qint64 filePos, int &size )
{
  if( filePos >= _fileSize )
    return 0;
  if( _fileSize - filePos < size )
    size = _fileSize - filePos;
  if( (_cache != 0) && ( filePos >= _cachePos ) && (filePos + size <= _cachePos + _cacheSize) )
    return _cache + ( filePos - _cachePos );

  int cacheSize = size * LISTER_CACHE_FACTOR;
  int negativeOffset = size * (LISTER_CACHE_FACTOR/2);
  qint64 cachePos = filePos - negativeOffset;
  if( cachePos < 0 )
    cachePos = 0;

  QFile myfile( _filePath );
  if( !myfile.open( QIODevice::ReadOnly ) ) {
    myfile.close();
    return 0;
  }

  if( !myfile.seek( cachePos ) ) {
    myfile.close();
    return 0;
  }

  char * newCache = new char [ cacheSize ];

  qint64 bytes = myfile.read( newCache, cacheSize );
  if( bytes == -1 ) {
    delete []newCache;
    myfile.close();
    return 0;
  }
  if( _cache )
    delete []_cache;

  _cache = newCache;
  _cacheSize = bytes;
  _cachePos = cachePos;

  return _cache + ( filePos - _cachePos );
}

qint64 Lister::getFileSize()
{
  return QFile( _filePath ).size();
}

void Lister::guiActivateEvent( KParts::GUIActivateEvent * event )
{
  if( event->activated() )
  {
    _active = true;
    _updateTimer.setInterval( 150 );
    _updateTimer.start();
    slotUpdate();
  }
  else
  {
    enableSearch( false );
    _active = false;
    _updateTimer.stop();
  }
  KParts::ReadOnlyPart::guiActivateEvent( event );
}

void Lister::slotUpdate()
{
  qint64 oldSize = _fileSize;
  _fileSize = getFileSize();
  if( oldSize != _fileSize )
    _textArea->sizeChanged();

  int cursorX = 0, cursorY = 0;
  _textArea->getCursorPosition( cursorX, cursorY );
  bool isfirst = false;
  qint64 cursor = _textArea->getCursorPosition( isfirst );

  int percent = (_fileSize == 0) ? 0 : (int)((201 * cursor) / _fileSize / 2);

  QString status = i18n( "Column: %1, Position: %2 (%3, %4%)" )
                     .arg( cursorX, 3, 10, QChar( ' ' ) )
                     .arg( cursor ).arg( _fileSize ).arg( percent );
  _statusLabel->setText( status );

  if( _searchProgressCounter )
    _searchProgressCounter--;
}

void Lister::enableSearch( bool enable )
{
  if( enable )
  {
    _listerLabel->setText( i18n( "Search:" ) );
    _searchLineEdit->show();
    _searchNextButton->show();
    _searchPrevButton->show();
    _searchOptions->show();
    _searchLineEdit->setFocus();
  } else {
    _listerLabel->setText( i18n( "Lister:" ) );
    _searchLineEdit->hide();
    _searchNextButton->hide();
    _searchPrevButton->hide();
    _searchOptions->hide();
  }
}

void Lister::searchNext()
{
  search( true );
}

void Lister::searchPrev()
{
  search( false );
}

void Lister::search( bool forward )
{
  if( _searchInProgress || _searchLineEdit->text().isEmpty() )
    return;
  if( _searchLineEdit->isHidden() )
    enableSearch( true );

  _searchPosition = forward ? 0 : _fileSize;
  if( _fromCursorAction->isChecked() )
  {
    bool isfirst;
    qint64 cursor = _textArea->getCursorPosition( isfirst );
    if( cursor != 0 && !forward )
      cursor--;
    if( _searchLastFailedPosition == -1 || _searchLastFailedPosition != cursor )
      _searchPosition = cursor;
  }
  bool caseSensitive = _caseSensitiveAction->isChecked();
  bool matchWholeWord = _matchWholeWordsOnlyAction->isChecked();
  bool regExp = _regExpAction->isChecked();

  _searchQuery.setContent( _searchLineEdit->text(), caseSensitive, matchWholeWord, false, _textArea->codec()->name(), regExp );
  _searchIsForward = forward;

  QTimer::singleShot( 0, this, SLOT( slotSearchMore() ) );
  _searchInProgress = true;
  _searchProgressCounter = 3;

  enableActions( false );
}

void Lister::enableActions( bool state )
{
  _actionSearch->setEnabled( state );
  _actionSearchNext->setEnabled( state );
  _actionSearchPrev->setEnabled( state );
}

void Lister::slotSearchMore()
{
  if( !_searchInProgress )
    return;

  updateProgressBar();
  if( !_searchIsForward )
    _searchPosition--;

  if( _searchPosition < 0 || _searchPosition >= _fileSize )
  {
    searchFailed();
    return;
  }

  int maxCacheSize = SEARCH_CACHE_CHARS;
  qint64 searchPos = _searchPosition;
  bool setPosition = true;
  if( !_searchIsForward )
  {
    qint64 origSearchPos = _searchPosition;
    searchPos -= maxCacheSize;
    if( searchPos <= 0 )
    {
      searchPos = 0;
      _searchPosition = 0;
      setPosition = false;
    }
    qint64 diff = origSearchPos - searchPos;
    if( diff < maxCacheSize )
      maxCacheSize = diff;
  }

  char * cache = cacheRef( searchPos, maxCacheSize );
  if( cache == 0 || maxCacheSize == 0 )
  {
    searchFailed();
    return;
  }

  QTextCodec * textCodec = _textArea->codec();
  QTextDecoder * decoder = textCodec->makeDecoder();

  int cnt = 0;
  int rowStart = 0;

  QString row = "";
  qint64 foundAnchor = -1;
  qint64 foundCursor = -1;

  while( cnt < maxCacheSize )
  {
    QString chr = decoder->toUnicode( cache + (cnt++), 1 );
    if( !chr.isEmpty() || cnt >= maxCacheSize )
    {
      if( chr != "\n" )
        row += chr;

      if( chr == "\n" || row.length() >= SEARCH_MAX_ROW_LEN || cnt >= maxCacheSize )
      {
        if( setPosition )
        {
          _searchPosition = searchPos + cnt;
          if( !_searchIsForward )
          {
            _searchPosition ++;
            setPosition = false;
          }
        }

        if( _searchQuery.checkLine( row, !_searchIsForward ) )
        {
          QByteArray cachedBuffer( cache + rowStart, maxCacheSize - rowStart );

          QTextStream stream( &cachedBuffer );
          stream.setCodec( textCodec );

          stream.read( _searchQuery.matchIndex() );
          foundAnchor = searchPos + rowStart + stream.pos();

          stream.read( _searchQuery.matchLength() );
          foundCursor = searchPos + rowStart + stream.pos();

          if( _searchIsForward )
            break;
        }

        row = "";
        rowStart = cnt;
      }
    }
  }

  delete decoder;

  if( foundAnchor != -1 && foundCursor != -1 ) {
    _textArea->setAnchorAndCursor( foundAnchor, foundCursor );
    searchSucceeded();
    return;
  }

  if( _searchIsForward && searchPos + cnt >= _fileSize )
  {
    searchFailed();
    return;
  }

  if( _searchPosition <= 0 || _searchPosition >= _fileSize )
  {
    searchFailed();
    return;
  }

  QTimer::singleShot( 0, this, SLOT( slotSearchMore() ) );
}

void Lister::searchSucceeded()
{
  _searchInProgress = false;
  setColor( true, false );
  hideProgressBar();
  _searchLastFailedPosition = -1;

  enableActions( true );
}

void Lister::searchFailed()
{
  _searchInProgress = false;
  setColor( false, false );
  hideProgressBar();
  bool isfirst;
  _searchLastFailedPosition = _textArea->getCursorPosition( isfirst );

  enableActions( true );
}

void Lister::searchDelete()
{
  _searchInProgress = false;
  setColor( false, true );
  hideProgressBar();
  _searchLastFailedPosition = -1;

  enableActions( true );
}

void Lister::setColor( bool match, bool restore ) {
  QColor  fore, back;

  if( !restore )
  {
    KConfigGroup gc( krConfig, "Colors");

    QString foreground, background;

    if( match )
    {
      foreground = "Quicksearch Match Foreground";
      background = "Quicksearch Match Background";
      fore = Qt::black;
      back = QColor( 192, 255, 192 );
    } else {
      foreground = "Quicksearch Non-match Foreground";
      background = "Quicksearch Non-match Background";
      fore = Qt::black;
      back = QColor(255,192,192);
    }

    if( gc.readEntry( foreground, QString() ) == "KDE default" )
      fore = KColorScheme(QPalette::Active, KColorScheme::View).foreground().color();
    else if( !gc.readEntry( foreground, QString() ).isEmpty() )
      fore = gc.readEntry( foreground, fore );

    if( gc.readEntry( background, QString() ) == "KDE default" )
      back = KColorScheme(QPalette::Active, KColorScheme::View).background().color();
    else if( !gc.readEntry( background, QString() ).isEmpty() )
      back = gc.readEntry( background, back );
   } else {
      back = _originalBackground;
      fore = _originalForeground;
   }

   QPalette pal = _searchLineEdit->palette();
   pal.setColor( QPalette::Base, back);
   pal.setColor( QPalette::Text, fore);
   _searchLineEdit->setPalette( pal );
}

void Lister::hideProgressBar()
{
  if( !_searchProgressBar->isHidden() )
  {
    _searchProgressBar->hide();
    _searchStopButton->hide();
    _searchLineEdit->show();
    _searchNextButton->show();
    _searchPrevButton->show();
    _searchOptions->show();
    _listerLabel->setText( i18n( "Search:" ) );
  }
}

void Lister::updateProgressBar()
{
  if( _searchProgressCounter )
    return;

  if( _searchProgressBar->isHidden() )
  {
    _searchProgressBar->show();
    _searchStopButton->show();
    _searchOptions->hide();
    _searchLineEdit->hide();
    _searchNextButton->hide();
    _searchPrevButton->hide();
    _listerLabel->setText( i18n( "Search position:" ) );
  }

  qint64 pcnt = (_fileSize == 0) ? 1000 : (2001 * _searchPosition ) / _fileSize / 2;
  int pctInt = (int)pcnt;
  if( _searchProgressBar->value() != pctInt )
    _searchProgressBar->setValue( pctInt );
}
