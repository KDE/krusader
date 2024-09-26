/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRSQUEEZEDTEXTLABEL_H
#define KRSQUEEZEDTEXTLABEL_H

// QtGui
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QResizeEvent>

#include <KSqueezedTextLabel>

class QMouseEvent;
class QDragEnterEvent;
class QPaintEvent;

/**
This class overloads KSqueezedTextLabel and simply adds a clicked signal,
so that users will be able to click the label and switch focus between panels.

NEW: a special setText() method allows to choose which part of the string should
     be displayed (example: make sure that search results won't be cut out)
*/
class KrSqueezedTextLabel : public KSqueezedTextLabel
{
    Q_OBJECT
public:
    explicit KrSqueezedTextLabel(QWidget *parent = nullptr);
    ~KrSqueezedTextLabel() override;

public slots:
    void setText(const QString &text, qsizetype index = -1, qsizetype length = -1);

signals:
    void clicked(QMouseEvent *); /**< emitted when someone clicks on the label */

protected:
    void resizeEvent(QResizeEvent *) override
    {
        squeezeTextToLabel(_index, _length);
    }
    void mousePressEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void squeezeTextToLabel(qsizetype index = -1, qsizetype length = -1);

    QString fullText;

private:
    qsizetype _index, _length;
};

#endif
