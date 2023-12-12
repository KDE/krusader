/*
    SPDX-FileCopyrightText: 2006 Csaba Karai <cskarai@freemail.hu>
    SPDX-FileCopyrightText: 2006-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MEDIABUTTON_H
#define MEDIABUTTON_H

// QtCore
#include <QEvent>
#include <QList>
#include <QMap>
#include <QTimer>
#include <QUrl>
// QtWidgets
#include <QMenu>
#include <QToolButton>
#include <QWidget>

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
    void slotDeviceAdded(const QString &);
    void slotDeviceRemoved(const QString &);
    void showMenu();
    void slotCheckMounts();
    void updateIcon(const QString &mountPoint);

signals:
    void openUrl(const QUrl &);
    void newTab(const QUrl &);
    void aboutToShow();

protected:
    bool eventFilter(QObject *o, QEvent *e) override;
    bool getNameAndIcon(Solid::Device &, QString &, QIcon &);

private:
    void createMediaList();
    void toggleMount(const QString &udi);
    void getStatus(const QString &udi, bool &mounted, QString *mountPointOut = nullptr, bool *ejectableOut = nullptr);
    void mount(const QString &, bool open = false, bool newtab = false);
    void umount(const QString &);
    void eject(QString);
    void rightClickMenu(const QString &udi, QPoint pos);

    QList<Solid::Device> storageDevices;

private slots:
    void slotSetupDone(Solid::ErrorType error, const QVariant &errorData, const QString &udi);

private:
    static QString remotePrefix;

    QMenu *popupMenu;
    QMenu *rightMenu;
    QString udiToOpen;
    bool openInNewTab;
    QMap<QString, QString> udiNameMap;
    QTimer mountCheckerTimer;
    QString currentMountPoint; // for performance optimization
};

#endif /* MEDIABUTTON_H */
