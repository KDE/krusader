/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <dehtris@yahoo.de>                          *
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

#ifndef _VFILECONTAINER_H__
#define _VFILECONTAINER_H__

#include <QObject>

class vfile;

class VfileContainer : public QObject
{
    Q_OBJECT
public:
    VfileContainer(QObject *parent) : QObject(parent) {}
    virtual ~VfileContainer() {}

    virtual QList<vfile*> vfiles() = 0;
    virtual unsigned long numVfiles() = 0;
    virtual bool isRoot() = 0;

signals:
    void startUpdate(); // emitted when the container has refreshed its list of vfiles.
    void addedVfile(vfile* vf);
    void deletedVfile(const QString& name);
    void updatedVfile(vfile* vf);
    void cleared();
};


#endif // _VFILECONTAINER_H__
