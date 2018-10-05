/*****************************************************************************
 * Copyright (C) 2003 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#ifndef SPLITTER_H
#define SPLITTER_H

// QtCore
#include <QString>
#include <QUrl>
// QtWidgets
#include <QProgressDialog>

#include <KIO/Job>

#include "crc32.h"

class Splitter : public QProgressDialog
{
    Q_OBJECT

public:
    Splitter(QWidget* parent,  QUrl fileNameIn, QUrl destinationDirIn, bool overWriteIn);
    ~Splitter() override;

    void split(KIO::filesize_t splitSizeIn);

private slots:
    void splitDataReceived(KIO::Job *, const QByteArray &);
    void splitDataSend(KIO::Job *, QByteArray &);
    void splitSendFinished(KJob *);
    void splitReceiveFinished(KJob *);
    void splitReceivePercent(KJob *, unsigned long);
    void splitFileSend(KIO::Job *, QByteArray &);
    void splitFileFinished(KJob *);
    void statOutputFileResult(KJob* job);

private:
    void splitAbortJobs();
    void nextOutputFile();
    void openOutputFile();


    // parameters
    QUrl            fileName;
    QUrl            destinationDir;
    KIO::filesize_t splitSize;
    int             permissions;
    bool            overwrite;

    // current split file stuff
    int             fileNumber;
    QUrl            writeURL;
    // how much can still be written to the current output file
    KIO::filesize_t outputFileRemaining;

    QByteArray      transferArray;
    KIO::filesize_t receivedSize;
    QString         splitInfoFileContent;
    CRC32          *crcContext;
    KIO::Job         *statJob;
    KIO::TransferJob *splitReadJob;
    KIO::TransferJob *splitWriteJob;
};

#endif /* __SPLITTER_H__ */
