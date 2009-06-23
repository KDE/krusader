/*****************************************************************************
 * Copyright (C) 2005 Dirk Eschler <deschler@users.sourceforge.net>          *
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

#ifndef SEARCHOBJECT_H
#define SEARCHOBJECT_H

#include <QtCore/QString>
#include <QtCore/QList>
#include "../krservices.h"

class SearchObject
{
public:
    SearchObject();
    SearchObject(const QString& name, bool found, const QString& note);
    virtual ~SearchObject();

    const QString& getSearchName() const {
        return _searchName;
    }
    const QString& getNote() const {
        return _note;
    }
    const bool getFound() const {
        return _found;
    }
    void setSearchName(const QString& s) {
        _searchName = s;
    }
    void setNote(const QString& s) {
        _note = s;
    }
    void setFound(const bool& b) {
        _found = b;
    }

protected:
    QString _searchName;
    bool _found;
    QString _note;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class Application : public SearchObject
{
public:
    Application();
    Application(const QString& searchName, bool found, const QString& appName, const QString& website = QString(), const QString& note = QString());
    Application(const QString& searchName, const QString& website, bool found, const QString& note = QString());
    virtual ~Application();

    const QString& getWebsite() const {
        return _website;
    }
    const QString& getAppName() const {
        return _appName;
    }
    const QString& getPath() const {
        return _path;
    }
    void setWebsite(const QString& s) {
        _website = s;
    }
    void setAppName(const QString& s) {
        _appName = s;
    }
    void setPath(const QString& s) {
        _path = s;
    }

protected:
    QString _appName;
    QString _website;
    QString _path;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class Archiver : public Application
{
public:
    Archiver();
    Archiver(const QString& searchName, const QString& website, bool found, bool isPacker, bool isUnpacker, const QString& note = QString());
    ~Archiver();

    const bool getIsPacker() const {
        return _isPacker;
    }
    const bool getIsUnpacker() const {
        return _isUnpacker;
    }
    void setIsPacker(const bool& b) {
        _isPacker = b;
    }
    void setIsUnpacker(const bool& b) {
        _isUnpacker = b;
    }

protected:
    bool _isPacker;
    bool _isUnpacker;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class ApplicationGroup : public SearchObject
{
public:
    ApplicationGroup(const QString& searchName, bool foundGroup, const QList<Application*>& apps, const QString& note = QString());
    ~ApplicationGroup();

    const QList<Application*>& getAppVec() const {
        return _apps;
    }
    const bool getFoundGroup() const {
        return _foundGroup;
    }

protected:
    QList<Application*> _apps;
    bool _foundGroup;
};

#endif
