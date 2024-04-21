/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009 Fathi Boudra <fboudra@gmail.com>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NEWFTPGUI_H
#define NEWFTPGUI_H

// QtWidgets
#include <QDialog>
#include <QLabel>
#include <QSpinBox>

#include <KComboBox>
#include <KLineEdit>

#include "../GUI/krhistorycombobox.h"

/**
 * The "New Network Connection" dialog
 */
class newFTPGUI : public QDialog
{
    Q_OBJECT

public:
    explicit newFTPGUI(QWidget *parent = nullptr);
    ~newFTPGUI() override;

    KComboBox *prefix;
    KrHistoryComboBox *url;
    QSpinBox *port;
    KLineEdit *username;
    KLineEdit *password;

protected:
    bool event(QEvent *) override;

private slots:
    void slotTextChanged(const QString &);

private:
    QLabel *iconLabel;
    QLabel *aboutLabel;
    QLabel *protocolLabel;
    QLabel *passwordLabel;
    QLabel *hostLabel;
    QLabel *usernameLabel;
    QLabel *portLabel;
};

#endif
