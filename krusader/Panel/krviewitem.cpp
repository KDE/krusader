/*****************************************************************************
 * Copyright (C) 2000-2002 Shie Erlich <erlich@users.sourceforge.net>        *
 * Copyright (C) 2000-2002 Rafi Yanai <yanai@users.sourceforge.net>          *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "krviewitem.h"

#include "krinterview.h"
#include "krvfsmodel.h"
#include "../FileSystem/krpermhandler.h"

// QtCore
#include <QLocale>
#include <QMimeDatabase>
#include <QMimeType>
// QtGui
#include <QPixmap>

#include <KI18n/KLocalizedString>

#define PROPS static_cast<const KrViewProperties*>(_viewProperties)

KrViewItem::KrViewItem(FileItem *fileitem, KrInterView *parentView):
        _fileitem(fileitem), _view(parentView), _viewProperties(parentView->properties()), _hasExtension(false),
        _hidden(false), _extension("")
{
    dummyFileItem = parentView->_model->dummyFileItem() == fileitem;

    if (fileitem) {
        // check if the file has an extension
        const QString& fileitemName = fileitem->getName();
        int loc = fileitemName.lastIndexOf('.');
        if (loc > 0) { // avoid mishandling of .bashrc and friend
            // check if it has one of the predefined 'atomic extensions'
            for (QStringList::const_iterator i = PROPS->atomicExtensions.begin(); i != PROPS->atomicExtensions.end(); ++i) {
                if (fileitemName.endsWith(*i)) {
                    loc = fileitemName.length() - (*i).length();
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

const QString& KrViewItem::name(bool withExtension) const
{
    if (!withExtension && _hasExtension) return _name;
    else return _fileitem->getName();
}

QString KrViewItem::description() const
{
    if (dummyFileItem)
        return i18n("Climb up the folder tree");
    // else is implied
    QString text = _fileitem->getName();
    QString comment;
    QMimeDatabase db;
    QMimeType mt = db.mimeTypeForName(_fileitem->getMime());
    if (mt.isValid())
        comment = mt.comment();
    QString myLinkDest = _fileitem->getSymDest();
    KIO::filesize_t mySize = _fileitem->getSize();

    QString text2 = text;
    mode_t m_fileMode = _fileitem->getMode();

    if (_fileitem->isSymLink()) {
        QString tmp;
        if (_fileitem->isBrokenLink())
            tmp = i18n("(Broken Link)");
        else if (comment.isEmpty())
            tmp = i18n("Symbolic Link") ;
        else
            tmp = i18n("%1 (Link)", comment);

        text += "->";
        text += myLinkDest;
        text += "  ";
        text += tmp;
    } else if (S_ISREG(m_fileMode)) {
        text = QString("%1").arg(text2) + QString(" (%1)").arg(PROPS->humanReadableSize ?
                KRpermHandler::parseSize(_fileitem->getSize()) : KIO::convertSize(mySize));
        text += "  ";
        text += comment;
    } else if (S_ISDIR(m_fileMode)) {
        text += "/  ";
        if (_fileitem->getSize() != 0) {
            text += '(' +
                    (PROPS->humanReadableSize ? KRpermHandler::parseSize(_fileitem->getSize()) : KIO::convertSize(mySize)) + ") ";
        }
        text += comment;
    } else {
        text += "  ";
        text += comment;
    }
    return text;
}

QPixmap KrViewItem::icon()
{
#if 0
    QPixmap *p;

    // This is bad - very bad. the function must return a valid reference,
    // This is an interface flow - shie please fix it with a function that return QPixmap*
    // this way we can return 0 - and do our error checking...

    // shie answers: why? what's the difference? if we return an empty pixmap, others can use it as it
    // is, without worrying or needing to do error checking. empty pixmap displays nothing
#endif
    if (dummyFileItem || !_viewProperties->displayIcons)
        return QPixmap();
    else return KrView::getIcon(_fileitem, true);
}

bool KrViewItem::isSelected() const {
    return _view->isSelected(_fileitem);
}

void KrViewItem::setSelected(bool s) {
    _view->setSelected(_fileitem, s);
    if(!_view->op()->isMassSelectionUpdate()) {
        redraw();
        _view->op()->emitSelectionChanged();
    }
}

QRect KrViewItem::itemRect() const {
    return _view->itemRect(_fileitem);
}

void KrViewItem::redraw() {
    _view->_itemView->viewport()->update(itemRect());
}
