/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DUFILELIGHT_H
#define DUFILELIGHT_H

// QtGui
#include <QMouseEvent>

#include "diskusage.h"
#include "filelightParts/Config.h"
#include "radialMap/widget.h"

class DUFilelight : public RadialMap::Widget
{
    Q_OBJECT

public:
    explicit DUFilelight(DiskUsage *usage);

    File *getCurrentFile();

public slots:
    void slotDirChanged(Directory *);
    void clear();
    void slotChanged(File *);
    void slotRefresh();

protected slots:
    void slotAboutToShow(int);

    void schemeRainbow()
    {
        setScheme(Filelight::Rainbow);
    }
    void schemeHighContrast()
    {
        setScheme(Filelight::HighContrast);
    }
    void schemeKDE()
    {
        setScheme(Filelight::KDE);
    }

    void increaseContrast();
    void decreaseContrast();
    void changeAntiAlias();
    void showSmallFiles();
    void varyLabelFontSizes();
    void minFontSize();

protected:
    void mousePressEvent(QMouseEvent *) override;

    void setScheme(Filelight::MapScheme);

    DiskUsage *diskUsage;
    Directory *currentDir;

private:
    bool refreshNeeded;
};

#endif /* __DU_FILELIGHT_H__ */
