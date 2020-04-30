/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2019 Krusader Krew [https://krusader.org]              *
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
