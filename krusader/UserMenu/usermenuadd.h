/****************************************************************************
** Form interface generated from reading ui file './usermenuadd.ui'
**
** Created: Die Mär 16 18:51:25 2004
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef USERMENUADD_H
#define USERMENUADD_H

#include <qvariant.h>
#include <qdialog.h>

#include "../UserAction/useraction.h"

class UserActionProperties;
class ActionProperty;
class KPushButton;

class UserMenuAdd : public QDialog
{
    Q_OBJECT

public:
    UserMenuAdd( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~UserMenuAdd();

private:
    ActionProperty *actionProperties;
    KPushButton* okButton;
    KPushButton* cancelButton;

private slots:
    void check();
    
signals:
    void newEntry( UserActionProperties* properties );
};

#endif // USERMENUADD_H
