/****************************************************************************
** Form interface generated from reading ui file 'remotemanbase.ui'
**
** Created: Thu Jun 7 16:23:59 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef REMOTEMANBASE_H
#define REMOTEMANBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListView;
class QListViewItem;
class QMultiLineEdit;
class QPushButton;
class QSpinBox;

class remoteManBase : public QDialog
{ 
    Q_OBJECT

public:
    remoteManBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~remoteManBase();

    QLabel* TextLabel1;
    QLineEdit* sessionName;
    QPushButton* moreBtn;
    QPushButton* closeBtn;
    QListView* sessions;
    QLabel* TextLabel1_3_3;
    QLineEdit* password;
    QLabel* TextLabel1_3;
    QLineEdit* userName;
    QCheckBox* anonymous;
    QLabel* TextLabel1_3_2;
    QLineEdit* remoteDir;
    QLabel* TextLabel1_3_2_2;
    QMultiLineEdit* description;
    QPushButton* removeBtn;
    QPushButton* connectBtn;
    QPushButton* newGroupBtn;
    QPushButton* addBtn;
    QLabel* TextLabel1_2;
    QComboBox* protocol;
    QLineEdit* hostName;
    QSpinBox* portNum;
    QLabel* TextLabel1_2_2;
    QLabel* TextLabel1_4;

public slots:
    virtual void addSession();
    virtual void connection();
    virtual void moreInfo();
    virtual void addGroup();
    virtual void refreshData();
    virtual void removeSession();
    virtual void updateName(const QString&);

protected:
    QGridLayout* remoteManBaseLayout;
    QVBoxLayout* Layout23;
    QHBoxLayout* Layout12;
    QVBoxLayout* Layout9;
    QGridLayout* Layout10;
    QVBoxLayout* Layout26;
    QVBoxLayout* Layout27;
    QGridLayout* layout;
    QGridLayout* Layout11;
    bool event( QEvent* );
};

#endif // REMOTEMANBASE_H
