/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krlayoutfactory.h"

#include "../compat.h"
#include "../krglobal.h"
#include "listpanel.h"
#include "listpanelframe.h"

// QtCore
#include <QDebug>
#include <QFile>
#include <QMetaEnum>
#include <QResource>
#include <QStandardPaths>
// QtWidgets
#include <QHBoxLayout>
#include <QVBoxLayout>
// QtXml
#include <QDomDocument>

#include <KLocalizedString>
#include <KSharedConfig>

#define XMLFILE_VERSION "1.0"
#define MAIN_FILE "layout.xml"
#define MAIN_FILE_RC_PATH ":/" MAIN_FILE
#define EXTRA_FILE_MASK "layouts/*.xml"
#define DEFAULT_LAYOUT "krusader:default"

bool KrLayoutFactory::_parsed = false;
QDomDocument KrLayoutFactory::_mainDoc;
QList<QDomDocument> KrLayoutFactory::_extraDocs;

QString KrLayoutFactory::layoutDescription(const QString &layoutName)
{
    if (layoutName == DEFAULT_LAYOUT)
        return i18nc("Default layout", "Default");
    else if (layoutName == "krusader:compact")
        return i18n("Compact");
    else if (layoutName == "krusader:classic")
        return i18n("Classic");
    else
        return i18n("Custom layout: \"%1\"", layoutName);
}

bool KrLayoutFactory::parseFiles()
{
    if (_parsed)
        return true;

    _parsed = parseResource(MAIN_FILE_RC_PATH, _mainDoc);
    if (!_parsed) {
        return false;
    }

    const QStringList extraFilePaths = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, EXTRA_FILE_MASK);

    for (const QString &path : extraFilePaths) {
        qWarning() << "extra file: " << path;
        QDomDocument doc;
        if (parseFile(path, doc))
            _extraDocs << doc;
    }

    return true;
}

bool KrLayoutFactory::parseFile(const QString &path, QDomDocument &doc)
{
    bool success = false;

    QFile file(path);

    if (file.open(QIODevice::ReadOnly))
        return parseContent(file.readAll(), path, doc);
    else
        qWarning() << "can't open" << path;

    return success;
}

bool KrLayoutFactory::parseResource(const QString &path, QDomDocument &doc)
{
    QFile f(path);

    if (f.open( QIODevice::ReadOnly)) {
        QTextStream t( &f );
        t.setEncoding(QStringConverter::Utf8);
        QString data = t.readAll();
        return parseContent(data, path, doc);
    } else {
        qWarning() << "resource does not exist:" << path;
        return false;
    }
}

bool KrLayoutFactory::parseContent(const QString &content, const QString &fileName, QDomDocument &doc)
{
    bool success = false;

    QString errorMsg;
    if (doc.setContent(content, &errorMsg)) {
        QDomElement root = doc.documentElement();
        if (root.tagName() == "KrusaderLayout") {
            QString version = root.attribute("version");
            if (version == XMLFILE_VERSION)
                success = true;
            else
                qWarning() << fileName << "has wrong version" << version << "- required is" << XMLFILE_VERSION;
        } else
            qWarning() << "root.tagName() != \"KrusaderLayout\"";
    } else
        qWarning() << "error parsing" << fileName << ":" << errorMsg;

    return success;
}

void KrLayoutFactory::getLayoutNames(const QDomDocument &doc, QStringList &names)
{
    QDomElement root = doc.documentElement();

    for (QDomElement e = root.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        if (e.tagName() == "layout") {
            QString name(e.attribute("name"));
            if (!name.isEmpty() && (name != DEFAULT_LAYOUT))
                names << name;
        }
    }
}

QStringList KrLayoutFactory::layoutNames()
{
    QStringList names;
    names << DEFAULT_LAYOUT;

    if (parseFiles()) {
        getLayoutNames(_mainDoc, names);

        for (const QDomDocument &doc : std::as_const(_extraDocs))
            getLayoutNames(doc, names);
    }

    return names;
}

QDomElement KrLayoutFactory::findLayout(const QDomDocument &doc, const QString &layoutName)
{
    QDomElement root = doc.documentElement();

    for (QDomElement e = root.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        if (e.tagName() == "layout" && e.attribute("name") == layoutName)
            return e;
    }

    return QDomElement();
}

QLayout *KrLayoutFactory::createLayout(QString layoutName)
{
    if (layoutName.isEmpty()) {
        KConfigGroup cg(krConfig, "PanelLayout");
        layoutName = cg.readEntry("Layout", DEFAULT_LAYOUT);
    }
    QLayout *layout = nullptr;

    if (parseFiles()) {
        QDomElement layoutRoot;

        layoutRoot = findLayout(_mainDoc, layoutName);
        if (layoutRoot.isNull()) {
            for (const QDomDocument &doc : std::as_const(_extraDocs)) {
                layoutRoot = findLayout(doc, layoutName);
                if (!layoutRoot.isNull())
                    break;
            }
        }
        if (layoutRoot.isNull()) {
            qWarning() << "no layout with name" << layoutName << "found";
            if (layoutName != DEFAULT_LAYOUT)
                return createLayout(DEFAULT_LAYOUT);
        } else {
            layout = createLayout(layoutRoot, panel);
        }
    }

    if (layout) {
        for (auto it = widgets.constBegin(), end = widgets.constEnd(); it != end; ++it) {
            qWarning() << "widget" << it.key() << "was not added to the layout";
            it.value()->hide();
        }
    } else
        qWarning() << "couldn't load layout" << layoutName;

    return layout;
}

QBoxLayout *KrLayoutFactory::createLayout(const QDomElement &e, QWidget *parent)
{
    QBoxLayout *l = nullptr;
    bool horizontal = false;

    if (e.attribute("type") == "horizontal") {
        horizontal = true;
        l = new QHBoxLayout();
    } else if (e.attribute("type") == "vertical")
        l = new QVBoxLayout();
    else {
        qWarning() << "unknown layout type:" << e.attribute("type");
        return nullptr;
    }

    l->setSpacing(0);
    l->setContentsMargins(0, 0, 0, 0);

    for (QDomElement child = e.firstChildElement(); !child.isNull(); child = child.nextSiblingElement()) {
        if (child.tagName() == "layout") {
            if (QLayout *childLayout = createLayout(child, parent))
                l->addLayout(childLayout);
        } else if (child.tagName() == "frame") {
            QWidget *frame = createFrame(child, parent);
            l->addWidget(frame);
        } else if (child.tagName() == "widget") {
            if (QWidget *w = widgets.take(child.attribute("name")))
                l->addWidget(w);
            else
                qWarning() << "layout: no such widget:" << child.attribute("name");
        } else if (child.tagName() == "hide_widget") {
            if (QWidget *w = widgets.take(child.attribute("name")))
                w->hide();
            else
                qWarning() << "layout: no such widget:" << child.attribute("name");
        } else if (child.tagName() == "spacer") {
            if (horizontal)
                l->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
            else
                l->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
        }
    }

    return l;
}

QWidget *KrLayoutFactory::createFrame(const QDomElement &e, QWidget *parent)
{
    KConfigGroup cg(krConfig, "PanelLayout");

    QString color = cg.readEntry("FrameColor", "default");
    if (color == "default")
        color = e.attribute("color");
    else if (color == "none")
        color.clear();

    int shadow = -1, shape = -1;

    QMetaEnum shadowEnum = QFrame::staticMetaObject.enumerator(QFrame::staticMetaObject.indexOfEnumerator("Shadow"));
    QString cfgShadow = cg.readEntry("FrameShadow", "default");
    if (cfgShadow != "default")
        shadow = shadowEnum.keyToValue(cfgShadow.toLatin1().data());
    if (shadow < 0)
        shadow = shadowEnum.keyToValue(e.attribute("shadow").toLatin1().data());

    QMetaEnum shapeEnum = QFrame::staticMetaObject.enumerator(QFrame::staticMetaObject.indexOfEnumerator("Shape"));
    QString cfgShape = cg.readEntry("FrameShape", "default");
    if (cfgShape != "default")
        shape = shapeEnum.keyToValue(cfgShape.toLatin1().data());
    if (shape < 0)
        shape = shapeEnum.keyToValue(e.attribute("shape").toLatin1().data());

    ListPanelFrame *frame = new ListPanelFrame(parent, color);
    frame->setFrameStyle(shape | shadow);
    frame->setAcceptDrops(true);

    if (QLayout *l = createLayout(e, frame)) {
        l->setContentsMargins(frame->frameWidth(), frame->frameWidth(), frame->frameWidth(), frame->frameWidth());
        frame->setLayout(l);
    }

    const QPointer<ListPanel> panelPointer(panel); // 'this' is not a QObject, need to declare field as local variable for lambda
    QObject::connect(frame, &ListPanelFrame::dropped, panel, [=](QDropEvent *event) {
        // handle drop on inner widgets without own drop handling (e.g. status bar) as drop to current directory
        panelPointer->handleDrop(event, frame);
    });
    if (!color.isEmpty())
        QObject::connect(panel, &ListPanel::signalRefreshColors, frame, &ListPanelFrame::refreshColors);

    return frame;
}
