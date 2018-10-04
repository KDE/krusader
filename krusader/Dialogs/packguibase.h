/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
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
#ifndef PACKGUIBASE_H
#define PACKGUIBASE_H

// QtCore
#include <QMap>
// QtWidgets
#include <QDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

class QHBoxLayout;
class QGridLayout;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;
class QSpinBox;
class QSlider;
class KHistoryComboBox;

class PackGUIBase : public QDialog
{
    Q_OBJECT

public:
    explicit PackGUIBase(QWidget* parent = nullptr);
    ~PackGUIBase();

    QLabel* TextLabel3;
    QLineEdit* nameData;
    QComboBox* typeData;
    QLabel* TextLabel5;
    QLineEdit* dirData;
    QToolButton* browseButton;
    QWidget* advancedWidget;
    QLabel* PixmapLabel1;
    QLabel* TextLabel1;
    QLabel* TextLabel4;
    QLabel* TextLabel6;
    QLabel* TextLabel7;
    QLabel* TextLabel8;
    QLabel* minLabel;
    QLabel* maxLabel;
    QLineEdit* password;
    QLineEdit* passwordAgain;
    QLabel* passwordConsistencyLabel;
    QPushButton* okButton;
    QPushButton* cancelButton;
    QPushButton* advancedButton;
    QCheckBox* encryptHeaders;
    QCheckBox* multipleVolume;
    QSpinBox* volumeSpinBox;
    QComboBox* volumeUnitCombo;
    QCheckBox* setCompressionLevel;
    QSlider*   compressionSlider;
    KHistoryComboBox *commandLineSwitches;

public slots:
    virtual void browse();
    virtual bool extraProperties(QMap<QString, QString> &);

    void expand();
    void checkConsistency();

protected:
    QHBoxLayout* hbox;
    QHBoxLayout* hbox_2;
    QHBoxLayout* hbox_3;
    QHBoxLayout* hbox_4;
    QGridLayout* hbox_5;
    QHBoxLayout* hbox_6;
    QHBoxLayout* hbox_7;
    QGridLayout* grid;

private:
    bool expanded;
};

#endif // PACKGUIBASE_H
