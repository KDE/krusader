#include "checksumdlg.h"
#include "../krusader.h"
#include <klocale.h>
#include <qlayout.h>
#include <klistbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <klineedit.h>
#include <qpixmap.h>
#include <kmessagebox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <kfiledialog.h>

CreateChecksumDlg::CreateChecksumDlg(const QStringList& data, const QString& path):
	KDialogBase(Plain, i18n("Create Checksum"), Ok | Cancel, Ok, krApp) {
	QGridLayout *layout = new QGridLayout( plainPage(), 1, 1,
		KDialogBase::marginHint(), KDialogBase::spacingHint());

	QHBoxLayout *hlayout = new QHBoxLayout(layout, KDialogBase::spacingHint());
	QLabel p(plainPage());
	p.setPixmap(QMessageBox::standardIcon(QMessageBox::Information));
	hlayout->addWidget(&p);

	QLabel *l = new QLabel(i18n("Checksum was created successfully.\nHere's the resulting checksums:"), plainPage());
	hlayout->addWidget(l);
	layout->addMultiCellLayout(hlayout,0,0,0,1, Qt::AlignLeft);

	KListBox *lb = new KListBox(plainPage());
	lb->insertStringList(data);
	layout->addMultiCellWidget(lb, 1,1, 0, 1);

	
	QHBoxLayout *hlayout2 = new QHBoxLayout(layout, KDialogBase::spacingHint());
	QCheckBox *cb = new QCheckBox(i18n("Save checksum to file"), plainPage());
	cb->setChecked(true);
	hlayout2->addWidget(cb);

	KLineEdit *le = new KLineEdit(plainPage());
	le->setText(path+"/checksum.md5");
	hlayout2->addWidget(le, Qt::AlignLeft);
	layout->addMultiCellLayout(hlayout2, 2,2,0,1, Qt::AlignLeft);
	connect(cb, SIGNAL(toggled(bool)), le, SLOT(setEnabled(bool)));	
	le->setFocus();
	
	if (exec() == Accepted && cb->isChecked() && !le->text().simplifyWhiteSpace().isEmpty()) {
		saveChecksum(data, le->text());
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
	for ( QStringList::ConstIterator it = data.constBegin(); it != data.constEnd(); ++it)
		stream << *it << "\n";
	file.close();
	return;
}
