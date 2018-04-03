/*****************************************************************************
 * Copyright (C) 2003 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#ifndef SPLITTERGUI_H
#define SPLITTERGUI_H

// QtCore
#include <QUrl>
// QtWidgets
#include <QDialog>

#include <KIOWidgets/KUrlRequester>
#include <KIO/Global>

class QComboBox;
class QCheckBox;
class QDoubleSpinBox;


class SplitterGUI : public QDialog
{
    Q_OBJECT
private:
    struct PredefinedDevice;

    static const QList<PredefinedDevice> &predefinedDevices();

    KIO::filesize_t                 userDefinedSize;
    int                             lastSelectedDevice;
    KIO::filesize_t                 division;

    QDoubleSpinBox  *spinBox;
    QComboBox       *deviceCombo;
    QComboBox       *sizeCombo;
    QCheckBox       *overwriteCb;
    KUrlRequester   *urlReq;

public:
    SplitterGUI(QWidget* parent,  QUrl fileURL, QUrl defaultDir);
    ~SplitterGUI();

    QUrl    getDestinationDir()     {
        return urlReq->url();
    }
    KIO::filesize_t getSplitSize();
    bool overWriteFiles();

public slots:
    virtual void sizeComboActivated(int item);
    virtual void predefinedComboActivated(int item);
    virtual void splitPressed();

protected:
    virtual void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
};

#endif /* __SPLITTERGUI_H__ */
