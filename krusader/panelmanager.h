#ifndef _PANEL_MANAGER_H
#define _PANEL_MANAGER_H

#include <qwidget.h>
#include <qlayout.h>
#include "paneltabbar.h"

class KConfig;
class ListPanel;
class QWidgetStack;
class QToolButton;

/**
 * Implements tabbed-browsing by managing a list of tabs and corrosponding panels.
 */
class PanelManager: public QWidget {
    Q_OBJECT

  public:
    /**
     * PanelManager is created where once panels were created. It accepts three references to pointers
     * (self, other, active), which enables it to manage pointers held by the panels transparently.
     * It also receives a bool (left) which is true if the manager is the left one, or false otherwise.
     */
    PanelManager( QWidget *parent, bool left, ListPanel* &self, ListPanel* &other, ListPanel* &active);
    /**
     * Called once by KrusaderView to create the first panel. Subsequent called are done internally
     * Note: only creates the panel, but doesn't start the VFS inside it. Use startPanel() for that.
     */
    ListPanel* createPanel();
    /**
     * Called once by KrusaderView to start the first panel. Subsequent called are done internally
     * Only starts the VFS inside the panel, you must first use createPanel() !
     */
    void startPanel(ListPanel *panel, QString path);
    /**
     * Called by Konfigurator to recreate the panels after changing the configuration
     */
    void recreatePanels();
    /**
     * Swaps the left / right directions of the panel
     */
    void swapPanels()             {_left = !_left;}
	 void saveSettings(KConfig *config, const QString& key);

  public slots:
    /**
     * Called externally to start a new tab. Example of usage would be the "open in a new tab"
     * action, from the context-menu.
     */
    void slotNewTab(QString path);
    void slotNewTab();
    void slotCloseTab();

  protected slots:
    void slotChangePanel(ListPanel *p);
    void slotRefreshActions();

  private:
    QGridLayout *_layout;
    QHBoxLayout *_barLayout;
    bool _left;
    PanelTabBar *_tabbar;
    QWidgetStack *_stack;
    QToolButton *_newTab, *_closeTab;
    ListPanel *&_self, *&_other, *&_active;
};


#endif // _PANEL_MANAGER_H
