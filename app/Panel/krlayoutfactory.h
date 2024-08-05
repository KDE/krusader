/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRLAYOUTFACTORY_H
#define KRLAYOUTFACTORY_H

// QtCore
#include <QHash>
#include <QString>
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
        : panel(panel)
        , widgets(widgets)
    {
    }
    // creates the layout and adds the widgets to it
    QLayout *createLayout(QString layoutName = QString());

    static QStringList layoutNames();
    static QString layoutDescription(const QString &layoutName);

private:
    QBoxLayout *createLayout(const QDomElement &e, QWidget *parent);
    QWidget *createFrame(const QDomElement &e, QWidget *parent);

    static bool parseFiles();
    static bool parseFile(const QString &path, QDomDocument &doc);
    static bool parseResource(const QString &path, QDomDocument &doc);
    static bool parseContent(const QString &content, const QString &fileName, QDomDocument &doc);
    static void getLayoutNames(const QDomDocument &doc, QStringList &names);
    static QDomElement findLayout(const QDomDocument &doc, const QString &layoutName);

    ListPanel *const panel;
    QHash<QString, QWidget *> &widgets;

    static bool _parsed;
    static QDomDocument _mainDoc;
    static QList<QDomDocument> _extraDocs;
};

#endif
