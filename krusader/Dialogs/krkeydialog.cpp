//
// C++ Implementation: krkeydialog
//
// Description: 
//
//
// Author: Jonas Bähr <http://jonas-baehr.de/>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "krkeydialog.h"

#include <qlayout.h>
#include <q3whatsthis.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kactionshortcutlist.h>
#include <kdebug.h>

#include "../krusader.h"

//This is the filter in the KFileDialog of Import/Export:
static const char* FILE_FILTER = I18N_NOOP("*.keymap|Krusader keymaps\n*|all files");


KrKeyDialog::KrKeyDialog( QWidget * parent ) : KKeyDialog( false /* allow letter shortcuts */, parent ) {
   insert( krApp->actionCollection() );

   // HACK This fetches the layout of the buttonbox from KDialogBase, although it is not accessable with KDialogBase's API
   // None the less it's quite save to use since this implementation hasn't changed since KDE-3.3 (I haven't looked at earlier
   // versions since we don't support them) and now all work is done in KDE-4.
   QWidget* buttonBox = static_cast<QWidget*>( actionButton(KDialogBase::Ok)->parent() );
   Q3BoxLayout* buttonBoxLayout = static_cast<Q3BoxLayout*>( buttonBox->layout() );

   KPushButton* importButton = new KPushButton( i18n("Import shortcuts"), buttonBox );
   Q3WhatsThis::add( importButton, i18n( "Load a keybinding profile, e.g., total_commander.keymap" ) );
   buttonBoxLayout->insertWidget( 1, importButton ); // the defaults-button should stay on position 0
   connect( importButton, SIGNAL( clicked() ), SLOT( slotImportShortcuts() ) );

   KPushButton* exportButton = new KPushButton( i18n("Export shortcuts"), buttonBox );
   Q3WhatsThis::add( exportButton, i18n( "Save current keybindings in a keymap file." ) );
   buttonBoxLayout->insertWidget( 2, exportButton );
   connect( exportButton, SIGNAL( clicked() ), SLOT( slotExportShortcuts() ) );

   // Also quite HACK 'isch but unfortunately KKeyDialog don't giveus access to this widget
   _chooser = static_cast<KKeyChooser*>( mainWidget() );

   configure( true /* SaveSettings */ ); // this runs the dialog
}

KrKeyDialog::~KrKeyDialog() {
}

void KrKeyDialog::slotImportShortcuts() {
   // find $KDEDIR/share/apps/krusader
   QString basedir = KGlobal::dirs()->findResourceDir("appdata", "total_commander.keymap");
   // let the user select a file to load
   QString filename = KFileDialog::getOpenFileName(basedir, i18n(FILE_FILTER), 0, i18n("Select a keymap file"));
   if ( filename.isEmpty() )
      return;

   KConfig conf( filename, true /*read only*/, false /*no KDEGlobal*/ );
   if ( ! conf.hasGroup("Shortcuts") ) {
      int answer = KMessageBox::warningContinueCancel( this,	//parent
		i18n("This file does not seem to be a valid keymap.\n"
			"It may be a keymap using a legacy format. The import can't be undone!"),	//text
		i18n("Try to import legacy format?"), 	//caption
		i18n("Import anyway"),	//Label for the continue-button
		"Confirm Import Legacy Shortcuts"	//dontAskAgainName (for the config-file)
	);
      if ( answer == KMessageBox::Continue )
         importLegacyShortcuts( filename );
      else
         return;
   }
   else
      _chooser->syncToConfig( "Shortcuts", &conf, false /* don't delete shortcuts of actions not listed in conf */ );
}

void KrKeyDialog::importLegacyShortcuts( const QString& file ) {
/*
 * This is basicaly Shie's code. It's copied from Kronfigurator's loog&feel page and adapted to the dialog
 */
	// check if there's an info file with the keymap
	QFile info(file+".info");
	if (info.open(QIODevice::ReadOnly)) {
		Q3TextStream stream(&info);
		QStringList infoText = QStringList::split("\n", stream.read());
		if (KMessageBox::questionYesNoList(krApp, i18n("The following information was attached to the keymap. Do you really want to import this keymap?"), infoText)!=KMessageBox::Yes)
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
	KAction *action;
	while (!stream.atEnd()) {
		stream >> actionName >> key;
		action = krApp->actionCollection()->action(actionName);
		if (action) {
			action->setShortcut(key);
//			krOut << "set shortcut for " << actionName <<endl;
		} else {
		   krOut << "unknown action " << actionName << endl;
		}
	}
	f.close();

	KMessageBox::information( this, // parent
		i18n("Please restart this dialog in order to see the changes"), // text
		i18n("Legacy import completed") // caption
		);
}

void KrKeyDialog::slotExportShortcuts() {
   QString filename = KFileDialog::getSaveFileName( QString::null, i18n(FILE_FILTER), 0, i18n("Select a keymap file") );
   if ( filename.isEmpty() )
      return;
   QFile f( filename );
   if ( f.exists() &&
   		KMessageBox::warningContinueCancel( this, 
		i18n("<qt>File <b>%1</b> already exists. Do you really want to overwrite it?</qt>").arg(filename),
		i18n("Warning"), i18n("Overwrite") )
	!= KMessageBox::Continue)
	return;
   if ( f.open( QIODevice::WriteOnly ) )
      // This is the only way to detect if the file is writable since we don't get feetback from KConfig's sync
      // Additionaly this prevents merging if the file already contains some shortcuts
      f.close();
   else {
      KMessageBox::error( this, i18n("<qt>Can't open <b>%1</b> for writing!</qt>").arg(filename) );
      return;
   }

   KConfig conf( filename, false /*read only*/, false /*no KDEGlobal*/ );

   // unfortunately we can't use this function since it only writes the actions which are different from default.
   //krApp->actionCollection()->writeShortcutSettings( "Shortcuts", &conf );
   KActionShortcutList list( krApp->actionCollection() );
   list.writeSettings( "Shortcuts", &conf, true /* write all actions */ );
   // That does KActionShortcutList::writeSettings for us
   //conf.sync(); // write back all changes
}

#include "krkeydialog.moc"
