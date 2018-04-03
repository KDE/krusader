/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
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

#include "krsqueezedtextlabel.h"

// QtGui
#include <QDragEnterEvent>
#include <QPainter>
// QtWidgets
#include <QToolTip>
#include <QLabel>

#include <KCoreAddons/KUrlMimeData>
#include <KCoreAddons/KStringHandler>

KrSqueezedTextLabel::KrSqueezedTextLabel(QWidget *parent):
        KSqueezedTextLabel(parent), _index(-1), _length(-1)
{
    setAutoFillBackground(true);
}


KrSqueezedTextLabel::~KrSqueezedTextLabel()
{
}

void KrSqueezedTextLabel::mousePressEvent(QMouseEvent *e)
{
    e->ignore();
    emit clicked(e);
}

void KrSqueezedTextLabel::squeezeTextToLabel(int index, int length)
{
    if (index == -1 || length == -1)
        KSqueezedTextLabel::squeezeTextToLabel();
    else {
        QString sqtext = fullText;
        QFontMetrics fm(fontMetrics());
        int labelWidth = size().width();
        int textWidth = fm.width(sqtext);
        if (textWidth > labelWidth) {
            int avgCharSize = textWidth / sqtext.length();
            int numOfExtraChars = (textWidth - labelWidth) / avgCharSize;
            int delta;

            // remove as much as possible from the left, and then from the right
            if (index > 3) {
                delta = qMin(index, numOfExtraChars);
                numOfExtraChars -= delta;
                sqtext.replace(0, delta, "...");
            }

            if (numOfExtraChars > 0 && ((int)sqtext.length() > length + 3)) {
                delta = qMin(numOfExtraChars, (int)sqtext.length() - (length + 3));
                sqtext.replace(sqtext.length() - delta, delta, "...");
            }
            QLabel::setText(sqtext);

            setToolTip(QString());
            setToolTip(fullText);
        } else {
            QLabel::setText(fullText);

            setToolTip(QString());
            QToolTip::hideText();
        }
    }
}

void KrSqueezedTextLabel::setText(const QString &text, int index, int length)
{
    _index = index;
    _length = length;
    fullText = text;
    KSqueezedTextLabel::setText(fullText);
    squeezeTextToLabel(_index, _length);
}

void KrSqueezedTextLabel::paintEvent(QPaintEvent * e)
{
    KSqueezedTextLabel::paintEvent(e);
}


