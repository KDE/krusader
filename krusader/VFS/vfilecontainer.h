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

#ifndef VFILECONTAINER_H
#define VFILECONTAINER_H

// QtCore
#include <QObject>

class vfile;

/**
 * A minimal interface for access to the files inside a filesystem directory.
 */
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
    /// Emitted when refreshing finished. The list of vfiles should now be updated by the view.
    void refreshDone();
    void addedVfile(vfile* vf);
    void deletedVfile(const QString& name);
    void updatedVfile(vfile* vf);
    /// Emitted when all vfiles in the VFS were removed
    void cleared();
};


#endif // VFILECONTAINER_H
