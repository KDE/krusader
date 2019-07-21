/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <krusader@users.sourceforge.net>            *
 * Copyright (C) 2010-2019 Krusader Krew [https://krusader.org]              *
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

#ifndef KRLAYOUTFACTORY_H
#define KRLAYOUTFACTORY_H

// QtCore
#include <QString>
#include <QHash>
// QtWidgets
#include <QBoxLayout>
#include <QLayout>
#include <QWidget>
// QtXml
#include <QDomElement>

class ListPanel;


class KrLayoutFactory
{
public:
    KrLayoutFactory(ListPanel *panel, QHash<QString, QWidget *> &widgets)
        : panel(panel), widgets(widgets)
    {}
    // creates the layout and adds the widgets to it
    QLayout *createLayout(QString layoutName = QString());

    static QStringList layoutNames();
    static QString layoutDescription(const QString& layoutName);

private:
    QBoxLayout *createLayout(const QDomElement& e, QWidget *parent);
    QWidget *createFrame(const QDomElement& e, QWidget *parent);

    static bool parseFiles();
    static bool parseFile(const QString& path, QDomDocument &doc);
    static bool parseResource(const QString& path, QDomDocument &doc);
    static bool parseContent(const QByteArray& content, const QString& fileName, QDomDocument &doc);
    static void getLayoutNames(const QDomDocument& doc, QStringList &names);
    static QDomElement findLayout(const QDomDocument& doc, const QString& layoutName);

    ListPanel *panel;
    QHash<QString, QWidget*> &widgets;

    static bool _parsed;
    static QDomDocument _mainDoc;
    static QList<QDomDocument> _extraDocs;
};

#endif
