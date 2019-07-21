/*****************************************************************************
 * Copyright (C) 2006 Csaba Karai <cskarai@freemail.hu>                      *
 * Copyright (C) 2006-2019 Krusader Krew [https://krusader.org]              *
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

#ifndef MEDIABUTTON_H
#define MEDIABUTTON_H

// QtCore
#include <QEvent>
#include <QList>
#include <QMap>
#include <QTimer>
#include <QUrl>
// QtWidgets
#include <QWidget>
#include <QToolButton>
#include <QMenu>

#include <Solid/Device>
#include <Solid/SolidNamespace>

class QMenu;

class MediaButton : public QToolButton
{
    Q_OBJECT
public:
    explicit MediaButton(QWidget *parent = nullptr);
    ~MediaButton() override;

public slots:
    void slotAboutToShow();
    void slotAboutToHide();
    void slotPopupActivated(QAction *);
    void slotAccessibilityChanged(bool, const QString &);
    void slotDeviceAdded(const QString&);
    void slotDeviceRemoved(const QString&);
    void showMenu();
    void slotCheckMounts();
    void updateIcon(const QString &mountPoint);

signals:
    void openUrl(const QUrl&);
    void newTab(const QUrl&);
    void aboutToShow();

protected:
    bool eventFilter(QObject *o, QEvent *e) Q_DECL_OVERRIDE;
    bool getNameAndIcon(Solid::Device &, QString &, QIcon &);

private:
    void createMediaList();
    void toggleMount(const QString& udi);
    void getStatus(const QString& udi, bool &mounted, QString *mountPointOut = nullptr, bool *ejectableOut = nullptr);
    void mount(const QString&, bool open = false, bool newtab = false);
    void umount(const QString&);
    void eject(QString);
    void rightClickMenu(const QString& udi, QPoint pos);

    QList<Solid::Device> storageDevices;

private slots:
    void slotSetupDone(Solid::ErrorType error, const QVariant& errorData, const QString &udi);

private:
    static QString remotePrefix;

    QMenu  *popupMenu;
    QMenu  *rightMenu;
    QString udiToOpen;
    bool    openInNewTab;
    QMap<QString, QString> udiNameMap;
    QTimer      mountCheckerTimer;
    QString currentMountPoint; // for performance optimization
};

#endif /* MEDIABUTTON_H */
