#include <qwidgetstack.h>
#include "panelmanager.h"
#include "Panel/listpanel.h"

PanelManager::PanelManager(QWidget *parent, bool left): QWidget(parent), _layout(0), _left(left) {
   _layout = new QVBoxLayout( this );
   _stack = new QWidgetStack(this);
   _tabbar = new PanelTabBar(this);

   _layout->addWidget(_stack);
   _layout->addWidget(_tabbar);
}

ListPanel* PanelManager::createPanel() {
  // create the panel and add it into the widgetstack
  ListPanel *p = new ListPanel( _stack, _left );
  _stack->raiseWidget(p);
  // now, create the corrosponding tab
  _tabbar->addPanel(p);


  return p;
}

void PanelManager::startPanel(ListPanel *panel, QString path) {
// check if panel is ours!!!
  panel->start( path );
}

