/***************************************************************************
                        kgpanel.cpp  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kgpanel.h"
#include "../defaults.h"
#include "../Dialogs/krdialogs.h"
#include <QtGui/QTabWidget>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <klocale.h>
#include <QtGui/QValidator>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include "../GUI/krtreewidget.h"
#include "../Panel/krselectionmode.h"
#include "../Panel/krview.h"
#include "../Panel/krviewfactory.h"

#define PAGE_VIEW         0
#define PAGE_PANELTOOLBAR  1
#define PAGE_MOUSE  2
#define PAGE_MISC     3


KgLookFeel::KgLookFeel(bool first, QWidget* parent) :
        KonfiguratorPage(first, parent)
{
    QWidget *innerWidget = this;
    QGridLayout *kgPanelLayout = new QGridLayout(innerWidget);
    kgPanelLayout->setSpacing(6);

    tabWidget = new QTabWidget(innerWidget);

    setupPanelTab();
    setupPanelToolbarTab();
    setupMouseModeTab();
    setupMiscTab();

    kgPanelLayout->addWidget(tabWidget, 0, 0);
}

// ---------------------------------------------------------------------------------------
//  ---------------------------- Misc TAB ------------------------------------------------
// ---------------------------------------------------------------------------------------
void KgLookFeel::setupMiscTab()
{
    QScrollArea *scrollArea = new QScrollArea(tabWidget);
    QWidget *tab = new QWidget(scrollArea);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setWidget(tab);
    scrollArea->setWidgetResizable(true);
    tabWidget->addTab(scrollArea, i18n("Misc"));

    QGridLayout *miscLayout = new QGridLayout(tab);
    miscLayout->setSpacing(6);
    miscLayout->setContentsMargins(11, 11, 11, 11);

// ---------------------------------------------------------------------------------------
// -----------------------------  Quicksearch  -------------------------------------------
// ---------------------------------------------------------------------------------------
    QGroupBox *quicksearchGroup = createFrame(i18n("Quicksearch"), tab);
    QGridLayout *quicksearchGrid = createGridLayout(quicksearchGroup);

    KONFIGURATOR_CHECKBOX_PARAM quicksearch[] = { //   cfg_class  cfg_name                default             text                              restart tooltip
        {"Look&Feel", "New Style Quicksearch",  _NewStyleQuicksearch, i18n("New style quicksearch"), false,  i18n("Opens a quick search dialog box.") },
        {"Look&Feel", "Case Sensitive Quicksearch",  _CaseSensitiveQuicksearch, i18n("Case sensitive quicksearch"), false,  i18n("All files beginning with capital letters appear before files beginning with non-capital letters (UNIX default).") },
        {"Look&Feel", "Up/Down Cancels Quicksearch",  false, i18n("Up/Down cancels Quicksearch"), false,  i18n("Pressing the Up/Down buttons cancels Quicksearch.") },
    };

    quicksearchCheckboxes = createCheckBoxGroup(2, 0, quicksearch, 3 /*count*/, quicksearchGroup, PAGE_MISC);
    quicksearchGrid->addWidget(quicksearchCheckboxes, 0, 0);
    connect(quicksearchCheckboxes->find("New Style Quicksearch"), SIGNAL(stateChanged(int)), this, SLOT(slotDisable()));
    slotDisable();

    // -------------- Quicksearch position -----------------------

    QHBoxLayout *hbox = new QHBoxLayout();

    hbox->addWidget(new QLabel(i18n("Quicksearch position:"), quicksearchGroup));

    KONFIGURATOR_NAME_VALUE_PAIR qsPositions[] = {{ i18n("Top"),                "top" },
        { i18n("Bottom"),                    "bottom" }
    };
    KonfiguratorComboBox *cmb = createComboBox("Look&Feel", "Quicksearch Position",
                            "bottom", qsPositions, 2, quicksearchGroup, true, false, PAGE_MISC);
    hbox->addWidget(cmb);

    hbox->addWidget(createSpacer(quicksearchGroup));

    quicksearchGrid->addLayout(hbox, 1, 0);


    miscLayout->addWidget(quicksearchGroup, 2, 0);


// ---------------------------------------------------------------------------------------
// ------------------------------ Tabs ---------------------------------------------------
// ---------------------------------------------------------------------------------------
    QGroupBox *miscGrp = createFrame(i18n("Tabs"), tab);
    QGridLayout *miscGrid = createGridLayout(miscGrp);

    KONFIGURATOR_CHECKBOX_PARAM tabbar_settings[] = { //   cfg_class  cfg_name                default             text                              restart tooltip
        {"Look&Feel", "Fullpath Tab Names",   _FullPathTabNames,  i18n("Use full path tab names"), true ,  i18n("Display the full path in the folder tabs. By default only the last part of the path is displayed.") },
    };
    KonfiguratorCheckBoxGroup *cbs = createCheckBoxGroup(2, 0, tabbar_settings, 1 /*count*/, miscGrp, PAGE_MISC);
    miscGrid->addWidget(cbs, 0, 0);

// -----------------  Tab Bar position ----------------------------------
    hbox = new QHBoxLayout();

    hbox->addWidget(new QLabel(i18n("Tab Bar position:"), miscGrp));

    cmb = createComboBox("Look&Feel", "Tab Bar Position",
                                "bottom", qsPositions, 2, miscGrp, true, false, PAGE_MISC);

    hbox->addWidget(cmb);
    hbox->addWidget(createSpacer(miscGrp));

    miscGrid->addLayout(hbox, 1, 0);

    miscLayout->addWidget(miscGrp, 1, 0);

// ---------------------------------------------------------------------------------------
// ------------------------------- Operation ---------------------------------------------
// ---------------------------------------------------------------------------------------
    miscGrp = createFrame(i18n("Operation"), tab);
    miscGrid = createGridLayout(miscGrp);

    KONFIGURATOR_CHECKBOX_PARAM operation_settings[] = { //   cfg_class  cfg_name                default             text                              restart tooltip
        {"Look&Feel", "Mark Dirs",            _MarkDirs,          i18n("Autoselect directories"),   false,  i18n("When matching the select criteria, not only files will be selected, but also directories.") },
        {"Look&Feel", "Rename Selects Extension", true,          i18n("Rename selects extension"),   false,  i18n("When renaming a file, the whole text is selected. If you want Total-Commander like renaming of just the name, without extension, uncheck this option.") },
    };
    cbs = createCheckBoxGroup(2, 0, operation_settings, 2 /*count*/, miscGrp, PAGE_MISC);
    miscGrid->addWidget(cbs, 0, 0);

    miscLayout->addWidget(miscGrp, 0, 0);

// --------------------------------------------------------------------------------------------
// ------------------------------- Status/Totalsbar settings ----------------------------------
// --------------------------------------------------------------------------------------------
    miscGrp = createFrame(i18n("Status/Totalsbar"), tab);
    miscGrid = createGridLayout(miscGrp);

    KONFIGURATOR_CHECKBOX_PARAM barSettings[] =
    {
        {"Look&Feel", "Statusbar Frame", false, i18n("Draw frame"), true,  i18n("Draw a Frame around the Status/Totalsbar") },
        {"Look&Feel", "Statusbar Background", false, i18n("Fill background"), true,  i18n("Fill Background of Status/Totalsbar") },
    };
    KonfiguratorCheckBoxGroup *barSett = createCheckBoxGroup(2, 0, barSettings,
                                          2 /*count*/, miscGrp, PAGE_MISC);
    miscGrid->addWidget(barSett, 1, 0, 1, 2);

    miscLayout->addWidget(miscGrp, 3, 0);
}

void KgLookFeel::setupView(KrViewInstance *instance, QWidget *parent)
{
    QGridLayout *grid = createGridLayout(parent);

// -------------------- Filelist icon size ----------------------------------
    QHBoxLayout *hbox = new QHBoxLayout();

    hbox->addWidget(new QLabel(i18n("Filelist icon size:"), parent));

    KONFIGURATOR_NAME_VALUE_PAIR *iconSizes = new KONFIGURATOR_NAME_VALUE_PAIR[KrView::iconSizes.count()];
    for(int i = 0; i < KrView::iconSizes.count(); i++)
        iconSizes[i].text =  iconSizes[i].value = QString::number(KrView::iconSizes[i]);
    KonfiguratorComboBox *cmb = createComboBox(instance->name(), "Filelist Icon Size", _FilelistIconSize, iconSizes, KrView::iconSizes.count(), parent, true, true, PAGE_VIEW);
    delete [] iconSizes;
    cmb->lineEdit()->setValidator(new QRegExpValidator(QRegExp("[1-9]\\d{0,1}"), cmb));
    hbox->addWidget(cmb);

    hbox->addWidget(createSpacer(parent));

    grid->addLayout(hbox, 1, 0);

//--------------------------------------------------------------------
    KONFIGURATOR_CHECKBOX_PARAM iconSettings[] =
        //   cfg_class             cfg_name        default       text            restart        tooltip
    {
        {instance->name(), "With Icons",      _WithIcons,   i18n("Use icons in the filenames"), true,  i18n("Show the icons for filenames and directories.") },
        {instance->name(), "Show Previews",   false,        i18n("Show previews"),              false, i18n("Show previews of files and directories.") },
    };

    KonfiguratorCheckBoxGroup *iconSett = createCheckBoxGroup(2, 0, iconSettings, 2 /*count*/, parent, PAGE_VIEW);

    grid->addWidget(iconSett, 2, 0, 1, 2);
}

// ----------------------------------------------------------------------------------
//  ---------------------------- VIEW TAB -------------------------------------------
// ----------------------------------------------------------------------------------
void KgLookFeel::setupPanelTab()
{
    QScrollArea *scrollArea = new QScrollArea(tabWidget);
    QWidget *tab_panel = new QWidget(scrollArea);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setWidget(tab_panel);
    scrollArea->setWidgetResizable(true);
    tabWidget->addTab(scrollArea, i18n("View"));

    QGridLayout *panelLayout = new QGridLayout(tab_panel);
    panelLayout->setSpacing(6);
    panelLayout->setContentsMargins(11, 11, 11, 11);
    QGroupBox *panelGrp = createFrame(i18n("General"), tab_panel);
    panelLayout->addWidget(panelGrp, 0, 0);
    QGridLayout *panelGrid = createGridLayout(panelGrp);

    // ----------------------------------------------------------------------------------
    //  ---------------------------- General settings -----------------------------------
    // ----------------------------------------------------------------------------------

    // -------------------- Panel Font ----------------------------------
    QHBoxLayout *hbox = new QHBoxLayout();

    hbox->addWidget(new QLabel(i18n("View font:"), panelGrp));

    KonfiguratorFontChooser * chsr = createFontChooser("Look&Feel", "Filelist Font", _FilelistFont, panelGrp, true, PAGE_VIEW);
    hbox->addWidget(chsr);

    hbox->addWidget(createSpacer(panelGrp));

    panelGrid->addLayout(hbox, 1, 0);

    // =========================================================
    panelGrid->addWidget(createLine(panelGrp), 2, 0);

    KONFIGURATOR_CHECKBOX_PARAM panelSettings[] =
        //   cfg_class  cfg_name                default text                                  restart tooltip
    {
        {"Look&Feel", "Human Readable Size",            _HumanReadableSize,      i18n("Use human-readable file size"), true ,  i18n("File sizes are displayed in B, KB, MB and GB, not just in bytes.") },
        {"Look&Feel", "Show Hidden",                    _ShowHidden,             i18n("Show hidden files"),      false,  i18n("Display files beginning with a dot.") },
        {"Look&Feel", "Numeric permissions",            _NumericPermissions,     i18n("Numeric Permissions"), true,  i18n("Show octal numbers (0755) instead of the standard permissions (rwxr-xr-x) in the permission column.") },
        {"Look&Feel", "Load User Defined Folder Icons", _UserDefinedFolderIcons, i18n("Load the user defined folder icons"), true ,  i18n("Load the user defined directory icons (can cause decrease in performance).") },
    };

    KonfiguratorCheckBoxGroup *panelSett = createCheckBoxGroup(2, 0, panelSettings, 4 /*count*/, panelGrp, PAGE_VIEW);

    panelGrid->addWidget(panelSett, 3, 0, 1, 2);

    // =========================================================
    panelGrid->addWidget(createLine(panelGrp), 4, 0);

    // ------------------------ Sort Method ----------------------------------

    hbox = new QHBoxLayout();

    hbox->addWidget(new QLabel(i18n("Sort method:"), panelGrp));

    KONFIGURATOR_NAME_VALUE_PAIR sortMethods[] = {{ i18n("Alphabetical"),                QString::number(KrViewProperties::Alphabetical) },
        { i18n("Alphabetical and numbers"),    QString::number(KrViewProperties::AlphabeticalNumbers) },
        { i18n("Character code"),              QString::number(KrViewProperties::CharacterCode) },
        { i18n("Character code and numbers"),  QString::number(KrViewProperties::CharacterCodeNumbers) },
        { i18n("Krusader"),                    QString::number(KrViewProperties::Krusader) }
    };
    KonfiguratorComboBox *cmb = createComboBox("Look&Feel", "Sort method", QString::number(_DefaultSortMethod),
                            sortMethods, 5, panelGrp, true, false, PAGE_VIEW);
    hbox->addWidget(cmb);
    hbox->addWidget(createSpacer(panelGrp));

    panelGrid->addLayout(hbox, 5, 0);

    // ------------------------ Sort Options ----------------------------------
    KONFIGURATOR_CHECKBOX_PARAM sortSettings[] =
        //   cfg_class  cfg_name                default text                                  restart tooltip
    {
        {"Look&Feel", "Case Sensative Sort",            _CaseSensativeSort,      i18n("Case sensitive sorting"), true ,  i18n("All files beginning with capital letters appear before files beginning with non-capital letters (UNIX default).") },
        {"Look&Feel", "Always sort dirs by name",       false,                   i18n("Always sort dirs by name"), true,  i18n("Directories are sorted by name, regardless of the sort column.") },
    };

    KonfiguratorCheckBoxGroup *sortSett = createCheckBoxGroup(2, 0, sortSettings,
                                          2 /*count*/, panelGrp, PAGE_VIEW);

    panelGrid->addWidget(sortSett, 6, 0, 1, 2);


    // ----------------------------------------------------------------------------------
    //  ---------------------------- View types -----------------------------------------
    // ----------------------------------------------------------------------------------

    panelGrp = createFrame(i18n("View types"), tab_panel);
    panelLayout->addWidget(panelGrp, 1, 0);
    panelGrid = createGridLayout(panelGrp);

    // -------------------- Default Panel Type ----------------------------------
    hbox = new QHBoxLayout();

    hbox->addWidget(new QLabel(i18n("Default view type:"), panelGrp));

    QList<KrViewInstance *> views = KrViewFactory::registeredViews();
    const int viewsSize = views.size();
    KONFIGURATOR_NAME_VALUE_PAIR *panelTypes = new KONFIGURATOR_NAME_VALUE_PAIR[ viewsSize ];

    QString defType = "0";

    for (int i = 0; i != viewsSize; i++) {
        KrViewInstance * inst = views[ i ];
        panelTypes[ i ].text = i18n(inst->description().toUtf8());
        panelTypes[ i ].text.remove('&');
        panelTypes[ i ].value = QString("%1").arg(inst->id());
        if (inst->id() == KrViewFactory::defaultViewId())
            defType = QString("%1").arg(inst->id());
    }

    cmb = createComboBox("Look&Feel", "Default Panel Type", defType, panelTypes, viewsSize, panelGrp, false, false, PAGE_VIEW);
    hbox->addWidget(cmb);
    hbox->addWidget(createSpacer(panelGrp));

    delete [] panelTypes;

    panelGrid->addLayout(hbox, 0, 0);

    // ----- Individual Settings Per View Type ------------------------
    QTabWidget *tabs_view = new QTabWidget(panelGrp);
    panelGrid->addWidget(tabs_view, 11, 0);

    for(int i = 0; i < views.count(); i++) {
        QWidget *tab = new QWidget(tabs_view);
        tabs_view->addTab(tab, views[i]->description());
        setupView(views[i], tab);
    }
}

// -----------------------------------------------------------------------------------
//  -------------------------- Panel Toolbar TAB ----------------------------------
// -----------------------------------------------------------------------------------
void KgLookFeel::setupPanelToolbarTab()
{
    QScrollArea *scrollArea = new QScrollArea(tabWidget);
    QWidget *tab_4 = new QWidget(scrollArea);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setWidget(tab_4);
    scrollArea->setWidgetResizable(true);
    tabWidget->addTab(scrollArea, i18n("Panel Toolbar"));

    QBoxLayout * panelToolbarVLayout = new QVBoxLayout(tab_4);
    panelToolbarVLayout->setSpacing(6);
    panelToolbarVLayout->setContentsMargins(11, 11, 11, 11);

    KONFIGURATOR_CHECKBOX_PARAM panelToolbarActiveCheckbox[] =
        //   cfg_class    cfg_name                default        text                          restart tooltip
    {{"Look&Feel", "Panel Toolbar visible", _PanelToolBar, i18n("Show Panel Toolbar"), true,   i18n("The panel toolbar will be visible.") }
    };

    panelToolbarActive = createCheckBoxGroup(1, 0, panelToolbarActiveCheckbox, 1, tab_4, PAGE_PANELTOOLBAR);
    connect(panelToolbarActive->find("Panel Toolbar visible"), SIGNAL(stateChanged(int)), this, SLOT(slotEnablePanelToolbar()));

    QGroupBox * panelToolbarGrp = createFrame(i18n("Visible Panel Toolbar buttons"), tab_4);
    QGridLayout * panelToolbarGrid = createGridLayout(panelToolbarGrp);

    KONFIGURATOR_CHECKBOX_PARAM panelToolbarCheckboxes[] = {
        //   cfg_class    cfg_name                default             text                       restart tooltip
        {"Look&Feel",  "Open Button Visible",  _Open,      i18n("Open button"),     true ,  i18n("Opens the directory browser.") },
        {"Look&Feel",  "Equal Button Visible", _cdOther,   i18n("Equal button (=)"), true ,  i18n("Changes the panel directory to the other panel directory.") },
        {"Look&Feel",  "Up Button Visible",    _cdUp,      i18n("Up button (..)"),  true ,  i18n("Changes the panel directory to the parent directory.") },
        {"Look&Feel",  "Home Button Visible",  _cdHome,    i18n("Home button (~)"), true ,  i18n("Changes the panel directory to the home directory.") },
        {"Look&Feel",  "Root Button Visible",  _cdRoot,    i18n("Root button (/)"), true ,  i18n("Changes the panel directory to the root directory.") },
        {"Look&Feel",  "SyncBrowse Button Visible",  _syncBrowseButton,    i18n("Toggle-button for sync-browsing"), true ,  i18n("Each directory change in the panel is also performed in the other panel.") },
    };


    pnlcbs = createCheckBoxGroup(1, 0, panelToolbarCheckboxes,
                                 sizeof(panelToolbarCheckboxes) / sizeof(*panelToolbarCheckboxes),
                                 panelToolbarGrp, PAGE_PANELTOOLBAR);

    panelToolbarVLayout->addWidget(panelToolbarActive, 0, 0);
    panelToolbarGrid->addWidget(pnlcbs, 0, 0);
    panelToolbarVLayout->addWidget(panelToolbarGrp,    1, 0);

    // Enable panel toolbar checkboxes
    slotEnablePanelToolbar();
}

// ---------------------------------------------------------------------------
//  -------------------------- Mouse TAB ----------------------------------
// ---------------------------------------------------------------------------
void KgLookFeel::setupMouseModeTab()
{
    QScrollArea *scrollArea = new QScrollArea(tabWidget);
    QWidget *tab_mouse = new QWidget(scrollArea);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setWidget(tab_mouse);
    scrollArea->setWidgetResizable(true);
    tabWidget->addTab(scrollArea, i18n("Selection Mode"));
    QGridLayout *mouseLayout = new QGridLayout(tab_mouse);
    mouseLayout->setSpacing(6);
    mouseLayout->setContentsMargins(11, 11, 11, 11);

    // -------------- General -----------------
    QGroupBox *mouseGeneralGroup = createFrame(i18n("General"), tab_mouse);
    QGridLayout *mouseGeneralGrid = createGridLayout(mouseGeneralGroup);
    mouseGeneralGrid->setSpacing(0);
    mouseGeneralGrid->setContentsMargins(5, 5, 5, 5);

    KONFIGURATOR_NAME_VALUE_TIP mouseSelection[] = {
        //     name           value          tooltip
        { i18n("Krusader Mode"), "0", i18n("Both keys allow selecting files. To select more than one file, hold the Ctrl key and click the left mouse button. Right-click menu is invoked using a short click on the right mouse button.") },
        { i18n("Konqueror Mode"), "1", i18n("Pressing the left mouse button selects files - you can click and select multiple files. Right-click menu is invoked using a short click on the right mouse button.") },
        { i18n("Total-Commander Mode"), "2", i18n("The left mouse button does not select, but sets the current file without affecting the current selection. The right mouse button selects multiple files and the right-click menu is invoked by pressing and holding the right mouse button.") },
        { i18n("Ergonomic Mode"), "4", i18n("The left mouse button does not select, but sets the current file without affecting the current selection. The right mouse button invokes the context-menu. You can select with Ctrl key and the left button.") },
        { i18n("Custom Selection Mode"), "3", i18n("Design your own selection mode!") }
    };
    mouseRadio = createRadioButtonGroup("Look&Feel", "Mouse Selection", "0", 1, 5, mouseSelection, 5, mouseGeneralGroup, true, PAGE_MOUSE);
    mouseRadio->layout()->setContentsMargins(0, 0, 0, 0);
    mouseGeneralGrid->addWidget(mouseRadio, 0, 0);

    for (int i = 0; i != mouseRadio->count(); i++)
        connect(mouseRadio->find(i), SIGNAL(clicked()), SLOT(slotSelectionModeChanged()));

    mouseLayout->addWidget(mouseGeneralGroup, 0, 0);

    // -------------- Details -----------------
    QGroupBox *mouseDetailGroup = createFrame(i18n("Details"), tab_mouse);
    QGridLayout *mouseDetailGrid = createGridLayout(mouseDetailGroup);
    mouseDetailGrid->setSpacing(0);
    mouseDetailGrid->setContentsMargins(5, 5, 5, 5);

    KONFIGURATOR_NAME_VALUE_TIP singleOrDoubleClick[] = {
        //          name            value            tooltip
        { i18n("Double-click selects (classic)"), "0", i18n("A single click on a file will select and focus, a double click opens the file or steps into the directory.") },
        { i18n("Obey KDE's global selection policy"), "1", i18n("<p>Use KDE's global setting:</p><p><i>KDE System Settings -> General -> Computer Administration -> Keyboard & Mouse -> Mouse</i></p>") }
    };
    KonfiguratorRadioButtons *clickRadio = createRadioButtonGroup("Look&Feel", "Single Click Selects", "0", 1, 0, singleOrDoubleClick, 2, mouseDetailGroup, true, PAGE_MOUSE);
    clickRadio->layout()->setContentsMargins(0, 0, 0, 0);
    mouseDetailGrid->addWidget(clickRadio, 0, 0);

    KONFIGURATOR_CHECKBOX_PARAM mouseCheckboxesParam[] = {
        // {cfg_class,   cfg_name,   default
        //  text,  restart,
        //  tooltip }
        {"Custom Selection Mode",  "QT Selection",  _QtSelection,
            i18n("Based on KDE's selection mode"), true,
            i18n("If checked, use a mode based on KDE's style.") },
        {"Custom Selection Mode",  "Left Selects",  _LeftSelects,
         i18n("Left mouse button selects"), true,
         i18n("If checked, left clicking an item will select it.") },
        {"Custom Selection Mode",  "Left Preserves", _LeftPreserves,
         i18n("Left mouse button preserves selection"), true,
         i18n("If checked, left clicking an item will select it, but will not unselect other, already selected items.") },
        {"Custom Selection Mode",  "ShiftCtrl Left Selects",  _ShiftCtrlLeft,
         i18n("Shift/Ctrl-Left mouse button selects"), true,
         i18n("If checked, shift/ctrl left clicking will select items. \nNote: This is meaningless if 'Left Button Selects' is checked.") },
        {"Custom Selection Mode",  "Right Selects",  _RightSelects,
         i18n("Right mouse button selects"), true,
         i18n("If checked, right clicking an item will select it.") },
        {"Custom Selection Mode",  "Right Preserves",  _RightPreserves,
         i18n("Right mouse button preserves selection"), true,
         i18n("If checked, right clicking an item will select it, but will not unselect other, already selected items.") },
        {"Custom Selection Mode",  "ShiftCtrl Right Selects",  _ShiftCtrlRight,
         i18n("Shift/Ctrl-Right mouse button selects"), true,
         i18n("If checked, shift/ctrl right clicking will select items. \nNote: This is meaningless if 'Right Button Selects' is checked.") },
        {"Custom Selection Mode",  "Space Moves Down",  _SpaceMovesDown,
         i18n("Spacebar moves down"), true,
         i18n("If checked, pressing the spacebar will select the current item and move down. \nOtherwise, current item is selected, but remains the current item.") },
        {"Custom Selection Mode",  "Space Calc Space",  _SpaceCalcSpace,
         i18n("Spacebar calculates disk space"), true,
         i18n("If checked, pressing the spacebar while the current item is a directory, will (except from selecting the directory) \ncalculate space occupied of the directory (recursively).") },
        {"Custom Selection Mode",  "Insert Moves Down",  _InsertMovesDown,
         i18n("Insert moves down"), true,
         i18n("If checked, pressing INSERT will select the current item, and move down to the next item. \nOtherwise, current item is not changed.") },
        {"Custom Selection Mode",  "Immediate Context Menu",  _ImmediateContextMenu,
         i18n("Right clicking pops context menu immediately"), true,
         i18n("If checked, right clicking will result in an immediate showing of the context menu. \nOtherwise, user needs to click and hold the right mouse button for 500ms.") },
    };


    mouseCheckboxes = createCheckBoxGroup(1, 0, mouseCheckboxesParam, 11 /*count*/, mouseDetailGroup, PAGE_MOUSE);
    mouseDetailGrid->addWidget(mouseCheckboxes, 1, 0);

    for (int i = 0; i < mouseCheckboxes->count(); i++)
        connect(mouseCheckboxes->find(i), SIGNAL(clicked()), SLOT(slotMouseCheckBoxChanged()));

    mouseLayout->addWidget(mouseDetailGroup, 0, 1, 2, 1);

    // Disable the details-button if not in custom-mode
    slotSelectionModeChanged();

    // -------------- Preview -----------------
    QGroupBox *mousePreviewGroup = createFrame(i18n("Preview"), tab_mouse);
    QGridLayout *mousePreviewGrid = createGridLayout(mousePreviewGroup);
    // TODO preview
    mousePreview = new KrTreeWidget(mousePreviewGroup);
    mousePreviewGrid->addWidget(mousePreview, 0 , 0);
    mousePreviewGroup->setEnabled(false); // TODO re-enable once the preview is implemented
    // ------------------------------------------
    mouseLayout->addWidget(mousePreviewGroup, 1, 0);
}

void KgLookFeel::slotDisable()
{
    bool isNewStyleQuickSearch = quicksearchCheckboxes->find("New Style Quicksearch")->isChecked();
    quicksearchCheckboxes->find("Case Sensitive Quicksearch")->setEnabled(isNewStyleQuickSearch);
}

void KgLookFeel::slotEnablePanelToolbar()
{
    bool enableTB = panelToolbarActive->find("Panel Toolbar visible")->isChecked();
    pnlcbs->find("Root Button Visible")->setEnabled(enableTB);
    pnlcbs->find("Home Button Visible")->setEnabled(enableTB);
    pnlcbs->find("Up Button Visible")->setEnabled(enableTB);
    pnlcbs->find("Equal Button Visible")->setEnabled(enableTB);
    pnlcbs->find("Open Button Visible")->setEnabled(enableTB);
    pnlcbs->find("SyncBrowse Button Visible")->setEnabled(enableTB);
}

void KgLookFeel::slotSelectionModeChanged()
{
    KrSelectionMode *selectionMode =
        KrSelectionMode::getSelectionHandlerForMode(mouseRadio->selectedValue());
    if (selectionMode == NULL)   //User mode
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
}

void KgLookFeel::slotMouseCheckBoxChanged()
{
    mouseRadio->selectButton("3");    //custom selection mode
}

int KgLookFeel::activeSubPage()
{
    return tabWidget->currentIndex();
}
