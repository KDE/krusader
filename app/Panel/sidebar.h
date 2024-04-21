/*
    SPDX-FileCopyrightText: 2003 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SIDEBAR_H
#define SIDEBAR_H

// QtCore
#include <QPointer>
// QtWidgets
#include <QButtonGroup>
#include <QStackedWidget>
#include <QToolButton>
#include <QWidget>

#include <KConfigGroup>
#include <KIO/PreviewJob>
#include <KImageFilePreview>

class KrSqueezedTextLabel;
class PanelViewer;
class DiskUsageViewer;
class KrFileTreeView;
class FileItem;

/**
 * Additional side widget showing various meta information for the current file/directories.
 */
class Sidebar : public QWidget
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
    explicit Sidebar(QWidget *parent);
    ~Sidebar() override;
    inline int currentPage() const
    {
        return stack->currentWidget()->property("KrusaderWidgetId").toInt();
    }
    void saveSettings(const KConfigGroup &cfg) const;
    void restoreSettings(const KConfigGroup &cfg);
    void setCurrentPage(int);

public slots:
    void update(const FileItem *fileitem);
    void onPanelPathChange(const QUrl &url);
    void show();
    void hide();

signals:
    void urlActivated(const QUrl &url);
    void hideMe();

protected slots:
    void tabSelected(int id);
    void handleOpenUrlRequest(const QUrl &url);

protected:
    void focusInEvent(QFocusEvent *) override;

    bool _hidden;
    QStackedWidget *stack;
    KImageFilePreview *imageFilePreview;
    KrSqueezedTextLabel *dataLine;
    QPointer<KIO::PreviewJob> pjob;
    KrFileTreeView *tree;
    QToolButton *treeBtn, *previewBtn, *viewerBtn, *duBtn;
    QButtonGroup *btns;
    PanelViewer *fileViewer;
    DiskUsageViewer *diskusage;
};

#endif
