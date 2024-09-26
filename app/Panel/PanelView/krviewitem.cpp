/*
    SPDX-FileCopyrightText: 2000-2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krviewitem.h"

#include "../FileSystem/fileitem.h"
#include "../FileSystem/krpermhandler.h"
#include "krinterview.h"
#include "listmodel.h"

// QtCore
#include <QLocale>
#include <QMimeDatabase>
#include <QMimeType>
// QtGui
#include <QPixmap>

#include <KLocalizedString>

KrViewItem::KrViewItem(FileItem *fileitem, KrInterView *parentView)
    : _fileitem(fileitem)
    , _view(parentView)
    , _viewProperties(parentView->properties())
    , _hasExtension(false)
    , _hidden(false)
    , _extension("")
{
    dummyFileItem = parentView->_model->dummyFileItem() == fileitem;

    if (fileitem) {
        // check if the file has an extension
        const QString &fileitemName = fileitem->getName();
        qsizetype loc = fileitemName.lastIndexOf('.');
        if (loc > 0) { // avoid mishandling of .bashrc and friend
            // check if it has one of the predefined 'atomic extensions'
            for (const auto &atomicExtension : _viewProperties->atomicExtensions) {
                if (fileitemName.endsWith(atomicExtension)) {
                    loc = fileitemName.length() - atomicExtension.length();
                    break;
                }
            }
            _name = fileitemName.left(loc);
            _extension = fileitemName.mid(loc + 1);
            _hasExtension = true;
        }

        if (fileitemName.startsWith('.'))
            _hidden = true;
    }
}

const QString &KrViewItem::name(bool withExtension) const
{
    if (!withExtension && _hasExtension)
        return _name;
    else
        return _fileitem->getName();
}

QString KrViewItem::description() const
{
    if (dummyFileItem)
        return i18n("Climb up the folder tree");

    // else is implied

    QString mimeTypeComment;
    QMimeType mt = QMimeDatabase().mimeTypeForName(_fileitem->getMime());
    if (mt.isValid())
        mimeTypeComment = mt.comment();

    QString text = _fileitem->getName();
    if (_fileitem->isSymLink()) {
        text += " -> " + _fileitem->getSymDest() + "  ";
        if (_fileitem->isBrokenLink())
            text += i18n("(Broken Link)");
        else if (mimeTypeComment.isEmpty())
            text += i18n("Symbolic Link");
        else
            text += i18n("%1 (Link)", mimeTypeComment);

    } else {
        if (_fileitem->isDir())
            text += '/';

        if (_fileitem->getUISize() != (KIO::filesize_t)-1) {
            const QString size = KrView::sizeText(_viewProperties, _fileitem->getUISize());
            text += QString("  (%1)").arg(size);
        }

        text += "  " + mimeTypeComment;
    }
    return text;
}

QPixmap KrViewItem::icon()
{
    if (dummyFileItem || !_viewProperties->displayIcons)
        return QPixmap();
    else
        return KrView::getIcon(_fileitem, true);
}

bool KrViewItem::isSelected() const
{
    return _view->isSelected(_fileitem);
}

void KrViewItem::setSelected(bool s)
{
    _view->setSelected(_fileitem, s);
    if (!_view->op()->isMassSelectionUpdate()) {
        redraw();
        _view->op()->emitSelectionChanged();
    }
}

QRect KrViewItem::itemRect() const
{
    return _view->itemRect(_fileitem);
}

void KrViewItem::redraw()
{
    _view->_itemView->viewport()->update(itemRect());
}

void KrViewItem::setSize(KIO::filesize_t size)
{
    _fileitem->setSize(size);
}
