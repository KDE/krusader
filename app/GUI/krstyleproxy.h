/*
    SPDX-FileCopyrightText: 2008 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2008-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRSTYLEPROXY_H
#define KRSTYLEPROXY_H

// QtWidgets
#include <QProxyStyle>

class KrStyleProxy : public QProxyStyle
{
public:
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;

    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const override;
};

#endif /* KRSTYLEPROXY_H */
