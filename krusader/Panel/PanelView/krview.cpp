/***************************************************************************
                                 krview.cpp
                            -------------------
   copyright            : (C) 2000-2002 by Shie Erlich & Rafi Yanai
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

#include "krview.h"

#include "krselectionmode.h"
#include "krviewfactory.h"
#include "krviewitem.h"
#include "../FileSystem/dirlisterinterface.h"
#include "../FileSystem/fileitem.h"
#include "../FileSystem/krpermhandler.h"
#include "../Filter/filterdialog.h"
#include "../defaults.h"
#include "../kicons.h"
#include "../krcolorcache.h"
#include "../krglobal.h"
#include "../krpreviews.h"
#include "../viewactions.h"

// QtCore
#include <QDebug>
#include <QDir>
// QtGui
#include <QPixmapCache>
#include <QBitmap>
#include <QPainter>
#include <QPixmap>
// QtWidgets
#include <QAction>
#include <QInputDialog>
#include <QMimeDatabase>
#include <QMimeType>
#include <qnamespace.h>

#include <KConfigCore/KSharedConfig>
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>

#define FILEITEM getFileItem()

KrView *KrViewOperator::_changedView = 0;
KrViewProperties::PropertyType KrViewOperator::_changedProperties = KrViewProperties::NoProperty;


// ----------------------------- operator
KrViewOperator::KrViewOperator(KrView *view, QWidget *widget) :
        _view(view), _widget(widget), _massSelectionUpdate(false)
{
    _saveDefaultSettingsTimer.setSingleShot(true);
    connect(&_saveDefaultSettingsTimer, SIGNAL(timeout()), SLOT(saveDefaultSettings()));
}

KrViewOperator::~KrViewOperator()
{
    if(_changedView == _view)
        saveDefaultSettings();
}

void KrViewOperator::startUpdate()
{
    _view->refresh();
}

void KrViewOperator::cleared()
{
    _view->clear();
}

void KrViewOperator::fileAdded(FileItem *fileitem)
{
    _view->addItem(fileitem);
}

void KrViewOperator::fileUpdated(FileItem *newFileitem)
{
    _view->updateItem(newFileitem);
}

void KrViewOperator::startDrag()
{
    QStringList items;
    _view->getSelectedItems(&items);
    if (items.empty())
        return ; // don't drag an empty thing
    QPixmap px;
    if (items.count() > 1 || _view->getCurrentKrViewItem() == 0)
        px = FL_LOADICON("queue");   // how much are we dragging
    else
        px = _view->getCurrentKrViewItem() ->icon();
    emit letsDrag(items, px);
}

bool KrViewOperator::searchItem(const QString &text, bool caseSensitive, int direction)
{
    KrViewItem * item = _view->getCurrentKrViewItem();
    if (!item) {
        return false;
    }
    QRegExp rx(text, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::Wildcard);
    if (!direction) {
        if (rx.indexIn(item->name()) == 0) {
            return true;
        }
        direction = 1;
    }
    KrViewItem * startItem = item;
    while (true) {
        item = (direction > 0) ? _view->getNext(item) : _view->getPrev(item);
        if (!item)
            item = (direction > 0) ? _view->getFirst() : _view->getLast();
        if (item == startItem) {
            return false;
        }
        if (rx.indexIn(item->name()) == 0) {
            _view->setCurrentKrViewItem(item);
            _view->makeItemVisible(item);
            return true;
        }
    }
}

bool KrViewOperator::filterSearch(const QString &text, bool caseSensitive)
{
    _view->_quickFilterMask = QRegExp(text,
                                      caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive,
                                      QRegExp::Wildcard);
    _view->refresh();
    return _view->_count || !_view->_files->numFileItems();
}

void KrViewOperator::setMassSelectionUpdate(bool upd)
{
    _massSelectionUpdate = upd;
    if (!upd) {
        emit selectionChanged();
        _view->redraw();
    }
}

void KrViewOperator::settingsChanged(KrViewProperties::PropertyType properties)
{
    if(!_view->_updateDefaultSettings || _view->_ignoreSettingsChange)
        return;

    if(_changedView != _view)
        saveDefaultSettings();
    _changedView = _view;
    _changedProperties = static_cast<KrViewProperties::PropertyType>(_changedProperties | properties);
    _saveDefaultSettingsTimer.start(100);
}

void KrViewOperator::saveDefaultSettings()
{
    _saveDefaultSettingsTimer.stop();
    if(_changedView)
        _changedView->saveDefaultSettings(_changedProperties);
    _changedProperties = KrViewProperties::NoProperty;
    _changedView = 0;
}

// ----------------------------- krview

const KrView::IconSizes KrView::iconSizes;

KrView::KrView(KrViewInstance &instance, KConfig *cfg)
    : _config(cfg), _properties(0), _focused(false), _fileIconSize(0),
      _instance(instance), _files(0), _mainWindow(0), _widget(0), _nameToMakeCurrent(QString()),
      _previews(0), _updateDefaultSettings(false), _ignoreSettingsChange(false), _count(0),
      _numDirs(0), _dummyFileItem(0)
{
}

KrView::~KrView()
{
    _instance.m_objects.removeOne(this);
    delete _previews;
    _previews = 0;
    delete _dummyFileItem;
    _dummyFileItem = 0;
    if (_properties)
        qFatal("A class inheriting KrView didn't delete _properties!");
    if (_operator)
        qFatal("A class inheriting KrView didn't delete _operator!");
}

void KrView::init(bool enableUpdateDefaultSettings)
{
    // sanity checks:
    if (!_widget)
        qFatal("_widget must be set during construction of KrView inheritors");
    // ok, continue
    initProperties();
    _operator = createOperator();
    setup();
    restoreDefaultSettings();

    _updateDefaultSettings = enableUpdateDefaultSettings &&
        KConfigGroup(_config, "Startup").readEntry("Update Default Panel Settings", _RememberPos);

    _instance.m_objects.append(this);
}

void KrView::initProperties()
{
    const KConfigGroup grpInstance(_config, _instance.name());
    const bool displayIcons = grpInstance.readEntry("With Icons", _WithIcons);

    const KConfigGroup grpSvr(_config, "Look&Feel");
    const bool numericPermissions = grpSvr.readEntry("Numeric permissions", _NumericPermissions);

    int sortOps = 0;
    if (grpSvr.readEntry("Show Directories First", true))
        sortOps |= KrViewProperties::DirsFirst;
    if(grpSvr.readEntry("Always sort dirs by name", false))
        sortOps |=  KrViewProperties::AlwaysSortDirsByName;
    if (!grpSvr.readEntry("Case Sensative Sort", _CaseSensativeSort))
        sortOps |= KrViewProperties::IgnoreCase;
    if (grpSvr.readEntry("Locale Aware Sort", true))
        sortOps |= KrViewProperties::LocaleAwareSort;
    KrViewProperties::SortOptions sortOptions = static_cast<KrViewProperties::SortOptions>(sortOps);

    KrViewProperties::SortMethod sortMethod = static_cast<KrViewProperties::SortMethod>(
        grpSvr.readEntry("Sort method", (int)_DefaultSortMethod));
    const bool humanReadableSize = grpSvr.readEntry("Human Readable Size", _HumanReadableSize);

    // see KDE bug #40131
    const bool localeAwareCompareIsCaseSensitive = QString("a").localeAwareCompare("B") > 0;

    QStringList defaultAtomicExtensions;
    defaultAtomicExtensions += ".tar.gz";
    defaultAtomicExtensions += ".tar.bz2";
    defaultAtomicExtensions += ".tar.lzma";
    defaultAtomicExtensions += ".tar.xz";
    defaultAtomicExtensions += ".moc.cpp";
    QStringList atomicExtensions = grpSvr.readEntry("Atomic Extensions", defaultAtomicExtensions);
    for (QStringList::iterator i = atomicExtensions.begin(); i != atomicExtensions.end();) {
        QString & ext = *i;
        ext = ext.trimmed();
        if (!ext.length()) {
            i = atomicExtensions.erase(i);
            continue;
        }
        if (!ext.startsWith('.'))
            ext.insert(0, '.');
        ++i;
    }

    _properties = new KrViewProperties(displayIcons, numericPermissions, sortOptions, sortMethod,
                                       humanReadableSize, localeAwareCompareIsCaseSensitive,
                                       atomicExtensions);
}

void KrView::showPreviews(bool show)
{
    if(show) {
        if(!_previews) {
            _previews = new KrPreviews(this);
            _previews->update();
        }
    } else {
        delete _previews;
        _previews = 0;
    }
    redraw();
//     op()->settingsChanged(KrViewProperties::PropShowPreviews);
    op()->emitRefreshActions();
}

void KrView::updatePreviews()
{
    if(_previews)
        _previews->update();
}

QPixmap KrView::processIcon(const QPixmap &icon, bool dim, const QColor & dimColor, int dimFactor, bool symlink)
{
    QPixmap pixmap = icon;

    if (symlink) {
        const QStringList overlays = QStringList() << QString() << "emblem-symbolic-link";
        KIconLoader::global()->drawOverlays(overlays, pixmap, KIconLoader::Desktop);
    }

    if(!dim)
        return pixmap;

    QImage dimmed = pixmap.toImage();

    QPainter p(&dimmed);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(0, 0, icon.width(), icon.height(), dimColor);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.setOpacity((qreal)dimFactor / (qreal)100);
    p.drawPixmap(0, 0, icon.width(), icon.height(), pixmap);

    return QPixmap::fromImage(dimmed, Qt::ColorOnly | Qt::ThresholdDither |
                                Qt::ThresholdAlphaDither | Qt::NoOpaqueDetection );
}

QPixmap KrView::getIcon(FileItem *fileitem, bool active, int size/*, KRListItem::cmpColor color*/)
{
    // KConfigGroup ag( krConfig, "Advanced");
    //////////////////////////////
    QPixmap icon;
    QString icon_name = fileitem->getIcon();
    QString cacheName;

    if(!size)
        size = _FilelistIconSize.toInt();

    QColor dimColor;
    int dimFactor;
    bool dim = !active && KrColorCache::getColorCache().getDimSettings(dimColor, dimFactor);

    if (icon_name.isNull())
        icon_name = "";

    cacheName.append(QString::number(size));
    if(fileitem->isSymLink())
        cacheName.append("LINK_");
    if(dim)
        cacheName.append("DIM_");
    cacheName.append(icon_name);

    //QPixmapCache::setCacheLimit( ag.readEntry("Icon Cache Size",_IconCacheSize) );

    // first try the cache
    if (!QPixmapCache::find(cacheName, icon)) {
        icon = processIcon(krLoader->loadIcon(icon_name, KIconLoader::Desktop, size),
                           dim, dimColor, dimFactor, fileitem->isSymLink());
        // insert it into the cache
        QPixmapCache::insert(cacheName, icon);
    }

    return icon;
}

QPixmap KrView::getIcon(FileItem *fileitem)
{
    if(_previews) {
        QPixmap icon;
        if(_previews->getPreview(fileitem, icon, _focused))
            return icon;
    }
    return getIcon(fileitem, _focused, _fileIconSize);
}

/**
 * this function ADDs a list of selected item names into 'names'.
 * it assumes the list is ready and doesn't initialize it, or clears it
 */
void KrView::getItemsByMask(QString mask, QStringList* names, bool dirs, bool files)
{
    for (KrViewItem * it = getFirst(); it != 0; it = getNext(it)) {
        if ((it->name() == "..") || !QDir::match(mask, it->name())) continue;
        // if we got here, than the item fits the mask
        if (it->getFileItem()->isDir() && !dirs) continue;   // do we need to skip folders?
        if (!it->getFileItem()->isDir() && !files) continue;   // do we need to skip files
        names->append(it->name());
    }
}

/**
 * this function ADDs a list of selected item names into 'names'.
 * it assumes the list is ready and doesn't initialize it, or clears it
 */
void KrView::getSelectedItems(QStringList *names, bool fallbackToFocused)
{
    for (KrViewItem *it = getFirst(); it != 0; it = getNext(it))
        if (it->isSelected() && (it->name() != ".."))
            names->append(it->name());

    if (fallbackToFocused) {
        // if all else fails, take the current item
        const QString item = getCurrentItem();
        if (names->empty() && !item.isEmpty() && item != "..") {
            names->append(item);
        }
    }
}

void KrView::getSelectedKrViewItems(KrViewItemList *items)
{
    for (KrViewItem * it = getFirst(); it != 0; it = getNext(it))
        if (it->isSelected() && (it->name() != "..")) items->append(it);

    // if all else fails, take the current item
    QString item = getCurrentItem();
    if (items->empty() &&
            !item.isEmpty() &&
            item != ".." &&
            getCurrentKrViewItem() != 0) {
        items->append(getCurrentKrViewItem());
    }
}

QString KrView::statistics()
{
    KIO::filesize_t size = calcSize();
    KIO::filesize_t selectedSize = calcSelectedSize();
    QString tmp;
    KConfigGroup grp(_config, "Look&Feel");
    if(grp.readEntry("Show Size In Bytes", false)) {
        tmp = i18nc("%1=number of selected items,%2=total number of items, \
                    %3=filesize of selected items,%4=filesize in Bytes, \
                    %5=filesize of all items in folder,%6=filesize in Bytes",
                    "%1 out of %2, %3 (%4) out of %5 (%6)",
                    numSelected(), _count, KIO::convertSize(selectedSize),
                    KRpermHandler::parseSize(selectedSize),
                    KIO::convertSize(size),
                    KRpermHandler::parseSize(size));
    } else {
        tmp = i18nc("%1=number of selected items,%2=total number of items, \
                    %3=filesize of selected items,%4=filesize of all items in folder",
                    "%1 out of %2, %3 out of %4",
                    numSelected(), _count, KIO::convertSize(selectedSize),
                    KIO::convertSize(size));
    }

    // notify if we're running a filtered view
    if (filter() != KrViewProperties::All)
        tmp = ">> [ " + filterMask().nameFilter() + " ]  " + tmp;
    return tmp;
}

bool KrView::changeSelection(const KRQuery& filter, bool select)
{
    KConfigGroup grpSvr(_config, "Look&Feel");
    return changeSelection(filter, select, grpSvr.readEntry("Mark Dirs", _MarkDirs), true);
}

bool KrView::changeSelection(const KRQuery& filter, bool select, bool includeDirs, bool makeVisible)
{
    if (op()) op()->setMassSelectionUpdate(true);

    KrViewItem *temp = getCurrentKrViewItem();
    KrViewItem *firstMatch = 0;
    for (KrViewItem * it = getFirst(); it != 0; it = getNext(it)) {
        if (it->name() == "..")
            continue;
        if (it->getFileItem()->isDir() && !includeDirs)
            continue;

        FileItem * file = it->getMutableFileItem(); // filter::match calls getMimetype which isn't const
        if (file == 0)
            continue;

        if (filter.match(file)) {
            it->setSelected(select);
            if (!firstMatch) firstMatch = it;
        }
    }

    if (op()) op()->setMassSelectionUpdate(false);
    updateView();
    if (ensureVisibilityAfterSelect() && temp != 0) {
        makeItemVisible(temp);
    } else if (makeVisible && firstMatch != 0) {
        // if no selected item is visible...
        KrViewItemList selectedItems;
        getSelectedKrViewItems(&selectedItems);
        bool anyVisible = false;
        for (KrViewItem *item : selectedItems) {
            if (isItemVisible(item)) {
                anyVisible = true;
                break;
            }
        }
        if (!anyVisible) {
            // ...scroll to fist selected item
            makeItemVisible(firstMatch);
        }
    }
    redraw();

    return firstMatch != 0; // return if any file was selected
}

void KrView::invertSelection()
{
    if (op()) op()->setMassSelectionUpdate(true);
    KConfigGroup grpSvr(_config, "Look&Feel");
    bool markDirs = grpSvr.readEntry("Mark Dirs", _MarkDirs);

    KrViewItem *temp = getCurrentKrViewItem();
    for (KrViewItem * it = getFirst(); it != 0; it = getNext(it)) {
        if (it->name() == "..")
            continue;
        if (it->getFileItem()->isDir() && !markDirs && !it->isSelected())
            continue;
        it->setSelected(!it->isSelected());
    }
    if (op()) op()->setMassSelectionUpdate(false);
    updateView();
    if (ensureVisibilityAfterSelect() && temp != 0)
        makeItemVisible(temp);
}

QString KrView::firstUnmarkedBelowCurrent()
{
    if (getCurrentKrViewItem() == 0)
        return QString();

    KrViewItem * iterator = getNext(getCurrentKrViewItem());
    while (iterator && iterator->isSelected())
        iterator = getNext(iterator);
    if (!iterator) {
        iterator = getPrev(getCurrentKrViewItem());
        while (iterator && iterator->isSelected())
            iterator = getPrev(iterator);
    }
    if (!iterator) return QString();
    return iterator->name();
}

void KrView::delItem(const QString &name)
{
    KrViewItem *it = findItemByName(name);
    if(!it)
        return;

    if(_previews)
        _previews->deletePreview(it);

    preDelItem(it);

    if (it->FILEITEM->isDir()) {
        --_numDirs;
    }

    --_count;
    delete it;

    op()->emitSelectionChanged();
}

void KrView::addItem(FileItem *fileitem)
{
    if (isFiltered(fileitem))
        return;
    KrViewItem *item = preAddItem(fileitem);
    if (!item)
        return; // don't add it after all

    if(_previews)
        _previews->updatePreview(item);

    if (fileitem->isDir())
        ++_numDirs;

    ++_count;

    if (item->name() == nameToMakeCurrent()) {
        setCurrentKrViewItem(item); // dictionary based - quick
        makeItemVisible(item);
    }

    op()->emitSelectionChanged();
}

void KrView::updateItem(FileItem *newFileItem)
{
    // file name did not change
    const QString name = newFileItem->getName();

    // preserve 'current' and 'selection'
    const bool isCurrent = getCurrentItem() == name;
    QStringList selectedNames;
    getSelectedItems(&selectedNames, false);
    const bool isSelected = selectedNames.contains(name);

    // delete old file item
    delItem(name);

    if (!isFiltered(newFileItem)) {
        addItem(newFileItem);
        if(_previews)
            _previews->updatePreview(findItemByFileItem(newFileItem));
    }

    if (isCurrent)
        setCurrentItem(name);
    if (isSelected)
        setSelected(newFileItem, true);

    op()->emitSelectionChanged();
}

void KrView::clear()
{
    if(_previews)
        _previews->clear();
    _count = _numDirs = 0;
    delete _dummyFileItem;
    _dummyFileItem = 0;
    redraw();
}

bool KrView::handleKeyEvent(QKeyEvent *e)
{
    qDebug() << "key event=" << e;
    switch (e->key()) {
    case Qt::Key_Enter :
    case Qt::Key_Return : {
        if (e->modifiers() & Qt::ControlModifier)           // let the panel handle it
            e->ignore();
        else {
            KrViewItem * i = getCurrentKrViewItem();
            if (i == 0)
                return true;
            QString tmp = i->name();
            op()->emitExecuted(tmp);
        }
        return true;
    }
    case Qt::Key_QuoteLeft :          // Terminal Emulator bugfix
        if (e->modifiers() == Qt::ControlModifier) {   // let the panel handle it
            e->ignore();
        } else {          // a normal click - do a lynx-like moving thing
            op()->emitGoHome(); // ask krusader to move to the home directory
        }
        return true;
    case Qt::Key_Delete : // delete/trash the file (delete with alternative mode is a panel action)
        if (e->modifiers() == Qt::NoModifier) {
            op()->emitDefaultDeleteFiles();
        }
        return true;
    case Qt::Key_Insert: {
        KrViewItem * i = getCurrentKrViewItem();
        if (!i)
            return true;
        i->setSelected(!i->isSelected());
        if (KrSelectionMode::getSelectionHandler()->insertMovesDown()) {
            KrViewItem * next = getNext(i);
            if (next) {
                setCurrentKrViewItem(next);
                makeItemVisible(next);
            }
        }
        op()->emitSelectionChanged();
        return true;
    }
    case Qt::Key_Space: {
        KrViewItem * viewItem = getCurrentKrViewItem();
        if (viewItem != 0) {
            viewItem->setSelected(!viewItem->isSelected());

            if (viewItem->getFileItem()->isDir() &&
                KrSelectionMode::getSelectionHandler()->spaceCalculatesDiskSpace()) {
                op()->emitQuickCalcSpace(viewItem);
            }
            if (KrSelectionMode::getSelectionHandler()->spaceMovesDown()) {
                KrViewItem * next = getNext(viewItem);
                if (next) {
                    setCurrentKrViewItem(next);
                    makeItemVisible(next);
                }
            }
            op()->emitSelectionChanged();
        }
        return true;
    }
    case Qt::Key_Backspace :                         // Terminal Emulator bugfix
    case Qt::Key_Left :
        if (e->modifiers() == Qt::ControlModifier || e->modifiers() == Qt::ShiftModifier ||
                e->modifiers() == Qt::AltModifier) {   // let the panel handle it
            e->ignore();
        } else {          // a normal click - do a lynx-like moving thing
            op()->emitDirUp(); // ask krusader to move up a directory
        }
        return true;         // safety
    case Qt::Key_Right :
        if (e->modifiers() == Qt::ControlModifier || e->modifiers() == Qt::ShiftModifier ||
                e->modifiers() == Qt::AltModifier) {   // let the panel handle it
            e->ignore();
        } else { // just a normal click - do a lynx-like moving thing
            KrViewItem *i = getCurrentKrViewItem();
            if (i)
                op()->emitGoInside(i->name());
        }
        return true;
    case Qt::Key_Up :
        if (e->modifiers() == Qt::ControlModifier) {   // let the panel handle it - jump to the Location Bar
            e->ignore();
        } else {
            KrViewItem *item = getCurrentKrViewItem();
            if (item) {
                if (e->modifiers() == Qt::ShiftModifier) {
                    item->setSelected(!item->isSelected());
                    op()->emitSelectionChanged();
                }
                item = getPrev(item);
                if (item) {
                    setCurrentKrViewItem(item);
                    makeItemVisible(item);
                }
            }
        }
        return true;
    case Qt::Key_Down :
        if (e->modifiers() == Qt::ControlModifier || e->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {     // let the panel handle it - jump to command line
            e->ignore();
        } else {
            KrViewItem *item = getCurrentKrViewItem();
            if (item) {
                if (e->modifiers() == Qt::ShiftModifier) {
                    item->setSelected(!item->isSelected());
                    op()->emitSelectionChanged();
                }
                item = getNext(item);
                if (item) {
                    setCurrentKrViewItem(item);
                    makeItemVisible(item);
                }
            }
        }
        return true;
    case Qt::Key_Home: {
        if (e->modifiers() & Qt::ShiftModifier) {  /* Shift+Home */
            bool select = true;
            KrViewItem *pos = getCurrentKrViewItem();
            if (pos == 0)
                pos = getLast();
            KrViewItem *item = getFirst();
            op()->setMassSelectionUpdate(true);
            while (item) {
                item->setSelected(select);
                if (item == pos)
                    select = false;
                item = getNext(item);
            }
            op()->setMassSelectionUpdate(false);
        }
        KrViewItem * first = getFirst();
        if (first) {
            setCurrentKrViewItem(first);
            makeItemVisible(first);
        }
    }
    return true;
    case Qt::Key_End:
        if (e->modifiers() & Qt::ShiftModifier) {
            bool select = false;
            KrViewItem *pos = getCurrentKrViewItem();
            if (pos == 0)
                pos = getFirst();
            op()->setMassSelectionUpdate(true);
            KrViewItem *item = getFirst();
            while (item) {
                if (item == pos)
                    select = true;
                item->setSelected(select);
                item = getNext(item);
            }
            op()->setMassSelectionUpdate(false);
        } else {
            KrViewItem *last = getLast();
            if (last) {
                setCurrentKrViewItem(last);
                makeItemVisible(last);
            }
        }
        return true;
    case Qt::Key_PageDown: {
        KrViewItem * current = getCurrentKrViewItem();
        int downStep = itemsPerPage();
        while (downStep != 0 && current) {
            KrViewItem * newCurrent = getNext(current);
            if (newCurrent == 0)
                break;
            current = newCurrent;
            downStep--;
        }
        if (current) {
            setCurrentKrViewItem(current);
            makeItemVisible(current);
        }
        return true;
    }
    case Qt::Key_PageUp: {
        KrViewItem * current = getCurrentKrViewItem();
        int upStep = itemsPerPage();
        while (upStep != 0 && current) {
            KrViewItem * newCurrent = getPrev(current);
            if (newCurrent == 0)
                break;
            current = newCurrent;
            upStep--;
        }
        if (current) {
            setCurrentKrViewItem(current);
            makeItemVisible(current);
        }
        return true;
    }
    case Qt::Key_Escape:
        e->ignore();
        return true; // otherwise the selection gets lost??!??
                     // also it is needed by the panel
    case Qt::Key_A : // mark all
        if (e->modifiers() == Qt::ControlModifier) {
            //FIXME: shouldn't there also be a shortcut for unselecting everything ?
            selectAllIncludingDirs();
            return true;
        }
        [[gnu::fallthrough]];
    default:
        return false;
    }
    return false;
}

void KrView::zoomIn()
{
    int idx = iconSizes.indexOf(_fileIconSize);
    if(idx >= 0 && (idx+1) < iconSizes.count())
        setFileIconSize(iconSizes[idx+1]);
}

void KrView::zoomOut()
{
    int idx = iconSizes.indexOf(_fileIconSize);
    if(idx > 0)
        setFileIconSize(iconSizes[idx-1]);
}

void KrView::setFileIconSize(int size)
{
    if(iconSizes.indexOf(size) < 0)
        return;
    _fileIconSize = size;
    if(_previews) {
        _previews->clear();
        _previews->update();
    }
    redraw();
    op()->emitRefreshActions();
}

int KrView::defaultFileIconSize()
{
    KConfigGroup grpSvr(_config, _instance.name());
    return grpSvr.readEntry("IconSize", _FilelistIconSize).toInt();
}

void KrView::saveDefaultSettings(KrViewProperties::PropertyType properties)
{
    saveSettings(KConfigGroup(_config, _instance.name()), properties);
    op()->emitRefreshActions();
}

void KrView::restoreDefaultSettings()
{
    restoreSettings(KConfigGroup(_config, _instance.name()));
}

void KrView::saveSettings(KConfigGroup group, KrViewProperties::PropertyType properties)
{
    if(properties & KrViewProperties::PropIconSize)
        group.writeEntry("IconSize", fileIconSize());
    if(properties & KrViewProperties::PropShowPreviews)
        group.writeEntry("ShowPreviews", previewsShown());
    if(properties & KrViewProperties::PropSortMode)
        saveSortMode(group);
    if(properties & KrViewProperties::PropFilter) {
        group.writeEntry("Filter", static_cast<int>(_properties->filter));
        group.writeEntry("FilterApplysToDirs", _properties->filterApplysToDirs);
        if(_properties->filterSettings.isValid())
            _properties->filterSettings.save(KConfigGroup(&group, "FilterSettings"));
    }
}

void KrView::restoreSettings(KConfigGroup group)
{
    _ignoreSettingsChange = true;
    doRestoreSettings(group);
    _ignoreSettingsChange = false;
    refresh();
}

void KrView::doRestoreSettings(KConfigGroup group)
{
    restoreSortMode(group);
    setFileIconSize(group.readEntry("IconSize", defaultFileIconSize()));
    showPreviews(group.readEntry("ShowPreviews", false));
    _properties->filter = static_cast<KrViewProperties::FilterSpec>(group.readEntry("Filter",
                                                    static_cast<int>(KrViewProperties::All)));
    _properties->filterApplysToDirs = group.readEntry("FilterApplysToDirs", false);
    _properties->filterSettings.load(KConfigGroup(&group, "FilterSettings"));
    _properties->filterMask = _properties->filterSettings.toQuery();
}

void KrView::applySettingsToOthers()
{
    for(int i = 0; i < _instance.m_objects.length(); i++) {
        KrView *view = _instance.m_objects[i];
        if(this != view) {
            view->_ignoreSettingsChange = true;
            view->copySettingsFrom(this);
            view->_ignoreSettingsChange = false;
        }
    }
}

void KrView::sortModeUpdated(KrViewProperties::ColumnType sortColumn, bool descending)
{
    if(sortColumn == _properties->sortColumn && descending == (bool) (_properties->sortOptions & KrViewProperties::Descending))
        return;

    int options = _properties->sortOptions;
    if(descending)
        options |= KrViewProperties::Descending;
    else
        options &= ~KrViewProperties::Descending;
    _properties->sortColumn = sortColumn;
    _properties->sortOptions = static_cast<KrViewProperties::SortOptions>(options);

    //     op()->settingsChanged(KrViewProperties::PropSortMode);
}

bool KrView::drawCurrent() const
{
    return isFocused() ||
           KConfigGroup(_config, "Look&Feel")
               .readEntry("Always Show Current Item", _AlwaysShowCurrentItem);
}

void KrView::saveSortMode(KConfigGroup &group)
{
    group.writeEntry("Sort Column", static_cast<int>(_properties->sortColumn));
    group.writeEntry("Descending Sort Order", _properties->sortOptions & KrViewProperties::Descending);
}

void KrView::restoreSortMode(KConfigGroup &group)
{
    int column = group.readEntry("Sort Column", static_cast<int>(KrViewProperties::Name));
    bool isDescending = group.readEntry("Descending Sort Order", false);
    setSortMode(static_cast<KrViewProperties::ColumnType>(column), isDescending);
}

QString KrView::krPermissionText(const FileItem * fileitem)
{
    QString tmp;
    switch (fileitem->isReadable()) {
    case ALLOWED_PERM: tmp+='r'; break;
    case UNKNOWN_PERM: tmp+='?'; break;
    case NO_PERM:      tmp+='-'; break;
    }
    switch (fileitem->isWriteable()) {
    case ALLOWED_PERM: tmp+='w'; break;
    case UNKNOWN_PERM: tmp+='?'; break;
    case NO_PERM:      tmp+='-'; break;
    }
    switch (fileitem->isExecutable()) {
    case ALLOWED_PERM: tmp+='x'; break;
    case UNKNOWN_PERM: tmp+='?'; break;
    case NO_PERM:      tmp+='-'; break;
    }
    return tmp;
}

QString KrView::permissionsText(const KrViewProperties *properties, const FileItem *fileItem)
{
    return properties->numericPermissions ?
               QString().asprintf("%.4o", fileItem->getMode() & (S_ISUID | S_ISGID | S_ISVTX |
                                                                 S_IRWXU | S_IRWXG | S_IRWXO)) :
               fileItem->getPerm();
}

QString KrView::sizeText(const KrViewProperties *properties, KIO::filesize_t size)
{
    return properties->humanReadableSize ? KIO::convertSize(size) : KRpermHandler::parseSize(size);
}

QString KrView::mimeTypeText(FileItem *fileItem)
{
    QMimeType mt = QMimeDatabase().mimeTypeForName(fileItem->getMime());
    return mt.isValid() ? mt.comment() : QString();
}

bool KrView::isFiltered(FileItem *fileitem)
{
    if (_quickFilterMask.isValid() && _quickFilterMask.indexIn(fileitem->getName()) == -1)
        return true;

    bool filteredOut = false;
    bool isDir = fileitem->isDir();
    if (!isDir || (isDir && properties()->filterApplysToDirs)) {
        switch (properties()->filter) {
        case KrViewProperties::All :
            break;
        case KrViewProperties::Custom :
            if (!properties()->filterMask.match(fileitem))
                filteredOut = true;
            break;
        case KrViewProperties::Dirs:
            if (!isDir)
                filteredOut = true;
            break;
        case KrViewProperties::Files:
            if (isDir)
                filteredOut = true;
            break;
        default:
            break;
        }
    }
    return filteredOut;
}

void KrView::setFiles(DirListerInterface *files)
{
    if(files != _files) {
        clear();
        if(_files)
            QObject::disconnect(_files, 0, op(), 0);
        _files = files;
    }

    if(!_files)
        return;

    QObject::disconnect(_files, 0, op(), 0);
    QObject::connect(_files, &DirListerInterface::scanDone, op(), &KrViewOperator::startUpdate);
    QObject::connect(_files, &DirListerInterface::cleared, op(), &KrViewOperator::cleared);
    QObject::connect(_files, &DirListerInterface::addedFileItem, op(), &KrViewOperator::fileAdded);
    QObject::connect(_files, &DirListerInterface::updatedFileItem, op(), &KrViewOperator::fileUpdated);
}

void KrView::setFilter(KrViewProperties::FilterSpec filter, FilterSettings customFilter, bool applyToDirs)
{
    _properties->filter = filter;
    _properties->filterSettings = customFilter;
    _properties->filterMask = customFilter.toQuery();
    _properties->filterApplysToDirs = applyToDirs;
    refresh();
}

void KrView::setFilter(KrViewProperties::FilterSpec filter)
{

    KConfigGroup cfg(_config, "Look&Feel");
    bool rememberSettings = cfg.readEntry("FilterDialogRemembersSettings", _FilterDialogRemembersSettings);
    bool applyToDirs = rememberSettings ? _properties->filterApplysToDirs : false;
    switch (filter) {
    case KrViewProperties::All :
        break;
    case KrViewProperties::Custom :
        {
            FilterDialog dialog(_widget, i18n("Filter Files"), QStringList(i18n("Apply filter to folders")), false);
            dialog.checkExtraOption(i18n("Apply filter to folders"), applyToDirs);
            if(rememberSettings)
                dialog.applySettings(_properties->filterSettings);
            dialog.exec();
            FilterSettings s(dialog.getSettings());
            if(!s.isValid()) // if the user canceled - quit
                return;
            _properties->filterSettings = s;
            _properties->filterMask = s.toQuery();
            applyToDirs = dialog.isExtraOptionChecked(i18n("Apply filter to folders"));
        }
        break;
    default:
        return;
    }
    _properties->filterApplysToDirs = applyToDirs;
    _properties->filter = filter;
    refresh();
}

void KrView::customSelection(bool select)
{
    KConfigGroup grpSvr(_config, "Look&Feel");
    bool includeDirs = grpSvr.readEntry("Mark Dirs", _MarkDirs);

    FilterDialog dialog(0, i18n("Select Files"), QStringList(i18n("Apply selection to folders")), false);
    dialog.checkExtraOption(i18n("Apply selection to folders"), includeDirs);
    dialog.exec();
    KRQuery query = dialog.getQuery();
    // if the user canceled - quit
    if (query.isNull())
        return;
    includeDirs = dialog.isExtraOptionChecked(i18n("Apply selection to folders"));

    changeSelection(query, select, includeDirs);
}

void KrView::refresh()
{
    QString currentItem = getCurrentItem();
    QList<QUrl> selection = selectedUrls();
    QModelIndex currentIndex = getCurrentIndex();

    clear();

    if(!_files)
        return;

    QList<FileItem*> fileItems;

    // if we are not at the root add the ".." entry
    if(!_files->isRoot()) {
        _dummyFileItem = FileItem::createDummy();
        fileItems << _dummyFileItem;
    }

    foreach(FileItem *fileitem, _files->fileItems()) {
        if(!fileitem || isFiltered(fileitem))
            continue;
        if(fileitem->isDir())
            _numDirs++;
        _count++;
        fileItems << fileitem;
    }

    populate(fileItems, _dummyFileItem);

    if(!selection.isEmpty())
        setSelectionUrls(selection);

    if (!nameToMakeCurrent().isEmpty()) {
        setCurrentItem(nameToMakeCurrent());
        setNameToMakeCurrent("");
    } else if (!currentItem.isEmpty()) {
        if (currentItem == ".." && _count > 0 &&
            !_quickFilterMask.isEmpty() && _quickFilterMask.isValid()) {
            // In a filtered view we should never select the dummy entry if
            // there are real matches.
            setCurrentKrViewItem(getNext(getFirst()));
        }
        else
            setCurrentItem(currentItem, currentIndex);
    } else {
        setCurrentKrViewItem(getFirst());
    }

    updatePreviews();
    redraw();

    op()->emitSelectionChanged();
}

void KrView::setSelected(const FileItem* fileitem, bool select)
{
    if (fileitem == _dummyFileItem)
        return;

    if (select)
        clearSavedSelection();
    intSetSelected(fileitem, select);
}

void KrView::saveSelection()
{
    _savedSelection = selectedUrls();
    op()->emitRefreshActions();
}

void KrView::restoreSelection()
{
    if(canRestoreSelection())
        setSelectionUrls(_savedSelection);
}

void KrView::clearSavedSelection() {
    _savedSelection.clear();
    op()->emitRefreshActions();
}

void KrView::markSameBaseName()
{
    KrViewItem* item = getCurrentKrViewItem();
    if (!item)
        return;
    KRQuery query(QString("%1.*").arg(item->name(false)));
    changeSelection(query, true, false);
}

void KrView::markSameExtension()
{
    KrViewItem* item = getCurrentKrViewItem();
    if (!item)
        return;
    KRQuery query(QString("*.%1").arg(item->extension()));
    changeSelection(query, true, false);
}
