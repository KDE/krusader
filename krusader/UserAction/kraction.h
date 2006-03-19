//
// C++ Interface: kraction
//
// Description: 
//
//
// Author: Krusader Krew <http://www.krusader.org>, (C) 2004, 2006
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
class QDomElement;
class QDomDocument;

/**
 * This subclass of KAction extends it with an individual executor and a struct UserActionProperties. It is used to integrate useractions into KDE's KAction-System
 * @author Jonas B�r (http://www.jonas-baehr.de)
 */
class KrAction: public KAction {
   Q_OBJECT
   public:
      KrAction( KActionCollection *parent, const char* name );
      ~KrAction();

      /**
       * This chekcs if the KrAction is for a specific file / location available
       * @param currentURL Check for this file
       * @return true if the KrAction if available
       */
      bool isAvailable( const KURL& currentURL );

      bool xmlRead( const QDomElement& element );
      QDomElement xmlDump( QDomDocument& doc ) const;

      void setName( const char* ) { /* empty reimplementation to prevent a name-change */ };

      QString category() const { return _category; };
      void setCategory( const QString& category ) { _category = category; };

      QString command() const { return _command; };
      void setCommand( const QString& command ) { _command = command; };

      QString user() const { return _user; };
      void setUser( const QString& user ) { _user = user; };

      QString startpath() const { return _startpath; };
      void setStartpath( const QString& startpath ) { _startpath = startpath; };

      enum ExecType { Normal, Terminal, CollectOutput, CollectOutputSeparateStderr };
      ExecType execType() const { return _execType; };
      void setExecType( ExecType execType ) { _execType = execType; };

      bool acceptURLs() const { return _acceptURLs; };
      void setAcceptURLs(const bool& acceptURLs) { _acceptURLs = acceptURLs; };

      bool confirmExecution() const { return _confirmExecution; };
      void setConfirmExecution(const bool& confirmExecution) { _confirmExecution = confirmExecution; };

      QStringList showonlyProtocol() const { return _showonlyProtocol; };
      void setShowonlyProtocol( const QStringList& showonlyProtocol ) { _showonlyProtocol = showonlyProtocol; };

      QStringList showonlyPath() const { return _showonlyPath; };
      void setShowonlyPath( const QStringList& showonlyPath ) { _showonlyPath = showonlyPath; };

      QStringList showonlyMime() const { return _showonlyMime; };
      void setShowonlyMime( const QStringList& showonlyMime ) { _showonlyMime = showonlyMime; };

      QStringList showonlyFile() const { return _showonlyFile; };
      void setShowonlyFile( const QStringList& showonlyFile ) { _showonlyFile = showonlyFile; };

   public slots:
      /**
       * connected to KAction's activated signal
       */
      void exec();

   private:
      void readCommand( const QDomElement& element );
      QDomElement dumpCommand( QDomDocument& doc ) const;

      void readAvailability( const QDomElement& element );
      QDomElement dumpAvailability( QDomDocument& doc ) const;

      QString _category;
      QString _command;
      QString _user;
      QString _startpath;
      ExecType _execType;
      bool _acceptURLs;
      bool _confirmExecution;
      QStringList _showonlyProtocol;
      QStringList _showonlyPath;
      QStringList _showonlyMime;
      QStringList _showonlyFile;

};


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

      KrActionProc( KrAction* action );
      ~KrActionProc();
      void start( QString cmdLine );
      void start( QStringList cmdLineList );

   protected slots:
      void kill() { _proc->kill( SIGINT ); }
      void processExited( KProcess *proc );

   private:
      KrAction* _action;
      KProcess *_proc;
      QString _stdout;
      QString _stderr;
      KrActionProcDlg *_output;
};


#endif //KRACTION_H
