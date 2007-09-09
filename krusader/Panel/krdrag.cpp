/***************************************************************************
                           krdrag.cpp
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

#include "krdrag.h"
#include <k3urldrag.h>
//Added by qt3to4:
#include <Q3StrList>
#include <Q3CString>

KRDrag * KRDrag::newDrag( const KUrl::List & urls, bool move, QWidget * dragSource, const char* name )
{
    // See K3URLDrag::newDrag
    Q3StrList uris;
    KUrl::List::ConstIterator uit = urls.begin();
    KUrl::List::ConstIterator uEnd = urls.end();
    // Get each URL encoded in utf8 - and since we get it in escaped
    // form on top of that, .toLatin1() is fine.
    for ( ; uit != uEnd ; ++uit )
        uris.append( K3URLDrag::urlToString( *uit ).toLatin1() );
    return new KRDrag( uris, move, dragSource, name );
}

KRDrag::KRDrag( const Q3StrList & urls, bool move, QWidget * dragSource, const char* name )
  : Q3UriDrag( urls, dragSource, name ),
    m_bCutSelection( move ), m_urls( urls )
{}

const char* KRDrag::format( int i ) const
{
    if ( i == 0 )
        return "text/uri-list";
    else if ( i == 1 )
        return "application/x-kde-cutselection";
    else if ( i == 2 )
        return "text/plain";
    else return 0;
}

QByteArray KRDrag::encodedData( const char* mime ) const
{
    QByteArray a;
    Q3CString mimetype( mime );
    if ( mimetype == "text/uri-list" )
        return Q3UriDrag::encodedData( mime );
    else if ( mimetype == "application/x-kde-cutselection" ) {
        Q3CString s ( m_bCutSelection ? "1" : "0" );
        a.resize( s.length() + 1 ); // trailing zero
        memcpy( a.data(), s.data(), s.length() + 1 );
    }
    else if ( mimetype == "text/plain" )
    {
      QStringList uris;
      for (Q3StrListIterator it(m_urls); *it; ++it)
          uris.append(K3URLDrag::stringToUrl(*it).prettyUrl());
      Q3CString s = uris.join( "\n" ).local8Bit();
      if( uris.count() > 1 )
          s.append( "\n" );
      a.resize( s.length() + 1 ); // trailing zero
      memcpy( a.data(), s.data(), s.length() + 1 );
    }
    return a;
}

//

// Used for KonqIconDrag too

bool KRDrag::decodeIsCutSelection( const QMimeSource *e )
{
  QByteArray a = e->encodedData( "application/x-kde-cutselection" );
  if ( a.isEmpty() )
    return false;
  else
  {
    return (a.at(0) == '1'); // true if "1", or similar
  }
 }
 
#include "krdrag.moc"
