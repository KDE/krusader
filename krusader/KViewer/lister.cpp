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
#include <QtCore/QFile>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtGui/QClipboard>
#include <QKeyEvent>
#include <QScrollBar>
#include <QtCore/QRect>

#include <klocale.h>
#include <kglobalsettings.h>
#include "../krusader.h"

/* TODO: Implement status line */
/* TODO: Implement search */
/* TODO: Implement implement select line event*/
/* TODO: Implement implement select word event */
/* TODO: Implement document change detection */
/* TODO: Implement jump to position */
/* TODO: Implement text codecs */
/* TODO: Implement right click */
/* TODO: Implement print */
/* TODO: Implement remote listener */
/* TODO: Implement hex viewer */
/* TODO: Implement size checking for viewing text files */

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

void ListerTextArea::resizeEvent ( QResizeEvent * event )
{
  QTextEdit::resizeEvent( event );
  redrawTextArea();
}

void ListerTextArea::calculateText()
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

  _sizeChanged = (_sizeY != sizeY) || (_sizeX != sizeX);
  _sizeY = sizeY;
  _sizeX = sizeX;

  QStringList list = readLines( _screenStartPos, _screenEndPos, _sizeY, &rowStarts );

  if( _sizeChanged )
  {
    _averagePageSize = _screenEndPos - _screenStartPos;
    setUpScrollBar();
  }

  rowStarts << _screenEndPos;
  qint64 endPos;
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
  stream.read( x );
  return rowStart + stream.pos();
}

void ListerTextArea::fileToTextPosition( qint64 p, bool isfirst, int &x, int &y )
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
  x = textCursor().position();
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
    p2 = p1;
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
  QTextEdit::keyPressEvent( ke );
}

void ListerTextArea::mousePressEvent( QMouseEvent * e )
{
  _cursorAnchorPos = -1;
  QTextEdit::mousePressEvent( e );
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
        circularCounter = (++circularCounter) % numRows;
        qint64 prevPos = readPos;
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

void ListerTextArea::redrawTextArea()
{
  bool isfirst;
  qint64 pos = getCursorPosition( isfirst );
  calculateText();
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

void ListerTextArea::copySelectedToClipboard()
{
  if( _cursorAnchorPos != -1 && _cursorAnchorPos != _cursorPos )
  {
    QString selection = readSection( _cursorAnchorPos, _cursorPos );
    QApplication::clipboard()->setText( selection );
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

Lister::Lister( QWidget *parent ) : KParts::ReadOnlyPart( parent ), _cache( 0 )
{
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
  QLabel *label = new QLabel( i18n( "Lister" ), widget );
  grid->addWidget( label, 1, 0, 1, 2 );
  setWidget( widget );

  connect( _scrollBar, SIGNAL( actionTriggered ( int ) ), _textArea, SLOT( slotActionTriggered( int ) ) );

  new ListerBrowserExtension( this );
}

Lister::~Lister()
{
  if( _cache != 0 )
  {
    delete []_cache;
    _cache = 0;
  }
}

bool Lister::openFile()
{
  KUrl listerUrl = url();
  if( listerUrl.isLocalFile() )
  {
    _filePath = listerUrl.path();
    if( !QFile::exists( _filePath ) )
      return false;
    _fileSize = getFileSize();
  }
  else
  {
    /* TODO: implement remote lister */
  }
  if( _cache )
  {
    delete []_cache;
    _cache = 0;
  }
  _textArea->reset();
  return true;
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
