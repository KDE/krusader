/*
    SPDX-FileCopyrightText: 2003 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPLITTERGUI_H
#define SPLITTERGUI_H

// QtCore
#include <QUrl>
// QtWidgets
#include <QDialog>

#include <KIO/Global>
#include <KUrlRequester>

class QComboBox;
class QCheckBox;
class QDoubleSpinBox;

class SplitterGUI : public QDialog
{
    Q_OBJECT
private:
    struct PredefinedDevice;

    static const QList<PredefinedDevice> &predefinedDevices();

    KIO::filesize_t userDefinedSize;
    int lastSelectedDevice;
    KIO::filesize_t division;

    QDoubleSpinBox *spinBox;
    QComboBox *deviceCombo;
    QComboBox *sizeCombo;
    QCheckBox *overwriteCb;
    KUrlRequester *urlReq;

public:
    SplitterGUI(QWidget *parent, const QUrl &fileURL, const QUrl &defaultDir);
    ~SplitterGUI() override;

    QUrl getDestinationDir()
    {
        return urlReq->url();
    }
    KIO::filesize_t getSplitSize();
    bool overWriteFiles();

public slots:
    virtual void sizeComboActivated(int item);
    virtual void predefinedComboActivated(int item);
    virtual void splitPressed();

protected:
    void keyPressEvent(QKeyEvent *e) override;
};

#endif /* __SPLITTERGUI_H__ */
