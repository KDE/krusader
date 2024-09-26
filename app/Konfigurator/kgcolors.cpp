/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgcolors.h"
#include "../Panel/krcolorcache.h"
#include "../Synchronizer/synchronizercolors.h"
#include "../defaults.h"

// QtCore
#include <QList>
#include <QStandardPaths>
// QtGui
#include <QGuiApplication>
// QtWidgets
#include <QFileDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTabWidget>

#include <KLocalizedString>
#include <KMessageBox>
#include <utility>

KgColors::KgColors(bool first, QWidget *parent)
    : KonfiguratorPage(first, parent)
    , offset(0)
    , activeTabIdx(-1)
    , inactiveTabIdx(-1)
    ,
#ifdef SYNCHRONIZER_ENABLED
    synchronizerTabIdx(-1)
    ,
#endif
    otherTabIdx(-1)
{
    QWidget *innerWidget = new QFrame(this);
    setWidget(innerWidget);
    setWidgetResizable(true);
    auto *kgColorsLayout = new QGridLayout(innerWidget);
    kgColorsLayout->setSpacing(6);

    //  -------------------------- GENERAL GROUPBOX ----------------------------------

    QGroupBox *generalGrp = createFrame(i18n("General"), innerWidget);
    QGridLayout *generalGrid = createGridLayout(generalGrp);

    generalGrid->setSpacing(0);
    generalGrid->setContentsMargins(5, 5, 5, 5);

    KONFIGURATOR_CHECKBOX_PARAM generalSettings[] =
        //  cfg_class   cfg_name    default     text    restart    tooltip
        {{"Colors",
          "KDE Default",
          _KDEDefaultColors,
          i18n("Use the default KDE colors"),
          false,
          "<p><img src='toolbar|kcontrol'></p>"
              + i18n("<p>Use KDE's global color configuration.</p><p><i>KDE System Settings -> Application Appearance  -> Colors</i></p>")},
         {"Colors",
          "Enable Alternate Background",
          _AlternateBackground,
          i18n("Use alternate background color"),
          false,
          i18n("<p>The <b>background color</b> and the <b>alternate background</b> color alternates line by line.</p><p>When you don't use the <i>KDE default "
               "colors</i>, you can configure the alternate colors in the <i>colors</i> box.</p>")},
         {"Colors",
          "Show Current Item Always",
          _ShowCurrentItemAlways,
          i18n("Show current item even if not focused"),
          false,
          i18n("<p>Shows the last cursor position in the non active list panel.</p><p>This option is only available when you don't use the <i>KDE default "
               "colors</i>.</p>")},
         {"Colors",
          "Dim Inactive Colors",
          _DimInactiveColors,
          i18n("Dim the colors of the inactive panel"),
          false,
          i18n("<p>The colors of the inactive panel are calculated by a dim color and a dim factor.</p>")}};

    generals = createCheckBoxGroup(0, 2, generalSettings, sizeof(generalSettings) / sizeof(generalSettings[0]), generalGrp);
    generalGrid->addWidget(generals, 1, 0);

    generals->layout()->setSpacing(5);

    connect(generals->find("KDE Default"), &KonfiguratorCheckBox::stateChanged, this, &KgColors::slotDisable);
    connect(generals->find("Show Current Item Always"), &KonfiguratorCheckBox::stateChanged, this, &KgColors::slotDisable);
    connect(generals->find("Dim Inactive Colors"), &KonfiguratorCheckBox::stateChanged, this, &KgColors::slotDisable);

    kgColorsLayout->addWidget(generalGrp, 0, 0, 1, 3);
    QWidget *hboxWidget = new QWidget(innerWidget);
    auto *hbox = new QHBoxLayout(hboxWidget);

    //  -------------------------- COLORS GROUPBOX ----------------------------------

    QGroupBox *colorsFrameGrp = createFrame(i18n("Colors"), hboxWidget);
    QGridLayout *colorsFrameGrid = createGridLayout(colorsFrameGrp);
    colorsFrameGrid->setSpacing(0);
    colorsFrameGrid->setContentsMargins(3, 3, 3, 3);

    colorTabWidget = new QTabWidget(colorsFrameGrp);

    colorsGrp = new QWidget(colorTabWidget);
    activeTabIdx = colorTabWidget->addTab(colorsGrp, i18n("Active"));

    colorsGrid = new QGridLayout(colorsGrp);
    colorsGrid->setSpacing(0);
    colorsGrid->setContentsMargins(2, 2, 2, 2);

    ADDITIONAL_COLOR transparent = {i18n("Transparent"), Qt::white, "transparent"};

    QPalette p = QGuiApplication::palette();

    addColorSelector("Foreground", i18n("Foreground:"), p.color(QPalette::Active, QPalette::Text));
    addColorSelector("Directory Foreground", i18n("Folder foreground:"), getColorSelector("Foreground")->getColor(), i18n("Same as foreground"));
    addColorSelector("Executable Foreground", i18n("Executable foreground:"), getColorSelector("Foreground")->getColor(), i18n("Same as foreground"));
    addColorSelector("Symlink Foreground", i18n("Symbolic link foreground:"), getColorSelector("Foreground")->getColor(), i18n("Same as foreground"));
    addColorSelector("Invalid Symlink Foreground", i18n("Invalid symlink foreground:"), getColorSelector("Foreground")->getColor(), i18n("Same as foreground"));
    addColorSelector("Background", i18n("Background:"), p.color(QPalette::Active, QPalette::Base));
    ADDITIONAL_COLOR sameAsBckgnd = {i18n("Same as background"), getColorSelector("Background")->getColor(), "Background"};
    addColorSelector("Alternate Background", i18n("Alternate background:"), p.color(QPalette::Active, QPalette::AlternateBase), "", &sameAsBckgnd, 1);
    addColorSelector("Marked Foreground", i18n("Selected foreground:"), p.color(QPalette::Active, QPalette::HighlightedText), "", &transparent, 1);
    addColorSelector("Marked Background", i18n("Selected background:"), p.color(QPalette::Active, QPalette::Highlight), "", &sameAsBckgnd, 1);
    ADDITIONAL_COLOR sameAsAltern = {i18n("Same as alt. background"), getColorSelector("Alternate Background")->getColor(), "Alternate Background"};
    addColorSelector("Alternate Marked Background",
                     i18n("Alternate selected background:"),
                     getColorSelector("Marked Background")->getColor(),
                     i18n("Same as selected background"),
                     &sameAsAltern,
                     1);
    addColorSelector("Current Foreground", i18n("Current foreground:"), Qt::white, i18n("Not used"));
    ADDITIONAL_COLOR sameAsMarkedForegnd = {i18n("Same as selected foreground"), getColorSelector("Marked Foreground")->getColor(), "Marked Foreground"};
    addColorSelector("Marked Current Foreground", i18n("Selected current foreground:"), Qt::white, i18n("Not used"), &sameAsMarkedForegnd, 1);
    addColorSelector("Current Background", i18n("Current background:"), Qt::white, i18n("Not used"), &sameAsBckgnd, 1);
    addColorSelector("Rename Foreground", i18n("Rename foreground:"), getColorSelector("Foreground")->getColor(), i18n("Same as foreground"));
    addColorSelector("Rename Background", i18n("Rename background:"), getColorSelector("Background")->getColor(), i18n("Same as background"));

    colorsGrid->addWidget(createSpacer(colorsGrp), static_cast<int>(itemList.count()) - offset, 1);

    connect(getColorSelector("Foreground"), &KonfiguratorColorChooser::colorChanged, this, &KgColors::slotForegroundChanged);
    connect(getColorSelector("Background"), &KonfiguratorColorChooser::colorChanged, this, &KgColors::slotBackgroundChanged);
    connect(getColorSelector("Alternate Background"), &KonfiguratorColorChooser::colorChanged, this, &KgColors::slotAltBackgroundChanged);
    connect(getColorSelector("Marked Background"), &KonfiguratorColorChooser::colorChanged, this, &KgColors::slotMarkedBackgroundChanged);

    inactiveColorStack = new QStackedWidget(colorTabWidget);
    inactiveTabIdx = colorTabWidget->addTab(inactiveColorStack, i18n("Inactive"));

    colorsGrp = normalInactiveWidget = new QWidget(inactiveColorStack);

    colorsGrid = new QGridLayout(normalInactiveWidget);
    colorsGrid->setSpacing(0);
    colorsGrid->setContentsMargins(2, 2, 2, 2);

    offset = endOfActiveColors = static_cast<int>(itemList.count());

    addColorSelector("Inactive Foreground", i18n("Foreground:"), getColorSelector("Foreground")->getColor(), i18n("Same as active"));
    ADDITIONAL_COLOR sameAsInactForegnd = {i18n("Same as foreground"), getColorSelector("Inactive Foreground")->getColor(), "Inactive Foreground"};
    addColorSelector("Inactive Directory Foreground",
                     i18n("Folder foreground:"),
                     getColorSelector("Directory Foreground")->getColor(),
                     i18n("Same as active"),
                     &sameAsInactForegnd,
                     1);
    addColorSelector("Inactive Executable Foreground",
                     i18n("Executable foreground:"),
                     getColorSelector("Executable Foreground")->getColor(),
                     i18n("Same as active"),
                     &sameAsInactForegnd,
                     1);
    addColorSelector("Inactive Symlink Foreground",
                     i18n("Symbolic link foreground:"),
                     getColorSelector("Symlink Foreground")->getColor(),
                     i18n("Same as active"),
                     &sameAsInactForegnd,
                     1);
    addColorSelector("Inactive Invalid Symlink Foreground",
                     i18n("Invalid symlink foreground:"),
                     getColorSelector("Invalid Symlink Foreground")->getColor(),
                     i18n("Same as active"),
                     &sameAsInactForegnd,
                     1);
    addColorSelector("Inactive Background", i18n("Background:"), getColorSelector("Background")->getColor(), i18n("Same as active"));
    ADDITIONAL_COLOR sameAsInactBckgnd = {i18n("Same as background"), getColorSelector("Inactive Background")->getColor(), "Inactive Background"};
    addColorSelector("Inactive Alternate Background",
                     i18n("Alternate background:"),
                     getColorSelector("Alternate Background")->getColor(),
                     i18n("Same as active"),
                     &sameAsInactBckgnd,
                     1);
    addColorSelector("Inactive Marked Foreground",
                     i18n("Selected foreground:"),
                     getColorSelector("Marked Foreground")->getColor(),
                     i18n("Same as active"),
                     &transparent,
                     1);
    addColorSelector("Inactive Marked Background",
                     i18n("Selected background:"),
                     getColorSelector("Marked Background")->getColor(),
                     i18n("Same as active"),
                     &sameAsInactBckgnd,
                     1);
    ADDITIONAL_COLOR sameAsInactAltern[] = {
        {i18n("Same as alt. background"), getColorSelector("Inactive Alternate Background")->getColor(), "Inactive Alternate Background"},
        {i18n("Same as selected background"), getColorSelector("Inactive Marked Background")->getColor(), "Inactive Marked Background"}};
    addColorSelector("Inactive Alternate Marked Background",
                     i18n("Alternate selected background:"),
                     getColorSelector("Alternate Marked Background")->getColor(),
                     i18n("Same as active"),
                     sameAsInactAltern,
                     2);
    addColorSelector("Inactive Current Foreground", i18n("Current foreground:"), getColorSelector("Current Foreground")->getColor(), i18n("Same as active"));
    ADDITIONAL_COLOR sameAsInactMarkedForegnd = {i18n("Same as selected foreground"),
                                                 getColorSelector("Inactive Marked Foreground")->getColor(),
                                                 "Inactive Marked Foreground"};
    addColorSelector("Inactive Marked Current Foreground",
                     i18n("Selected current foreground:"),
                     getColorSelector("Marked Current Foreground")->getColor(),
                     i18n("Same as active"),
                     &sameAsInactMarkedForegnd,
                     1);
    addColorSelector("Inactive Current Background",
                     i18n("Current background:"),
                     getColorSelector("Current Background")->getColor(),
                     i18n("Same as active"),
                     &sameAsInactBckgnd,
                     1);

    colorsGrid->addWidget(createSpacer(normalInactiveWidget), static_cast<int>(itemList.count()) - offset, 1);

    connect(getColorSelector("Inactive Foreground"), &KonfiguratorColorChooser::colorChanged, this, &KgColors::slotInactiveForegroundChanged);
    connect(getColorSelector("Inactive Background"), &KonfiguratorColorChooser::colorChanged, this, &KgColors::slotInactiveBackgroundChanged);
    connect(getColorSelector("Inactive Alternate Background"), &KonfiguratorColorChooser::colorChanged, this, &KgColors::slotInactiveAltBackgroundChanged);
    connect(getColorSelector("Inactive Marked Background"), &KonfiguratorColorChooser::colorChanged, this, &KgColors::slotInactiveMarkedBackgroundChanged);

    offset = endOfPanelColors = static_cast<int>(itemList.count());

    inactiveColorStack->addWidget(normalInactiveWidget);

    colorsGrp = dimmedInactiveWidget = new QWidget(inactiveColorStack);

    colorsGrid = new QGridLayout(dimmedInactiveWidget);
    colorsGrid->setSpacing(0);
    colorsGrid->setContentsMargins(2, 2, 2, 2);

    addColorSelector("Dim Target Color", i18n("Dim target color:"), Qt::black);

    int index = static_cast<int>(itemList.count()) - offset;
    QLabel *spinBoxLabel = addLabel(colorsGrid, index, 0, i18n("Dim factor:"), colorsGrp);
    labelList.append(spinBoxLabel);
    dimFactor = createSpinBox("Colors", "Dim Factor", 80, 0, 100, spinBoxLabel, colorsGrp);
    dimFactor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    colorsGrid->addWidget(dimFactor, index++, 1);

    colorsGrid->addWidget(createSpacer(dimmedInactiveWidget), static_cast<int>(itemList.count()) + 1 - offset, 1);

    inactiveColorStack->addWidget(dimmedInactiveWidget);

    inactiveColorStack->setCurrentWidget(normalInactiveWidget);

    ADDITIONAL_COLOR KDEDefaultBase = {i18n("KDE default"), p.color(QPalette::Active, QPalette::Base), "KDE default"};
    ADDITIONAL_COLOR KDEDefaultFore = {i18n("KDE default"), p.color(QPalette::Active, QPalette::Text), "KDE default"};

#ifdef SYNCHRONIZER_ENABLED
    colorsGrp = new QWidget(colorTabWidget);
    synchronizerTabIdx = colorTabWidget->addTab(colorsGrp, i18n("Synchronizer"));

    colorsGrid = new QGridLayout(colorsGrp);
    colorsGrid->setSpacing(0);
    colorsGrid->setContentsMargins(2, 2, 2, 2);

    offset = endOfPanelColors = static_cast<int>(itemList.count());

    DECLARE_SYNCHRONIZER_BACKGROUND_DEFAULTS;
    DECLARE_SYNCHRONIZER_FOREGROUND_DEFAULTS;

    addColorSelector("Synchronizer Equals Foreground", i18n("Equals foreground:"), SYNCHRONIZER_FOREGROUND_DEFAULTS[0], QString(), &KDEDefaultFore, 1);
    addColorSelector("Synchronizer Equals Background", i18n("Equals background:"), SYNCHRONIZER_BACKGROUND_DEFAULTS[0], QString(), &KDEDefaultBase, 1);
    addColorSelector("Synchronizer Differs Foreground", i18n("Differing foreground:"), SYNCHRONIZER_FOREGROUND_DEFAULTS[1], QString(), &KDEDefaultFore, 1);
    addColorSelector("Synchronizer Differs Background", i18n("Differing background:"), SYNCHRONIZER_BACKGROUND_DEFAULTS[1], QString(), &KDEDefaultBase, 1);
    addColorSelector("Synchronizer LeftCopy Foreground", i18n("Copy to left foreground:"), SYNCHRONIZER_FOREGROUND_DEFAULTS[2], QString(), &KDEDefaultFore, 1);
    addColorSelector("Synchronizer LeftCopy Background", i18n("Copy to left background:"), SYNCHRONIZER_BACKGROUND_DEFAULTS[2], QString(), &KDEDefaultBase, 1);
    addColorSelector("Synchronizer RightCopy Foreground",
                     i18n("Copy to right foreground:"),
                     SYNCHRONIZER_FOREGROUND_DEFAULTS[3],
                     QString(),
                     &KDEDefaultFore,
                     1);
    addColorSelector("Synchronizer RightCopy Background",
                     i18n("Copy to right background:"),
                     SYNCHRONIZER_BACKGROUND_DEFAULTS[3],
                     QString(),
                     &KDEDefaultBase,
                     1);
    addColorSelector("Synchronizer Delete Foreground", i18n("Delete foreground:"), SYNCHRONIZER_FOREGROUND_DEFAULTS[4], QString(), &KDEDefaultFore, 1);
    addColorSelector("Synchronizer Delete Background", i18n("Delete background:"), SYNCHRONIZER_BACKGROUND_DEFAULTS[4], QString(), &KDEDefaultBase, 1);

    colorsGrid->addWidget(createSpacer(colorsGrp), static_cast<int>(itemList.count()) - offset, 1);
#endif

    colorsGrp = new QWidget(colorTabWidget);
    otherTabIdx = colorTabWidget->addTab(colorsGrp, i18n("Other"));

    colorsGrid = new QGridLayout(colorsGrp);
    colorsGrid->setSpacing(0);
    colorsGrid->setContentsMargins(2, 2, 2, 2);

    offset = endOfPanelColors = static_cast<int>(itemList.count());

    addColorSelector("Quicksearch Match Foreground", i18n("Quicksearch, match foreground:"), Qt::black, QString(), &KDEDefaultFore, 1);
    addColorSelector("Quicksearch Match Background", i18n("Quicksearch, match background:"), QColor(192, 255, 192), QString(), &KDEDefaultBase, 1);
    addColorSelector("Quicksearch Non-match Foreground", i18n("Quicksearch, non-match foreground:"), Qt::black, QString(), &KDEDefaultFore, 1);
    addColorSelector("Quicksearch Non-match Background", i18n("Quicksearch, non-match background:"), QColor(255, 192, 192), QString(), &KDEDefaultBase, 1);
    ADDITIONAL_COLOR KDEDefaultWindowFore = {i18n("KDE default"), p.color(QPalette::Active, QPalette::WindowText), "KDE default"};
    ADDITIONAL_COLOR KDEDefaultWindowBack = {i18n("KDE default"), p.color(QPalette::Active, QPalette::Window), "KDE default"};
    addColorSelector("Statusbar Foreground Active",
                     i18n("Statusbar, active foreground:"),
                     p.color(QPalette::Active, QPalette::HighlightedText),
                     QString(),
                     &KDEDefaultWindowFore,
                     1);
    addColorSelector("Statusbar Background Active",
                     i18n("Statusbar, active background:"),
                     p.color(QPalette::Active, QPalette::Highlight),
                     QString(),
                     &KDEDefaultWindowBack,
                     1);
    addColorSelector("Statusbar Foreground Inactive",
                     i18n("Statusbar, inactive foreground:"),
                     p.color(QPalette::Inactive, QPalette::Text),
                     QString(),
                     &KDEDefaultWindowFore,
                     1);
    addColorSelector("Statusbar Background Inactive",
                     i18n("Statusbar, inactive background:"),
                     p.color(QPalette::Inactive, QPalette::Base),
                     QString(),
                     &KDEDefaultWindowBack,
                     1);

    colorsGrid->addWidget(createSpacer(colorsGrp), static_cast<int>(itemList.count()) - offset, 1);

    colorsFrameGrid->addWidget(colorTabWidget, 0, 0);
    hbox->addWidget(colorsFrameGrp);

    //  -------------------------- PREVIEW GROUPBOX ----------------------------------

    previewGrp = createFrame(i18n("Preview"), hboxWidget);
    previewGrid = createGridLayout(previewGrp);

    preview = new KrTreeWidget(previewGrp);
    preview->setBackgroundRole(QPalette::Window);
    preview->setAutoFillBackground(true);

    QStringList labels;
    labels << i18n("Colors");
    preview->setHeaderLabels(labels);

    preview->header()->setSortIndicatorShown(false);
    preview->setSortingEnabled(false);
    preview->setEnabled(false);

    previewGrid->addWidget(preview, 0, 0);
    hbox->addWidget(previewGrp);

    connect(generals->find("Enable Alternate Background"), &KonfiguratorCheckBox::stateChanged, this, &KgColors::generatePreview);
    connect(colorTabWidget, &QTabWidget::currentChanged, this, &KgColors::generatePreview);
    connect(dimFactor, QOverload<int>::of(&KonfiguratorSpinBox::valueChanged), this, &KgColors::generatePreview);

    kgColorsLayout->addWidget(hboxWidget, 1, 0, 1, 3);

    importBtn = new QPushButton(i18n("Import color-scheme"), innerWidget);
    kgColorsLayout->addWidget(importBtn, 2, 0);
    exportBtn = new QPushButton(i18n("Export color-scheme"), innerWidget);
    kgColorsLayout->addWidget(exportBtn, 2, 1);
    kgColorsLayout->addWidget(createSpacer(innerWidget), 2, 2);
    connect(importBtn, &QPushButton::clicked, this, &KgColors::slotImportColors);
    connect(exportBtn, &QPushButton::clicked, this, &KgColors::slotExportColors);

    slotDisable();
}

int KgColors::addColorSelector(const QString &cfgName, QString name, QColor defaultValue, const QString &dfltName, ADDITIONAL_COLOR *addColor, int addColNum)
{
    int index = static_cast<int>(itemList.count()) - offset;

    labelList.append(addLabel(colorsGrid, index, 0, std::move(name), colorsGrp));
    KonfiguratorColorChooser *chooser = createColorChooser("Colors", cfgName, std::move(defaultValue), colorsGrp, false, addColor, addColNum);
    chooser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    if (!dfltName.isEmpty())
        chooser->setDefaultText(dfltName);

    colorsGrid->addWidget(chooser, index, 1);

    connect(chooser, &KonfiguratorColorChooser::colorChanged, this, &KgColors::generatePreview);
    if (!offset)
        connect(chooser, &KonfiguratorColorChooser::colorChanged, this, &KgColors::slotActiveChanged);

    itemList.append(chooser);
    itemNames.append(cfgName);

    return index;
}

KonfiguratorColorChooser *KgColors::getColorSelector(const QString &name)
{
    QList<QString>::iterator it;
    int position = 0;

    for (it = itemNames.begin(); it != itemNames.end(); it++, position++)
        if (*it == name)
            return itemList.at(position);

    return nullptr;
}

QLabel *KgColors::getSelectorLabel(const QString &name)
{
    QList<QString>::iterator it;
    int position = 0;

    for (it = itemNames.begin(); it != itemNames.end(); it++, position++)
        if (*it == name)
            return labelList.at(position);

    return nullptr;
}

void KgColors::slotDisable()
{
    bool enabled = generals->find("KDE Default")->isChecked();

    importBtn->setEnabled(!enabled);
    exportBtn->setEnabled(!enabled);

    for (int i = 0; i < labelList.count() && i < endOfPanelColors; i++)
        labelList.at(i)->setEnabled(!enabled);

    for (int j = 0; j < itemList.count() && j < endOfPanelColors; j++)
        itemList.at(j)->setEnabled(!enabled);

    generals->find("Enable Alternate Background")->setEnabled(enabled);
    generals->find("Show Current Item Always")->setEnabled(!enabled);
    generals->find("Dim Inactive Colors")->setEnabled(!enabled);

    bool dimmed = !enabled && generals->find("Dim Inactive Colors")->isChecked();
    if (dimmed)
        inactiveColorStack->setCurrentWidget(dimmedInactiveWidget);
    else
        inactiveColorStack->setCurrentWidget(normalInactiveWidget);

    enabled = enabled || !generals->find("Show Current Item Always")->isChecked();

    getColorSelector("Inactive Current Foreground")->setEnabled(!enabled);
    getColorSelector("Inactive Current Background")->setEnabled(!enabled);

    generatePreview();
}

void KgColors::slotActiveChanged()
{
    for (int i = 0; i != endOfActiveColors; i++)
        itemList.at(endOfActiveColors + i)->setDefaultColor(itemList.at(i)->getColor());
}

void KgColors::slotForegroundChanged()
{
    QColor color = getColorSelector("Foreground")->getColor();

    getColorSelector("Directory Foreground")->setDefaultColor(color);
    getColorSelector("Executable Foreground")->setDefaultColor(color);
    getColorSelector("Symlink Foreground")->setDefaultColor(color);
    getColorSelector("Invalid Symlink Foreground")->setDefaultColor(color);
}

void KgColors::slotBackgroundChanged()
{
    QColor color = getColorSelector("Background")->getColor();

    getColorSelector("Alternate Background")->changeAdditionalColor(0, color);
    getColorSelector("Marked Background")->changeAdditionalColor(0, color);
    getColorSelector("Current Background")->changeAdditionalColor(0, color);
}

void KgColors::slotAltBackgroundChanged()
{
    QColor color = getColorSelector("Alternate Background")->getColor();
    getColorSelector("Alternate Marked Background")->changeAdditionalColor(0, color);
}

void KgColors::slotMarkedBackgroundChanged()
{
    QColor color = getColorSelector("Marked Background")->getColor();
    getColorSelector("Alternate Marked Background")->setDefaultColor(color);
}

void KgColors::slotInactiveForegroundChanged()
{
    QColor color = getColorSelector("Inactive Foreground")->getColor();

    getColorSelector("Inactive Directory Foreground")->changeAdditionalColor(0, color);
    getColorSelector("Inactive Executable Foreground")->changeAdditionalColor(0, color);
    getColorSelector("Inactive Symlink Foreground")->changeAdditionalColor(0, color);
    getColorSelector("Inactive Invalid Symlink Foreground")->changeAdditionalColor(0, color);
}

void KgColors::slotInactiveBackgroundChanged()
{
    QColor color = getColorSelector("Inactive Background")->getColor();

    getColorSelector("Inactive Alternate Background")->changeAdditionalColor(0, color);
    getColorSelector("Inactive Marked Background")->changeAdditionalColor(0, color);
    getColorSelector("Inactive Current Background")->changeAdditionalColor(0, color);
}

void KgColors::slotInactiveAltBackgroundChanged()
{
    QColor color = getColorSelector("Inactive Alternate Background")->getColor();
    getColorSelector("Inactive Alternate Marked Background")->changeAdditionalColor(0, color);
}

void KgColors::slotInactiveMarkedBackgroundChanged()
{
    QColor color = getColorSelector("Inactive Marked Background")->getColor();
    getColorSelector("Inactive Alternate Marked Background")->changeAdditionalColor(1, color);
}

void KgColors::setColorWithDimming(PreviewItem *item, QColor foreground, QColor background, bool dimmed)
{
    if (dimmed && dimFactor->value() < 100) {
        int dim = dimFactor->value();
        QColor dimColor = getColorSelector("Dim Target Color")->getColor();

        foreground = QColor((dimColor.red() * (100 - dim) + foreground.red() * dim) / 100,
                            (dimColor.green() * (100 - dim) + foreground.green() * dim) / 100,
                            (dimColor.blue() * (100 - dim) + foreground.blue() * dim) / 100);
        background = QColor((dimColor.red() * (100 - dim) + background.red() * dim) / 100,
                            (dimColor.green() * (100 - dim) + background.green() * dim) / 100,
                            (dimColor.blue() * (100 - dim) + background.blue() * dim) / 100);
    }
    item->setColor(foreground, background);
}

void KgColors::generatePreview()
{
    const int currentPage = colorTabWidget->currentIndex();

    preview->clear();

    if (currentPage == activeTabIdx || currentPage == inactiveTabIdx) {
        PreviewItem *pwMarkCur = new PreviewItem(preview, i18n("Selected + Current"));
        PreviewItem *pwMark2 = new PreviewItem(preview, i18n("Selected 2"));
        PreviewItem *pwMark1 = new PreviewItem(preview, i18n("Selected 1"));
        PreviewItem *pwCurrent = new PreviewItem(preview, i18n("Current"));
        PreviewItem *pwInvLink = new PreviewItem(preview, i18n("Invalid symlink"));
        PreviewItem *pwSymLink = new PreviewItem(preview, i18n("Symbolic link"));
        PreviewItem *pwApp = new PreviewItem(preview, i18n("Application"));
        PreviewItem *pwFile = new PreviewItem(preview, i18n("File"));
        PreviewItem *pwDir = new PreviewItem(preview, i18n("Folder"));

        bool isActive = currentPage == 0;

        // create local color cache instance, which does NOT affect the color cache instance using to paint the panels
        KrColorCache colCache;

        // create local color settings instance, which initially contains the settings from krConfig
        KrColorSettings colorSettings;

        // copy over local settings to color settings instance, which does not affect the persisted krConfig settings
        QList<QString> names = KrColorSettings::getColorNames();
        for (auto &name : names) {
            KonfiguratorColorChooser *chooser = getColorSelector(name);
            if (!chooser)
                continue;
            colorSettings.setColorTextValue(name, chooser->getValue());
            if (chooser->isValueRGB())
                colorSettings.setColorValue(name, chooser->getColor());
            else
                colorSettings.setColorValue(name, QColor());
        }

        colorSettings.setBoolValue("KDE Default", generals->find("KDE Default")->isChecked());
        colorSettings.setBoolValue("Enable Alternate Background", generals->find("Enable Alternate Background")->isChecked());
        colorSettings.setBoolValue("Show Current Item Always", generals->find("Show Current Item Always")->isChecked());
        colorSettings.setBoolValue("Dim Inactive Colors", generals->find("Dim Inactive Colors")->isChecked());
        colorSettings.setNumValue("Dim Factor", dimFactor->value());

        // let the color cache use the local color settings
        colCache.setColors(colorSettings);

        // ask the local color cache for certain color groups and use them to color the preview
        KrColorGroup cg;

        // setting the background color
        colCache.getColors(cg, KrColorItemType(KrColorItemType::File, false, isActive, false, false));
        QPalette pal = preview->palette();
        pal.setColor(QPalette::Base, cg.background());
        preview->setPalette(pal);

        colCache.getColors(cg, KrColorItemType(KrColorItemType::Directory, false, isActive, false, false));
        pwDir->setColor(cg.text(), cg.background());
        colCache.getColors(cg, KrColorItemType(KrColorItemType::File, true, isActive, false, false));
        pwFile->setColor(cg.text(), cg.background());
        colCache.getColors(cg, KrColorItemType(KrColorItemType::Executable, false, isActive, false, false));
        pwApp->setColor(cg.text(), cg.background());
        colCache.getColors(cg, KrColorItemType(KrColorItemType::Symlink, true, isActive, false, false));
        pwSymLink->setColor(cg.text(), cg.background());
        colCache.getColors(cg, KrColorItemType(KrColorItemType::InvalidSymlink, false, isActive, false, false));
        pwInvLink->setColor(cg.text(), cg.background());
        colCache.getColors(cg, KrColorItemType(KrColorItemType::File, true, isActive, true, false));
        pwCurrent->setColor(cg.text(), cg.background());
        colCache.getColors(cg, KrColorItemType(KrColorItemType::File, false, isActive, false, true));
        pwMark1->setColor(cg.highlightedText(), cg.highlight());
        colCache.getColors(cg, KrColorItemType(KrColorItemType::File, true, isActive, false, true));
        pwMark2->setColor(cg.highlightedText(), cg.highlight());
        colCache.getColors(cg, KrColorItemType(KrColorItemType::File, false, isActive, true, true));
        pwMarkCur->setColor(cg.highlightedText(), cg.highlight());
    }
#ifdef SYNCHRONIZER_ENABLED
    else if (currentPage == synchronizerTabIdx) {
        PreviewItem *pwDelete = new PreviewItem(preview, i18n("Delete"));
        PreviewItem *pwRightCopy = new PreviewItem(preview, i18n("Copy to right"));
        PreviewItem *pwLeftCopy = new PreviewItem(preview, i18n("Copy to left"));
        PreviewItem *pwDiffers = new PreviewItem(preview, i18n("Differing"));
        PreviewItem *pwEquals = new PreviewItem(preview, i18n("Equals"));

        pwEquals->setColor(getColorSelector("Synchronizer Equals Foreground")->getColor(), getColorSelector("Synchronizer Equals Background")->getColor());
        pwDiffers->setColor(getColorSelector("Synchronizer Differs Foreground")->getColor(), getColorSelector("Synchronizer Differs Background")->getColor());
        pwLeftCopy->setColor(getColorSelector("Synchronizer LeftCopy Foreground")->getColor(),
                             getColorSelector("Synchronizer LeftCopy Background")->getColor());
        pwRightCopy->setColor(getColorSelector("Synchronizer RightCopy Foreground")->getColor(),
                              getColorSelector("Synchronizer RightCopy Background")->getColor());
        pwDelete->setColor(getColorSelector("Synchronizer Delete Foreground")->getColor(), getColorSelector("Synchronizer Delete Background")->getColor());
    }
#endif
    else if (currentPage == otherTabIdx) {
        PreviewItem *pwNonMatch = new PreviewItem(preview, i18n("Quicksearch non-match"));
        PreviewItem *pwMatch = new PreviewItem(preview, i18n("Quicksearch match"));
        pwMatch->setColor(getColorSelector("Quicksearch Match Foreground")->getColor(), getColorSelector("Quicksearch Match Background")->getColor());
        pwNonMatch->setColor(getColorSelector("Quicksearch Non-match Foreground")->getColor(),
                             getColorSelector("Quicksearch Non-match Background")->getColor());
        PreviewItem *pwStatusActive = new PreviewItem(preview, i18n("Statusbar active"));
        PreviewItem *pwStatusInactive = new PreviewItem(preview, i18n("Statusbar inactive"));
        pwStatusActive->setColor(getColorSelector("Statusbar Foreground Active")->getColor(), getColorSelector("Statusbar Background Active")->getColor());
        pwStatusInactive->setColor(getColorSelector("Statusbar Foreground Inactive")->getColor(),
                                   getColorSelector("Statusbar Background Inactive")->getColor());
    }
}

bool KgColors::apply()
{
    bool result = KonfiguratorPage::apply();
    KrColorCache::getColorCache().refreshColors();
    return result;
}

void KgColors::slotImportColors()
{
    // find $KDEDIR/share/apps/krusader
    QString basedir = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("total_commander.keymap"));
    basedir = QFileInfo(basedir).absolutePath();
    // let the user select a file to load
    QString file = QFileDialog::getOpenFileName(nullptr, i18n("Select a color-scheme file"), basedir, QStringLiteral("*.color"));
    if (file.isEmpty()) {
        return;
    }
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)) {
        KMessageBox::error(this, i18n("Error: unable to read from file"), i18n("Error"));
        return;
    }
    QDataStream stream(&f);
    // ok, import away
    deserialize(stream);
    generatePreview();
}

void KgColors::slotExportColors()
{
    QString file = QFileDialog::getSaveFileName(nullptr, i18n("Select a color scheme file"), QString(), QStringLiteral("*"));
    if (file.isEmpty()) {
        return;
    }
    QFile f(file);
    if (f.exists()
        && KMessageBox::warningContinueCancel(this,
                                              i18n("File %1 already exists. Are you sure you want to overwrite it?", file),
                                              i18n("Warning"),
                                              KStandardGuiItem::overwrite())
            != KMessageBox::Continue)
        return;
    if (!f.open(QIODevice::WriteOnly)) {
        KMessageBox::error(this, i18n("Error: unable to write to file"), i18n("Error"));
        return;
    }
    QDataStream stream(&f);
    serialize(stream);
}

void KgColors::serialize(QDataStream &stream)
{
    serializeItem(stream, "Alternate Background");
    serializeItem(stream, "Alternate Marked Background");
    serializeItem(stream, "Background");
    serializeItem(stream, "Current Background");
    serializeItem(stream, "Current Foreground");
    serializeItem(stream, "Enable Alternate Background");
    serializeItem(stream, "Foreground");
    serializeItem(stream, "Directory Foreground");
    serializeItem(stream, "Executable Foreground");
    serializeItem(stream, "Symlink Foreground");
    serializeItem(stream, "Invalid Symlink Foreground");
    serializeItem(stream, "Inactive Alternate Background");
    serializeItem(stream, "Inactive Alternate Marked Background");
    serializeItem(stream, "Inactive Background");
    serializeItem(stream, "Inactive Current Foreground");
    serializeItem(stream, "Inactive Current Background");
    serializeItem(stream, "Inactive Marked Background");
    serializeItem(stream, "Inactive Marked Current Foreground");
    serializeItem(stream, "Inactive Marked Foreground");
    serializeItem(stream, "Inactive Foreground");
    serializeItem(stream, "Inactive Directory Foreground");
    serializeItem(stream, "Inactive Executable Foreground");
    serializeItem(stream, "Inactive Symlink Foreground");
    serializeItem(stream, "Inactive Invalid Symlink Foreground");
    serializeItem(stream, "Dim Inactive Colors");
    serializeItem(stream, "Dim Target Color");
    serializeItem(stream, "Dim Factor");
    serializeItem(stream, "KDE Default");
    serializeItem(stream, "Marked Background");
    serializeItem(stream, "Marked Current Foreground");
    serializeItem(stream, "Marked Foreground");
    serializeItem(stream, "Show Current Item Always");
#ifdef SYNCHRONIZER_ENABLED
    serializeItem(stream, "Synchronizer Equals Foreground");
    serializeItem(stream, "Synchronizer Equals Background");
    serializeItem(stream, "Synchronizer Differs Foreground");
    serializeItem(stream, "Synchronizer Differs Background");
    serializeItem(stream, "Synchronizer LeftCopy Foreground");
    serializeItem(stream, "Synchronizer LeftCopy Background");
    serializeItem(stream, "Synchronizer RightCopy Foreground");
    serializeItem(stream, "Synchronizer RightCopy Background");
    serializeItem(stream, "Synchronizer Delete Foreground");
    serializeItem(stream, "Synchronizer Delete Background");
#endif
    serializeItem(stream, "Quicksearch Match Foreground");
    serializeItem(stream, "Quicksearch Match Background");
    serializeItem(stream, "Quicksearch Non-match Foreground");
    serializeItem(stream, "Quicksearch Non-match Background");
    serializeItem(stream, "Statusbar Foreground Active");
    serializeItem(stream, "Statusbar Background Active");
    serializeItem(stream, "Statusbar Foreground Inactive");
    serializeItem(stream, "Statusbar Background Inactive");
    stream << QString("") << QString("");
}

void KgColors::deserialize(QDataStream &stream)
{
    for (;;) {
        QString name;
        QString value;
        stream >> name >> value;
        if (name.isEmpty())
            break;

        if (name == "KDE Default" || name == "Enable Alternate Background" || name == "Show Current Item Always" || name == "Dim Inactive Colors") {
            bool bValue = false;
            value = value.toLower();
            if (value == "true" || value == "yes" || value == "on" || value == "1")
                bValue = true;

            generals->find(name)->setChecked(bValue);
            continue;
        }

        if (name == "Dim Factor") {
            dimFactor->setValue(value.toInt());
            continue;
        }

        KonfiguratorColorChooser *selector = getColorSelector(name);
        if (selector == nullptr)
            break;
        selector->setValue(value);
    }
}

void KgColors::serializeItem(class QDataStream &stream, const char *name)
{
    stream << QString(name);
    if (strcmp(name, "KDE Default") == 0 || strcmp(name, "Enable Alternate Background") == 0 || strcmp(name, "Show Current Item Always") == 0
        || strcmp(name, "Dim Inactive Colors") == 0) {
        bool bValue = generals->find(name)->isChecked();
        stream << QString(bValue ? "true" : "false");
    } else if (strcmp(name, "Dim Factor") == 0)
        stream << QString::number(dimFactor->value());
    else {
        KonfiguratorColorChooser *selector = getColorSelector(name);
        stream << selector->getValue();
    }
}
