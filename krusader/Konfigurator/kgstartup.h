/****************************************************************************
** Form interface generated from reading ui file 'kgstartup.ui'
**
** Created: Mon May 21 16:00:00 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KGSTARTUP_H
#define KGSTARTUP_H

#include <qvariant.h>
#include <qframe.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QToolButton;

class kgStartup : public QFrame
{ 
    Q_OBJECT

public:
    kgStartup( QWidget* parent = 0, const char* name = 0 );
    ~kgStartup();

    QGroupBox* GroupBox3;
    QButtonGroup* ButtonGroup1;
    QRadioButton* panelsDontSave;
    QRadioButton* panelsSave;
    QComboBox* panelsTypes;
    QComboBox* panelsRightType;
    QLabel* TextLabel1;
    QLabel* panelsLeftLabel1;
    QLabel* panelsRightLabel1;
    QComboBox* panelsLeftType;
    QLineEdit* panelsRightHomepage;
    QLineEdit* panelsLeftHomepage;
    QLabel* panelsLeftLabel2;
    QLabel* panelsRightLabel2;
    QToolButton* panelsLeftBrowse;
    QToolButton* panelsRightBrowse;
    QGroupBox* GroupBox20;
    QCheckBox* uiSaveSettings;
    QCheckBox* uiToolbar;
    QCheckBox* uiCmdLine;
    QCheckBox* uiTerminalEmulator;
    QCheckBox* uiPositionSize;
    QCheckBox* uiFnKeys;
    QCheckBox* uiStatusbar;

public slots:
    virtual void slotUiSave();
    virtual void slotApplyChanges();
    virtual void slotDefaultSettings();
    virtual void slotLeftBrowse();
    virtual void slotPanelsDontSave();
    virtual void slotPanelsLeftType();
    virtual void slotPanelsRightType();
    virtual void slotPanelsSave();
    virtual void slotPanelsTypes();
    virtual void slotRightBrowse();

protected:
    QGridLayout* kgStartupLayout;
    QGridLayout* GroupBox3Layout;
    QGridLayout* GroupBox20Layout;
};

#endif // KGSTARTUP_H
