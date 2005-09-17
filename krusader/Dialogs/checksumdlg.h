#ifndef CHECKSUMDLG_H
#define CHECKSUMDLG_H

#include <kdialogbase.h>
#include <qvaluelist.h>

extern void initChecksumModule();

class CreateChecksumDlg: public KDialogBase {
public:
	CreateChecksumDlg(const QStringList& files, bool containFolders, const QString& path);
};


class MatchChecksumDlg: public KDialogBase {
public:
	MatchChecksumDlg(const QStringList& files, bool containFolders, const QString& path);

protected:
	bool verifyChecksumFile(QString path, QString& extension);
};


class ChecksumResultsDlg: public KDialogBase {
public:
	ChecksumResultsDlg(const QStringList& stdout, const QStringList& stderr, 
		const QString& path, const QString& binary, const QString& type);

protected:
	void saveChecksum(const QStringList& data, QString filename);
	
private:
	QString _binary;
};


class VerifyResultDlg: public KDialogBase {
public:
	VerifyResultDlg(const QStringList& failed);
};

#endif // CHECKSUMDLG_H
