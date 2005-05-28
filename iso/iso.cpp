/***************************************************************************
                                iso.cpp
                             -------------------
    begin                : Oct 25 2002
    copyright            : (C) 2002 by Szombathelyi Gy�gy
    email                : gyurco@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

 /* This file is heavily based on tar.cc from kdebase
  * (c) David Faure <faure@kde.org>
  */

#include <zlib.h>
  
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include <qfile.h>
#include <kurl.h>
#include <kdebug.h>
#include <kinstance.h>
#include <kiso.h>
#include <kmimemagic.h>

#include <errno.h> // to be removed

#include "libisofs/iso_fs.h"

#include "kisofile.h"
#include "kisodirectory.h"
#include "iso.h"

typedef struct {
    char magic[8];
    char uncompressed_len[4];
    unsigned char header_size;
    unsigned char block_size;
    char reserved[2];   /* Reserved for future use, MBZ */
} compressed_file_header;

static const unsigned char zisofs_magic[8] = {
    0x37, 0xE4, 0x53, 0x96, 0xC9, 0xDB, 0xD6, 0x07
};

using namespace KIO;

extern "C" { int kdemain(int argc, char **argv); }

int kdemain( int argc, char **argv )
{
  KInstance instance( "kio_iso" );

  kdDebug()   << "Starting " << getpid() << endl;

  if (argc != 4)
  {
     fprintf(stderr, "Usage: kio_iso protocol domain-socket1 domain-socket2\n");
     exit(-1);
  }

  kio_isoProtocol slave(argv[2], argv[3]);
  slave.dispatchLoop();

  kdDebug()   << "Done" << endl;
  return 0;
}


kio_isoProtocol::kio_isoProtocol( const QCString &pool, const QCString &app ) : SlaveBase( "iso", pool, app )
{
  kdDebug() << "kio_isoProtocol::kio_isoProtocol" << endl;
  m_isoFile = 0L;
}

kio_isoProtocol::~kio_isoProtocol()
{
    delete m_isoFile;
}

bool kio_isoProtocol::checkNewFile( QString fullPath, QString & path, int startsec )
{
    kdDebug()   << "kio_isoProtocol::checkNewFile " << fullPath << " startsec: " <<
        startsec << endl;

    // Are we already looking at that file ?
    if ( m_isoFile && startsec == m_isoFile->startSec() &&
        m_isoFile->fileName() == fullPath.left(m_isoFile->fileName().length()) )
    {
        // Has it changed ?
        struct stat statbuf;
        if ( ::stat( QFile::encodeName( m_isoFile->fileName() ), &statbuf ) == 0 )
        {
            if ( m_mtime == statbuf.st_mtime )
            {
                path = fullPath.mid( m_isoFile->fileName().length() );
                kdDebug()   << "kio_isoProtocol::checkNewFile returning " << path << endl;
                return true;
            }
        }
    }
    kdDebug() << "Need to open a new file" << endl;

    // Close previous file
    if ( m_isoFile )
    {
        m_isoFile->close();
        delete m_isoFile;
        m_isoFile = 0L;
    }

    // Find where the iso file is in the full path
    int pos = 0;
    QString isoFile;
    path = QString::null;

    int len = fullPath.length();
    if ( len != 0 && fullPath[ len - 1 ] != '/' )
        fullPath += '/';

    kdDebug()   << "the full path is " << fullPath << endl;
    while ( (pos=fullPath.find( '/', pos+1 )) != -1 )
    {
        QString tryPath = fullPath.left( pos );
        kdDebug()   << fullPath << "  trying " << tryPath << endl;
        struct stat statbuf;
        if ( ::stat( QFile::encodeName(tryPath), &statbuf ) == 0 && !S_ISDIR(statbuf.st_mode) )
        {
            isoFile = tryPath;
            m_mtime = statbuf.st_mtime;
            m_mode = statbuf.st_mode;
            path = fullPath.mid( pos + 1 );
            kdDebug()   << "fullPath=" << fullPath << " path=" << path << endl;
            len = path.length();
            if ( len > 1 )
            {
                if ( path[ len - 1 ] == '/' )
                    path.truncate( len - 1 );
            }
            else
                path = QString::fromLatin1("/");
            kdDebug()   << "Found. isoFile=" << isoFile << " path=" << path << endl;
            break;
        }
    }
    if ( isoFile.isEmpty() )
    {
        kdDebug()   << "kio_isoProtocol::checkNewFile: not found" << endl;
        return false;
    }

    // Open new file
    kdDebug() << "Opening KIso on " << isoFile << endl;
    m_isoFile = new KIso( isoFile );
    m_isoFile->setStartSec(startsec);
    if ( !m_isoFile->open( IO_ReadOnly ) )
    {
        kdDebug()   << "Opening " << isoFile << " failed." << endl;
        delete m_isoFile;
        m_isoFile = 0L;
        return false;
    }

    return true;
}


void kio_isoProtocol::createUDSEntry( const KArchiveEntry * isoEntry, UDSEntry & entry )
{
    UDSAtom atom;

    entry.clear();
    atom.m_uds = UDS_NAME;
    atom.m_str = isoEntry->name();
    entry.append(atom);

    atom.m_uds = UDS_FILE_TYPE;
    atom.m_long = isoEntry->permissions() & S_IFMT; // keep file type only
    entry.append( atom );

    atom.m_uds = UDS_ACCESS;
    atom.m_long = isoEntry->permissions() & 07777; // keep permissions only
    entry.append( atom );

    atom.m_uds = UDS_SIZE;
    if (isoEntry->isFile()) {
        atom.m_long = ((KIsoFile *)isoEntry)->realsize();
        if (!atom.m_long) atom.m_long = ((KIsoFile *)isoEntry)->size();
    } else {
        atom.m_long = 0L;
    }
    entry.append( atom );

    atom.m_uds = UDS_USER;
    atom.m_str = isoEntry->user();
    entry.append( atom );

    atom.m_uds = UDS_GROUP;
    atom.m_str = isoEntry->group();
    entry.append( atom );

    atom.m_uds = UDS_MODIFICATION_TIME;
    atom.m_long = isoEntry->date();
    entry.append( atom );

    atom.m_uds = UDS_ACCESS_TIME;
    atom.m_long = isoEntry->isFile() ? ((KIsoFile *)isoEntry)->adate() :
                                       ((KIsoDirectory *)isoEntry)->adate();
    entry.append( atom );

    atom.m_uds = UDS_CREATION_TIME;
    atom.m_long = isoEntry->isFile() ? ((KIsoFile *)isoEntry)->cdate() :
                                       ((KIsoDirectory *)isoEntry)->cdate();
    entry.append( atom );

    atom.m_uds = UDS_LINK_DEST;
    atom.m_str = isoEntry->symlink();
    entry.append( atom );
}

void kio_isoProtocol::listDir( const KURL & url )
{
    kdDebug() << "kio_isoProtocol::listDir " << url.url() << endl;

    QString path;
    if ( !checkNewFile( url.path(), path, url.hasRef() ? url.htmlRef().toInt() : -1 ) )
    {
        QCString _path( QFile::encodeName(url.path()));
        kdDebug()  << "Checking (stat) on " << _path << endl;
        struct stat buff;
        if ( ::stat( _path.data(), &buff ) == -1 || !S_ISDIR( buff.st_mode ) ) {
            error( KIO::ERR_DOES_NOT_EXIST, url.path() );
            return;
        }
        // It's a real dir -> redirect
        KURL redir;
        redir.setPath( url.path() );
        if (url.hasRef()) redir.setRef(url.htmlRef());
        kdDebug()  << "Ok, redirection to " << redir.url() << endl;
        redirection( redir );
        finished();
        // And let go of the iso file - for people who want to unmount a cdrom after that
        delete m_isoFile;
        m_isoFile = 0L;
        return;
    }

    if ( path.isEmpty() )
    {
        KURL redir( QString::fromLatin1( "iso:/") );
        kdDebug() << "url.path()==" << url.path() << endl;
        if (url.hasRef()) redir.setRef(url.htmlRef());
        redir.setPath( url.path() + QString::fromLatin1("/") );
        kdDebug() << "kio_isoProtocol::listDir: redirection " << redir.url() << endl;
        redirection( redir );
        finished();
        return;
    }

    kdDebug()  << "checkNewFile done" << endl;
    const KArchiveDirectory* root = m_isoFile->directory();
    const KArchiveDirectory* dir;
    if (!path.isEmpty() && path != "/")
    {
        kdDebug()   << QString("Looking for entry %1").arg(path) << endl;
        const KArchiveEntry* e = root->entry( path );
        if ( !e )
        {
            error( KIO::ERR_DOES_NOT_EXIST, path );
            return;
        }
        if ( ! e->isDirectory() )
        {
            error( KIO::ERR_IS_FILE, path );
            return;
        }
        dir = (KArchiveDirectory*)e;
    } else {
        dir = root;
    }

    QStringList l = dir->entries();
    totalSize( l.count() );

    UDSEntry entry;
    QStringList::Iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
        kdDebug()   << (*it) << endl;
        const KArchiveEntry* isoEntry = dir->entry( (*it) );

        createUDSEntry( isoEntry, entry );

        listEntry( entry, false );
    }

    listEntry( entry, true ); // ready

    finished();

    kdDebug()  << "kio_isoProtocol::listDir done" << endl;
}

void kio_isoProtocol::stat( const KURL & url )
{
    QString path;
    UDSEntry entry;

    kdDebug() << "kio_isoProtocol::stat " << url.url() << endl;
    if ( !checkNewFile( url.path(), path, url.hasRef() ? url.htmlRef().toInt() : -1 ) )
    {
        // We may be looking at a real directory - this happens
        // when pressing up after being in the root of an archive
        QCString _path( QFile::encodeName(url.path()));
        kdDebug()  << "kio_isoProtocol::stat (stat) on " << _path << endl;
        struct stat buff;
        if ( ::stat( _path.data(), &buff ) == -1 || !S_ISDIR( buff.st_mode ) ) {
            kdDebug() << "isdir=" << S_ISDIR( buff.st_mode ) << "  errno=" << strerror(errno) << endl;
            error( KIO::ERR_DOES_NOT_EXIST, url.path() );
            return;
        }
        // Real directory. Return just enough information for KRun to work
        UDSAtom atom;
        atom.m_uds = KIO::UDS_NAME;
        atom.m_str = url.fileName();
        entry.append( atom );
        kdDebug()  << "kio_isoProtocol::stat returning name=" << url.fileName() << endl;

        atom.m_uds = KIO::UDS_FILE_TYPE;
        atom.m_long = buff.st_mode & S_IFMT;
        entry.append( atom );

        statEntry( entry );

        finished();

        // And let go of the iso file - for people who want to unmount a cdrom after that
        delete m_isoFile;
        m_isoFile = 0L;
        return;
    }

    const KArchiveDirectory* root = m_isoFile->directory();
    const KArchiveEntry* isoEntry;
    if ( path.isEmpty() )
    {
        path = QString::fromLatin1( "/" );
        isoEntry = root;
    } else {
        isoEntry = root->entry( path );
    }
    if ( !isoEntry )
    {
        error( KIO::ERR_DOES_NOT_EXIST, path );
        return;
    }
    createUDSEntry( isoEntry, entry );
    statEntry( entry );
    finished();
}

void kio_isoProtocol::getFile( const KIsoFile *isoFileEntry, const QString &path )
{
    unsigned long long size, pos = 0;
    bool mime=false,zlib=false;
    QByteArray fileData, pointer_block, inbuf, outbuf;
    char *pptr = 0;
    compressed_file_header *hdr;
    int block_shift;
    unsigned long nblocks;
    unsigned long fullsize = 0, block_size = 0, block_size2 = 0;
    size_t ptrblock_bytes;
    unsigned long cstart, cend, csize;
    uLong bytes;

    size = isoFileEntry->realsize();
    if (size >= sizeof(sizeof(compressed_file_header))) zlib=true;
    if (!size) size = isoFileEntry->size();
    totalSize( size );
    if (!size) mimeType("application/x-zerosize");

    if (size && !m_isoFile->device()->isOpen()) m_isoFile->device()->open(IO_ReadOnly);

    if (zlib) {
        fileData=isoFileEntry->data(0, sizeof(compressed_file_header));
        if ( fileData.size() == sizeof(compressed_file_header) &&
            !memcmp(fileData.data(), zisofs_magic, sizeof (zisofs_magic)) ) {

            hdr=(compressed_file_header*) fileData.data();
            block_shift = hdr->block_size;
            block_size  = 1UL << block_shift;
            block_size2 = block_size << 1;
            fullsize    = isonum_731(hdr->uncompressed_len);
            nblocks = (fullsize + block_size - 1) >> block_shift;
            ptrblock_bytes = (nblocks+1) * 4;
            pointer_block=isoFileEntry->data( hdr->header_size << 2, ptrblock_bytes );
            if (pointer_block.size() == ptrblock_bytes &&
                inbuf.resize(block_size2) &&
                outbuf.resize(block_size)) {
                    
                pptr = pointer_block.data();
            } else {
                error(KIO::ERR_COULD_NOT_READ, path);
                return;
            }
        } else {
            zlib=false;
        }
    }

    while (pos<size) {
        if (zlib) {
            cstart = isonum_731(pptr);
            pptr += 4;
            cend   = isonum_731(pptr);

            csize = cend-cstart;

            if ( csize == 0 ) {
                outbuf.fill(0, -1);
            } else {
                if ( csize > block_size2 ) {
                    //err = EX_DATAERR;
                    break;
                }

                inbuf=isoFileEntry->data(cstart, csize);
                if (inbuf.size() != csize) {
                    break;
                }

                bytes = block_size; // Max output buffer size
                if ( (uncompress((Bytef*) outbuf.data(), &bytes, (Bytef*) inbuf.data(), csize)) != Z_OK ) {
                    break;
                }
            }

            if ( ((fullsize > block_size) && (bytes != block_size))
                || ((fullsize <= block_size) && (bytes < fullsize)) ) {

                break;
            }

            if ( bytes > fullsize )
                bytes = fullsize;
            fileData.assign(outbuf);
            fileData.resize(bytes);
            fullsize -= bytes;
        } else {
            fileData=isoFileEntry->data(pos,65536);
            if (fileData.size()==0) break;
        }
        if (!mime) {
            KMimeMagicResult * result = KMimeMagic::self()->findBufferFileType( fileData, path );
            kdDebug() << "Emitting mimetype " << result->mimeType() << endl;
            mimeType( result->mimeType() );
            mime=true;
        }
        data(fileData);
        pos+=fileData.size();
        processedSize(pos);
    }

    if (pos!=size) {
        error(KIO::ERR_COULD_NOT_READ, path);
        return;
    }

    fileData.resize(0);
    data(fileData);
    processedSize(pos);
    finished();

}

void kio_isoProtocol::get( const KURL & url )
{
    kdDebug()  << "kio_isoProtocol::get" << url.url() << endl;

    QString path;
    if ( !checkNewFile( url.path(), path, url.hasRef() ? url.htmlRef().toInt() : -1 ) )
    {
        error( KIO::ERR_DOES_NOT_EXIST, url.path() );
        return;
    }

    const KArchiveDirectory* root = m_isoFile->directory();
    const KArchiveEntry* isoEntry = root->entry( path );

    if ( !isoEntry )
    {
        error( KIO::ERR_DOES_NOT_EXIST, path );
        return;
    }
    if ( isoEntry->isDirectory() )
    {
        error( KIO::ERR_IS_DIRECTORY, path );
        return;
    }

    const KIsoFile* isoFileEntry = static_cast<const KIsoFile *>(isoEntry);
    if ( !isoEntry->symlink().isEmpty() )
    {
      kdDebug() << "Redirection to " << isoEntry->symlink() << endl;
      KURL realURL( url, isoEntry->symlink() );
      kdDebug() << "realURL= " << realURL.url() << endl;
      redirection( realURL.url() );
      finished();
      return;
    }
    getFile(isoFileEntry, path);
    if (m_isoFile->device()->isOpen()) m_isoFile->device()->close();
}
