#ifndef KRADDBOOKMARKDLG_H
#define KRADDBOOKMARKDLG_H

#include "krbookmark.h"
#include "../VFS/vfs.h"
#include "../GUI/krtreewidget.h"
#include <kdialog.h>
#include <kurl.h>
#include <klineedit.h>
#include <qmap.h>
#include <QtGui/QToolButton>

class KrAddBookmarkDlg: public KDialog
{
    Q_OBJECT
public:
    KrAddBookmarkDlg(QWidget *parent, KUrl url = KUrl());
    KUrl url() const {
        return KUrl(_url->text());
    }
    QString name() const {
        return _name->text();
    }
    KrBookmark *folder() const;

protected:
    QWidget *createInWidget();
    void populateCreateInWidget(KrBookmark *root, QTreeWidgetItem *parent);

protected slots:
    void toggleCreateIn(bool show);
    void slotSelectionChanged();
    void newFolder();

private:
    KLineEdit *_name;
    KLineEdit *_url;
    KLineEdit *_folder;
    KrTreeWidget *_createIn;
    QMap<QTreeWidgetItem*, KrBookmark*> _xr;
    QToolButton *_createInBtn;
};

#endif // KRADDBOOKMARKDLG_H
