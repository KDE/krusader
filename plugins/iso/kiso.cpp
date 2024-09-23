/*
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2002 Szombathelyi György <gyurco@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Leo Savernik <l.savernik@aon.at>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    This file is heavily based on ktar from kdelibs

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kiso.h"

// QtCore
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMimeDatabase>
#include <QMimeType>
#include <qplatformdefs.h>

#include <KCompressionDevice>
#include <KConfig>
#include <KConfigGroup>
#include <KFilterBase>

#include "libisofs/isofs.h"
#include "qfilehack.h"

#ifdef Q_OS_LINUX
#undef __STRICT_ANSI__
#include <linux/cdrom.h>
#define __STRICT_ANSI__
#include <fcntl.h>
#include <sys/ioctl.h>
#endif

////////////////////////////////////////////////////////////////////////
/////////////////////////// KIso ///////////////////////////////////
////////////////////////////////////////////////////////////////////////

/**
 * puts the track layout of the device 'fname' into 'tracks'
 * tracks structure: start sector, track number, ...
 * tracks should be 100*2 entry long (this is the maximum in the CD-ROM standard)
 * currently it's linux only, porters are welcome
 */

static int getTracks(const char *fname, int *tracks)
{
    KRFUNC;
    int ret = 0;
    memset(tracks, 0, 200 * sizeof(int));

#ifdef Q_OS_LINUX
    int fd, i;
    struct cdrom_tochdr tochead;
    struct cdrom_tocentry tocentry;

    // qDebug() << "getTracks open:" << fname << endl;
    fd = QT_OPEN(fname, O_RDONLY | O_NONBLOCK);
    if (fd > 0) {
        if (ioctl(fd, CDROMREADTOCHDR, &tochead) != -1) {
            //            qDebug() << "getTracks first track:" << tochead.cdth_trk0
            //            << " last track " << tochead.cdth_trk1 << endl;
            for (i = tochead.cdth_trk0; i <= tochead.cdth_trk1; ++i) {
                if (ret > 99)
                    break;
                memset(&tocentry, 0, sizeof(struct cdrom_tocentry));
                tocentry.cdte_track = static_cast<__u8>(i);
                tocentry.cdte_format = CDROM_LBA;
                if (ioctl(fd, CDROMREADTOCENTRY, &tocentry) < 0)
                    break;
                //                qDebug() << "getTracks got track " << i << " starting at: " <<
                //                tocentry.cdte_addr.lba << endl;
                if ((tocentry.cdte_ctrl & 0x4) == 0x4) {
                    tracks[ret << 1] = tocentry.cdte_addr.lba;
                    tracks[(ret << 1) + 1] = i;
                    ret++;
                }
            }
        }
        close(fd);
    }

#endif

    return ret;
}

class KIso::KIsoPrivate
{
public:
    KIsoPrivate() = default;
    QStringList dirList;
};

KIso::KIso(const QString &filename, const QString &_mimetype)
    : KArchive(nullptr)
{
    KRFUNC;
    KRDEBUG("Starting KIso: " << filename << " - type: " << _mimetype);

    m_startsec = -1;
    m_filename = filename;
    d = new KIsoPrivate;
    QString mimetype(_mimetype);
    bool forced = true;
    if (mimetype.isEmpty()) {
        QMimeDatabase db;
        QMimeType mt = db.mimeTypeForFile(filename, QMimeDatabase::MatchContent);
        if (mt.isValid())
            mimetype = mt.name();

        // qDebug() << "KIso::KIso mimetype=" << mimetype << endl;

        // Don't move to prepareDevice - the other constructor theoretically allows ANY filter
        if (mimetype == "application/x-tgz" || mimetype == "application/x-targz" || // the latter is deprecated but might still be around
            mimetype == "application/x-webarchive")
            // that's a gzipped tar file, so ask for gzip filter
            mimetype = "application/x-gzip";
        else if (mimetype == "application/x-tbz") // that's a bzipped2 tar file, so ask for bz2 filter
            mimetype = "application/x-bzip2";
        else {
            // Something else. Check if it's not really gzip though (e.g. for KOffice docs)
            QFile file(filename);
            if (file.open(QIODevice::ReadOnly)) {
                char firstByte;
                char secondByte;
                char thirdByte;
                file.getChar(&firstByte);
                file.getChar(&secondByte);
                file.getChar(&thirdByte);
                if (firstByte == 0037 && secondByte == (char)0213)
                    mimetype = "application/x-gzip";
                else if (firstByte == 'B' && secondByte == 'Z' && thirdByte == 'h')
                    mimetype = "application/x-bzip2";
                else if (firstByte == 'P' && secondByte == 'K' && thirdByte == 3) {
                    char fourthByte;
                    file.getChar(&fourthByte);
                    if (fourthByte == 4)
                        mimetype = "application/x-zip";
                }
            }
        }
        forced = false;
    }

    prepareDevice(filename, mimetype, forced);
}

void KIso::prepareDevice(const QString &filename, const QString &mimetype, bool forced)
{
    KRFUNC;
    KRDEBUG("Preparing: " << filename << " - type: " << mimetype << " - using the force: " << forced);
    /* 'hack' for Qt's false assumption that only S_ISREG is seekable */
    if ("inode/blockdevice" == mimetype)
        setDevice(new QFileHack(filename));
    else {
        if ("application/x-gzip" == mimetype || "application/x-bzip2" == mimetype)
            forced = true;

        KCompressionDevice *device;
        if (mimetype.isEmpty()) {
            device = new KCompressionDevice(filename);
        } else {
            device = new KCompressionDevice(filename, COMPRESSIONTYPEFORMIMETYPE(mimetype));
        }
        if (device->compressionType() == KCompressionDevice::None && forced) {
            delete device;
        } else {
            setDevice(device);
        }
    }
}

KIso::KIso(QIODevice *dev)
    : KArchive(dev)
{
    d = new KIsoPrivate;
}

KIso::~KIso()
{
    // mjarrett: Closes to prevent ~KArchive from aborting w/o device
    if (isOpen())
        close();
    if (!m_filename.isEmpty())
        delete device(); // we created it ourselves
    delete d;
}

/* callback function for libisofs */
static int readf(char *buf, unsigned int start, unsigned int len, void *udata)
{
    KRFUNC;

    QIODevice *dev = (static_cast<KIso *>(udata))->device();

    // seek(0) ensures integrity with the QIODevice's built-in buffer
    // see bug #372023 for details
    dev->seek(0);

    if (dev->seek((qint64)start << (qint64)11)) {
        if ((dev->read(buf, len << 11u)) != -1)
            return (len);
    }
    // qDebug() << "KIso::ReadRequest failed start: " << start << " len: " << len << endl;

    return -1;
}

/* callback function for libisofs */
static int mycallb(struct iso_directory_record *idr, void *udata)
{
    KRFUNC;

    auto *iso = static_cast<KIso *>(udata);
    QString path, user, group, symlink;
    int i;
    int access;
    time_t time, cdate, adate;
    rr_entry rr;
    bool special = false;
    KArchiveEntry *entry = nullptr, *oldentry = nullptr;
    char z_algo[2], z_params[2];
    long long z_size = 0;

    if ((idr->flags[0] & 1) && !iso->showhidden)
        return 0;
    if (iso->level) {
        if (isonum_711(idr->name_len) == 1) {
            switch (idr->name[0]) {
            case 0:
                path += (".");
                special = true;
                break;
            case 1:
                path += ("..");
                special = true;
                break;
            }
        }
        if (iso->showrr && ParseRR(idr, &rr) > 0) {
            if (!special)
                path = rr.name;
            symlink = rr.sl;
            access = rr.mode;
            time = rr.t_mtime;
            adate = rr.t_atime;
            cdate = rr.t_ctime;
            user.setNum(rr.uid);
            group.setNum(rr.gid);
            z_algo[0] = rr.z_algo[0];
            z_algo[1] = rr.z_algo[1];
            z_params[0] = rr.z_params[0];
            z_params[1] = rr.z_params[1];
            z_size = rr.z_size;
        } else {
            access = iso->dirent->permissions() & ~S_IFMT;
            adate = cdate = time = isodate_915(idr->date, 0);
            user = iso->dirent->user();
            group = iso->dirent->group();
            if (idr->flags[0] & 2)
                access |= S_IFDIR;
            else
                access |= S_IFREG;
            if (!special) {
                if (iso->joliet) {
                    for (i = 0; i < (isonum_711(idr->name_len) - 1); i += 2) {
                        void *p = &(idr->name[i]);
                        QChar ch(be2me_16(*(ushort *)p));
                        if (ch == ';')
                            break;
                        path += ch;
                    }
                } else {
                    for (i = 0; i < isonum_711(idr->name_len); ++i) {
                        if (idr->name[i] == ';')
                            break;
                        if (idr->name[i])
                            path += (idr->name[i]);
                    }
                }
                if (path.endsWith('.'))
                    path.resize(path.length() - 1);
            }
        }
        if (iso->showrr)
            FreeRR(&rr);
        if (idr->flags[0] & 2) {
            entry = new KIsoDirectory(iso, path, access | S_IFDIR, time, adate, cdate, user, group, symlink);
        } else {
            entry = new KIsoFile(iso,
                                 path,
                                 access,
                                 time,
                                 adate,
                                 cdate,
                                 user,
                                 group,
                                 symlink,
                                 (long long)(isonum_733(idr->extent)) << (long long)11,
                                 isonum_733(idr->size));
            if (z_size)
                (dynamic_cast<KIsoFile *>(entry))->setZF(z_algo, z_params, z_size);
        }
        iso->dirent->addEntry(entry);
    }
    if ((idr->flags[0] & 2) && (iso->level == 0 || !special)) {
        if (iso->level) {
            oldentry = iso->dirent;
            iso->dirent = dynamic_cast<KIsoDirectory *>(entry);
        }
        iso->level++;
        ProcessDir(&readf, isonum_733(idr->extent), isonum_733(idr->size), &mycallb, udata);
        iso->level--;
        if (iso->level)
            iso->dirent = dynamic_cast<KIsoDirectory *>(oldentry);
    }
    return 0;
}

void KIso::addBoot(struct el_torito_boot_descriptor *bootdesc)
{
    KRFUNC;

    int i;
    long long size;
    boot_head boot;
    boot_entry *be;
    QString path;
    KIsoFile *entry;

    path = "Catalog";
    entry = new KIsoFile(this,
                         path,
                         dirent->permissions() & ~S_IFDIR,
                         dirent->date(),
                         dirent->adate(),
                         dirent->cdate(),
                         dirent->user(),
                         dirent->group(),
                         QString(),
                         (long long)isonum_731(bootdesc->boot_catalog) << (long long)11,
                         (long long)2048);
    dirent->addEntry(entry);
    if (!ReadBootTable(&readf, isonum_731(bootdesc->boot_catalog), &boot, this)) {
        i = 1;
        be = boot.defentry;
        while (be) {
            size = BootImageSize(isonum_711(be->data.d_e.media), isonum_721(be->data.d_e.seccount));
            path = "Default Image";
            if (i > 1)
                path += " (" + QString::number(i) + ')';
            entry = new KIsoFile(this,
                                 path,
                                 dirent->permissions() & ~S_IFDIR,
                                 dirent->date(),
                                 dirent->adate(),
                                 dirent->cdate(),
                                 dirent->user(),
                                 dirent->group(),
                                 QString(),
                                 (long long)isonum_731(be->data.d_e.start) << (long long)11,
                                 size << (long long)9);
            dirent->addEntry(entry);
            be = be->next;
            i++;
        }

        FreeBootTable(&boot);
    }
}

void KIso::readParams()
{
    KRFUNC;
    KConfig *config;

    config = new KConfig("kio_isorc");

    KConfigGroup group(config, QString());
    showhidden = group.readEntry("showhidden", false);
    showrr = group.readEntry("showrr", true);
    delete config;
}

bool KIso::openArchive(QIODevice::OpenMode mode)
{
    KRFUNC;
    iso_vol_desc *desc;
    QString path, uid, gid;
    QT_STATBUF buf;
    int tracks[2 * 100], trackno = 0, i, access, c_b, c_i, c_j;
    KArchiveDirectory *root;
    struct iso_directory_record *idr;
    struct el_torito_boot_descriptor *bootdesc;

    if (mode == QIODevice::WriteOnly)
        return false;

    readParams();
    d->dirList.clear();

    tracks[0] = 0;
    if (m_startsec > 0)
        tracks[0] = m_startsec;
    // qDebug() << " m_startsec: " << m_startsec << endl;
    /* We'll use the permission and user/group of the 'host' file except
     * in Rock Ridge, where the permissions are stored on the file system
     */
    if (QT_STAT(m_filename.toLocal8Bit().data(), &buf) < 0) {
        /* defaults, if stat fails */
        memset(&buf, 0, sizeof(struct stat));
        buf.st_mode = 0777;
    } else {
        /* If it's a block device, try to query the track layout (for multisession) */
        if (m_startsec == -1 && S_ISBLK(buf.st_mode))
            trackno = getTracks(m_filename.toLatin1().data(), (int *)&tracks);
    }
    uid.setNum(buf.st_uid);
    gid.setNum(buf.st_gid);
    access = buf.st_mode & ~S_IFMT;

    root = new KIsoDirectory(this, QStringLiteral("/"), 0777 | S_IFDIR, buf.st_mtime, buf.st_atime, buf.st_ctime, uid, gid, QString());
    setRootDir(root);

    // qDebug() << "KIso::openArchive number of tracks: " << trackno << endl;

    if (trackno == 0)
        trackno = 1;
    for (i = 0; i < trackno; ++i) {
        c_b = 1;
        c_i = 1;
        c_j = 1;
        root = rootDir();
        if (trackno > 1) {
            path.clear();
            QTextStream(&path) << "Track " << tracks[(i << 1) + 1];
            root = new KIsoDirectory(this, path, access | S_IFDIR, buf.st_mtime, buf.st_atime, buf.st_ctime, uid, gid, QString());
            rootDir()->addEntry(root);
        }

        desc = ReadISO9660(&readf, tracks[i << 1], this);
        if (!desc) {
            // qDebug() << "KIso::openArchive no volume descriptors" << endl;
            continue;
        }

        while (desc) {
            switch (isonum_711(desc->data.type)) {
            case ISO_VD_BOOT:

                bootdesc = (struct el_torito_boot_descriptor *)&(desc->data);
                if (!memcmp(EL_TORITO_ID, bootdesc->system_id, ISODCL(8, 39))) {
                    path = "El Torito Boot";
                    if (c_b > 1)
                        path += " (" + QString::number(c_b) + ')';

                    dirent = new KIsoDirectory(this, path, access | S_IFDIR, buf.st_mtime, buf.st_atime, buf.st_ctime, uid, gid, QString());
                    root->addEntry(dirent);

                    addBoot(bootdesc);
                    c_b++;
                }
                break;

            case ISO_VD_PRIMARY:
            case ISO_VD_SUPPLEMENTARY:
                idr = (struct iso_directory_record *)&(((struct iso_primary_descriptor *)&desc->data)->root_directory_record);
                joliet = JolietLevel(&desc->data);
                if (joliet) {
                    QTextStream(&path) << "Joliet level " << joliet;
                    if (c_j > 1)
                        path += " (" + QString::number(c_j) + ')';
                } else {
                    path = "ISO9660";
                    if (c_i > 1)
                        path += " (" + QString::number(c_i) + ')';
                }
                dirent = new KIsoDirectory(this, path, access | S_IFDIR, buf.st_mtime, buf.st_atime, buf.st_ctime, uid, gid, QString());
                root->addEntry(dirent);
                level = 0;
                mycallb(idr, this);
                if (joliet)
                    c_j++;
                else
                    c_i++;
                break;
            }
            desc = desc->next;
        }
        free(desc);
    }
    device()->close();
    return true;
}

bool KIso::closeArchive()
{
    KRFUNC;
    d->dirList.clear();
    return true;
}

bool KIso::writeDir(const QString &, const QString &, const QString &, mode_t, time_t, time_t, time_t)
{
    return false;
}

bool KIso::prepareWriting(const QString &, const QString &, const QString &, qint64, mode_t, time_t, time_t, time_t)
{
    return false;
}

bool KIso::finishWriting(qint64)
{
    return false;
}

bool KIso::writeSymLink(const QString &, const QString &, const QString &, const QString &, mode_t, time_t, time_t, time_t)
{
    return false;
}

bool KIso::doWriteDir(const QString &, const QString &, const QString &, mode_t, const QDateTime &, const QDateTime &, const QDateTime &)
{
    return false;
}

bool KIso::doWriteSymLink(const QString &, const QString &, const QString &, const QString &, mode_t, const QDateTime &, const QDateTime &, const QDateTime &)
{
    return false;
}

bool KIso::doPrepareWriting(const QString &, const QString &, const QString &, qint64, mode_t, const QDateTime &, const QDateTime &, const QDateTime &)
{
    return false;
}

bool KIso::doFinishWriting(qint64)
{
    return false;
}

void KIso::virtual_hook(int id, void *data)
{
    KArchive::virtual_hook(id, data);
}
