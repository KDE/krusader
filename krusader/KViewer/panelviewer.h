#ifndef _SUPERVIEW_H
#define _SUPERVIEW_H

#include <kurl.h>
#include <qstring.h>
#include <qwidgetstack.h>
#include <kparts/part.h>
#include <qdict.h>
#include <qlabel.h>

class PanelViewer: public QWidgetStack {
  Q_OBJECT
  public slots:
    bool openURL(const KURL &url);
    bool closeURL();
    
  signals:
    void openURLRequest(const KURL &url);
  
  public:
    PanelViewer(QWidget *parent = 0);
    ~PanelViewer();
    inline KURL url() const;
    inline KParts::ReadOnlyPart *part() const;

  protected:
    QDict<KParts::ReadOnlyPart> *mimes;
    KParts::ReadOnlyPart *getPart(QString mimetype);
    KParts::ReadOnlyPart *cpart;
    QString cmimetype;
    KURL curl;   
    QLabel *fallback;     
};



#endif
