#ifndef _PANEL_MANAGER_H
#define _PANEL_MANAGER_H

#include <qwidget.h>
#include <qlayout.h>
#include "paneltabbar.h"

class ListPanel;
class QWidgetStack;

class PanelManager: public QWidget {
    Q_OBJECT

  public:
    PanelManager( QWidget *parent, bool left );
    ListPanel* createPanel();
    void startPanel(ListPanel *panel, QString path);

  private:
    QVBoxLayout *_layout;
    bool _left;
    PanelTabBar *_tabbar;
    QWidgetStack *_stack;
};


#endif // _PANEL_MANAGER_H
