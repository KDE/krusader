/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcmdline.h"

// QtCore
#include <QDir>
#include <QStringList>
#include <QUrl>
// QtGui
#include <QFontDatabase>
#include <QFontMetrics>
#include <QIcon>
#include <QImage>
#include <QKeyEvent>
// QtWidgets
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QSizePolicy>

#include <KLocalizedString>
#include <KSharedConfig>
#include <utility>

#include "../ActionMan/addplaceholderpopup.h"
#include "../compat.h"
#include "../defaults.h"
#include "../icon.h"
#include "../krglobal.h"
#include "../krservices.h"
#include "../krslots.h"
#include "../krusaderview.h"
#include "kcmdmodebutton.h"

CmdLineCombo::CmdLineCombo(QWidget *parent)
    : KrHistoryComboBox(parent)
    , _handlingLineEditResize(false)
{
    lineEdit()->installEventFilter(this);
    _pathLabel = new QLabel(this);
    _pathLabel->setWhatsThis(i18n("Name of folder where command will be processed."));
    _pathLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
}

bool CmdLineCombo::eventFilter(QObject *watched, QEvent *e)
{
    if (watched == lineEdit() && (e->type() == QEvent::Move || e->type() == QEvent::Resize)) {
        if (!_handlingLineEditResize) { // avoid infinite recursion
            _handlingLineEditResize = true;
            updateLineEditGeometry();
            _handlingLineEditResize = false;
        }
    }
    return false;
}

void CmdLineCombo::setPath(QString path)
{
    _path = std::move(path);
    doLayout();
}

void CmdLineCombo::updateLineEditGeometry()
{
    QRect r = lineEdit()->geometry();
    r.setLeft(_pathLabel->geometry().right());
    lineEdit()->setGeometry(r);
}

void CmdLineCombo::doLayout()
{
    QString pathNameLabel = _path;
    QFontMetrics fm(_pathLabel->fontMetrics());
    int textWidth = fm.horizontalAdvance(_path);
    int maxWidth = (width() + _pathLabel->width()) * 2 / 5;
    qsizetype letters = _path.length() / 2;

    while (letters && textWidth > maxWidth) {
        pathNameLabel = _path.left(letters) + "..." + _path.right(letters);
        letters--;
        textWidth = fm.horizontalAdvance(pathNameLabel);
    }

    _pathLabel->setText(pathNameLabel + "> ");
    _pathLabel->adjustSize();

    QStyleOptionComboBox opt;
    initStyleOption(&opt);
    QRect labelRect = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField, this);
    labelRect.adjust(2, 0, 0, 0);
    labelRect.setWidth(_pathLabel->width());
    _pathLabel->setGeometry(labelRect);

    updateLineEditGeometry();
}

void CmdLineCombo::resizeEvent(QResizeEvent *e)
{
    KrHistoryComboBox::resizeEvent(e);
    doLayout();
}

void CmdLineCombo::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (e->modifiers() & Qt::ControlModifier) {
            SLOTS->insertFileName((e->modifiers() & Qt::ShiftModifier) != 0);
            break;
        }
        KrHistoryComboBox::keyPressEvent(e);
        break;
    case Qt::Key_Down:
        if (e->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {
            MAIN_VIEW->focusTerminalEmulator();
            return;
        } else
            KrHistoryComboBox::keyPressEvent(e);
        break;
    case Qt::Key_Up:
        if (e->modifiers() == Qt::ControlModifier || e->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {
            emit returnToPanel();
            return;
        } else
            KrHistoryComboBox::keyPressEvent(e);
        break;
    case Qt::Key_Escape:
        if (e->modifiers() == 0) {
            emit returnToPanel();
            return;
        } else
            KrHistoryComboBox::keyPressEvent(e);
        break;
    default:
        KrHistoryComboBox::keyPressEvent(e);
    }
}

KCMDLine::KCMDLine(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QGridLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    int height = QFontMetrics(QFontDatabase::systemFont(QFontDatabase::GeneralFont)).height();
    height = height + 5 * (height > 14) + 6;

    // and editable command line
    completion.setMode(KUrlCompletion::FileCompletion);
    cmdLine = new CmdLineCombo(this);
    cmdLine->setMaxCount(100); // remember 100 commands
    cmdLine->setMinimumContentsLength(10);
    cmdLine->setDuplicatesEnabled(false);
    cmdLine->setMaximumHeight(height);
    cmdLine->setCompletionObject(&completion);
    cmdLine->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    // load the history
    KConfigGroup grpSvr(krConfig, "Private");
    QStringList list = grpSvr.readEntry("cmdline history", QStringList());
    cmdLine->setHistoryItems(list);

    connect(cmdLine, QOverload<const QString &>::of(&CmdLineCombo::returnPressed), this, &KCMDLine::slotRun);
    connect(cmdLine, QOverload<const QString &>::of(&CmdLineCombo::returnPressed), cmdLine->lineEdit(), &QLineEdit::clear);
    connect(cmdLine, &CmdLineCombo::returnToPanel, this, &KCMDLine::slotReturnFocus);

    cmdLine->setWhatsThis(
        i18n("<qt><p>Well, it is actually quite simple: you type your command here and Krusader obeys.</p><p><b>Tip</b>: move within command line history with "
             "&lt;Up&gt; and &lt;Down&gt; arrows.</p></qt>"));
    layout->addWidget(cmdLine, 0, 1);

    buttonAddPlaceholder = new QToolButton(this);
    buttonAddPlaceholder->setAutoRaise(true);
    buttonAddPlaceholder->setIcon(Icon("list-add"));
    connect(buttonAddPlaceholder, &QToolButton::clicked, this, &KCMDLine::addPlaceholder);
    buttonAddPlaceholder->setWhatsThis(i18n("Add <b>Placeholders</b> for the selected files in the panel."));

    layout->addWidget(buttonAddPlaceholder, 0, 2);

    // a run in terminal button
    terminal = new KCMDModeButton(this);
    terminal->setAutoRaise(true);
    layout->addWidget(terminal, 0, 3);

    layout->activate();
}

KCMDLine::~KCMDLine()
{
    KConfigGroup grpSvr(krConfig, "Private");
    QStringList list = cmdLine->historyItems();
    // qWarning() << list[0];
    grpSvr.writeEntry("cmdline history", list);
    krConfig->sync();
}

void KCMDLine::addPlaceholder()
{
    AddPlaceholderPopup popup(this);
    QString exp = popup.getPlaceholder(buttonAddPlaceholder->mapToGlobal(QPoint(0, 0)));
    this->addText(exp);
}

void KCMDLine::setCurrent(const QString &path)
{
    cmdLine->setPath(path);

    completion.setDir(QUrl::fromLocalFile(path));
    // make sure our command is executed in the right directory
    // This line is important for Krusader overall functions -> do not remove !
    QDir::setCurrent(path);
}

void KCMDLine::slotRun()
{
    const QString command1(cmdLine->currentText());
    if (command1.isEmpty())
        return;

    cmdLine->addToHistory(command1);
    // bugfix by aardvark: current editline is destroyed by addToHistory() in some cases
    cmdLine->setEditText(command1);

    if (command1.simplified().left(3) == "cd ") { // cd command effect the active panel
        QString dir = command1.right(command1.length() - command1.indexOf(" ")).trimmed();
        if (dir == "~")
            dir = QDir::homePath();
        else if (dir.left(1) != "/" && !dir.contains(":/"))
            dir = cmdLine->path() + (cmdLine->path() == "/" ? "" : "/") + dir;
        SLOTS->refresh(QUrl::fromUserInput(dir, QDir::currentPath(), QUrl::AssumeLocalFile));
    } else {
        exec();
        cmdLine->clearEditText();
    }
}

void KCMDLine::slotReturnFocus()
{
    MAIN_VIEW->cmdLineUnFocus();
}

static const KrActionBase::ExecType execModesMenu[] = {
    KrActionBase::Normal,
    KrActionBase::CollectOutputSeparateStderr,
    KrActionBase::CollectOutput,
    KrActionBase::Terminal,
    KrActionBase::RunInTE,
};

QString KCMDLine::command() const
{
    return cmdLine->currentText();
}

KrActionBase::ExecType KCMDLine::execType() const
{
    KConfigGroup grp(krConfig, "Private");
    int i = grp.readEntry("Command Execution Mode", (int)0);
    return execModesMenu[i];
}

QString KCMDLine::startpath() const
{
    return cmdLine->path();
    //     return path->text().left(path->text().length() - 1);
}

QString KCMDLine::user() const
{
    return QString();
}

QString KCMDLine::text() const
{
    return cmdLine->currentText();
}

bool KCMDLine::acceptURLs() const
{
    return false;
}

bool KCMDLine::confirmExecution() const
{
    return false;
}

bool KCMDLine::doSubstitution() const
{
    return true;
}

void KCMDLine::setText(const QString &text)
{
    cmdLine->lineEdit()->setText(text);
}
