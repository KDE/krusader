/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpanel.h"
#include "../Dialogs/krdialogs.h"
#include "../defaults.h"
#include "../krglobal.h"

// QtGui
#include <QValidator>
// QtWidgets
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <QtWidgets/QInputDialog>

#include "../GUI/krtreewidget.h"
#include "../Panel/PanelView/krselectionmode.h"
#include "../Panel/PanelView/krview.h"
#include "../Panel/PanelView/krviewfactory.h"
#include "../Panel/krlayoutfactory.h"
#include "../Panel/krsearchbar.h"
#include "../icon.h"
#include "../krglobal.h"

enum { PAGE_GENERAL = 0, PAGE_VIEW, PAGE_PANELTOOLBAR, PAGE_MOUSE, PAGE_MEDIA_MENU, PAGE_LAYOUT };

KgPanel::KgPanel(bool first, QWidget *parent)
    : KonfiguratorPage(first, parent)
{
    tabWidget = new QTabWidget(this);
    setWidget(tabWidget);
    setWidgetResizable(true);

    setupGeneralTab();
    setupPanelTab();
    setupButtonsTab();
    setupMouseModeTab();
    setupMediaMenuTab();
    setupLayoutTab();
}

// ---------------------------------------------------------------------------------------
// ---------------------------- General TAB ----------------------------------------------
// ---------------------------------------------------------------------------------------
void KgPanel::setupGeneralTab()
{
    auto *scrollArea = new QScrollArea(tabWidget);
    QWidget *tab = new QWidget(scrollArea);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setWidget(tab);
    scrollArea->setWidgetResizable(true);
    tabWidget->addTab(scrollArea, i18n("General"));

    auto *layout = new QVBoxLayout(tab);
    layout->setSpacing(6);
    layout->setContentsMargins(11, 11, 11, 11);

    // ---------------------------------------------------------------------------------------
    // ------------------------------- Navigator bar -------------------------------------
    // ---------------------------------------------------------------------------------------
    QGroupBox *groupBox = createFrame(i18n("Navigator bar"), tab);
    QGridLayout *gridLayout = createGridLayout(groupBox);

    KONFIGURATOR_CHECKBOX_PARAM navigatorbar_settings[] = {
        // cfg_class, cfg_name, default, text, restart, tooltip
        {"Look&Feel", "Navigator Edit Mode", false, i18n("Edit Mode by default"), true, i18n("Show editable path in Navigator bar by default")},
        {"Look&Feel", "Navigator Full Path", false, i18n("Show full path by default"), true, i18n("Always show full path in Navigator bar by default.")},
    };
    KonfiguratorCheckBoxGroup *cbs = createCheckBoxGroup(2, 0, navigatorbar_settings, 2 /*count*/, groupBox, PAGE_GENERAL);
    gridLayout->addWidget(cbs, 0, 0);

    layout->addWidget(groupBox);

    // ---------------------------------------------------------------------------------------
    // ------------------------------- Operation ---------------------------------------------
    // ---------------------------------------------------------------------------------------
    groupBox = createFrame(i18n("Operation"), tab);
    gridLayout = createGridLayout(groupBox);

    KONFIGURATOR_CHECKBOX_PARAM operation_settings[] = {
        // cfg_class, cfg_name, default, text, restart, tooltip
        {"Look&Feel",
         "Mark Dirs",
         _MarkDirs,
         i18n("Autoselect folders"),
         false,
         i18n("When matching the select criteria, not only files will be selected, but also folders.")},
        {"Look&Feel",
         "Rename Selects Extension",
         true,
         i18n("Rename selects extension"),
         false,
         i18n("When renaming a file, the whole text is selected. If you want Total-Commander like renaming of just the name, without extension, uncheck this "
              "option.")},
        {"Look&Feel",
         "UnselectBeforeOperation",
         _UnselectBeforeOperation,
         i18n("Unselect files before copy/move"),
         false,
         i18n("Unselect files, which are to be copied/moved, before the operation starts.")},
        {"Look&Feel",
         "FilterDialogRemembersSettings",
         _FilterDialogRemembersSettings,
         i18n("Filter dialog remembers settings"),
         false,
         i18n("The filter dialog is opened with the last filter settings that where applied to the panel.")},
    };
    cbs = createCheckBoxGroup(2, 0, operation_settings, 4 /*count*/, groupBox, PAGE_GENERAL);
    gridLayout->addWidget(cbs, 0, 0);

    layout->addWidget(groupBox);

    // ---------------------------------------------------------------------------------------
    // ------------------------------ Tabs ---------------------------------------------------
    // ---------------------------------------------------------------------------------------
    groupBox = createFrame(i18n("Tabs"), tab);
    gridLayout = createGridLayout(groupBox);

    KONFIGURATOR_CHECKBOX_PARAM tabbar_settings[] = {
        //   cfg_class    cfg_name                      default             text                                     restart tooltip
        {"Look&Feel",
         "Fullpath Tab Names",
         _FullPathTabNames,
         i18n("Use full path tab names"),
         true,
         i18n("Display the full path in the folder tabs. By default only the last part of the path is displayed.")},
        {"Look&Feel", "Show Close Tab Buttons", true, i18n("Show close tab buttons"), true, i18n("Show close tab buttons.")},
        {"Look&Feel", "Expanding Tabs", true, i18n("Expanding tabs"), true, i18n("Expanding tabs.")},
        {"Look&Feel", "Show New Tab Button", true, i18n("Show new tab button"), true, i18n("Show new tab button.")},
        {"Look&Feel", "Insert Tabs After Current", false, i18n("Insert tabs after current"), false, i18n("Insert new tabs to the right of the current one.")},
        {"Look&Feel", "Show Tab Bar On Single Tab", true, i18n("Show Tab Bar on single tab"), true, i18n("Show the tab bar with only one tab.")},
        {"Look&Feel", "Close Tab By Double Click", false, i18n("Close tab by double click"), false, i18n("Close tab by double click.")},
        {"Look&Feel",
         "New Tab Button Duplicates",
         false,
         i18n("New tab button duplicates a tab"),
         false,
         i18n("If checked, new tab button duplicates a tab, otherwise it opens a tab in home directory.")}};
    cbs = createCheckBoxGroup(2, 0, tabbar_settings, 8 /*count*/, groupBox, PAGE_GENERAL);
    gridLayout->addWidget(cbs, 0, 0, 1, 2);

    // -------------- Duplicate tab by click --------------------
    QHBoxLayout *hbox = new QHBoxLayout();

    QLabel *labelDupTabs = new QLabel(i18n("Duplicate active tab by click:"), groupBox);
    hbox->addWidget(labelDupTabs);

    KONFIGURATOR_NAME_VALUE_PAIR duptabs[] = {{i18n("Disabled"), "disabled"}, {i18n("Alt+Click"), "alt_click"}, {i18n("Ctrl+Click"), "ctrl_click"}};

    KonfiguratorComboBox *cmb = createComboBox("Look&Feel",
                                               "Duplicate Tab Click",
                                               "disabled",
                                               duptabs,
                                               3,
                                               labelDupTabs,
                                               groupBox,
                                               true,
                                               false,
                                               i18n("Duplicate active tab by clicking Ctrl/Alt+LMB"),
                                               PAGE_GENERAL);

    hbox->addWidget(cmb);
    hbox->addWidget(createSpacer(groupBox));
    gridLayout->addLayout(hbox, 1, 0);

    // -------------- Tab Bar position --------------------------
    hbox = new QHBoxLayout();

    QLabel *labelTabBar = new QLabel(i18n("Tab Bar position:"), groupBox);
    hbox->addWidget(labelTabBar);

    KONFIGURATOR_NAME_VALUE_PAIR positions[] = {{i18n("Top"), "top"}, {i18n("Bottom"), "bottom"}};

    cmb = createComboBox("Look&Feel", "Tab Bar Position", _TabBarPosition, positions, 2, labelTabBar, groupBox, true, false, QString(), PAGE_GENERAL);

    hbox->addWidget(cmb);
    gridLayout->addLayout(hbox, 2, 0, Qt::AlignLeft);

    layout->addWidget(groupBox);

    // ---------------------------------------------------------------------------------------
    // -----------------------------  Search bar  --------------------------------------------
    // ---------------------------------------------------------------------------------------
    groupBox = createFrame(i18n("Search bar"), tab);
    gridLayout = createGridLayout(groupBox);

    KONFIGURATOR_CHECKBOX_PARAM quicksearch[] = {
        //   cfg_class  cfg_name                default             text                              restart tooltip
        {"Look&Feel",
         "New Style Quicksearch",
         _NewStyleQuicksearch,
         i18n("Start by typing"),
         false,
         i18n("Open search bar and start searching by typing in panel.")},
        {"Look&Feel", "Case Sensitive Quicksearch", _CaseSensitiveQuicksearch, i18n("Case sensitive"), false, i18n("Search must match case.")},
        {"Look&Feel",
         "Up/Down Cancels Quicksearch",
         false,
         i18n("Up/Down cancels search"),
         false,
         i18n("Pressing the Up/Down buttons closes the search bar (only in search mode).")},
        {"Look&Feel",
         "Navigation with Right Arrow Quicksearch",
         _NavigationWithRightArrowQuicksearch,
         i18n("Folder navigation with Right Arrow"),
         false,
         i18n("Pressing the Right button enters folder if no search text editing intention is captured.")},
    };

    cbs = createCheckBoxGroup(2, 0, quicksearch, 4 /*count*/, groupBox, PAGE_GENERAL);
    gridLayout->addWidget(cbs, 0, 0, 1, -1);

    // -------------- Search bar position -----------------------

    hbox = new QHBoxLayout();
    QLabel *labelPosit = new QLabel(i18n("Position:"), groupBox);
    hbox->addWidget(labelPosit);
    cmb = createComboBox("Look&Feel", "Quicksearch Position", "bottom", positions, 2, labelPosit, groupBox, true, false, QString(), PAGE_GENERAL);
    hbox->addWidget(cmb);
    hbox->addWidget(createSpacer(groupBox));
    gridLayout->addLayout(hbox, 1, 0);

    // -------------- Default search mode -----------------------

    hbox = new QHBoxLayout();
    QLabel *labelMode = new QLabel(i18n("Default mode:"), groupBox);
    hbox->addWidget(labelMode);
    KONFIGURATOR_NAME_VALUE_PAIR modes[] = {{i18n("Search"), QString::number(KrSearchBar::MODE_SEARCH)},
                                            {i18n("Select"), QString::number(KrSearchBar::MODE_SELECT)},
                                            {i18n("Filter"), QString::number(KrSearchBar::MODE_FILTER)}};
    cmb = createComboBox("Look&Feel",
                         "Default Search Mode",
                         QString::number(KrSearchBar::MODE_SEARCH),
                         modes,
                         3,
                         labelMode,
                         groupBox,
                         true,
                         false,
                         i18n("Set the default mode on first usage"),
                         PAGE_GENERAL);
    hbox->addWidget(cmb);
    hbox->addWidget(createSpacer(groupBox));
    gridLayout->addLayout(hbox, 1, 1);

    layout->addWidget(groupBox);

    // --------------------------------------------------------------------------------------------
    // ------------------------------- Bookmark search settings ----------------------------------
    // --------------------------------------------------------------------------------------------
    groupBox = createFrame(i18n("Bookmark Search"), tab);
    gridLayout = createGridLayout(groupBox);

    KONFIGURATOR_CHECKBOX_PARAM bookmarkSearchSettings[] = {
        {"Look&Feel", "Always show search bar", true, i18n("Always show search bar"), false, i18n("Make bookmark search bar always visible")},
        {"Look&Feel",
         "Search in special items",
         false,
         i18n("Search in special items"),
         false,
         i18n("Bookmark search is also applied to special items in bookmark menu like Trash, Popular URLs, Jump Back, etc.")},
    };
    KonfiguratorCheckBoxGroup *bookmarkSearchSettingsGroup = createCheckBoxGroup(2, 0, bookmarkSearchSettings, 2 /*count*/, groupBox, PAGE_GENERAL);
    gridLayout->addWidget(bookmarkSearchSettingsGroup, 1, 0, 1, 2);
    layout->addWidget(groupBox);

    // --------------------------------------------------------------------------------------------
    // ------------------------------- Status/Totalsbar settings ----------------------------------
    // --------------------------------------------------------------------------------------------
    groupBox = createFrame(i18n("Status/Totalsbar"), tab);
    gridLayout = createGridLayout(groupBox);

    KONFIGURATOR_CHECKBOX_PARAM barSettings[] = {
        {"Look&Feel", "Show Size In Bytes", false, i18n("Show size in bytes too"), true, i18n("Show size in bytes too")},
        {"Look&Feel", "ShowSpaceInformation", true, i18n("Show space information"), true, i18n("Show free/total space on the device")},
    };
    KonfiguratorCheckBoxGroup *barSett = createCheckBoxGroup(2, 0, barSettings, 2 /*count*/, groupBox, PAGE_GENERAL);
    gridLayout->addWidget(barSett, 1, 0, 1, 2);

    layout->addWidget(groupBox);
}

// --------------------------------------------------------------------------------------------
// ------------------------------------ Layout Tab --------------------------------------------
// --------------------------------------------------------------------------------------------
void KgPanel::setupLayoutTab()
{
    auto *scrollArea = new QScrollArea(tabWidget);
    QWidget *tab = new QWidget(scrollArea);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setWidget(tab);
    scrollArea->setWidgetResizable(true);
    tabWidget->addTab(scrollArea, i18n("Layout"));

    QGridLayout *grid = createGridLayout(tab);

    QStringList layoutNames = KrLayoutFactory::layoutNames();
    int numLayouts = static_cast<int>(layoutNames.count());

    grid->addWidget(createSpacer(tab), 0, 2);

    QLabel *l = new QLabel(i18n("Layout:"), tab);
    l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(l, 0, 0);
    auto *layouts = new KONFIGURATOR_NAME_VALUE_PAIR[numLayouts];
    for (int i = 0; i != numLayouts; i++) {
        layouts[i].text = KrLayoutFactory::layoutDescription(layoutNames[i]);
        layouts[i].value = layoutNames[i];
    }
    KonfiguratorComboBox *cmb = createComboBox("PanelLayout", "Layout", "default", layouts, numLayouts, l, tab, true, false, QString(), PAGE_LAYOUT);
    grid->addWidget(cmb, 0, 1);
    delete[] layouts;

    l = new QLabel(i18n("Frame Color:"), tab);
    l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(l, 1, 0);
    KONFIGURATOR_NAME_VALUE_PAIR frameColor[] = {{i18nc("Frame color", "Defined by Layout"), "default"},
                                                 {i18nc("Frame color", "None"), "none"},
                                                 {i18nc("Frame color", "Statusbar"), "Statusbar"}};
    cmb = createComboBox("PanelLayout", "FrameColor", "default", frameColor, 3, l, tab, true, false, QString(), PAGE_LAYOUT);
    grid->addWidget(cmb, 1, 1);

    l = new QLabel(i18n("Frame Shape:"), tab);
    l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(l, 2, 0);
    KONFIGURATOR_NAME_VALUE_PAIR frameShape[] = {
        {i18nc("Frame shape", "Defined by Layout"), "default"},
        {i18nc("Frame shape", "None"), "NoFrame"},
        {i18nc("Frame shape", "Box"), "Box"},
        {i18nc("Frame shape", "Panel"), "Panel"},
    };
    cmb = createComboBox("PanelLayout", "FrameShape", "default", frameShape, 4, l, tab, true, false, QString(), PAGE_LAYOUT);
    grid->addWidget(cmb, 2, 1);

    l = new QLabel(i18n("Frame Shadow:"), tab);
    l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(l, 3, 0);
    KONFIGURATOR_NAME_VALUE_PAIR frameShadow[] = {
        {i18nc("Frame shadow", "Defined by Layout"), "default"},
        {i18nc("Frame shadow", "None"), "Plain"},
        {i18nc("Frame shadow", "Raised"), "Raised"},
        {i18nc("Frame shadow", "Sunken"), "Sunken"},
    };
    cmb = createComboBox("PanelLayout", "FrameShadow", "default", frameShadow, 4, l, tab, true, false, QString(), PAGE_LAYOUT);
    grid->addWidget(cmb, 3, 1);
}

void KgPanel::setupView(KrViewInstance *instance, QWidget *parent)
{
    QGridLayout *grid = createGridLayout(parent);

    // -------------------- Filelist icon size ----------------------------------
    auto *hbox = new QHBoxLayout();

    QLabel *labelIconSize = new QLabel(i18n("Default icon size:"), parent);
    hbox->addWidget(labelIconSize);

    auto *iconSizes = new KONFIGURATOR_NAME_VALUE_PAIR[KrView::iconSizes.count()];
    for (int i = 0; i < KrView::iconSizes.count(); i++)
        iconSizes[i].text = iconSizes[i].value = QString::number(KrView::iconSizes[i]);
    KonfiguratorComboBox *cmb = createComboBox(instance->name(),
                                               "IconSize",
                                               _FilelistIconSize,
                                               iconSizes,
                                               static_cast<int>(KrView::iconSizes.count()),
                                               labelIconSize,
                                               parent,
                                               true,
                                               true,
                                               QString(),
                                               PAGE_VIEW);
    delete[] iconSizes;
    cmb->lineEdit()->setValidator(new QRegularExpressionValidator(QRegularExpression("[1-9]\\d{0,1}"), cmb));
    hbox->addWidget(cmb);

    hbox->addWidget(createSpacer(parent));

    grid->addLayout(hbox, 1, 0);

    //--------------------------------------------------------------------
    KONFIGURATOR_CHECKBOX_PARAM iconSettings[] =
        //   cfg_class             cfg_name        default       text            restart        tooltip
        {
            {instance->name(), "With Icons", _WithIcons, i18n("Use icons in the filenames"), true, i18n("Show the icons for filenames and folders.")},
            {instance->name(), "ShowPreviews", false, i18n("Show previews by default"), false, i18n("Show previews of files and folders.")},
        };

    KonfiguratorCheckBoxGroup *iconSett = createCheckBoxGroup(2, 0, iconSettings, 2 /*count*/, parent, PAGE_VIEW);

    grid->addWidget(iconSett, 2, 0, 1, 2);
}

// ----------------------------------------------------------------------------------
//  ---------------------------- VIEW TAB -------------------------------------------
// ----------------------------------------------------------------------------------
void KgPanel::setupPanelTab()
{
    auto *scrollArea = new QScrollArea(tabWidget);
    QWidget *tab_panel = new QWidget(scrollArea);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setWidget(tab_panel);
    scrollArea->setWidgetResizable(true);
    tabWidget->addTab(scrollArea, i18n("View"));

    auto *panelLayout = new QGridLayout(tab_panel);
    panelLayout->setSpacing(6);
    panelLayout->setContentsMargins(11, 11, 11, 11);
    QGroupBox *panelGrp = createFrame(i18n("General"), tab_panel);
    panelLayout->addWidget(panelGrp, 0, 0);
    QGridLayout *panelGrid = createGridLayout(panelGrp);

    // ----------------------------------------------------------------------------------
    //  ---------------------------- General settings -----------------------------------
    // ----------------------------------------------------------------------------------

    // -------------------- Panel Font ----------------------------------
    auto *hbox = new QHBoxLayout();

    auto *fontLayout = new QHBoxLayout();
    QLabel *labelFontLay = new QLabel(i18n("View font:"), panelGrp);
    fontLayout->addWidget(labelFontLay);
    KonfiguratorFontChooser *chsr = createFontChooser("Look&Feel", "Filelist Font", _FilelistFont, labelFontLay, panelGrp, true, QString(), PAGE_VIEW);
    fontLayout->addWidget(chsr);
    fontLayout->addStretch(1);
    hbox->addLayout(fontLayout, 1);

    // -------------------- Panel Tooltip ----------------------------------
    auto *tooltipLayout = new QHBoxLayout();
    const QString delayTip = i18n(
        "The duration after a tooltip is shown for a file item, in "
        "milliseconds. Set a negative value to disable tooltips.");
    QLabel *tooltipLabel = new QLabel(i18n("Tooltip delay (msec):"));
    tooltipLayout->addWidget(tooltipLabel);
    KonfiguratorSpinBox *tooltipSpinBox =
        createSpinBox("Look&Feel", "Panel Tooltip Delay", 1000, -100, 5000, tooltipLabel, panelGrp, false, delayTip, PAGE_VIEW);
    tooltipSpinBox->setSingleStep(100);
    tooltipLayout->addWidget(tooltipSpinBox);
    tooltipLayout->addStretch(1);
    hbox->addLayout(tooltipLayout, 1);

    panelGrid->addLayout(hbox, 1, 0);

    // -------------------- General options ----------------------------------
    KONFIGURATOR_CHECKBOX_PARAM panelSettings[] =
        //   cfg_class  cfg_name                default text                                  restart tooltip
        {
            {"Look&Feel",
             "Human Readable Size",
             _HumanReadableSize,
             i18n("Use human-readable file size"),
             true,
             i18n("File sizes are displayed in B, KB, MB and GB, not just in bytes.")},
            {"Look&Feel", "Show Hidden", _ShowHidden, i18n("Show hidden files"), false, i18n("Display files beginning with a dot.")},
            {"Look&Feel",
             "Numeric permissions",
             _NumericPermissions,
             i18n("Numeric Permissions"),
             true,
             i18n("Show octal numbers (0755) instead of the standard permissions (rwxr-xr-x) in the permission column.")},
            {"Look&Feel",
             "Load User Defined Folder Icons",
             _UserDefinedFolderIcons,
             i18n("Load the user defined folder icons"),
             true,
             i18n("Load the user defined folder icons (can cause decrease in performance).")},
            {"Look&Feel",
             "Always Show Current Item",
             _AlwaysShowCurrentItem,
             i18n("Always show current item"),
             false,
             i18n("Show current item border decoration in inactive panel.")},
        };

    KonfiguratorCheckBoxGroup *panelSett = createCheckBoxGroup(2, 0, panelSettings, 5 /*count*/, panelGrp, PAGE_VIEW);

    panelGrid->addWidget(panelSett, 3, 0, 1, 2);

    // =========================================================
    panelGrid->addWidget(createLine(panelGrp), 4, 0);

    // ------------------------ Sort Method ----------------------------------

    hbox = new QHBoxLayout();

    QLabel *labelSort = new QLabel(i18n("Sort method:"), panelGrp);
    hbox->addWidget(labelSort);

    KONFIGURATOR_NAME_VALUE_PAIR sortMethods[] = {{i18n("Alphabetical"), QString::number(KrViewProperties::Alphabetical)},
                                                  {i18n("Alphabetical and numbers"), QString::number(KrViewProperties::AlphabeticalNumbers)},
                                                  {i18n("Character code"), QString::number(KrViewProperties::CharacterCode)},
                                                  {i18n("Character code and numbers"), QString::number(KrViewProperties::CharacterCodeNumbers)},
                                                  {i18nc("Krusader sort", "Krusader"), QString::number(KrViewProperties::Krusader)}};
    KonfiguratorComboBox *cmb =
        createComboBox("Look&Feel", "Sort method", QString::number(_DefaultSortMethod), sortMethods, 5, labelSort, panelGrp, true, false, QString(), PAGE_VIEW);
    hbox->addWidget(cmb);
    hbox->addWidget(createSpacer(panelGrp));

    panelGrid->addLayout(hbox, 5, 0);

    // ------------------------ Sort Options ----------------------------------
    KONFIGURATOR_CHECKBOX_PARAM sortSettings[] =
        // cfg_class, cfg_name, default, text, restart, tooltip
        {
            {"Look&Feel",
             "Case Sensative Sort",
             _CaseSensativeSort,
             i18n("Case sensitive sorting"),
             true,
             i18n("All files beginning with capital letters appear before files beginning with non-capital letters (UNIX default).")},
            {"Look&Feel", "Show Directories First", true, i18n("Show folders first"), true, nullptr},
            {"Look&Feel",
             "Always sort dirs by name",
             false,
             i18n("Always sort dirs by name"),
             true,
             i18n("Folders are sorted by name, regardless of the sort column.")},
            {"Look&Feel",
             "Locale Aware Sort",
             true,
             i18n("Locale aware sorting"),
             true,
             i18n("The sorting is performed in a locale- and also platform-dependent manner. Can be slow.")},
        };

    KonfiguratorCheckBoxGroup *sortSett = createCheckBoxGroup(2, 0, sortSettings, 4 /*count*/, panelGrp, PAGE_VIEW);
    sortSett->find("Show Directories First")->addDep(sortSett->find("Always sort dirs by name"));

    panelGrid->addWidget(sortSett, 6, 0, 1, 2);

    // ----------------------------------------------------------------------------------
    //  ---------------------------- View modes -----------------------------------------
    // ----------------------------------------------------------------------------------

    panelGrp = createFrame(i18n("View modes"), tab_panel);
    panelLayout->addWidget(panelGrp, 1, 0);
    panelGrid = createGridLayout(panelGrp);

    // -------------------- Default Panel Type ----------------------------------
    hbox = new QHBoxLayout();

    QLabel *labelViewMode = new QLabel(i18n("Default view mode:"), panelGrp);
    hbox->addWidget(labelViewMode);

    QList<KrViewInstance *> views = KrViewFactory::registeredViews();
    const int viewsSize = static_cast<int>(views.size());
    auto *panelTypes = new KONFIGURATOR_NAME_VALUE_PAIR[viewsSize];

    QString defType = QString('0');

    for (int i = 0; i != viewsSize; i++) {
        KrViewInstance *inst = views[i];
        panelTypes[i].text = inst->description();
        panelTypes[i].text.remove('&');
        panelTypes[i].value = QString("%1").arg(inst->id());
        if (inst->id() == KrViewFactory::defaultViewId())
            defType = QString("%1").arg(inst->id());
    }

    cmb = createComboBox("Look&Feel", "Default Panel Type", defType, panelTypes, viewsSize, labelViewMode, panelGrp, false, false, QString(), PAGE_VIEW);
    hbox->addWidget(cmb);
    hbox->addWidget(createSpacer(panelGrp));

    delete[] panelTypes;

    panelGrid->addLayout(hbox, 0, 0);

    // ----- Individual Settings Per View Type ------------------------
    auto *tabs_view = new QTabWidget(panelGrp);
    panelGrid->addWidget(tabs_view, 11, 0);

    for (int i = 0; i < views.count(); i++) {
        QWidget *tab = new QWidget(tabs_view);
        tabs_view->addTab(tab, views[i]->description());
        setupView(views[i], tab);
    }
}

// -----------------------------------------------------------------------------------
//  -------------------------- Panel Toolbar TAB ----------------------------------
// -----------------------------------------------------------------------------------
void KgPanel::setupButtonsTab()
{
    auto *scrollArea = new QScrollArea(tabWidget);
    QWidget *tab = new QWidget(scrollArea);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setWidget(tab);
    scrollArea->setWidgetResizable(true);
    tabWidget->addTab(scrollArea, i18n("Buttons"));

    QBoxLayout *tabLayout = new QVBoxLayout(tab);
    tabLayout->setSpacing(6);
    tabLayout->setContentsMargins(11, 11, 11, 11);

    KONFIGURATOR_CHECKBOX_PARAM buttonsParams[] =
        //   cfg_class    cfg_name                default        text                          restart tooltip
        {
            {"ListPanelButtons", "Icons", false, i18n("Toolbar buttons have icons"), true, ""},
            {"Look&Feel", "Media Button Visible", true, i18n("Show Media Button"), true, i18n("The media button will be visible.")},
            {"Look&Feel", "Back Button Visible", false, i18n("Show Back Button"), true, "Goes back in history."},
            {"Look&Feel", "Forward Button Visible", false, i18n("Show Forward Button"), true, "Goes forward in history."},
            {"Look&Feel", "History Button Visible", true, i18n("Show History Button"), true, i18n("The history button will be visible.")},
            {"Look&Feel", "Bookmarks Button Visible", true, i18n("Show Bookmarks Button"), true, i18n("The bookmarks button will be visible.")},
            {"Look&Feel", "Panel Toolbar visible", _PanelToolBar, i18n("Show Panel Toolbar"), true, i18n("The panel toolbar will be visible.")},
        };
    buttonsCheckboxes = createCheckBoxGroup(1, 0, buttonsParams, 7 /*count*/, tab, PAGE_PANELTOOLBAR);
    connect(buttonsCheckboxes->find("Panel Toolbar visible"), &KonfiguratorCheckBox::stateChanged, this, &KgPanel::slotEnablePanelToolbar);
    tabLayout->addWidget(buttonsCheckboxes, 0, Qt::Alignment());

    QGroupBox *panelToolbarGrp = createFrame(i18n("Visible Panel Toolbar buttons"), tab);
    QGridLayout *panelToolbarGrid = createGridLayout(panelToolbarGrp);
    KONFIGURATOR_CHECKBOX_PARAM panelToolbarButtonsParams[] = {
        //   cfg_class    cfg_name                default             text                       restart tooltip
        {"Look&Feel", "Equal Button Visible", _cdOther, i18n("Equal button (=)"), true, i18n("Changes the panel folder to the other panel folder.")},
        {"Look&Feel", "Up Button Visible", _cdUp, i18n("Up button (..)"), true, i18n("Changes the panel folder to the parent folder.")},
        {"Look&Feel", "Home Button Visible", _cdHome, i18n("Home button (~)"), true, i18n("Changes the panel folder to the home folder.")},
        {"Look&Feel", "Root Button Visible", _cdRoot, i18n("Root button (/)"), true, i18n("Changes the panel folder to the root folder.")},
        {"Look&Feel",
         "SyncBrowse Button Visible",
         _syncBrowseButton,
         i18n("Toggle-button for sync-browsing"),
         true,
         i18n("Each folder change in the panel is also performed in the other panel.")},
    };
    panelToolbarButtonsCheckboxes = createCheckBoxGroup(1,
                                                        0,
                                                        panelToolbarButtonsParams,
                                                        sizeof(panelToolbarButtonsParams) / sizeof(*panelToolbarButtonsParams),
                                                        panelToolbarGrp,
                                                        PAGE_PANELTOOLBAR);
    panelToolbarGrid->addWidget(panelToolbarButtonsCheckboxes, 0, 0);
    tabLayout->addWidget(panelToolbarGrp, 1, Qt::Alignment());

    // Enable panel toolbar checkboxes
    slotEnablePanelToolbar();
}

// ---------------------------------------------------------------------------
//  -------------------------- Mouse TAB ----------------------------------
// ---------------------------------------------------------------------------
void KgPanel::setupMouseModeTab()
{
    auto *scrollArea = new QScrollArea(tabWidget);
    QWidget *tab_mouse = new QWidget(scrollArea);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setWidget(tab_mouse);
    scrollArea->setWidgetResizable(true);
    tabWidget->addTab(scrollArea, i18n("Selection Mode"));
    auto *mouseLayout = new QGridLayout(tab_mouse);
    mouseLayout->setSpacing(6);
    mouseLayout->setContentsMargins(11, 11, 11, 11);

    // -------------- General -----------------
    QGroupBox *mouseGeneralGroup = createFrame(i18n("General"), tab_mouse);
    QGridLayout *mouseGeneralGrid = createGridLayout(mouseGeneralGroup);
    mouseGeneralGrid->setSpacing(0);
    mouseGeneralGrid->setContentsMargins(5, 5, 5, 5);

    KONFIGURATOR_NAME_VALUE_TIP mouseSelection[] = {
        //     name           value          tooltip
        {i18n("Krusader Mode"),
         "0",
         i18n("Both keys allow selecting files. To select more than one file, hold the Ctrl key and click the left mouse button. Right-click menu is invoked "
              "using a short click on the right mouse button.")},
        {i18n("Konqueror Mode"),
         "1",
         i18n("Pressing the left mouse button selects files - you can click and select multiple files. Right-click menu is invoked using a short click on the "
              "right mouse button.")},
        {i18n("Total-Commander Mode"),
         "2",
         i18n("The left mouse button does not select, but sets the current file without affecting the current selection. The right mouse button selects "
              "multiple files and the right-click menu is invoked by pressing and holding the right mouse button.")},
        {i18n("Ergonomic Mode"),
         "4",
         i18n("The left mouse button does not select, but sets the current file without affecting the current selection. The right mouse button invokes the "
              "context-menu. You can select with Ctrl key and the left button.")},
        {i18n("Custom Selection Mode"), "3", i18n("Design your own selection mode.")}};
    mouseRadio = createRadioButtonGroup("Look&Feel", "Mouse Selection", "0", 1, 5, mouseSelection, 5, mouseGeneralGroup, true, PAGE_MOUSE);
    mouseRadio->layout()->setContentsMargins(0, 0, 0, 0);
    mouseGeneralGrid->addWidget(mouseRadio, 0, 0);

    for (int i = 0; i != mouseRadio->count(); i++)
        connect(mouseRadio->find(i), &QRadioButton::clicked, this, &KgPanel::slotSelectionModeChanged);

    mouseLayout->addWidget(mouseGeneralGroup, 0, 0);

    // -------------- Details -----------------
    QGroupBox *mouseDetailGroup = createFrame(i18n("Details"), tab_mouse);
    QGridLayout *mouseDetailGrid = createGridLayout(mouseDetailGroup);
    mouseDetailGrid->setSpacing(0);
    mouseDetailGrid->setContentsMargins(5, 5, 5, 5);

    KONFIGURATOR_NAME_VALUE_TIP singleOrDoubleClick[] = {
        //          name            value            tooltip
        {i18n("Double-click selects (classic)"),
         "0",
         i18n("A single click on a file will select and focus, a double click opens the file or steps into the folder.")},
        {i18n("Obey global selection policy"), "1", i18n("<p>Use global setting:</p><p><i>Plasma System Settings -> Input Devices -> Mouse</i></p>")}};
    KonfiguratorRadioButtons *clickRadio =
        createRadioButtonGroup("Look&Feel", "Single Click Selects", "0", 1, 0, singleOrDoubleClick, 2, mouseDetailGroup, true, PAGE_MOUSE);
    clickRadio->layout()->setContentsMargins(0, 0, 0, 0);
    mouseDetailGrid->addWidget(clickRadio, 0, 0);

    KONFIGURATOR_CHECKBOX_PARAM mouseCheckboxesParam[] = {
        // {cfg_class,   cfg_name,   default
        //  text,  restart,
        //  tooltip }
        {"Custom Selection Mode",
         "QT Selection",
         _QtSelection,
         i18n("Based on KDE's selection mode"),
         true,
         i18n("If checked, use a mode based on KDE's style.")},
        {"Custom Selection Mode",
         "Left Selects",
         _LeftSelects,
         i18n("Left mouse button selects"),
         true,
         i18n("If checked, left clicking an item will select it.")},
        {"Custom Selection Mode",
         "Left Preserves",
         _LeftPreserves,
         i18n("Left mouse button preserves selection"),
         true,
         i18n("If checked, left clicking an item will select it, but will not unselect other, already selected items.")},
        {"Custom Selection Mode",
         "ShiftCtrl Left Selects",
         _ShiftCtrlLeft,
         i18n("Shift/Ctrl-Left mouse button selects"),
         true,
         i18n("If checked, Shift/Ctrl left clicking will select items.\nNote: this is meaningless if 'Left Button Selects' is checked.")},
        {"Custom Selection Mode",
         "Right Selects",
         _RightSelects,
         i18n("Right mouse button selects"),
         true,
         i18n("If checked, right clicking an item will select it.")},
        {"Custom Selection Mode",
         "Right Preserves",
         _RightPreserves,
         i18n("Right mouse button preserves selection"),
         true,
         i18n("If checked, right clicking an item will select it, but will not unselect other, already selected items.")},
        {"Custom Selection Mode",
         "ShiftCtrl Right Selects",
         _ShiftCtrlRight,
         i18n("Shift/Ctrl-Right mouse button selects"),
         true,
         i18n("If checked, Shift/Ctrl right clicking will select items.\nNote: this is meaningless if 'Right Button Selects' is checked.")},
        {"Custom Selection Mode",
         "Space Moves Down",
         _SpaceMovesDown,
         i18n("Spacebar moves down"),
         true,
         i18n("If checked, pressing the spacebar will select the current item and move down.\nOtherwise, current item is selected, but remains the current "
              "item.")},
        {"Custom Selection Mode",
         "Space Calc Space",
         _SpaceCalcSpace,
         i18n("Spacebar calculates disk space"),
         true,
         i18n("If checked, pressing the spacebar while the current item is a folder, will (except from selecting the folder)\ncalculate space occupied of the "
              "folder (recursively).")},
        {"Custom Selection Mode",
         "Insert Moves Down",
         _InsertMovesDown,
         i18n("Insert moves down"),
         true,
         i18n("If checked, pressing Insert will select the current item, and move down to the next item.\nOtherwise, current item is not changed.")},
        {"Custom Selection Mode",
         "Immediate Context Menu",
         _ImmediateContextMenu,
         i18n("Right clicking pops context menu immediately"),
         true,
         i18n("If checked, right clicking will result in an immediate showing of the context menu.\nOtherwise, user needs to click and hold the right mouse "
              "button for 500ms.")},
        {"Custom Selection Mode",
         "Reset Selection Items",
         _ResetSelectionItems,
         i18n("A plain mouse button resets selection"),
         true,
         i18n("Reset selection on a mouse click without modifiers (i.e. Shift or Ctrl).\nSelection with Shift/Ctrl for the mouse button have to be enabled for "
              "this setting to work.")},
    };

    mouseCheckboxes = createCheckBoxGroup(1, 0, mouseCheckboxesParam, 12 /*count*/, mouseDetailGroup, PAGE_MOUSE);
    mouseDetailGrid->addWidget(mouseCheckboxes, 1, 0);

    for (int i = 0; i < mouseCheckboxes->count(); i++)
        connect(mouseCheckboxes->find(i), &KonfiguratorCheckBox::clicked, this, &KgPanel::slotMouseCheckBoxChanged);

    mouseLayout->addWidget(mouseDetailGroup, 0, 1, 2, 1);

    // Disable the details-button if not in custom-mode
    slotSelectionModeChanged();

    // -------------- Preview -----------------
    QGroupBox *mousePreviewGroup = createFrame(i18n("Preview"), tab_mouse);
    QGridLayout *mousePreviewGrid = createGridLayout(mousePreviewGroup);
    // TODO preview
    mousePreview = new KrTreeWidget(mousePreviewGroup);
    mousePreviewGrid->addWidget(mousePreview, 0, 0);
    mousePreviewGroup->setEnabled(false); // TODO re-enable once the preview is implemented
    // ------------------------------------------
    mouseLayout->addWidget(mousePreviewGroup, 1, 0);
}

// ---------------------------------------------------------------------------
//  -------------------------- Media Menu TAB ----------------------------------
// ---------------------------------------------------------------------------
void KgPanel::setupMediaMenuTab()
{
    auto *scrollArea = new QScrollArea(tabWidget);
    QWidget *tab = new QWidget(scrollArea);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setWidget(tab);
    scrollArea->setWidgetResizable(true);
    tabWidget->addTab(scrollArea, i18n("Media Menu"));

    QBoxLayout *tabLayout = new QVBoxLayout(tab);
    tabLayout->setSpacing(6);
    tabLayout->setContentsMargins(11, 11, 11, 11);

    KONFIGURATOR_CHECKBOX_PARAM mediaMenuParams[] = {
        //   cfg_class    cfg_name    default    text   restart tooltip
        {"MediaMenu", "ShowPath", true, i18n("Show Mount Path"), false, nullptr},
        {"MediaMenu", "ShowFSType", true, i18n("Show File System Type"), false, nullptr},
        {"MediaMenu", "HideSquashFS", false, i18n("Hide SquashFS entries"), false, nullptr},
        {"MediaMenu", "HideUnknownFS", false, i18n("Hide unknown File System entries"), false, nullptr},
    };
    KonfiguratorCheckBoxGroup *mediaMenuCheckBoxes =
        createCheckBoxGroup(1, 0, mediaMenuParams, sizeof(mediaMenuParams) / sizeof(*mediaMenuParams), tab, PAGE_MEDIA_MENU);
    tabLayout->addWidget(mediaMenuCheckBoxes, 0, Qt::Alignment());

    auto *showSizeHBox = new QHBoxLayout();
    QLabel *labelShowSize = new QLabel(i18n("Show Size:"), tab);
    showSizeHBox->addWidget(labelShowSize);
    KONFIGURATOR_NAME_VALUE_PAIR showSizeValues[] = {
        {i18nc("setting 'show size'", "Always"), "Always"},
        {i18nc("setting 'show size'", "When Device has no Label"), "WhenNoLabel"},
        {i18nc("setting 'show size'", "Never"), "Never"},
    };
    KonfiguratorComboBox *showSizeCmb = createComboBox("MediaMenu",
                                                       "ShowSize",
                                                       "Always",
                                                       showSizeValues,
                                                       sizeof(showSizeValues) / sizeof(*showSizeValues),
                                                       labelShowSize,
                                                       tab,
                                                       false,
                                                       false,
                                                       QString(),
                                                       PAGE_MEDIA_MENU);
    showSizeHBox->addWidget(showSizeCmb);
    createIgnoredMountpointsList(tab, tabLayout);
    showSizeHBox->addStretch();
    tabLayout->addLayout(showSizeHBox);

    tabLayout->addStretch();
}

void KgPanel::createIgnoredMountpointsList(QWidget *tab, QBoxLayout *tabLayout)
{
    QWidget *vboxWidget2 = new QWidget(tab);
    tabLayout->addWidget(vboxWidget2);
    auto *vbox2 = new QVBoxLayout(vboxWidget2);

    QWidget *hboxWidget3 = new QWidget(vboxWidget2);
    vbox2->addWidget(hboxWidget3);

    auto *hbox3 = new QHBoxLayout(hboxWidget3);

    QLabel *atomLabel = new QLabel(i18n("Hide following mountpoints:"), hboxWidget3);
    hbox3->addWidget(atomLabel);

    int size = QFontMetrics(atomLabel->font()).height();

    auto *addButton = new QToolButton(hboxWidget3);
    hbox3->addWidget(addButton);

    QPixmap iconPixmap = Icon("list-add").pixmap(size);
    addButton->setFixedSize(iconPixmap.width() + 4, iconPixmap.height() + 4);
    addButton->setIcon(QIcon(iconPixmap));
    connect(addButton, &QToolButton::clicked, this, &KgPanel::slotAddMountpoint);

    auto *removeButton = new QToolButton(hboxWidget3);
    hbox3->addWidget(removeButton);

    iconPixmap = Icon("list-remove").pixmap(size);
    removeButton->setFixedSize(iconPixmap.width() + 4, iconPixmap.height() + 4);
    removeButton->setIcon(QIcon(iconPixmap));
    connect(removeButton, &QToolButton::clicked, this, &KgPanel::slotRemoveMountpoint);

    QStringList defaultHiddenMountpoints; // Empty list
    listBox = createListBox("MediaMenu", "Hidden Mountpoints", defaultHiddenMountpoints, vboxWidget2, true, QString(), PAGE_MEDIA_MENU);
    vbox2->addWidget(listBox);
}

void KgPanel::slotAddMountpoint()
{
    bool ok;
    QString atomExt = QInputDialog::getText(this, i18n("Add new hidden mount point"), i18n("Mount point:"), QLineEdit::Normal, QString(), &ok);

    if (ok) {
        listBox->addItem(atomExt);
    }
}

void KgPanel::slotRemoveMountpoint()
{
    QList<QListWidgetItem *> list = listBox->selectedItems();

    for (int i = 0; i != list.count(); i++)
        listBox->removeItem(list[i]->text());
}

void KgPanel::slotEnablePanelToolbar()
{
    bool enableTB = buttonsCheckboxes->find("Panel Toolbar visible")->isChecked();
    panelToolbarButtonsCheckboxes->find("Root Button Visible")->setEnabled(enableTB);
    panelToolbarButtonsCheckboxes->find("Home Button Visible")->setEnabled(enableTB);
    panelToolbarButtonsCheckboxes->find("Up Button Visible")->setEnabled(enableTB);
    panelToolbarButtonsCheckboxes->find("Equal Button Visible")->setEnabled(enableTB);
    panelToolbarButtonsCheckboxes->find("SyncBrowse Button Visible")->setEnabled(enableTB);
}

void KgPanel::slotSelectionModeChanged()
{
    KrSelectionMode *selectionMode = KrSelectionMode::getSelectionHandlerForMode(mouseRadio->selectedValue());
    if (selectionMode == nullptr) // User mode
        return;
    selectionMode->init();
    mouseCheckboxes->find("QT Selection")->setChecked(selectionMode->useQTSelection());
    mouseCheckboxes->find("Left Selects")->setChecked(selectionMode->leftButtonSelects());
    mouseCheckboxes->find("Left Preserves")->setChecked(selectionMode->leftButtonPreservesSelection());
    mouseCheckboxes->find("ShiftCtrl Left Selects")->setChecked(selectionMode->shiftCtrlLeftButtonSelects());
    mouseCheckboxes->find("Right Selects")->setChecked(selectionMode->rightButtonSelects());
    mouseCheckboxes->find("Right Preserves")->setChecked(selectionMode->rightButtonPreservesSelection());
    mouseCheckboxes->find("ShiftCtrl Right Selects")->setChecked(selectionMode->shiftCtrlRightButtonSelects());
    mouseCheckboxes->find("Space Moves Down")->setChecked(selectionMode->spaceMovesDown());
    mouseCheckboxes->find("Space Calc Space")->setChecked(selectionMode->spaceCalculatesDiskSpace());
    mouseCheckboxes->find("Insert Moves Down")->setChecked(selectionMode->insertMovesDown());
    mouseCheckboxes->find("Immediate Context Menu")->setChecked(selectionMode->showContextMenu() == -1);
    mouseCheckboxes->find("Reset Selection Items")->setChecked(selectionMode->resetSelectionItems());
}

void KgPanel::slotMouseCheckBoxChanged()
{
    mouseRadio->selectButton("3"); // custom selection mode
}

int KgPanel::activeSubPage()
{
    return tabWidget->currentIndex();
}

bool KgPanel::apply()
{
    // We read the IconSize config file before and after
    // applying the change, to see if it really changed
    KConfigGroup briefView(krConfig, "KrInterBriefView");
    KConfigGroup detailedView(krConfig, "KrInterDetailedView");

    int oldiconBriefSize = briefView.readEntry("IconSize", _FilelistIconSize).toInt();
    int oldiconDetailSize = detailedView.readEntry("IconSize", _FilelistIconSize).toInt();
    KonfiguratorPage::apply();
    int iconBriefSize = briefView.readEntry("IconSize", _FilelistIconSize).toInt();
    int iconDetailSize = detailedView.readEntry("IconSize", _FilelistIconSize).toInt();

    return oldiconBriefSize != iconBriefSize || oldiconDetailSize != iconDetailSize;
}
