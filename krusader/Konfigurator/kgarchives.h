/****************************************************************************
** Form interface generated from reading ui file 'kgarchives.ui'
**
** Created: Tue Apr 17 20:21:41 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KGARCHIVES_H
#define KGARCHIVES_H

#include <qvariant.h>
#include <qframe.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QGroupBox;
class QLabel;
class QPushButton;

class kgArchives : public QFrame
{ 
    Q_OBJECT

public:
    kgArchives( QWidget* parent = 0, const char* name = 0 );
    ~kgArchives();

    QGroupBox* GroupBox13;
    QCheckBox* kgBZip2;
    QCheckBox* kgGZip;
    QCheckBox* kgRpm;
    QCheckBox* kgZip;
    QCheckBox* kgTar;
    QCheckBox* kgArj;
    QPushButton* kgAutoConfigure;
    QLabel* TextLabel1_2;
    QCheckBox* kgRar;
    QCheckBox* kgAce;
    QLabel* TextLabel1;
    QGroupBox* GroupBox2;
    QCheckBox* kgMoveIntoArchives;
    QCheckBox* kgTestArchives;

public slots:
    virtual void slotApplyChanges();
    virtual void slotAutoConfigure();
    virtual void slotDefaultSettings();

protected:
    QGridLayout* kgArchivesLayout;
    QGridLayout* GroupBox13Layout;
    QVBoxLayout* Layout4;
};

#endif // KGARCHIVES_H
