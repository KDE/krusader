/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dufilelight.h"
#include "radialMap/radialMap.h"

// QtGui
#include <QMouseEvent>
#include <QPixmap>
// QtWidgets
#include <QInputDialog>
#include <QMenu>

#include <KLocalizedString>

#define SCHEME_POPUP_ID 6730

DUFilelight::DUFilelight(DiskUsage *usage)
    : RadialMap::Widget(usage)
    , diskUsage(usage)
    , currentDir(nullptr)
    , refreshNeeded(true)
{
    //     setFocusPolicy(Qt::StrongFocus);

    connect(diskUsage, &DiskUsage::enteringDirectory, this, &DUFilelight::slotDirChanged);
    connect(diskUsage, &DiskUsage::clearing, this, &DUFilelight::clear);
    connect(diskUsage, &DiskUsage::changed, this, &DUFilelight::slotChanged);
    connect(diskUsage, &DiskUsage::deleted, this, &DUFilelight::slotChanged);
    connect(diskUsage, &DiskUsage::changeFinished, this, &DUFilelight::slotRefresh);
    connect(diskUsage, &DiskUsage::deleteFinished, this, &DUFilelight::slotRefresh);
    connect(diskUsage, &DiskUsage::currentChanged, this, &DUFilelight::slotAboutToShow);
}

void DUFilelight::slotDirChanged(Directory *dir)
{
    if (diskUsage->currentWidget() != this)
        return;

    if (currentDir != dir) {
        currentDir = dir;

        invalidate(false);
        create(dir);
        refreshNeeded = false;
    }
}

void DUFilelight::clear()
{
    invalidate(false);
    currentDir = nullptr;
}

File *DUFilelight::getCurrentFile()
{
    const RadialMap::Segment *focus = focusSegment();

    if (!focus || focus->isFake() || focus->file() == currentDir)
        return nullptr;

    return const_cast<File *>(focus->file());
}

void DUFilelight::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        File *file = nullptr;

        const RadialMap::Segment *focus = focusSegment();

        if (focus && !focus->isFake() && focus->file() != currentDir)
            file = const_cast<File *>(focus->file());

        QMenu filelightPopup;
        filelightPopup.addAction(i18n("Zoom In"), Qt::Key_Plus, this, SLOT(zoomIn()));
        filelightPopup.addAction(i18n("Zoom Out"), Qt::Key_Minus, this, SLOT(zoomOut()));

        QMenu schemePopup;
        schemePopup.addAction(i18n("Rainbow"), this, SLOT(schemeRainbow()));
        schemePopup.addAction(i18n("High Contrast"), this, SLOT(schemeHighContrast()));
        schemePopup.addAction(i18n("KDE"), this, SLOT(schemeKDE()));

        filelightPopup.addMenu(&schemePopup);
        schemePopup.setTitle(i18n("Scheme"));

        filelightPopup.addAction(i18n("Increase contrast"), this, SLOT(increaseContrast()));
        filelightPopup.addAction(i18n("Decrease contrast"), this, SLOT(decreaseContrast()));

        QAction *act = filelightPopup.addAction(i18n("Use anti-aliasing"), this, SLOT(changeAntiAlias()));
        act->setCheckable(true);
        act->setChecked(Filelight::Config::antiAliasFactor > 1);

        act = filelightPopup.addAction(i18n("Show small files"), this, SLOT(showSmallFiles()));
        act->setCheckable(true);
        act->setChecked(Filelight::Config::showSmallFiles);

        act = filelightPopup.addAction(i18n("Vary label font sizes"), this, SLOT(varyLabelFontSizes()));
        act->setCheckable(true);
        act->setChecked(Filelight::Config::varyLabelFontSizes);

        filelightPopup.addAction(i18n("Minimum font size"), this, SLOT(minFontSize()));

        diskUsage->rightClickMenu(event->globalPosition().toPoint(), file, &filelightPopup, i18n("Filelight"));
        return;
    } else if (event->button() == Qt::LeftButton) {
        const RadialMap::Segment *focus = focusSegment();

        if (focus && !focus->isFake() && focus->file() == currentDir) {
            diskUsage->dirUp();
            return;
        } else if (focus && !focus->isFake() && focus->file()->isDir()) {
            // NOTE: unsafe removable of constness here when casting
            diskUsage->changeDirectory(static_cast<Directory *>(const_cast<File *>(focus->file())));
            return;
        }
    }

    RadialMap::Widget::mousePressEvent(event);
}

void DUFilelight::setScheme(Filelight::MapScheme scheme)
{
    Filelight::Config::scheme = scheme;
    Filelight::Config::write();
    slotRefresh();
}

void DUFilelight::increaseContrast()
{
    if ((Filelight::Config::contrast += 10) > 100)
        Filelight::Config::contrast = 100;

    Filelight::Config::write();
    slotRefresh();
}

void DUFilelight::decreaseContrast()
{
    if ((Filelight::Config::contrast -= 10) > 100)
        Filelight::Config::contrast = 0;

    Filelight::Config::write();
    slotRefresh();
}

void DUFilelight::changeAntiAlias()
{
    Filelight::Config::antiAliasFactor = 1 + (Filelight::Config::antiAliasFactor == 1);
    Filelight::Config::write();
    slotRefresh();
}

void DUFilelight::showSmallFiles()
{
    Filelight::Config::showSmallFiles = !Filelight::Config::showSmallFiles;
    Filelight::Config::write();
    slotRefresh();
}

void DUFilelight::varyLabelFontSizes()
{
    Filelight::Config::varyLabelFontSizes = !Filelight::Config::varyLabelFontSizes;
    Filelight::Config::write();
    slotRefresh();
}

void DUFilelight::minFontSize()
{
    bool ok = false;

    int result = QInputDialog::getInt(this, i18n("Krusader::Filelight"), i18n("Minimum font size"), (int)Filelight::Config::minFontPitch, 1, 100, 1, &ok);

    if (ok) {
        Filelight::Config::minFontPitch = (uint)result;

        Filelight::Config::write();
        slotRefresh();
    }
}

void DUFilelight::slotAboutToShow(int ndx)
{
    QWidget *widget = diskUsage->widget(ndx);
    if (widget == this && (diskUsage->getCurrentDir() != currentDir || refreshNeeded)) {
        refreshNeeded = false;
        if ((currentDir = diskUsage->getCurrentDir()) != nullptr) {
            invalidate(false);
            create(currentDir);
        }
    }
}

void DUFilelight::slotRefresh()
{
    if (diskUsage->currentWidget() != this)
        return;

    refreshNeeded = false;
    if (currentDir && currentDir == diskUsage->getCurrentDir()) {
        invalidate(false);
        create(currentDir);
    }
}

void DUFilelight::slotChanged(File *)
{
    if (!refreshNeeded)
        refreshNeeded = true;
}
