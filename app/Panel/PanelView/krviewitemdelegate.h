/*
    SPDX-FileCopyrightText: 2009 Csaba Karai <cskarai@freemail.hu>
    SPDX-FileCopyrightText: 2009-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRVIEWITEMDELEGATE_H
#define KRVIEWITEMDELEGATE_H

// QtWidgets
#include <QItemDelegate>

class KrViewItemDelegate : public QItemDelegate
{
public:
    explicit KrViewItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &sovi, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool eventFilter(QObject *object, QEvent *event) override;

    /// Set the next file name selection in the editor.
    void cycleEditorSelection();

private:
    mutable int _currentlyEdited;
    mutable bool _dontDraw;
    mutable QWidget *_editor;

    /// Init editor-related members when editor is closed.
    void onEditorClose()
    {
        _currentlyEdited = -1;
        _editor = nullptr;
    }
};

#endif
