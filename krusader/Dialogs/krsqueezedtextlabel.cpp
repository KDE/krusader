#include "krsqueezedtextlabel.h"

KrSqueezedTextLabel::KrSqueezedTextLabel(QWidget *parent, const char *name): KSqueezedTextLabel(parent, name) {
}


KrSqueezedTextLabel::~KrSqueezedTextLabel() {
}

void KrSqueezedTextLabel::mousePressEvent(QMouseEvent *) {
  emit clicked();
  
}

#include "krsqueezedtextlabel.moc"

