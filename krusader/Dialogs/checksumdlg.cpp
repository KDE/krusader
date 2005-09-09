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

CreateChecksumDlg::CreateChecksumDlg(const QStringList& stdout, const QStringList& stderr, 
	const QString& path, const QString& binary):
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
	p.setPixmap(QMessageBox::standardIcon(errors ? QMessageBox::Critical : QMessageBox::Information));
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
	KLineEdit *le=0;
	QCheckBox *cb=0;
	if (successes) {
		QHBoxLayout *hlayout2 = new QHBoxLayout(layout, KDialogBase::spacingHint());
		cb = new QCheckBox(i18n("Save checksum to file"), plainPage());
		cb->setChecked(true);
		hlayout2->addWidget(cb);
	
		le = new KLineEdit(plainPage());
		le->setText(path+"/checksum.md5");
		hlayout2->addWidget(le, Qt::AlignLeft);
		layout->addMultiCellLayout(hlayout2, row, row,0,1, Qt::AlignLeft);
		++row;
		connect(cb, SIGNAL(toggled(bool)), le, SLOT(setEnabled(bool)));	
		le->setFocus();
	}
	
	if (exec() == Accepted && successes && cb->isChecked() &&
		!le->text().simplifyWhiteSpace().isEmpty()) {
		saveChecksum(stdout, le->text());
	}
}

void CreateChecksumDlg::saveChecksum(const QStringList& data, QString filename) {
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
