/***************************************************************************
                          kcmmodebutton.h  -  description
                             -------------------

    this file contains a class KCMDModeButton, which represents a button with
    popup menu to switch the mode of the krusader built-in command-line

    begin                : Oct 2006
    inspired by          : other Krusader source files
    author of this file  : Vaclav Juza
    email                : vaclavjuza at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KCMDMODEBUTTON_H
#define KCMDMODEBUTTON_H

#include <QtGui/QToolButton>

class KActionMenu;

/**
  * @author Vaclav Juza
  *
  * represents a button for switching the command-line execution mode
  * It extends QToolButton, set the icon etc., and creates a popup menu
  * containing the actions to actually switch the mode.
  */
class KCMDModeButton : public QToolButton  {
  Q_OBJECT
public: 
  /** Constructor. Sets up the menu, and the icon */
  KCMDModeButton(QWidget *parent=0);
  ~KCMDModeButton();

  /** Shows the popup menu. Called when clicked to the button */
  void showMenu();

private:
  /** The menu containing the actions for switching the mode */
  KActionMenu *action;
};

#endif
