/****************************************************************************
** Form interface generated from reading ui file 'bookmanguibase.ui'
**
** Created: Wed Oct 24 17:47:23 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef BOOKMANGUIBASE_H
#define BOOKMANGUIBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QFrame;
class QLabel;
class QLineEdit;
class QListView;
class QListViewItem;
class QPushButton;
class QTabWidget;
class QToolButton;
class QWidget;

class BookManGUIBase : public QDialog
{ 
    Q_OBJECT

public:
    BookManGUIBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~BookManGUIBase();

    QCheckBox* sortedCheckbox;
    QPushButton* okBtn;
    QPushButton* closeBtn;
    QTabWidget* tabWidget;
    QWidget* Widget2;
    QLineEdit* nameData;
    QLabel* TextLabel4;
    QPushButton* addButton;
    QLabel* TextLabel2;
    QLabel* TextLabel1;
    QFrame* Line1;
    QLabel* TextLabel3;
    QToolButton* browseButton;
    QLabel* TextLabel5;
    QLineEdit* urlData;
    QWidget* Widget3;
    QLabel* TextLabel1_2;
    QFrame* Line1_2;
    QListView* bookmarks;
    QToolButton* upButton;
    QToolButton* downButton;
    QPushButton* editButton;
    QPushButton* removeButton;
    QPushButton* applyButton;

public slots:
    virtual void slotBrowse();
    virtual void slotAddBookmark();
    virtual void slotApply();
    virtual void slotDown();
    virtual void slotEditBookmark();
    virtual void slotPreAddBookmark();
    virtual void slotRemoveBookmark();
    virtual void slotUp();

protected:
    QGridLayout* BookManGUIBaseLayout;
    QHBoxLayout* Layout7;
    QGridLayout* Widget2Layout;
    QGridLayout* Layout9;
    QVBoxLayout* Layout9_2;
    QGridLayout* Layout8;
    QGridLayout* Widget3Layout;
    QVBoxLayout* Layout5;
    QVBoxLayout* Layout7_2;
    bool event( QEvent* );
};

#endif // BOOKMANGUIBASE_H
