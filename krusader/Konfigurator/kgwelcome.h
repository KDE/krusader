/****************************************************************************
** Form interface generated from reading ui file 'kgwelcome.ui'
**
** Created: Sun Feb 18 22:39:31 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef KGWELCOME_H
#define KGWELCOME_H

#include <qvariant.h>
#include <qframe.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLabel;

class kgWelcome : public QFrame
{ 
    Q_OBJECT

public:
    kgWelcome( QWidget* parent = 0, const char* name = 0 );
    ~kgWelcome();

    QLabel* PixmapLabel1;

protected:
    QGridLayout* kgWelcomeLayout;
};

#endif // KGWELCOME_H
