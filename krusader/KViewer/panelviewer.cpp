#include <kurl.h>
#include <qstring.h>
#include <qwidgetstack.h>
#include <kparts/part.h>
#include <kparts/browserextension.h>
#include <qdict.h>
#include <qlabel.h>
#include <kmimetype.h>
#include <klocale.h>
#include <klibloader.h>
#include <kuserprofile.h>
#include "panelviewer.h"

#define DICTSIZE 211

PanelViewer::PanelViewer(QWidget *parent):
  QWidgetStack(parent) {
  setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Ignored ) );
  
  mimes = new QDict<KParts::ReadOnlyPart>(DICTSIZE, false);
  cpart = 0;
  fallback = new QLabel(i18n("No file selected or selected file can't be displayed."), this);
  fallback->setAlignment(AlignCenter | ExpandTabs | WordBreak);
  addWidget(fallback);
  raiseWidget(fallback);
}

PanelViewer::~PanelViewer() { 
  mimes->clear();
  delete mimes;
  delete fallback;
}

bool PanelViewer::openURL(const KURL &url) {
  closeURL();
  curl = url;
  cmimetype = KMimeType::findByURL(curl)->name();
  cpart = (*mimes)[cmimetype];
  if (!cpart) cpart = getPart(cmimetype);
  if (!cpart) cpart = getPart("text/plain");
  if (!cpart) cpart = getPart("all/allfiles");
  if (cpart) {
    mimes->insert(cmimetype, cpart);
    addWidget(cpart->widget());
    raiseWidget(cpart->widget());
  }
  if (cpart && cpart->openURL(curl)) return true;
  else {
    raiseWidget(fallback);
    return false;
  }
}

bool PanelViewer::closeURL() {
  raiseWidget(fallback);
  if (cpart && cpart->closeURL()) return true;
  return false;
}

KParts::ReadOnlyPart *PanelViewer::getPart(QString mimetype) {
  KParts::ReadOnlyPart *part = 0L;
  KLibFactory  *factory = 0;
  KService::Ptr ptr = KServiceTypeProfile::preferredService(mimetype,"KParts/ReadOnlyPart");
  if (ptr) {
    QStringList args;
    QVariant argsProp = ptr->property( "X-KDE-BrowserView-Args" );
    if ( argsProp.isValid() )
    {
      QString argStr = argsProp.toString();
      args = QStringList::split( " ", argStr );
    }
    QVariant prop = ptr->property( "X-KDE-BrowserView-AllowAsDefault" );
    if ( !prop.isValid() || prop.toBool() ) // defaults to true
    {
      factory = KLibLoader::self()->factory( ptr->library().latin1() );
      if (factory) {
        part = static_cast<KParts::ReadOnlyPart *>(factory->create(this,
                             ptr->name().latin1(), QString("KParts::ReadOnlyPart").latin1(), args ));
      }
    }
  }
  if (part) {
    KParts::BrowserExtension *ext = KParts::BrowserExtension::childObject(part);
    if (ext) {
      connect(ext, SIGNAL(openURLRequestDelayed(const KURL &, const KParts::URLArgs &)), this, SLOT(openURL(const KURL &)));
      connect(ext, SIGNAL(openURLRequestDelayed(const KURL &, const KParts::URLArgs &)), this, SIGNAL(openURLRequest(const KURL &)));
    }
  }
  return part;
}

inline KURL PanelViewer::url() const { return curl; }
inline KParts::ReadOnlyPart *PanelViewer::part() const { return cpart; }

#include "panelviewer.moc"
