/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kggeneral.h"

// QtCore
#include <QPointer>
// QtGui
#include <QFontMetrics>
#include <QPixmap>
// QtWidgets
#include <QFrame>
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>

#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

#include "../defaults.h"
#include "../icon.h"
#include "../krglobal.h"
#include "krresulttabledialog.h"

#define PAGE_GENERAL 0
#define PAGE_VIEWER 1
#define PAGE_EXTENSIONS 2

KgGeneral::KgGeneral(bool first, QWidget *parent)
    : KonfiguratorPage(first, parent)
{
    if (first)
        slotFindTools();

    tabWidget = new QTabWidget(this);
    setWidget(tabWidget);
    setWidgetResizable(true);

    createGeneralTab();
    createViewerTab();
    createExtensionsTab();
}

QWidget *KgGeneral::createTab(const QString &name)
{
    auto *scrollArea = new QScrollArea(tabWidget);
    tabWidget->addTab(scrollArea, name);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);

    QWidget *tab = new QWidget(scrollArea);
    scrollArea->setWidget(tab);

    return tab;
}

void KgGeneral::createViewerTab()
{
    QWidget *tab = createTab(i18n("Viewer/Editor"));
    auto *tabLayout = new QGridLayout(tab);
    tabLayout->setSpacing(6);
    tabLayout->setContentsMargins(11, 11, 11, 11);

    tabLayout->addWidget(createCheckBox("General",
                                        "View In Separate Window",
                                        _ViewInSeparateWindow,
                                        i18n("Internal editor and viewer opens each file in a separate window"),
                                        tab,
                                        false,
                                        i18n("If checked, each file will open in a separate window, otherwise, the viewer will work in a single, tabbed mode"),
                                        PAGE_VIEWER));

    tabLayout->addWidget(createCheckBox("General",
                                        "Viewer Hide Single Tab",
                                        _ViewerHideSingleTab,
                                        i18n("Hide the tab bar when only one tab is opened"),
                                        tab,
                                        false,
                                        i18n("If checked, the Viewer tab bar will hide if only one tab is open"),
                                        PAGE_VIEWER));

    // ------------------------- viewer ----------------------------------

    QGroupBox *viewerGrp = createFrame(i18n("Viewer"), tab);
    tabLayout->addWidget(viewerGrp, 2, 0);

    QGridLayout *viewerGrid = createGridLayout(viewerGrp);

    QWidget *hboxWidget2 = new QWidget(viewerGrp);
    auto *hbox2 = new QHBoxLayout(hboxWidget2);

    QWidget *vboxWidget = new QWidget(hboxWidget2);
    auto *vbox = new QVBoxLayout(vboxWidget);

    vbox->addWidget(new QLabel(i18n("Default viewer mode:"), vboxWidget));

    KONFIGURATOR_NAME_VALUE_TIP viewMode[] =
        //   name    value    tooltip
        {
            {i18n("Generic mode"), "generic", i18n("Use the system's default viewer")},
            {i18n("Text mode"), "text", i18n("View the file in text-only mode")},
            {i18n("Hex mode"), "hex", i18n("View the file in hex-mode (better for binary files)")},
            {i18n("Lister mode"), "lister", i18n("View the file with lister (for huge text files)")},
        };

    vbox->addWidget(createRadioButtonGroup("General", "Default Viewer Mode", "generic", 0, 4, viewMode, 4, vboxWidget, false, PAGE_VIEWER));

    vbox->addWidget(createCheckBox("General",
                                   "UseOktetaViewer",
                                   _UseOktetaViewer,
                                   i18n("Use Okteta as Hex viewer"),
                                   vboxWidget,
                                   false,
                                   i18n("If available, use Okteta as Hex viewer instead of the internal viewer"),
                                   PAGE_VIEWER));

    QWidget *hboxWidget4 = new QWidget(vboxWidget);
    auto *hbox4 = new QHBoxLayout(hboxWidget4);

    const QString listerLimitTip = i18n("If a text file is bigger than this size then lister will be used.");
    QLabel *label5 = new QLabel(i18n("Use lister if the text file is bigger than:"), hboxWidget4);
    hbox4->addWidget(label5);
    KonfiguratorSpinBox *spinBox =
        createSpinBox("General", "Lister Limit", _ListerLimit, 0, 0x7FFFFFFF, label5, hboxWidget4, false, listerLimitTip, PAGE_VIEWER);
    hbox4->addWidget(spinBox);
    QLabel *label6 = new QLabel(i18n("MB"), hboxWidget4);
    hbox4->addWidget(label6);

    vbox->addWidget(hboxWidget4);

    hbox2->addWidget(vboxWidget);

    viewerGrid->addWidget(hboxWidget2, 0, 0);

    // ------------------------- editor ----------------------------------

    QGroupBox *editorGrp = createFrame(i18n("Editor"), tab);
    tabLayout->addWidget(editorGrp, 3, 0);
    QGridLayout *editorGrid = createGridLayout(editorGrp);

    QLabel *label1 = new QLabel(i18n("Editor:"), editorGrp);
    editorGrid->addWidget(label1, 0, 0);
    KonfiguratorURLRequester *urlReq = createURLRequester("General", "Editor", "internal editor", label1, editorGrp, false, QString(), PAGE_VIEWER, false);
    editorGrid->addWidget(urlReq, 0, 1);

    QLabel *label2 = new QLabel(i18n("Hint: use 'internal editor' if you want to use Krusader's fast built-in editor"), editorGrp);
    editorGrid->addWidget(label2, 1, 0, 1, 2);
}

void KgGeneral::createExtensionsTab()
{
    // ------------------------- atomic extensions ----------------------------------

    QWidget *tab = createTab(i18n("Atomic extensions"));
    auto *tabLayout = new QGridLayout(tab);
    tabLayout->setSpacing(6);
    tabLayout->setContentsMargins(11, 11, 11, 11);

    QWidget *vboxWidget2 = new QWidget(tab);
    tabLayout->addWidget(vboxWidget2);

    auto *vbox2 = new QVBoxLayout(vboxWidget2);

    QWidget *hboxWidget3 = new QWidget(vboxWidget2);
    vbox2->addWidget(hboxWidget3);

    auto *hbox3 = new QHBoxLayout(hboxWidget3);

    QLabel *atomLabel = new QLabel(i18n("Atomic extensions:"), hboxWidget3);
    hbox3->addWidget(atomLabel);

    int size = QFontMetrics(atomLabel->font()).height();

    auto *addButton = new QToolButton(hboxWidget3);
    hbox3->addWidget(addButton);

    QPixmap iconPixmap = Icon("list-add").pixmap(size);
    addButton->setFixedSize(iconPixmap.width() + 4, iconPixmap.height() + 4);
    addButton->setIcon(QIcon(iconPixmap));
    connect(addButton, &QToolButton::clicked, this, &KgGeneral::slotAddExtension);

    auto *removeButton = new QToolButton(hboxWidget3);
    hbox3->addWidget(removeButton);

    iconPixmap = Icon("list-remove").pixmap(size);
    removeButton->setFixedSize(iconPixmap.width() + 4, iconPixmap.height() + 4);
    removeButton->setIcon(QIcon(iconPixmap));
    connect(removeButton, &QToolButton::clicked, this, &KgGeneral::slotRemoveExtension);

    QStringList defaultAtomicExtensions;
    defaultAtomicExtensions += ".tar.gz";
    defaultAtomicExtensions += ".tar.bz2";
    defaultAtomicExtensions += ".tar.lzma";
    defaultAtomicExtensions += ".tar.xz";
    defaultAtomicExtensions += ".moc.cpp";

    listBox = createListBox("Look&Feel", "Atomic Extensions", defaultAtomicExtensions, vboxWidget2, true, QString(), PAGE_EXTENSIONS);
    vbox2->addWidget(listBox);
}

void KgGeneral::createGeneralTab()
{
    QWidget *tab = createTab(i18n("General"));
    auto *kgGeneralLayout = new QGridLayout(tab);
    kgGeneralLayout->setSpacing(6);
    kgGeneralLayout->setContentsMargins(11, 11, 11, 11);

    //  -------------------------- GENERAL GROUPBOX ----------------------------------

    QGroupBox *generalGrp = createFrame(i18n("General"), tab);
    QGridLayout *generalGrid = createGridLayout(generalGrp);

    KONFIGURATOR_CHECKBOX_PARAM settings[] = {
        //   cfg_class  cfg_name                default             text                              restart tooltip
        {"Look&Feel",
         "Warn On Exit",
         _WarnOnExit,
         i18n("Warn on exit"),
         false,
         i18n("Display a warning when trying to close the main window.")}, // KDE4: move warn on exit to the other confirmations
        {"Look&Feel",
         "Minimize To Tray",
         _ShowTrayIcon,
         i18n("Show and close to tray"),
         false,
         i18n("Show an icon in the system tray and keep running in the background when the window is closed.")},
    };
    KonfiguratorCheckBoxGroup *cbs = createCheckBoxGroup(2, 0, settings, 2 /*count*/, generalGrp, PAGE_GENERAL);
    generalGrid->addWidget(cbs, 0, 0);

    // temp dir

    auto *hbox = new QHBoxLayout();

    QLabel *labelGrp = new QLabel(i18n("Temp Folder:"), generalGrp);
    hbox->addWidget(labelGrp);
    KonfiguratorURLRequester *urlReq3 = createURLRequester("General", "Temp Directory", _TempDirectory, labelGrp, generalGrp, false, QString(), PAGE_GENERAL);
    urlReq3->setMode(KFile::Directory);
    connect(urlReq3->extension(), &KonfiguratorExtension::applyManually, this, &KgGeneral::applyTempDir);
    hbox->addWidget(urlReq3);
    generalGrid->addLayout(hbox, 13, 0, 1, 1);

    QLabel *label4 = new QLabel(i18n("Note: you must have full permissions for the temporary folder."), generalGrp);
    generalGrid->addWidget(label4, 14, 0, 1, 1);

    kgGeneralLayout->addWidget(generalGrp, 0, 0);

    // ----------------------- delete mode --------------------------------------

    QGroupBox *delGrp = createFrame(i18n("Delete mode"), tab);
    QGridLayout *delGrid = createGridLayout(delGrp);

    KONFIGURATOR_NAME_VALUE_TIP deleteMode[] =
        //            name            value    tooltip
        {{i18n("Move to trash"), "true", i18n("Files will be moved to trash when deleted.")},
         {i18n("Delete files"), "false", i18n("Files will be permanently deleted.")}};
    KonfiguratorRadioButtons *trashRadio =
        createRadioButtonGroup("General", "Move To Trash", _MoveToTrash ? "true" : "false", 2, 0, deleteMode, 2, delGrp, false, PAGE_GENERAL);
    delGrid->addWidget(trashRadio);

    kgGeneralLayout->addWidget(delGrp, 1, 0);

    // ----------------------- terminal -----------------------------------------

    QGroupBox *terminalGrp = createFrame(i18n("Terminal"), tab);
    QGridLayout *terminalGrid = createGridLayout(terminalGrp);

    QLabel *label3 = new QLabel(i18n("External Terminal:"), generalGrp);
    terminalGrid->addWidget(label3, 0, 0);
    KonfiguratorURLRequester *urlReq2 = createURLRequester("General", "Terminal", _Terminal, label3, generalGrp, false, QString(), PAGE_GENERAL, false);
    terminalGrid->addWidget(urlReq2, 0, 1);
    QLabel *terminalLabel = new QLabel(i18n("%d will be replaced by the workdir."), terminalGrp);
    terminalGrid->addWidget(terminalLabel, 1, 1);

    KONFIGURATOR_CHECKBOX_PARAM terminal_settings[] = {
        //   cfg_class  cfg_name     default        text            restart tooltip
        {"General",
         "Send CDs",
         _SendCDs,
         i18n("Embedded Terminal sends Chdir on panel change"),
         false,
         i18n("When checked, whenever the panel is changed (for example, by pressing Tab), Krusader changes the current folder in the embedded terminal.")},
        {"General",
         "Follow Terminal CD",
         _FollowTerminalCD,
         i18n("Active panel follows Embedded Terminal working directory"),
         false,
         i18n("When checked, whenever terminal working directory is changed (for example, by executing 'cd' or 'popd' command), Krusader changes the current "
              "folder of active panel.")},
    };
    cbs = createCheckBoxGroup(1, 0, terminal_settings, 2 /*count*/, terminalGrp, PAGE_GENERAL);
    terminalGrid->addWidget(cbs, 2, 0, 1, 2);

    kgGeneralLayout->addWidget(terminalGrp, 2, 0);
}

void KgGeneral::applyTempDir(QObject *obj, const QString &configGroup, const QString &name)
{
    auto *urlReq = qobject_cast<KonfiguratorURLRequester *>(obj);
    QString value = urlReq->url().toDisplayString(QUrl::PreferLocalFile);

    KConfigGroup(krConfig, configGroup).writeEntry(name, value);
}

void KgGeneral::slotFindTools()
{
    QPointer<KrResultTableDialog> dlg = new KrResultTableDialog(this,
                                                                KrResultTableDialog::Tool,
                                                                i18n("Search results"),
                                                                i18n("Searching for tools..."),
                                                                "tools-wizard",
                                                                i18n("Make sure to install new tools in your <code>$PATH</code> (e.g. /usr/bin)"));
    dlg->exec();
    delete dlg;
}

void KgGeneral::slotAddExtension()
{
    bool ok;
    QString atomExt = QInputDialog::getText(this, i18n("Add new atomic extension"), i18n("Extension:"), QLineEdit::Normal, QString(), &ok);

    if (ok) {
        if (!atomExt.startsWith('.') || atomExt.indexOf('.', 1) == -1)
            KMessageBox::error(krMainWindow, i18n("Atomic extensions must start with '.' and must contain at least one more '.' character."), i18n("Error"));
        else
            listBox->addItem(atomExt);
    }
}

void KgGeneral::slotRemoveExtension()
{
    QList<QListWidgetItem *> list = listBox->selectedItems();

    for (int i = 0; i != list.count(); i++)
        listBox->removeItem(list[i]->text());
}
