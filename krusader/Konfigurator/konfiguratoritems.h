/***************************************************************************
                      konfiguratoritems.h  -  description
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

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __KONFIGURATOR_ITEMS_H__
#define __KONFIGURATOR_ITEMS_H__

#include <qobject.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qptrlist.h>
#include <qvaluelist.h>
#include <qvaluevector.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <kurlrequester.h>
#include <qhbox.h>
#include <kfontdialog.h>
#include <qlabel.h>
#include <qfont.h>
#include <qtoolbutton.h>
#include <qcombobox.h>

class KonfiguratorExtension : public QObject
{
  Q_OBJECT

public:
  KonfiguratorExtension(QObject *obj, QString cfgClass, QString cfgName, bool rst = false );

  virtual void    loadInitialValue();
  virtual bool    apply();
  virtual void    setDefaults();
  virtual bool    isChanged();

  inline QObject *object()        {return objectPtr;}

  inline QString  getCfgClass()   {return configClass;}
  inline QString  getCfgName()    {return configName;}

public slots:
  void    setChanged()            {emit sigChanged( changed = true);}
  void    setChanged( bool chg )  {emit sigChanged( changed = chg);}

signals:
  void    applyManually(QObject *,QString, QString);
  void    applyAuto(QObject *,QString, QString);
  void    setDefaultsManually(QObject *);
  void    setDefaultsAuto(QObject *);
  void    setInitialValue(QObject *);
  void    sigChanged( bool );

protected:
  QObject *objectPtr;

  bool    applyConnected;
  bool    setDefaultsConnected;
  bool    changed;
  bool    restartNeeded;

  QString configClass;
  QString configName;

  virtual void connectNotify( const char *signal );
};


// KonfiguratorCheckBox class
///////////////////////////////

class KonfiguratorCheckBox : public QCheckBox
{
  Q_OBJECT

public:
  KonfiguratorCheckBox( QString cls, QString name, bool dflt, QString text,
                        QWidget *parent=0, const char *widgetName=0, bool rst=false );
  ~KonfiguratorCheckBox();

  inline KonfiguratorExtension *extension()   {return ext;}

public slots:
  virtual void loadInitialValue();
  void slotApply(QObject *,QString, QString);
  void slotSetDefaults(QObject *);

protected:
  bool  defaultValue;
  KonfiguratorExtension *ext;
};

// KonfiguratorSpinBox class
///////////////////////////////

class KonfiguratorSpinBox : public QSpinBox
{
  Q_OBJECT

public:
  KonfiguratorSpinBox( QString cls, QString name, int dflt, int min, int max,
                       QWidget *parent=0, const char *widgetName=0, bool rst=false );
  ~KonfiguratorSpinBox();

  inline KonfiguratorExtension *extension()   {return ext;}

public slots:
  virtual void loadInitialValue();
  void slotApply(QObject *,QString, QString);
  void slotSetDefaults(QObject *);

protected:
  int  defaultValue;
  KonfiguratorExtension *ext;
};

// KonfiguratorCheckBoxGroup class
///////////////////////////////

class KonfiguratorCheckBoxGroup : public QWidget
{
public:
  KonfiguratorCheckBoxGroup( QWidget * parent = 0, const char * name = 0 ) :
    QWidget( parent, name ) {};

  void                    add( KonfiguratorCheckBox * );
  KonfiguratorCheckBox *  find( int index );
  KonfiguratorCheckBox *  find( QString name );

private:
  QPtrList<KonfiguratorCheckBox>  checkBoxList;
};

// KonfiguratorRadioButtons class
///////////////////////////////

class KonfiguratorRadioButtons : public QButtonGroup
{
  Q_OBJECT

public:
  KonfiguratorRadioButtons( QString cls, QString name, QString dflt, QWidget *parent=0,
                            const char *widgetName=0, bool rst=false );
  ~KonfiguratorRadioButtons();

  inline KonfiguratorExtension *extension()   {return ext;}

  void  addRadioButton( QRadioButton *radioWidget, QString name, QString value );

  void            selectButton( QString value );

  QRadioButton*   find( int index );
  QRadioButton*   find( QString name );

public slots:
  virtual void loadInitialValue();
  void slotApply(QObject *,QString, QString);
  void slotSetDefaults(QObject *);

protected:
  QPtrList<QRadioButton>  radioButtons;
  QValueList<QString>   radioValues;
  QValueList<QString>   radioNames;

  QString         defaultValue;
  QButtonGroup    *buttonGroup;

  KonfiguratorExtension *ext;
};

// KonfiguratorEditBox class
///////////////////////////////

class KonfiguratorEditBox : public QLineEdit
{
  Q_OBJECT

public:
  KonfiguratorEditBox( QString cls, QString name, QString dflt, QWidget *parent=0,
                       const char *widgetName=0, bool rst=false );
  ~KonfiguratorEditBox();

  inline KonfiguratorExtension *extension()   {return ext;}

public slots:
  virtual void loadInitialValue();
  void slotApply(QObject *,QString, QString);
  void slotSetDefaults(QObject *);

protected:
  QString  defaultValue;
  KonfiguratorExtension *ext;
};


// KonfiguratorURLRequester class
///////////////////////////////

class KonfiguratorURLRequester : public KURLRequester
{
  Q_OBJECT

public:
  KonfiguratorURLRequester( QString cls, QString name, QString dflt, QWidget *parent=0,
                       const char *widgetName=0, bool rst=false );
  ~KonfiguratorURLRequester();

  inline KonfiguratorExtension *extension()   {return ext;}

public slots:
  virtual void loadInitialValue();
  void slotApply(QObject *,QString, QString);
  void slotSetDefaults(QObject *);

protected:
  QString  defaultValue;
  KonfiguratorExtension *ext;
};

// KonfiguratorFontChooser class
///////////////////////////////

class KonfiguratorFontChooser : public QHBox
{
  Q_OBJECT

public:
  KonfiguratorFontChooser( QString cls, QString name, QFont *dflt, QWidget *parent=0,
                            const char *widgetName=0, bool rst=false );
  ~KonfiguratorFontChooser();

  inline KonfiguratorExtension *extension()   {return ext;}

public slots:
  virtual void    loadInitialValue();
  void            slotApply(QObject *,QString, QString);
  void            slotSetDefaults(QObject *);
  void            slotBrowseFont();

protected:
  QFont         * defaultValue;
  QFont           font;
  KonfiguratorExtension *ext;

  QLabel        * pLabel;
  QToolButton   * pToolButton;

  void            setFont();
};

// KONFIGURATOR_NAME_VALUE_PAIR structure
///////////////////////////////

struct KONFIGURATOR_NAME_VALUE_PAIR
{
  QString text;
  QString value;
};

// KONFIGURATOR_NAME_VALUE_TIP structure
///////////////////////////////

struct KONFIGURATOR_NAME_VALUE_TIP
{
  QString text;
  QString value;
  QString tooltip;
};

// KonfiguratorComboBox class
///////////////////////////////

class KonfiguratorComboBox : public QComboBox
{
  Q_OBJECT

public:
  KonfiguratorComboBox( QString cls, QString name, QString dflt,
                        KONFIGURATOR_NAME_VALUE_PAIR *listIn, int listInLen,
                        QWidget *parent=0, const char *widgetName=0,
                        bool rst=false,  bool editable=false );
  ~KonfiguratorComboBox();

  inline KonfiguratorExtension *extension()   {return ext;}

public slots:
  virtual void loadInitialValue();
  void slotApply(QObject *,QString, QString);
  void slotSetDefaults(QObject *);

protected:
  QString                       defaultValue;
  KONFIGURATOR_NAME_VALUE_PAIR *list;
  int                           listLen;
  KonfiguratorExtension        *ext;

  void                          selectEntry( QString entry );
};


// KonfiguratorColorChooser class
///////////////////////////////

typedef struct
{
  QString name;
  QColor  color;
  QString value;
} ADDITIONAL_COLOR;

class KonfiguratorColorChooser : public QComboBox
{
  Q_OBJECT

public:
  KonfiguratorColorChooser( QString cls, QString name, QColor dflt,
                            QWidget *parent=0, const char *widgetName=0, bool rst=false,
                            ADDITIONAL_COLOR *addColPtr = 0, int addColNum = 0 );
  ~KonfiguratorColorChooser();

  inline KonfiguratorExtension *extension()   {return ext;}

  void          setDefaultColor( QColor dflt );
  void          setDefaultText( QString text );
  QColor        getColor();
  void          changeAdditionalColor( unsigned int num, QColor color );
  QString       getValue();
  void          setValue( QString value );

public slots:
  virtual void  loadInitialValue();
  void          slotApply(QObject *,QString, QString);
  void          slotSetDefaults(QObject *);
  void          slotCurrentChanged( int number );

signals:
  void          colorChanged();

private:
  void          addColor( QString text, QColor color );
  QPixmap       createPixmap( QColor color );

protected:
  QColor                          defaultValue;
  QColor                          customValue;
  QValueVector<QColor>            palette;
  QValueVector<ADDITIONAL_COLOR>  additionalColors;
  KonfiguratorExtension          *ext;
  bool                            disableColorChooser;
};

#endif /* __KONFIGURATOR_ITEMS_H__ */

