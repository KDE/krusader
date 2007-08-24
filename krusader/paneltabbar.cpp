/***************************************************************************
                          paneltabbar.cpp  -  description
                             -------------------
    begin                : Sun Jun 2 2002
    copyright            : (C) 2002 by Shie Erlich & Rafi Yanai
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

#include "paneltabbar.h"
#include "Panel/listpanel.h"
#include "krusaderview.h"
#include "krslots.h"
#include "defaults.h"
#include <kaction.h>
#include <klocale.h>
#include <kshortcut.h>
#include <qevent.h>
#include <q3widgetstack.h>
#include <qfontmetrics.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QDragMoveEvent>
#include <kdebug.h>
#include <kactionmenu.h>


#define DISPLAY(X)	(X.isLocalFile() ? X.path() : X.prettyUrl())

PanelTabBar::PanelTabBar(QWidget *parent): QTabBar(parent), _maxTabLength(0) {
  _panelActionMenu = new KActionMenu( i18n("Panel"), this );

  setAcceptDrops(true);  
  insertAction(krNewTab);
  insertAction(krDupTab);
  insertAction(krPreviousTab);
  insertAction(krNextTab);
  insertAction(krCloseTab);
  krCloseTab->setEnabled(false); //can't close a single tab

  setShape(QTabBar::TriangularSouth);
}

void PanelTabBar::mousePressEvent( QMouseEvent* e ) {
  int clickedTab = tabAt(e->pos());
  if (-1 == clickedTab) { // clicked on nothing ...
    QTabBar::mousePressEvent(e);
    return;
  }

  setCurrentIndex( clickedTab );

  ListPanel *panel = (ListPanel*)tabData(clickedTab).toLongLong();
  emit changePanel(panel);

  if ( e->button() == Qt::RightButton ) {
    // show the popup menu
    _panelActionMenu->popupMenu()->popup( e->globalPos() );
  } else
  if ( e->button() == Qt::LeftButton ) { // we need to change tabs
    // first, find the correct panel to load
    int id = currentIndex();
    ListPanel *listpanel = (ListPanel*)tabData(id).toLongLong();
    emit changePanel(listpanel);
  } else
  if (e->button() == Qt::MidButton) { // close the current tab
    emit closeCurrentTab();
  }
  QTabBar::mousePressEvent(e);
}

void PanelTabBar::insertAction( KAction* action ) {
  _panelActionMenu->addAction( action );
}

int PanelTabBar::addPanel(ListPanel *panel, bool setCurrent ) {
  int newId = addTab(squeeze(DISPLAY(panel->virtualPath())));
  QVariant v;
  v.setValue((long long)panel);
  setTabData(newId, v);

  // make sure all tabs lengths are correct
  for (int i=0; i<count(); i++)
    setTabText(i, squeeze(DISPLAY(((ListPanel*)tabData(i).toLongLong())->virtualPath()), i));
// layoutTabs();
  
  if( setCurrent )
    setCurrentIndex(newId);

  // enable close-tab action
  if (count()>1) {
    krCloseTab->setEnabled(true);
  }


connect((ListPanel*)tabData(newId).toLongLong(), SIGNAL(pathChanged(ListPanel*)),
          this, SLOT(updateTab(ListPanel*)));

  return newId;
}

ListPanel* PanelTabBar::removeCurrentPanel(ListPanel* &panelToDelete) {
  int id = currentIndex();
  ListPanel *oldp = (ListPanel*)tabData(id).toLongLong(); // old panel to kill later
  disconnect(oldp);
  removeTab(id);

  for (int i=0; i<count(); i++)
    setTabText(i, squeeze(DISPLAY(((ListPanel*)tabData(i).toLongLong())->virtualPath()), i));
  //layoutTabs();

  // setup current one
  id = currentIndex();
  ListPanel *p = (ListPanel*)tabData(id).toLongLong();
  // disable close action?
  if (count()==1) {
    krCloseTab->setEnabled(false);
  }

  panelToDelete = oldp;
  return p;
}

void PanelTabBar::updateTab(ListPanel *panel) {
  // find which is the correct tab
  for (int i=0; i<count(); i++) {
    if ((ListPanel*)tabData(i).toLongLong() == panel) {
      setTabText(i, squeeze(DISPLAY(panel->virtualPath()),i));
      break;
    }
  }
}

void PanelTabBar::duplicateTab() {
  int id = currentIndex();
  emit newTab(((ListPanel*)tabData(id).toLongLong())->virtualPath());
}

void PanelTabBar::closeTab() {
  emit closeCurrentTab();
}

QString PanelTabBar::squeeze(QString text, int index) {
  QString originalText = text;
  
  QString lastGroup = krConfig->group();  
  krConfig->setGroup( "Look&Feel" );
  bool longNames = krConfig->readBoolEntry( "Fullpath Tab Names", _FullPathTabNames );
  krConfig->setGroup( lastGroup );
  
  if( !longNames )
  {
    while( text.endsWith( "/" ) )
      text.truncate( text.length() -1 );      
    if( text.isEmpty() || text.endsWith(":") )
      text += "/";
    else
    {
      QString shortName;
                    
      if( text.contains( ":/" ) )
        shortName = text.left( text.indexOf( ":/" ) ) + ":";
    
      shortName += text.mid( text.lastIndexOf( "/" ) + 1 );      
      text = shortName;
    }
    
    if( index >= 0 )
      setTabToolTip( index, originalText );
    
    index = -1;
  }
  
  QFontMetrics fm(fontMetrics());

  // set the real max length
  _maxTabLength = (static_cast<QWidget*>(parent())->width()-(6*fm.width("W")))/fm.width("W");
  // each tab gets a fair share of the max tab length
  int _effectiveTabLength = _maxTabLength / (count() == 0 ? 1 : count());

  int labelWidth = fm.width("W")*_effectiveTabLength;
  int textWidth = fm.width(text);
  if (textWidth > labelWidth) {
    // start with the dots only
    QString squeezedText = "...";
    int squeezedWidth = fm.width(squeezedText);

    // estimate how many letters we can add to the dots on both sides
    int letters = text.length() * (labelWidth - squeezedWidth) / textWidth / 2;
    if (labelWidth < squeezedWidth) letters=1;
    squeezedText = text.left(letters) + "..." + text.right(letters);
    squeezedWidth = fm.width(squeezedText);

    if (squeezedWidth < labelWidth) {
        // we estimated too short
        // add letters while text < label
        do {
                letters++;
                squeezedText = text.left(letters) + "..." + text.right(letters);
                squeezedWidth = fm.width(squeezedText);
        } while (squeezedWidth < labelWidth);
        letters--;
        squeezedText = text.left(letters) + "..." + text.right(letters);
    } else if (squeezedWidth > labelWidth) {
        // we estimated too long
        // remove letters while text > label
        do {
            letters--;
            squeezedText = text.left(letters) + "..." + text.right(letters);
            squeezedWidth = fm.width(squeezedText);
        } while (letters && squeezedWidth > labelWidth);
    }

    if( index >= 0 )
      setTabToolTip( index, originalText );

    if (letters < 5) {
    	// too few letters added -> we give up squeezing
      //return text;
    	return squeezedText;
    } else {
	    return squeezedText;
    }
  } else {
    //if( index >= 0 )
    //  removeTabToolTip( index );
      
    return text;
  };
}

void PanelTabBar::resizeEvent ( QResizeEvent *e ) {
    QTabBar::resizeEvent( e );
     
    for (int i=0; i<count(); i++)
	    setTabText(i, squeeze(DISPLAY(((ListPanel*)tabData(i).toLongLong())->virtualPath()), i));
    //layoutTabs();
}

#if 0 // TODO: fix this
void PanelTabBar::dragEnterEvent(QDragEnterEvent *e) {
	QTab *t = selectTab(e->pos());
	if (!t) return;
	if (tab(currentIndex()) != t) {
		setCurrentIndex(t);
		emit changePanel(static_cast<ListPanel*>((ListPanel*)tabData(t)));
	}
}

void PanelTabBar::dragMoveEvent(QDragMoveEvent *e) {
	QTab *t = selectTab(e->pos());
	if (!t) return;
	if (tab(currentIndex()) != t) {
		setCurrentIndex(t);
		emit changePanel(static_cast<ListPanel*>((ListPanel*)tabData(t)));
	}
}
#endif

#include "paneltabbar.moc"
