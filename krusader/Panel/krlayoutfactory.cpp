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
#include "../krglobal.h"

#include <QtXml/QDomDocument>
#include <QFile>
#include <QWidget>
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>


#define XMLFILE "krusader/layout.xml"


QDomDocument _doc("KrusaderLayout");
QDomElement _root;
bool _parsed = false;


QLayout *KrLayoutFactory::createLayout(QString layoutName, QHash<QString, QWidget*> &widgets)
{
    QLayout *layout = 0;

    if(parseFile()) {
        bool found = false;
        for(QDomElement e = _root.firstChildElement(); ! e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "layout" && e.attribute("name") == layoutName) {
                found = true;
                layout = processElement(e, widgets);
                break;
            }
        }
        if(!found)
            krOut << "no layout with name" << layoutName << "found\n";
    }

    if(layout) {
        foreach(QString name, widgets.keys())
            krOut << "widget" << name << "was not added to the layout\n";
    } else
         krOut << "couldn't load layout" << layoutName << endl;


    return layout;
}

bool KrLayoutFactory::parseFile()
{
    if(_parsed)
        return true;

    QString fileName = KStandardDirs::locate("data", XMLFILE);
    if(fileName.isEmpty()) {
        krOut << "can't locate" << XMLFILE << endl;
        return false;
    }

    QFile file(fileName);

    if (file.open(QIODevice::ReadOnly)) {
        QString errorMsg;
        if (_doc.setContent(&file, &errorMsg)) {
            _root = _doc.documentElement();
            if (_root.tagName() == "KrusaderLayout")
                _parsed = true;
            else
                krOut << "_root.tagName() != \"KrusaderLayout\"\n";
        } else
            krOut << "error parsing" << fileName << ":" << errorMsg << endl;
    } else
        krOut << "can't open" << fileName << endl;

    return _parsed;
}

QBoxLayout *KrLayoutFactory::processElement(QDomElement e, QHash<QString, QWidget*> &widgets)
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
            if(QLayout *childLayout = processElement(child, widgets))
                l->addLayout(childLayout);
        } else if(child.tagName() == "widget") {
            if(QWidget *w = widgets.take(child.text()))
                l->addWidget(w);
            else
                krOut << "layout: so such widget:" << child.text() << endl;
        } else if(child.tagName() == "spacer") {
            if(horizontal)
                l->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
            else
                l->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
        }
    }

    return l;
}
