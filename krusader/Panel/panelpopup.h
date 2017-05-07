/*****************************************************************************
 * Copyright (C) 2003 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2003 Rafi Yanai <yanai@users.sourceforge.net>               *
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

#ifndef PANELPOPUP_H
#define PANELPOPUP_H

// QtCore
#include <QPointer>
// QtGui
// QtWidgets
#include <QButtonGroup>
#include <QSplitter>
#include <QStackedWidget>
#include <QToolButton>
#include <QWidget>

#include <KCompletion/KComboBox>
#include <KConfigCore/KConfigGroup>
#include <KIO/PreviewJob>
#include <KIOFileWidgets/KImageFilePreview>

class KrSqueezedTextLabel;
class PanelViewer;
class DiskUsageViewer;
class KrFileTreeView;
class FileItem;
class KrMainWindow;

/**
 * Additional side widget showing various meta information for the current file/directories.
 */
class PanelPopup: public QWidget
{
    Q_OBJECT

    enum Parts {
        /** Folder tree view */
        Tree,
        /** Preview image for current file/directory */
        Preview,
        /** File view: show file in most appropriate, read-only editor */
        View,
        /** Disk usage for current directory structure */
        DskUsage,
        /** Dummy */
        Last = 0xFF
    };

public:
    PanelPopup(QSplitter *splitter, bool left, KrMainWindow *mainWindow);
    ~PanelPopup();
    inline int currentPage() const {
        return stack->currentWidget()->property("KrusaderWidgetId").toInt();
    }
    void saveSettings(KConfigGroup cfg) const;
    void restoreSettings(KConfigGroup cfg);
    void setCurrentPage(int);

public slots:
    void update(const FileItem *fileitem);
    void onPanelPathChange(const QUrl &url);
    void show();
    void hide();

signals:
    void selection(const QUrl &url);
    void hideMe();

protected slots:
    void tabSelected(int id);
    void treeSelection();
    void handleOpenUrlRequest(const QUrl &url);

protected:
    virtual void focusInEvent(QFocusEvent*) Q_DECL_OVERRIDE;

    bool _left;
    bool _hidden;
    KrMainWindow *_mainWindow;
    QStackedWidget *stack;
    KImageFilePreview *imageFilePreview;
    KrSqueezedTextLabel *dataLine;
    QPointer<KIO::PreviewJob> pjob;
    KrFileTreeView *tree;
    QToolButton *treeBtn, *previewBtn, *viewerBtn, *duBtn;
    QButtonGroup *btns;
    KComboBox *quickSelectCombo;
    PanelViewer *fileViewer;
    DiskUsageViewer *diskusage;
    QList<int> splitterSizes;
    QSplitter *splitter;
};

#endif
