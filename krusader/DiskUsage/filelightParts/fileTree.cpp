//Author: Max Howell <max.howell@methylblue.com>, (C) 2004
//Copyright: See COPYING file that comes with this distribution

#include "fileTree.h"
#include <kglobal.h>
#include <klocale.h>
#include <qstring.h>


//static definitions
const FileSize File::DENOMINATOR[4] = { 1ull<<0, 1ull<<10, 1ull<<20, 1ull<<30 };
const char File::PREFIX[5][3]   = { "", "Ki", "Mi", "Gi", "Ti" };


QString
File::fullPath( const Directory *root /*= 0*/ ) const
{
    QString path;

    if( root == this ) root = 0; //prevent returning empty string when there is something we could return

    for( const Directory *d = (Directory*)this; d != root && d; d = d->parent() )
    {
      if( !path.isEmpty() )
        path += "/";
        
      path += d->fileName();
    }

    return path;
}

QString
File::humanReadableSize( UnitPrefix key /*= mega*/ ) const //FIXME inline
{
    return humanReadableSize( m_size, key );
}

QString
File::humanReadableSize( FileSize size, UnitPrefix key /*= mega*/ ) //static
{
    QString s;
    double prettySize = (double)size / (double)DENOMINATOR[key];
    const KLocale &locale = *KGlobal::locale();

    if( prettySize >= 0.01 )
    {
        if( prettySize < 1 )        s = locale.formatNumber( prettySize, 2 );
        else if( prettySize < 100 ) s = locale.formatNumber( prettySize, 1 );
        else                        s = locale.formatNumber( prettySize, 0 );

        s += ' ';
        s += PREFIX[key];
        s += 'B';
    }

    if( prettySize < 0.1 )
    {
        s += " (";
        s += locale.formatNumber( size / DENOMINATOR[key - 1], 0 );
        s += ' ';
        s += PREFIX[key];
        s += "B)";
    }

    return s;
}
