/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef PACKGUIBASE_H
#define PACKGUIBASE_H

// QtCore
#include <QMap>
// QtWidgets
#include <QDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

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
class KrHistoryComboBox;

class PackGUIBase : public QDialog
{
    Q_OBJECT

public:
    explicit PackGUIBase(QWidget *parent = nullptr);
    ~PackGUIBase() override;

    QLabel *TextLabel3;
    QLineEdit *nameData;
    QComboBox *typeData;
    QLabel *TextLabel5;
    QLineEdit *dirData;
    QToolButton *browseButton;
    QWidget *advancedWidget;
    QLabel *PixmapLabel1;
    QLabel *TextLabel1;
    QLabel *TextLabel4;
    QLabel *TextLabel6;
    QLabel *TextLabel7;
    QLabel *TextLabel8;
    QLabel *minLabel;
    QLabel *maxLabel;
    QLineEdit *password;
    QLineEdit *passwordAgain;
    QLabel *passwordConsistencyLabel;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QPushButton *advancedButton;
    QCheckBox *encryptHeaders;
    QCheckBox *multipleVolume;
    QSpinBox *volumeSpinBox;
    QComboBox *volumeUnitCombo;
    QCheckBox *setCompressionLevel;
    QSlider *compressionSlider;
    KrHistoryComboBox *commandLineSwitches;

public slots:
    virtual void browse();
    virtual bool extraProperties(QMap<QString, QString> &);

    void expand();
    void checkConsistency();

protected:
    QHBoxLayout *hbox;
    QHBoxLayout *hbox_2;
    QHBoxLayout *hbox_3;
    QHBoxLayout *hbox_4;
    QGridLayout *hbox_5;
    QHBoxLayout *hbox_6;
    QHBoxLayout *hbox_7;
    QGridLayout *grid;

private:
    bool expanded;
};

#endif // PACKGUIBASE_H
