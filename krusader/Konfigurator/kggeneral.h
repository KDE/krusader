/****************************************************************************
** Form interface generated from reading ui file 'kggeneral.ui'
**
** Created: Fri Jun 1 18:20:39 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KGGENERAL_H
#define KGGENERAL_H

#include <qvariant.h>
#include <qframe.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QToolButton;

class kgGeneral : public QFrame
{ 
    Q_OBJECT

public:
    kgGeneral( QWidget* parent = 0, const char* name = 0 );
    ~kgGeneral();

    QGroupBox* GroupBox2;
    QLineEdit* kgTerminal;
    QLabel* TextLabel2;
    QLineEdit* kgEditor;
    QToolButton* kgBrowseEditor;
    QLabel* TextLabel1_2;
    QToolButton* kgBrowseTerminal;
    QFrame* Line2_2;
    QLineEdit* kgTempDir;
    QToolButton* kgBrowseTempDir;
    QLabel* TextLabel2_2;
    QLabel* TextLabel1;
    QButtonGroup* ButtonGroup1;
    QRadioButton* kgMoveToTrash;
    QRadioButton* kgDeleteFiles;
    QFrame* Line2;
    QCheckBox* kgMimetypeMagic;

public slots:
    virtual void slotBrowseTempDir();
    virtual void slotApplyChanges();
    virtual void slotBrowseEditor();
    virtual void slotBrowseTerminal();
    virtual void slotDefaultSettings();

protected:
    QGridLayout* kgGeneralLayout;
    QGridLayout* GroupBox2Layout;
    QGridLayout* Layout8;
    QGridLayout* ButtonGroup1Layout;
};

#endif // KGGENERAL_H
