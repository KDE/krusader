#include "krsqueezedtextlabel.h"
#include <kstringhandler.h>
#include <kurldrag.h>
#include <qtooltip.h>

KrSqueezedTextLabel::KrSqueezedTextLabel(QWidget *parent, const char *name):
  KSqueezedTextLabel(parent, name), acceptDrops( false ), _index(-1), _length(-1) {
}


KrSqueezedTextLabel::~KrSqueezedTextLabel() {
}

void KrSqueezedTextLabel::mousePressEvent(QMouseEvent *) {
  emit clicked();
  
}

void KrSqueezedTextLabel::enableDrops( bool flag )
{
  setAcceptDrops( acceptDrops = flag );
}

void KrSqueezedTextLabel::dropEvent(QDropEvent *e) {
  emit dropped(e);
}

void KrSqueezedTextLabel::dragEnterEvent(QDragEnterEvent *e) {
  if( acceptDrops )
    e->accept( KURLDrag::canDecode( e ) );
  else
    KSqueezedTextLabel::dragEnterEvent( e );
}

void KrSqueezedTextLabel::squeezeTextToLabel(int index, int length) {
	if (index==-1 || length==-1)
		KSqueezedTextLabel::squeezeTextToLabel();
	else {
		QString sqtext=fullText;
		QFontMetrics fm(fontMetrics());
		int labelWidth = size().width();
		int textWidth = fm.width(sqtext);
		if (textWidth > labelWidth) {
			int avgCharSize = textWidth / sqtext.length();
			int numOfExtraChars = (textWidth-labelWidth)/avgCharSize;
			int delta;
			
			// remove as much as possible from the left, and then from the right
			if (index>3) {
				delta=QMIN(index, numOfExtraChars);
				numOfExtraChars -= delta;
				sqtext.replace(0, delta, "...");
			}
			
			if (numOfExtraChars>0 && ((int)sqtext.length() > length+3)) {
				delta = QMIN(numOfExtraChars, (int)sqtext.length() - (length+3));
				sqtext.replace(sqtext.length()-delta, delta, "...");
			}
			QLabel::setText(sqtext);

			QToolTip::remove( this );
			QToolTip::add( this, fullText );
		} else {
			QLabel::setText(fullText);

			QToolTip::remove( this );
			QToolTip::hide();
		}
	}
}

void KrSqueezedTextLabel::setText( const QString &text, int index, int length ) {
	_index=index;
	_length=length;
	fullText = text;
	squeezeTextToLabel(_index,_length);
}

#include "krsqueezedtextlabel.moc"

