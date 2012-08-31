/***************************************************************************
                         kggeneral.cpp  -  description
                             -------------------
    copyright            : (C) 2004 by Csaba Karai
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

#include "kggeneral.h"

#include <QtGui/QLabel>
#include <QtGui/QFontMetrics>
#include <QGridLayout>
#include <QFrame>
#include <QPixmap>
#include <QPointer>

#include <klocale.h>
#include <kmessagebox.h>
#include <kinputdialog.h>

#include "krresulttabledialog.h"
#include "../defaults.h"
#include "../kicons.h"

#define PAGE_GENERAL        0
#define PAGE_VIEWER         1
#define PAGE_EXTENSIONS     2

KgGeneral::KgGeneral(bool first, QWidget* parent) :
        KonfiguratorPage(first, parent)
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

QWidget* KgGeneral::createTab(QString name)
{
    QScrollArea *scrollArea = new QScrollArea(tabWidget);
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
    QGridLayout *tabLayout = new QGridLayout(tab);
    tabLayout->setSpacing(6);
    tabLayout->setContentsMargins(11, 11, 11, 11);

    tabLayout->addWidget(createCheckBox("General", "View In Separate Window", _ViewInSeparateWindow,
                                   i18n("Internal editor and viewer opens each file in a separate window"), tab, false,
                                   i18n("If checked, each file will open in a separate window, otherwise, the viewer will work in a single, tabbed mode"), PAGE_VIEWER));

    // ------------------------- viewer ----------------------------------

    QGroupBox *viewerGrp = createFrame(i18n("Viewer"), tab);
    tabLayout->addWidget(viewerGrp, 1, 0);

    QGridLayout *viewerGrid = createGridLayout(viewerGrp);

    QWidget * hboxWidget2 = new QWidget(viewerGrp);
    QHBoxLayout * hbox2 = new QHBoxLayout(hboxWidget2);

    QWidget * vboxWidget = new QWidget(hboxWidget2);
    QVBoxLayout * vbox = new QVBoxLayout(vboxWidget);

    vbox->addWidget(new QLabel(i18n("Default viewer mode:"), vboxWidget));

    KONFIGURATOR_NAME_VALUE_TIP viewMode[] =
        //            name            value    tooltip
    {{ i18n("Generic mode"),  "generic", i18n("Use the system's default viewer") },
        { i18n("Text mode"), "text",  i18n("View the file in text-only mode") },
        { i18n("Hex mode"), "hex",  i18n("View the file in hex-mode (better for binary files)") },
        { i18n("Lister mode"), "lister",  i18n("View the file with lister (for huge text files)") }
    };

    vbox->addWidget(createRadioButtonGroup("General", "Default Viewer Mode",
                                           "generic", 0, 4, viewMode, 4, vboxWidget, false, PAGE_VIEWER));


    QWidget * hboxWidget4 = new QWidget(vboxWidget);
    QHBoxLayout * hbox4 = new QHBoxLayout(hboxWidget4);

    QLabel *label5 = new QLabel(i18n("Use lister if the text file is bigger than:"), hboxWidget4);
    hbox4->addWidget(label5);
    KonfiguratorSpinBox *spinBox = createSpinBox("General", "Lister Limit", _ListerLimit,
                                   0, 0x7FFFFFFF, hboxWidget4, false, PAGE_VIEWER);
    hbox4->addWidget(spinBox);
    QLabel *label6 = new QLabel(i18n("MB"), hboxWidget4);
    hbox4->addWidget(label6);

    vbox->addWidget(hboxWidget4);

    hbox2->addWidget(vboxWidget);

    viewerGrid->addWidget(hboxWidget2, 0, 0);


    // ------------------------- editor ----------------------------------

    QGroupBox *editorGrp = createFrame(i18n("Editor"), tab);
    tabLayout->addWidget(editorGrp, 2, 0);
    QGridLayout *editorGrid = createGridLayout(editorGrp);

    QLabel *label1 = new QLabel(i18n("Editor:"), editorGrp);
    editorGrid->addWidget(label1, 0, 0);
    KonfiguratorURLRequester *urlReq = createURLRequester("General", "Editor", "internal editor",
                                       editorGrp, false, PAGE_VIEWER);
    editorGrid->addWidget(urlReq, 0, 1);

    QLabel *label2 = new QLabel(i18n("Hint: use 'internal editor' if you want to use Krusader's fast built-in editor"), editorGrp);
    editorGrid->addWidget(label2, 1, 0, 1, 2);
}

void KgGeneral::createExtensionsTab()
{
    // ------------------------- atomic extensions ----------------------------------

    QWidget *tab = createTab(i18n("Atomic extensions"));
    QGridLayout *tabLayout = new QGridLayout(tab);
    tabLayout->setSpacing(6);
    tabLayout->setContentsMargins(11, 11, 11, 11);

    QWidget * vboxWidget2 = new QWidget(tab);
    tabLayout->addWidget(vboxWidget2);

    QVBoxLayout * vbox2 = new QVBoxLayout(vboxWidget2);

    QWidget * hboxWidget3 = new QWidget(vboxWidget2);
    vbox2->addWidget(hboxWidget3);

    QHBoxLayout * hbox3 = new QHBoxLayout(hboxWidget3);

    QLabel * atomLabel = new QLabel(i18n("Atomic extensions:"), hboxWidget3);
    hbox3->addWidget(atomLabel);

    int size = QFontMetrics(atomLabel->font()).height();

    QToolButton *addButton = new QToolButton(hboxWidget3);
    hbox3->addWidget(addButton);

    QPixmap icon = krLoader->loadIcon("list-add", KIconLoader::Desktop, size);
    addButton->setFixedSize(icon.width() + 4, icon.height() + 4);
    addButton->setIcon(QIcon(icon));
    connect(addButton, SIGNAL(clicked()), this, SLOT(slotAddExtension()));

    QToolButton *removeButton = new QToolButton(hboxWidget3);
    hbox3->addWidget(removeButton);

    icon = krLoader->loadIcon("list-remove", KIconLoader::Desktop, size);
    removeButton->setFixedSize(icon.width() + 4, icon.height() + 4);
    removeButton->setIcon(QIcon(icon));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(slotRemoveExtension()));

    QStringList defaultAtomicExtensions;
    defaultAtomicExtensions += ".tar.gz";
    defaultAtomicExtensions += ".tar.bz2";
    defaultAtomicExtensions += ".tar.lzma";
    defaultAtomicExtensions += ".tar.xz";
    defaultAtomicExtensions += ".moc.cpp";

    listBox = createListBox("Look&Feel", "Atomic Extensions",
                            defaultAtomicExtensions, vboxWidget2, true, PAGE_EXTENSIONS);
    vbox2->addWidget(listBox);
}


void KgGeneral::createGeneralTab()
{
    QWidget *tab = createTab(i18n("General"));
    QGridLayout *kgGeneralLayout = new QGridLayout(tab);
    kgGeneralLayout->setSpacing(6);
    kgGeneralLayout->setContentsMargins(11, 11, 11, 11);


    //  -------------------------- GENERAL GROUPBOX ----------------------------------

    QGroupBox *generalGrp = createFrame(i18n("General"), tab);
    QGridLayout *generalGrid = createGridLayout(generalGrp);



    KONFIGURATOR_CHECKBOX_PARAM settings[] = { //   cfg_class  cfg_name                default             text                              restart tooltip
        {"Look&Feel", "Warn On Exit",         _WarnOnExit,        i18n("Warn on exit"),           false,  i18n("Display a warning when trying to close the main window.") },    // KDE4: move warn on exit to the other confirmations
        {"Look&Feel", "Minimize To Tray",     _MinimizeToTray,    i18n("Minimize to tray"),       false,  i18n("The icon will appear in the system tray instead of the taskbar, when Krusader is minimized.") },
    };
    KonfiguratorCheckBoxGroup *cbs = createCheckBoxGroup(2, 0, settings, 2 /*count*/, generalGrp, PAGE_GENERAL);
    generalGrid->addWidget(cbs, 0, 0);


    KonfiguratorCheckBox *checkBox = createCheckBox("General", "Mimetype Magic", _MimetypeMagic,
                                     i18n("Use mimetype magic"), generalGrp, false,
                                     i18n("Mimetype magic allows better distinction of file types, but is slower."), PAGE_GENERAL);
    generalGrid->addWidget(checkBox, 1, 0, 1, 1);


    // temp dir

    QHBoxLayout *hbox = new QHBoxLayout();

    hbox->addWidget(new QLabel(i18n("Temp Directory:"), generalGrp));
    KonfiguratorURLRequester *urlReq3 = createURLRequester("General", "Temp Directory", "/tmp/krusader.tmp",
                                        generalGrp, false, PAGE_GENERAL);
    urlReq3->setMode(KFile::Directory);
    connect(urlReq3->extension(), SIGNAL(applyManually(QObject *, QString, QString)),
            this, SLOT(applyTempDir(QObject *, QString, QString)));
    hbox->addWidget(urlReq3);
    generalGrid->addLayout(hbox, 13, 0, 1, 1);

    QLabel *label4 = new QLabel(i18n("Note: you must have full permissions for the temporary directory!"),
                                generalGrp);
    generalGrid->addWidget(label4, 14, 0, 1, 1);


    kgGeneralLayout->addWidget(generalGrp, 0 , 0);


    // ----------------------- delete mode --------------------------------------

    QGroupBox *delGrp = createFrame(i18n("Delete mode"), tab);
    QGridLayout *delGrid = createGridLayout(delGrp);

    KONFIGURATOR_NAME_VALUE_TIP deleteMode[] =
        //            name            value    tooltip
    {{ i18n("Delete files"),  "false", i18n("Files will be permanently deleted.") },
        { i18n("Move to trash"), "true",  i18n("Files will be moved to trash when deleted.") }
    };
    KonfiguratorRadioButtons *trashRadio = createRadioButtonGroup("General", "Move To Trash",
                                           _MoveToTrash ? "true" : "false", 2, 0, deleteMode, 2, delGrp, false, PAGE_GENERAL);
    delGrid->addWidget(trashRadio);

    kgGeneralLayout->addWidget(delGrp, 1 , 0);


    // ----------------------- terminal -----------------------------------------

    QGroupBox *terminalGrp = createFrame(i18n("Terminal"), tab);
    QGridLayout *terminalGrid = createGridLayout(terminalGrp);

    QLabel *label3 = new QLabel(i18n("Terminal:"), generalGrp);
    terminalGrid->addWidget(label3, 0, 0);
    KonfiguratorURLRequester *urlReq2 = createURLRequester("General", "Terminal", _Terminal,
                                        generalGrp, false, PAGE_GENERAL, false);
    terminalGrid->addWidget(urlReq2, 0, 1);

    KONFIGURATOR_CHECKBOX_PARAM terminal_settings[] = { //   cfg_class  cfg_name     default        text            restart tooltip
        {"General", "Send CDs", _SendCDs, i18n("Terminal Emulator sends Chdir on panel change"), false, i18n("When checked, whenever the panel is changed (for example, by pressing TAB), krusader changes the current directory in the terminal emulator.") },
        {"Look&Feel", "Fullscreen Terminal Emulator", false, i18n("Fullscreen terminal (mc-style)"), false,  i18n("Terminal is shown instead of the Krusader window (full screen).") },
    };
    cbs = createCheckBoxGroup(1, 0, terminal_settings, 2 /*count*/, terminalGrp, PAGE_GENERAL);
    terminalGrid->addWidget(cbs, 1, 0, 1, 2);


    kgGeneralLayout->addWidget(terminalGrp, 2 , 0);
}

void KgGeneral::applyTempDir(QObject *obj, QString cls, QString name)
{
    KonfiguratorURLRequester *urlReq = (KonfiguratorURLRequester *)obj;
    QString value = QDir(urlReq->url().prettyUrl()).path();

    KConfigGroup(krConfig, cls).writeEntry(name, value);
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
    QString atomExt =
        KInputDialog::getText(i18n("Add new atomic extension"), i18n("Extension:"), QString(), &ok);

    if (ok) {
        if (!atomExt.startsWith('.') || atomExt.indexOf('.', 1) == -1)
            KMessageBox::error(krMainWindow, i18n("Atomic extensions must start with '.'\n and must contain at least one more '.' character"), i18n("Error"));
        else
            listBox->addItem(atomExt);
    }
}

void KgGeneral::slotRemoveExtension()
{
    QList<QListWidgetItem *> list = listBox->selectedItems();

    for (int i = 0; i != list.count(); i++)
        listBox->removeItem(list[ i ]->text());
}

#include "kggeneral.moc"
