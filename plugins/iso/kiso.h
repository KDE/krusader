/*
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2002 Szombathelyi György <gyurco@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Leo Savernik <l.savernik@aon.at>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    This file is heavily based on ktar from kdelibs

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KISO_H
#define KISO_H

// QtCore
#include <QDateTime>
#include <QString>
#include <QStringList>

#include "../../app/krdebuglogger.h"
#include "kisodirectory.h"
#include "kisofile.h"

/**
 * @short A class for reading (optionally compressed) iso9660 files.
 */
class KIso : public KArchive
{
public:
    /**
     * Creates an instance that operates on the given filename.
     * using the compression filter associated to given mimetype.
     *
     * @param filename is a local path (e.g. "/home/weis/myfile.tgz")
     * @param mimetype "application/x-gzip" or "application/x-bzip2"
     * Do not use application/x-tgz or so. Only the compression layer !
     * If the mimetype is omitted, it will be determined from the filename.
     */
    explicit KIso(const QString &filename, const QString &mimetype = QString());

    /**
     * Creates an instance that operates on the given device.
     * The device can be compressed (KFilterDev) or not (QFile, etc.).
     * WARNING: don't assume that giving a QFile here will decompress the file,
     * in case it's compressed!
     */
    explicit KIso(QIODevice *dev);

    /**
     * If the .iso is still opened, then it will be
     * closed automatically by the destructor.
     */
    virtual ~KIso();

    /**
     * The name of the os file, as passed to the constructor
     * Null if you used the QIODevice constructor.
     */
    QString fileName()
    {
        return m_filename;
    }

    bool writeDir(const QString &, const QString &, const QString &, mode_t, time_t, time_t, time_t);
    bool writeSymLink(const QString &, const QString &, const QString &, const QString &, mode_t, time_t, time_t, time_t);
    bool prepareWriting(const QString &, const QString &, const QString &, qint64, mode_t, time_t, time_t, time_t);
    bool finishWriting(qint64);

    void setStartSec(int startsec)
    {
        m_startsec = startsec;
    }
    int startSec()
    {
        return m_startsec;
    }

    bool showhidden, showrr;
    int level, joliet;
    KIsoDirectory *dirent;

protected:
    /**
     * Opens the archive for reading.
     * Parses the directory listing of the archive
     * and creates the KArchiveDirectory/KArchiveFile entries.
     *
     */
    void readParams();
    virtual bool openArchive(QIODevice::OpenMode mode) override;
    virtual bool closeArchive() override;
    virtual bool doWriteDir(const QString &, const QString &, const QString &, mode_t, const QDateTime &, const QDateTime &, const QDateTime &) override;
    virtual bool
    doWriteSymLink(const QString &, const QString &, const QString &, const QString &, mode_t, const QDateTime &, const QDateTime &, const QDateTime &)
        override;
    virtual bool
    doPrepareWriting(const QString &, const QString &, const QString &, qint64, mode_t, const QDateTime &, const QDateTime &, const QDateTime &) override;
    virtual bool doFinishWriting(qint64) override;

private:
    /**
     * @internal
     */
    void addBoot(struct el_torito_boot_descriptor *bootdesc);
    void prepareDevice(const QString &filename, const QString &mimetype, bool forced = false);
    int m_startsec;

    QString m_filename;

protected:
    virtual void virtual_hook(int id, void *data) override;

private:
    class KIsoPrivate;
    KIsoPrivate *d;
};

#endif
