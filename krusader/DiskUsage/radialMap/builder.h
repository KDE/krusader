/*****************************************************************************
 * Copyright (C) 2003-2004 Max Howell <max.howell@methylblue.com>            *
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

#ifndef BUILDER_H
#define BUILDER_H

#include "radialMap.h"   //Segment, defines
#include "fileTree.h"

template <class T> class Chain;

namespace RadialMap
{
    class Map;

    //temporary class that builds the Map signature

    class Builder
    {
    public:
        Builder( Map*, const Directory* const, bool fast=false );

    private:
        void findVisibleDepth( const Directory* const dir, const uint=0 );
        void setLimits( const uint& );
        bool build( const Directory* const, const uint=0, uint=0, const uint=5760 );

        Map             *m_map;
        const Directory* const m_root;
        const FileSize   m_minSize;
        uint            *m_depth;
        Chain<Segment>  *m_signature;
        FileSize        *m_limits;
    };
}

#endif
