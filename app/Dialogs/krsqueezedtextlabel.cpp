/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krsqueezedtextlabel.h"

// QtGui
#include <QDragEnterEvent>
#include <QPainter>
// QtWidgets
#include <QLabel>
#include <QToolTip>

#include <KStringHandler>
#include <KUrlMimeData>

KrSqueezedTextLabel::KrSqueezedTextLabel(QWidget *parent)
    : KSqueezedTextLabel(parent)
    , _index(-1)
    , _length(-1)
{
    setAutoFillBackground(true);
}

KrSqueezedTextLabel::~KrSqueezedTextLabel() = default;

void KrSqueezedTextLabel::mousePressEvent(QMouseEvent *e)
{
    e->ignore();
    emit clicked(e);
}

void KrSqueezedTextLabel::squeezeTextToLabel(qsizetype index, qsizetype length)
{
    if (index == -1 || length == -1)
        KSqueezedTextLabel::squeezeTextToLabel();
    else {
        QString sqtext = fullText;
        QFontMetrics fm(fontMetrics());
        int labelWidth = size().width();
        int textWidth = fm.horizontalAdvance(sqtext);

        bool hasToSqueeze = false;
        qsizetype avgCharSize = 0;
        if (textWidth > labelWidth) {
            avgCharSize = textWidth / sqtext.length();
            if (avgCharSize != 0) {
                // In this case, there is no risk of division by zero when
                // dividing by avgCharSize. Note: avgCharSize is zero (because
                // sqtext.length() is bigger than textWidth) for example when
                // searching inside some binary files whose content is displayed
                // using UTF-8 characters
                hasToSqueeze = true;
            }
        }

        if (hasToSqueeze) {
            qsizetype numOfExtraChars = (textWidth - labelWidth) / avgCharSize;
            qsizetype delta;

            // remove as much as possible from the left, and then from the right
            if (index > 3) {
                delta = qMin(index, numOfExtraChars);
                numOfExtraChars -= delta;
                sqtext.replace(0, delta, "...");
            }

            if (numOfExtraChars > 0 && (static_cast<int>(sqtext.length()) > length + 3)) {
                delta = qMin(numOfExtraChars, static_cast<int>(sqtext.length()) - (length + 3));
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

void KrSqueezedTextLabel::setText(const QString &text, qsizetype index, qsizetype length)
{
    _index = index;
    _length = length;
    fullText = text;
    KSqueezedTextLabel::setText(fullText);
    squeezeTextToLabel(_index, _length);
}

void KrSqueezedTextLabel::paintEvent(QPaintEvent *e)
{
    KSqueezedTextLabel::paintEvent(e);
}
