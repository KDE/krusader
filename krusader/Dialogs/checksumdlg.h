#ifndef CHECKSUMDLG_H
#define CHECKSUMDLG_H

#include <kdialogbase.h>
#include <q3valuelist.h>

class KTempFile;
extern void initChecksumModule();

class CreateChecksumDlg: public KDialogBase {
public:
	CreateChecksumDlg(const QStringList& files, bool containFolders, const QString& path);

private:
	KTempFile *tmpOut, *tmpErr;
};


class MatchChecksumDlg: public KDialogBase {
public:
	MatchChecksumDlg(const QStringList& files, bool containFolders, 
		const QString& path, const QString& checksumFile=QString());

	static QString checksumTypesFilter;

protected:
	bool verifyChecksumFile(QString path, QString& extension);

private:
	KTempFile *tmpOut, *tmpErr;
};


class ChecksumResultsDlg: public KDialogBase {
public:
	ChecksumResultsDlg(const QStringList& stdOut, const QStringList& stdErr,
		const QString& suggestedFilename, const QString& binary, const QString& type,
		bool standardFormat);

protected:
	bool saveChecksum(const QStringList& data, QString filename);
	void savePerFile(const QStringList& data, const QString& type);
	
private:
	QString _binary;
};


class VerifyResultDlg: public KDialogBase {
public:
	VerifyResultDlg(const QStringList& failed);
};

#endif // CHECKSUMDLG_H
