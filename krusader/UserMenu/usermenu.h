/***************************************************************************
                      usermenu.h  -  description
                         -------------------
begin                : Sat Dec 6 2003
copyright            : (C) 2003 by Shie Erlich & Rafi Yanai
email                :
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef USERMENU_H
#define USERMENU_H

#include <qwidget.h>
#include <kpopupmenu.h>
#include <qstringlist.h>
#include <kaction.h>
#include <kprocess.h>
#include <kdialogbase.h>
#include <qtextedit.h>

class KDialogBase;

class UserMenuProcDlg: public KDialogBase {
      Q_OBJECT
   public:
      UserMenuProcDlg( QString caption, bool enableStderr = false, QWidget *parent = 0 );

   protected slots:
      void addStderr( KProcess *proc, char *buffer, int buflen );
      void addStdout( KProcess *proc, char *buffer, int buflen );

   private:
      QTextEdit *_stdout, *_stderr;
};

class UserMenuProc: public QObject {
      Q_OBJECT
   public:

      enum ExecType { Terminal, OutputOnly, None };

      UserMenuProc(ExecType execType = Terminal, bool enableStderr = false);
      ~UserMenuProc();
      bool start( QString cmdLine );

   protected slots:
      void kill() { _proc->kill( SIGINT ); }
      void processExited( KProcess *proc );

   private:
      ExecType _execType;
      bool _enableStderr;
      KProcess *_proc;
      QString _stdout;
      QString _stderr;
      UserMenuProcDlg *_output;
};

///////////////////////////////////////////////////////////////////////////////

class UserMenuGui: public KPopupMenu {
      Q_OBJECT
   public:
      UserMenuGui( QWidget *parent = 0 );
      QString run();

   protected:
      void readEntries();

   private:
      QStringList _entries;
};

////////////////////////////////////////////////////////////////////////////////

// an expander is a function that receives a QString input, expands
// it and returns a new QString output containing the expanded expression
typedef QString ( *EXPANDER ) ( const QString& );

// a UMCmd is an entry containing the expression and its expanding function
typedef struct UserMenuCmd {
   QString expression;
   EXPANDER expFunc;
}
UMCmd;

class UserMenu : public QWidget {
#define NUM_EXPS  1

      Q_OBJECT
   public:
      /**
      * Executes the menu, does the work and returns a QString
      * containing a command to run. Run that command from a shell and that's it.
      */
      void exec();

      /**
      * cycle through the input line, replacing every %% expression with valid
      * data from krusader. return the expanded string
      */
      static QString expand( QString str );

      UserMenu( QWidget *parent = 0, const char *name = 0 );

   protected:
      static QString expPath( const QString& str );

   private:
      UserMenuGui _popup;
      static UMCmd _expressions[ NUM_EXPS ];
};

#endif
