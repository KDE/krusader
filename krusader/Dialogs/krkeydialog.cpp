/*****************************************************************************
 * Copyright (C) 2006 Jonas Bähr <jonas.baehr@web.de>                        *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "krkeydialog.h"

#include <QtGui/QLayout>
#include <QtCore/QTextStream>
#include <klocale.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kactioncollection.h>

#include "../krusader.h"
#include "../krglobal.h"

//This is the filter in the KFileDialog of Import/Export:
static const char* FILE_FILTER = I18N_NOOP("*.keymap|Krusader keymaps\n*|all files");


KrKeyDialog::KrKeyDialog(QWidget * parent) : KShortcutsDialog(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsDisallowed /* allow letter shortcuts */, parent)
{
    addCollection(krApp->actionCollection());

    // HACK This fetches the layout of the buttonbox from KDialog, although it is not accessable with KDialog's API
    // None the less it's quite safe to use since this implementation hasn't changed since KDE-3.3 (I haven't looked at earlier
    // versions since we don't support them) and now all work is done in KDE-4.
    QWidget* buttonBox = static_cast<QWidget*>(button(KDialog::Ok)->parent());
    QBoxLayout* buttonBoxLayout = static_cast<QBoxLayout*>(buttonBox->layout());

    KPushButton* importButton = new KPushButton(i18n("Import Shortcuts"), buttonBox);
    importButton->setWhatsThis(i18n("Load a keybinding profile, e.g., total_commander.keymap"));
    buttonBoxLayout->insertWidget(1, importButton);   // the defaults-button should stay on position 0
    connect(importButton, SIGNAL(clicked()), SLOT(slotImportShortcuts()));

    KPushButton* exportButton = new KPushButton(i18n("Export Shortcuts"), buttonBox);
    exportButton->setWhatsThis(i18n("Save current keybindings in a keymap file."));
    buttonBoxLayout->insertWidget(2, exportButton);
    connect(exportButton, SIGNAL(clicked()), SLOT(slotExportShortcuts()));

    // Also quite HACK 'isch but unfortunately KKeyDialog don't giveus access to this widget
    _chooser = static_cast<KShortcutsEditor*>(mainWidget());

    configure(true /* SaveSettings */);   // this runs the dialog
}

KrKeyDialog::~KrKeyDialog()
{
}

void KrKeyDialog::slotImportShortcuts()
{
    // find $KDEDIR/share/apps/krusader
    QString basedir = KGlobal::dirs()->findResourceDir("appdata", "total_commander.keymap");
    // let the user select a file to load
    QString filename = KFileDialog::getOpenFileName(basedir, i18n(FILE_FILTER), 0, i18n("Select a keymap file"));
    if (filename.isEmpty())
        return;

    KConfig conf(filename, KConfig::NoGlobals /*no KDEGlobal*/);
    if (! conf.hasGroup("Shortcuts")) {
        int answer = KMessageBox::warningContinueCancel(this,  //parent
                     i18n("This file does not seem to be a valid keymap.\n"
                          "It may be a keymap using a legacy format. The import cannot be undone!"), //text
                     i18n("Try to import legacy format?"),  //caption
                     KGuiItem(i18n("Import Anyway")),   //Label for the continue-button
                     KStandardGuiItem::cancel(),
                     "Confirm Import Legacy Shortcuts" //dontAskAgainName (for the config-file)
                                                       );
        if (answer == KMessageBox::Continue)
            importLegacyShortcuts(filename);
        else
            return;
    } else
        _chooser->save();
}

void KrKeyDialog::importLegacyShortcuts(const QString& file)
{
    /*
     * This is basicaly Shie's code. It's copied from Kronfigurator's look&feel page and adapted to the dialog
     */
    // check if there's an info file with the keymap
    QFile info(file + ".info");
    if (info.open(QIODevice::ReadOnly)) {
        QTextStream stream(&info);
        QStringList infoText = stream.readAll().split('\n');
        if (KMessageBox::questionYesNoList(krApp, i18n("The following information was attached to the keymap. Do you really want to import this keymap?"), infoText) != KMessageBox::Yes)
            return;
    }

    // ok, import away
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)) {
        krOut << "Error opening " << file << endl;
        return;
    }
    char *actionName;
    QDataStream stream(&f);
    int key;
    QAction *action;
    while (!stream.atEnd()) {
        stream >> actionName >> key;
        action = krApp->actionCollection()->action(actionName);
        if (action) {
            action->setShortcut(key);
//   krOut << "set shortcut for " << actionName <<endl;
        } else {
            krOut << "unknown action " << actionName << endl;
        }
    }
    f.close();

    KMessageBox::information(this,  // parent
                             i18n("Please restart this dialog in order to see the changes"), // text
                             i18n("Legacy Import Completed") // caption
                            );
}

void KrKeyDialog::slotExportShortcuts()
{
    QString filename = KFileDialog::getSaveFileName(QString(), i18n(FILE_FILTER), 0, i18n("Select a keymap file"));
    if (filename.isEmpty())
        return;
    QFile f(filename);
    if (f.exists() &&
            KMessageBox::warningContinueCancel(this,
                                               i18n("<qt>File <b>%1</b> already exists. Do you really want to overwrite it?</qt>", filename),
                                               i18n("Warning"), KGuiItem(i18n("Overwrite")))
            != KMessageBox::Continue)
        return;
    if (f.open(QIODevice::WriteOnly))
        // This is the only way to detect if the file is writable since
        // we don't get feetback from KConfig's sync
        // additionally this prevents merging if the file already contains some
        // shortcuts
        f.close();
    else {
        KMessageBox::error(this, i18n("<qt>Cannot open <b>%1</b> for writing!</qt>", filename));
        return;
    }

    KConfig conf(filename, KConfig::NoGlobals);
    KConfigGroup cg = conf.group("Shortcuts");

    // unfortunately we can't use this function since it only writes the actions which are different from default.
    //krApp->actionCollection()->writeShortcutSettings( "Shortcuts", &conf );
    krApp->actionCollection()->writeSettings(&cg, true /* write all actions */);
    // That does KActionShortcutList::writeSettings for us
    //conf.sync(); // write back all changes
}

#include "krkeydialog.moc"
