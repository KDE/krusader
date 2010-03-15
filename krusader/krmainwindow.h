#ifndef KRMAINWINDOW_H
#define KRMAINWINDOW_H

class QWidget;
class KrView;
class QAction;
class KActionCollection;

// abstract interface to the main window
class KrMainWindow
{
public:
    virtual QWidget *widget() = 0;
    virtual KrView *activeView() = 0;
    virtual KActionCollection *actions() = 0;

    void enableAction(const char *name, bool enable);
    QAction *action(const char *name);
};

#endif
