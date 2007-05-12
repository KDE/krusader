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
//Added by qt3to4:
#include <Q3GridLayout>
#include <QEvent>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>
class Q3VBoxLayout; 
class Q3HBoxLayout; 
class Q3GridLayout; 
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class Q3ListView;
class Q3ListViewItem;
class Q3MultiLineEdit;
class QPushButton;
class QSpinBox;

class remoteManBase : public QDialog
{ 
    Q_OBJECT

public:
    remoteManBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~remoteManBase();

    QLabel* TextLabel1;
    QLineEdit* sessionName;
    QPushButton* moreBtn;
    QPushButton* closeBtn;
    Q3ListView* sessions;
    QLabel* TextLabel1_3_3;
    QLineEdit* password;
    QLabel* TextLabel1_3;
    QLineEdit* userName;
    QCheckBox* anonymous;
    QLabel* TextLabel1_3_2;
    QLineEdit* remoteDir;
    QLabel* TextLabel1_3_2_2;
    Q3MultiLineEdit* description;
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
    Q3GridLayout* remoteManBaseLayout;
    Q3VBoxLayout* Layout23;
    Q3HBoxLayout* Layout12;
    Q3VBoxLayout* Layout9;
    Q3GridLayout* Layout10;
    Q3VBoxLayout* Layout26;
    Q3VBoxLayout* Layout27;
    Q3GridLayout* layout;
    Q3GridLayout* Layout11;
    bool event( QEvent* );
};

#endif // REMOTEMANBASE_H
