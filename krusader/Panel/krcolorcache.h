#ifndef KRCOLORCACHE_H
#define KRCOLORCACHE_H

#include <qobject.h>
#include <qdict.h>

class KrColorCache : public QObject
{
  Q_OBJECT
  KrColorCache();
  static KrColorCache * instance;
  QDict<QColor> colorCache;
  QDict<QString> textCache;
  bool kdeDefault, alternateBackgroundEnabled, showCurrentItemAlways;
public:
  static KrColorCache & getColorCache();
  const QString & getTextValue(const QString & textName) const;
  const QColor * getColor(const QString & colorName) const;
  const QColor * getForegroundColor() const;
  const QColor * getDirectoryForegroundColor() const;
  const QColor * getExecutableForegroundColor() const;
  const QColor * getSymlinkForegroundColor() const;
  const QColor * getInvalidSymlinkForegroundColor() const;
  const QColor * getMarkedForegroundColor() const;
  const QColor * getMarkedBackgroundColor() const;
  const QColor * getCurrentForegroundColor() const;
  const QColor * getCurrentBackgroundColor() const;
  const QColor * getBackgroundColor() const;
  const QColor * getAlternateBackgroundColor() const;
  const QColor * getAlternateMarkedBackgroundColor() const;
  bool isKDEDefault() const {return kdeDefault;}
  bool isAlternateBackgroundEnabled() const {return alternateBackgroundEnabled;}
  bool isShowCurrentItemAlways() const {return showCurrentItemAlways;}
  
public slots:
  void refreshColors();
signals:
  void colorsRefreshed();
};

#endif
