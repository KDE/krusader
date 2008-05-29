#ifndef __krvfsmodel__
#define __krvfsmodel__

#include <QAbstractListModel>
#include <QVector>

class vfs;
class vfile;

class KrVfsModel: public QAbstractListModel {
	Q_OBJECT
	
public:
	KrVfsModel();
	virtual ~KrVfsModel();
	
	inline bool ready() const { return _vfs != 0; }
	void setVfs(vfs* v);
	
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	
protected:
	vfs *_vfs;
	QVector<vfile*> _vfiles;
};
#endif // __krvfsmodel__
