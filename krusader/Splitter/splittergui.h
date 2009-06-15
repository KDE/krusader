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

#ifndef SPLITTERGUI_H
#define SPLITTERGUI_H

#include <QtGui/QDialog>
#include <QtCore/QString>
#include <QDoubleSpinBox>
#include <QtGui/QValidator>
#include <QtGui/QComboBox>
#include <QKeyEvent>
#include <QtGui/QLineEdit>

#include <kurlrequester.h>
#include <kio/global.h>

#include "../VFS/vfs.h"

class SplitterSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

private:
    KIO::filesize_t m_division;
    mutable KIO::filesize_t m_longValue;

public:
    SplitterSpinBox(QWidget * parent = 0) : QDoubleSpinBox(parent), m_division(1), m_longValue(1) {
        setMinimum(1);
        setMaximum(9999999999.0);
    }

    void setLongValue(KIO::filesize_t valueIn) {
        m_longValue = valueIn;
        if (m_longValue == 0)
            m_longValue++;
        setValue((double)m_longValue / m_division);
    }

    KIO::filesize_t longValue() const {
        return m_longValue;
    }

    double valueFromText(const QString& text) const {
        double value = QDoubleSpinBox::valueFromText(text);
        m_longValue = (KIO::filesize_t)(value * m_division);
        return value;
    }

    void setDivision(KIO::filesize_t div) {
        m_division = div;
        setValue((double)m_longValue / m_division);
    }
};

struct PredefinedDevice {
    QString name;
    KIO::filesize_t capacity;
};

class SplitterGUI : QDialog
{
    Q_OBJECT

private:
    int                             predefinedDeviceNum;
    KIO::filesize_t                 userDefinedSize;
    int                             lastSelectedDevice;
    int                             resultCode;

    static PredefinedDevice predefinedDevices[];

    SplitterSpinBox *spinBox;
    QComboBox       *deviceCombo;
    QComboBox       *sizeCombo;
    KUrlRequester   *urlReq;

public:
    SplitterGUI(QWidget* parent,  KUrl fileURL, KUrl defaultDir);

    KUrl    getDestinationDir()     {
        return KUrl(urlReq->url().prettyUrl()); /* TODO: is prettyUrl what we need? */
    }
    KIO::filesize_t getSplitSize()  {
        return spinBox->longValue();
    }
    int     result()                {
        return resultCode;
    }

public slots:
    virtual void sizeComboActivated(int item);
    virtual void predefinedComboActivated(int item);
    virtual void splitPressed();

protected:
    virtual void keyPressEvent(QKeyEvent *e);

public:
};

#endif /* __SPLITTERGUI_H__ */
