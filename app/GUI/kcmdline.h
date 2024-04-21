/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCMDLINE_H
#define KCMDLINE_H

// QtGui
#include <QKeyEvent>
// QtWidgets
#include <QLabel>
#include <QLayout>
#include <QToolButton>
#include <QWidget>

#include <KLineEdit>
#include <KShellCompletion>

#include "../GUI/krhistorycombobox.h"
#include "../UserAction/kractionbase.h"

class KCMDModeButton;

class CmdLineCombo : public KrHistoryComboBox
{
    Q_OBJECT
public:
    explicit CmdLineCombo(QWidget *parent);

    bool eventFilter(QObject *watched, QEvent *e) override;

    QString path()
    {
        return _path;
    }
    void setPath(QString path);

signals:
    void returnToPanel();

protected slots:
    void doLayout();

protected:
    void resizeEvent(QResizeEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

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
    ~KCMDLine() override;
    void setCurrent(const QString &path);
    // virtual methods from KrActionBase
    void setText(const QString &text);
    QString command() const override;
    ExecType execType() const override;
    QString startpath() const override;
    QString user() const override;
    QString text() const override;
    bool acceptURLs() const override;
    bool confirmExecution() const override;
    bool doSubstitution() const override;

signals:
    void signalRun();

public slots:
    void slotReturnFocus(); // returns keyboard focus to panel
    void slotRun();
    void addPlaceholder();
    void addText(const QString &text)
    {
        cmdLine->lineEdit()->setText(cmdLine->lineEdit()->text() + text);
    }
    void popup()
    {
        cmdLine->showPopup();
    }

protected:
    void focusInEvent(QFocusEvent *) override
    {
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
