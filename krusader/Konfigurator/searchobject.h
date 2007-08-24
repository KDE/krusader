/***************************************************************************
                             searchobject.h
                             -------------------
    copyright            : (C) 2005 by Dirk Eschler & Krusader Krew
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

#ifndef SEARCHOBJECT_H
#define SEARCHOBJECT_H

#include <qstring.h>
#include <q3valuevector.h>
#include "../krservices.h"

/**
@author Dirk Eschler <deschler@users.sourceforge.net>
*/
class SearchObject
{
public:
  SearchObject();
  SearchObject(const QString& name, bool found, const QString& note);
  virtual ~SearchObject();

  const QString& getSearchName() const { return _searchName; }
  const QString& getNote() const { return _note; }
  const bool getFound() const { return _found; }
  void setSearchName(const QString& s) { _searchName = s; }
  void setNote(const QString& s) { _note = s; }
  void setFound(const bool& b) { _found = b; }

protected:
  QString _searchName;
  bool _found;
  QString _note;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
@author Dirk Eschler <deschler@users.sourceforge.net>
*/
class Application : public SearchObject
{
public:
  Application();
  Application(const QString& searchName, bool found, const QString& appName, const QString& website=QString(), const QString& note=QString());
  Application(const QString& searchName, const QString& website, bool found, const QString& note=QString());
  virtual ~Application();

  const QString& getWebsite() const { return _website; }
  const QString& getAppName() const { return _appName; }
  const QString& getPath() const { return _path; }
  void setWebsite(const QString& s) { _website = s; }
  void setAppName(const QString& s) { _appName = s; }
  void setPath(const QString& s) { _path = s; }

protected:
  QString _appName;
  QString _website;
  QString _path;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
@author Dirk Eschler <deschler@users.sourceforge.net>
*/
class Archiver : public Application
{
public:
  Archiver();
  Archiver(const QString& searchName, const QString& website, bool found, bool isPacker, bool isUnpacker, const QString& note=QString());
  ~Archiver();

  const bool getIsPacker() const { return _isPacker; }
  const bool getIsUnpacker() const { return _isUnpacker; }
  void setIsPacker(const bool& b) { _isPacker = b; }
  void setIsUnpacker(const bool& b) { _isUnpacker = b; }

protected:
  bool _isPacker;
  bool _isUnpacker;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
@author Dirk Eschler <deschler@users.sourceforge.net>
*/
class ApplicationGroup : public SearchObject
{
public:
  ApplicationGroup(const QString& searchName, bool foundGroup, const Q3ValueVector<Application*>& apps, const QString& note=QString());
  ~ApplicationGroup();

  const Q3ValueVector<Application*>& getAppVec() const { return _apps; }
  const bool getFoundGroup() const { return _foundGroup; }

protected:
  Q3ValueVector<Application*> _apps;
  bool _foundGroup;
};

#endif
