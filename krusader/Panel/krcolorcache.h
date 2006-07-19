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
  const QString & getTextValue(const QString & textName) const;
  QColor getColor(const QString & colorName) const;
  void setColor(const QString & colorName, QColor * color, bool isActive, bool isBackgroundColor=false);
  QColor getForegroundColor_Int(bool isActive) const;
  QColor getDirectoryForegroundColor_Int(bool isActive) const;
  QColor getExecutableForegroundColor_Int(bool isActive) const;
  QColor getSymlinkForegroundColor_Int(bool isActive) const;
  QColor getInvalidSymlinkForegroundColor_Int(bool isActive) const;
  QColor getMarkedForegroundColor_Int(bool isActive) const;
  QColor getMarkedBackgroundColor_Int(bool isActive) const;
  QColor getCurrentForegroundColor_Int(bool isActive) const;
  QColor getCurrentBackgroundColor_Int(bool isActive) const;
  QColor getCurrentMarkedForegroundColor_Int(bool isActive) const;
  QColor getBackgroundColor_Int(bool isActive) const;
  QColor getAlternateBackgroundColor_Int(bool isActive) const;
  QColor getAlternateMarkedBackgroundColor_Int(bool isActive) const;
public:
  static QColor dimColor(const QColor & color, int dim, const QColor & targetColor);
  static KrColorCache & getColorCache();
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
