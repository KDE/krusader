/*
    SPDX-FileCopyrightText: 2004-2007 Jonas Bähr <jonas.baehr@web.de>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "actionproperty.h"
#include "addplaceholderpopup.h"

#include "../UserAction/kraction.h"
#include "../UserAction/useraction.h"
#include "../icon.h"
#include "../krglobal.h"
#include "../krusader.h"

// QtWidgets
#include <QFileDialog>
#include <QInputDialog>

#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>

ActionProperty::ActionProperty(QWidget *parent, KrAction *action)
    : QWidget(parent)
    , _modified(false)
{
    setupUi(this);

    if (action) {
        _action = action;
        updateGUI(_action);
    }

    ButtonAddPlaceholder->setIcon(Icon("list-add"));
    ButtonAddStartpath->setIcon(Icon("document-open"));

    // fill with all existing categories
    cbCategory->addItems(krUserAction->allCategories());

    connect(ButtonAddPlaceholder, &QToolButton::clicked, this, &ActionProperty::addPlaceholder);
    connect(ButtonAddStartpath, &QToolButton::clicked, this, &ActionProperty::addStartpath);
    connect(ButtonNewProtocol, &QToolButton::clicked, this, &ActionProperty::newProtocol);
    connect(ButtonEditProtocol, &QToolButton::clicked, this, &ActionProperty::editProtocol);
    connect(ButtonRemoveProtocol, &QToolButton::clicked, this, &ActionProperty::removeProtocol);
    connect(ButtonAddPath, &QToolButton::clicked, this, &ActionProperty::addPath);
    connect(ButtonEditPath, &QToolButton::clicked, this, &ActionProperty::editPath);
    connect(ButtonRemovePath, &QToolButton::clicked, this, &ActionProperty::removePath);
    connect(ButtonAddMime, &QToolButton::clicked, this, &ActionProperty::addMime);
    connect(ButtonEditMime, &QToolButton::clicked, this, &ActionProperty::editMime);
    connect(ButtonRemoveMime, &QToolButton::clicked, this, &ActionProperty::removeMime);
    connect(ButtonNewFile, &QToolButton::clicked, this, &ActionProperty::newFile);
    connect(ButtonEditFile, &QToolButton::clicked, this, &ActionProperty::editFile);
    connect(ButtonRemoveFile, &QToolButton::clicked, this, &ActionProperty::removeFile);
    connect(KeyButtonShortcut, &KKeySequenceWidget::keySequenceChanged, this, &ActionProperty::changedShortcut);
    // track modifications:
    connect(leDistinctName, &KLineEdit::textChanged, this, [=]() {
        setModified(true);
    });
    connect(leTitle, &KLineEdit::textChanged, this, [=]() {
        setModified(true);
    });
    connect(ButtonIcon, &KIconButton::iconChanged, this, [=]() {
        setModified(true);
    });
    connect(cbCategory, &KComboBox::currentTextChanged, this, [=]() {
        setModified(true);
    });
    connect(leTooltip, &KLineEdit::textChanged, this, [=]() {
        setModified(true);
    });
    connect(textDescription, &KTextEdit::textChanged, this, [=]() {
        setModified(true);
    });
    connect(leCommandline, &KLineEdit::textChanged, this, [=]() {
        setModified(true);
    });
    connect(leStartpath, &KLineEdit::textChanged, this, [=]() {
        setModified(true);
    });
    connect(chkSeparateStdError, &QCheckBox::clicked, this, &ActionProperty::setModified);
    connect(radioCollectOutput, &QRadioButton::clicked, this, &ActionProperty::setModified);
    connect(radioNormal, &QRadioButton::clicked, this, &ActionProperty::setModified);
    connect(radioTE, &QRadioButton::clicked, this, &ActionProperty::setModified);
    connect(radioTerminal, &QRadioButton::clicked, this, &ActionProperty::setModified);
    connect(radioLocal, &QRadioButton::clicked, this, &ActionProperty::setModified);
    connect(radioUrl, &QRadioButton::clicked, this, &ActionProperty::setModified);
    connect(KeyButtonShortcut, &KKeySequenceWidget::keySequenceChanged, this, [=]() {
        setModified(true);
    });
    connect(chkEnabled, &QCheckBox::clicked, this, &ActionProperty::setModified);
    connect(leDifferentUser, &KLineEdit::textChanged, this, [=]() {
        setModified(true);
    });
    connect(chkDifferentUser, &QCheckBox::clicked, this, &ActionProperty::setModified);
    connect(chkConfirmExecution, &QCheckBox::clicked, this, &ActionProperty::setModified);
    connect(chkSeparateStdError, &QCheckBox::clicked, this, &ActionProperty::setModified);
    // The modified-state of the ShowOnly-lists is tracked in the access-functions below
}

ActionProperty::~ActionProperty() = default;

void ActionProperty::changedShortcut(const QKeySequence &shortcut)
{
    KeyButtonShortcut->setKeySequence(shortcut);
}

void ActionProperty::clear()
{
    _action = nullptr;

    // This prevents the changed-signal from being emitted during the GUI-update
    _modified = true; // The real state is set at the end of this function.

    leDistinctName->clear();
    cbCategory->clearEditText();
    leTitle->clear();
    leTooltip->clear();
    textDescription->clear();
    leCommandline->clear();
    leStartpath->clear();
    KeyButtonShortcut->clearKeySequence();

    lbShowonlyProtocol->clear();
    lbShowonlyPath->clear();
    lbShowonlyMime->clear();
    lbShowonlyFile->clear();

    chkSeparateStdError->setChecked(false);
    radioNormal->setChecked(true);

    radioLocal->setChecked(true);

    chkEnabled->setChecked(true);

    chkConfirmExecution->setChecked(false);

    ButtonIcon->resetIcon();

    leDifferentUser->clear();
    chkDifferentUser->setChecked(false);

    setModified(false);
}

void ActionProperty::updateGUI(KrAction *action)
{
    if (action)
        _action = action;
    if (!_action)
        return;

    // This prevents the changed-signal from being emitted during the GUI-update.
    _modified = true; // The real state is set at the end of this function.

    leDistinctName->setText(_action->objectName());
    cbCategory->lineEdit()->setText(_action->category());
    leTitle->setText(_action->text());
    leTooltip->setText(_action->toolTip());
    textDescription->setText(_action->whatsThis());
    leCommandline->setText(_action->command());
    leCommandline->home(false);
    leStartpath->setText(_action->startpath());
    KeyButtonShortcut->setKeySequence(_action->shortcut());

    lbShowonlyProtocol->clear();
    lbShowonlyProtocol->addItems(_action->showonlyProtocol());
    lbShowonlyPath->clear();
    lbShowonlyPath->addItems(_action->showonlyPath());
    lbShowonlyMime->clear();
    lbShowonlyMime->addItems(_action->showonlyMime());
    lbShowonlyFile->clear();
    lbShowonlyFile->addItems(_action->showonlyFile());

    chkSeparateStdError->setChecked(false);
    switch (_action->execType()) {
    case KrAction::CollectOutputSeparateStderr:
        chkSeparateStdError->setChecked(true);
        radioCollectOutput->setChecked(true);
        break;
    case KrAction::CollectOutput:
        radioCollectOutput->setChecked(true);
        break;
    case KrAction::Terminal:
        radioTerminal->setChecked(true);
        break;
    case KrAction::RunInTE:
        radioTE->setChecked(true);
        break;
    default: // case KrAction::Normal:
        radioNormal->setChecked(true);
        break;
    }

    if (_action->acceptURLs())
        radioUrl->setChecked(true);
    else
        radioLocal->setChecked(true);

    chkEnabled->setChecked(_action->isVisible());

    chkConfirmExecution->setChecked(_action->confirmExecution());

    if (!_action->icon().isNull())
        ButtonIcon->setIcon(_action->iconName());
    else
        ButtonIcon->resetIcon();

    leDifferentUser->setText(_action->user());
    if (_action->user().isEmpty())
        chkDifferentUser->setChecked(false);
    else
        chkDifferentUser->setChecked(true);

    setModified(false);
}

void ActionProperty::updateAction(KrAction *action)
{
    if (action)
        _action = action;
    if (!_action)
        return;

    if (_action->category() != cbCategory->currentText()) {
        _action->setCategory(cbCategory->currentText());
        // Update the category-list
        cbCategory->clear();
        cbCategory->addItems(krUserAction->allCategories());
        cbCategory->lineEdit()->setText(_action->category());
    }

    _action->setObjectName(leDistinctName->text());
    _action->setText(leTitle->text());
    _action->setToolTip(leTooltip->text());
    _action->setWhatsThis(textDescription->toPlainText());
    _action->setCommand(leCommandline->text());
    _action->setStartpath(leStartpath->text());
    _action->setShortcut(KeyButtonShortcut->keySequence());

    QStringList list;

    for (int i1 = 0; i1 != lbShowonlyProtocol->count(); i1++) {
        QListWidgetItem *lbi = lbShowonlyProtocol->item(i1);

        list << lbi->text();
    }
    _action->setShowonlyProtocol(list);

    list = QStringList();
    for (int i1 = 0; i1 != lbShowonlyPath->count(); i1++) {
        QListWidgetItem *lbi = lbShowonlyPath->item(i1);

        list << lbi->text();
    }
    _action->setShowonlyPath(list);

    list = QStringList();
    for (int i1 = 0; i1 != lbShowonlyMime->count(); i1++) {
        QListWidgetItem *lbi = lbShowonlyMime->item(i1);

        list << lbi->text();
    }
    _action->setShowonlyMime(list);

    list = QStringList();
    for (int i1 = 0; i1 != lbShowonlyFile->count(); i1++) {
        QListWidgetItem *lbi = lbShowonlyFile->item(i1);

        list << lbi->text();
    }
    _action->setShowonlyFile(list);

    if (radioCollectOutput->isChecked() && chkSeparateStdError->isChecked())
        _action->setExecType(KrAction::CollectOutputSeparateStderr);
    else if (radioCollectOutput->isChecked() && !chkSeparateStdError->isChecked())
        _action->setExecType(KrAction::CollectOutput);
    else if (radioTerminal->isChecked())
        _action->setExecType(KrAction::Terminal);
    else if (radioTE->isChecked())
        _action->setExecType(KrAction::RunInTE);
    else
        _action->setExecType(KrAction::Normal);

    if (radioUrl->isChecked())
        _action->setAcceptURLs(true);
    else
        _action->setAcceptURLs(false);

    _action->setEnabled(chkEnabled->isChecked());
    _action->setVisible(chkEnabled->isChecked());

    _action->setConfirmExecution(chkConfirmExecution->isChecked());

    _action->setIcon(Icon(ButtonIcon->icon()));
    _action->setIconName(ButtonIcon->icon());

    _action->setUser(leDifferentUser->text());

    setModified(false);
}

void ActionProperty::addPlaceholder()
{
    AddPlaceholderPopup popup(this);
    QString exp = popup.getPlaceholder(mapToGlobal(QPoint(ButtonAddPlaceholder->pos().x() + ButtonAddPlaceholder->width() + 6, // 6 is the default margin
                                                          ButtonAddPlaceholder->pos().y())));
    leCommandline->insert(exp);
}

void ActionProperty::addStartpath()
{
    QString folder = QFileDialog::getExistingDirectory(this);
    if (!folder.isEmpty()) {
        leStartpath->setText(folder);
    }
}

void ActionProperty::newProtocol()
{
    bool ok;

    QString currentText;
    if (lbShowonlyProtocol->currentItem())
        currentText = lbShowonlyProtocol->currentItem()->text();

    QString text = QInputDialog::getText(this, i18n("New protocol"), i18n("Set a protocol:"), QLineEdit::Normal, currentText, &ok);
    if (ok && !text.isEmpty()) {
        lbShowonlyProtocol->addItems(text.split(';'));
        setModified();
    }
}

void ActionProperty::editProtocol()
{
    if (lbShowonlyProtocol->currentItem() == nullptr)
        return;

    bool ok;

    QString currentText = lbShowonlyProtocol->currentItem()->text();

    QString text = QInputDialog::getText(this, i18n("Edit Protocol"), i18n("Set another protocol:"), QLineEdit::Normal, currentText, &ok);
    if (ok && !text.isEmpty()) {
        lbShowonlyProtocol->currentItem()->setText(text);
        setModified();
    }
}

void ActionProperty::removeProtocol()
{
    if (lbShowonlyProtocol->currentItem() != nullptr) {
        delete lbShowonlyProtocol->currentItem();
        setModified();
    }
}

void ActionProperty::addPath()
{
    QString folder = QFileDialog::getExistingDirectory(this);
    if (!folder.isEmpty()) {
        lbShowonlyPath->addItem(folder);
        setModified();
    }
}

void ActionProperty::editPath()
{
    if (lbShowonlyPath->currentItem() == nullptr)
        return;

    bool ok;

    QString currentText = lbShowonlyPath->currentItem()->text();

    QString text = QInputDialog::getText(this, i18n("Edit Path"), i18n("Set another path:"), QLineEdit::Normal, currentText, &ok);
    if (ok && !text.isEmpty()) {
        lbShowonlyPath->currentItem()->setText(text);
        setModified();
    }
}

void ActionProperty::removePath()
{
    if (lbShowonlyPath->currentItem() != nullptr) {
        delete lbShowonlyPath->currentItem();
        setModified();
    }
}

void ActionProperty::addMime()
{
    bool ok;

    QString currentText;
    if (lbShowonlyMime->currentItem())
        currentText = lbShowonlyMime->currentItem()->text();

    QString text = QInputDialog::getText(this, i18n("New MIME Type"), i18n("Set a MIME type:"), QLineEdit::Normal, currentText, &ok);
    if (ok && !text.isEmpty()) {
        lbShowonlyMime->addItems(text.split(';'));
        setModified();
    }
}

void ActionProperty::editMime()
{
    if (lbShowonlyMime->currentItem() == nullptr)
        return;

    bool ok;

    QString currentText = lbShowonlyMime->currentItem()->text();

    QString text = QInputDialog::getText(this, i18n("Edit MIME Type"), i18n("Set another MIME type:"), QLineEdit::Normal, currentText, &ok);
    if (ok && !text.isEmpty()) {
        lbShowonlyMime->currentItem()->setText(text);
        setModified();
    }
}

void ActionProperty::removeMime()
{
    if (lbShowonlyMime->currentItem() != nullptr) {
        delete lbShowonlyMime->currentItem();
        setModified();
    }
}

void ActionProperty::newFile()
{
    bool ok;

    QString currentText;
    if (lbShowonlyFile->currentItem())
        currentText = lbShowonlyFile->currentItem()->text();

    QString text = QInputDialog::getText(this, i18n("New File Name"), i18n("Set a file name:"), QLineEdit::Normal, currentText, &ok);
    if (ok && !text.isEmpty()) {
        lbShowonlyFile->addItems(text.split(';'));
        setModified();
    }
}

void ActionProperty::editFile()
{
    if (lbShowonlyFile->currentItem() == nullptr)
        return;

    bool ok;

    QString currentText = lbShowonlyFile->currentItem()->text();

    QString text = QInputDialog::getText(this, i18n("Edit File Name"), i18n("Set another file name:"), QLineEdit::Normal, currentText, &ok);
    if (ok && !text.isEmpty()) {
        lbShowonlyFile->currentItem()->setText(text);
        setModified();
    }
}

void ActionProperty::removeFile()
{
    if (lbShowonlyFile->currentItem() != nullptr) {
        delete lbShowonlyFile->currentItem();
        setModified();
    }
}

bool ActionProperty::validProperties()
{
    if (leDistinctName->text().simplified().isEmpty()) {
        KMessageBox::error(this, i18n("Please set a unique name for the useraction"));
        leDistinctName->setFocus();
        return false;
    }
    if (leTitle->text().simplified().isEmpty()) {
        KMessageBox::error(this, i18n("Please set a title for the menu entry"));
        leTitle->setFocus();
        return false;
    }
    if (leCommandline->text().simplified().isEmpty()) {
        KMessageBox::error(this, i18n("Command line is empty"));
        leCommandline->setFocus();
        return false;
    }
    if (leDistinctName->isEnabled())
        if (krApp->actionCollection()->action(leDistinctName->text())) {
            KMessageBox::error(this,
                               i18n("There already is an action with this name.\n"
                                    "If you do not have such a useraction the name is used by Krusader for an internal action."));
            leDistinctName->setFocus();
            return false;
        }

    return true;
}

void ActionProperty::setModified(bool m)
{
    if (m && !_modified) { // emit only when the state _changes_to_true_,
        emit changed();
    }
    _modified = m;
}
