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
  const QColor & getColor(const QString & colorName) const;
  QColor getForegroundColor(bool isActive) const;
  QColor getDirectoryForegroundColor(bool isActive) const;
  QColor getExecutableForegroundColor(bool isActive) const;
  QColor getSymlinkForegroundColor(bool isActive) const;
  QColor getInvalidSymlinkForegroundColor(bool isActive) const;
  QColor getMarkedForegroundColor(bool isActive) const;
  QColor getMarkedBackgroundColor(bool isActive) const;
  QColor getCurrentForegroundColor(bool isActive) const;
  QColor getCurrentBackgroundColor(bool isActive) const;
  QColor getCurrentMarkedForegroundColor(bool isActive) const;
  QColor getBackgroundColor(bool isActive) const;
  QColor getAlternateBackgroundColor(bool isActive) const;
  QColor getAlternateMarkedBackgroundColor(bool isActive) const;
  bool isKDEDefault() const {return kdeDefault;}
  bool isAlternateBackgroundEnabled() const {return alternateBackgroundEnabled;}
  bool isShowCurrentItemAlways() const {return showCurrentItemAlways;}
  
public slots:
  void refreshColors();
signals:
  void colorsRefreshed();
};

#endif
