//
// C++ Interface: kraction
//
// Description: 
//
//
// Author: Shie Erlich and Rafi Yanai <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef KRACTION_H
#define KRACTION_H

#include <kaction.h>
#include <kprocess.h>
#include <kdialogbase.h>


class UserActionProperties;
class QTextEdit;
class KActionCollection;

/**
 * This diesplays the output of a process
 * @author Shie Erlich
 */
class KrActionProcDlg: public KDialogBase {
      Q_OBJECT
   public:
      KrActionProcDlg( QString caption, bool enableStderr = false, QWidget *parent = 0 );

   protected slots:
      void addStderr( KProcess *proc, char *buffer, int buflen );
      void addStdout( KProcess *proc, char *buffer, int buflen );

   private:
      QTextEdit *_stdout, *_stderr;
};

/**
 * This executes a command of a UserAction
 * @author Shie Erlich
 * @todo jonas: call a list of commands separately (I began it but it doesn't work)
 */
class KrActionProc: public QObject {
      Q_OBJECT
   public:

      KrActionProc( UserActionProperties* prop );
      ~KrActionProc();
      void start( QString cmdLine );
      void start( QStringList cmdLineList );

   protected slots:
      void kill() { _proc->kill( SIGINT ); }
      void processExited( KProcess *proc );

   private:
      UserActionProperties* _properties;
      KProcess *_proc;
      QString _stdout;
      QString _stderr;
      KrActionProcDlg *_output;
};

/**
 * This subclass of KAction extends it with an individual executor and a struct UserActionProperties. It is used to integrate useractions into KDE's KAction-System
 * @author Jonas Bähr (http://www.jonas-baehr.de)
 */
class KrAction: public KAction {
   Q_OBJECT
   public:
      KrAction(UserActionProperties* prop, KActionCollection *parent);
      ~KrAction();
      /**
       * @param prop gives the KrAction these properties
       */
      void setProperties(UserActionProperties* prop);
      /**
       * @return the properties of the KrAction
       */
      UserActionProperties* properties();
      
      /**
       * This chekcs if the KrAction is for a specific file / location available
       * @param currentURL Check for this file
       * @return true if the KrAction if available
       */
      bool isAvailable( const KURL& currentURL );
      
   public slots:
      /**
       * connected to KAction's activated signal
       */
      void exec();

   private:
   UserActionProperties *_properties;
};

#endif //KRACTION_H
