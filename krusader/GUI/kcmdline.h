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


#ifndef KCMDLINE_H
#define KCMDLINE_H

// QtGui
#include <QKeyEvent>
// QtWidgets
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QToolButton>

#include <KCompletion/KLineEdit>
#include <KIOWidgets/KShellCompletion>
#include <KCompletion/KHistoryComboBox>

#include "../UserAction/kractionbase.h"

class KCMDModeButton;

class CmdLineCombo : public KHistoryComboBox
{
    Q_OBJECT
public:
    explicit CmdLineCombo(QWidget *parent);

    virtual bool eventFilter(QObject *watched, QEvent *e) Q_DECL_OVERRIDE;

    QString path() {
        return _path;
    }
    void setPath(QString path);

signals:
    void returnToPanel();

protected slots:
    void doLayout();

protected:
    virtual void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    virtual void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

    void updateLineEditGeometry();

    QLabel *_pathLabel;
    QString _path;
    bool _handlingLineEditResize;
};


class KCMDLine : public QWidget, KrActionBase
{
    Q_OBJECT
public:
    explicit KCMDLine(QWidget *parent = nullptr);
    ~KCMDLine();
    void setCurrent(const QString &path);
    //virtual methods from KrActionBase
    void setText(QString text);
    QString command() const Q_DECL_OVERRIDE;
    ExecType execType() const Q_DECL_OVERRIDE;
    QString startpath() const Q_DECL_OVERRIDE;
    QString user() const Q_DECL_OVERRIDE;
    QString text() const Q_DECL_OVERRIDE;
    bool acceptURLs() const Q_DECL_OVERRIDE;
    bool confirmExecution() const Q_DECL_OVERRIDE;
    bool doSubstitution() const Q_DECL_OVERRIDE;

signals:
    void signalRun();

public slots:
    void slotReturnFocus(); // returns keyboard focus to panel
    void slotRun();
    void addPlaceholder();
    void addText(QString text) {
        cmdLine->lineEdit()->setText(cmdLine->lineEdit()->text() + text);
    }
    void popup() {
        cmdLine->showPopup();
    }

protected:
    virtual void focusInEvent(QFocusEvent*) Q_DECL_OVERRIDE {
        cmdLine->setFocus();
    }

    void calcLabelSize();

private:
    CmdLineCombo *cmdLine;
    KCMDModeButton *terminal;
    QToolButton *buttonAddPlaceholder;
    KShellCompletion completion;
};

#endif
