#include "checksumdlg.h"
#include "../krusader.h"
#include "../GUI/krlistwidget.h"
#include "../GUI/krtreewidget.h"
#include <klocale.h>
#include <kprocess.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <QHBoxLayout>
#include <QGridLayout>
#include <klineedit.h>
#include <qpixmap.h>
#include <kcursor.h>
#include <kmessagebox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <qfileinfo.h>
#include <kurlrequester.h>
#include "../krservices.h"
#include <qlist.h>
#include <qmap.h>
#include <ktemporaryfile.h>
#include <kstandarddirs.h>
#include <unistd.h> // for usleep

class CS_Tool; // forward
typedef void PREPARE_PROC_FUNC(KProcess& proc, CS_Tool *self, const QStringList& files,
	const QString checksumFile, bool recursive, const QString& type=QString());
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
	QList<CS_Tool *> tools, r_tools; // normal and recursive tools	
};

// handles md5sum and sha1sum
void sumCreateFunc(KProcess& proc, CS_Tool *self, const QStringList& files,
	const QString, bool recursive, const QString&) {
	proc << KrServices::fullPathName( self->binary );
	Q_ASSERT(!recursive); 
	proc << files;
}

void sumVerifyFunc(KProcess& proc, CS_Tool *self, const QStringList& /* files */,
	const QString checksumFile, bool recursive, const QString& /* type */) {
	proc << KrServices::fullPathName( self->binary );
	Q_ASSERT(!recursive);
	proc << "-c" << checksumFile;
}

QStringList sumFailedFunc(const QStringList& stdOut, const QStringList& stdErr) {
	// md5sum and sha1sum print "...: FAILED" for failed files and display
	// the number of failures to stderr. so if stderr is empty, we'll assume all is ok
	QStringList result;
	if (stdErr.size()==0) return result;
	result += stdErr;
	// grep for the ":FAILED" substring
	const QString tmp = QString(": FAILED").toLocal8Bit();
	for (int i=0; i<stdOut.size();++i) {
		if (stdOut[i].indexOf(tmp) != -1)
			result += stdOut[i];
	}
	
	return result;
}

// handles *deep binaries
void deepCreateFunc(KProcess& proc, CS_Tool *self, const QStringList& files,
	const QString, bool recursive, const QString&) {
	proc << KrServices::fullPathName( self->binary );
	if (recursive) proc << "-r";
	proc << "-l" << files;
}

void deepVerifyFunc(KProcess& proc, CS_Tool *self, const QStringList& files,
	const QString checksumFile, bool recursive, const QString&) {
	proc << KrServices::fullPathName( self->binary );
	if (recursive) proc << "-r";
	proc << "-x" << checksumFile << files;
}

QStringList deepFailedFunc(const QStringList& stdOut, const QStringList&/* stdErr */) {
	// *deep dumps (via -x) all failed hashes to stdout
	return stdOut;
}

// handles cfv binary
void cfvCreateFunc(KProcess& proc, CS_Tool *self, const QStringList& files,
	const QString, bool recursive, const QString& type) {
	proc << KrServices::fullPathName( self->binary ) << "-C" << "-VV";
	if (recursive) proc << "-rr";
	proc << "-t" << type << "-f-" << "-U" << files;
}

void cfvVerifyFunc(KProcess& proc, CS_Tool *self, const QStringList& /* files */,
	const QString checksumFile, bool recursive, const QString&type) {
	proc << KrServices::fullPathName( self->binary ) << "-M";
	if (recursive) proc << "-rr";
	proc << "-U" << "-VV" << "-t" << type << "-f" << checksumFile;// << files;
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
static QList<CS_Tool *> getTools(bool folders) {
	QList<CS_Tool *> result;
	uint i;
	for (i=0; i < sizeof(cs_tools)/sizeof(CS_Tool); ++i) {
		if (!result.isEmpty() && result.last()->type == cs_tools[i].type) continue; // 1 from each type please
		if (folders && !cs_tools[i].recursive) continue;
		if (KrServices::cmdExist(cs_tools[i].binary))
			result.append(&cs_tools[i]);
	}
	
	return result;
}

// ------------- CreateChecksumDlg

CreateChecksumDlg::CreateChecksumDlg(const QStringList& files, bool containFolders, const QString& path):
	KDialog(krApp) {
	setButtons( KDialog::Ok | KDialog::Cancel );
	setDefaultButton( KDialog::Ok );
	setWindowTitle( i18n("Create Checksum") );
	setWindowModality( Qt::WindowModal );
	
	QList<CS_Tool *> tools = getTools(containFolders);

	if (tools.count() == 0) { // nothing was suggested?!
		QString error = i18n("<qt>Can't calculate checksum since no supported tool was found. "
			"Please check the <b>Dependencies</b> page in Krusader's settings.</qt>");
		if (containFolders) 
			error += i18n("<qt><b>Note</b>: you've selected directories, and probably have no recursive checksum tool installed."
			" Krusader currently supports <i>md5deep, sha1deep, sha256deep, tigerdeep and cfv</i></qt>");
		KMessageBox::error(0, error);
		return;
	}
	
	QWidget * widget = new QWidget( this );
	QGridLayout *layout = new QGridLayout( widget );
	layout->setContentsMargins( KDialog::marginHint(), KDialog::marginHint(), KDialog::marginHint(), KDialog::marginHint() );
	layout->setSpacing( KDialog::spacingHint());
	
	int row=0;
		
	// title (icon+text)	
	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->setSpacing( KDialog::spacingHint() );
	QLabel *p = new QLabel(widget);
	p->setPixmap(krLoader->loadIcon("binary", KIconLoader::Desktop, 32));
	hlayout->addWidget(p);
	QLabel *l1 = new QLabel(i18n("About to calculate checksum for the following files") + 
		(containFolders ? i18n(" and folders:") : ":"), widget);
	hlayout->addWidget(l1);
	layout->addLayout(hlayout, row, 0, 1, 2, Qt::AlignLeft); 
	++row;
	
	// file list
	KrListWidget *lb = new KrListWidget(widget);
	lb->addItems(files);
	layout->addWidget(lb, row, 0, 1, 2);
	++row;

	// checksum method
	QHBoxLayout *hlayout2 = new QHBoxLayout;
	hlayout2->setSpacing( KDialog::spacingHint() );
	QLabel *l2 = new QLabel(i18n("Select the checksum method:"), widget);
	hlayout2->addWidget(l2);
	KComboBox *method = new KComboBox(widget);
	// -- fill the combo with available methods
	int i;
	for ( i=0; i<tools.count(); ++i )
		method->addItem( cs_typeToText[tools.at(i)->type], i);
	method->setFocus();
	hlayout2->addWidget(method);	
	layout->addLayout(hlayout2, row, 0, 1, 2, Qt::AlignLeft);
	++row;
	setMainWidget( widget );

	if (exec() != Accepted) return;
	// else implied: run the process
	tmpOut = new KTemporaryFile();
	tmpOut->setSuffix(".stdout");
	tmpOut->open(); // nessesary to create the filename
	tmpErr = new KTemporaryFile();
	tmpErr->setSuffix(".stderr");
	tmpErr->open(); // nessesary to create the filename
	KProcess proc;
	CS_Tool *mytool = tools.at(method->currentIndex());
	mytool->create(proc, mytool, files, QString(), containFolders, method->currentText());
	proc.setOutputChannelMode(KProcess::SeparateChannels); // without this the next 2 lines have no effect!
	proc.setStandardOutputFile(tmpOut->fileName());
	proc.setStandardErrorFile(tmpErr->fileName());
	proc.setWorkingDirectory(path);
	
	krApp->startWaiting(i18n("Calculating checksums ..."), 0, true);	
	QApplication::setOverrideCursor( Qt::WaitCursor );
	proc.start();
	// TODO make use of asynchronous process starting. waitForStarted(int msec = 30000) is blocking
	// it would be better to connect to started(), error() and finished()
	if (proc.waitForStarted()) while ( proc.state() == QProcess::Running ) {
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
	if (proc.exitStatus() != QProcess::NormalExit) {
		KMessageBox::error(0, i18n("<qt>There was an error while running <b>%1</b>.</qt>", mytool->binary));
		return;
	}
	
	// suggest a filename
	QString suggestedFilename = path + '/';
	if (files.count() > 1) suggestedFilename += ("checksum." + cs_typeToText[mytool->type]);
	else suggestedFilename += (files[0] + '.' + cs_typeToText[mytool->type]);
	// send both stdout and stderr
	QStringList stdOut, stdErr;
	if (!KrServices::fileToStringList(tmpOut, stdOut) ||
			!KrServices::fileToStringList(tmpErr, stdErr)) {
		KMessageBox::error(krApp, i18n("Error reading stdout or stderr"));
		return;
	}

	ChecksumResultsDlg dlg( stdOut, stdErr, suggestedFilename, mytool->binary, cs_typeToText[mytool->type], mytool->standardFormat);
	delete tmpOut; // this also unlinks the files
	delete tmpErr;
}

// ------------- MatchChecksumDlg

QString MatchChecksumDlg::checksumTypesFilter;

MatchChecksumDlg::MatchChecksumDlg(const QStringList& files, bool containFolders, 
	const QString& path, const QString& checksumFile):
	KDialog(krApp) {
	setButtons( KDialog::Ok | KDialog::Cancel );
	setDefaultButton( KDialog::Ok );
	setWindowTitle( i18n("Verify Checksum") );
	setWindowModality( Qt::WindowModal );
	
	QList<CS_Tool *> tools = getTools(containFolders);

	if (tools.count() == 0) { // nothing was suggested?!
		QString error = i18n("<qt>Can't verify checksum since no supported tool was found. "
			"Please check the <b>Dependencies</b> page in Krusader's settings.</qt>");
		if (containFolders) 
			error += i18n("<qt><b>Note</b>: you've selected directories, and probably have no recursive checksum tool installed."
			" Krusader currently supports <i>md5deep, sha1deep, sha256deep, tigerdeep and cfv</i></qt>");
		KMessageBox::error(0, error);
		return;
	}
	
	QWidget * widget = new QWidget( this );
	QGridLayout *layout = new QGridLayout( widget );
	layout->setContentsMargins( KDialog::marginHint(), KDialog::marginHint(), KDialog::marginHint(), KDialog::marginHint() );
	layout->setSpacing( KDialog::spacingHint());
	
	int row=0;
		
	// title (icon+text)	
	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->setSpacing( KDialog::spacingHint() );
	QLabel *p = new QLabel(widget);
	p->setPixmap(krLoader->loadIcon("binary", KIconLoader::Desktop, 32));
	hlayout->addWidget(p);
	QLabel *l1 = new QLabel(i18n("About to verify checksum for the following files") +
		(containFolders ? i18n(" and folders:") : ":"), widget);
	hlayout->addWidget(l1);
	layout->addLayout(hlayout, row, 0, 1, 2, Qt::AlignLeft); 
	++row;
	
	// file list
	KrListWidget *lb = new KrListWidget(widget);
	lb->addItems(files);
	layout->addWidget(lb, row, 0, 1, 2);
	++row;

	// checksum file
	QHBoxLayout *hlayout2 = new QHBoxLayout;
	hlayout2->setSpacing( KDialog::spacingHint() );
	QLabel *l2 = new QLabel(i18n("Checksum file:"), widget);
	hlayout2->addWidget(l2);
	KUrlRequester *checksumFileReq = new KUrlRequester( widget );
	if (!checksumFile.isEmpty())
		checksumFileReq->setUrl(checksumFile);
	checksumFileReq->fileDialog()->setUrl(path);
	checksumFileReq->setFocus();
	hlayout2->addWidget(checksumFileReq);
	layout->addLayout(hlayout2, row, 0, 1, 2, Qt::AlignLeft);
	setMainWidget( widget );

	if (exec() != Accepted) return;
	QString file = checksumFileReq->url().pathOrUrl();
	QString extension;
	if (!verifyChecksumFile(file, extension)) {
		KMessageBox::error(0, i18n("<qt>Error reading checksum file <i>%1</i>.<br />Please specify a valid checksum file.</qt>", file));
		return;
	}
	
	// do we have a tool for that extension?
	int i;
	CS_Tool *mytool = 0;
	for ( i=0; i < tools.count(); ++i )
		if (cs_typeToText[tools.at(i)->type] == extension.toLower()) {
			mytool = tools.at(i);
			break;
		}
	if (!mytool) {
		KMessageBox::error(0, i18n("<qt>Krusader can't find a checksum tool that handles %1 on your system. Please check the <b>Dependencies</b> page in Krusader's settings.</qt>", extension));
		return;
	}
	
	// else implied: run the process
	tmpOut = new KTemporaryFile();
	tmpOut->setSuffix(".stdout");
	tmpOut->open(); // nessesary to create the filename
	tmpErr = new KTemporaryFile();
	tmpErr->setSuffix(".stderr");
	tmpErr->open(); // nessesary to create the filename
	KProcess proc;
	mytool->verify(proc, mytool, files, file, containFolders, extension);
	proc.setOutputChannelMode(KProcess::SeparateChannels); // without this the next 2 lines have no effect!
	proc.setStandardOutputFile(tmpOut->fileName());
	proc.setStandardErrorFile(tmpErr->fileName());
	proc.setWorkingDirectory(path);

	krApp->startWaiting(i18n("Verifying checksums ..."), 0, true);	
	QApplication::setOverrideCursor( Qt::WaitCursor );
	proc.start();
	// TODO make use of asynchronous process starting. waitForStarted(int msec = 30000) is blocking
	// it would be better to connect to started(), error() and finished()
	if (proc.waitForStarted()) while ( proc.state() == QProcess::Running ) {
		usleep( 500 );
  		qApp->processEvents();
		if (krApp->wasWaitingCancelled()) { // user cancelled
			proc.kill();
			QApplication::restoreOverrideCursor();
			return;
		}
   };
	if (proc.exitStatus() != QProcess::NormalExit) {
		KMessageBox::error(0, i18n("<qt>There was an error while running <b>%1</b>.</qt>", mytool->binary));
		return;
	}
	QApplication::restoreOverrideCursor();
	krApp->stopWait();
	// send both stdout and stderr
	QStringList stdOut,stdErr;
	if (!KrServices::fileToStringList(tmpOut, stdOut) ||
			!KrServices::fileToStringList(tmpErr, stdErr)) {
		KMessageBox::error(krApp, i18n("Error reading stdout or stderr"));
		return;
	}
	VerifyResultDlg dlg(mytool->failed(stdOut, stdErr));
	delete tmpOut;
	delete tmpErr;
}

bool MatchChecksumDlg::verifyChecksumFile(QString path,  QString& extension) {
	QFileInfo f(path);
	if (!f.exists() || f.isDir()) return false;
	// find the extension
	extension = path.mid(path.lastIndexOf(".")+1);
	
	// TODO: do we know the extension? if not, ask the user for one
	
	
	return true;
}

// ------------- VerifyResultDlg
VerifyResultDlg::VerifyResultDlg(const QStringList& failed):
	KDialog(krApp) {
	setButtons( KDialog::Close );
	setDefaultButton( KDialog::Close );
	setWindowTitle( i18n("Verify Checksum") );
	setWindowModality( Qt::WindowModal );
	
	QWidget * widget = new QWidget( this );
	QGridLayout *layout = new QGridLayout( widget );
	layout->setContentsMargins( KDialog::marginHint(), KDialog::marginHint(), KDialog::marginHint(), KDialog::marginHint() );
	layout->setSpacing( KDialog::spacingHint());

	bool errors = failed.size()>0;
	int row = 0;
	
	// create the icon and title
	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->setSpacing( KDialog::spacingHint() );
	QLabel p(widget);
	p.setPixmap(krLoader->loadIcon(errors ? "messagebox_critical" : "messagebox_info", KIconLoader::Desktop, 32));
	hlayout->addWidget(&p);
	
	QLabel *l1 = new QLabel((errors ? i18n("Errors were detected while verifying the checksums") :
		i18n("Checksums were verified successfully")), widget);
	hlayout->addWidget(l1);
	layout->addLayout(hlayout,row,0,1,2, Qt::AlignLeft);
	++row;

	if (errors) { 
		QLabel *l3 = new QLabel(i18n("The following files have failed:"), widget);
		layout->addWidget(l3, row, 0, 1, 2);
		++row;
		KrListWidget *lb2 = new KrListWidget(widget);
		lb2->addItems(failed);
		layout->addWidget(lb2, row, 0, 1, 2);
		++row;
	}
		
	setMainWidget( widget );
	exec();
}

// ------------- ChecksumResultsDlg

ChecksumResultsDlg::ChecksumResultsDlg(const QStringList& stdOut, const QStringList& stdErr,
	const QString& suggestedFilename, const QString& binary, const QString& /* type */, bool standardFormat):
	KDialog(krApp), _binary(binary) {
	setButtons( KDialog::Ok | KDialog::Cancel );
	setDefaultButton( KDialog::Ok );
	setWindowTitle( i18n("Create Checksum") );
	setWindowModality( Qt::WindowModal );
	
	QWidget * widget = new QWidget( this );
	QGridLayout *layout = new QGridLayout( widget );
	layout->setContentsMargins( KDialog::marginHint(), KDialog::marginHint(), KDialog::marginHint(), KDialog::marginHint() );
	layout->setSpacing( KDialog::spacingHint());

	// md5 tools display errors into stderr, so we'll use that to determine the result of the job
	bool errors = stdErr.size()>0;
	bool successes = stdOut.size()>0;
	int row = 0;
	
	// create the icon and title
	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->setSpacing( KDialog::spacingHint() );
	QLabel p(widget);
	p.setPixmap(krLoader->loadIcon(errors ? "messagebox_critical" : "messagebox_info", KIconLoader::Desktop, 32));
	hlayout->addWidget(&p);
	
	QLabel *l1 = new QLabel((errors ? i18n("Errors were detected while creating the checksums") :
		i18n("Checksums were created successfully")), widget);
	hlayout->addWidget(l1);
	layout->addLayout(hlayout,row,0,1,2, Qt::AlignLeft);
	++row;

	if (successes) {
		if (errors) {
			QLabel *l2 = new QLabel(i18n("Here are the calculated checksums:"), widget);
			layout->addWidget(l2, row, 0, 1, 2);
			++row;
		}
		KrTreeWidget *lv = new KrTreeWidget(widget);
		
		QStringList columns;
		if(standardFormat){
			columns << i18n("Hash");
			columns << i18n("File");
			lv->setAllColumnsShowFocus(true);
		} else {
			columns << i18n("File and hash");
		}
		lv->setHeaderLabels( columns );
		
		for ( QStringList::ConstIterator it = stdOut.begin(); it != stdOut.end(); ++it ) {
			QString line = (*it);
			if(standardFormat) {
				int space = line.indexOf(' ');
				QTreeWidgetItem * item = new QTreeWidgetItem( lv );
				item->setText(0, line.left(space));
				item->setText(1, line.mid(space+2));
			} else {
				QTreeWidgetItem * item = new QTreeWidgetItem( lv );
				item->setText(0, line);
			}	
		}
		lv->sortItems( standardFormat ? 1 : 0, Qt::AscendingOrder );
		
		layout->addWidget(lv, row, 0, 1, 2);
		++row;
	}

	if (errors) {
		QFrame *line1 = new QFrame( widget );
		line1->setGeometry( QRect( 60, 210, 501, 20 ) );
		line1->setFrameShape( QFrame::HLine );
		line1->setFrameShadow( QFrame::Sunken );
		layout->addWidget(line1, row, 0, 1, 2);
		++row;
    
		QLabel *l3 = new QLabel(i18n("Here are the errors received:"), widget);
		layout->addWidget(l3, row, 0, 1, 2);
		++row;
		KrListWidget *lb = new KrListWidget(widget);
		lb->addItems(stdErr);
		layout->addWidget(lb, row, 0, 1, 2);
		++row;
	}

	// save checksum to disk, if any hashes are found
	KUrlRequester *checksumFile=0;
	QCheckBox *saveFileCb=0;
	if (successes) {
		QHBoxLayout *hlayout2 = new QHBoxLayout;
		hlayout2->setSpacing( KDialog::spacingHint() );
		saveFileCb = new QCheckBox(i18n("Save checksum to file:"), widget);
		saveFileCb->setChecked(true);
		hlayout2->addWidget(saveFileCb);

		checksumFile = new KUrlRequester( suggestedFilename, widget );
		hlayout2->addWidget(checksumFile, Qt::AlignLeft);
		layout->addLayout(hlayout2, row, 0,1,2, Qt::AlignLeft);
		++row;
		connect(saveFileCb, SIGNAL(toggled(bool)), checksumFile, SLOT(setEnabled(bool)));
		checksumFile->setFocus();
	}
	
	QCheckBox *onePerFile=0;
	if (stdOut.size() > 1 && standardFormat) {
		onePerFile = new QCheckBox(i18n("Checksum file for each source file"), widget);
		onePerFile->setChecked(false);
		// clicking this, disables the 'save as' part
		connect(onePerFile, SIGNAL(toggled(bool)), saveFileCb, SLOT(toggle()));
		connect(onePerFile, SIGNAL(toggled(bool)), saveFileCb, SLOT(setDisabled(bool)));
		connect(onePerFile, SIGNAL(toggled(bool)), checksumFile, SLOT(setDisabled(bool)));
		layout->addWidget(onePerFile, row, 0,1,2, Qt::AlignLeft);
		++row;
	}
	setMainWidget( widget );
	
	if (exec() == Accepted && successes) {
		if (stdOut.size()>1 && standardFormat && onePerFile->isChecked()) {
			savePerFile(stdOut, suggestedFilename.mid(suggestedFilename.lastIndexOf('.')));
		} else if (saveFileCb->isEnabled() && saveFileCb->isChecked() && !checksumFile->url().isEmpty()) {
			saveChecksum(stdOut, checksumFile->url().pathOrUrl());
		}
	}
}

bool ChecksumResultsDlg::saveChecksum(const QStringList& data, QString filename) {
	if (QFile::exists(filename) &&
		KMessageBox::warningContinueCancel(this,
		i18n("File %1 already exists.\nAre you sure you want to overwrite it?", filename),
		i18n("Warning"), KGuiItem( i18n("Overwrite"))) != KMessageBox::Continue) {
		// find a better name to save to
		filename = KFileDialog::getSaveFileName(QString(), "*", 0, i18n("Select a file to save to"));
		if (filename.simplified().isEmpty()) return false;
	} 
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly)) {
		KMessageBox::detailedError(0, i18n("Error saving file %1", filename),
			file.errorString());
		return false;
	}
	QTextStream stream(&file);
	for ( QStringList::ConstIterator it = data.constBegin(); it != data.constEnd(); ++it)
		stream << *it << "\n";
	file.close();
	return true;
}

void ChecksumResultsDlg::savePerFile(const QStringList& data, const QString& type) {
	krApp->startWaiting(i18n("Saving checksum files..."), 0);
	for ( QStringList::ConstIterator it = data.begin(); it != data.end(); ++it ) {
			QString line = (*it);
			QString filename = line.mid(line.indexOf(' ')+2)+type;
			QStringList l;
			l << line;
			if (!saveChecksum(l, filename)) {
				KMessageBox::error(0, i18n("Errors occured while saving multiple checksums. Stopping"));
				krApp->stopWait();
				return;
			}
	}
	krApp->stopWait();
}
