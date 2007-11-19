#include "krusaderapp.h"

KrusaderApp::KrusaderApp(): KApplication() {}

void KrusaderApp::focusInEvent(QFocusEvent *event) {
	emit windowActive();
}

void KrusaderApp::focusOutEvent(QFocusEvent *event) {
	emit windowInactive();
}
