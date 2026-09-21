/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Jonas Bähr <jonas.baehr@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACTIONMAN_H
#define ACTIONMAN_H

// QtWidgets
#include <QDialog>

class UserActionPage;

/**
 * This manages all useractions
 */
class ActionMan : public QDialog
{
    Q_OBJECT
public:
    explicit ActionMan(QWidget *parent = nullptr);
    ~ActionMan() override;

protected slots:
    void slotClose();
    void slotApply();
    void slotEnableApplyButton();
    void slotDisableApplyButton();

private:
    UserActionPage *userActionPage;
    QPushButton *applyButton;
};

#endif // ifndef ACTIONMAN_H
