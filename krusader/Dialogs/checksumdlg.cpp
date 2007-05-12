#include "checksumdlg.h"
#include "../krusader.h"
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>
#include <klineedit.h>
#include <k3listview.h>
#include <qpixmap.h>
#include <kcursor.h>
#include <kmessagebox.h>
#include <qfile.h>
#include <q3textstream.h>
#include <kfiledialog.h>
#include <q3frame.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <qfileinfo.h>
#include <kurlrequester.h>
#include "../krservices.h"
#include <q3ptrlist.h>
#include <qmap.h>
#include <ktempfile.h>
#include <kstandarddirs.h>

class CS_Tool; // forward
typedef void PREPARE_PROC_FUNC(K3Process& proc, CS_Tool *self, const QStringList& files, 
	const QString checksumFile, bool recursive, const QString& stdoutFileName, 
	const QString& stderrFileName,	const QString& type=QString::null);
typedef QStringList GET_FAILED_FUNC(const QStringList& stdOut, const QStringList& stdErr);

class CS_Tool {
public:
	enum Type {
          MD5=0, SHA1, SHA256, TIGER, WHIRLPOOL, SFV, CRC,
          SHA224, SHA384, SHA512,
	  NumOfTypes
	};
	
	Type type;
	QString binary;
	bool recursive;
	bool standardFormat;
	PREPARE_PROC_FUNC *create, *verify;
	GET_FAILED_FUNC *failed;
};

class CS_ToolByType {
public:
	Q3PtrList<CS_Tool> tools, r_tools; // normal and recursive tools	
};

// handles md5sum and sha1sum
void sumCreateFunc(K3Process& proc, CS_Tool *self, const QStringList& files, 
	const QString, bool recursive, const QString& stdoutFileName, 
	const QString& stderrFileName, const QString&) {
	proc.setUseShell(true, "/bin/bash");
	proc << KrServices::fullPathName( self->binary );
	Q_ASSERT(!recursive); 
	proc << files << "1>" << stdoutFileName << "2>" << stderrFileName;	
}

void sumVerifyFunc(K3Process& proc, CS_Tool *self, const QStringList& /* files */, 
	const QString checksumFile, bool recursive, const QString& stdoutFileName, 
	const QString& stderrFileName, const QString& /* type */) {
	proc.setUseShell(true, "/bin/bash");
	proc << KrServices::fullPathName( self->binary );
	Q_ASSERT(!recursive);
	proc << "-c" << checksumFile << "1>" << stdoutFileName << "2>" << stderrFileName;
}

QStringList sumFailedFunc(const QStringList& stdOut, const QStringList& stdErr) {
	// md5sum and sha1sum print "...: FAILED" for failed files and display
	// the number of failures to stderr. so if stderr is empty, we'll assume all is ok
	QStringList result;
	if (stdErr.size()==0) return result;
	result += stdErr;
	// grep for the ":FAILED" substring
	const QString tmp = QString(": FAILED").local8Bit();
	for (uint i=0; i<stdOut.size();++i) {
		if (stdOut[i].find(tmp) != -1)
			result += stdOut[i];
	}
	
	return result;
}

// handles *deep binaries
void deepCreateFunc(K3Process& proc, CS_Tool *self, const QStringList& files, 
	const QString, bool recursive, const QString& stdoutFileName, 
	const QString& stderrFileName, const QString&) {
	proc.setUseShell(true, "/bin/bash");
	proc << KrServices::fullPathName( self->binary );
	if (recursive) proc << "-r";
	proc << "-l" << files << "1>" << stdoutFileName << "2>" << stderrFileName;
}

void deepVerifyFunc(K3Process& proc, CS_Tool *self, const QStringList& files, 
	const QString checksumFile, bool recursive, const QString& stdoutFileName, 
	const QString& stderrFileName, const QString&) {
	proc.setUseShell(true, "/bin/bash");
	proc << KrServices::fullPathName( self->binary );
	if (recursive) proc << "-r";
	proc << "-x" << checksumFile << files << "1>" << stdoutFileName << "2>" << stderrFileName;
}

QStringList deepFailedFunc(const QStringList& stdOut, const QStringList&/* stdErr */) {
	// *deep dumps (via -x) all failed hashes to stdout
	return stdOut;
}

// handles cfv binary
void cfvCreateFunc(K3Process& proc, CS_Tool *self, const QStringList& files, 
	const QString, bool recursive, const QString& stdoutFileName, 
	const QString& stderrFileName, const QString& type) {
	proc.setUseShell(true, "/bin/bash");
	proc << KrServices::fullPathName( self->binary ) << "-C" << "-VV";
	if (recursive) proc << "-rr";
	proc << "-t" << type << "-f-" << "-U" << files << "1>" << stdoutFileName << "2>" << stderrFileName;	
}

void cfvVerifyFunc(K3Process& proc, CS_Tool *self, const QStringList& /* files */, 
	const QString checksumFile, bool recursive, const QString& stdoutFileName, 
	const QString& stderrFileName, const QString&type) {
	proc.setUseShell(true, "/bin/bash");
	proc << KrServices::fullPathName( self->binary ) << "-M";
	if (recursive) proc << "-rr";
	proc << "-U" << "-VV" << "-t" << type << "-f" << checksumFile << "1>" << stdoutFileName << "2>" << stderrFileName;// << files;
}

QStringList cfvFailedFunc(const QStringList& /* stdOut */, const QStringList& stdErr) {
	// cfv dumps all failed hashes to stderr
	return stdErr;
}

// important: this table should be ordered like so that all md5 tools should be
// one after another, and then all sha1 and so on and so forth. they tools must be grouped,
// since the code in getTools() counts on it!
CS_Tool cs_tools[] = {
	// type              binary            recursive   stdFmt           create_func       verify_func      failed_func
	{CS_Tool::MD5,       "md5sum",         false,      true,            sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
	{CS_Tool::MD5,       "md5deep",        true,       true,            deepCreateFunc,   deepVerifyFunc,  deepFailedFunc},
	{CS_Tool::MD5,       "cfv",            true,       true,            cfvCreateFunc,    cfvVerifyFunc,   cfvFailedFunc},
	{CS_Tool::SHA1,      "sha1sum",        false,      true,            sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
	{CS_Tool::SHA1,      "sha1deep",       true,       true,            deepCreateFunc,   deepVerifyFunc,  deepFailedFunc},
	{CS_Tool::SHA1,      "cfv",            true,       true,            cfvCreateFunc,    cfvVerifyFunc,   cfvFailedFunc},
	{CS_Tool::SHA224,    "sha224sum",      false,      true,            sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
	{CS_Tool::SHA256,    "sha256sum",      false,      true,            sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
	{CS_Tool::SHA256,    "sha256deep",     true,       true,            deepCreateFunc,   deepVerifyFunc,  deepFailedFunc},
	{CS_Tool::SHA384,    "sha384sum",      false,      true,            sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
	{CS_Tool::SHA512,    "sha512sum",      false,      true,            sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
	{CS_Tool::TIGER,     "tigerdeep",      true,       true,            deepCreateFunc,   deepVerifyFunc,  deepFailedFunc},
	{CS_Tool::WHIRLPOOL, "whirlpooldeep",  true,       true,            deepCreateFunc,   deepVerifyFunc,  deepFailedFunc},
	{CS_Tool::SFV,       "cfv",            true,       false,           cfvCreateFunc,    cfvVerifyFunc,   cfvFailedFunc},
	{CS_Tool::CRC,       "cfv",            true,       false,           cfvCreateFunc,    cfvVerifyFunc,   cfvFailedFunc},
};

QMap<QString, CS_Tool::Type> cs_textToType;
QMap<CS_Tool::Type, QString> cs_typeToText;

void initChecksumModule() {
	// prepare the dictionaries - pity it has to be manually
	cs_textToType["md5"]=CS_Tool::MD5;
	cs_textToType["sha1"]=CS_Tool::SHA1;
	cs_textToType["sha256"]=CS_Tool::SHA256;
	cs_textToType["sha224"]=CS_Tool::SHA224;
	cs_textToType["sha384"]=CS_Tool::SHA384;
	cs_textToType["sha512"]=CS_Tool::SHA512;
	cs_textToType["tiger"]=CS_Tool::TIGER;
	cs_textToType["whirlpool"]=CS_Tool::WHIRLPOOL;
	cs_textToType["sfv"]=CS_Tool::SFV;
	cs_textToType["crc"]=CS_Tool::CRC;

	cs_typeToText[CS_Tool::MD5]="md5";
	cs_typeToText[CS_Tool::SHA1]="sha1";
	cs_typeToText[CS_Tool::SHA256]="sha256";
	cs_typeToText[CS_Tool::SHA224]="sha224";
	cs_typeToText[CS_Tool::SHA384]="sha384";
	cs_typeToText[CS_Tool::SHA512]="sha512";
	cs_typeToText[CS_Tool::TIGER]="tiger";
	cs_typeToText[CS_Tool::WHIRLPOOL]="whirlpool";
	cs_typeToText[CS_Tool::SFV]="sfv";
	cs_typeToText[CS_Tool::CRC]="crc";

	// build the checksumFilter (for usage in KRQuery)
	QMap<QString, CS_Tool::Type>::Iterator it;
	for (it=cs_textToType.begin(); it!=cs_textToType.end(); ++it)
		MatchChecksumDlg::checksumTypesFilter += ("*."+it.key()+" ");
}

// --------------------------------------------------

// returns a list of tools which can work with recursive or non-recursive mode and are installed
// note: only 1 tool from each type is suggested
static Q3PtrList<CS_Tool> getTools(bool folders) {
	Q3PtrList<CS_Tool> result;
	uint i;
	for (i=0; i < sizeof(cs_tools)/sizeof(CS_Tool); ++i) {
		if (result.last() && result.last()->type == cs_tools[i].type) continue; // 1 from each type please
		if (folders && !cs_tools[i].recursive) continue;
		if (KrServices::cmdExist(cs_tools[i].binary))
			result.append(&cs_tools[i]);
	}
	
	return result;
}

// ------------- CreateChecksumDlg

CreateChecksumDlg::CreateChecksumDlg(const QStringList& files, bool containFolders, const QString& path):
	KDialogBase(Plain, i18n("Create Checksum"), Ok | Cancel, Ok, krApp) {
	
	Q3PtrList<CS_Tool> tools = getTools(containFolders);

	if (tools.count() == 0) { // nothing was suggested?!
		QString error = i18n("<qt>Can't calculate checksum since no supported tool was found. "
			"Please check the <b>Dependencies</b> page in Krusader's settings.</qt>");
		if (containFolders) 
			error += i18n("<qt><b>Note</b>: you've selected directories, and probably have no recursive checksum tool installed."
			" Krusader currently supports <i>md5deep, sha1deep, sha256deep, tigerdeep and cfv</i></qt>");
		KMessageBox::error(0, error);
		return;
	}
	
	Q3GridLayout *layout = new Q3GridLayout( plainPage(), 1, 1,
		KDialogBase::marginHint(), KDialogBase::spacingHint());
	
	int row=0;
		
	// title (icon+text)	
	Q3HBoxLayout *hlayout = new Q3HBoxLayout(layout, KDialogBase::spacingHint());
	QLabel *p = new QLabel(plainPage());
	p->setPixmap(krLoader->loadIcon("binary", KIcon::Desktop, 32));
	hlayout->addWidget(p);
	QLabel *l1 = new QLabel(i18n("About to calculate checksum for the following files") + 
		(containFolders ? i18n(" and folders:") : ":"), plainPage());
	hlayout->addWidget(l1);
	layout->addMultiCellLayout(hlayout, row, row, 0, 1, Qt::AlignLeft); 
	++row;
	
	// file list
	K3ListBox *lb = new K3ListBox(plainPage());
	lb->insertStringList(files);
	layout->addMultiCellWidget(lb, row, row, 0, 1);
	++row;

	// checksum method
	Q3HBoxLayout *hlayout2 = new Q3HBoxLayout(layout, KDialogBase::spacingHint());
	QLabel *l2 = new QLabel(i18n("Select the checksum method:"), plainPage());
	hlayout2->addWidget(l2);
	KComboBox *method = new KComboBox(plainPage());
	// -- fill the combo with available methods
	uint i;
	for ( i=0; i<tools.count(); ++i )
		method->insertItem( cs_typeToText[tools.at(i)->type], i);
	method->setFocus();
	hlayout2->addWidget(method);	
	layout->addMultiCellLayout(hlayout2, row, row, 0, 1, Qt::AlignLeft);
	++row;

	if (exec() != Accepted) return;
	// else implied: run the process
	tmpOut = new KTempFile(locateLocal("tmp", "krusader"), ".stdout" );
	tmpErr = new KTempFile(locateLocal("tmp", "krusader"), ".stderr" );
	K3Process proc;
	CS_Tool *mytool = tools.at(method->currentItem());
	mytool->create(proc, mytool, KrServices::quote(files), QString::null, containFolders, 
		tmpOut->name(), tmpErr->name(), method->currentText());
	
	krApp->startWaiting(i18n("Calculating checksums ..."), 0, true);	
	QApplication::setOverrideCursor( Qt::WaitCursor );
	bool r = proc.start(K3Process::NotifyOnExit, K3Process::AllOutput);
	if (r) while ( proc.isRunning() ) {
		usleep( 500 );
		qApp->processEvents();
		if (krApp->wasWaitingCancelled()) { // user cancelled
			proc.kill();
			QApplication::restoreOverrideCursor();
			return;
		}
   };
	krApp->stopWait();
	QApplication::restoreOverrideCursor();
	if (!r || !proc.normalExit()) {	
		KMessageBox::error(0, i18n("<qt>There was an error while running <b>%1</b>.</qt>").arg(mytool->binary));
		return;
	}
	
	// suggest a filename
	QString suggestedFilename = path + '/';
	if (files.count() > 1) suggestedFilename += ("checksum." + cs_typeToText[mytool->type]);
	else suggestedFilename += (files[0] + '.' + cs_typeToText[mytool->type]);
	// send both stdout and stderr
	QStringList stdOut, stdErr;
	if (!KrServices::fileToStringList(tmpOut->textStream(), stdOut) || 
			!KrServices::fileToStringList(tmpErr->textStream(), stdErr)) {
		KMessageBox::error(krApp, i18n("Error reading stdout or stderr"));
		return;
	}

	ChecksumResultsDlg dlg( stdOut, stdErr, suggestedFilename, mytool->binary, cs_typeToText[mytool->type], mytool->standardFormat);
	tmpOut->unlink(); delete tmpOut;
	tmpErr->unlink(); delete tmpErr;
}

// ------------- MatchChecksumDlg

QString MatchChecksumDlg::checksumTypesFilter;

MatchChecksumDlg::MatchChecksumDlg(const QStringList& files, bool containFolders, 
	const QString& path, const QString& checksumFile):
	KDialogBase(Plain, i18n("Verify Checksum"), Ok | Cancel, Ok, krApp) {
	
	Q3PtrList<CS_Tool> tools = getTools(containFolders);

	if (tools.count() == 0) { // nothing was suggested?!
		QString error = i18n("<qt>Can't verify checksum since no supported tool was found. "
			"Please check the <b>Dependencies</b> page in Krusader's settings.</qt>");
		if (containFolders) 
			error += i18n("<qt><b>Note</b>: you've selected directories, and probably have no recursive checksum tool installed."
			" Krusader currently supports <i>md5deep, sha1deep, sha256deep, tigerdeep and cfv</i></qt>");
		KMessageBox::error(0, error);
		return;
	}
	
	Q3GridLayout *layout = new Q3GridLayout( plainPage(), 1, 1,
		KDialogBase::marginHint(), KDialogBase::spacingHint());
	
	int row=0;
		
	// title (icon+text)	
	Q3HBoxLayout *hlayout = new Q3HBoxLayout(layout, KDialogBase::spacingHint());
	QLabel *p = new QLabel(plainPage());
	p->setPixmap(krLoader->loadIcon("binary", KIcon::Desktop, 32));
	hlayout->addWidget(p);
	QLabel *l1 = new QLabel(i18n("About to verify checksum for the following files") +
		(containFolders ? i18n(" and folders:") : ":"), plainPage());
	hlayout->addWidget(l1);
	layout->addMultiCellLayout(hlayout, row, row, 0, 1, Qt::AlignLeft); 
	++row;
	
	// file list
	K3ListBox *lb = new K3ListBox(plainPage());
	lb->insertStringList(files);
	layout->addMultiCellWidget(lb, row, row, 0, 1);
	++row;

	// checksum file
	Q3HBoxLayout *hlayout2 = new Q3HBoxLayout(layout, KDialogBase::spacingHint());
	QLabel *l2 = new QLabel(i18n("Checksum file:"), plainPage());
	hlayout2->addWidget(l2);
	KUrlRequester *checksumFileReq = new KUrlRequester( plainPage() );
	if (!checksumFile.isEmpty())
		checksumFileReq->setURL(checksumFile);
	checksumFileReq->fileDialog()->setURL(path);
	checksumFileReq->setFocus();
	hlayout2->addWidget(checksumFileReq);
	layout->addMultiCellLayout(hlayout2, row, row, 0, 1, Qt::AlignLeft);

	if (exec() != Accepted) return;
	QString file = checksumFileReq->url();
	QString extension;
	if (!verifyChecksumFile(file, extension)) {
		KMessageBox::error(0, i18n("<qt>Error reading checksum file <i>%1</i>.<br />Please specify a valid checksum file.</qt>").arg(file));
		return;
	}
	
	// do we have a tool for that extension?
	uint i;
	CS_Tool *mytool = 0;
	for ( i=0; i < tools.count(); ++i )
		if (cs_typeToText[tools.at(i)->type] == extension.toLower()) {
			mytool = tools.at(i);
			break;
		}
	if (!mytool) {
		KMessageBox::error(0, i18n("<qt>Krusader can't find a checksum tool that handles %1 on your system. Please check the <b>Dependencies</b> page in Krusader's settings.</qt>").arg(extension));
		return;
	}
	
	// else implied: run the process
	tmpOut = new KTempFile(locateLocal("tmp", "krusader"), ".stdout" );
	tmpErr = new KTempFile(locateLocal("tmp", "krusader"), ".stderr" );
	K3Process proc;
	mytool->verify(proc, mytool, KrServices::quote(files), KrServices::quote(file), containFolders, tmpOut->name(), tmpErr->name(), extension);
	krApp->startWaiting(i18n("Verifying checksums ..."), 0, true);	
	QApplication::setOverrideCursor( Qt::WaitCursor );
	bool r = proc.start(K3Process::NotifyOnExit, K3Process::AllOutput);
	if (r) while ( proc.isRunning() ) {
		usleep( 500 );
  		qApp->processEvents();
		if (krApp->wasWaitingCancelled()) { // user cancelled
			proc.kill();
			QApplication::restoreOverrideCursor();
			return;
		}
   };
	if (!r || !proc.normalExit()) {	
		KMessageBox::error(0, i18n("<qt>There was an error while running <b>%1</b>.</qt>").arg(mytool->binary));
		return;
	}
	QApplication::restoreOverrideCursor();
	krApp->stopWait();
	// send both stdout and stderr
	QStringList stdOut,stdErr;
	if (!KrServices::fileToStringList(tmpOut->textStream(), stdOut) || 
			!KrServices::fileToStringList(tmpErr->textStream(), stdErr)) {
		KMessageBox::error(krApp, i18n("Error reading stdout or stderr"));
		return;
	}
	VerifyResultDlg dlg(mytool->failed(stdOut, stdErr));
	tmpOut->unlink(); delete tmpOut;
	tmpErr->unlink(); delete tmpErr;
}

bool MatchChecksumDlg::verifyChecksumFile(QString path,  QString& extension) {
	QFileInfo f(path);
	if (!f.exists() || f.isDir()) return false;
	// find the extension
	extension = path.mid(path.findRev(".")+1);
	
	// TODO: do we know the extension? if not, ask the user for one
	
	
	return true;
}

// ------------- VerifyResultDlg
VerifyResultDlg::VerifyResultDlg(const QStringList& failed):
	KDialogBase(Plain, i18n("Verify Checksum"), Close, Close, krApp) {
	Q3GridLayout *layout = new Q3GridLayout( plainPage(), 1, 1,
		KDialogBase::marginHint(), KDialogBase::spacingHint());

	bool errors = failed.size()>0;
	int row = 0;
	
	// create the icon and title
	Q3HBoxLayout *hlayout = new Q3HBoxLayout(layout, KDialogBase::spacingHint());
	QLabel p(plainPage());
	p.setPixmap(krLoader->loadIcon(errors ? "messagebox_critical" : "messagebox_info", KIcon::Desktop, 32));
	hlayout->addWidget(&p);
	
	QLabel *l1 = new QLabel((errors ? i18n("Errors were detected while verifying the checksums") :
		i18n("Checksums were verified successfully")), plainPage());
	hlayout->addWidget(l1);
	layout->addMultiCellLayout(hlayout,row,row,0,1, Qt::AlignLeft);
	++row;

	if (errors) { 
		QLabel *l3 = new QLabel(i18n("The following files have failed:"), plainPage());
		layout->addMultiCellWidget(l3, row, row, 0, 1);
		++row;
		K3ListBox *lb2 = new K3ListBox(plainPage());
		lb2->insertStringList(failed);
		layout->addMultiCellWidget(lb2, row, row, 0, 1);
		++row;
	}
		
	exec();
}

// ------------- ChecksumResultsDlg

ChecksumResultsDlg::ChecksumResultsDlg(const QStringList& stdOut, const QStringList& stdErr,
	const QString& suggestedFilename, const QString& binary, const QString& /* type */, bool standardFormat):
	KDialogBase(Plain, i18n("Create Checksum"), Ok | Cancel, Ok, krApp), _binary(binary) {
	Q3GridLayout *layout = new Q3GridLayout( plainPage(), 1, 1,
		KDialogBase::marginHint(), KDialogBase::spacingHint());

	// md5 tools display errors into stderr, so we'll use that to determine the result of the job
	bool errors = stdErr.size()>0;
	bool successes = stdOut.size()>0;
	int row = 0;
	
	// create the icon and title
	Q3HBoxLayout *hlayout = new Q3HBoxLayout(layout, KDialogBase::spacingHint());
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
		K3ListView *lv = new K3ListView(plainPage());
		if(standardFormat){
			lv->addColumn(i18n("Hash"));
			lv->addColumn(i18n("File"));
			lv->setAllColumnsShowFocus(true);
		} else {
			lv->addColumn(i18n("File and hash"));
		}
		for ( QStringList::ConstIterator it = stdOut.begin(); it != stdOut.end(); ++it ) {
			QString line = (*it);
			if(standardFormat) {
				int space = line.find(' ');
				new K3ListViewItem(lv, line.left(space), line.mid(space+2));
			} else {
				new K3ListViewItem(lv, line);
			}	
		}
		layout->addMultiCellWidget(lv, row, row, 0, 1);
		++row;
	}

	if (errors) {
		QFrame *line1 = new Q3Frame( plainPage() );
		line1->setGeometry( QRect( 60, 210, 501, 20 ) );
		line1->setFrameShape( Q3Frame::HLine );
		line1->setFrameShadow( Q3Frame::Sunken );
		layout->addMultiCellWidget(line1, row, row, 0, 1);
		++row;
    
		QLabel *l3 = new QLabel(i18n("Here are the errors received:"), plainPage());
		layout->addMultiCellWidget(l3, row, row, 0, 1);
		++row;
		K3ListBox *lb = new K3ListBox(plainPage());
		lb->insertStringList(stdErr);
		layout->addMultiCellWidget(lb, row, row, 0, 1);
		++row;
	}

	// save checksum to disk, if any hashes are found
	KUrlRequester *checksumFile=0;
	QCheckBox *saveFileCb=0;
	if (successes) {
		Q3HBoxLayout *hlayout2 = new Q3HBoxLayout(layout, KDialogBase::spacingHint());
		saveFileCb = new QCheckBox(i18n("Save checksum to file:"), plainPage());
		saveFileCb->setChecked(true);
		hlayout2->addWidget(saveFileCb);

		checksumFile = new KUrlRequester( suggestedFilename, plainPage() );
		hlayout2->addWidget(checksumFile, Qt::AlignLeft);
		layout->addMultiCellLayout(hlayout2, row, row,0,1, Qt::AlignLeft);
		++row;
		connect(saveFileCb, SIGNAL(toggled(bool)), checksumFile, SLOT(setEnabled(bool)));
		checksumFile->setFocus();
	}
	
	QCheckBox *onePerFile=0;
	if (stdOut.size() > 1 && standardFormat) {
		onePerFile = new QCheckBox(i18n("Checksum file for each source file"), plainPage());
		onePerFile->setChecked(false);
		// clicking this, disables the 'save as' part
		connect(onePerFile, SIGNAL(toggled(bool)), saveFileCb, SLOT(toggle()));
		connect(onePerFile, SIGNAL(toggled(bool)), saveFileCb, SLOT(setDisabled(bool)));
		connect(onePerFile, SIGNAL(toggled(bool)), checksumFile, SLOT(setDisabled(bool)));
		layout->addMultiCellWidget(onePerFile, row, row,0,1, Qt::AlignLeft);
		++row;
	}
	
	if (exec() == Accepted && successes) {
		if (stdOut.size()>1 && standardFormat && onePerFile->isChecked()) {
			savePerFile(stdOut, suggestedFilename.mid(suggestedFilename.findRev('.')));
		} else if (saveFileCb->isEnabled() && saveFileCb->isChecked() && !checksumFile->url().simplified().isEmpty()) {
			saveChecksum(stdOut, checksumFile->url());
		}
	}
}

bool ChecksumResultsDlg::saveChecksum(const QStringList& data, QString filename) {
	if (QFile::exists(filename) &&
		KMessageBox::warningContinueCancel(this,
		i18n("File %1 already exists.\nAre you sure you want to overwrite it?").arg(filename),
		i18n("Warning"), i18n("Overwrite")) != KMessageBox::Continue) {
		// find a better name to save to
		filename = KFileDialog::getSaveFileName(QString::null, "*", 0, i18n("Select a file to save to"));
		if (filename.simplified().isEmpty()) return false;
	} 
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly)) {
		KMessageBox::detailedError(0, i18n("Error saving file %1").arg(filename),
			file.errorString());
		return false;
	}
	Q3TextStream stream(&file);
	for ( QStringList::ConstIterator it = data.constBegin(); it != data.constEnd(); ++it)
		stream << *it << "\n";
	file.close();
	return true;
}

void ChecksumResultsDlg::savePerFile(const QStringList& data, const QString& type) {
	krApp->startWaiting(i18n("Saving checksum files..."), 0);
	for ( QStringList::ConstIterator it = data.begin(); it != data.end(); ++it ) {
			QString line = (*it);
			QString filename = line.mid(line.find(' ')+2)+type;
			if (!saveChecksum(*it, filename)) {
				KMessageBox::error(0, i18n("Errors occured while saving multiple checksums. Stopping"));
				krApp->stopWait();
				return;
			}
	}
	krApp->stopWait();
}
