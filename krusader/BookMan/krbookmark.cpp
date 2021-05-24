/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2020 Krusader Krew [https://krusader.org]

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krbookmark.h"
#include "../krglobal.h"
#include "../icon.h"
#include "../Archive/krarchandler.h"
#include "../FileSystem/krtrashhandler.h"
#include "../Panel/listpanelactions.h"

#include <KI18n/KLocalizedString>
#include <KXmlGui/KActionCollection>
#include <utility>

#define BM_NAME(X)  (QString("Bookmark:")+X)

static const char* NAME_TRASH = I18N_NOOP("Trash bin");
static const char* NAME_VIRTUAL = I18N_NOOP("Virtual Filesystem");
static const char* NAME_LAN = I18N_NOOP("Local Network");

KrBookmark::KrBookmark(const QString& name, QUrl url, KActionCollection *parent, const QString& iconName, const QString& actionName) :
        QAction(parent), _url(std::move(url)), _iconName(iconName), _folder(false), _separator(false), _autoDelete(true)
{
    QString actName = actionName.isNull() ? BM_NAME(name) : BM_NAME(actionName);
    setText(name);
    parent->addAction(actName, this);
    connect(this, &KrBookmark::triggered, this, &KrBookmark::activatedProxy);

    setIconName(iconName);
}

KrBookmark::KrBookmark(const QString& name, const QString& iconName) :
        QAction(Icon(iconName), name, nullptr), _iconName(iconName), _folder(true), _separator(false), _autoDelete(false)
{
    setIcon(Icon(iconName == "" ? "folder" : iconName));
}

KrBookmark::~KrBookmark()
{
    if (_autoDelete) {
        QListIterator<KrBookmark *> it(_children);
        while (it.hasNext())
            delete it.next();
        _children.clear();
    }
}

void KrBookmark::setIconName(const QString& iconName)
{
    if (!iconName.isEmpty()) {
        setIcon(Icon(iconName));
    } else if (_url.isLocalFile()) {
        setIcon(Icon("folder"));
    } else if (KrArcHandler::isArchive(_url)) {
        setIcon(Icon("application-x-tar"));
    } else {
        setIcon(Icon("folder-html"));
    }
}

KrBookmark * KrBookmark::getExistingBookmark(const QString& actionName, KActionCollection *collection)
{
    return dynamic_cast<KrBookmark*>(collection->action(BM_NAME(actionName)));
}

KrBookmark * KrBookmark::trash(KActionCollection *collection)
{
    KrBookmark *bm = getExistingBookmark(i18n(NAME_TRASH), collection);
    if (!bm)
        bm = new KrBookmark(i18n(NAME_TRASH), QUrl("trash:/"), collection);

    bm->setIcon(Icon(KrTrashHandler::trashIconName()));
    return bm;
}

KrBookmark * KrBookmark::virt(KActionCollection *collection)
{
    KrBookmark *bm = getExistingBookmark(i18n(NAME_VIRTUAL), collection);
    if (!bm) {
        bm = new KrBookmark(i18n(NAME_VIRTUAL), QUrl("virt:/"), collection);
        bm->setIcon(Icon("document-open-remote"));
    }
    return bm;
}

KrBookmark * KrBookmark::lan(KActionCollection *collection)
{
    KrBookmark *bm = getExistingBookmark(i18n(NAME_LAN), collection);
    if (!bm) {
        bm = new KrBookmark(i18n(NAME_LAN), QUrl("remote:/"), collection);
        bm->setIcon(Icon("network-workgroup"));
    }
    return bm;
}

QAction * KrBookmark::jumpBackAction(KActionCollection *collection, bool isSetter, ListPanelActions *sourceActions)
{
    auto actionName = isSetter ? QString("setJumpBack") : QString("jumpBack");
    auto action = collection->action(actionName);
    if (action) {
        return action;
    }

    if (!sourceActions) {
        return nullptr;
    }

    // copy essential part of source action
    auto sourceAction = isSetter ? sourceActions->actSetJumpBack : sourceActions->actJumpBack;
    action = new QAction(sourceAction->icon(), sourceAction->text(), sourceAction);
    action->setShortcut(sourceAction->shortcut());
    action->setShortcutContext(Qt::WidgetShortcut);
    connect(action, &QAction::triggered, sourceAction, &QAction::trigger);
    // ensure there are no accelerator keys coming from another menu
    action->setText(KLocalizedString::removeAcceleratorMarker(action->text()));

    collection->addAction(actionName, action);
    return action;
}

KrBookmark * KrBookmark::separator()
{
    KrBookmark *bm = new KrBookmark("");
    bm->_separator = true;
    bm->_folder = false;
    return bm;
}


void KrBookmark::activatedProxy()
{
    emit activated(url());
}

