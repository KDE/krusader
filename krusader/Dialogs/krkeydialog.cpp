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
#include <qwhatsthis.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include "../krusader.h"

KrKeyDialog::KrKeyDialog( QWidget * parent ) : KKeyDialog( false /* allow letter shortcuts */, parent ) {
   insert( krApp->actionCollection() );

   // HACK This fetches the layout of the buttonbox from KDialogBase, although it is not accessable with KDialogBase's API
   // None the less it's quite save to use since this implementation hasn't changed since KDE-3.3 (I haven't looked at earlier
   // versions since we don't support them) and now all work is done in KDE-4.
   QWidget* buttonBox = static_cast<QWidget*>( actionButton(KDialogBase::Ok)->parent() );
   QBoxLayout* buttonBoxLayout = static_cast<QBoxLayout*>( buttonBox->layout() );

   KPushButton* importButton = new KPushButton( i18n("Import shortcuts"), buttonBox );
   QWhatsThis::add( importButton, i18n( "Load a keybinding profile, e.g., total_commander.keymap" ) );
   buttonBoxLayout->insertWidget( 1, importButton ); // the defaults-button should stay on position 0
   KPushButton* exportButton = new KPushButton( i18n("Export shortcuts"), buttonBox );
   QWhatsThis::add( exportButton, i18n( "Save current keybindings in a keymap file." ) );
   buttonBoxLayout->insertWidget( 2, exportButton );

   connect( importButton, SIGNAL( clicked() ), SLOT( slotImportShortcuts() ) );
   connect( exportButton, SIGNAL( clicked() ), SLOT( slotExportShortcuts() ) );

   configure( true /* SaveSettings */ ); // this runs the dialog
}

KrKeyDialog::~KrKeyDialog() {
}

/* TODO

I suggest to change the binary format from the keymap to an xml-format.
This could be load step by step. This makes it possible to revert the changes
when the user presses cancel and also to update the view.
In addition it's nicer then a binary-format and is portable acress different platforms
XML also saves us from having to files, one for the shortcuts and one for the info.

*/

void KrKeyDialog::slotImportShortcuts() {
/*
 * This is Shie's code. It's copied from Kronfigurator's loog&feel page
 */
	// find $KDEDIR/share/apps/krusader
	QString basedir = KGlobal::dirs()->findResourceDir("appdata", "total_commander.keymap");
	// let the user select a file to load
	QString file = KFileDialog::getOpenFileName(basedir, "*.keymap", 0, i18n("Select a shortcuts file"));
	if (file == QString::null) return;
	// check if there's an info file with the keymap
	QFile info(file+".info");
	if (info.open(IO_ReadOnly)) {
		QTextStream stream(&info);
		QStringList infoText = QStringList::split("\n", stream.read());
		if (KMessageBox::questionYesNoList(krApp, i18n("The following information was attached to the keymap. Are you sure you want to import this keymap ?"), infoText)!=KMessageBox::Yes)
			return;
	}
	// ok, import away
	krApp->importKeyboardShortcuts(file);
//TODO reload after import!
}

void KrKeyDialog::slotExportShortcuts() {
/*
 * This is Shie's code. It's copied from Kronfigurator's loog&feel page
 */
	QString file = KFileDialog::getSaveFileName(QString::null, "*", 0, i18n("Select a shortcuts file"));
	if (file == QString::null) return;
	krApp->exportKeyboardShortcuts(file);
}

#include "krkeydialog.moc"
