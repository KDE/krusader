/***************************************************************************
                          krservices.h  -  description
                             -------------------
    begin                : Thu Aug 8 2002
    copyright            : (C) 2002 by Shie Erlich & Rafi Yanai
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

#ifndef KRSERVICES_H
#define KRSERVICES_H

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <k3process.h>

/**
  *@author Shie Erlich & Rafi Yanai
  */

class KrServices {
public: 
	KrServices(){}
	~KrServices(){}

	static bool         cmdExist(QString cmdName);
	static QString      detectFullPathName( QString name );
	static QString      fullPathName( QString name, QString confName = QString::null );
	static QStringList  separateArgs( QString args );
	static QString      registerdProtocol(QString mimetype);
	static void         clearProtocolCache();
	static bool         fileToStringList(Q3TextStream *stream, QStringList& target, bool keepEmptyLines=false);
	static QString		  quote( QString name );
	static QStringList  quote( const QStringList& names );

protected:
	static QString 	  escape( QString name );

private:
	static QMap<QString,QString>* slaveMap;

};


// TODO: make KrServices a namespace and move it there

// wraps over kprocess, but buffers stdout and stderr and allows easy access to them later
// note, that you still have to enable stdout,stderr in KEasyProcess::start() for buffering
// to happen (ie: start(KEasyProcess::Block, KEasyProcess::AllOutput);)
class KEasyProcess: public K3Process {
	Q_OBJECT
public:
	KEasyProcess(QObject *parent, const char *name=0);
	KEasyProcess();
	virtual ~KEasyProcess() {}

	const QString& getStdout() const { return _stdout; }
	const QString& getStderr() const { return _stderr; }

protected slots:
	void receivedStdout (K3Process *proc, char *buffer, int buflen);
	void receivedStderr (K3Process *proc, char *buffer, int buflen);
	void init();

private:
	QString _stdout, _stderr;
};

#endif
