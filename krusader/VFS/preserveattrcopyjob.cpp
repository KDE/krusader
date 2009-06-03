// Krusader modifications:
//
// Replace: CopyJob -> PreserveAttrCopyJob
// Replace: copyjob -> preserveattrcopyjob
// Compile fixes

/* This file is part of the KDE libraries
    Copyright 2000       Stephan Kulow <coolo@kde.org>
    Copyright 2000-2006  David Faure <faure@kde.org>
    Copyright 2000       Waldo Bastian <bastian@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "preserveattrcopyjob.h"
#include <kio/deletejob.h>

#include <klocale.h>
#include <kdesktopfile.h>
#include <kdebug.h>
#include <kde_file.h>

#include <kio/slave.h>
#include <kio/scheduler.h>
#include <kdirwatch.h>
#include <kprotocolmanager.h>

#include <kio/jobuidelegate.h>

#include <kdirnotify.h>
#include <ktemporaryfile.h>
#include <kuiserverjobtracker.h>

#if defined(Q_OS_UNIX) || defined(Q_WS_WIN)
#include <utime.h>
#endif
#include <assert.h>

#include <QtCore/QTimer>
#include <QtCore/QFile>
#include <sys/stat.h> // mode_t
#include <sys/types.h>
#include <QPointer>

#include <pwd.h>
#include <grp.h>

Attributes::Attributes() 
{
  time = (time_t)-1;
  uid = (uid_t)-1;
  gid = (gid_t)-1;
  mode = (mode_t)-1;
  acl = QString();
}

Attributes::Attributes( time_t tIn, uid_t uIn, gid_t gIn, mode_t modeIn, const QString & aclIn ) 
{
  time = tIn, uid = uIn, gid = gIn, mode = modeIn, acl = aclIn;
}

Attributes::Attributes( time_t tIn, QString user, QString group, mode_t modeIn, const QString & aclIn ) 
{
  time = tIn;
  uid = (uid_t)-1;
  struct passwd* pw = getpwnam(QFile::encodeName( user ));
  if ( pw != 0L )
    uid = pw->pw_uid;
  gid = (gid_t)-1;
  struct group* g = getgrnam(QFile::encodeName( group ));
  if ( g != 0L )
    gid = g->gr_gid;
  mode = modeIn;
  acl = aclIn;
}

using namespace KIO;

//this will update the report dialog with 5 Hz, I think this is fast enough, aleXXX
#define REPORT_TIMEOUT 200

#define KIO_ARGS QByteArray packedArgs; QDataStream stream( &packedArgs, QIODevice::WriteOnly ); stream

PreserveAttrCopyJob::PreserveAttrCopyJob(const KUrl::List& src, const KUrl& dest,
                   CopyJob::CopyMode mode, bool asMethod)
    : Job(), m_globalDest(dest)
        , m_globalDestinationState(DEST_NOT_STATED)
        , m_defaultPermissions(false)
        , m_bURLDirty(false)
        , m_mode(mode)
        , m_asMethod(asMethod)
        , destinationState(DEST_NOT_STATED)
        , state(STATE_STATING)
        , m_totalSize(0)
        , m_processedSize(0)
        , m_fileProcessedSize(0)
        , m_processedFiles(0)
        , m_processedDirs(0)
        , m_srcList(src)
        , m_currentStatSrc(m_srcList.begin())
        , m_bCurrentOperationIsLink(false)
        , m_bSingleFileCopy(false)
        , m_bOnlyRenames(mode==CopyJob::Move)
        , m_dest(dest)
        , m_bAutoSkip( false )
        , m_bOverwriteAll( false )
        , m_conflictError(0)
        , m_reportTimer(0)

{
    QTimer::singleShot(0, this, SLOT(slotStart()));
}

PreserveAttrCopyJob::~PreserveAttrCopyJob()
{
}

KUrl::List PreserveAttrCopyJob::srcUrls() const
{
    return m_srcList;
}

KUrl PreserveAttrCopyJob::destUrl() const
{
    return m_dest;
}

void PreserveAttrCopyJob::slotStart()
{
    /**
       We call the functions directly instead of using signals.
       Calling a function via a signal takes approx. 65 times the time
       compared to calling it directly (at least on my machine). aleXXX
    */
    m_reportTimer = new QTimer(this);

    connect(m_reportTimer,SIGNAL(timeout()),this,SLOT(slotReport()));
    m_reportTimer->start(REPORT_TIMEOUT);
    connect( this, SIGNAL( result( KJob * ) ), this, SLOT( slotFinished() ) );

    // Stat the dest
    KIO::Job * job = KIO::stat( m_dest, StatJob::DestinationSide, 2, KIO::HideProgressInfo );
    //kDebug(7007) << "PreserveAttrCopyJob:stating the dest " << m_dest;
    addSubjob(job);
}

// For unit test purposes
static bool kio_resolve_local_urls = true;

void PreserveAttrCopyJob::slotResultStating( KJob *job )
{
    //kDebug(7007) << "PreserveAttrCopyJob::slotResultStating";
    // Was there an error while stating the src ?
    if (job->error() && destinationState != DEST_NOT_STATED )
    {
        KUrl srcurl = ((SimpleJob*)job)->url();
        if ( !srcurl.isLocalFile() )
        {
            // Probably : src doesn't exist. Well, over some protocols (e.g. FTP)
            // this info isn't really reliable (thanks to MS FTP servers).
            // We'll assume a file, and try to download anyway.
            //kDebug(7007) << "Error while stating source. Activating hack";
            removeSubjob( job );
            assert ( !hasSubjobs() ); // We should have only one job at a time ...
            struct CopyInfo info;
            info.permissions = (mode_t) -1;
            info.mtime = (time_t) -1;
            info.ctime = (time_t) -1;
            info.size = (KIO::filesize_t)-1;
            info.uSource = srcurl;
            info.uDest = m_dest;
            // Append filename or dirname to destination URL, if allowed
            if ( destinationState == DEST_IS_DIR && !m_asMethod )
                info.uDest.addPath( srcurl.fileName() );

            files.append( info );
            statNextSrc();
            return;
        }
        // Local file. If stat fails, the file definitely doesn't exist.
        // yes, Job::, because we don't want to call our override
        Job::slotResult( job ); // will set the error and emit result(this)
        return;
    }

    // Keep copy of the stat result
    const UDSEntry entry = static_cast<StatJob*>(job)->statResult();
    const QString sLocalPath = entry.stringValue( KIO::UDSEntry::UDS_LOCAL_PATH );
    const bool isDir = entry.isDir();

    if ( destinationState == DEST_NOT_STATED )
        // we were stating the dest
    {
        if (job->error())
            destinationState = DEST_DOESNT_EXIST;
        else {
            // Treat symlinks to dirs as dirs here, so no test on isLink
            destinationState = isDir ? DEST_IS_DIR : DEST_IS_FILE;
            //kDebug(7007) << "PreserveAttrCopyJob::slotResultStating dest is dir:" << bDir;
        }
        const bool isGlobalDest = m_dest == m_globalDest;
        if ( isGlobalDest )
            m_globalDestinationState = destinationState;

        if ( !sLocalPath.isEmpty() && kio_resolve_local_urls ) {
            m_dest = KUrl();
            m_dest.setPath(sLocalPath);
            if ( isGlobalDest )
                m_globalDest = m_dest;
        }

        removeSubjob( job );
        assert ( !hasSubjobs() );

        // After knowing what the dest is, we can start stat'ing the first src.
        statCurrentSrc();
        return;
    }

    // Is it a file or a dir ?
    const QString sName = entry.stringValue( KIO::UDSEntry::UDS_NAME );

    // We were stating the current source URL
    m_currentDest = m_dest; // used by slotEntries
    // Create a dummy list with it, for slotEntries
    UDSEntryList lst;
    lst.append(entry);

    // There 6 cases, and all end up calling slotEntries(job, lst) first :
    // 1 - src is a dir, destination is a directory,
    // slotEntries will append the source-dir-name to the destination
    // 2 - src is a dir, destination is a file, ERROR (done later on)
    // 3 - src is a dir, destination doesn't exist, then it's the destination dirname,
    // so slotEntries will use it as destination.

    // 4 - src is a file, destination is a directory,
    // slotEntries will append the filename to the destination.
    // 5 - src is a file, destination is a file, m_dest is the exact destination name
    // 6 - src is a file, destination doesn't exist, m_dest is the exact destination name
    // Tell slotEntries not to alter the src url
    m_bCurrentSrcIsDir = false;
    slotEntries(static_cast<KIO::Job*>( job ), lst);

    KUrl srcurl;
    if (!sLocalPath.isEmpty())
        srcurl.setPath(sLocalPath);
    else
        srcurl = ((SimpleJob*)job)->url();

    removeSubjob( job );
    assert ( !hasSubjobs() ); // We should have only one job at a time ...

    if ( isDir
         // treat symlinks as files (no recursion)
         && !entry.isLink()
         && m_mode != CopyJob::Link ) // No recursion in Link mode either.
    {
        //kDebug(7007) << " Source is a directory ";

        m_bCurrentSrcIsDir = true; // used by slotEntries
        if ( destinationState == DEST_IS_DIR ) // (case 1)
        {
            if ( !m_asMethod )
            {
                // Use <desturl>/<directory_copied> as destination, from now on
                QString directory = srcurl.fileName();
                if ( !sName.isEmpty() && KProtocolManager::fileNameUsedForCopying( srcurl ) == KProtocolInfo::Name )
                {
                    directory = sName;
                }
                m_currentDest.addPath( directory );
            }
        }
        else if ( destinationState == DEST_IS_FILE ) // (case 2)
        {
            setError( ERR_IS_FILE );
            setErrorText( m_dest.prettyUrl() );
            emitResult();
            return;
        }
        else // (case 3)
        {
            // otherwise dest is new name for toplevel dir
            // so the destination exists, in fact, from now on.
            // (This even works with other src urls in the list, since the
            //  dir has effectively been created)
            destinationState = DEST_IS_DIR;
            if ( m_dest == m_globalDest )
                m_globalDestinationState = destinationState;
        }

        startListing( srcurl );
    }
    else
    {
        //kDebug(7007) << " Source is a file (or a symlink), or we are linking -> no recursive listing ";
        statNextSrc();
    }
}

bool PreserveAttrCopyJob::doSuspend()
{
    slotReport();
    return Job::doSuspend();
}

void PreserveAttrCopyJob::slotReport()
{
    if ( isSuspended() )
        return;
    // If showProgressInfo was set, progressId() is > 0.
    switch (state) {
        case STATE_COPYING_FILES:
            setProcessedAmount( KJob::Files, m_processedFiles );
            if (m_bURLDirty)
            {
                // Only emit urls when they changed. This saves time, and fixes #66281
                m_bURLDirty = false;
                if (m_mode==CopyJob::Move)
                {
                    emit description(this, i18nc("@title job","Moving"),
                          qMakePair(i18n("Source"), m_currentSrcURL.prettyUrl()),
                          qMakePair(i18n("Destination"), m_currentDestURL.prettyUrl()));
                    emit moving( this, m_currentSrcURL, m_currentDestURL);
                }
                else if (m_mode==CopyJob::Link)
                {
                    emit description(this, i18nc("@title job","Copying"),
                          qMakePair(i18n("Source"), m_currentSrcURL.prettyUrl()),
                          qMakePair(i18n("Destination"), m_currentDestURL.prettyUrl()));
                    emit linking( this, m_currentSrcURL.path(), m_currentDestURL );
                }
                else
                {
                    emit description(this, i18nc("@title job","Copying"),
                          qMakePair(i18n("Source"), m_currentSrcURL.prettyUrl()),
                          qMakePair(i18n("Destination"), m_currentDestURL.prettyUrl()));
                    emit copying( this, m_currentSrcURL, m_currentDestURL );
                }
            }
            break;

        case STATE_CREATING_DIRS:
            setProcessedAmount( KJob::Directories, m_processedDirs );
            if (m_bURLDirty)
            {
                m_bURLDirty = false;
                emit description(this, i18nc("@title job","Creating directory"),
                          qMakePair(i18n("Directory"), m_currentDestURL.prettyUrl()));
                emit creatingDir( this, m_currentDestURL );
            }
            break;

        case STATE_STATING:
        case STATE_LISTING:
            if (m_bURLDirty)
            {
                m_bURLDirty = false;
                emit description(this, i18nc("@title job","Copying"),
                          qMakePair(i18n("Source"), m_currentSrcURL.prettyUrl()),
                          qMakePair(i18n("Destination"), m_currentDestURL.prettyUrl()));
            }
            setTotalAmount(KJob::Bytes, m_totalSize);
            setTotalAmount(KJob::Files, files.count());
            setTotalAmount(KJob::Directories, dirs.count());
            break;

        default:
            break;
    }
}

void PreserveAttrCopyJob::slotEntries(KIO::Job* job, const UDSEntryList& list)
{
    saveEntries( job, list );

    UDSEntryList::ConstIterator it = list.begin();
    UDSEntryList::ConstIterator end = list.end();
    for (; it != end; ++it) {
        const UDSEntry& entry = *it;
        struct CopyInfo info;
        info.permissions = entry.numberValue( KIO::UDSEntry::UDS_ACCESS, -1 );
        info.mtime = (time_t) entry.numberValue( KIO::UDSEntry::UDS_MODIFICATION_TIME, -1 );
        info.ctime = (time_t) entry.numberValue( KIO::UDSEntry::UDS_CREATION_TIME, -1 );
        info.size = (KIO::filesize_t) entry.numberValue( KIO::UDSEntry::UDS_SIZE, -1 );
        if ( info.size != (KIO::filesize_t) -1 )
            m_totalSize += info.size;

        // recursive listing, displayName can be a/b/c/d
        const QString displayName = entry.stringValue( KIO::UDSEntry::UDS_NAME );
        const QString urlStr = entry.stringValue( KIO::UDSEntry::UDS_URL );
        KUrl url;
        if ( !urlStr.isEmpty() )
            url = urlStr;
        QString localPath = entry.stringValue( KIO::UDSEntry::UDS_LOCAL_PATH );
        const bool isDir = entry.isDir();
        info.linkDest = entry.stringValue( KIO::UDSEntry::UDS_LINK_DEST );

        if (displayName != ".." && displayName != ".")
        {
            bool hasCustomURL = !url.isEmpty() || !localPath.isEmpty();
            if( !hasCustomURL ) {
                // Make URL from displayName
                url = static_cast<SimpleJob *>(job)->url();
                if ( m_bCurrentSrcIsDir ) { // Only if src is a directory. Otherwise uSource is fine as is
                    //kDebug(7007) << "adding path " << displayName;
                    url.addPath( displayName );
                }
            }
            //kDebug(7007) << "displayName=" << displayName << " url=" << url;
            if (!localPath.isEmpty() && kio_resolve_local_urls) {
                url = KUrl();
                url.setPath(localPath);
            }

            info.uSource = url;
            info.uDest = m_currentDest;
            //kDebug(7007) << " uSource=" << info.uSource << " uDest(1)=" << info.uDest;
            // Append filename or dirname to destination URL, if allowed
            if ( destinationState == DEST_IS_DIR &&
                 // "copy/move as <foo>" means 'foo' is the dest for the base srcurl
                 // (passed here during stating) but not its children (during listing)
                 ( ! ( m_asMethod && state == STATE_STATING ) ) )
            {
                QString destFileName;
                if ( hasCustomURL &&
                     KProtocolManager::fileNameUsedForCopying( url ) == KProtocolInfo::FromUrl ) {
                    //destFileName = url.fileName(); // Doesn't work for recursive listing
                    // Count the number of prefixes used by the recursive listjob
                    int numberOfSlashes = displayName.count( '/' ); // don't make this a find()!
                    QString path = url.path();
                    int pos = 0;
                    for ( int n = 0; n < numberOfSlashes + 1; ++n ) {
                        pos = path.lastIndexOf( '/', pos - 1 );
                        if ( pos == -1 ) { // error
                            kWarning(7007) << "kioslave bug: not enough slashes in UDS_URL " << path << " - looking for " << numberOfSlashes << " slashes";
                            break;
                        }
                    }
                    if ( pos >= 0 ) {
                        destFileName = path.mid( pos + 1 );
                    }

                } else { // destination filename taken from UDS_NAME
                    destFileName = displayName;
                }

                // Here we _really_ have to add some filename to the dest.
                // Otherwise, we end up with e.g. dest=..../Desktop/ itself.
                // (This can happen when dropping a link to a webpage with no path)
                if ( destFileName.isEmpty() )
                    destFileName = KIO::encodeFileName( info.uSource.prettyUrl() );

                //kDebug(7007) << " adding destFileName=" << destFileName;
                info.uDest.addPath( destFileName );
            }
            //kDebug(7007) << " uDest(2)=" << info.uDest;
            //kDebug(7007) << " " << info.uSource << " -> " << info.uDest;
            if ( info.linkDest.isEmpty() && isDir && m_mode != CopyJob::Link ) // Dir
            {
                dirs.append( info ); // Directories
                if (m_mode == CopyJob::Move)
                    dirsToRemove.append( info.uSource );
            }
            else {
                files.append( info ); // Files and any symlinks
            }
        }
    }
}

void PreserveAttrCopyJob::skipSrc()
{
    m_dest = m_globalDest;
    destinationState = m_globalDestinationState;
    ++m_currentStatSrc;
    skip( m_currentSrcURL );
    statCurrentSrc();
}

void PreserveAttrCopyJob::statNextSrc()
{
    /* Revert to the global destination, the one that applies to all source urls.
     * Imagine you copy the items a b and c into /d, but /d/b exists so the user uses "Rename" to put it in /foo/b instead.
     * m_dest is /foo/b for b, but we have to revert to /d for item c and following.
     */
    m_dest = m_globalDest;
    destinationState = m_globalDestinationState;
    ++m_currentStatSrc;
    statCurrentSrc();
}

void PreserveAttrCopyJob::statCurrentSrc()
{
    if ( m_currentStatSrc != m_srcList.end() )
    {
        m_currentSrcURL = (*m_currentStatSrc);
        m_bURLDirty = true;
        if ( m_mode == CopyJob::Link )
        {
            // Skip the "stating the source" stage, we don't need it for linking
            m_currentDest = m_dest;
            struct CopyInfo info;
            info.permissions = -1;
            info.mtime = (time_t) -1;
            info.ctime = (time_t) -1;
            info.size = (KIO::filesize_t)-1;
            info.uSource = m_currentSrcURL;
            info.uDest = m_currentDest;
            // Append filename or dirname to destination URL, if allowed
            if ( destinationState == DEST_IS_DIR && !m_asMethod )
            {
                if (
                    (m_currentSrcURL.protocol() == info.uDest.protocol()) &&
                    (m_currentSrcURL.host() == info.uDest.host()) &&
                    (m_currentSrcURL.port() == info.uDest.port()) &&
                    (m_currentSrcURL.user() == info.uDest.user()) &&
                    (m_currentSrcURL.pass() == info.uDest.pass()) )
                {
                    // This is the case of creating a real symlink
                    info.uDest.addPath( m_currentSrcURL.fileName() );
                }
                else
                {
                    // Different protocols, we'll create a .desktop file
                    // We have to change the extension anyway, so while we're at it,
                    // name the file like the URL
                    info.uDest.addPath( KIO::encodeFileName( m_currentSrcURL.prettyUrl() )+".desktop" );
                }
            }
            files.append( info ); // Files and any symlinks
            statNextSrc(); // we could use a loop instead of a recursive call :)
            return;
        }
        else if ( m_mode == CopyJob::Move && (
                // Don't go renaming right away if we need a stat() to find out the destination filename
                KProtocolManager::fileNameUsedForCopying( m_currentSrcURL ) == KProtocolInfo::FromUrl ||
                destinationState != DEST_IS_DIR || m_asMethod )
            )
        {
           // If moving, before going for the full stat+[list+]copy+del thing, try to rename
           // The logic is pretty similar to FilePreserveAttrCopyJob::slotStart()
           if ( (m_currentSrcURL.protocol() == m_dest.protocol()) &&
              (m_currentSrcURL.host() == m_dest.host()) &&
              (m_currentSrcURL.port() == m_dest.port()) &&
              (m_currentSrcURL.user() == m_dest.user()) &&
              (m_currentSrcURL.pass() == m_dest.pass()) )
           {
              startRenameJob( m_currentSrcURL );
              return;
           }
           else if ( m_currentSrcURL.isLocalFile() && KProtocolManager::canRenameFromFile( m_dest ) )
           {
              startRenameJob( m_dest );
              return;
           }
           else if ( m_dest.isLocalFile() && KProtocolManager::canRenameToFile( m_currentSrcURL ) )
           {
              startRenameJob( m_currentSrcURL );
              return;
           }
        }

        // if the file system doesn't support deleting, we do not even stat
        if (m_mode == CopyJob::Move && !KProtocolManager::supportsDeleting(m_currentSrcURL)) {
            QPointer<PreserveAttrCopyJob> that = this;
            emit warning( this, buildErrorString(ERR_CANNOT_DELETE, m_currentSrcURL.prettyUrl()) );
            if (that)
                statNextSrc(); // we could use a loop instead of a recursive call :)
            return;
        }

        // Stat the next src url
        Job * job = KIO::stat( m_currentSrcURL, StatJob::SourceSide, 2, KIO::HideProgressInfo );
        //kDebug(7007) << "KIO::stat on " << m_currentSrcURL;
        state = STATE_STATING;
        addSubjob(job);
        m_currentDestURL=m_dest;
        m_bOnlyRenames = false;
        m_bURLDirty = true;
    }
    else
    {
        // Finished the stat'ing phase
        // First make sure that the totals were correctly emitted
        state = STATE_STATING;
        m_bURLDirty = true;
        slotReport();
        if (!dirs.isEmpty()) {
           slotAboutToCreate( dirs );
           emit aboutToCreate( this, dirs );
        }
        if (!files.isEmpty()) {
           slotAboutToCreate( files );
           emit aboutToCreate( this, files );
        }
        // Check if we are copying a single file
        m_bSingleFileCopy = ( files.count() == 1 && dirs.isEmpty() );
        // Then start copying things
        state = STATE_CREATING_DIRS;
        createNextDir();
    }
}

void PreserveAttrCopyJob::startRenameJob( const KUrl& slave_url )
{
    KUrl dest = m_dest;
    // Append filename or dirname to destination URL, if allowed
    if ( destinationState == DEST_IS_DIR && !m_asMethod )
        dest.addPath( m_currentSrcURL.fileName() );
    //kDebug(7007) << "This seems to be a suitable case for trying to rename before stat+[list+]copy+del";
    state = STATE_RENAMING;

    struct CopyInfo info;
    info.permissions = -1;
    info.mtime = (time_t) -1;
    info.ctime = (time_t) -1;
    info.size = (KIO::filesize_t)-1;
    info.uSource = m_currentSrcURL;
    info.uDest = dest;
    QList<CopyInfo> files;
    files.append(info);
    slotAboutToCreate( files );
    emit aboutToCreate( this, files );

    SimpleJob * newJob = KIO::rename( m_currentSrcURL, dest, 0);
    newJob->setUiDelegate(new JobUiDelegate());
    Scheduler::scheduleJob(newJob);
    addSubjob( newJob );
    if ( m_currentSrcURL.directory() != dest.directory() ) // For the user, moving isn't renaming. Only renaming is.
        m_bOnlyRenames = false;
}

void PreserveAttrCopyJob::startListing( const KUrl & src )
{
    state = STATE_LISTING;
    m_bURLDirty = true;
    ListJob * newjob = listRecursive(src, KIO::HideProgressInfo);
    newjob->setUiDelegate(new JobUiDelegate());
    newjob->setUnrestricted(true);
    connect(newjob, SIGNAL(entries( KIO::Job *,const KIO::UDSEntryList& )),
               SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList& )));
    addSubjob( newjob );
}

void PreserveAttrCopyJob::skip( const KUrl & sourceUrl )
{
    // If this is one if toplevel sources,
    // remove it from m_srcList, for a correct FilesRemoved() signal
    //kDebug(7007) << "PreserveAttrCopyJob::skip: looking for " << sourceUrl;
    m_srcList.removeAll( sourceUrl );
    dirsToRemove.removeAll( sourceUrl );
}

bool PreserveAttrCopyJob::shouldOverwrite( const QString& path ) const
{
    if ( m_bOverwriteAll )
        return true;
    QStringList::ConstIterator sit = m_overwriteList.begin();
    for( ; sit != m_overwriteList.end(); ++sit )
        if ( path.startsWith( *sit ) )
            return true;
    return false;
}

bool PreserveAttrCopyJob::shouldSkip( const QString& path ) const
{
    QStringList::ConstIterator sit = m_skipList.begin();
    for( ; sit != m_skipList.end(); ++sit )
        if ( path.startsWith( *sit ) )
            return true;
    return false;
}

void PreserveAttrCopyJob::slotResultCreatingDirs( KJob * job )
{
    // The dir we are trying to create:
    QList<CopyInfo>::Iterator it = dirs.begin();
    // Was there an error creating a dir ?
    if ( job->error() )
    {
        m_conflictError = job->error();
        if ( (m_conflictError == ERR_DIR_ALREADY_EXIST)
             || (m_conflictError == ERR_FILE_ALREADY_EXIST) ) // can't happen?
        {
            KUrl oldURL = ((SimpleJob*)job)->url();
            // Should we skip automatically ?
            if ( m_bAutoSkip ) {
                // We don't want to copy files in this directory, so we put it on the skip list
              m_skipList.append( oldURL.path( KUrl::AddTrailingSlash ) );
                skip( oldURL );
                dirs.erase( it ); // Move on to next dir
            } else {
                // Did the user choose to overwrite already?
                const QString destFile = (*it).uDest.path();
                if ( shouldOverwrite( destFile ) ) { // overwrite => just skip
                    slotCopyingDone( (*it).uSource, (*it).uDest, true );
                    emit copyingDone( this, (*it).uSource, (*it).uDest, (*it).mtime, true /* directory */, false /* renamed */ );
                    dirs.erase( it ); // Move on to next dir
                } else {
                    if ( !isInteractive() ) {
                        Job::slotResult( job ); // will set the error and emit result(this)
                        return;
                    }

                    assert( ((SimpleJob*)job)->url().url() == (*it).uDest.url() );
                    removeSubjob( job );
                    assert ( !hasSubjobs() ); // We should have only one job at a time ...

                    // We need to stat the existing dir, to get its last-modification time
                    KUrl existingDest( (*it).uDest );
                    SimpleJob * newJob = KIO::stat( existingDest, StatJob::DestinationSide, 2, KIO::HideProgressInfo );
                    Scheduler::scheduleJob(newJob);
                    //kDebug(7007) << "KIO::stat for resolving conflict on " << existingDest;
                    state = STATE_CONFLICT_CREATING_DIRS;
                    addSubjob(newJob);
                    return; // Don't move to next dir yet !
                }
            }
        }
        else
        {
            // Severe error, abort
            Job::slotResult( job ); // will set the error and emit result(this)
            return;
        }
    }
    else // no error : remove from list, to move on to next dir
    {
        //this is required for the undo feature
        slotCopyingDone( (*it).uSource, (*it).uDest, true );
        emit copyingDone( this, (*it).uSource, (*it).uDest, (*it).mtime, true, false );
        m_directoriesCopied.append( *it );
        dirs.erase( it );
    }

    m_processedDirs++;
    //emit processedAmount( this, KJob::Directories, m_processedDirs );
    removeSubjob( job );
    assert( !hasSubjobs() ); // We should have only one job at a time ...
    createNextDir();
}

void PreserveAttrCopyJob::slotResultConflictCreatingDirs( KJob * job )
{
    // We come here after a conflict has been detected and we've stated the existing dir

    // The dir we were trying to create:
    QList<CopyInfo>::Iterator it = dirs.begin();

    const UDSEntry entry = ((KIO::StatJob*)job)->statResult();

    // Its modification time:
    const time_t destmtime = (time_t) entry.numberValue( KIO::UDSEntry::UDS_MODIFICATION_TIME, -1 );
    const time_t destctime = (time_t) entry.numberValue( KIO::UDSEntry::UDS_CREATION_TIME, -1 );

    const KIO::filesize_t destsize = entry.numberValue( KIO::UDSEntry::UDS_SIZE );
    const QString linkDest = entry.stringValue( KIO::UDSEntry::UDS_LINK_DEST );

    removeSubjob( job );
    assert ( !hasSubjobs() ); // We should have only one job at a time ...

    // Always multi and skip (since there are files after that)
    RenameDialog_Mode mode = (RenameDialog_Mode)( M_MULTI | M_SKIP );
    // Overwrite only if the existing thing is a dir (no chance with a file)
    if ( m_conflictError == ERR_DIR_ALREADY_EXIST )
    {
        if( (*it).uSource == (*it).uDest ||
            ((*it).uSource.protocol() == (*it).uDest.protocol() &&
              (*it).uSource.path( KUrl::RemoveTrailingSlash ) == linkDest) )
          mode = (RenameDialog_Mode)( mode | M_OVERWRITE_ITSELF);
        else
          mode = (RenameDialog_Mode)( mode | M_OVERWRITE );
    }

    QString existingDest = (*it).uDest.path();
    QString newPath;
    if (m_reportTimer)
        m_reportTimer->stop();
    RenameDialog_Result r = ui()->askFileRename( this, i18n("Folder Already Exists"),
                                         (*it).uSource.url(),
                                         (*it).uDest.url(),
                                         mode, newPath,
                                         (*it).size, destsize,
                                         (*it).ctime, destctime,
                                         (*it).mtime, destmtime );
    if (m_reportTimer)
        m_reportTimer->start(REPORT_TIMEOUT);
    switch ( r ) {
        case R_CANCEL:
            setError( ERR_USER_CANCELED );
            emitResult();
            return;
        case R_RENAME:
        {
          QString oldPath = (*it).uDest.path( KUrl::AddTrailingSlash );
            KUrl newUrl( (*it).uDest );
            newUrl.setPath( newPath );
            emit renamed( this, (*it).uDest, newUrl ); // for e.g. kpropsdlg

            // Change the current one and strip the trailing '/'
            (*it).uDest.setPath( newUrl.path( KUrl::RemoveTrailingSlash ) );
            newPath = newUrl.path( KUrl::AddTrailingSlash ); // With trailing slash
            QList<CopyInfo>::Iterator renamedirit = it;
            ++renamedirit;
            // Change the name of subdirectories inside the directory
            for( ; renamedirit != dirs.end() ; ++renamedirit )
            {
                QString path = (*renamedirit).uDest.path();
                if ( path.startsWith( oldPath ) ) {
                    QString n = path;
                    n.replace( 0, oldPath.length(), newPath );
                    //kDebug(7007) << "dirs list: " << (*renamedirit).uSource.path() << " was going to be " << path << ", changed into " << n << endl;
                    (*renamedirit).uDest.setPath( n );
                }
            }
            // Change filenames inside the directory
            QList<CopyInfo>::Iterator renamefileit = files.begin();
            for( ; renamefileit != files.end() ; ++renamefileit )
            {
                QString path = (*renamefileit).uDest.path();
                if ( path.startsWith( oldPath ) ) {
                    QString n = path;
                    n.replace( 0, oldPath.length(), newPath );
                    //kDebug(7007) << "files list: " << (*renamefileit).uSource.path() << " was going to be " << path << ", changed into " << n << endl;
                    (*renamefileit).uDest.setPath( n );
                }
            }
            if (!dirs.isEmpty()) {
                slotAboutToCreate( dirs );
                emit aboutToCreate( this, dirs );
            }
            if (!files.isEmpty()) {
                slotAboutToCreate( files );
                emit aboutToCreate( this, files );
            }
        }
        break;
        case R_AUTO_SKIP:
            m_bAutoSkip = true;
            // fall through
        case R_SKIP:
            m_skipList.append( existingDest );
            skip( (*it).uSource );
            // Move on to next dir
            dirs.erase( it );
            m_processedDirs++;
            break;
        case R_OVERWRITE:
            m_overwriteList.append( existingDest );
            slotCopyingDone( (*it).uSource, (*it).uDest, true );
            emit copyingDone( this, (*it).uSource, (*it).uDest, (*it).mtime, true /* directory */, false /* renamed */ );
            // Move on to next dir
            dirs.erase( it );
            m_processedDirs++;
            break;
        case R_OVERWRITE_ALL:
            m_bOverwriteAll = true;
            slotCopyingDone( (*it).uSource, (*it).uDest, true );
            emit copyingDone( this, (*it).uSource, (*it).uDest, (*it).mtime, true /* directory */, false /* renamed */ );
            // Move on to next dir
            dirs.erase( it );
            m_processedDirs++;
            break;
        default:
            assert( 0 );
    }
    state = STATE_CREATING_DIRS;
    //emit processedAmount( this, KJob::Directories, m_processedDirs );
    createNextDir();
}

void PreserveAttrCopyJob::createNextDir()
{
    KUrl udir;
    if ( !dirs.isEmpty() )
    {
        // Take first dir to create out of list
        QList<CopyInfo>::Iterator it = dirs.begin();
        // Is this URL on the skip list or the overwrite list ?
        while( it != dirs.end() && udir.isEmpty() )
        {
            const QString dir = (*it).uDest.path();
            if ( shouldSkip( dir ) ) {
                dirs.erase( it );
                it = dirs.begin();
            } else
                udir = (*it).uDest;
        }
    }
    if ( !udir.isEmpty() ) // any dir to create, finally ?
    {
        // Create the directory - with default permissions so that we can put files into it
        // TODO : change permissions once all is finished; but for stuff coming from CDROM it sucks...
        KIO::SimpleJob *newjob = KIO::mkdir( udir, -1 );
        Scheduler::scheduleJob(newjob);

        m_currentDestURL = udir;
        m_bURLDirty = true;

        addSubjob(newjob);
        return;
    }
    else // we have finished creating dirs
    {
        setProcessedAmount( KJob::Directories, m_processedDirs ); // make sure final number appears

        state = STATE_COPYING_FILES;
        m_processedFiles++; // Ralf wants it to start at 1, not 0
        copyNextFile();
    }
}

void PreserveAttrCopyJob::slotResultCopyingFiles( KJob * job )
{
    // The file we were trying to copy:
    QList<CopyInfo>::Iterator it = files.begin();
    if ( job->error() )
    {
        // Should we skip automatically ?
        if ( m_bAutoSkip )
        {
            skip( (*it).uSource );
            m_fileProcessedSize = (*it).size;
            files.erase( it ); // Move on to next file
        }
        else
        {
            if ( !isInteractive() ) {
                Job::slotResult( job ); // will set the error and emit result(this)
                return;
            }

            m_conflictError = job->error(); // save for later
            // Existing dest ?
            if ( ( m_conflictError == ERR_FILE_ALREADY_EXIST )
                 || ( m_conflictError == ERR_DIR_ALREADY_EXIST )
                 || ( m_conflictError == ERR_IDENTICAL_FILES ) )
            {
                removeSubjob( job );
                assert ( !hasSubjobs() );
                // We need to stat the existing file, to get its last-modification time
                KUrl existingFile( (*it).uDest );
                SimpleJob * newJob = KIO::stat( existingFile, StatJob::DestinationSide, 2, KIO::HideProgressInfo );
                Scheduler::scheduleJob(newJob);
                //kDebug(7007) << "KIO::stat for resolving conflict on " << existingFile;
                state = STATE_CONFLICT_COPYING_FILES;
                addSubjob(newJob);
                return; // Don't move to next file yet !
            }
            else
            {
                if ( m_bCurrentOperationIsLink && qobject_cast<KIO::DeleteJob*>( job ) )
                {
                    // Very special case, see a few lines below
                    // We are deleting the source of a symlink we successfully moved... ignore error
                    m_fileProcessedSize = (*it).size;
                    files.erase( it );
                } else {
                    // Go directly to the conflict resolution, there is nothing to stat
                    slotResultConflictCopyingFiles( job );
                    return;
                }
            }
        }
    } else // no error
    {
        // Special case for moving links. That operation needs two jobs, unlike others.
        if ( m_bCurrentOperationIsLink && m_mode == CopyJob::Move
             && !qobject_cast<KIO::DeleteJob *>( job ) // Deleting source not already done
             )
        {
            removeSubjob( job );
            assert ( !hasSubjobs() );
            // The only problem with this trick is that the error handling for this del operation
            // is not going to be right... see 'Very special case' above.
            KIO::Job * newjob = KIO::del( (*it).uSource, HideProgressInfo );
            addSubjob( newjob );
            return; // Don't move to next file yet !
        }

        if ( m_bCurrentOperationIsLink )
        {
            QString target = ( m_mode == CopyJob::Link ? (*it).uSource.path() : (*it).linkDest );
            //required for the undo feature
            emit copyingLinkDone( this, (*it).uSource, target, (*it).uDest );
        }
        else {
            //required for the undo feature
            slotCopyingDone( (*it).uSource, (*it).uDest, false );
            emit copyingDone( this, (*it).uSource, (*it).uDest, (*it).mtime, false, false );
       }
        // remove from list, to move on to next file
        files.erase( it );
    }
    m_processedFiles++;

    // clear processed size for last file and add it to overall processed size
    m_processedSize += m_fileProcessedSize;
    m_fileProcessedSize = 0;

    //kDebug(7007) << files.count() << " files remaining";

    // Merge metadata from subjob
    KIO::Job* kiojob = dynamic_cast<KIO::Job*>(job);
    Q_ASSERT(kiojob);
    //m_incomingMetaData += kiojob->metaData();
    removeSubjob( job );
    assert( !hasSubjobs() ); // We should have only one job at a time ...
    copyNextFile();
}

void PreserveAttrCopyJob::slotResultConflictCopyingFiles( KJob * job )
{
    // We come here after a conflict has been detected and we've stated the existing file
    // The file we were trying to create:
    QList<CopyInfo>::Iterator it = files.begin();

    RenameDialog_Result res;
    QString newPath;

    if (m_reportTimer)
        m_reportTimer->stop();

    if ( ( m_conflictError == ERR_FILE_ALREADY_EXIST )
         || ( m_conflictError == ERR_DIR_ALREADY_EXIST )
         || ( m_conflictError == ERR_IDENTICAL_FILES ) )
    {
        // Its modification time:
        const UDSEntry entry = ((KIO::StatJob*)job)->statResult();

        const time_t destmtime = (time_t) entry.numberValue( KIO::UDSEntry::UDS_MODIFICATION_TIME, -1 );
        const time_t destctime = (time_t) entry.numberValue( KIO::UDSEntry::UDS_CREATION_TIME, -1 );
        const KIO::filesize_t destsize = entry.numberValue( KIO::UDSEntry::UDS_SIZE );
        const QString linkDest = entry.stringValue( KIO::UDSEntry::UDS_LINK_DEST );

        // Offer overwrite only if the existing thing is a file
        // If src==dest, use "overwrite-itself"
        RenameDialog_Mode mode;
        bool isDir = true;

        if( m_conflictError == ERR_DIR_ALREADY_EXIST )
            mode = (RenameDialog_Mode) 0;
        else
        {
            if ( (*it).uSource == (*it).uDest  ||
                 ((*it).uSource.protocol() == (*it).uDest.protocol() &&
                   (*it).uSource.path( KUrl::RemoveTrailingSlash ) == linkDest) )
                mode = M_OVERWRITE_ITSELF;
            else
                mode = M_OVERWRITE;
            isDir = false;
        }

        if ( m_bSingleFileCopy )
            mode = (RenameDialog_Mode) ( mode | M_SINGLE );
        else
            mode = (RenameDialog_Mode) ( mode | M_MULTI | M_SKIP );

        res = ui()->askFileRename( this, !isDir ?
                                   i18n("File Already Exists") : i18n("Already Exists as Folder"),
                                   (*it).uSource.url(),
                                   (*it).uDest.url(),
                                   mode, newPath,
                                   (*it).size, destsize,
                                   (*it).ctime, destctime,
                                   (*it).mtime, destmtime );

    }
    else
    {
        if ( job->error() == ERR_USER_CANCELED )
            res = R_CANCEL;
        else if ( !isInteractive() ) {
            Job::slotResult( job ); // will set the error and emit result(this)
            return;
        }
        else
        {
            SkipDialog_Result skipResult = ui()->askSkip( this, files.count() > 1,
                                                          job->errorString() );

            // Convert the return code from SkipDialog into a RenameDialog code
            res = ( skipResult == S_SKIP ) ? R_SKIP :
                         ( skipResult == S_AUTO_SKIP ) ? R_AUTO_SKIP :
                                        R_CANCEL;
        }
    }

    if (m_reportTimer)
        m_reportTimer->start(REPORT_TIMEOUT);

    removeSubjob( job );
    assert ( !hasSubjobs() );
    switch ( res ) {
        case R_CANCEL:
            setError( ERR_USER_CANCELED );
            emitResult();
            return;
        case R_RENAME:
        {
            KUrl newUrl( (*it).uDest );
            newUrl.setPath( newPath );
            emit renamed( this, (*it).uDest, newUrl ); // for e.g. kpropsdlg
            (*it).uDest = newUrl;

            QList<CopyInfo> files;
            files.append(*it);
            slotAboutToCreate( files );
            emit aboutToCreate( this, files );
        }
        break;
        case R_AUTO_SKIP:
            m_bAutoSkip = true;
            // fall through
        case R_SKIP:
            // Move on to next file
            skip( (*it).uSource );
            m_processedSize += (*it).size;
            files.erase( it );
            m_processedFiles++;
            break;
       case R_OVERWRITE_ALL:
            m_bOverwriteAll = true;
            break;
        case R_OVERWRITE:
            // Add to overwrite list, so that copyNextFile knows to overwrite
            m_overwriteList.append( (*it).uDest.path() );
            break;
        default:
            assert( 0 );
    }
    state = STATE_COPYING_FILES;
    //emit processedAmount( this, KJob::Files, m_processedFiles );
    copyNextFile();
}

KIO::Job* PreserveAttrCopyJob::linkNextFile( const KUrl& uSource, const KUrl& uDest, JobFlags flags )
{
    //kDebug(7007) << "Linking";
    if (
        (uSource.protocol() == uDest.protocol()) &&
        (uSource.host() == uDest.host()) &&
        (uSource.port() == uDest.port()) &&
        (uSource.user() == uDest.user()) &&
        (uSource.pass() == uDest.pass()) )
    {
        // This is the case of creating a real symlink
        KIO::SimpleJob *newJob = KIO::symlink( uSource.path(), uDest, flags|HideProgressInfo /*no GUI*/ );
        Scheduler::scheduleJob(newJob);
        //kDebug(7007) << "PreserveAttrCopyJob::copyNextFile : Linking target=" << uSource.path() << " link=" << uDest;
        //emit linking( this, uSource.path(), uDest );
        m_bCurrentOperationIsLink = true;
        m_currentSrcURL=uSource;
        m_currentDestURL=uDest;
        m_bURLDirty = true;
        //Observer::self()->slotCopying( this, uSource, uDest ); // should be slotLinking perhaps
        return newJob;
    } else {
        //kDebug(7007) << "PreserveAttrCopyJob::copyNextFile : Linking URL=" << uSource << " link=" << uDest;
        if ( uDest.isLocalFile() ) {
            // if the source is a devices url, handle it a littlebit special

            QString path = uDest.path();
            //kDebug(7007) << "PreserveAttrCopyJob::copyNextFile path=" << path;
            QFile f( path );
            if ( f.open( QIODevice::ReadWrite ) )
            {
                f.close();
                KDesktopFile desktopFile( path );
                KConfigGroup config = desktopFile.desktopGroup();
                KUrl url = uSource;
                url.setPass( "" );
                config.writePathEntry( "URL", url.url() );
                config.writeEntry( "Name", url.url() );
                config.writeEntry( "Type", QString::fromLatin1("Link") );
                QString protocol = uSource.protocol();
                if ( protocol == QLatin1String("ftp") )
                    config.writeEntry( "Icon", QString::fromLatin1("folder-remote") );
                else if ( protocol == QLatin1String("http") )
                    config.writeEntry( "Icon", QString::fromLatin1("text-html") );
                else if ( protocol == QLatin1String("document-properties") )
                    config.writeEntry( "Icon", QString::fromLatin1("text-x-texinfo") );
                else if ( protocol == QLatin1String("mailto") )   // sven:
                    config.writeEntry( "Icon", QString::fromLatin1("internet-mail") ); // added mailto: support
                else
                    config.writeEntry( "Icon", QString::fromLatin1("unknown") );
                config.sync();
                files.erase( files.begin() ); // done with this one, move on
                m_processedFiles++;
                //emit processedAmount( this, KJob::Files, m_processedFiles );
                copyNextFile();
                return 0;
            }
            else
            {
                //kDebug(7007) << "PreserveAttrCopyJob::copyNextFile ERR_CANNOT_OPEN_FOR_WRITING";
                setError( ERR_CANNOT_OPEN_FOR_WRITING );
                setErrorText( uDest.path() );
                emitResult();
                return 0;
            }
        } else {
            // Todo: not show "link" on remote dirs if the src urls are not from the same protocol+host+...
            setError( ERR_CANNOT_SYMLINK );
            setErrorText( uDest.prettyUrl() );
            emitResult();
            return 0;
        }
    }
}

void PreserveAttrCopyJob::copyNextFile()
{
    bool bCopyFile = false;
    //kDebug(7007) << "PreserveAttrCopyJob::copyNextFile()";
    // Take the first file in the list
    QList<CopyInfo>::Iterator it = files.begin();
    // Is this URL on the skip list ?
    while (it != files.end() && !bCopyFile)
    {
        const QString destFile = (*it).uDest.path();
        bCopyFile = !shouldSkip( destFile );
        if ( !bCopyFile ) {
            files.erase( it );
            it = files.begin();
        }
    }

    if (bCopyFile) // any file to create, finally ?
    {
        const KUrl& uSource = (*it).uSource;
        const KUrl& uDest = (*it).uDest;
        // Do we set overwrite ?
        bool bOverwrite;
        const QString destFile = uDest.path();
        //kDebug(7007) << "copying " << destFile;
        if ( uDest == uSource )
            bOverwrite = false;
        else
            bOverwrite = shouldOverwrite( destFile );

        m_bCurrentOperationIsLink = false;
        KIO::Job * newjob = 0;
        if ( m_mode == CopyJob::Link ) {
            // User requested that a symlink be made
          JobFlags flags = bOverwrite ? Overwrite : DefaultFlags;
            newjob = linkNextFile(uSource, uDest, flags);
            if (!newjob)
                return;
        } else if ( !(*it).linkDest.isEmpty() &&
                  (uSource.protocol() == uDest.protocol()) &&
                  (uSource.host() == uDest.host()) &&
                  (uSource.port() == uDest.port()) &&
                  (uSource.user() == uDest.user()) &&
                  (uSource.pass() == uDest.pass()))
            // Copying a symlink - only on the same protocol/host/etc. (#5601, downloading an FTP file through its link),
        {
            JobFlags flags = bOverwrite ? Overwrite : DefaultFlags;
            KIO::SimpleJob *newJob = KIO::symlink( (*it).linkDest, uDest, flags | HideProgressInfo /*no GUI*/ );
            Scheduler::scheduleJob(newJob);
            newjob = newJob;
            //kDebug(7007) << "PreserveAttrCopyJob::copyNextFile : Linking target=" << (*it).linkDest << " link=" << uDest;
            m_currentSrcURL = KUrl( (*it).linkDest );
            m_currentDestURL = uDest;
            m_bURLDirty = true;
            //emit linking( this, (*it).linkDest, uDest );
            //Observer::self()->slotCopying( this, m_currentSrcURL, uDest ); // should be slotLinking perhaps
            m_bCurrentOperationIsLink = true;
            // NOTE: if we are moving stuff, the deletion of the source will be done in slotResultCopyingFiles
        } else if (m_mode == CopyJob::Move) // Moving a file
        {
            JobFlags flags = bOverwrite ? Overwrite : DefaultFlags;
            KIO::FileCopyJob * moveJob = KIO::file_move( uSource, uDest, (*it).permissions, flags | HideProgressInfo/*no GUI*/ );
            moveJob->setSourceSize( (*it).size );
            newjob = moveJob;
            //kDebug(7007) << "PreserveAttrCopyJob::copyNextFile : Moving " << uSource << " to " << uDest;
            //emit moving( this, uSource, uDest );
            m_currentSrcURL=uSource;
            m_currentDestURL=uDest;
            m_bURLDirty = true;
            //Observer::self()->slotMoving( this, uSource, uDest );
        }
        else // Copying a file
        {
            // If source isn't local and target is local, we ignore the original permissions
            // Otherwise, files downloaded from HTTP end up with -r--r--r--
            bool remoteSource = !KProtocolManager::supportsListing(uSource);
            int permissions = (*it).permissions;
            if ( m_defaultPermissions || ( remoteSource && uDest.isLocalFile() ) )
                permissions = -1;
            JobFlags flags = bOverwrite ? Overwrite : DefaultFlags;
            KIO::FileCopyJob * copyJob = KIO::file_copy( uSource, uDest, permissions, flags | HideProgressInfo/*no GUI*/ );
            copyJob->setParentJob( this ); // in case of rename dialog
            copyJob->setSourceSize( (*it).size );
            if ((*it).mtime != -1) {
                QDateTime dt; dt.setTime_t( (*it).mtime );
                copyJob->setModificationTime( dt );
            }
            newjob = copyJob;
            //kDebug(7007) << "PreserveAttrCopyJob::copyNextFile : Copying " << uSource << " to " << uDest;
            m_currentSrcURL=uSource;
            m_currentDestURL=uDest;
            m_bURLDirty = true;
        }
        addSubjob(newjob);
        connect( newjob, SIGNAL( processedSize( KJob*, qulonglong ) ),
                    SLOT( slotProcessedSize( KJob*, qulonglong ) ) );
        connect( newjob, SIGNAL( totalSize( KJob*, qulonglong ) ),
                    SLOT( slotTotalSize( KJob*, qulonglong ) ) );
    }
    else
    {
        // We're done
        //kDebug(7007) << "copyNextFile finished";
        deleteNextDir();
    }
}

void PreserveAttrCopyJob::deleteNextDir()
{
    if ( m_mode == CopyJob::Move && !dirsToRemove.isEmpty() ) // some dirs to delete ?
    {
        state = STATE_DELETING_DIRS;
        m_bURLDirty = true;
        // Take first dir to delete out of list - last ones first !
        KUrl::List::Iterator it = --dirsToRemove.end();
        SimpleJob *job = KIO::rmdir( *it );
        Scheduler::scheduleJob(job);
        dirsToRemove.erase(it);
        addSubjob( job );
    }
    else
    {
        // This step is done, move on
        state = STATE_SETTING_DIR_ATTRIBUTES;
        m_directoriesCopiedIterator = m_directoriesCopied.begin();
        setNextDirAttribute();
    }
}

void PreserveAttrCopyJob::setNextDirAttribute()
{
    while (m_directoriesCopiedIterator != m_directoriesCopied.end() &&
           (*m_directoriesCopiedIterator).mtime == -1) {
        ++m_directoriesCopiedIterator;
    }
    if ( m_directoriesCopiedIterator != m_directoriesCopied.end() ) {
        const KUrl url = (*m_directoriesCopiedIterator).uDest;
        const time_t mtime = (*m_directoriesCopiedIterator).mtime;
        const QDateTime dt = QDateTime::fromTime_t(mtime);
        ++m_directoriesCopiedIterator;

        KIO::SimpleJob *job = KIO::setModificationTime( url, dt );
        Scheduler::scheduleJob(job);
        addSubjob( job );


#if 0 // ifdef Q_OS_UNIX
        // TODO: can be removed now. Or reintroduced as a fast path for local files
        // if launching even more jobs as done above is a performance problem.
        //
        QLinkedList<CopyInfo>::Iterator it = m_directoriesCopied.begin();
        for ( ; it != m_directoriesCopied.end() ; ++it ) {
            const KUrl& url = (*it).uDest;
            if ( url.isLocalFile() && (*it).mtime != (time_t)-1 ) {
                const QByteArray path = QFile::encodeName( url.path() );
                KDE_struct_stat statbuf;
                if (KDE_lstat(path, &statbuf) == 0) {
                    struct utimbuf utbuf;
                    utbuf.actime = statbuf.st_atime; // access time, unchanged
                    utbuf.modtime = (*it).mtime; // modification time
                    utime( path, &utbuf );
                }

            }
        }
        m_directoriesCopied.clear();
        // but then we need to jump to the else part below. Maybe with a recursive call?
#endif
    } else {
        // Finished - tell the world
        if ( !m_bOnlyRenames )
        {
            KUrl url( m_globalDest );
            if ( m_globalDestinationState != DEST_IS_DIR || m_asMethod )
                url.setPath( url.directory() );
            //kDebug(7007) << "KDirNotify'ing FilesAdded " << url;
            org::kde::KDirNotify::emitFilesAdded( url.url() );

            if ( m_mode == CopyJob::Move && !m_srcList.isEmpty() ) {
                //kDebug(7007) << "KDirNotify'ing FilesRemoved " << m_srcList.toStringList();
                org::kde::KDirNotify::emitFilesRemoved( m_srcList.toStringList() );
            }
        }
        if (m_reportTimer)
            m_reportTimer->stop();
        --m_processedFiles; // undo the "start at 1" hack
        slotReport(); // display final numbers, important if progress dialog stays up

        emitResult();
    }
}

void PreserveAttrCopyJob::slotProcessedSize( KJob*, qulonglong data_size )
{
  //kDebug(7007) << "PreserveAttrCopyJob::slotProcessedSize " << data_size;
  m_fileProcessedSize = data_size;
  setProcessedAmount(KJob::Bytes, m_processedSize + m_fileProcessedSize);

  if ( m_processedSize + m_fileProcessedSize > m_totalSize )
  {
    m_totalSize = m_processedSize + m_fileProcessedSize;
    //kDebug(7007) << "Adjusting m_totalSize to " << m_totalSize;
    setTotalAmount(KJob::Bytes, m_totalSize); // safety
  }
  //kDebug(7007) << "emit processedSize " << (unsigned long) (m_processedSize + m_fileProcessedSize);
  setProcessedAmount(KJob::Bytes, m_processedSize + m_fileProcessedSize);
}

void PreserveAttrCopyJob::slotTotalSize( KJob*, qulonglong size )
{
  //kDebug(7007) << "slotTotalSize: " << size;
  // Special case for copying a single file
  // This is because some protocols don't implement stat properly
  // (e.g. HTTP), and don't give us a size in some cases (redirection)
  // so we'd rather rely on the size given for the transfer
  if ( m_bSingleFileCopy && size > m_totalSize)
  {
    //kDebug(7007) << "slotTotalSize: updating totalsize to " << size;
    m_totalSize = size;
    setTotalAmount(KJob::Bytes, size);
  }
}

void PreserveAttrCopyJob::slotResultDeletingDirs( KJob * job )
{
    if (job->error())
    {
        // Couldn't remove directory. Well, perhaps it's not empty
        // because the user pressed Skip for a given file in it.
        // Let's not display "Could not remove dir ..." for each of those dir !
    }
    removeSubjob( job );
    assert( !hasSubjobs() );
    deleteNextDir();
}

void PreserveAttrCopyJob::slotResultSettingDirAttributes( KJob * job )
{
    if (job->error())
    {
        // Couldn't set directory attributes. Ignore the error, it can happen
        // with inferior file systems like VFAT.
        // Let's not display warnings for each dir like "cp -a" does.
    }
    removeSubjob( job );
    assert( !hasSubjobs() );
    setNextDirAttribute();
}

// We were trying to do a direct renaming, before even stat'ing
void PreserveAttrCopyJob::slotResultRenaming( KJob* job )
{
    int err = job->error();
    const QString errText = job->errorText();
    // Merge metadata from subjob
    KIO::Job* kiojob = dynamic_cast<KIO::Job*>(job);
    Q_ASSERT(kiojob);
    //m_incomingMetaData += kiojob->metaData();
    removeSubjob( job );
    assert ( !hasSubjobs() );
    // Determine dest again
    KUrl dest = m_dest;
    if ( destinationState == DEST_IS_DIR && !m_asMethod )
        dest.addPath( m_currentSrcURL.fileName() );
    if ( err )
    {
        // Direct renaming didn't work. Try renaming to a temp name,
        // this can help e.g. when renaming 'a' to 'A' on a VFAT partition.
        // In that case it's the _same_ dir, we don't want to copy+del (data loss!)
      if ( m_currentSrcURL.isLocalFile() && m_currentSrcURL.url(KUrl::RemoveTrailingSlash) != dest.url(KUrl::RemoveTrailingSlash) &&
           m_currentSrcURL.url(KUrl::RemoveTrailingSlash).toLower() == dest.url(KUrl::RemoveTrailingSlash).toLower() &&
             ( err == ERR_FILE_ALREADY_EXIST ||
               err == ERR_DIR_ALREADY_EXIST ||
               err == ERR_IDENTICAL_FILES ) )
        {
            //kDebug(7007) << "Couldn't rename directly, dest already exists. Detected special case of lower/uppercase renaming in same dir, try with 2 rename calls";
            QByteArray _src( QFile::encodeName(m_currentSrcURL.path()) );
            QByteArray _dest( QFile::encodeName(dest.path()) );
            KTemporaryFile tmpFile;
            tmpFile.setPrefix(m_currentSrcURL.directory(KUrl::ObeyTrailingSlash));
            tmpFile.setAutoRemove(false);
            tmpFile.open();
            QByteArray _tmp( QFile::encodeName(tmpFile.fileName()) );
            //kDebug(7007) << "PreserveAttrCopyJob::slotResult KTemporaryFile using " << _tmp << " as intermediary";
            if ( ::rename( _src, _tmp ) == 0 )
            {
                if ( !QFile::exists( _dest ) && ::rename( _tmp, _dest ) == 0 )
                {
                    //kDebug(7007) << "Success.";
                    err = 0;
                }
                else
                {
                    // Revert back to original name!
                    if ( ::rename( _tmp, _src ) != 0 ) {
                        kError(7007) << "Couldn't rename " << tmpFile.fileName() << " back to " << _src << " !" << endl;
                        // Severe error, abort
                        Job::slotResult( job ); // will set the error and emit result(this)
                        return;
                    }
                }
            }
        }
    }
    if ( err )
    {
        // This code is similar to PreserveAttrCopyJob::slotResultConflictCopyingFiles
        // but here it's about the base src url being moved/renamed
        // (*m_currentStatSrc) and its dest (m_dest), not about a single file.
        // It also means we already stated the dest, here.
        // On the other hand we haven't stated the src yet (we skipped doing it
        // to save time, since it's not necessary to rename directly!)...

        Q_ASSERT( m_currentSrcURL == *m_currentStatSrc );

        // Existing dest?
        if ( ( err == ERR_DIR_ALREADY_EXIST ||
               err == ERR_FILE_ALREADY_EXIST ||
               err == ERR_IDENTICAL_FILES )
             && isInteractive() )
        {
            if (m_reportTimer)
                m_reportTimer->stop();

            // Should we skip automatically ?
            if ( m_bAutoSkip ) {
                // Move on to next file
                skipSrc();
                return;
            } else if ( m_bOverwriteAll ) {
                ; // nothing to do, stat+copy+del will overwrite
            } else {
                QString newPath;
                // If src==dest, use "overwrite-itself"
                RenameDialog_Mode mode = (RenameDialog_Mode)
                                      ( ( m_currentSrcURL == dest ) ? M_OVERWRITE_ITSELF : M_OVERWRITE );

                if ( m_srcList.count() > 1 )
                    mode = (RenameDialog_Mode) ( mode | M_MULTI | M_SKIP );
                else
                    mode = (RenameDialog_Mode) ( mode | M_SINGLE );

                // we lack mtime info for both the src (not stated)
                // and the dest (stated but this info wasn't stored)
                // Let's do it for local files, at least
                KIO::filesize_t sizeSrc = (KIO::filesize_t) -1;
                KIO::filesize_t sizeDest = (KIO::filesize_t) -1;
                time_t ctimeSrc = (time_t) -1;
                time_t ctimeDest = (time_t) -1;
                time_t mtimeSrc = (time_t) -1;
                time_t mtimeDest = (time_t) -1;

                KDE_struct_stat stat_buf;
                if ( m_currentSrcURL.isLocalFile() &&
                    KDE_stat(QFile::encodeName(m_currentSrcURL.path()), &stat_buf) == 0 ) {
                    sizeSrc = stat_buf.st_size;
                    ctimeSrc = stat_buf.st_ctime;
                    mtimeSrc = stat_buf.st_mtime;
                }
                if ( dest.isLocalFile() &&
                    KDE_stat(QFile::encodeName(dest.path()), &stat_buf) == 0 ) {
                    sizeDest = stat_buf.st_size;
                    ctimeDest = stat_buf.st_ctime;
                    mtimeDest = stat_buf.st_mtime;
                }

                RenameDialog_Result r = ui()->askFileRename(
                    this,
                    err != ERR_DIR_ALREADY_EXIST ? i18n("File Already Exists") : i18n("Already Exists as Folder"),
                    m_currentSrcURL.url(),
                    dest.url(),
                    mode, newPath,
                    sizeSrc, sizeDest,
                    ctimeSrc, ctimeDest,
                    mtimeSrc, mtimeDest );
                if (m_reportTimer)
                    m_reportTimer->start(REPORT_TIMEOUT);

                switch ( r )
                {
                case R_CANCEL:
                {
                    setError( ERR_USER_CANCELED );
                    emitResult();
                    return;
                }
                case R_RENAME:
                {
                    // Set m_dest to the chosen destination
                    // This is only for this src url; the next one will revert to m_globalDest
                    m_dest.setPath( newPath );
                    KIO::Job* job = KIO::stat( m_dest, StatJob::DestinationSide, 2, KIO::HideProgressInfo );
                    state = STATE_STATING;
                    destinationState = DEST_NOT_STATED;
                    addSubjob(job);
                    return;
                }
                case R_AUTO_SKIP:
                    m_bAutoSkip = true;
                    // fall through
                case R_SKIP:
                    // Move on to next file
                    skipSrc();
                    return;
                case R_OVERWRITE_ALL:
                    m_bOverwriteAll = true;
                    break;
                case R_OVERWRITE:
                    // Add to overwrite list
                    // Note that we add dest, not m_dest.
                    // This ensures that when moving several urls into a dir (m_dest),
                    // we only overwrite for the current one, not for all.
                    // When renaming a single file (m_asMethod), it makes no difference.
                    //kDebug(7007) << "adding to overwrite list: " << dest.path();
                    m_overwriteList.append( dest.path() );
                    break;
                default:
                    //assert( 0 );
                    break;
                }
            }
        } else if ( err != KIO::ERR_UNSUPPORTED_ACTION ) {
            //kDebug(7007) << "Couldn't rename " << m_currentSrcURL << " to " << dest << ", aborting";
            setError( err );
            setErrorText( errText );
            emitResult();
            return;
        }
        //kDebug(7007) << "Couldn't rename " << m_currentSrcURL << " to " << dest << ", reverting to normal way, starting with stat";
        //kDebug(7007) << "KIO::stat on " << m_currentSrcURL;
        KIO::Job* job = KIO::stat( m_currentSrcURL, StatJob::SourceSide, 2, KIO::HideProgressInfo );
        state = STATE_STATING;
        addSubjob(job);
        m_bOnlyRenames = false;
    }
    else
    {
        //kDebug(7007) << "Renaming succeeded, move on";
        slotCopyingDone( *m_currentStatSrc, dest, true );
        emit copyingDone( this, *m_currentStatSrc, dest, -1 /*mtime unknown, and not needed*/, true, true );
        statNextSrc();
    }
}

void PreserveAttrCopyJob::slotResult( KJob *job )
{
    //kDebug(7007) << "PreserveAttrCopyJob::slotResult() state=" << (int) state;
    // In each case, what we have to do is :
    // 1 - check for errors and treat them
    // 2 - removeSubjob(job);
    // 3 - decide what to do next

    switch ( state ) {
        case STATE_STATING: // We were trying to stat a src url or the dest
            slotResultStating( job );
            break;
        case STATE_RENAMING: // We were trying to do a direct renaming, before even stat'ing
        {
            slotResultRenaming( job );
            break;
        }
        case STATE_LISTING: // recursive listing finished
            //kDebug(7007) << "totalSize: " << (unsigned int) m_totalSize << " files: " << files.count() << " dirs: " << dirs.count();
            // Was there an error ?
            if (job->error())
            {
                Job::slotResult( job ); // will set the error and emit result(this)
                return;
            }

            removeSubjob( job );
            assert ( !hasSubjobs() );

            statNextSrc();
            break;
        case STATE_CREATING_DIRS:
            slotResultCreatingDirs( job );
            break;
        case STATE_CONFLICT_CREATING_DIRS:
            slotResultConflictCreatingDirs( job );
            break;
        case STATE_COPYING_FILES:
            slotResultCopyingFiles( job );
            break;
        case STATE_CONFLICT_COPYING_FILES:
            slotResultConflictCopyingFiles( job );
            break;
        case STATE_DELETING_DIRS:
            slotResultDeletingDirs( job );
            break;
        case STATE_SETTING_DIR_ATTRIBUTES:
            slotResultSettingDirAttributes( job );
            break;
        default:
            assert( 0 );
    }
}

void PreserveAttrCopyJob::saveEntries(KIO::Job *job, const KIO::UDSEntryList &list) {
  KIO::UDSEntryList::const_iterator it = list.begin();
  KIO::UDSEntryList::const_iterator end = list.end();
  for (; it != end; ++it) {
    KUrl url = ((KIO::SimpleJob *)job)->url();
    QString relName, user, group;
    time_t mtime = (time_t)-1;
    mode_t mode = 0755;
    QString acl;

    if( (*it).contains( KIO::UDSEntry::UDS_URL ) )
      relName = KUrl( (*it).stringValue( KIO::UDSEntry::UDS_URL ) ).fileName();
    else if( (*it).contains( KIO::UDSEntry::UDS_NAME ) )
      relName = (*it).stringValue( KIO::UDSEntry::UDS_NAME );

    if( (*it).contains( KIO::UDSEntry::UDS_MODIFICATION_TIME ) )
      mtime = (time_t)((*it).numberValue( KIO::UDSEntry::UDS_MODIFICATION_TIME ));

    if( (*it).contains( KIO::UDSEntry::UDS_USER ) )
      user = (*it).stringValue( KIO::UDSEntry::UDS_USER );

    if( (*it).contains( KIO::UDSEntry::UDS_GROUP ) )
      group = (*it).stringValue( KIO::UDSEntry::UDS_GROUP );

    if( (*it).contains( KIO::UDSEntry::UDS_ACCESS ) )
      mode = ((*it).numberValue( KIO::UDSEntry::UDS_ACCESS ));

#ifdef HAVE_POSIX_ACL
    if( (*it).contains( KIO::UDSEntry::UDS_ACL_STRING ) )
      acl = (*it).stringValue( KIO::UDSEntry::UDS_ACL_STRING );
#endif

    url.addPath( relName );

    fileAttributes[ url ] = Attributes( mtime, user, group, mode, acl );
  }
}

void PreserveAttrCopyJob::slotAboutToCreate( const QList< KIO::CopyInfo > &files )
{
  for ( QList< KIO::CopyInfo >::ConstIterator it = files.begin(); it != files.end(); ++it ) {
  
    if( (*it).uSource.isLocalFile() ) {
      KDE_struct_stat stat_p;
      KDE_lstat( (*it).uSource.path(KUrl::RemoveTrailingSlash).toLocal8Bit(),&stat_p);    /* getting the date information */
      
      QString aclStr;
#ifdef HAVE_POSIX_ACL
      acl_t acl = acl_get_file( (*it).uSource.path(KUrl::RemoveTrailingSlash).toLocal8Bit(), ACL_TYPE_ACCESS );

      bool aclExtended = false;
      if( acl )
      {
#ifdef HAVE_NON_POSIX_ACL_EXTENSIONS
        aclExtended = acl_equiv_mode( acl, 0 );
#else
        acl_entry_t entry;
        int ret = acl_get_entry( acl, ACL_FIRST_ENTRY, &entry );
        while ( ret == 1 ) {
          acl_tag_t currentTag;
          acl_get_tag_type( entry, &currentTag );
          if ( currentTag != ACL_USER_OBJ &&
            currentTag != ACL_GROUP_OBJ &&
            currentTag != ACL_OTHER )
          {
            aclExtended = true;
            break;
          }
          ret = acl_get_entry( acl, ACL_NEXT_ENTRY, &entry );
        }
#endif
      }


      if ( acl && !aclExtended ) {
        acl_free( acl );
        acl = NULL;
      }
      if( acl )
      {
        char *aclString = acl_to_text( acl, 0 );
        aclStr = QString::fromLatin1( aclString );
        acl_free( (void*)aclString );
        acl_free( acl );
      }
#endif
      fileAttributes[ (*it).uSource ] = Attributes( stat_p.st_mtime, stat_p.st_uid, stat_p.st_gid, stat_p.st_mode & 07777, aclStr );
    }
    else {
      time_t mtime = (*it).mtime;
  
      if( mtime != 0 && mtime != ((time_t) -1 ) )       /* is it correct? */
        fileAttributes[ (*it).uSource ].time = mtime;

      int permissions = (*it).permissions;
      fileAttributes[ (*it).uSource ].mode = permissions;
    }
  }
}

void PreserveAttrCopyJob::slotCopyingDone( const KUrl &from, const KUrl &to, bool postpone )
{
  if( m_mode == CopyJob::Link )
    return;

  if( postpone ) { // the directories are stamped at the last step, so if it's a directory, we postpone
    int i=0;
    QString path = to.path( KUrl::RemoveTrailingSlash );

    for( ; i != directoriesToStamp.count(); i++ ) // sort the URL-s to avoid parent time stamp modification
      if( path >= directoriesToStamp[ i ].path( KUrl::RemoveTrailingSlash ) )
        break;

    if( i != directoriesToStamp.count() ) {
      directoriesToStamp.insert( directoriesToStamp.begin() + i, to );
      originalDirectories.insert( originalDirectories.begin()+ i, from );
    } else {
      directoriesToStamp.append( to );
      originalDirectories.append( from );
    }
  }
  else if( fileAttributes.count( from ) ) {
    Attributes attrs = fileAttributes[ from ];
    fileAttributes.remove( from );
    
    time_t mtime = attrs.time;
   
    if( to.isLocalFile() )
    {
      if( mtime != 0 && mtime != ((time_t) -1 ) )
      {
        struct utimbuf timestamp;

        timestamp.actime  = time( 0 );
        timestamp.modtime = mtime;

        ::utime( (const char *)( to.path( KUrl::RemoveTrailingSlash ).toLocal8Bit() ), &timestamp );
      }

      if( attrs.uid != (uid_t)-1 )
        ::chown( (const char *)( to.path( KUrl::RemoveTrailingSlash ).toLocal8Bit() ), attrs.uid, (gid_t)-1 );
      if( attrs.gid != (gid_t)-1 )
        ::chown( (const char *)( to.path( KUrl::RemoveTrailingSlash ).toLocal8Bit() ), (uid_t)-1, attrs.gid );

      if( attrs.mode != (mode_t) -1 )
        ::chmod( (const char *)( to.path( KUrl::RemoveTrailingSlash ).toLocal8Bit() ), attrs.mode );

#ifdef HAVE_POSIX_ACL
      if( !attrs.acl.isNull() )
      {
        acl_t acl = acl_from_text( attrs.acl.toLatin1() );
        if( acl && !acl_valid( acl ) )
          acl_set_file( to.path( KUrl::RemoveTrailingSlash ).toLocal8Bit(), ACL_TYPE_ACCESS, acl );
        if( acl )
          acl_free( acl );
      }
#endif
    }
  }
}

void PreserveAttrCopyJob::slotFinished() {
  for( unsigned i=0; i != directoriesToStamp.count(); i++ ) {
    KUrl from = originalDirectories[ i ];
    KUrl to = directoriesToStamp[ i ];

    slotCopyingDone( from, to, false );
  }
}

void KIO::PreserveAttrCopyJob::setDefaultPermissions( bool b )
{
    m_defaultPermissions = b;
}
#include "preserveattrcopyjob.moc"
