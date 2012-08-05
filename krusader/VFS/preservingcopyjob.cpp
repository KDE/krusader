/***************************************************************************
                    preservingcopyjob.cpp  -  description
                             -------------------
    copyright            : (C) 2005 + by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "preservingcopyjob.h"
#include "../defaults.h"
#include "../krglobal.h"
#include <kio/global.h>
#include <kio/jobclasses.h>
#include <kio/jobuidelegate.h>
#include <kuiserverjobtracker.h>


KIO::Job * PreservingCopyJob::createCopyJob(PreserveMode pmode, const KUrl::List& src, const KUrl& dest, KIO::CopyJob::CopyMode mode, bool /*asMethod*/, bool showProgressInfo)
{
    switch (pmode) {
    // KIO always preserves attributes

//     case PM_DEFAULT: {
//         KConfigGroup grp(krConfig, "Advanced");
//         bool preserve = grp.readEntry("PreserveAttributes", _PreserveAttributes);
//     }
// //     case PM_NONE:
//     case PM_PRESERVE_ATTR:
    default: {
        switch (mode) {
        case KIO::CopyJob::Copy:
            return KIO::copy(src, dest, showProgressInfo ? KIO::DefaultFlags : KIO::HideProgressInfo);
        case KIO::CopyJob::Move:
            return KIO::move(src, dest, showProgressInfo ? KIO::DefaultFlags : KIO::HideProgressInfo);
        case KIO::CopyJob::Link:
            return KIO::link(src, dest, showProgressInfo ? KIO::DefaultFlags : KIO::HideProgressInfo);
        default:
            return 0;
        }
    }
    }
}

