#ifndef CHECKSUMDLG_H
#define CHECKSUMDLG_H

#include <kdialogbase.h>

class CreateChecksumDlg: public KDialogBase {
//	Q_OBJECT
public:
	CreateChecksumDlg(const QStringList& stdout, const QStringList& stderr, const QString& path);

protected:
	void saveChecksum(const QStringList& data, QString filename);
};


#endif // CHECKSUMDLG_H
