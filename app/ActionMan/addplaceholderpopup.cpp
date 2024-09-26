/*
    SPDX-FileCopyrightText: 2004 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004 Jonas Bähr <jonas.baehr@web.de>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "addplaceholderpopup.h"

// for ParameterDialog
#include "../BookMan/krbookmarkbutton.h"
#include "../GUI/profilemanager.h"
#include "../icon.h"
#include "../krglobal.h" // for konfig-access

// QtWidgets
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>

#include <KComboBox>
#include <KLineEdit>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KUrlCompletion>

#define ACTIVE_MASK 0x0100
#define OTHER_MASK 0x0200
#define LEFT_MASK 0x0400
#define RIGHT_MASK 0x0800
#define INDEPENDENT_MASK 0x1000
#define EXECUTABLE_ID 0xFFFF

AddPlaceholderPopup::AddPlaceholderPopup(QWidget *parent)
    : QMenu(parent)
{
    _activeSub = new QMenu(i18n("Active panel"), this);
    _otherSub = new QMenu(i18n("Other panel"), this);
    _leftSub = new QMenu(i18n("Left panel"), this);
    _rightSub = new QMenu(i18n("Right panel"), this);
    _independentSub = new QMenu(i18n("Panel independent"), this);

    addMenu(_activeSub);
    addMenu(_otherSub);
    addMenu(_leftSub);
    addMenu(_rightSub);
    addMenu(_independentSub);

    QAction *chooseExecAct = _independentSub->addAction(i18n("Choose executable..."));
    chooseExecAct->setData(QVariant(EXECUTABLE_ID));

    _independentSub->addSeparator();

    // read the expressions array from the user menu and populate menus
    Expander expander;
    for (int i = 0; i < expander.placeholderCount(); ++i) {
        if (expander.placeholder(i)->expression().isEmpty()) {
            if (expander.placeholder(i)->needPanel()) {
                _activeSub->addSeparator();
                _otherSub->addSeparator();
                _leftSub->addSeparator();
                _rightSub->addSeparator();
            } else
                _independentSub->addSeparator();
        } else {
            QAction *action;

            if (expander.placeholder(i)->needPanel()) {
                action = _activeSub->addAction(i18n(expander.placeholder(i)->description().toUtf8().data()));
                action->setData(QVariant(i | ACTIVE_MASK));
                action = _otherSub->addAction(i18n(expander.placeholder(i)->description().toUtf8().data()));
                action->setData(QVariant(i | OTHER_MASK));
                action = _leftSub->addAction(i18n(expander.placeholder(i)->description().toUtf8().data()));
                action->setData(QVariant(i | LEFT_MASK));
                action = _rightSub->addAction(i18n(expander.placeholder(i)->description().toUtf8().data()));
                action->setData(QVariant(i | RIGHT_MASK));
            } else {
                action = _independentSub->addAction(i18n(expander.placeholder(i)->description().toUtf8().data()));
                action->setData(QVariant(i | INDEPENDENT_MASK));
            }
        }
    }
}

QString AddPlaceholderPopup::getPlaceholder(const QPoint &pos)
{
    QAction *res = exec(pos);
    if (res == nullptr)
        return QString();

    // add the selected flag to the command line
    if (res->data().toInt() == EXECUTABLE_ID) { // did the user need an executable ?
        // select an executable
        QString filename = QFileDialog::getOpenFileName(this);
        if (!filename.isEmpty()) {
            return filename + ' '; // with extra space
            // return filename; // without extra space
        }
    } else { // user selected something from the menus
        Expander expander;
        const exp_placeholder *currentPlaceholder =
            expander.placeholder(res->data().toInt() & ~(ACTIVE_MASK | OTHER_MASK | LEFT_MASK | RIGHT_MASK | INDEPENDENT_MASK));
        // if (&currentPlaceholder->expFunc == 0) {
        //     KMessageBox::error(this, "BOFH Excuse #93:\nFeature not yet implemented");
        //     return QString();
        // }
        auto *parameterDialog = new ParameterDialog(currentPlaceholder, this);
        QString panel, parameter = parameterDialog->getParameter();
        delete parameterDialog;
        // indicate the panel with 'a' 'o', 'l', 'r' or '_'.
        if (res->data().toInt() & ACTIVE_MASK)
            panel = 'a';
        else if (res->data().toInt() & OTHER_MASK)
            panel = 'o';
        else if (res->data().toInt() & LEFT_MASK)
            panel = 'l';
        else if (res->data().toInt() & RIGHT_MASK)
            panel = 'r';
        else if (res->data().toInt() & INDEPENDENT_MASK)
            panel = '_';
        // return '%' + panel + currentPlaceholder->expression() + parameter + "% "; // with extra space
        return '%' + panel + currentPlaceholder->expression() + parameter + '%'; // without extra space
    }
    return QString();
}

////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// ParameterDialog ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

ParameterDialog::ParameterDialog(const exp_placeholder *currentPlaceholder, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("User Action Parameter Dialog"));
    auto *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    _parameter.clear();
    _parameterCount = currentPlaceholder->parameterCount();

    QWidget *page = new QWidget(this);
    mainLayout->addWidget(page);
    auto *layout = new QVBoxLayout(page);
    layout->setSpacing(11);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel(i18n("This placeholder allows some parameter:"), page));

    for (int i = 0; i < _parameterCount; ++i) {
        if (currentPlaceholder->parameter(i).preset() == "__placeholder")
            _parameter.append(new ParameterPlaceholder(currentPlaceholder->parameter(i), page));
        else if (currentPlaceholder->parameter(i).preset() == "__yes")
            _parameter.append(new ParameterYes(currentPlaceholder->parameter(i), page));
        else if (currentPlaceholder->parameter(i).preset() == "__no")
            _parameter.append(new ParameterNo(currentPlaceholder->parameter(i), page));
        else if (currentPlaceholder->parameter(i).preset() == "__file")
            _parameter.append(new ParameterFile(currentPlaceholder->parameter(i), page));
        else if (currentPlaceholder->parameter(i).preset().indexOf("__choose") != -1)
            _parameter.append(new ParameterChoose(currentPlaceholder->parameter(i), page));
        else if (currentPlaceholder->parameter(i).preset() == "__select")
            _parameter.append(new ParameterSelect(currentPlaceholder->parameter(i), page));
        else if (currentPlaceholder->parameter(i).preset() == "__goto")
            _parameter.append(new ParameterGoto(currentPlaceholder->parameter(i), page));
        else if (currentPlaceholder->parameter(i).preset() == "__syncprofile")
            _parameter.append(new ParameterSyncprofile(currentPlaceholder->parameter(i), page));
        else if (currentPlaceholder->parameter(i).preset() == "__searchprofile")
            _parameter.append(new ParameterSearch(currentPlaceholder->parameter(i), page));
        else if (currentPlaceholder->parameter(i).preset() == "__panelprofile")
            _parameter.append(new ParameterPanelprofile(currentPlaceholder->parameter(i), page));
        else if (currentPlaceholder->parameter(i).preset().indexOf("__int") != -1)
            _parameter.append(new ParameterInt(currentPlaceholder->parameter(i), page));
        else
            _parameter.append(new ParameterText(currentPlaceholder->parameter(i), page));

        layout->addWidget(_parameter.last());
    }

    QFrame *line = new QFrame(page);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::RestoreDefaults);
    mainLayout->addWidget(buttonBox);

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(okButton, &QPushButton::clicked, this, &ParameterDialog::slotOk);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ParameterDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ParameterDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &ParameterDialog::reset);
}

QString ParameterDialog::getParameter()
{
    if (_parameterCount == 0) // meaning no parameters
        return QString();

    if (exec() == -1)
        return QString();

    qsizetype lastParameter = _parameterCount;
    while (--lastParameter > -1) {
        if (_parameter[lastParameter]->text() != _parameter[lastParameter]->preset() || _parameter[lastParameter]->necessary())
            break;
    }

    if (lastParameter < 0) // all parameters have default-values
        return QString();

    QString parameter;
    for (int i = 0; i <= lastParameter; ++i) {
        if (i > 0)
            parameter += ", ";
        parameter += '\"' + _parameter[i]->text().replace('\"', "\\\"") + '\"';
    }
    return '(' + parameter + ')';
}

void ParameterDialog::reset()
{
    for (int i = 0; i < _parameterCount; ++i)
        _parameter[i]->reset();
}

void ParameterDialog::slotOk()
{
    bool valid = true;
    for (int i = 0; i < _parameterCount; ++i) {
        if (_parameter[i]->necessary() && !_parameter[i]->valid())
            valid = false;
    }

    if (valid)
        accept();
}

///////////// ParameterText
ParameterText::ParameterText(const exp_parameter &parameter, QWidget *parent)
    : ParameterBase(parameter, parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel(i18n(parameter.description().toUtf8().data()), this));
    layout->addWidget(_lineEdit = new KLineEdit(parameter.preset(), this));
    _preset = parameter.preset();
}

QString ParameterText::text()
{
    return _lineEdit->text();
}
QString ParameterText::preset()
{
    return _preset;
}
void ParameterText::reset()
{
    _lineEdit->setText(_preset);
}
bool ParameterText::valid()
{
    if (_lineEdit->text().isEmpty())
        return false;
    else
        return true;
}

///////////// ParameterPlaceholder
ParameterPlaceholder::ParameterPlaceholder(const exp_parameter &parameter, QWidget *parent)
    : ParameterBase(parameter, parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel(i18n(parameter.description().toUtf8().data()), this));
    QWidget *hboxWidget = new QWidget(this);
    layout->addWidget(hboxWidget);
    auto *hbox = new QHBoxLayout(hboxWidget);

    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(6);
    _lineEdit = new KLineEdit(hboxWidget);
    hbox->addWidget(_lineEdit);
    _button = new QToolButton(hboxWidget);
    _button->setIcon(Icon("list-add"));
    hbox->addWidget(_button);
    connect(_button, &QToolButton::clicked, this, &ParameterPlaceholder::addPlaceholder);
}

QString ParameterPlaceholder::text()
{
    return _lineEdit->text();
}
QString ParameterPlaceholder::preset()
{
    return QString();
}
void ParameterPlaceholder::reset()
{
    _lineEdit->setText(QString());
}
bool ParameterPlaceholder::valid()
{
    if (_lineEdit->text().isEmpty())
        return false;
    else
        return true;
}
void ParameterPlaceholder::addPlaceholder()
{
    auto *popup = new AddPlaceholderPopup(this);
    QString exp = popup->getPlaceholder(mapToGlobal(QPoint(_button->pos().x() + _button->width() + 6, _button->pos().y() + _button->height() / 2)));
    _lineEdit->insert(exp);
    delete popup;
}

///////////// ParameterYes
ParameterYes::ParameterYes(const exp_parameter &parameter, QWidget *parent)
    : ParameterBase(parameter, parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(_checkBox = new QCheckBox(i18n(parameter.description().toUtf8().data()), this));
    _checkBox->setChecked(true);
}

QString ParameterYes::text()
{
    if (_checkBox->isChecked())
        return QString();
    else
        return "No";
}
QString ParameterYes::preset()
{
    return QString();
}
void ParameterYes::reset()
{
    _checkBox->setChecked(true);
}
bool ParameterYes::valid()
{
    return true;
}

///////////// ParameterNo
ParameterNo::ParameterNo(const exp_parameter &parameter, QWidget *parent)
    : ParameterBase(parameter, parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(_checkBox = new QCheckBox(i18n(parameter.description().toUtf8().data()), this));
    _checkBox->setChecked(false);
}

QString ParameterNo::text()
{
    if (_checkBox->isChecked())
        return "Yes";
    else
        return QString();
}
QString ParameterNo::preset()
{
    return QString();
}
void ParameterNo::reset()
{
    _checkBox->setChecked(false);
}
bool ParameterNo::valid()
{
    return true;
}

///////////// ParameterFile
ParameterFile::ParameterFile(const exp_parameter &parameter, QWidget *parent)
    : ParameterBase(parameter, parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel(i18n(parameter.description().toUtf8().data()), this));

    QWidget *hboxWidget = new QWidget(this);
    layout->addWidget(hboxWidget);
    auto *hbox = new QHBoxLayout(hboxWidget);

    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(6);
    _lineEdit = new KLineEdit(hboxWidget);
    hbox->addWidget(_lineEdit);
    _button = new QToolButton(hboxWidget);
    hbox->addWidget(_button);
    _button->setIcon(Icon("document-open"));
    connect(_button, &QToolButton::clicked, this, &ParameterFile::addFile);
}

QString ParameterFile::text()
{
    return _lineEdit->text();
}
QString ParameterFile::preset()
{
    return QString();
}
void ParameterFile::reset()
{
    _lineEdit->setText(QString());
}
bool ParameterFile::valid()
{
    if (_lineEdit->text().isEmpty())
        return false;
    else
        return true;
}
void ParameterFile::addFile()
{
    QString filename = QFileDialog::getOpenFileName(this);
    _lineEdit->insert(filename);
}

///////////// ParameterChoose
ParameterChoose::ParameterChoose(const exp_parameter &parameter, QWidget *parent)
    : ParameterBase(parameter, parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel(i18n(parameter.description().toUtf8().data()), this));
    layout->addWidget(_combobox = new KComboBox(this));
    _combobox->addItems(parameter.preset().section(':', 1).split(';'));
}

QString ParameterChoose::text()
{
    return _combobox->currentText();
}
QString ParameterChoose::preset()
{
    return _combobox->itemText(0);
}
void ParameterChoose::reset()
{
    _combobox->setCurrentIndex(0);
}
bool ParameterChoose::valid()
{
    return true;
}

///////////// ParameterSelect
ParameterSelect::ParameterSelect(const exp_parameter &parameter, QWidget *parent)
    : ParameterBase(parameter, parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel(i18n(parameter.description().toUtf8().data()), this));
    layout->addWidget(_combobox = new KComboBox(this));
    _combobox->setEditable(true);

    KConfigGroup group(krConfig, "Private");
    QStringList lst = group.readEntry("Predefined Selections", QStringList());
    if (lst.size() > 0)
        _combobox->addItems(lst);

    _combobox->lineEdit()->setText("*");
}

QString ParameterSelect::text()
{
    return _combobox->currentText();
}
QString ParameterSelect::preset()
{
    return "*";
}
void ParameterSelect::reset()
{
    _combobox->lineEdit()->setText("*");
}
bool ParameterSelect::valid()
{
    return true;
}

///////////// ParameterGoto
ParameterGoto::ParameterGoto(const exp_parameter &parameter, QWidget *parent)
    : ParameterBase(parameter, parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel(i18n(parameter.description().toUtf8().data()), this));

    QWidget *hboxWidget = new QWidget(this);
    auto *hbox = new QHBoxLayout(hboxWidget);

    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(6);
    _lineEdit = new KLineEdit(hboxWidget);
    _lineEdit->setCompletionObject(new KUrlCompletion(KUrlCompletion::DirCompletion));
    hbox->addWidget(_lineEdit);
    _dirButton = new QToolButton(hboxWidget);
    hbox->addWidget(_dirButton);
    _dirButton->setIcon(Icon("document-open"));
    connect(_dirButton, &QToolButton::clicked, this, &ParameterGoto::setDir);
    _placeholderButton = new QToolButton(hboxWidget);
    _placeholderButton->setIcon(Icon("list-add"));
    hbox->addWidget(_placeholderButton);
    connect(_placeholderButton, &QToolButton::clicked, this, &ParameterGoto::addPlaceholder);

    layout->addWidget(hboxWidget);
}

QString ParameterGoto::text()
{
    return _lineEdit->text();
}
QString ParameterGoto::preset()
{
    return QString();
}
void ParameterGoto::reset()
{
    _lineEdit->setText(QString());
}
bool ParameterGoto::valid()
{
    if (_lineEdit->text().isEmpty())
        return false;
    else
        return true;
}
void ParameterGoto::setDir()
{
    QString folder = QFileDialog::getExistingDirectory(this);
    _lineEdit->setText(folder);
}
void ParameterGoto::addPlaceholder()
{
    auto *popup = new AddPlaceholderPopup(this);
    QString exp = popup->getPlaceholder(
        mapToGlobal(QPoint(_placeholderButton->pos().x() + _placeholderButton->width() + 6, _placeholderButton->pos().y() + _placeholderButton->height() / 2)));
    _lineEdit->insert(exp);
    delete popup;
}

///////////// ParameterSyncprofile
ParameterSyncprofile::ParameterSyncprofile(const exp_parameter &parameter, QWidget *parent)
    : ParameterBase(parameter, parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel(i18n(parameter.description().toUtf8().data()), this));
    layout->addWidget(_combobox = new KComboBox(this));

    _combobox->addItems(ProfileManager::availableProfiles("SynchronizerProfile"));
}

QString ParameterSyncprofile::text()
{
    return _combobox->currentText();
}
QString ParameterSyncprofile::preset()
{
    return _combobox->itemText(0);
}
void ParameterSyncprofile::reset()
{
    _combobox->setCurrentIndex(0);
}
bool ParameterSyncprofile::valid()
{
    return true;
}

///////////// ParameterSearch
ParameterSearch::ParameterSearch(const exp_parameter &parameter, QWidget *parent)
    : ParameterBase(parameter, parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel(i18n(parameter.description().toUtf8().data()), this));
    layout->addWidget(_combobox = new KComboBox(this));

    _combobox->addItems(ProfileManager::availableProfiles("SearcherProfile"));
}

QString ParameterSearch::text()
{
    return _combobox->currentText();
}
QString ParameterSearch::preset()
{
    return _combobox->itemText(0);
}
void ParameterSearch::reset()
{
    _combobox->setCurrentIndex(0);
}
bool ParameterSearch::valid()
{
    return true;
}

///////////// ParameterPanelprofile
ParameterPanelprofile::ParameterPanelprofile(const exp_parameter &parameter, QWidget *parent)
    : ParameterBase(parameter, parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel(i18n(parameter.description().toUtf8().data()), this));
    layout->addWidget(_combobox = new KComboBox(this));

    _combobox->addItems(ProfileManager::availableProfiles("Panel"));
}

QString ParameterPanelprofile::text()
{
    return _combobox->currentText();
}
QString ParameterPanelprofile::preset()
{
    return _combobox->itemText(0);
}
void ParameterPanelprofile::reset()
{
    _combobox->setCurrentIndex(0);
}
bool ParameterPanelprofile::valid()
{
    return true;
}

///////////// ParameterInt
ParameterInt::ParameterInt(const exp_parameter &parameter, QWidget *parent)
    : ParameterBase(parameter, parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel(i18n(parameter.description().toUtf8().data()), this));
    layout->addWidget(_spinbox = new QSpinBox(this));
    QStringList para = parameter.preset().section(':', 1).split(';');

    _spinbox->setMinimum(para[0].toInt());
    _spinbox->setMaximum(para[1].toInt());
    _spinbox->setSingleStep(para[2].toInt());
    _spinbox->setValue(para[3].toInt());

    _default = _spinbox->value();
}

QString ParameterInt::text()
{
    return _spinbox->text();
}
QString ParameterInt::preset()
{
    return QString("%1").arg(_default);
}
void ParameterInt::reset()
{
    return _spinbox->setValue(_default);
}
bool ParameterInt::valid()
{
    return true;
}
