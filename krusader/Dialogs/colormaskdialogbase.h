/****************************************************************************
** Form interface generated from reading ui file 'colormaskdialog.ui'
**
** Created: Wed Sep 26 21:56:29 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef COLORMASKDIALOGBASE_H
#define COLORMASKDIALOGBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QGroupBox;
class QLabel;
class QPushButton;

class colorMaskDialogBase : public QDialog
{ 
    Q_OBJECT

public:
    colorMaskDialogBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~colorMaskDialogBase();

    QGroupBox* GroupBox1;
    QLabel* PixmapLabel4;
    QLabel* PixmapLabel3;
    QLabel* PixmapLabel5;
    QLabel* PixmapLabel2;
    QCheckBox* identicalLeft;
    QCheckBox* olderLeft;
    QCheckBox* newerLeft;
    QCheckBox* exclusiveLeft;
    QPushButton* buttonHelp;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QGroupBox* GroupBox1_2;
    QLabel* PixmapLabel3_2;
    QLabel* PixmapLabel2_2;
    QLabel* PixmapLabel5_2;
    QLabel* PixmapLabel4_2;
    QCheckBox* newerRight;
    QCheckBox* olderRight;
    QCheckBox* identicalRight;
    QCheckBox* exclusiveRight;
    QLabel* TextLabel2;

protected:
    QGridLayout* colorMaskDialogBaseLayout;
    QHBoxLayout* Layout1;
};

#endif // COLORMASKDIALOGBASE_H
