#ifndef KRSQUEEZEDTEXTLABEL_H
#define KRSQUEEZEDTEXTLABEL_H

#include <ksqueezedtextlabel.h>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QDropEvent>
#include <QDragEnterEvent>

class QMouseEvent;
class QDropEvent;
class QDragEnterEvent;
class QPaintEvent;

/**
This class overloads KSqueezedTextLabel and simply adds a clicked signal,
so that users will be able to click the label and switch focus between panels.

NEW: a special setText() method allows to choose which part of the string should
     be displayed (example: make sure that search results won't be cut out)
*/
class KrSqueezedTextLabel : public KSqueezedTextLabel
{
    Q_OBJECT
public:
    KrSqueezedTextLabel(QWidget *parent = 0);
    ~KrSqueezedTextLabel();

    void enableDrops(bool flag);

public slots:
    void setText(const QString &text, int index = -1, int length = -1);

signals:
    void clicked(); /**< emitted when someone clicks on the label */
    void dropped(QDropEvent *); /**< emitted when someone drops URL onto the label */

protected:
    void resizeEvent(QResizeEvent *) {
        squeezeTextToLabel(_index, _length);
    }
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void dropEvent(QDropEvent *e);
    virtual void dragEnterEvent(QDragEnterEvent *e);
    virtual void paintEvent(QPaintEvent * e);
    void squeezeTextToLabel(int index = -1, int length = -1);

    QString fullText;

private:
    bool  acceptDrops;
    int _index, _length;
};

#endif
