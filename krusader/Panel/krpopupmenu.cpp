/***************************************************************************
                          krpopupmenu.cpp  -  description
                             -------------------
    begin                : Tue Aug 26 2003
    copyright            : (C) 2003 by Shie Erlich & Rafi Yanai
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "krpopupmenu.h"

KrPopupMenu::KrPopupMenu(QWidget *parent, const char *name ) : QPopupMenu(parent,name) {
}
KrPopupMenu::~KrPopupMenu(){
}

#include "krpopupmenu.moc"
