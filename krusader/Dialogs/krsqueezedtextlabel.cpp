#include "krsqueezedtextlabel.h"
#include <kurldrag.h>

KrSqueezedTextLabel::KrSqueezedTextLabel(QWidget *parent, const char *name):
  KSqueezedTextLabel(parent, name), acceptDrops( false ) {
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

#include "krsqueezedtextlabel.moc"

