/***************************************************************************
                        splittergui.h  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
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

#ifndef __SPLITTERGUI_H__
#define __SPLITTERGUI_H__

#include <qdialog.h>
#include <qstring.h>
#include <qspinbox.h>
#include <qvalidator.h>
#include <qcombobox.h>
#include <kurlrequester.h>
#include <kio/global.h>

struct PredefinedDevice
{
  QString name;
  KIO::filesize_t capacity;
};

class SplitterGUI : QDialog
{
  Q_OBJECT

public:
  class   SplitterSpinBox;
  
private:
  int                             predefinedDeviceNum;
  KIO::filesize_t                 userDefinedSize;
  int                             lastSelectedDevice;
  int                             resultCode;

  static PredefinedDevice predefinedDevices[];
  QString fileToSplit;

  SplitterSpinBox *spinBox;
  QComboBox       *deviceCombo;
  QComboBox       *sizeCombo;
  KURLRequester   *urlReq;
  
public:
  SplitterGUI( QWidget* parent,  QString fileName, QString defaultDir );

  QString getDestinationDir()     {return urlReq->url();}
  KIO::filesize_t getSplitSize()  {return spinBox->longValue();}
  int     result()                {return resultCode;}

public slots:
  virtual void sizeComboActivated( int item );
  virtual void predefinedComboActivated( int item );
  virtual void splitPressed();

protected:
  virtual void keyPressEvent( QKeyEvent *e );

public:
  class SplitterSpinBox : public QSpinBox
  {
  private:
    KIO::filesize_t division;
    KIO::filesize_t value;
    
  public:
    SplitterSpinBox ( QWidget * parent = 0, const char * name = 0 ) : QSpinBox( parent, name ), division( 1 ), value( 1 )
    {
      setMaxValue( 0x7FFFFFFF );     /* setting the minimum and maximum values */
      setMinValue( 1 );
      QDoubleValidator *dval = new QDoubleValidator( this );
      setValidator ( dval );
    }

    void setLongValue( KIO::filesize_t valueIn ) {
      value = valueIn;
      if( value == 0 )
        value++;
      updateDisplay();
    }
    
    KIO::filesize_t longValue() {
      KIO::filesize_t val = (KIO::filesize_t)( division * text().toDouble() + 0.5 ) ;
      if( val == 0 )
        val++;
      return val;
    }
    
    QString mapValueToText( int )
    {
      QString frac("");
      
      KIO::filesize_t int_part  = value / division;
      KIO::filesize_t frac_mod = value % division;
            
      if( frac_mod )
      {
        KIO::filesize_t frac_part = (KIO::filesize_t)((1000. * frac_mod) /division + 0.5);

        if( frac_part )
        {
          frac = QString( "%1" ).arg( frac_part ).rightJustify( 3, '0' );
          frac = "." + frac;
          while( frac.endsWith("0") )
            frac.truncate( frac.length() - 1 );
        }
      }

      return QString( "%1%2" ).arg( int_part ).arg( frac );
    }

    int mapTextToValue( bool * )
    {
      value = longValue();
    
      if( value > 0x7FFFFFFF )
        return 0x7FFFFFFF;
      else
        return value;
    }

    void setDivision( KIO::filesize_t div )
    {
      division = div;
      updateDisplay();
    }
    
  public slots:
    
    void stepUp()
    {
      value = longValue();
    
      if( value + division > value )
        value += division;
      updateDisplay();
    }
    
    void stepDown()
    {
      value = longValue();
    
      if( value < division + 1 )
        value = 1;
      else
        value -= division;
      updateDisplay();     
    }
  };
};

#endif /* __SPLITTERGUI_H__ */
