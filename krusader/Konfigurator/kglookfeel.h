/****************************************************************************
** Form interface generated from reading ui file 'kglookfeel.ui'
**
** Created: Wed Jun 6 14:58:08 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KGLOOKFEEL_H
#define KGLOOKFEEL_H

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
class QRadioButton;
class QSpinBox;
class QTabWidget;
class QToolButton;
class QWidget;

class kgLookFeel : public QFrame
{ 
    Q_OBJECT

public:
    kgLookFeel( QWidget* parent = 0, const char* name = 0 );
    ~kgLookFeel();

    QTabWidget* TabWidget2;
    QWidget* tab;
    QGroupBox* GroupBox3;
    QLabel* TextLabel1;
    QSpinBox* kgHTMLMinFontSize;
    QLabel* TextLabel1_2;
    QLabel* TextLabel1_3;
    QComboBox* kgIconSize;
    QLabel* kgFontLabel;
    QToolButton* kgBrowseFont;
    QCheckBox* kgWarnOnExit;
    QCheckBox* kgShowHidden;
    QCheckBox* kgMarkDirs;
    QCheckBox* kgMinimizeToTray;
    QCheckBox* kgCaseSensativeSort;
    QFrame* Line1_2;
    QLabel* TextLabel1_4;
    QFrame* Line1_2_2;
    QButtonGroup* ButtonGroup1;
    QRadioButton* kgMouseSelectionClassic;
    QRadioButton* kgMouseSelectionLeft;
    QRadioButton* kgMouseSelectionRight;
    QWidget* tab_2;
    QWidget* tab_3;

public slots:
    virtual void slotBrowseFont();
    virtual void slotApplyChanges();
    virtual void slotChangeFont();
    virtual void slotDefaultSettings();

protected:
    QGridLayout* kgLookFeelLayout;
    QGridLayout* tabLayout;
    QGridLayout* GroupBox3Layout;
    QGridLayout* Layout17_2;
    QGridLayout* ButtonGroup1Layout;
};

#endif // KGLOOKFEEL_H
