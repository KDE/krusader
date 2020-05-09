/*****************************************************************************
 * Copyright (C) 2005 Dirk Eschler <deschler@users.sourceforge.net>          *
 * Copyright (C) 2005-2020 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#include "searchobject.h"

#include "../krservices.h"

SearchObject::SearchObject()
= default;

SearchObject::SearchObject(const QString& searchName, bool found, const QString& note)
        : _searchName(searchName),
        _found(found),
        _note(note)
{
}

SearchObject::~SearchObject()
= default;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

Application::Application()
= default;

Application::Application(const QString& searchName, bool found, const QString& appName, const QString& website, const QString& note)
        : SearchObject(searchName, found, note),
        _appName(appName),
        _website(website),
        _path(KrServices::fullPathName(appName))
{
}

Application::Application(const QString& searchName, const QString& website, bool found, const QString& note)
        : SearchObject(searchName, found, note),
        _appName(searchName),
        _website(website),
        _path(KrServices::fullPathName(searchName))
{
}

Application::~Application()
= default;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

Archiver::Archiver()
        : Application()
{
}

Archiver::Archiver(const QString& searchName, const QString& website, bool found, bool isPacker, bool isUnpacker, const QString& note)
        : Application(searchName, website, found, note),
        _isPacker(isPacker),
        _isUnpacker(isUnpacker)
{
}

Archiver::~Archiver()
= default;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

ApplicationGroup::ApplicationGroup(const QString& searchName, bool foundGroup, const QList<Application*>& apps, const QString& note)
        : SearchObject(searchName, foundGroup, note),
        _apps(apps),
        _foundGroup(foundGroup)
{
}

ApplicationGroup::~ApplicationGroup()
= default;
