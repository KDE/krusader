#ifndef KRUSADERAPP_H
#define KRUSADERAPP_H

#include <kapplication.h>
class QFocusEvent;

// declare a dummy kapplication, just to get Qt's focusin focusout events
class KrusaderApp: public KApplication
{
    Q_OBJECT
public:
    KrusaderApp();
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);

signals:
    void windowActive();
    void windowInactive();
};


#endif // KRUSADERAPP_H
