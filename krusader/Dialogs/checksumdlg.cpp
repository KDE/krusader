#include "checksumdlg.h"
#include "../krusader.h"
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <klineedit.h>
#include <klistview.h>
#include <qpixmap.h>
#include <kmessagebox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <kfiledialog.h>
#include <qframe.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <kurlrequester.h>
#include "../krservices.h"

// this part is so ugly since g++ 3.3.x has a bug regarding arrays containing arrays :-(
// to add a new tool, do the following:
// 1) if you just want to add a binary that does MD5 (or any other supported checksum), just
//    add it to the correct array (ie: md5Binaries or md5Binaries_r) depending if it's recursive or not
// 2) if you want to add a new checksum type, you'll need to
//    * create newBinaries[], newBinaries_r[] and define a new ToolType for it
//    * add it to the end of the ToolType tools[] array

// defines a type of tool (ie: sha1, md5)
typedef struct _toolType {
	QString type; // md5 or sha1
	const char **tools; // list of binaries
	int numTools;
	const char **tools_r; // list of recursive binaries
	int numTools_r;
} ToolType;

#define MAKE_TOOLS(TYPE, BINARIES, BINARIES_R) \
	{ TYPE, BINARIES, sizeof(BINARIES)/sizeof(QString), BINARIES_R, sizeof(BINARIES_R)/sizeof(QString) }

// md5
const char *md5Binaries[] = {"md5sum", "md5deep"};
const char *md5Binaries_r[] = {"md5deep"};
ToolType md5Tools = MAKE_TOOLS("MD5", md5Binaries, md5Binaries_r);

// sha1
const char *sha1Binaries[] = {"sha1sum", "sha1deep"};
const char *sha1Binaries_r[] = {"sha1deep"};
ToolType sha1Tools = MAKE_TOOLS("SHA1", sha1Binaries, sha1Binaries_r);

// sha256
const char *sha256Binaries[] = {"sha256deep"};
const char *sha256Binaries_r[] = {"sha256deep"};
ToolType sha256Tools = MAKE_TOOLS("SHA256", sha256Binaries, sha256Binaries_r);

// tiger
const char *tigerBinaries[] = {"tigerdeep"};
const char *tigerBinaries_r[] = {"tigerdeep"};
ToolType tigerTools = MAKE_TOOLS("Tiger", tigerBinaries, tigerBinaries_r);

// whirlpool
const char *whirlpoolBinaries[] = {"tigerdeep"};
const char *whirlpoolBinaries_r[] = {"tigerdeep"};
ToolType whirlpoolTools = MAKE_TOOLS("Whirlpool", whirlpoolBinaries, whirlpoolBinaries_r);

ToolType tools[] = { md5Tools, sha1Tools, sha256Tools, tigerTools, whirlpoolTools };

// returns a list of tools which can work with recursive or non-recursive mode and are installed
static SuggestedTools getTools(bool folders) {
	SuggestedTools result;
	uint i;
	for (i=0; i < sizeof(tools)/sizeof(ToolType); ++i)
		if (folders) {
			for (int j=0; j < tools[i].numTools_r; ++j) {
				if (KrServices::cmdExist(tools[i].tools_r[j])) {
					result.append(SuggestedTool(tools[i].type, tools[i].tools_r[j]));
					break; // use the first tool we can find
				}
			}
		} else {
			for (int j=0; j < tools[i].numTools; ++j) {
				if (KrServices::cmdExist(tools[i].tools[j])) {
					result.append(SuggestedTool(tools[i].type, tools[i].tools[j]));
					break; // use the first tool we can find
				}
			}
		}
	
	return result;
}

// ------------- CreateChecksumDlg

CreateChecksumDlg::CreateChecksumDlg(const QStringList& files, bool containFolders, const QString& path):
	KDialogBase(Plain, i18n("Create Checksum"), Ok | Cancel, Ok, krApp) {
	
	SuggestedTools tools = getTools(containFolders);

	if (tools.size() == 0) { // nothing was suggested?!
		QString error = i18n("<qt>Can't calculate checksum since no supported tool was found. "
			"Please check the <b>Dependencies</b> page in Krusader's settings.");
		if (containFolders) 
			error += i18n("<qt><b>Note</b>: you've selected directories, and probably have no recursive checksum tool installed."
			" Krusader currently supports <i>md5deep, sha1deep, sha256deep and tigerdeep</i>");
		KMessageBox::error(0, error);
		return;
	}
	
	QGridLayout *layout = new QGridLayout( plainPage(), 1, 1,
		KDialogBase::marginHint(), KDialogBase::spacingHint());
	
	int row=0;
		
	// title (icon+text)	
	QHBoxLayout *hlayout = new QHBoxLayout(layout, KDialogBase::spacingHint());
	QLabel *p = new QLabel(plainPage());
	p->setPixmap(krLoader->loadIcon("binary", KIcon::Desktop, 32));
	hlayout->addWidget(p);
	QLabel *l1 = new QLabel(i18n("About to calculate checksum for the following files") + 
		(containFolders ? i18n(" and folders:") : ":"), plainPage());
	hlayout->addWidget(l1);
	layout->addMultiCellLayout(hlayout, row, row, 0, 1, Qt::AlignLeft); 
	++row;
	
	// file list
	KListBox *lb = new KListBox(plainPage());
	lb->insertStringList(files);
	layout->addMultiCellWidget(lb, row, row, 0, 1);
	++row;

	// checksum method
	QHBoxLayout *hlayout2 = new QHBoxLayout(layout, KDialogBase::spacingHint());
	QLabel *l2 = new QLabel(i18n("Select the checksum method:"), plainPage());
	hlayout2->addWidget(l2);
	KComboBox *method = new KComboBox(plainPage());
	// -- fill the combo with available methods
	SuggestedTools::iterator it;
	int i;
	for ( i=0, it = tools.begin(); it != tools.end(); ++it, ++i )
		method->insertItem((*it).type, i);
	method->setFocus();
	hlayout2->addWidget(method);	
	layout->addMultiCellLayout(hlayout2, row, row, 0, 1, Qt::AlignLeft);
	++row;

	if (exec() != Accepted) return;
	// else implied: run the process
	KEasyProcess proc;
	QString binary = tools[method->currentItem()].binary;
	proc << KrServices::fullPathName( binary );
	if (containFolders) proc << "-r"; // BUG: this works only for md5deep and friends
	proc << files;
	bool r = proc.start(KEasyProcess::Block, KEasyProcess::AllOutput);
	if (r) proc.wait();
	if (!r || !proc.normalExit()) {	
		KMessageBox::error(0, i18n("<qt>There was an error while running <b>%1</b>.").arg(binary));
		return;
	}
	// send both stdout and stderr
	ChecksumResultsDlg dlg(QStringList::split('\n', proc.stdout(), false),
								QStringList::split('\n', proc.stderr(), false),
								path, binary, tools[method->currentItem()].type.lower());

}

// ------------- MatchChecksumDlg
MatchChecksumDlg::MatchChecksumDlg(const QStringList& files, bool containFolders, const QString& path):
	KDialogBase(Plain, i18n("Verify Checksum"), Ok | Cancel, Ok, krApp) {
	
	SuggestedTools tools = getTools(containFolders);

	if (tools.size() == 0) { // nothing was suggested?!
		QString error = i18n("<qt>Can't verify checksum since no supported tool was found. "
			"Please check the <b>Dependencies</b> page in Krusader's settings.");
		if (containFolders) 
			error += i18n("<qt><b>Note</b>: you've selected directories, and probably have no recursive checksum tool installed."
			" Krusader currently supports <i>md5deep, sha1deep, sha256deep and tigerdeep</i>");
		KMessageBox::error(0, error);
		return;
	}
	
	QGridLayout *layout = new QGridLayout( plainPage(), 1, 1,
		KDialogBase::marginHint(), KDialogBase::spacingHint());
	
	int row=0;
		
	// title (icon+text)	
	QHBoxLayout *hlayout = new QHBoxLayout(layout, KDialogBase::spacingHint());
	QLabel *p = new QLabel(plainPage());
	p->setPixmap(krLoader->loadIcon("binary", KIcon::Desktop, 32));
	hlayout->addWidget(p);
	QLabel *l1 = new QLabel(i18n("About to verify checksum for the following files") +
		(containFolders ? i18n(" and folders:") : ":"), plainPage());
	hlayout->addWidget(l1);
	layout->addMultiCellLayout(hlayout, row, row, 0, 1, Qt::AlignLeft); 
	++row;
	
	// file list
	KListBox *lb = new KListBox(plainPage());
	lb->insertStringList(files);
	layout->addMultiCellWidget(lb, row, row, 0, 1);
	++row;

	// checksum file
	QHBoxLayout *hlayout2 = new QHBoxLayout(layout, KDialogBase::spacingHint());
	QLabel *l2 = new QLabel(i18n("Checksum file:"), plainPage());
	hlayout2->addWidget(l2);
	KURLRequester *checksumFile = new KURLRequester( plainPage() );
	hlayout2->addWidget(checksumFile);
	layout->addMultiCellLayout(hlayout2, row, row, 0, 1, Qt::AlignLeft);


/*
	KComboBox *method = new KComboBox(plainPage());
	// -- fill the combo with available methods
	SuggestedTools::iterator it;
	int i;
	for ( i=0, it = tools.begin(); it != tools.end(); ++it, ++i )
		method->insertItem((*it).type, i);
	method->setFocus();
	hlayout2->addWidget(method);	
	layout->addMultiCellLayout(hlayout2, row, row, 0, 1, Qt::AlignLeft);
	++row;
*/
	if (exec() != Accepted) return;
/*	
	// else implied: run the process
	KEasyProcess proc;
	QString binary = tools[method->currentItem()].binary;
	proc << KrServices::fullPathName( binary );
	if (containFolders) proc << "-r"; // BUG: this works only for md5deep and friends
	proc << files;
	bool r = proc.start(KEasyProcess::Block, KEasyProcess::AllOutput);
	if (r) proc.wait();
	if (!r || !proc.normalExit()) {	
		KMessageBox::error(0, i18n("<qt>There was an error while running <b>%1</b>.").arg(binary));
		return;
	}
	// send both stdout and stderr
	ChecksumResultsDlg dlg(QStringList::split('\n', proc.stdout(), false),
								QStringList::split('\n', proc.stderr(), false),
								path, binary, tools[method->currentItem()].type.lower());
*/
}

// ------------- ChecksumResultsDlg

ChecksumResultsDlg::ChecksumResultsDlg(const QStringList& stdout, const QStringList& stderr, 
	const QString& path, const QString& binary, const QString& type):
	KDialogBase(Plain, i18n("Create Checksum"), Ok | Cancel, Ok, krApp), _binary(binary) {
	QGridLayout *layout = new QGridLayout( plainPage(), 1, 1,
		KDialogBase::marginHint(), KDialogBase::spacingHint());

	// md5 tools display errors into stderr, so we'll use that to determine the result of the job
	bool errors = stderr.size()>0;
	bool successes = stdout.size()>0;
	int row = 0;
	
	// create the icon and title
	QHBoxLayout *hlayout = new QHBoxLayout(layout, KDialogBase::spacingHint());
	QLabel p(plainPage());
	p.setPixmap(krLoader->loadIcon(errors ? "messagebox_critical" : "messagebox_info", KIcon::Desktop, 32));
	hlayout->addWidget(&p);
	
	QLabel *l1 = new QLabel((errors ? i18n("Errors were detected while creating the checksums") :
		i18n("Checksums were created successfully")), plainPage());
	hlayout->addWidget(l1);
	layout->addMultiCellLayout(hlayout,row,row,0,1, Qt::AlignLeft);
	++row;

	if (successes) {
		if (errors) {
			QLabel *l2 = new QLabel(i18n("Here are the calculated checksums:"), plainPage());
			layout->addMultiCellWidget(l2, row, row, 0, 1);
			++row;
		}
		KListView *lv = new KListView(plainPage());
		lv->addColumn(i18n("Hash"));
		lv->addColumn(i18n("File"));
		lv->setAllColumnsShowFocus(true);
		for ( QStringList::ConstIterator it = stdout.begin(); it != stdout.end(); ++it ) {
			QString line = (*it).simplifyWhiteSpace();
			int space = line.find(' ');
			new KListViewItem(lv, line.left(space), line.mid(space+1));
		}
		layout->addMultiCellWidget(lv, row, row, 0, 1);
		++row;
	}

	if (errors) {
		QFrame *line1 = new QFrame( plainPage() );
		line1->setGeometry( QRect( 60, 210, 501, 20 ) );
		line1->setFrameShape( QFrame::HLine );
		line1->setFrameShadow( QFrame::Sunken );
		layout->addMultiCellWidget(line1, row, row, 0, 1);
		++row;
    
		QLabel *l3 = new QLabel(i18n("Here are the errors received:"), plainPage());
		layout->addMultiCellWidget(l3, row, row, 0, 1);
		++row;
		KListBox *lb = new KListBox(plainPage());
		lb->insertStringList(stderr);
		layout->addMultiCellWidget(lb, row, row, 0, 1);
		++row;
	}

	// save checksum to disk, if any hashes are found
	KURLRequester *checksumFile=0;
	QCheckBox *cb=0;
	if (successes) {
		QHBoxLayout *hlayout2 = new QHBoxLayout(layout, KDialogBase::spacingHint());
		cb = new QCheckBox(i18n("Save checksum to file:"), plainPage());
		cb->setChecked(true);
		hlayout2->addWidget(cb);

		checksumFile = new KURLRequester( path+"/checksum."+type, plainPage() );
		hlayout2->addWidget(checksumFile, Qt::AlignLeft);
		layout->addMultiCellLayout(hlayout2, row, row,0,1, Qt::AlignLeft);
		++row;
		connect(cb, SIGNAL(toggled(bool)), checksumFile, SLOT(setEnabled(bool)));
		checksumFile->setFocus();
	}
	
	if (exec() == Accepted && successes && cb->isChecked() &&
		!checksumFile->url().simplifyWhiteSpace().isEmpty()) {
		saveChecksum(stdout, checksumFile->url());
	}
}

void ChecksumResultsDlg::saveChecksum(const QStringList& data, QString filename) {
	if (QFile::exists(filename) &&
		KMessageBox::warningContinueCancel(this,
		i18n("File %1 already exists.\nAre you sure you want to overwrite it?").arg(filename),
		i18n("Warning"), i18n("Overwrite")) != KMessageBox::Continue) {
		// find a better name to save to
		filename = KFileDialog::getSaveFileName(QString::null, "*", 0, i18n("Select a file to save to"));
		if (filename.simplifyWhiteSpace().isEmpty()) return;
	} 
	QFile file(filename);
	if (!file.open(IO_WriteOnly)) {
		KMessageBox::detailedError(0, i18n("Error saving file %1").arg(filename),
			file.errorString());
		return;
	}
	QTextStream stream(&file);
	stream << "# " << _binary << " - created by krusader. please don't modify this line" << "\n";
	for ( QStringList::ConstIterator it = data.constBegin(); it != data.constEnd(); ++it)
		stream << *it << "\n";
	file.close();
	return;
}
