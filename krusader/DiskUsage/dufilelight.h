/*****************************************************************************
 * Copyright (C) 2004 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2004-2020 Krusader Krew [https://krusader.org]              *
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

#ifndef DUFILELIGHT_H
#define DUFILELIGHT_H

// QtGui
#include <QMouseEvent>

#include "diskusage.h"
#include "radialMap/widget.h"
#include "filelightParts/Config.h"

class DUFilelight : public RadialMap::Widget
{
    Q_OBJECT

public:
    explicit DUFilelight(DiskUsage *usage);

    File * getCurrentFile();

public slots:
    void slotDirChanged(Directory *);
    void clear();
    void slotChanged(File *);
    void slotRefresh();

protected slots:
    void slotAboutToShow(int);

    void schemeRainbow()        {
        setScheme(Filelight::Rainbow);
    }
    void schemeHighContrast()   {
        setScheme(Filelight::HighContrast);
    }
    void schemeKDE()            {
        setScheme(Filelight::KDE);
    }

    void increaseContrast();
    void decreaseContrast();
    void changeAntiAlias();
    void showSmallFiles();
    void varyLabelFontSizes();
    void minFontSize();

protected:
    void mousePressEvent(QMouseEvent*) override;

    void setScheme(Filelight::MapScheme);

    DiskUsage *diskUsage;
    Directory *currentDir;

private:
    bool refreshNeeded;
};

#endif /* __DU_FILELIGHT_H__ */

