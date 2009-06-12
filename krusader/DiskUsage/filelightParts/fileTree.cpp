/*****************************************************************************
 * Copyright (C) 2004 Max Howell <max.howell@methylblue.com>                 *
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

#include "fileTree.h"
#include <kglobal.h>
#include <klocale.h>
#include <QString>

//static definitions
const FileSize File::DENOMINATOR[4] = { 1ull, 1ull<<10, 1ull<<20, 1ull<<30 };
const char File::PREFIX[5][2]   = { "", "K", "M", "G", "T" };

QString
File::fullPath( const Directory *root /*= 0*/ ) const
{
    QString path;

    if( root == this ) root = 0; //prevent returning empty string when there is something we could return

    const File *d;
    
    for( d = this; d != root && d && d->parent() != 0; d = d->parent() )
    {
      if( !path.isEmpty() )
        path = '/' + path;
        
      path = d->name() + path;
    }
    
    if( d )
    {
      while( d->parent() )
        d = d->parent();
    
      if( d->directory().endsWith( '/' ) )  
        return d->directory() + path;
      else
        return d->directory() + '/' + path;
    }
    else
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
        s += locale.formatNumber( size / DENOMINATOR[ key ? key - 1 : 0 ], 0 );
        s += ' ';
        s += PREFIX[key];
        s += "B)";
    }

    return s;
}
