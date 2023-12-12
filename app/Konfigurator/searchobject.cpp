/*
    SPDX-FileCopyrightText: 2005 Dirk Eschler <deschler@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "searchobject.h"

#include "../krservices.h"

SearchObject::SearchObject() = default;

SearchObject::SearchObject(const QString &searchName, bool found, const QString &note)
    : _searchName(searchName)
    , _found(found)
    , _note(note)
{
}

SearchObject::~SearchObject() = default;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

Application::Application() = default;

Application::Application(const QString &searchName, bool found, const QString &appName, const QString &website, const QString &note)
    : SearchObject(searchName, found, note)
    , _appName(appName)
    , _website(website)
    , _path(KrServices::fullPathName(appName))
{
}

Application::Application(const QString &searchName, const QString &website, bool found, const QString &note)
    : SearchObject(searchName, found, note)
    , _appName(searchName)
    , _website(website)
    , _path(KrServices::fullPathName(searchName))
{
}

Application::~Application() = default;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

Archiver::Archiver()
    : Application()
{
}

Archiver::Archiver(const QString &searchName, const QString &website, bool found, bool isPacker, bool isUnpacker, const QString &note)
    : Application(searchName, website, found, note)
    , _isPacker(isPacker)
    , _isUnpacker(isUnpacker)
{
}

Archiver::~Archiver() = default;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

ApplicationGroup::ApplicationGroup(const QString &searchName, bool foundGroup, const QList<Application *> &apps, const QString &note)
    : SearchObject(searchName, foundGroup, note)
    , _apps(apps)
    , _foundGroup(foundGroup)
{
}

ApplicationGroup::~ApplicationGroup() = default;
