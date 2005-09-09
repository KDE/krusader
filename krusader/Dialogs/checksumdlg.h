#ifndef CHECKSUMDLG_H
#define CHECKSUMDLG_H

#include <kdialogbase.h>

class CreateChecksumDlg: public KDialogBase {
//	Q_OBJECT
public:
	CreateChecksumDlg(const QStringList& stdout, const QStringList& stderr, 
		const QString& path, const QString& binary);

protected:
	void saveChecksum(const QStringList& data, QString filename);
	
private:
	QString _binary;
};


#endif // CHECKSUMDLG_H
