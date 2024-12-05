/*
    SPDX-FileCopyrightText: 2000-2002 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRVIEW_H
#define KRVIEW_H

// QtCore
#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QRegExp>
#include <QTimer>
#include <QVariant>
// QtGui
#include <QDropEvent>
#include <QPixmap>
#include <utility>

#include "krviewproperties.h"

class KrView;
class KrViewItem;
class KrPreviews;
class KrViewInstance;
class DirListerInterface;

typedef QList<KrViewItem *> KrViewItemList;

// operator can handle two ways of doing things:
// 1. if the view is a widget (inherits krview and klistview for example)
// 2. if the view HAS A widget (a krview-son has a member of klistview)
// this is done by specifying the view and the widget in the constructor,
// even if they are actually the same object (specify it twice in that case)
class KrViewOperator : public QObject
{
    Q_OBJECT
public:
    KrViewOperator(KrView *view, QWidget *widget);
    ~KrViewOperator() override;

    KrView *view() const
    {
        return _view;
    }
    QWidget *widget() const
    {
        return _widget;
    }
    void startDrag();

    void emitGotDrop(QDropEvent *e)
    {
        emit gotDrop(e);
    }
    void emitLetsDrag(QStringList items, const QPixmap &icon)
    {
        emit letsDrag(std::move(items), icon);
    }
    void emitItemDescription(const QString &desc)
    {
        emit itemDescription(desc);
    }
    void emitContextMenu(const QPoint &point)
    {
        emit contextMenu(point);
    }
    void emitEmptyContextMenu(const QPoint &point)
    {
        emit emptyContextMenu(point);
    }
    void emitRenameItem(const QString &oldName, const QString &newName)
    {
        emit renameItem(oldName, newName);
    }
    void emitExecuted(const QString &name)
    {
        emit executed(name);
    }
    void emitGoInside(const QString &name)
    {
        emit goInside(name);
    }
    void emitNeedFocus()
    {
        emit needFocus();
    }
    void emitMiddleButtonClicked(KrViewItem *item)
    {
        emit middleButtonClicked(item);
    }
    void emitCurrentChanged(KrViewItem *item)
    {
        emit currentChanged(item);
    }
    void emitPreviewJobStarted(KJob *job)
    {
        emit previewJobStarted(job);
    }
    void emitGoHome()
    {
        emit goHome();
    }
    void emitDirUp()
    {
        emit dirUp();
    }
    void emitQuickCalcSpace(KrViewItem *item)
    {
        emit quickCalcSpace(item);
    }
    void emitDefaultDeleteFiles()
    {
        emit defaultDeleteFiles();
    }
    void emitRefreshActions()
    {
        emit refreshActions();
    }
    void emitGoBack()
    {
        emit goBack();
    }
    void emitGoForward()
    {
        emit goForward();
    }

    /**
     * Search for an item by file name beginning at the current cursor position and set the
     * cursor to it.
     *
     * @param text file name to search for, can be regex
     * @param caseSensitive whether the search is case sensitive
     * @param direction @c 0 is for forward, @c 1 is for backward
     * @return true if there is a next/previous item matching the text, else false
     */
    bool searchItem(const QString &text, bool caseSensitive, int direction = 0);
    /**
     * Filter view items.
     */
    bool filterSearch(const QString &, bool);
    void setMassSelectionUpdate(bool upd);
    bool isMassSelectionUpdate()
    {
        return _massSelectionUpdate;
    }
    void settingsChanged(KrViewProperties::PropertyType properties);

public slots:
    void emitSelectionChanged()
    {
        if (!_massSelectionUpdate)
            emit selectionChanged();
    }

    void startUpdate();
    void cleared();
    void fileAdded(FileItem *fileitem);
    void fileUpdated(FileItem *newFileitem);

signals:
    void selectionChanged();
    void gotDrop(QDropEvent *e);
    void letsDrag(QStringList items, QPixmap icon);
    void itemDescription(const QString &desc);
    void contextMenu(const QPoint &point);
    void emptyContextMenu(const QPoint &point);
    void renameItem(const QString &oldName, const QString &newName);
    void executed(const QString &name);
    void goInside(const QString &name);
    void needFocus();
    void middleButtonClicked(KrViewItem *item);
    void currentChanged(KrViewItem *item);
    void previewJobStarted(KJob *job);
    void goHome();
    void defaultDeleteFiles();
    void dirUp();
    void quickCalcSpace(KrViewItem *item);
    void refreshActions();
    void goBack();
    void goForward();

protected slots:
    void saveDefaultSettings();

protected:
    // never delete those
    KrView *_view;
    QWidget *_widget;

private:
    bool _massSelectionUpdate;
    QTimer _saveDefaultSettingsTimer;
    static KrViewProperties::PropertyType _changedProperties;
    static KrView *_changedView;
};

/****************************************************************************
 * READ THIS FIRST: Using the view
 *
 * You always hold a pointer to KrView, thus you can only use functions declared
 * in this class. If you need something else, either this class is missing something
 * or you are ;-)
 *
 * The functions you'd usually want:
 * 1) getSelectedItems - returns all selected items, or (if none) the current item.
 *    it never returns anything which includes the "..", and thus can return an empty list!
 * 2) getSelectedKrViewItems - the same as (1), but returns a QValueList with KrViewItems
 * 3) getCurrentItem, setCurrentItem - work with QString
 * 4) getFirst, getNext, getPrev, getCurrentKrViewItem - all work with KrViewItems, and
 *    used to iterate through a list of items. note that getNext and getPrev accept a pointer
 *    to the current item (used in detailedview for safe iterating), thus your loop should be:
 *       for (KrViewItem *it = view->getFirst(); it!=0; it = view->getNext(it)) { blah; }
 * 5) nameToMakeCurrent(), setNameToMakeCurrent() - work with QString
 *
 * IMPORTANT NOTE: every one who subclasses this must call initProperties() in the constructor !!!
 */
class KrView
{
    friend class KrViewItem;
    friend class KrViewOperator;

public:
    class IconSizes : public QVector<int>
    {
    public:
        IconSizes()
        {
            *this << 12 << 16 << 22 << 32 << 48 << 64 << 128 << 256;
        }
    };

    // instantiating a new view
    // 1. new KrView
    // 2. view->init()
    // notes: constructor does as little as possible, setup() does the rest. esp, note that
    // if you need something from operator or properties, move it into setup()
    void init(bool enableUpdateDefaultSettings = true);
    KrViewInstance *instance()
    {
        return &_instance;
    }
    static const IconSizes iconSizes;

protected:
    void initProperties();
    KrViewOperator *createOperator()
    {
        return new KrViewOperator(this, _widget);
    }
    virtual void setup() = 0;

    ///////////////////////////////////////////////////////
    // Every view must implement the following functions //
    ///////////////////////////////////////////////////////
public:
    // interview related functions
    virtual QModelIndex getCurrentIndex() = 0;
    virtual bool isSelected(const QModelIndex &) = 0;
    virtual bool ensureVisibilityAfterSelect() = 0;
    virtual void selectRegion(KrViewItem *, KrViewItem *, bool) = 0;

    virtual int numSelected() const = 0;
    virtual QList<QUrl> selectedUrls() = 0;
    virtual void setSelectionUrls(const QList<QUrl> urls) = 0;
    virtual KrViewItem *getFirst() = 0;
    virtual KrViewItem *getLast() = 0;
    virtual KrViewItem *getNext(KrViewItem *current) = 0;
    virtual KrViewItem *getPrev(KrViewItem *current) = 0;
    virtual KrViewItem *getCurrentKrViewItem() = 0;
    virtual KrViewItem *getKrViewItemAt(const QPoint &vp) = 0;
    virtual KrViewItem *findItemByName(const QString &name) = 0;
    virtual KrViewItem *findItemByUrl(const QUrl &url) = 0;
    virtual QString getCurrentItem() const = 0;
    virtual void setCurrentItem(const QString &name, bool scrollToCurrent = true, const QModelIndex &fallbackToIndex = QModelIndex()) = 0;
    virtual void setCurrentKrViewItem(KrViewItem *item, bool scrollToCurrent = true) = 0;
    virtual void makeItemVisible(const KrViewItem *item) = 0;
    virtual bool isItemVisible(const KrViewItem *item) = 0;
    virtual void updateView() = 0;
    virtual void sort() = 0;
    virtual void refreshColors() = 0;
    virtual void redraw() = 0;
    virtual bool handleKeyEvent(QKeyEvent *e);
    virtual void prepareForActive() = 0;
    virtual void prepareForPassive() = 0;
    virtual void renameCurrentItem() = 0; // Rename current item. returns immediately
    virtual int itemsPerPage() = 0;
    virtual void showContextMenu(const QPoint &point = QPoint(0, 0)) = 0;

protected:
    virtual KrViewItem *preAddItem(FileItem *fileitem) = 0;
    virtual void preDeleteItem(KrViewItem *item) = 0;
    virtual void copySettingsFrom(KrView *other) = 0;
    virtual void populate(const QList<FileItem *> &fileItems, FileItem *dummy) = 0;
    virtual void intSetSelected(const FileItem *fileitem, bool select) = 0;
    virtual void clear();

    void addItem(FileItem *fileItem, bool onUpdate = false);
    void deleteItem(const QString &name, bool onUpdate = false);
    void updateItem(FileItem *newFileItem);

public:
    //////////////////////////////////////////////////////
    // the following functions are already implemented, //
    // and normally - should NOT be re-implemented.     //
    //////////////////////////////////////////////////////
    uint numFiles() const
    {
        return _count - _numDirs;
    }
    uint numDirs() const
    {
        return _numDirs;
    }
    uint count() const
    {
        return _count;
    }
    void getSelectedItems(QStringList *names, bool fallbackToFocused = true);
    void getItemsByMask(const QString &mask, QStringList *names, bool dirs = true, bool files = true);
    KrViewItemList getSelectedKrViewItems();
    void selectAllIncludingDirs()
    {
        changeSelection(KrQuery("*"), true, true);
    }
    void select(const KrQuery &filter = KrQuery("*"))
    {
        changeSelection(filter, true);
    }
    void unselect(const KrQuery &filter = KrQuery("*"))
    {
        changeSelection(filter, false);
    }
    void unselectAll()
    {
        changeSelection(KrQuery("*"), false, true);
    }
    void invertSelection();
    void setNameToMakeCurrent(const QString &name)
    {
        _nameToMakeCurrent = name;
    }
    QString firstUnmarkedBelowCurrent(const bool skipCurrent);
    QString statistics();
    const KrViewProperties *properties() const
    {
        return _properties;
    }
    KrViewOperator *op() const
    {
        return _operator;
    }
    void showPreviews(bool show);
    bool previewsShown()
    {
        return _previews != nullptr;
    }
    void applySettingsToOthers();

    void setFiles(DirListerInterface *files);

    /**
     * Refresh the file view items after the underlying file model changed.
     *
     * Tries to preserve current file and file selection if applicable.
     */
    void refresh();

    bool changeSelection(const KrQuery &filter, bool select);
    bool changeSelection(const KrQuery &filter, bool select, bool includeDirs, bool makeVisible = false);
    bool isFiltered(FileItem *fileitem);
    void setSelected(const FileItem *fileitem, bool select);

    /////////////////////////////////////////////////////////////
    // the following functions have a default and minimalistic //
    // implementation, and may be re-implemented if needed     //
    /////////////////////////////////////////////////////////////
    virtual void setSortMode(KrViewProperties::ColumnType sortColumn, bool descending)
    {
        sortModeUpdated(sortColumn, descending);
    }
    const KrQuery &filterMask() const
    {
        return _properties->filterMask;
    }
    KrViewProperties::FilterSpec filter() const
    {
        return _properties->filter;
    }
    void setFilter(KrViewProperties::FilterSpec filter);
    void setFilter(KrViewProperties::FilterSpec filter, const FilterSettings &customFilter, bool applyToDirs);
    void customSelection(bool select);
    int defaultFileIconSize();
    virtual void setFileIconSize(int size);
    void setDefaultFileIconSize()
    {
        setFileIconSize(defaultFileIconSize());
    }
    void zoomIn();
    void zoomOut();

    // save this view's settings to be restored after restart
    virtual void saveSettings(KConfigGroup grp, KrViewProperties::PropertyType properties = KrViewProperties::AllProperties);

    inline QWidget *widget()
    {
        return _widget;
    }
    inline int fileIconSize() const
    {
        return _fileIconSize;
    }
    inline bool isFocused() const
    {
        return _focused;
    }

    QPixmap getIcon(FileItem *fileitem);

    void setMainWindow(QWidget *mainWindow)
    {
        _mainWindow = mainWindow;
    }

    // save this view's settings as default for new views of this type
    void saveDefaultSettings(KrViewProperties::PropertyType properties = KrViewProperties::AllProperties);
    // restore the default settings for this view type
    void restoreDefaultSettings();
    // call this to restore this view's settings after restart
    void restoreSettings(const KConfigGroup &grp);

    void saveSelection();
    void restoreSelection();
    bool canRestoreSelection()
    {
        return !_savedSelection.isEmpty();
    }
    void clearSavedSelection();

    void markSameBaseName();
    void markSameExtension();

    // todo: what about selection modes ???
    virtual ~KrView();

    static QPixmap getIcon(FileItem *fileitem, bool active, int size = 0);
    static QPixmap processIcon(const QPixmap &icon, bool dim, const QColor &dimColor, int dimFactor, bool symlink);

    // Get GUI strings for file item properties
    static QString krPermissionText(const FileItem *fileitem);
    static QString permissionsText(const KrViewProperties *properties, const FileItem *fileItem);
    static QString sizeText(const KrViewProperties *properties, KIO::filesize_t size);
    static QString mimeTypeText(FileItem *fileItem);

protected:
    KrView(KrViewInstance &instance, KConfig *cfg);

    virtual void doRestoreSettings(KConfigGroup grp);
    virtual KIO::filesize_t calcSize() = 0;
    virtual KIO::filesize_t calcSelectedSize() = 0;
    void sortModeUpdated(KrViewProperties::ColumnType sortColumn, bool descending);
    inline void setWidget(QWidget *w)
    {
        _widget = w;
    }
    bool drawCurrent() const;

    KConfig *_config;
    KrViewProperties *_properties;
    KrViewOperator *_operator;
    bool _focused;
    int _fileIconSize;

private:
    void updatePreviews();
    void saveSortMode(KConfigGroup &group);
    void restoreSortMode(KConfigGroup &group);

    KrViewInstance &_instance;
    DirListerInterface *_files;
    QWidget *_mainWindow;
    QWidget *_widget;
    QList<QUrl> _savedSelection;
    QString _nameToMakeCurrent;
    KrPreviews *_previews;
    bool _updateDefaultSettings;
    bool _ignoreSettingsChange;
    QRegExp _quickFilterMask;
    uint _count, _numDirs;
    FileItem *_dummyFileItem;
};

#endif /* KRVIEW_H */
