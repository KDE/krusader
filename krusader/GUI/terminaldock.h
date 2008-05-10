//
// C++ Interface: terminaldock
//
// Description: A widget containing the konsolepart for the Embedded terminal emulator
//
//
// Author: Vaclav Juza (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef TERMINALDOCK_H
#define TERMINALDOCK_H

#include <kde_terminal_interface.h>
#include <kparts/part.h>

#include <QHBoxLayout>
#include <QWidget>
#include <QEvent>
#include <QString>
#include <QObject>

/**
 * This is a class to make the work with the embedded terminal emulator easier.
 * It takes care of loading the library and initialising when necessary.
 * A big advantage is that the code which uses this class does not need to
 * check if the part was loaded successfully.
 * 
 */
class TerminalDock : public QWidget
{
  Q_OBJECT
public slots:
  void killTerminalEmulator();
public:
  TerminalDock(QWidget* parent);
  virtual ~TerminalDock();
  void sendInput(const QString& input);
  void sendCd(const QString& path);
  virtual bool eventFilter( QObject * watched, QEvent * e );
  bool isTerminalVisible() const;
  bool isInitialised() const;
  bool initialise();
  virtual void hideEvent(QHideEvent * e);
  virtual void showEvent(QShowEvent * e);
  inline KParts::Part* part()
  {
    return konsole_part;
  }
private:
  QString lastPath;                       // path of the last sendCd
  QHBoxLayout *terminal_hbox;             // hbox for terminal_dock
  KParts::ReadOnlyPart* konsole_part;     // the actual part pointer
  TerminalInterface* t;                   // TerminalInterface of the konsole part
  bool initialised;
  bool applyShortcuts( QKeyEvent * ke );
};

#endif /* TERMINALDOCK_H */
