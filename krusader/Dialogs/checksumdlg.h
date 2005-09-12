#ifndef CHECKSUMDLG_H
#define CHECKSUMDLG_H

#include <kdialogbase.h>
#include <qvaluelist.h>

// defines a type of tool (ie: sha1, md5)
typedef struct _toolType {
	QString type; // md5 or sha1
	QString *tools; // list of binaries
	int numTools;
	QString *tools_r; // list of recursive binaries
	int numTools_r;
} ToolType;

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
		const QString& path, const QString& binary);

protected:
	void saveChecksum(const QStringList& data, QString filename);
	
private:
	QString _binary;
};


#endif // CHECKSUMDLG_H
