/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2020 Krusader Krew [https://krusader.org]

    This file is part of Krusader [https://krusader.org].

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KRSPWIDGETS_H
#define KRSPWIDGETS_H

// QtCore
#include <QEvent>
// QtGui
#include <QMouseEvent>

#include <KCompletion/KLineEdit>

#include "krmaskchoice.h"
#include "newftpgui.h"
#include "../FileSystem/krquery.h"

class KrMaskChoiceSub;

class KrSpWidgets
{
    friend class KrMaskChoiceSub;

public:
    KrSpWidgets();

    static KrQuery getMask(const QString& caption, bool nameOnly = false, QWidget * parent = 0); // get file-mask for (un)selecting files
    static QUrl newFTP();

private:
    static QStringList maskList;  // used by KrMaskChoiceSub
};

/////////////////////////// newFTPSub ///////////////////////////////////////
class newFTPSub : public newFTPGUI
{
public:
    newFTPSub();
    ~newFTPSub() override;

protected:
    void reject() override;
    void accept() override;
};

/////////////////////////// KrMaskChoiceSub /////////////////////////////////
// Inherits KrMaskChoice's generated code to fully implement the functions //
/////////////////////////////////////////////////////////////////////////////
class KrMaskChoiceSub : public KrMaskChoice
{
public:
    explicit KrMaskChoiceSub(QWidget * parent = 0);

public slots:
    void addSelection() override;
    void deleteSelection() override;
    void clearSelections() override;
    void acceptFromList(QListWidgetItem *i) override;
    void currentItemChanged(QListWidgetItem *i) override;

protected:
    void reject() override;
    void accept() override;
};

#endif
