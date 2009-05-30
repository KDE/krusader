/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/** This file was modified for Krusader by Shie Erlich & Rafi Yanai **/

#ifndef __kr_progress_h__
#define __kr_progress_h__

#include <qlabel.h>
#include <QCloseEvent>

#include <kio/global.h>

#include <kprogressdialog.h>
#include <ksqueezedtextlabel.h>

#include <kio/progressbase.h>

#include <qobject.h>

class KrProgress : public KIO::ProgressBase {
  Q_OBJECT
public:

  KrProgress(KIO::Job* job);
  virtual ~KrProgress();

public slots:
  virtual void slotTotalSize( KIO::Job*, KIO::filesize_t bytes );
  virtual void slotTotalFiles( KIO::Job*, unsigned long files );
  virtual void slotTotalDirs( KIO::Job*, unsigned long dirs );

  virtual void slotProcessedSize( KIO::Job*, KIO::filesize_t bytes );
  virtual void slotProcessedFiles( KIO::Job*, unsigned long files );
  virtual void slotProcessedDirs( KIO::Job*, unsigned long dirs );

  virtual void slotSpeed( KIO::Job*, unsigned long bytes_per_second );
  virtual void slotPercent( KIO::Job*, unsigned long percent );
  virtual void slotInfoMessage( KIO::Job*, const QString & msg );
	
	virtual void slotStop();
  virtual void closeEvent( QCloseEvent* );

protected:
  void showTotals();
  void setDestVisible( bool visible );

  KSqueezedTextLabel* sourceLabel;
  KSqueezedTextLabel* destLabel;
  QLabel* progressLabel;
  QLabel* destInvite;
  QLabel* speedLabel;
  QLabel* sizeLabel;
  QLabel* resumeLabel;

  KProgress* m_pProgressBar;

  KIO::filesize_t m_iTotalSize;
  unsigned long m_iTotalFiles;
  unsigned long m_iTotalDirs;

  KIO::filesize_t m_iProcessedSize;
  unsigned long m_iProcessedDirs;
  unsigned long m_iProcessedFiles;

protected:
  virtual void virtual_hook( int id, void* data );
};


#endif // __kr_progress_h__

