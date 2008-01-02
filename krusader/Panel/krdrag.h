/***************************************************************************
                            krdrag.h
                      -------------------
copyright            : (C) 2003 by Heiner Eichmann
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

                                              H e a d e r    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef KRDRAG_H
#define KRDRAG_H

#include <q3dragobject.h> 
//Added by qt3to4:
#include <Q3StrList>
#include <kurl.h>

class KRDrag : public Q3UriDrag
{
    Q_OBJECT
public:
    static KRDrag * newDrag( const KUrl::List & urls, bool move, QWidget * dragSource = 0 );

protected:
    KRDrag( const Q3StrList & urls, bool move, QWidget * dragSource );

public:
    virtual ~KRDrag() {}

    virtual const char* format( int i ) const;
    virtual QByteArray encodedData( const char* mime ) const;

    void setMoveSelection( bool move ) { m_bCutSelection = move; }

    // Returns true if the data was cut (used for KonqIconDrag too)
    static bool decodeIsCutSelection( const QMimeSource *e );

protected:
    bool m_bCutSelection;
    Q3StrList m_urls;
};

#endif /* KRDRAG_H */
