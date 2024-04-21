/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "packguibase.h"

// QtCore
#include <QVariant>
// QtGui
#include <QGuiApplication>
#include <QImage>
#include <QKeyEvent>
#include <QPixmap>
// QtWidgets
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>

#include <KComboBox>
#include <KIO/Global>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KStandardGuiItem>

#include "../GUI/krhistorycombobox.h"
#include "../compat.h"
#include "../defaults.h"
#include "../icon.h"
#include "../krglobal.h"

/*
 *  Constructs a PackGUIBase which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
PackGUIBase::PackGUIBase(QWidget *parent)
    : QDialog(parent)
    , expanded(false)
{
    KConfigGroup group(krConfig, "Archives");

    setModal(true);
    resize(430, 140);
    setWindowTitle(i18n("Pack"));
    grid = new QGridLayout(this);
    grid->setSpacing(6);
    grid->setContentsMargins(11, 11, 11, 11);

    hbox = new QHBoxLayout;
    hbox->setSpacing(6);
    hbox->setContentsMargins(0, 0, 0, 0);

    TextLabel3 = new QLabel(this);
    TextLabel3->setText(i18n("To archive"));
    hbox->addWidget(TextLabel3);

    nameData = new QLineEdit(this);
    hbox->addWidget(nameData);

    typeData = new QComboBox(this);
    typeData->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    connect(typeData, QOverload<const QString &>::of(&QComboBox::QCOMBOBOX_ACTIVATED), this, &PackGUIBase::checkConsistency);
    connect(typeData, QOverload<const QString &>::of(&QComboBox::QCOMBOBOX_HIGHLIGHTED), this, &PackGUIBase::checkConsistency);
    hbox->addWidget(typeData);

    grid->addLayout(hbox, 1, 0);

    hbox_2 = new QHBoxLayout;
    hbox_2->setSpacing(6);
    hbox_2->setContentsMargins(0, 0, 0, 0);

    TextLabel5 = new QLabel(this);
    TextLabel5->setText(i18n("In folder"));
    hbox_2->addWidget(TextLabel5);

    dirData = new QLineEdit(this);
    hbox_2->addWidget(dirData);

    browseButton = new QToolButton(this);
    browseButton->setIcon(Icon("document-open"));
    hbox_2->addWidget(browseButton);
    auto *spacer = new QSpacerItem(48, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
    hbox_2->addItem(spacer);

    grid->addLayout(hbox_2, 2, 0);

    hbox_3 = new QHBoxLayout;
    hbox_3->setSpacing(6);
    hbox_3->setContentsMargins(0, 0, 0, 0);

    PixmapLabel1 = new QLabel(this);
    PixmapLabel1->setPixmap(Icon("package-x-generic").pixmap(32));
    PixmapLabel1->setScaledContents(true);
    PixmapLabel1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    hbox_3->addWidget(PixmapLabel1);

    TextLabel1 = new QLabel(this);
    TextLabel1->setText(i18n("Pack"));
    hbox_3->addWidget(TextLabel1);

    grid->addLayout(hbox_3, 0, 0);

    hbox_4 = new QHBoxLayout;
    hbox_4->setSpacing(6);
    hbox_4->setContentsMargins(0, 0, 0, 0);

    auto *spacer_3 = new QSpacerItem(20, 26, QSizePolicy::Fixed, QSizePolicy::Expanding);
    hbox_4->addItem(spacer_3);
    grid->addLayout(hbox_4, 3, 0);

    advancedWidget = new QWidget(this);

    hbox_5 = new QGridLayout(advancedWidget);
    hbox_5->setSpacing(6);
    hbox_5->setContentsMargins(0, 0, 0, 0);

    auto *compressLayout = new QVBoxLayout;
    compressLayout->setSpacing(6);
    compressLayout->setContentsMargins(0, 0, 0, 0);

    multipleVolume = new QCheckBox(i18n("Multiple volume archive"), advancedWidget);
    connect(multipleVolume, &QCheckBox::toggled, this, &PackGUIBase::checkConsistency);
    compressLayout->addWidget(multipleVolume, 0, Qt::Alignment());

    auto *volumeHbox = new QHBoxLayout;

    auto *spacer_5 = new QSpacerItem(20, 26, QSizePolicy::Fixed, QSizePolicy::Fixed);
    volumeHbox->addItem(spacer_5);

    TextLabel7 = new QLabel(i18n("Size:"), advancedWidget);
    volumeHbox->addWidget(TextLabel7);

    volumeSpinBox = new QSpinBox(advancedWidget);
    volumeSpinBox->setMinimum(1);
    volumeSpinBox->setMaximum(9999);
    volumeSpinBox->setValue(1440);
    volumeHbox->addWidget(volumeSpinBox);

    volumeUnitCombo = new QComboBox(advancedWidget);
    volumeUnitCombo->addItem("B");
    volumeUnitCombo->addItem("KB");
    volumeUnitCombo->addItem("MB");
    volumeUnitCombo->setCurrentIndex(1);
    volumeHbox->addWidget(volumeUnitCombo);

    compressLayout->addLayout(volumeHbox);

    int level = group.readEntry("Compression level", _defaultCompressionLevel);
    setCompressionLevel = new QCheckBox(i18n("Set compression level"), advancedWidget);
    if (level != _defaultCompressionLevel)
        setCompressionLevel->setChecked(true);
    connect(setCompressionLevel, &QCheckBox::toggled, this, &PackGUIBase::checkConsistency);
    compressLayout->addWidget(setCompressionLevel, 0, Qt::Alignment());

    auto *sliderHbox = new QHBoxLayout;

    auto *spacer_6 = new QSpacerItem(20, 26, QSizePolicy::Fixed, QSizePolicy::Fixed);
    sliderHbox->addItem(spacer_6);

    QWidget *sliderVBoxWidget = new QWidget(advancedWidget);
    auto *sliderVBox = new QVBoxLayout(sliderVBoxWidget);

    compressionSlider = new QSlider(Qt::Horizontal, sliderVBoxWidget);
    compressionSlider->setMinimum(1);
    compressionSlider->setMaximum(9);
    compressionSlider->setPageStep(1);
    compressionSlider->setValue(level);
    compressionSlider->setTickPosition(QSlider::TicksBelow);
    sliderVBox->addWidget(compressionSlider);

    QWidget *minmaxWidget = new QWidget(sliderVBoxWidget);
    sliderVBox->addWidget(minmaxWidget);

    auto *minmaxHbox = new QHBoxLayout(minmaxWidget);

    minLabel = new QLabel(i18n("MIN"), minmaxWidget);
    maxLabel = new QLabel(i18n("MAX"), minmaxWidget);
    maxLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    minmaxHbox->addWidget(minLabel);
    minmaxHbox->addWidget(maxLabel);

    sliderHbox->addWidget(sliderVBoxWidget);

    compressLayout->addLayout(sliderHbox);

    compressLayout->addStretch(0);
    hbox_5->addLayout(compressLayout, 0, 0);

    QFrame *vline = new QFrame(advancedWidget);
    vline->setFrameStyle(QFrame::VLine | QFrame::Sunken);
    vline->setMinimumWidth(20);
    hbox_5->addWidget(vline, 0, 1);

    auto *passwordGrid = new QGridLayout;
    passwordGrid->setSpacing(6);
    passwordGrid->setContentsMargins(0, 0, 0, 0);

    TextLabel4 = new QLabel(advancedWidget);
    TextLabel4->setText(i18n("Password"));
    passwordGrid->addWidget(TextLabel4, 0, 0);

    password = new QLineEdit(advancedWidget);
    password->setEchoMode(QLineEdit::Password);
    connect(password, &QLineEdit::textChanged, this, &PackGUIBase::checkConsistency);

    passwordGrid->addWidget(password, 0, 1);

    TextLabel6 = new QLabel(advancedWidget);
    TextLabel6->setText(i18n("Again"));
    passwordGrid->addWidget(TextLabel6, 1, 0);

    passwordAgain = new QLineEdit(advancedWidget);
    passwordAgain->setEchoMode(QLineEdit::Password);
    connect(passwordAgain, &QLineEdit::textChanged, this, &PackGUIBase::checkConsistency);

    passwordGrid->addWidget(passwordAgain, 1, 1);

    auto *consistencyHbox = new QHBoxLayout;

    auto *spacer_cons = new QSpacerItem(48, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
    consistencyHbox->addItem(spacer_cons);

    passwordConsistencyLabel = new QLabel(advancedWidget);
    consistencyHbox->addWidget(passwordConsistencyLabel);
    passwordGrid->addLayout(consistencyHbox, 2, 0, 1, 2);

    encryptHeaders = new QCheckBox(i18n("Encrypt headers"), advancedWidget);
    passwordGrid->addWidget(encryptHeaders, 3, 0, 1, 2);

    auto *spacer_psw = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Expanding);
    passwordGrid->addItem(spacer_psw, 4, 0);

    hbox_5->addLayout(passwordGrid, 0, 2);

    hbox_7 = new QHBoxLayout;
    hbox_7->setSpacing(6);
    hbox_7->setContentsMargins(0, 0, 0, 0);

    TextLabel8 = new QLabel(i18n("Command line switches:"), advancedWidget);
    TextLabel8->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hbox_7->addWidget(TextLabel8);

    commandLineSwitches = new KrHistoryComboBox(advancedWidget);
    commandLineSwitches->setMaxCount(25); // remember 25 items
    commandLineSwitches->setDuplicatesEnabled(false);
    commandLineSwitches->setMinimumContentsLength(10);

    QStringList list = group.readEntry("Command Line Switches", QStringList());
    commandLineSwitches->setHistoryItems(list);

    hbox_7->addWidget(commandLineSwitches);

    hbox_5->addLayout(hbox_7, 1, 0, 1, 3);

    advancedWidget->hide();
    checkConsistency();

    grid->addWidget(advancedWidget, 4, 0);

    hbox_6 = new QHBoxLayout;
    hbox_6->setSpacing(6);
    hbox_6->setContentsMargins(0, 0, 0, 0);

    advancedButton = new QPushButton(this);
    advancedButton->setText(i18n("&Advanced >>"));
    hbox_6->addWidget(advancedButton);

    auto *spacer_2 = new QSpacerItem(140, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
    hbox_6->addItem(spacer_2);

    okButton = new QPushButton(this);
    KStandardGuiItem::assign(okButton, KStandardGuiItem::Ok);
    okButton->setDefault(true);
    hbox_6->addWidget(okButton);

    cancelButton = new QPushButton(this);
    KStandardGuiItem::assign(cancelButton, KStandardGuiItem::Cancel);
    hbox_6->addWidget(cancelButton);

    grid->addLayout(hbox_6, 6, 0);

    // signals and slots connections
    connect(okButton, &QPushButton::clicked, this, &PackGUIBase::accept);
    connect(advancedButton, &QPushButton::clicked, this, &PackGUIBase::expand);
    connect(cancelButton, &QPushButton::clicked, this, &PackGUIBase::reject);
    connect(browseButton, &QToolButton::clicked, this, &PackGUIBase::browse);
}

/*
 *  Destroys the object and frees any allocated resources
 */
PackGUIBase::~PackGUIBase()
{
    // no need to delete child widgets, Qt does it all for us
}

void PackGUIBase::browse()
{
    qWarning("PackGUIBase::browse(): Not implemented yet!");
}

void PackGUIBase::expand()
{
    expanded = !expanded;

    advancedButton->setText(expanded ? i18n("&Advanced <<") : i18n("&Advanced >>"));

    if (expanded)
        advancedWidget->show();
    else {
        advancedWidget->hide();
        layout()->activate();
        QSize minSize = minimumSize();
        resize(width(), minSize.height());
    }
    show();
}

void PackGUIBase::checkConsistency()
{
    QPalette p = QGuiApplication::palette();
    QPalette pal = passwordConsistencyLabel->palette();
    if (password->text().isEmpty() && passwordAgain->text().isEmpty()) {
        pal.setColor(passwordConsistencyLabel->foregroundRole(), p.color(QPalette::Active, QPalette::Text));
        passwordConsistencyLabel->setText(i18n("No password specified"));
    } else if (password->text() == passwordAgain->text()) {
        pal.setColor(passwordConsistencyLabel->foregroundRole(), p.color(QPalette::Active, QPalette::Text));
        passwordConsistencyLabel->setText(i18n("The passwords are equal"));
    } else {
        pal.setColor(passwordConsistencyLabel->foregroundRole(), Qt::red);
        passwordConsistencyLabel->setText(i18n("The passwords are different"));
    }
    passwordConsistencyLabel->setPalette(pal);

    QString packer = typeData->currentText();

    bool passworded = false;
    if (packer == "7z" || packer == "rar" || packer == "zip" || packer == "arj")
        passworded = true;

    passwordConsistencyLabel->setEnabled(passworded);
    password->setEnabled(passworded);
    passwordAgain->setEnabled(passworded);
    TextLabel4->setEnabled(passworded);
    TextLabel6->setEnabled(passworded);

    encryptHeaders->setEnabled(packer == "rar");

    multipleVolume->setEnabled(packer == "rar" || packer == "arj");
    bool volumeEnabled = multipleVolume->isEnabled() && multipleVolume->isChecked();
    volumeSpinBox->setEnabled(volumeEnabled);
    volumeUnitCombo->setEnabled(volumeEnabled);
    TextLabel7->setEnabled(volumeEnabled);

    /* TODO */
    setCompressionLevel->setEnabled(packer == "rar" || packer == "arj" || packer == "zip" || packer == "7z");
    bool sliderEnabled = setCompressionLevel->isEnabled() && setCompressionLevel->isChecked();
    compressionSlider->setEnabled(sliderEnabled);
    minLabel->setEnabled(sliderEnabled);
    maxLabel->setEnabled(sliderEnabled);
}

bool PackGUIBase::extraProperties(QMap<QString, QString> &inMap)
{
    inMap.clear();

    KConfigGroup group(krConfig, "Archives");

    if (password->isEnabled() && passwordAgain->isEnabled()) {
        if (password->text() != passwordAgain->text()) {
            KMessageBox::error(this, i18n("Cannot pack, the passwords are different."));
            return false;
        }

        if (!password->text().isEmpty()) {
            inMap["Password"] = password->text();

            if (encryptHeaders->isEnabled() && encryptHeaders->isChecked())
                inMap["EncryptHeaders"] = '1';
        }
    }

    if (multipleVolume->isEnabled() && multipleVolume->isChecked()) {
        KIO::filesize_t size = volumeSpinBox->value();

        switch (volumeUnitCombo->currentIndex()) {
        case 2:
            size *= 1000;
#if __GNUC__ >= 7
            [[gnu::fallthrough]];
#endif
        case 1:
            size *= 1000;
        default:
            break;
        }

        if (size < 10000) {
            KMessageBox::error(this, i18n("Invalid volume size."));
            return false;
        }

        QString sbuffer;
        sbuffer.asprintf("%llu", size);

        inMap["VolumeSize"] = sbuffer;
    }

    if (setCompressionLevel->isEnabled() && setCompressionLevel->isChecked()) {
        inMap["CompressionLevel"] = QString("%1").arg(compressionSlider->value());
        int level = compressionSlider->value();
        group.writeEntry("Compression level", level);
    }

    QString cmdArgs = commandLineSwitches->currentText().trimmed();
    if (!cmdArgs.isEmpty()) {
        bool firstChar = true;
        QChar quote = QChar::Null;

        for (int i = 0; i < cmdArgs.length(); i++) {
            QChar ch(cmdArgs[i]);
            if (ch.isSpace())
                continue;

            if (ch == quote) {
                quote = QChar::Null;
                continue;
            }

            if (firstChar && ch != QLatin1Char('-')) {
                KMessageBox::error(this, i18n("Invalid command line switch.\nA switch must start with '-'."));
                return false;
            }

            firstChar = false;

            if (quote == QLatin1Char('"'))
                continue;
            if (quote == QChar::Null && (ch == QLatin1Char('\'') || ch == QLatin1Char('"')))
                quote = ch;
            if (ch == QLatin1Char('\\')) {
                if (i == cmdArgs.length() - 1) {
                    KMessageBox::error(this, i18n("Invalid command line switch.\nBackslashes cannot be the last character."));
                    return false;
                }
                i++;
            }
        }

        if (quote != QChar::Null) {
            KMessageBox::error(this, i18n("Invalid command line switch.\nUnclosed quotation mark."));
            return false;
        }

        commandLineSwitches->addToHistory(cmdArgs);

        inMap["CommandLineSwitches"] = cmdArgs;
    }

    return true;
}
