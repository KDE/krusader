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

#include "viewactions.h"
#include "krviewfactory.h"
#include "krviewitem.h"
#include "krselectionmode.h"
#include "krcolorcache.h"
#include "krpreviews.h"
#include "quickfilter.h"
#include "../kicons.h"
#include "../defaults.h"
#include "../VFS/krpermhandler.h"
#include "../VFS/vfilecontainer.h"
#include "../Dialogs/krspecialwidgets.h"
#include "../Filter/filterdialog.h"

#include <qnamespace.h>
#include <qpixmapcache.h>
#include <QtCore/QDir>
#include <QtGui/QBitmap>
#include <QtGui/QPainter>
#include <QPixmap>
#include <QAction>
#include <kmimetype.h>
#include <klocale.h>
#include <kinputdialog.h>


#define VF getVfile()

KrView *KrViewOperator::_changedView = 0;
KrViewProperties::PropertyType KrViewOperator::_changedProperties = KrViewProperties::NoProperty;


// ----------------------------- operator
KrViewOperator::KrViewOperator(KrView *view, QWidget *widget) :
        _view(view), _widget(widget), _quickSearch(0), _quickFilter(0), _massSelectionUpdate(false)
{
    _saveDefaultSettingsTimer.setSingleShot(true);
    connect(&_saveDefaultSettingsTimer, SIGNAL(timeout()), SLOT(saveDefaultSettings()));
    _widget->installEventFilter(this);
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

void KrViewOperator::fileAdded(vfile *vf)
{
    _view->addItem(vf);
}

void KrViewOperator::fileDeleted(const QString& name)
{
    _view->delItem(name);
}

void KrViewOperator::fileUpdated(vfile *vf)
{
    _view->updateItem(vf);
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

void KrViewOperator::setQuickSearch(KrQuickSearch *quickSearch)
{
    _quickSearch = quickSearch;

    _quickSearch->setFocusProxy(_view->widget());

    connect(quickSearch, SIGNAL(textChanged(const QString&)), this, SLOT(quickSearch(const QString&)));
    connect(quickSearch, SIGNAL(otherMatching(const QString&, int)), this, SLOT(quickSearch(const QString& , int)));
    connect(quickSearch, SIGNAL(stop(QKeyEvent*)), this, SLOT(stopQuickSearch(QKeyEvent*)));
    connect(quickSearch, SIGNAL(process(QKeyEvent*)), this, SLOT(handleQuickSearchEvent(QKeyEvent*)));
}

void KrViewOperator::handleQuickSearchEvent(QKeyEvent * e)
{
    switch (e->key()) {
    case Qt::Key_Insert: {
        KrViewItem * item = _view->getCurrentKrViewItem();
        if (item) {
            item->setSelected(!item->isSelected());
            quickSearch(_quickSearch->text(), 1);
        }
        break;
    }
    case Qt::Key_Home: {
        KrViewItem * item = _view->getLast();
        if (item) {
            _view->setCurrentKrViewItem(_view->getLast());
            quickSearch(_quickSearch->text(), 1);
        }
        break;
    }
    case Qt::Key_End: {
        KrViewItem * item = _view->getFirst();
        if (item) {
            _view->setCurrentKrViewItem(_view->getFirst());
            quickSearch(_quickSearch->text(), -1);
        }
        break;
    }
    }
}

void KrViewOperator::quickSearch(const QString & str, int direction)
{
    KrViewItem * item = _view->getCurrentKrViewItem();
    if (!item) {
        _quickSearch->setMatch(false);
        return;
    }
    KConfigGroup grpSvr(_view->_config, "Look&Feel");
    bool caseSensitive = grpSvr.readEntry("Case Sensitive Quicksearch", _CaseSensitiveQuicksearch);
    QRegExp rx(str, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::Wildcard);
    if (!direction) {
        if (rx.indexIn(item->name()) == 0) {
            _quickSearch->setMatch(true);
            return ;
        }
        direction = 1;
    }
    KrViewItem * startItem = item;
    while (true) {
        item = (direction > 0) ? _view->getNext(item) : _view->getPrev(item);
        if (!item)
            item = (direction > 0) ? _view->getFirst() : _view->getLast();
        if (item == startItem) {
            _quickSearch->setMatch(false);
            return ;
        }
        if (rx.indexIn(item->name()) == 0) {
            _view->setCurrentKrViewItem(item);
            _view->makeItemVisible(item);
            _quickSearch->setMatch(true);
            return ;
        }
    }
}

void KrViewOperator::stopQuickSearch(QKeyEvent * e)
{
    if (_quickSearch) {
        _quickSearch->hide();
        _quickSearch->clear();
        if (e)
            _view->handleKeyEvent(e);
    }
}

void KrViewOperator::setQuickFilter(QuickFilter *quickFilter)
{
    _quickFilter = quickFilter;
    _quickFilter->lineEdit()->installEventFilter(this);
    connect(_quickFilter, SIGNAL(stop()), SLOT(stopQuickFilter()));
    connect(_quickFilter->lineEdit(), SIGNAL(textEdited(const QString&)), SLOT(quickFilterChanged(const QString&)));
    connect(_quickFilter->lineEdit(), SIGNAL(returnPressed(const QString&)), _view->widget(), SLOT(setFocus()));
}

void KrViewOperator::quickFilterChanged(const QString &text)
{
    KConfigGroup grpSvr(_view->_config, "Look&Feel");
    bool caseSensitive = grpSvr.readEntry("Case Sensitive Quicksearch", _CaseSensitiveQuicksearch);

    _view->_quickFilterMask = QRegExp(text, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::Wildcard);
    _view->refresh();
    _quickFilter->setMatch(_view->_count || !_view->_files->numVfiles());
}

void KrViewOperator::startQuickFilter()
{
    _quickFilter->show();
    _quickFilter->lineEdit()->setFocus();
}

void KrViewOperator::stopQuickFilter(bool refreshView)
{
    if(_quickFilter->lineEdit()->hasFocus())
        _widget->setFocus();
    _quickFilter->hide();
    _quickFilter->lineEdit()->clear();
    _quickFilter->setMatch(true);
    _view->_quickFilterMask = QRegExp();
    if(refreshView)
        _view->refresh();
}

void KrViewOperator::prepareForPassive()
{
    if (_quickSearch && !_quickSearch->isHidden()) {
        stopQuickSearch(0);
    }
}

bool KrViewOperator::handleKeyEvent(QKeyEvent * e)
{
    if (!_quickSearch->isHidden()) {
        _quickSearch->myKeyPressEvent(e);
        return true;
    }
    return false;
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
    if(_view->_updateDefaultSettings) {
        if(_changedView != _view)
            saveDefaultSettings();
        _changedView = _view;
        _changedProperties = static_cast<KrViewProperties::PropertyType>(_changedProperties | properties);
        _saveDefaultSettingsTimer.start(100);
    }
}

void KrViewOperator::saveDefaultSettings()
{
    _saveDefaultSettingsTimer.stop();
    if(_changedView)
        _changedView->saveDefaultSettings(_changedProperties);
    _changedProperties = KrViewProperties::NoProperty;
    _changedView = 0;
}

bool KrViewOperator::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        if(ke->key() == Qt::Key_Escape && ke->modifiers() == Qt::NoModifier) {
            if(!_quickSearch->isHidden())
                stopQuickSearch(0);
            else if(!_quickFilter->isHidden())
                stopQuickFilter();
            else
                return false;
            event->accept();
            return true;
        }
    }
    else if(event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        if(ke->key() == Qt::Key_Escape && ke->modifiers() == Qt::NoModifier &&
                (!_quickFilter->isHidden() || !_quickFilter->isHidden())) {
            event->accept();
            return true;
        }
        else if(watched == _widget && !_quickSearch->isHidden())
            return _quickSearch->shortcutOverride(ke);
    }
    return false;
}

// ----------------------------- krview

const KrView::IconSizes KrView::iconSizes;


KrView::KrView(KrViewInstance &instance, KConfig *cfg) :
    _instance(instance), _files(0), _config(cfg), _mainWindow(0), _widget(0),
    _nameToMakeCurrent(QString()), _nameToMakeCurrentIfAdded(QString()),
    _count(0), _numDirs(0), _properties(0), _focused(false),
    _previews(0), _fileIconSize(0), _updateDefaultSettings(false), _dummyVfile(0)
{
}

KrView::~KrView()
{
    _instance.m_objects.removeOne(this);
    delete _previews;
    _previews = 0;
    delete _dummyVfile;
    _dummyVfile = 0;
    if (_properties)
        qFatal("A class inheriting KrView didn't delete _properties!");
    if (_operator)
        qFatal("A class inheriting KrView didn't delete _operator!");
}

void KrView::init()
{
    // sanity checks:
    if (!_widget)
        qFatal("_widget must be set during construction of KrView inheritors");
    // ok, continue
    initProperties();
    _operator = createOperator();
    setup();
    restoreDefaultSettings();
    KConfigGroup grp(_config, _instance.name());
    enableUpdateDefaultSettings(true);
    _instance.m_objects.append(this);
}

void KrView::initProperties()
{
    _properties = createViewProperties();

    KConfigGroup grpSvr(_config, "Look&Feel");
    KConfigGroup grpInstance(_config, _instance.name());

    _properties->displayIcons = grpInstance.readEntry("With Icons", _WithIcons);
    _properties->numericPermissions = grpSvr.readEntry("Numeric permissions", _NumericPermissions);

    int sortOptions = _properties->sortOptions;
    if (grpSvr.readEntry("Show Directories First", true))
        sortOptions |= KrViewProperties::DirsFirst;
    if(grpSvr.readEntry("Always sort dirs by name", false))
        sortOptions |=  KrViewProperties::AlwaysSortDirsByName;
    if (!grpSvr.readEntry("Case Sensative Sort", _CaseSensativeSort))
        sortOptions |= KrViewProperties::IgnoreCase;
    if (grpSvr.readEntry("Locale Aware Sort", true))
        sortOptions |= KrViewProperties::LocaleAwareSort;
    _properties->sortOptions = static_cast<KrViewProperties::SortOptions>(sortOptions);

    _properties->sortMethod = static_cast<KrViewProperties::SortMethod>(
                                  grpSvr.readEntry("Sort method", (int) _DefaultSortMethod));
    _properties->humanReadableSize = grpSvr.readEntry("Human Readable Size", _HumanReadableSize);

    _properties->localeAwareCompareIsCaseSensitive = QString("a").localeAwareCompare("B") > 0;     // see KDE bug #40131
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
    _properties->atomicExtensions = atomicExtensions;
}

void KrView::enableUpdateDefaultSettings(bool enable)
{
    if(enable) {
        KConfigGroup grpStartup(_config, "Startup");
        _updateDefaultSettings = grpStartup.readEntry("Update Default Panel Settings", _RememberPos)
                                        || grpStartup.readEntry("UI Save Settings", _UiSave);
    } else
        _updateDefaultSettings  = false;
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
        QPixmap link(link_xpm);
        QPainter painter(&pixmap);
        painter.drawPixmap(0, icon.height() - 11, link, 0, 21, 10, 11);
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

QPixmap KrView::getIcon(vfile *vf, bool active, int size/*, KRListItem::cmpColor color*/)
{
    // KConfigGroup ag( krConfig, "Advanced");
    //////////////////////////////
    QPixmap icon;
    QString icon_name = vf->vfile_getIcon();
    QString cacheName;

    if(!size)
        size = _FilelistIconSize.toInt();

    QColor dimColor;
    int dimFactor;
    bool dim = !active && KrColorCache::getColorCache().getDimSettings(dimColor, dimFactor);

    if (icon_name.isNull())
        icon_name = "";

    cacheName.append(QString::number(size));
    if(vf->vfile_isSymLink())
        cacheName.append("LINK_");
    if(dim)
        cacheName.append("DIM_");
    cacheName.append(icon_name);

    //QPixmapCache::setCacheLimit( ag.readEntry("Icon Cache Size",_IconCacheSize) );

    // first try the cache
    if (!QPixmapCache::find(cacheName, icon)) {
        icon = processIcon(krLoader->loadIcon(icon_name, KIconLoader::Desktop, size),
                           dim, dimColor, dimFactor, vf->vfile_isSymLink());
        // insert it into the cache
        QPixmapCache::insert(cacheName, icon);
    }

    return icon;
}

QPixmap KrView::getIcon(vfile *vf)
{
    if(_previews) {
        QPixmap icon;
        if(_previews->getPreview(vf, icon, _focused))
            return icon;
    }
    return getIcon(vf, _focused, _fileIconSize);
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
        if (it->getVfile()->vfile_isDir() && !dirs) continue;   // do we need to skip folders?
        if (!it->getVfile()->vfile_isDir() && !files) continue;   // do we need to skip files
        names->append(it->name());
    }
}

/**
 * this function ADDs a list of selected item names into 'names'.
 * it assumes the list is ready and doesn't initialize it, or clears it
 */
void KrView::getSelectedItems(QStringList *names)
{
    for (KrViewItem * it = getFirst(); it != 0; it = getNext(it))
        if (it->isSelected() && (it->name() != "..")) names->append(it->name());

    // if all else fails, take the current item
    QString item = getCurrentItem();
    if (names->empty() && !item.isEmpty() && item != "..") {
        names->append(item);
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
    if(grp.readEntry("Show Size In Bytes", true)) {
        tmp = i18nc("%1=number of selected items,%2=total number of items, \
                    %3=filesize of selected items,%4=filesize in Bytes, \
                    %5=filesize of all items in directory,%6=filesize in Bytes",
                    "%1 out of %2, %3 (%4) out of %5 (%6)",
                    numSelected(), _count, KIO::convertSize(selectedSize),
                    KRpermHandler::parseSize(selectedSize),
                    KIO::convertSize(size),
                    KRpermHandler::parseSize(size));
    } else {
        tmp = i18nc("%1=number of selected items,%2=total number of items, \
                    %3=filesize of selected items,%4=filesize of all items in directory",
                    "%1 out of %2, %3 out of %4",
                    numSelected(), _count, KIO::convertSize(selectedSize),
                    KIO::convertSize(size));
    }

    // notify if we're running a filtered view
    if (filter() != KrViewProperties::All)
        tmp = ">> [ " + filterMask().nameFilter() + " ]  " + tmp;
    return tmp;
}

void KrView::changeSelection(const KRQuery& filter, bool select)
{
    KConfigGroup grpSvr(_config, "Look&Feel");
    changeSelection(filter, select, grpSvr.readEntry("Mark Dirs", _MarkDirs));
}

void KrView::changeSelection(const KRQuery& filter, bool select, bool includeDirs)
{
    if (op()) op()->setMassSelectionUpdate(true);

    KrViewItem *temp = getCurrentKrViewItem();
    for (KrViewItem * it = getFirst(); it != 0; it = getNext(it)) {
        if (it->name() == "..")
            continue;
        if (it->getVfile()->vfile_isDir() && !includeDirs)
            continue;

        vfile * file = it->getMutableVfile(); // filter::match calls getMimetype which isn't const
        if (file == 0)
            continue;

        if (filter.match(file))
            it->setSelected(select);
    }

    if (op()) op()->setMassSelectionUpdate(false);
    updateView();
    if (ensureVisibilityAfterSelect() && temp != 0)
        makeItemVisible(temp);
    redraw();
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
        if (it->getVfile()->vfile_isDir() && !markDirs && !it->isSelected())
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

    if (it->VF->vfile_isDir()) {
        --_numDirs;
    }

    --_count;
    delete it;

    op()->emitSelectionChanged();
}

void KrView::addItem(vfile *vf)
{
    if (isFiltered(vf))
        return;
    KrViewItem *item = preAddItem(vf);
    if (!item) 
        return; // don't add it after all

    if(_previews)
        _previews->updatePreview(item);

    if (vf->vfile_isDir())
        ++_numDirs;

    ++_count;

    if (item->name() == nameToMakeCurrent()) {
        setCurrentKrViewItem(item); // dictionary based - quick
        makeItemVisible(item);
    }
    if (item->name() == nameToMakeCurrentIfAdded()) {
        setCurrentKrViewItem(item);
        setNameToMakeCurrentIfAdded(QString());
        makeItemVisible(item);
    }

    op()->emitSelectionChanged();
}

void KrView::updateItem(vfile *vf)
{
    if (isFiltered(vf))
        delItem(vf->vfile_getName());
    else {
        preUpdateItem(vf);
        if(_previews)
            _previews->updatePreview(findItemByVfile(vf));
    }

    op()->emitSelectionChanged();
}

void KrView::clear()
{
    if(_previews)
        _previews->clear();
    _count = _numDirs = 0;
    delete _dummyVfile;
    _dummyVfile = 0;
    redraw();
}

// good old dialog box
void KrView::renameCurrentItem()
{
    QString newName, fileName;

    KrViewItem *it = getCurrentKrViewItem();
    if (it) fileName = it->name();
    else return ; // quit if no current item available

    // don't allow anyone to rename ..
    if (fileName == "..") return ;

    bool ok = false;
    newName = KInputDialog::getText(i18n("Rename"), i18n("Rename %1 to:", fileName),
                                    fileName, &ok, _mainWindow);
    // if the user canceled - quit
    if (!ok || newName == fileName)
        return ;
    op()->emitRenameItem(it->name(), newName);
}

bool KrView::handleKeyEvent(QKeyEvent *e)
{
    if (op()->handleKeyEvent(e))
        return true;
    bool res = handleKeyEventInt(e);

    // emit the new item description
    KrViewItem * current = getCurrentKrViewItem();
    if (current != 0) {
        QString desc = current->description();
        op()->emitItemDescription(desc);
    }

    return res;
}

bool KrView::handleKeyEventInt(QKeyEvent *e)
{
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
    case Qt::Key_Delete :                   // kill file
        op()->emitDeleteFiles(e->modifiers() == Qt::ShiftModifier || e->modifiers() == Qt::ControlModifier);
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

            if (viewItem->name() != ".." && viewItem->getVfile()->vfile_isDir() && viewItem->getVfile()->vfile_getSize() <= 0 &&
                    KrSelectionMode::getSelectionHandler()->spaceCalculatesDiskSpace()) {
                op()->emitCalcSpace(viewItem);
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
    case Qt::Key_A :                 // mark all
        if (e->modifiers() == Qt::ControlModifier) {
            //FIXME: shouldn't there also be a shortcut for unselecting everything ?
            selectAllIncludingDirs();
            return true;
        }
        // default continues here !!!!!!!!!!!
    default:
        // if the key is A..Z or 1..0 do quick search otherwise...
        if (e->text().length() > 0 && e->text()[ 0 ].isPrint())    // better choice. Otherwise non-ascii characters like  can not be the first character of a filename
            // are we doing quicksearch? if not, send keys to panel
            //if ( _config->readBoolEntry( "Do Quicksearch", _DoQuicksearch ) ) {
            // are we using krusader's classic quicksearch, or wincmd style?
        {
            KConfigGroup grpSv(_config, "Look&Feel");
            if (!grpSv.readEntry("New Style Quicksearch", _NewStyleQuicksearch))
                return false;
            else {
                // first, show the quicksearch if its hidden
                if (op()->quickSearch()->isHidden()) {
                    op()->quickSearch()->show();
                    // HACK: if the pressed key requires a scroll down, the selected
                    // item is "below" the quick search window, as the icon view will
                    // realize its new size after the key processing. The following line
                    // will resize the icon view immediately.
                    // ACTIVE_PANEL->gui->layout->activate();
                    // UPDATE: it seems like this isn't needed anymore, in case I'm wrong
                    // do something like op()->emitQuickSearchStartet()
                    // -----------------------------
                }
                // now, send the key to the quicksearch
                op()->quickSearch()->myKeyPressEvent(e);
                return true;
            }
        } else {
            if (!op()->quickSearch()->isHidden()) {
                op()->quickSearch()->hide();
                op()->quickSearch()->clear();
            }
        }
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
    bool tmp = _updateDefaultSettings;
    _updateDefaultSettings = false;
    doRestoreSettings(group);
    _updateDefaultSettings = tmp;
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
            bool tmp = view->_updateDefaultSettings;
            view->_updateDefaultSettings = false;
            _instance.m_objects[i]->copySettingsFrom(this);
            view->_updateDefaultSettings = tmp;
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

QString KrView::krPermissionString(const vfile * vf)
{
    QString tmp;
    switch (vf->vfile_isReadable()) {
    case ALLOWED_PERM: tmp+='r'; break;
    case UNKNOWN_PERM: tmp+='?'; break;
    case NO_PERM:      tmp+='-'; break;
    }
    switch (vf->vfile_isWriteable()) {
    case ALLOWED_PERM: tmp+='w'; break;
    case UNKNOWN_PERM: tmp+='?'; break;
    case NO_PERM:      tmp+='-'; break;
    }
    switch (vf->vfile_isExecutable()) {
    case ALLOWED_PERM: tmp+='x'; break;
    case UNKNOWN_PERM: tmp+='?'; break;
    case NO_PERM:      tmp+='-'; break;
    }
    return tmp;
}

bool KrView::isFiltered(vfile *vf)
{
    if (_quickFilterMask.isValid() && _quickFilterMask.indexIn(vf->vfile_getName()) == -1)
        return true;

    bool filteredOut = false;
    bool isDir = vf->vfile_isDir();
    if (!isDir || (isDir && properties()->filterApplysToDirs)) {
        switch (properties()->filter) {
        case KrViewProperties::All :
            break;
        case KrViewProperties::Custom :
            if (!properties()->filterMask.match(vf))
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

void KrView::setFiles(VfileContainer *files)
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
    QObject::connect(_files, SIGNAL(startUpdate()), op(), SLOT(startUpdate()));
    QObject::connect(_files, SIGNAL(cleared()), op(), SLOT(cleared()));
    QObject::connect(_files, SIGNAL(addedVfile(vfile*)), op(), SLOT(fileAdded(vfile*)));
    QObject::connect(_files, SIGNAL(updatedVfile(vfile*)), op(), SLOT(fileUpdated(vfile*)));
    QObject::connect(_files, SIGNAL(deletedVfile(const QString&)), op(), SLOT(fileDeleted(const QString&)));
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
            FilterDialog dialog(_widget, i18n("Filter Files"), QStringList(i18n("Apply filter to directories")), false);
            dialog.checkExtraOption(i18n("Apply filter to directories"), applyToDirs);
            if(rememberSettings)
                dialog.applySettings(_properties->filterSettings);
            dialog.exec();
            FilterSettings s(dialog.getSettings());
            if(!s.isValid()) // if the user canceled - quit
                return;
            _properties->filterSettings = s;
            _properties->filterMask = s.toQuery();
            applyToDirs = dialog.isExtraOptionChecked(i18n("Apply filter to directories"));
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

    FilterDialog dialog(0, i18n("Select Files"), QStringList(i18n("Apply selection to directories")), false);
    dialog.checkExtraOption(i18n("Apply selection to directories"), includeDirs);
    dialog.exec();
    KRQuery query = dialog.getQuery();
    // if the user canceled - quit
    if (query.isNull())
        return ;
    includeDirs = dialog.isExtraOptionChecked(i18n("Apply selection to directories"));

    changeSelection(query, select, includeDirs);
}

void KrView::refresh()
{
    QString current = getCurrentItem();
    KUrl::List selection = selectedUrls();

    clear();

    if(!_files)
        return;

    QList<vfile*> vfiles;

    // if we are not at the root add the ".." entery
    if(!_files->isRoot()) {
        _dummyVfile = new vfile("..", 0, "drwxrwxrwx", 0, false, false, 0, 0, "", "", 0, -1);
        _dummyVfile->vfile_setIcon("go-up");
        vfiles << _dummyVfile;
    }

    foreach(vfile *vf, _files->vfiles()) {
        if(!vf || isFiltered(vf))
            continue;
        if(vf->vfile_isDir())
            _numDirs++;
        _count++;
        vfiles << vf;
    }

    populate(vfiles, _dummyVfile);

    if(!selection.isEmpty())
        setSelection(selection);

    if (!nameToMakeCurrent().isEmpty())
        setCurrentItem(nameToMakeCurrent());
    else if (!current.isEmpty())
        setCurrentItem(current);

    updatePreviews();
    redraw();

    op()->emitSelectionChanged();
}

void KrView::setSelected(const vfile* vf, bool select)
{
    if(vf == _dummyVfile)
        return;

    if(select)
        clearSavedSelection();
    intSetSelected(vf, select);
}

void KrView::saveSelection()
{
    _savedSelection = selectedUrls();
    op()->emitRefreshActions();
}

void KrView::restoreSelection()
{
    if(canRestoreSelection())
        setSelection(_savedSelection);
}

void KrView::clearSavedSelection() {
    _savedSelection.clear();
    op()->emitRefreshActions();
}
