/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
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
#ifndef KMOUNTMAN_H
#define KMOUNTMAN_H

// QtCore
#include <QExplicitlySharedDataPointer>
#include <QObject>
#include <QString>
#include <QPointer>
#include <QUrl>
// QtWidgets
#include <QWidget>
#include <QAction>

#include <KIO/Job>
#include <KIO/Global>
#include <KIOCore/KMountPoint>

#include <Solid/Device>
#include <Solid/SolidNamespace>

class KMountManGUI;
class KToolBarPopupAction;

class KMountMan : public QObject
{
    Q_OBJECT
    friend class KMountManGUI;

public:
    enum mntStatus {DOESNT_EXIST, NOT_MOUNTED, MOUNTED};

    inline bool operational() {
        return _operational;
    } // check this 1st

    void mount(const QString& mntPoint, bool blocking = true); // this is probably what you need for mount
    void unmount(const QString& mntPoint, bool blocking = true); // this is probably what you need for unmount
    mntStatus getStatus(const QString& mntPoint);    // return the status of a mntPoint (if any)
    void eject(const QString& mntPoint);
    bool ejectable(QString path);
    bool removable(QString path);
    bool removable(Solid::Device d);
    bool invalidFilesystem(const QString& type);
    bool networkFilesystem(const QString& type);
    bool nonmountFilesystem(const QString& type, const QString& mntPoint);
    QAction *action() {
        return (QAction *) _action;
    }

    explicit KMountMan(QWidget *parent);
    ~KMountMan();

    // NOTE: this function needs some time (~50msec)
    QString findUdiForPath(const QString& path, const Solid::DeviceInterface::Type &expType = Solid::DeviceInterface::Unknown);
    QString pathForUdi(const QString& udi);

public slots:
    void mainWindow();                        // opens up the GUI
    void autoMount(const QString& path);             // just call it before refreshing into a dir
    void delayedPerformAction(const QAction *action);
    void quickList();

protected slots:
    void jobResult(KJob *job);
    void slotTeardownDone(Solid::ErrorType error, const QVariant& errorData, const QString &udi);
    void slotSetupDone(Solid::ErrorType error, const QVariant& errorData, const QString &udi);

protected:
    // used internally
    static QExplicitlySharedDataPointer<KMountPoint> findInListByMntPoint(KMountPoint::List &lst, QString value);
    void toggleMount(const QString& mntPoint);
    void emitRefreshPanel(const QUrl &url) {
        emit refreshPanel(url);
    }

signals:
    void refreshPanel(const QUrl &);

private:
    enum ActionType { Mount, Unmount };

    KToolBarPopupAction *_action;
    QAction *_manageAction;

    bool _operational;   // if false, something went terribly wrong on startup
    bool waiting; // used to block krusader while waiting for (un)mount operation
    KMountManGUI *mountManGui;
    // the following is the FS type
    QStringList invalid_fs;
    QStringList nonmount_fs;
    QStringList network_fs;
    // the following is the FS name
    QStringList nonmount_fs_mntpoint;
    QPointer<QWidget> parentWindow;
};

#endif
