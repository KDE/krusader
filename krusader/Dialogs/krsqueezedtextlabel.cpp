#include "krsqueezedtextlabel.h"
#include <kstringhandler.h>
#include <QtGui/QToolTip>
#include <QMouseEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QLabel>
#include <QPainter>
#include <kurl.h>

KrSqueezedTextLabel::KrSqueezedTextLabel(QWidget *parent):
        KSqueezedTextLabel(parent), acceptDrops(false), _index(-1), _length(-1)
{
    setAutoFillBackground(true);
}


KrSqueezedTextLabel::~KrSqueezedTextLabel()
{
}

void KrSqueezedTextLabel::mousePressEvent(QMouseEvent *)
{
    emit clicked();

}

void KrSqueezedTextLabel::enableDrops(bool flag)
{
    setAcceptDrops(acceptDrops = flag);
}

void KrSqueezedTextLabel::dropEvent(QDropEvent *e)
{
    emit dropped(e);
}

void KrSqueezedTextLabel::dragEnterEvent(QDragEnterEvent *e)
{
    if (acceptDrops) {
        KUrl::List URLs = KUrl::List::fromMimeData(e->mimeData());
        e->setAccepted(!URLs.isEmpty());
    } else
        KSqueezedTextLabel::dragEnterEvent(e);
}

void KrSqueezedTextLabel::squeezeTextToLabel(int index, int length)
{
    if (index == -1 || length == -1)
        KSqueezedTextLabel::squeezeTextToLabel();
    else {
        QString sqtext = fullText;
        QFontMetrics fm(fontMetrics());
        int labelWidth = size().width();
        int textWidth = fm.width(sqtext);
        if (textWidth > labelWidth) {
            int avgCharSize = textWidth / sqtext.length();
            int numOfExtraChars = (textWidth - labelWidth) / avgCharSize;
            int delta;

            // remove as much as possible from the left, and then from the right
            if (index > 3) {
                delta = qMin(index, numOfExtraChars);
                numOfExtraChars -= delta;
                sqtext.replace(0, delta, "...");
            }

            if (numOfExtraChars > 0 && ((int)sqtext.length() > length + 3)) {
                delta = qMin(numOfExtraChars, (int)sqtext.length() - (length + 3));
                sqtext.replace(sqtext.length() - delta, delta, "...");
            }
            QLabel::setText(sqtext);

            setToolTip(QString::null);
            setToolTip(fullText);
        } else {
            QLabel::setText(fullText);

            setToolTip(QString::null);
            QToolTip::hideText();
        }
    }
}

void KrSqueezedTextLabel::setText(const QString &text, int index, int length)
{
    _index = index;
    _length = length;
    fullText = text;
    KSqueezedTextLabel::setText(fullText);
    squeezeTextToLabel(_index, _length);
}

void KrSqueezedTextLabel::paintEvent(QPaintEvent * e)
{
    {
        QPainter painter(this);

        QRect cr = contentsRect();
        painter.fillRect(cr, palette().brush(backgroundRole()));
    }

    KSqueezedTextLabel::paintEvent(e);
}

#include "krsqueezedtextlabel.moc"

