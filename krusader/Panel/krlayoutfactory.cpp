/***************************************************************************
                            krlayoutfactory.cpp
                         -------------------
copyright            : (C) 2010 by Jan Lepper
e-mail               : krusader@users.sourceforge.net
web site             : http://krusader.sourceforge.net
---------------------------------------------------------------------------
Description
***************************************************************************

A

 db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
 88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
 88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
 88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
 88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
 YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                 S o u r c e    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "krlayoutfactory.h"

#include "listpanelframe.h"
#include "../krglobal.h"

#include <QMetaEnum>
#include <QtXml/QDomDocument>
#include <QFile>
#include <QWidget>
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QResource>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>


#define XMLFILE_VERSION "1.0"
#define MAIN_FILE "layout.xml"
#define MAIN_FILE_PATH "krusader/" MAIN_FILE
#define MAIN_FILE_RC_PATH ":/" MAIN_FILE
#define EXTRA_FILE_MASK "krusader/layouts/*.xml"
#define DEFAULT_LAYOUT "krusader:default"


bool KrLayoutFactory::_parsed = false;
QDomDocument KrLayoutFactory::_mainDoc;
QList<QDomDocument> KrLayoutFactory::_extraDocs;


QString KrLayoutFactory::layoutDescription(QString layoutName)
{
    if(layoutName == DEFAULT_LAYOUT)
        return i18n("Default");
    else if(layoutName == "krusader:compact")
        return i18n("Compact");
    else if(layoutName == "krusader:classic")
        return i18n("Classic");
    else
        return i18n("Custom layout: \"%1\"", layoutName);
}

bool KrLayoutFactory::parseFiles()
{
    if (_parsed)
        return true;

    QString mainFilePath = KStandardDirs::locate("data", MAIN_FILE_PATH);

    if (!mainFilePath.isEmpty())
        _parsed = parseFile(mainFilePath, _mainDoc);
    else
        krOut << "can't locate" << MAIN_FILE_PATH << endl;

    if (!_parsed)
        _parsed = parseRessource(MAIN_FILE_RC_PATH, _mainDoc);

    if (!_parsed)
        return false;

    QStringList extraFilePaths = KGlobal::dirs()->findAllResources("data", EXTRA_FILE_MASK);

    foreach(QString path, extraFilePaths) {
        krOut << "extra file: " << path << endl;
        QDomDocument doc;
        if (parseFile(path, doc))
            _extraDocs << doc;
    }

    return true;
}

bool KrLayoutFactory::parseFile(QString path, QDomDocument &doc)
{
    bool success = false;

    QFile file(path);

    if (file.open(QIODevice::ReadOnly))
        return parseContent(file.readAll(), path, doc);
    else
        krOut << "can't open" << path << endl;

    return success;
}

bool KrLayoutFactory::parseRessource(QString path, QDomDocument &doc)
{
    QResource res(path);
    if (res.isValid()) {
        QByteArray data;
        if (res.isCompressed())
            data = qUncompress(res.data(), res.size());
        else
            data = QByteArray(reinterpret_cast<const char*>(res.data()), res.size());
        return parseContent(data, path, doc);
    } else {
        krOut << "resource does not exist:" << path;
        return false;
    }
}

bool KrLayoutFactory::parseContent(QByteArray content, QString fileName, QDomDocument &doc)
{
    bool success = false;

    QString errorMsg;
    if (doc.setContent(content, &errorMsg)) {
        QDomElement root = doc.documentElement();
        if (root.tagName() == "KrusaderLayout") {
            QString version = root.attribute("version");
            if(version == XMLFILE_VERSION)
                success = true;
            else
                krOut << fileName << "has wrong version" << version << "- required is" << XMLFILE_VERSION << endl;
        } else
            krOut << "root.tagName() != \"KrusaderLayout\"\n";
    } else
        krOut << "error parsing" << fileName << ":" << errorMsg << endl;

    return success;
}

void KrLayoutFactory::getLayoutNames(QDomDocument doc, QStringList &names)
{
    QDomElement root = doc.documentElement();

    for(QDomElement e = root.firstChildElement(); ! e.isNull(); e = e.nextSiblingElement()) {
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

        foreach(QDomDocument doc, _extraDocs)
            getLayoutNames(doc, names);
    }

    return names;
}

QDomElement KrLayoutFactory::findLayout(QDomDocument doc, QString layoutName)
{
    QDomElement root = doc.documentElement();

    for(QDomElement e = root.firstChildElement(); ! e.isNull(); e = e.nextSiblingElement()) {
        if (e.tagName() == "layout" && e.attribute("name") == layoutName)
            return e;
    }

    return QDomElement();
}

QLayout *KrLayoutFactory::createLayout(QString layoutName)
{
    if(layoutName.isEmpty()) {
        KConfigGroup cg(krConfig, "PanelLayout");
        layoutName = cg.readEntry("Layout", DEFAULT_LAYOUT);
    }
    QLayout *layout = 0;

    if (parseFiles()) {
        QDomElement layoutRoot;

        layoutRoot = findLayout(_mainDoc, layoutName);
        if (layoutRoot.isNull()) {
            foreach(QDomDocument doc, _extraDocs) {
                layoutRoot = findLayout(doc, layoutName);
                if(!layoutRoot.isNull())
                    break;
            }
        }
        if (layoutRoot.isNull()) {
            krOut << "no layout with name" << layoutName << "found\n";
            if(layoutName != DEFAULT_LAYOUT)
                return createLayout(DEFAULT_LAYOUT);
        } else {
            layout = createLayout(layoutRoot, panel);
        }
    }

    if(layout) {
        foreach(QString name, widgets.keys()) {
            krOut << "widget" << name << "was not added to the layout\n";
            widgets[name]->hide();
        }
    } else
         krOut << "couldn't load layout" << layoutName << endl;


    return layout;
}

QBoxLayout *KrLayoutFactory::createLayout(QDomElement e, QWidget *parent)
{
    QBoxLayout *l = 0;
    bool horizontal = false;

    if(e.attribute("type") == "horizontal") {
        horizontal = true;
        l = new QHBoxLayout();
    }
    else if(e.attribute("type") == "vertical")
        l = new QVBoxLayout();
    else {
        krOut << "unknown layout type:" << e.attribute("type") << endl;
        return 0;
    }

    l->setSpacing(0);
    l->setContentsMargins(0, 0, 0, 0);

    for(QDomElement child = e.firstChildElement(); ! child.isNull(); child = child.nextSiblingElement()) {
        if (child.tagName() == "layout") {
            if(QLayout *childLayout = createLayout(child, parent))
                l->addLayout(childLayout);
        } else if(child.tagName() == "frame") {
            QWidget *frame = createFrame(child, parent);
            l->addWidget(frame);
        } else if(child.tagName() == "widget") {
            if(QWidget *w = widgets.take(child.attribute("name")))
                l->addWidget(w);
            else
                krOut << "layout: so such widget:" << child.attribute("name") << endl;
        } else if(child.tagName() == "hide_widget") {
            if(QWidget *w = widgets.take(child.attribute("name")))
                w->hide();
            else
                krOut << "layout: so such widget:" << child.attribute("name") << endl;
        } else if(child.tagName() == "spacer") {
            if(horizontal)
                l->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
            else
                l->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
        }
    }

    return l;
}

QWidget *KrLayoutFactory::createFrame(QDomElement e, QWidget *parent)
{
    KConfigGroup cg(krConfig, "PanelLayout");

    QString color = cg.readEntry("FrameColor", "default");
    if(color == "default")
        color = e.attribute("color");
    else if(color == "none")
        color.clear();

    int shadow = -1, shape = -1;

    QMetaEnum shadowEnum = QFrame::staticMetaObject.enumerator(QFrame::staticMetaObject.indexOfEnumerator("Shadow"));
    QString cfgShadow = cg.readEntry("FrameShadow", "default");
    if(cfgShadow != "default")
        shadow = shadowEnum.keyToValue(cfgShadow.toAscii().data());
    if(shadow < 0)
        shadow = shadowEnum.keyToValue(e.attribute("shadow").toAscii().data());

    QMetaEnum shapeEnum = QFrame::staticMetaObject.enumerator(QFrame::staticMetaObject.indexOfEnumerator("Shape"));
    QString cfgShape = cg.readEntry("FrameShape", "default");
    if(cfgShape!= "default")
        shape = shapeEnum.keyToValue(cfgShape.toAscii().data());
    if(shape < 0)
        shape = shapeEnum.keyToValue(e.attribute("shape").toAscii().data());

    QFrame *frame = new ListPanelFrame(parent, color);
    frame->setFrameStyle(shape | shadow);
    frame->setAcceptDrops(true);

    if(QLayout *l = createLayout(e, frame)) {
        l->setContentsMargins(frame->frameWidth(), frame->frameWidth(), frame->frameWidth(), frame->frameWidth());
        frame->setLayout(l);
    }

    QObject::connect(frame, SIGNAL(dropped(QDropEvent*, QWidget*)), panel, SLOT(handleDropOnView(QDropEvent*, QWidget*)));
    if(!color.isEmpty())
        QObject::connect(panel, SIGNAL(refreshColors(bool)), frame, SLOT(refreshColors(bool)));

    return frame;
}
