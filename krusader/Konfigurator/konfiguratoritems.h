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
#include <qradiobutton.h>
#include <qptrlist.h>
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

// KonfiguratorRadioButtons class
///////////////////////////////

class KonfiguratorRadioButtons : public KonfiguratorExtension
{
  Q_OBJECT

public:
  KonfiguratorRadioButtons( QString cls, QString name, QString dflt, QButtonGroup *grp, bool rst=false );

  void  addRadioButton( QRadioButton *radioWidget, QString value );

  virtual void    loadInitialValue();
  virtual bool    apply();
  virtual void    setDefaults();

  QButtonGroup*   getGroupWidget() { return buttonGroup; }

  void            selectButton( QString value );  
  
  QPtrList<QRadioButton>  radioButtons;
  QValueVector<QString>   radioValues;  

protected:
  QString         defaultValue;
  QButtonGroup    *buttonGroup;
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


// KonfiguratorEditBox class
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

struct KONFIGURATOR_NAME_VALUE_PAIR
{
  QString text;
  QString value;
};

// KonfiguratorComboBox class
///////////////////////////////

class KonfiguratorComboBox : public QComboBox
{
  Q_OBJECT

public:
  KonfiguratorComboBox( QString cls, QString name, QString dflt,
                        KONFIGURATOR_NAME_VALUE_PAIR *listIn, int listInLen,
                        QWidget *parent=0, const char *widgetName=0, bool rst=false );
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
  int                           selected;
  KonfiguratorExtension        *ext;

  void                          selectEntry( QString entry );
};

#endif /* __KONFIGURATOR_ITEMS_H__ */

