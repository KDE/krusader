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
  const QColor * getForegroundColor(bool isActive) const;
  const QColor * getDirectoryForegroundColor(bool isActive) const;
  const QColor * getExecutableForegroundColor(bool isActive) const;
  const QColor * getSymlinkForegroundColor(bool isActive) const;
  const QColor * getInvalidSymlinkForegroundColor(bool isActive) const;
  const QColor * getMarkedForegroundColor(bool isActive) const;
  const QColor * getMarkedBackgroundColor(bool isActive) const;
  const QColor * getCurrentForegroundColor(bool isActive) const;
  const QColor * getCurrentBackgroundColor(bool isActive) const;
  const QColor * getBackgroundColor(bool isActive) const;
  const QColor * getAlternateBackgroundColor(bool isActive) const;
  const QColor * getAlternateMarkedBackgroundColor(bool isActive) const;
  bool isKDEDefault() const {return kdeDefault;}
  bool isAlternateBackgroundEnabled() const {return alternateBackgroundEnabled;}
  bool isShowCurrentItemAlways() const {return showCurrentItemAlways;}
  
public slots:
  void refreshColors();
signals:
  void colorsRefreshed();
};

#endif
