/*****************************************************************************
 * Copyright (C) 2000 David Faure <faure@kde.org>                            *
 * Copyright (C) 2003 Leo Savernik <l.savernik@aon.at>                       *
 * Copyright (C) 2002 Szombathelyi György <gyurco@users.sourceforge.net>     *
 * This file is heavily based on ktar from kdelibs                           *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "kiso.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <QtCore/QDir>
#include <QtCore/QFile>

#include <kde_file.h>
#include <KDebug>
#include <KUrl>
#include <KMimeType>
#include <KConfig>
#include <KConfigGroup>
#include <KFilterBase>
#include <KFilterDev>

#include "libisofs/isofs.h"
#include "qfilehack.h"

#ifdef Q_OS_LINUX
#undef __STRICT_ANSI__
#include <linux/cdrom.h>
#define __STRICT_ANSI__
#include <sys/ioctl.h>
#include <fcntl.h>
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
    int ret = 0;
    memset(tracks, 0, 200*sizeof(int));

#ifdef Q_OS_LINUX
    int fd, i;
    struct cdrom_tochdr tochead;
    struct cdrom_tocentry tocentry;

    kDebug() << "getTracks open:" << fname << endl;
    fd = KDE_open(fname, O_RDONLY | O_NONBLOCK);
    if (fd > 0) {
        if (ioctl(fd, CDROMREADTOCHDR, &tochead) != -1) {
            kDebug() << "getTracks first track:" << tochead.cdth_trk0
            << " last track " << tochead.cdth_trk1 << endl;
            for (i = tochead.cdth_trk0;i <= tochead.cdth_trk1;i++) {
                if (ret > 99) break;
                memset(&tocentry, 0, sizeof(struct cdrom_tocentry));
                tocentry.cdte_track = i;
                tocentry.cdte_format = CDROM_LBA;
                if (ioctl(fd, CDROMREADTOCENTRY, &tocentry) < 0) break;
                kDebug() << "getTracks got track " << i << " starting at: " <<
                tocentry.cdte_addr.lba << endl;
                if ((tocentry.cdte_ctrl & 0x4) == 0x4) {
                    tracks[ret<<1] = tocentry.cdte_addr.lba;
                    tracks[(ret<<1)+1] = i;
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
    KIsoPrivate() {}
    QStringList dirList;
};

KIso::KIso(const QString& filename, const QString & _mimetype)
        : KArchive(0L)
{
    m_startsec = -1;
    m_filename = filename;
    d = new KIsoPrivate;
    QString mimetype(_mimetype);
    bool forced = true;
    if (mimetype.isEmpty()) {
        KSharedPtr<KMimeType> result = KMimeType::findByFileContent(filename);
        if (result)
            mimetype = result->name();

        kDebug() << "KIso::KIso mimetype=" << mimetype << endl;

        // Don't move to prepareDevice - the other constructor theoretically allows ANY filter
        if (mimetype == "application/x-tgz" || mimetype == "application/x-targz" ||  // the latter is deprecated but might still be around
                mimetype == "application/x-webarchive")
            // that's a gzipped tar file, so ask for gzip filter
            mimetype = "application/x-gzip";
        else if (mimetype == "application/x-tbz")   // that's a bzipped2 tar file, so ask for bz2 filter
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

void KIso::prepareDevice(const QString & filename,
                         const QString & mimetype, bool forced)
{
    /* 'hack' for Qt's false assumption that only S_ISREG is seekable */
    if ("inode/blockdevice" == mimetype)
        setDevice(new QFileHack(filename));
    else {
        if ("application/x-gzip" == mimetype
                || "application/x-bzip2" == mimetype)
            forced = true;

        QIODevice *dev = KFilterDev::deviceForFile(filename, mimetype, forced);
        if (dev)
            setDevice(dev);
    }

}

KIso::KIso(QIODevice * dev)
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

    QIODevice* dev = (static_cast<KIso*>(udata))->device();

    if (dev->seek((qint64)start << (qint64)11)) {
        if ((dev->read(buf, len << 11u)) != -1) return (len);
    }
    kDebug() << "KIso::ReadRequest failed start: " << start << " len: " << len << endl;

    return -1;
}

/* callback function for libisofs */
static int mycallb(struct iso_directory_record *idr, void *udata)
{

    KIso *iso = static_cast<KIso*>(udata);
    QString path, user, group, symlink;
    int i;
    int access;
    int time, cdate, adate;
    rr_entry rr;
    bool special = false;
    KArchiveEntry *entry = NULL, *oldentry = NULL;
    char z_algo[2], z_params[2];
    long long z_size = 0;

    if ((idr->flags[0] & 1) && !iso->showhidden) return 0;
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
            if (!special) path = rr.name;
            symlink = rr.sl;
            access = rr.mode;
            time = rr.t_mtime;
            adate = rr.t_atime;
            cdate = rr.t_ctime;
            user.setNum(rr.uid);
            group.setNum(rr.gid);
            z_algo[0] = rr.z_algo[0];z_algo[1] = rr.z_algo[1];
            z_params[0] = rr.z_params[0];z_params[1] = rr.z_params[1];
            z_size = rr.z_size;
        } else {
            access = iso->dirent->permissions() & ~S_IFMT;
            adate = cdate = time = isodate_915(idr->date, 0);
            user = iso->dirent->user();
            group = iso->dirent->group();
            if (idr->flags[0] & 2) access |= S_IFDIR; else access |= S_IFREG;
            if (!special) {
                if (iso->joliet) {
                    for (i = 0;i < (isonum_711(idr->name_len) - 1);i += 2) {
                        QChar ch(be2me_16(*((ushort*)&(idr->name[i]))));
                        if (ch == ';') break;
                        path += ch;
                    }
                } else {
                    for (i = 0;i < isonum_711(idr->name_len);i++) {
                        if (idr->name[i] == ';') break;
                        if (idr->name[i]) path += (idr->name[i]);
                    }
                }
                if (path.endsWith('.')) path.resize(path.length() - 1);
            }
        }
        if (iso->showrr) FreeRR(&rr);
        if (idr->flags[0] & 2) {
            entry = new KIsoDirectory(iso, path, access | S_IFDIR, time, adate, cdate,
                                      user, group, symlink);
        } else {
            entry = new KIsoFile(iso, path, access, time, adate, cdate,
                                 user, group, symlink, (long long)(isonum_733(idr->extent)) << (long long)11, isonum_733(idr->size));
            if (z_size)(static_cast <KIsoFile*>(entry))->setZF(z_algo, z_params, z_size);

        }
        iso->dirent->addEntry(entry);
    }
    if ((idr->flags[0] & 2) && (iso->level == 0 || !special)) {
        if (iso->level) {
            oldentry = iso->dirent;
            iso->dirent = static_cast<KIsoDirectory*>(entry);
        }
        iso->level++;
        ProcessDir(&readf, isonum_733(idr->extent), isonum_733(idr->size), &mycallb, udata);
        iso->level--;
        if (iso->level) iso->dirent = static_cast<KIsoDirectory*>(oldentry);
    }
    return 0;
}

void KIso::addBoot(struct el_torito_boot_descriptor* bootdesc)
{

    int i;
    long long size;
    boot_head boot;
    boot_entry *be;
    QString path;
    KIsoFile *entry;

    entry = new KIsoFile(this, "Catalog", dirent->permissions() & ~S_IFDIR,
                         dirent->date(), dirent->adate(), dirent->cdate(),
                         dirent->user(), dirent->group(), QString(),
                         (long long)isonum_731(bootdesc->boot_catalog) << (long long)11, 2048);
    dirent->addEntry(entry);
    if (!ReadBootTable(&readf, isonum_731(bootdesc->boot_catalog), &boot, this)) {
        i = 1;
        be = boot.defentry;
        while (be) {
            size = BootImageSize(isonum_711(((struct default_entry*) be->data)->media),
                                 isonum_721(((struct default_entry*) be->data)->seccount));
            path = "Default Image";
            if (i > 1) path += " (" + QString::number(i) + ')';
            entry = new KIsoFile(this, path, dirent->permissions() & ~S_IFDIR,
                                 dirent->date(), dirent->adate(), dirent->cdate(),
                                 dirent->user(), dirent->group(), QString(),
                                 (long long)isonum_731(((struct default_entry*) be->data)->start) << (long long)11, size << (long long)9);
            dirent->addEntry(entry);
            be = be->next;
            i++;
        }

        FreeBootTable(&boot);
    }
}

void KIso::readParams()
{
    KConfig *config;

    config = new KConfig("kio_isorc");

    KConfigGroup group(config, QString());
    showhidden = group.readEntry("showhidden", false);
    showrr = group.readEntry("showrr", true);
    delete config;
}

bool KIso::openArchive(QIODevice::OpenMode mode)
{
    iso_vol_desc *desc;
    QString path, tmp, uid, gid;
    KDE_struct_stat buf;
    int tracks[2*100], trackno = 0, i, access, c_b, c_i, c_j;
    KArchiveDirectory *root;
    struct iso_directory_record* idr;
    struct el_torito_boot_descriptor* bootdesc;

    if (mode == QIODevice::WriteOnly)
        return false;

    readParams();
    d->dirList.clear();

    tracks[0] = 0;
    if (m_startsec > 0) tracks[0] = m_startsec;
    kDebug() << " m_startsec: " << m_startsec << endl;
    /* We'll use the permission and user/group of the 'host' file except
     * in Rock Ridge, where the permissions are stored on the file system
     */
    if (KDE_stat(m_filename.toLocal8Bit(), &buf) < 0) {
        /* defaults, if stat fails */
        memset(&buf, 0, sizeof(struct stat));
        buf.st_mode = 0777;
    } else {
        /* If it's a block device, try to query the track layout (for multisession) */
        if (m_startsec == -1 && S_ISBLK(buf.st_mode))
            trackno = getTracks(m_filename.toLatin1(), (int*) & tracks);
    }
    uid.setNum(buf.st_uid);
    gid.setNum(buf.st_gid);
    access = buf.st_mode & ~S_IFMT;

    kDebug() << "KIso::openArchive number of tracks: " << trackno << endl;

    if (trackno == 0) trackno = 1;
    for (i = 0;i < trackno;i++) {

        c_b = 1;c_i = 1;c_j = 1;
        root = rootDir();
        if (trackno > 1) {
            path.clear();
            QTextStream(&path) << "Track " << tracks[(i<<1)+1];
            root = new KIsoDirectory(this, path, access | S_IFDIR,
                                     buf.st_mtime, buf.st_atime, buf.st_ctime, uid, gid, QString());
            rootDir()->addEntry(root);
        }

        desc = ReadISO9660(&readf, tracks[i<<1], this);
        if (!desc) {
            kDebug() << "KIso::openArchive no volume descriptors" << endl;
            continue;
        }

        while (desc) {
            switch (isonum_711(desc->data.type)) {
            case ISO_VD_BOOT:

                bootdesc = (struct el_torito_boot_descriptor*) & (desc->data);
                if (!memcmp(EL_TORITO_ID, bootdesc->system_id, ISODCL(8, 39))) {
                    path = "El Torito Boot";
                    if (c_b > 1) path += " (" + QString::number(c_b) + ')';

                    dirent = new KIsoDirectory(this, path, access | S_IFDIR,
                                               buf.st_mtime, buf.st_atime, buf.st_ctime, uid, gid, QString());
                    root->addEntry(dirent);

                    addBoot(bootdesc);
                    c_b++;
                }
                break;

            case ISO_VD_PRIMARY:
            case ISO_VD_SUPPLEMENTARY:
                idr = (struct iso_directory_record*) & (((struct iso_primary_descriptor*) & desc->data)->root_directory_record);
                joliet = JolietLevel(&desc->data);
                if (joliet) {
                    QTextStream(&path) << "Joliet level " << joliet;
                    if (c_j > 1) path += " (" + QString::number(c_j) + ')';
                } else {
                    path = "ISO9660";
                    if (c_i > 1) path += " (" + QString::number(c_i) + ')';
                }
                dirent = new KIsoDirectory(this, path, access | S_IFDIR,
                                           buf.st_mtime, buf.st_atime, buf.st_ctime, uid, gid, QString());
                root->addEntry(dirent);
                level = 0;
                mycallb(idr, this);
                if (joliet) c_j++; else c_i++;
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
    d->dirList.clear();
    return true;
}

bool KIso::writeDir(const QString&, const QString&, const QString&, mode_t, time_t, time_t, time_t)
{
    return false;
}

bool KIso::prepareWriting(const QString&, const QString&, const QString&, qint64, mode_t, time_t, time_t, time_t)
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

bool KIso::doWriteDir(const QString&, const QString&, const QString&, mode_t, time_t, time_t, time_t)
{
    return false;
}

bool KIso::doWriteSymLink(const QString &, const QString &, const QString &, const QString &, mode_t, time_t, time_t, time_t)
{
    return false;
}

bool KIso::doPrepareWriting(const QString& , const QString& , const QString& , qint64, mode_t, time_t, time_t, time_t)
{
    return false;
}

bool KIso::doFinishWriting(qint64)
{
    return false;
}

void KIso::virtual_hook(int id, void* data)
{
    KArchive::virtual_hook(id, data);
}

