/****************************************************************************
** Form interface generated from reading ui file 'krsearchdialogbase.ui'
**
** Created: Wed Oct 24 15:47:11 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KRSEARCHBASE_H
#define KRSEARCHBASE_H

#include "generalfilter.h"

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class KHistoryCombo;
class KComboBox;
class KLineEdit;
class KSqueezedTextLabel;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListView;
class QListViewItem;
class QMultiLineEdit;
class QListBox;
class QPushButton;
class QRadioButton;
class QTabWidget;
class QToolButton;
class QWidget;

class KrSearchBase : public QDialog
{ 
    Q_OBJECT

public:
    KrSearchBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~KrSearchBase();

    GeneralFilter *generalFilter;
    QPushButton* mainHelpBtn;
    QPushButton* mainSearchBtn;
    QPushButton* mainStopBtn;
    QPushButton* mainCloseBtn;
    QTabWidget* TabWidget2;
    QWidget* tab_2;
    QGroupBox* GroupBox29;
    QCheckBox* smallerThanEnabled;
    QLineEdit* smallerThanAmount;
    KComboBox* smallerThanType;
    QCheckBox* biggerThanEnabled;
    KComboBox* biggerThanType;
    QLineEdit* biggerThanAmount;
    QButtonGroup* ButtonGroup2;
    QLineEdit* modifiedBetweenData1;
    QToolButton* modifiedBetweenBtn1;
    QLabel* TextLabel2_2;
    QLineEdit* modifiedBetweenData2;
    QToolButton* modifiedBetweenBtn2;
    QToolButton* notModifiedAfterBtn;
    QLineEdit* notModifiedAfterData;
    QRadioButton* modifiedInTheLastEnabled;
    QLineEdit* modifiedInTheLastData;
    QComboBox* modifiedInTheLastType;
    QComboBox* notModifiedInTheLastType;
    QLineEdit* notModifiedInTheLastData;
    QLabel* TextLabel3_2;
    QRadioButton* notModifiedAfterEnabled;
    QLabel* modifiedInTheLast;
    QRadioButton* modifiedBetweenEnabled;
    QGroupBox* GroupBox181;
    QCheckBox* belongsToUserEnabled;
    QComboBox* belongsToUserData;
    QCheckBox* belongsToGroupEnabled;
    QComboBox* belongsToGroupData;
    QGroupBox* GroupBox206;
    QComboBox* ownerW;
    QComboBox* ownerR;
    QComboBox* ownerX;
    QCheckBox* permissionsEnabled;
    QGroupBox* GroupBox206_2;
    QComboBox* groupW;
    QComboBox* groupR;
    QComboBox* groupX;
    QGroupBox* GroupBox206_3;
    QComboBox* allW;
    QComboBox* allX;
    QComboBox* allR;
    QLabel* TextLabel4;
    QWidget* tab_3;
    QLabel* foundLabel;
    KSqueezedTextLabel* searchingLabel;
    QListView* resultsList;

public slots:
    virtual void closeDialog();
    virtual void modifiedBetweenSetDate1();
    virtual void modifiedBetweenSetDate2();
    virtual void notModifiedAfterSetDate();
    virtual void resultClicked(QListViewItem*);
    virtual void rightClickMenu(QListViewItem*, const QPoint&, int);
    virtual void startSearch();
    virtual void stopSearch();

protected:
    QGridLayout* KrSearchBaseLayout;
    QHBoxLayout* Layout9;
    QGridLayout* tabLayout_2;
    QGridLayout* ButtonGroup2Layout;
    QGridLayout* GroupBox181Layout;
    QHBoxLayout* Layout10;
    QGridLayout* tabLayout_3;
    QHBoxLayout* Layout11;
    bool event( QEvent* );
};

#endif // KRSEARCHBASE_H
