/****************************************************************************
** Form interface generated from reading ui file 'kgadvanced.ui'
**
** Created: Tue Apr 10 01:13:00 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KGADVANCED_H
#define KGADVANCED_H

#include <qvariant.h>
#include <qframe.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QGroupBox;
class QLabel;
class QSpinBox;

class kgAdvanced : public QFrame
{ 
    Q_OBJECT

public:
    kgAdvanced( QWidget* parent = 0, const char* name = 0 );
    ~kgAdvanced();

    QGroupBox* GroupBox6;
    QLabel* TextLabel1;
    QCheckBox* kgNonEmpty;
    QCheckBox* kgDelete;
    QCheckBox* kgCopy;
    QCheckBox* kgMove;
    QGroupBox* GroupBox15;
    QLabel* TextLabel2_2;
    QSpinBox* kgCacheSize;
    QGroupBox* GroupBox2;
    QCheckBox* kgRootSwitch;
    QCheckBox* kgAutomount;

public slots:
    virtual void slotApplyChanges();
    virtual void slotDefaultSettings();

protected:
    QGridLayout* kgAdvancedLayout;
    QVBoxLayout* Layout6;
    QHBoxLayout* Layout5;
    QGridLayout* Layout3;
};

#endif // KGADVANCED_H
