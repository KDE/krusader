/***************************************************************************
                                remoteman.cpp
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description 
 ***************************************************************************

  A 

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#include "remoteman.h"
#include "../kicons.h"
#include "../resources.h"
#include "../krusader.h"
#include <qlineedit.h>
#include <klocale.h>
#include <qpixmap.h>
#include <qspinbox.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <kstandarddirs.h>

QString remoteMan::url=QString::null;

remoteMan::remoteMan() : remoteManBase(0,0,true), currentItem(0) {
  // Read the connection list from the configuration file and re-create the tree
  config2tree();
  sessions->setCurrentItem(sessions->firstChild());
  // some minor fixes, left out of the designer - do again in next version !
  password->setEchoMode(QLineEdit::Password);
  // ===> should be moved to remoteManBase <=====
  connect( hostName, SIGNAL( textChanged(const QString&) ),
           this, SLOT( updateConnect(const QString&) ) );
  connect( sessions, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(connection()));
  connect( sessions, SIGNAL(returnPressed(QListViewItem *)), this, SLOT(connection()));
  // execute the main dialog
  exec();
}

void remoteMan::expandDecos(QListViewItem* i) {
  if (i->text(1)!="SESSION") i->setPixmap(0,LOADICON("folder_open"));
}
void remoteMan::collapseDecos(QListViewItem* i) {
  if (i->text(1)!="SESSION") i->setPixmap(0,LOADICON("folder_open"));
}

// This is the static memeber, you'd want to call this directly !
QString remoteMan::getHost() {
  remoteMan *p=new remoteMan();
  delete p;
  return url;
}

// adds a new group submenu
void remoteMan::addGroup() {
  QListViewItem *i=0,*current;
  current=sessions->currentItem();
  if (!current) // no item is current, or the list is empty (shouldn't happen)
    i=new QListViewItem(sessions,i18n("New group"),"GROUP"); // insert into the backplane
   else
  if (current->text(1).left(5)=="GROUP")
    i=new QListViewItem(current,i18n("New group"),"GROUP"); // insert under the current item
      else return;  // cannot create a new group under an item
  // set an open folder pixmap for the group
  i->setPixmap(0,LOADICON("folder_open"));
  // make the new item current and refresh the view
  sessions->setCurrentItem(i); sessions->ensureItemVisible(i);
}

// add an actual session
void remoteMan::addSession() {
  QListViewItem *i=0,*current;
  current=sessions->currentItem();
  // if we are pointing to a session, then the new session will be
  // created under the current session's parent group
  if (current->text(1)=="SESSION") current=current->parent();
  // create a new item and give it the appropriate pixmap
  i=new QListViewItem(current,i18n("New session"),"SESSION");
  i->setPixmap(0,LOADICON("kr_ftp_new"));
  // make the new item current and refresh the view
  sessions->setCurrentItem(i); sessions->ensureItemVisible(i);
}

// called upon a selection change to update the view
void remoteMan::refreshData() {
  // first, update the last changes the user made
  if (currentItem) {
    currentItem->setText(2,hostName->text());
    currentItem->setText(3,portNum->cleanText());
    currentItem->setText(4,userName->text());
    currentItem->setText(5,password->text());
    currentItem->setText(6,remoteDir->text());
    currentItem->setText(7,description->text());
    currentItem->setText(8,protocol->currentText());
    // we have to check if there's another brother-session with
    // the same name, if so, we add a <2> to it
    QListViewItem *i;
    if (currentItem->parent()) i=currentItem->parent()->firstChild();
      else i=sessions->firstChild();
    while (i) {
      if (i->text(0)==currentItem->text(0) && i!=currentItem) {
        QString temp=currentItem->text(0).right(4);
        if (temp.left(1)=="<" && temp.right(1)==">") {
          int h=temp.mid(1,1).toInt();
          int l=temp.mid(2,1).toInt();
          if ((++l)==10) { ++h; l=0; }
          temp=QString("<%1%2>").arg(h).arg(l);
          temp=currentItem->text(0).replace(currentItem->text(0).length()-4,4,temp);
          currentItem->setText(0,temp);
        } else currentItem->setText(0,currentItem->text(0)+"<02>");
        i=currentItem->parent()->firstChild();
      } else i=i->nextSibling();
    }
  }
  // here begins the actual function
  removeBtn->setEnabled(true);  // just incase we turned it off last time
  connectBtn->setEnabled(true);
  QListViewItem *current=sessions->currentItem();
  if (!current) return; // if no item is current yet
  if (current->text(1).left(5)=="GROUP") { // disable all that don't belong to a group
    // first, disable all the fields a user cannot change
    hostName->setEnabled(false);
    password->setEnabled(false);
    portNum->setEnabled(false);
    remoteDir->setEnabled(false);
    userName->setEnabled(false);
    if (current->text(1)=="GROUP!") {
      sessionName->setEnabled(false); // even name can't be changed here!
      description->setEnabled(false);
      removeBtn->setEnabled(false);
      connectBtn->setEnabled(false);
    } else sessionName->setEnabled(true);
  } else {  // otherwise, a normal url is under the cursor
    if( !current->text(8).isEmpty() )
      protocol->setCurrentText(current->text(8));
    else
      protocol->setCurrentItem(0);
    hostName->setEnabled(true);
    // anonymous connection ??
    userName->setEnabled(!anonymous->isChecked());
    password->setEnabled(!anonymous->isChecked());
    // for now, don't allow port settings for smb://
    portNum->setEnabled( protocol->currentText()=="ftp://" );
    remoteDir->setEnabled(true);
    sessionName->setEnabled(true);
    description->setEnabled(true);
  }
  // now, update the session name (or group name) and needed info
  sessionName->setText(current->text(0));
  hostName->setText(current->text(2));
  portNum->setValue(current->text(3).toInt());
  if (portNum->value()==0) portNum->setValue(21);
  userName->setText(current->text(4));
  password->setText(current->text(5));
  remoteDir->setText(current->text(6));
  description->setText(current->text(7));
  sessions->setSorting(1); sessions->sort();  // resort the tree
  currentItem=current;  // keep the active item for other jobs
  // disable connect button if no host name is defined
  if (hostName->text().simplifyWhiteSpace().isEmpty())
    connectBtn->setEnabled(false);
}

// called when we are changing the session/group name, so that remoteMan
// is able to change it in the corrosponding listview at the same time
void remoteMan::updateName(const QString &text) {
  QListViewItem *i=sessions->currentItem();
  i->setText(0,text);
}

void remoteMan::updateConnect(const QString &) {
  // disable connect button if no host name is defined
  if (hostName->text().simplifyWhiteSpace().isEmpty())
    connectBtn->setEnabled(false);
  else connectBtn->setEnabled(true);
}

// take an item, and create a "connection", built of the full
// path of folders to the item, seperated by the ` character
QString item2connection(QListViewItem *item) {
  QString con=item->text(0);
  QListViewItem *iterator=item->parent();
  while (iterator!=0) {
    //////////////////////// explanation: ///////////////////////////
    // since the` char is special to us, we use it to seperate items
    // in the connection, we cannot allow it to be inside a name, so
    // we find it (if it exists) and replace it with a ' character
    QString temp=iterator->text(0);
    int i=temp.find('`'); // are there any ` in the name ?
    while (i>-1) {        // if so, change them until none are left
      temp.replace(i,1,QChar('\''));
      i=temp.find('`');
    }
    con=temp+'`'+con;
    iterator=iterator->parent();
  }
  return con;
}

// find an item with a specific path - if the path doesn't exist, create it
QListViewItem* remoteMan::findItem(const QString &name, QListViewItem *p) {
  QListViewItem *it;
  if (name.isEmpty()) return p; // the end of the recursion has been reached!!!
  if (p==0) it=sessions->firstChild(); else it=p->firstChild();
  int loc=name.find('`');
  while (it) {
    if (it->text(0)==name.left(loc)) break;
    it=it->nextSibling();
  }
  if (!it) // item has not been found, create it
    if (!p) it=new QListViewItem(sessions,name.left(loc),"GROUP");
     else it=new QListViewItem(p,name.left(loc),"GROUP");
  // now, it points to the item we want, so let's go down one level
  it=findItem(name.mid(loc+1),it);
  return it;
}

// re-create the tree from a config file
void remoteMan::config2tree() {
  // attempt to read the tree from krusader's config file
  krConfig->setGroup("RemoteMan");
  QStringList lst=krConfig->readListEntry("Connections");
  if (lst.count()<1) { // no such list in the config file - create the basics
    // add the default groups
    QListViewItem *i;
    i=new QListViewItem(sessions,i18n("Sessions"),"GROUP!");
    // the GROUP text signifies a group (duh), the GROUP! signifies a group
    // that the user cannot change
    i->setPixmap(0,LOADICON("folder_open"));
    sessions->setCurrentItem(i);
//#    i=new QListViewItem(sessions,i18n("Samba sessions (comming soon)"),"GROUP!");
//#    i->setPixmap(0,LOADICON("folder"));
    return;
  }
  // if we got here, we have a connection entry in the config file
  // let's work on it...
  QStringList::Iterator it;
  QListViewItem *item=0;
  for( it = lst.begin(); it != lst.end(); ++it ) {
    QString t=(*it);  // easier to work with
    // this part creates the path to the session
    int loc=t.findRev('`');
    if (loc>-1) item=findItem(t.left(loc+1),0);
    // at this point we have the complete path to the object
    // let's keep only the item's name
    t=t.mid(loc+1);
    // now, we are left with only the session name itself
    // so, we create it as a son of the active item
    QStringList newLst=krConfig->readListEntry(*it);      // and fill in
    bool group=(newLst[1]!="SESSION");
    QListViewItem *newItem;
    if (item==0)  // if this item originating from the backplane
      newItem=new QListViewItem(sessions,t,group ? "GROUP" : "SESSION");
     else
      newItem=new QListViewItem(item,t,group ? "GROUP" : "SESSION");
    newItem->setPixmap(0,group ? LOADICON("folder") : LOADICON("kr_ftp_new"));  // update a pixmap
    newItem->setText(0,t);
    // expand the item, if needed
    ++it; if ((*it) == "expanded") newItem->setOpen(true);
    for (int i=1; i<9; ++i) newItem->setText(i,newLst[i]);// the details
    item=0; // clean up pointers
  }
}

// make a single pass on the tree and create a configuration file
void remoteMan::tree2config() {
  // first, set the stage
  krConfig->setGroup("RemoteMan");
  QStringList lst;
  QListViewItemIterator it(sessions);
  while (it.current()) { // while there are still items in the tree
    QString temp=item2connection((it.current()));
    lst.append(temp); // write the connection into the "connection-index"
    if (it.current()->isOpen())
      lst.append("expanded"); else lst.append("collapsed");
    QStringList data;
    for (int i=0; i<9; ++i) data.append((it.current())->text(i));
    // write the specific entry into the config file
    krConfig->writeEntry(temp,data);
    ++it;
  }
  // now we write the index
  krConfig->writeEntry("Connections",lst);
  krConfig->sync(); // force an immediate write to the disk
}

void remoteMan::connection() {
  // if clicked on a group...
  QListViewItem *i=sessions->currentItem();
  if (i->text(1)!="SESSION") {
    i->setOpen(i->isOpen());
    return;
  }
  // build a url
  if (anonymous->isChecked()) {
    userName->setText(QString::null);
    password->setText(QString::null);
  } else {
    userName->setText(userName->text().simplifyWhiteSpace());
    password->setText(password->text().simplifyWhiteSpace());
  }
  hostName->setText(hostName->text().simplifyWhiteSpace().lower());
  if (hostName->text().left(6)=="ftp://" || hostName->text().left(6)=="smb://")
    hostName->setText(hostName->text().mid(6));
  remoteDir->setText(remoteDir->text().simplifyWhiteSpace());
  if ( !remoteDir->text().isEmpty() && remoteDir->text().left(1)!="/")
		remoteDir->setText("/"+remoteDir->text());

	QString port="";
  if (protocol->currentText()=="ftp://" && portNum->value()!=21 )
		port=":"+portNum->cleanText();

	url=protocol->currentText();

	if( !userName->text().isEmpty() ){
		url = url+userName->text();
		if( !password->text().isEmpty() );
			url = url+":"+password->text();
		url = url+"@";
	}
	url=url+ hostName->text()+port+remoteDir->text();

  // now, let's close cleanly
  refreshData();  // make sure all is updated
  tree2config();  // write the sessions to a configuration file
  remoteManBase::accept();  // tidy up
}

// remove a connection or a group of connections
void remoteMan::removeSession() {

  QListViewItem *it=sessions->currentItem();
  if (!it) return;  // don't do anything if no item is selected
  switch( QMessageBox::warning( this, i18n("RemoteMan"),
    i18n("Are you sure you want to delete this item ???"),
    QMessageBox::Yes,
    QMessageBox::No | QMessageBox::Default | QMessageBox::Escape)) {
    case QMessageBox::No  : return;
  }
  // since we're here, the user must have pressed YES, let's delete
  delete it;
  sessions->triggerUpdate();
}

// what we do when the user clicked the close button
void remoteMan::accept() {
  refreshData();  // make sure all is updated
  tree2config();  // write the sessions to a configuration file
  url=QString::null; // this signals getHost() we didn't click CONNECT
  remoteManBase::accept();  // tidy up
}

void remoteMan::reject() {
  refreshData();  // make sure all is updated
  tree2config();  // write the sessions to a configuration file
  url=QString::null; // this signals getHost() we didn't click CONNECT
  remoteManBase::reject();
}

#include "remoteman.moc"
