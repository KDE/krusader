#ifndef CHECKSUMDLG_H
#define CHECKSUMDLG_H

#include <kdialog.h>
#include <q3valuelist.h>

class KTempFile;
extern void initChecksumModule();

class CreateChecksumDlg: public KDialog {
public:
	CreateChecksumDlg(const QStringList& files, bool containFolders, const QString& path);

private:
	KTempFile *tmpOut, *tmpErr;
};


class MatchChecksumDlg: public KDialog {
public:
	MatchChecksumDlg(const QStringList& files, bool containFolders, 
		const QString& path, const QString& checksumFile=QString());

	static QString checksumTypesFilter;

protected:
	bool verifyChecksumFile(QString path, QString& extension);

private:
	KTempFile *tmpOut, *tmpErr;
};


class ChecksumResultsDlg: public KDialog {
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


class VerifyResultDlg: public KDialog {
public:
	VerifyResultDlg(const QStringList& failed);
};

#endif // CHECKSUMDLG_H
