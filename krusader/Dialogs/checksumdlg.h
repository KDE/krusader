#ifndef CHECKSUMDLG_H
#define CHECKSUMDLG_H

#include <kdialogbase.h>
#include <qvaluelist.h>

// defines a suggestion: binary and a type
class SuggestedTool {
public:
	SuggestedTool() {}
	SuggestedTool(const QString& t, const QString& b): type(t), binary(b) {}
	QString type; // same as ToolType::type
	QString binary; // binary name
};
typedef QValueList<SuggestedTool> SuggestedTools;

class CreateChecksumDlg: public KDialogBase {
public:
	CreateChecksumDlg(const QStringList& files, bool containFolders, const QString& path);
	
protected:
	SuggestedTools getTools(bool folders);
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


#endif // CHECKSUMDLG_H
