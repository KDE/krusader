#ifndef KRSQUEEZEDTEXTLABEL_H
#define KRSQUEEZEDTEXTLABEL_H

#include <ksqueezedtextlabel.h>

class QMouseEvent;
class QDropEvent;
class QDragEnterEvent;

/**
This class overloads KSqueezedTextLabel and simply adds a clicked signal,
so that users will be able to click the label and switch focus between panels.
*/
class KrSqueezedTextLabel : public KSqueezedTextLabel {
Q_OBJECT
  public:
    KrSqueezedTextLabel(QWidget *parent = 0, const char *name = 0);
    ~KrSqueezedTextLabel();

    void enableDrops( bool flag );

  signals:
    void clicked(); /**< emitted when someone clicks on the label */
    void dropped(QDropEvent *); /**< emitted when someone drops URL onto the label */

  protected:
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void dropEvent(QDropEvent *e);
    virtual void dragEnterEvent(QDragEnterEvent *e);

  private:
    bool  acceptDrops;
};

#endif
