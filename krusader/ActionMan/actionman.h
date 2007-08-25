//
// C++ Interface: actionman
//
// Description: This manages all useractions
//
//
// Author: Jonas B�r (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef ACTIONMAN_H
#define ACTIONMAN_H

#include <kdialog.h>

class UserActionPage;

class ActionMan : public KDialog {
Q_OBJECT
public:
   ActionMan( QWidget* parent=0 );
   ~ActionMan();

protected slots:
   void slotClose();
   void slotApply();
   void slotEnableApplyButton();
   void slotDisableApplyButton();

private:
  UserActionPage* userActionPage;
};

#endif // ifndef ACTIONMAN_H
