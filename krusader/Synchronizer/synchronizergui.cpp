/***************************************************************************
                       synchronizergui.cpp  -  description
                             -------------------
    copyright            : (C) 2003 + by Csaba Karai
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

#include "synchronizergui.h"
#include "../krglobal.h"
#include "../defaults.h"
#include "../krusaderview.h"
#include "../Panel/listpanel.h"
#include "../Panel/panelfunc.h"
#include "../FileSystem/krpermhandler.h"
#include "../KViewer/krviewer.h"
#include "../Dialogs/krspwidgets.h"
#include "../FileSystem/krquery.h"
#include "../krservices.h"
#include "../krslots.h"
#include "../kicons.h"
#include "synchronizedialog.h"
#include "feedtolistboxdialog.h"
#include "synchronizercolors.h"

// QtCore
#include <QEventLoop>
#include <QRegExp>
#include <QMimeData>
#include <QHash>
// QtGui
#include <QResizeEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QCursor>
#include <QDrag>
#include <QClipboard>
// QtWidgets
#include <QApplication>
#include <QLabel>
#include <QGridLayout>
#include <QLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QFrame>
#include <QHeaderView>
#include <QMenu>
#include <QSpinBox>

#include <KConfigCore/KSharedConfig>
#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KMessageBox>
#include <KIOWidgets/KUrlRequester>
#include <KGuiAddons/KColorUtils>


class SynchronizerListView : public KrTreeWidget
{
private:
    Synchronizer   *synchronizer;
    bool            isLeft;

public:
    SynchronizerListView(Synchronizer * sync, QWidget * parent) : KrTreeWidget(parent), synchronizer(sync) {
    }

    void mouseMoveEvent(QMouseEvent * e) {
        isLeft = ((e->modifiers() & Qt::ShiftModifier) == 0);
        KrTreeWidget::mouseMoveEvent(e);
    }

    void startDrag(Qt::DropActions /* supportedActs */) {
        QList<QUrl> urls;

        unsigned              ndx = 0;
        SynchronizerFileItem  *currentItem;

        while ((currentItem = synchronizer->getItemAt(ndx++)) != 0) {
            SynchronizerGUI::SyncViewItem *viewItem = (SynchronizerGUI::SyncViewItem *)currentItem->userData();

            if (!viewItem || !viewItem->isSelected() || viewItem->isHidden())
                continue;

            SynchronizerFileItem *item = viewItem->synchronizerItemRef();
            if (item) {
                if (isLeft && item->existsInLeft()) {
                    QString leftDirName = item->leftDirectory().isEmpty() ? "" : item->leftDirectory() + '/';
                    QUrl leftURL = Synchronizer::fsUrl(synchronizer->leftBaseDirectory()  + leftDirName + item->leftName());
                    urls.push_back(leftURL);
                } else if (!isLeft && item->existsInRight()) {
                    QString rightDirName = item->rightDirectory().isEmpty() ? "" : item->rightDirectory() + '/';
                    QUrl rightURL = Synchronizer::fsUrl(synchronizer->rightBaseDirectory()  + rightDirName + item->rightName());
                    urls.push_back(rightURL);
                }
            }
        }

        if (urls.count() == 0)
            return;

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setImageData(FL_LOADICON(isLeft ? "arrow-left-double" : "arrow-right-double"));
        mimeData->setUrls(urls);
        drag->setMimeData(mimeData);
        drag->start();
    }
};


SynchronizerGUI::SynchronizerGUI(QWidget* parent,  QUrl leftURL, QUrl rightURL, QStringList selList) :
        QDialog(parent)
{
    initGUI(QString(), leftURL, rightURL, selList);
}

SynchronizerGUI::SynchronizerGUI(QWidget* parent,  QString profile) :
        QDialog(parent)
{
    initGUI(profile, QUrl(), QUrl(), QStringList());
}

void SynchronizerGUI::initGUI(QString profileName, QUrl leftURL, QUrl rightURL, QStringList selList)
{
    setAttribute(Qt::WA_DeleteOnClose);

    selectedFiles = selList;
    isComparing = wasClosed = wasSync = false;
    firstResize = true;
    sizeX = sizeY = -1;

    bool equalSizes = false;

    hasSelectedFiles = (selectedFiles.count() != 0);

    if (leftURL.isEmpty())
        leftURL = QUrl::fromLocalFile(ROOT_DIR);
    if (rightURL.isEmpty())
        rightURL = QUrl::fromLocalFile(ROOT_DIR);

    setWindowTitle(i18n("Krusader::Synchronize Folders"));
    QGridLayout *synchGrid = new QGridLayout(this);
    synchGrid->setSpacing(6);
    synchGrid->setContentsMargins(11, 11, 11, 11);

    synchronizerTabs = new QTabWidget(this);

    /* ============================== Compare groupbox ============================== */

    QWidget *synchronizerTab = new QWidget(this);
    QGridLayout *synchronizerGrid = new QGridLayout(synchronizerTab);
    synchronizerGrid->setSpacing(6);
    synchronizerGrid->setContentsMargins(11, 11, 11, 11);

    QGroupBox *compareDirs = new QGroupBox(synchronizerTab);
    compareDirs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    compareDirs->setTitle(i18n("Folder Comparison"));

    QGridLayout *grid = new QGridLayout(compareDirs);
    grid->setSpacing(6);
    grid->setContentsMargins(11, 11, 11, 11);

    leftDirLabel = new QLabel(compareDirs);
    leftDirLabel->setAlignment(Qt::AlignHCenter);
    grid->addWidget(leftDirLabel, 0 , 0);

    QLabel *filterLabel = new QLabel(compareDirs);
    filterLabel->setText(i18n("File &Filter:"));
    filterLabel->setAlignment(Qt::AlignHCenter);
    grid->addWidget(filterLabel, 0 , 1);

    rightDirLabel = new QLabel(compareDirs);
    rightDirLabel->setAlignment(Qt::AlignHCenter);
    grid->addWidget(rightDirLabel, 0 , 2);

    KConfigGroup group(krConfig, "Synchronize");

    leftLocation = new KHistoryComboBox(false, compareDirs);
    leftLocation->setMaxCount(25);  // remember 25 items
    leftLocation->setDuplicatesEnabled(false);
    leftLocation->setEditable(true);
    leftLocation->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    QStringList list = group.readEntry("Left Folder History", QStringList());
    leftLocation->setHistoryItems(list);
    KUrlRequester *leftUrlReq = new KUrlRequester(leftLocation, compareDirs);
    leftUrlReq->setUrl(leftURL);
    leftUrlReq->setMode(KFile::Directory);
    leftUrlReq->setMinimumWidth(250);
    grid->addWidget(leftUrlReq, 1 , 0);
    leftLocation->setWhatsThis(i18n("The left base folder used during the synchronization process."));
    leftUrlReq->setEnabled(!hasSelectedFiles);
    leftLocation->setEnabled(!hasSelectedFiles);
    leftDirLabel->setBuddy(leftLocation);

    fileFilter = new KHistoryComboBox(false, compareDirs);
    fileFilter->setMaxCount(25);  // remember 25 items
    fileFilter->setDuplicatesEnabled(false);
    fileFilter->setMinimumWidth(100);
    fileFilter->setMaximumWidth(100);
    fileFilter->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    list = group.readEntry("File Filter", QStringList());
    fileFilter->setHistoryItems(list);
    fileFilter->setEditText("*");
    grid->addWidget(fileFilter, 1 , 1);
    filterLabel->setBuddy(fileFilter);

    QString wtFilter = "<p><img src='toolbar|find'></p>" + i18n("<p>The filename filtering criteria is defined here.</p><p>You can make use of wildcards. Multiple patterns are separated by space (means logical OR) and patterns are excluded from the search using the pipe symbol.</p><p>If the pattern is ended with a slash (<code>*pattern*/</code>), that means that pattern relates to recursive search of folders.<ul><li><code>pattern</code> - means to search those files/folders that name is <code>pattern</code>, recursive search goes through all subfolders independently of the value of <code>pattern</code></li><li><code>pattern/</code> - means to search all files/folders, but recursive search goes through/excludes the folders that name is <code>pattern</code></li></ul></p><p>It is allowed to use quotation marks for names that contain space. Filter <code>\"Program&nbsp;Files\"</code> searches out those files/folders that name is <code>Program&nbsp;Files</code>.</p><p>Examples:</p><ul><li><code>*.o</code></li><li><code>*.h *.c\?\?</code></li><li><code>*.cpp *.h | *.moc.cpp</code></li><li><code>* | .svn/ .git/</code></li></ul><p><b>Note</b>: the search term '<code>text</code>' is equivalent to '<code>*text*</code>'.</p>");
    fileFilter->setWhatsThis(wtFilter);
    filterLabel->setWhatsThis(wtFilter);

    rightLocation = new KHistoryComboBox(compareDirs);
    rightLocation->setMaxCount(25);  // remember 25 items
    rightLocation->setDuplicatesEnabled(false);
    rightLocation->setEditable(true);
    rightLocation->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    list = group.readEntry("Right Folder History", QStringList());
    rightLocation->setHistoryItems(list);
    KUrlRequester *rightUrlReq = new KUrlRequester(rightLocation, compareDirs);
    rightUrlReq->setUrl(rightURL);
    rightUrlReq->setMode(KFile::Directory);
    rightUrlReq->setMinimumWidth(250);
    grid->addWidget(rightUrlReq, 1 , 2);
    rightLocation->setWhatsThis(i18n("The right base folder used during the synchronization process."));
    rightUrlReq->setEnabled(!hasSelectedFiles);
    rightLocation->setEnabled(!hasSelectedFiles);
    rightDirLabel->setBuddy(rightLocation);

    QWidget *optionWidget  = new QWidget(compareDirs);
    QHBoxLayout *optionBox = new QHBoxLayout(optionWidget);
    optionBox->setContentsMargins(0, 0, 0, 0);

    QWidget *optionGridWidget = new QWidget(optionWidget);
    QGridLayout *optionGrid = new QGridLayout(optionGridWidget);

    optionBox->addWidget(optionGridWidget);

    cbSubdirs         = new QCheckBox(i18n("Recurse subfolders"), optionGridWidget);
    cbSubdirs->setChecked(group.readEntry("Recurse Subdirectories", _RecurseSubdirs));
    optionGrid->addWidget(cbSubdirs, 0, 0);
    cbSubdirs->setWhatsThis(i18n("Compare not only the base folders but their subfolders as well."));
    cbSymlinks        = new QCheckBox(i18n("Follow symlinks"), optionGridWidget);
    cbSymlinks->setChecked(group.readEntry("Follow Symlinks", _FollowSymlinks));
    cbSymlinks->setEnabled(cbSubdirs->isChecked());
    optionGrid->addWidget(cbSymlinks, 0, 1);
    cbSymlinks->setWhatsThis(i18n("Follow symbolic links during the compare process."));
    cbByContent       = new QCheckBox(i18n("Compare by content"), optionGridWidget);
    cbByContent->setChecked(group.readEntry("Compare By Content", _CompareByContent));
    optionGrid->addWidget(cbByContent, 0, 2);
    cbByContent->setWhatsThis(i18n("Compare duplicated files with same size by content."));
    cbIgnoreDate      = new QCheckBox(i18n("Ignore Date"), optionGridWidget);
    cbIgnoreDate->setChecked(group.readEntry("Ignore Date", _IgnoreDate));
    optionGrid->addWidget(cbIgnoreDate, 1, 0);
    cbIgnoreDate->setWhatsThis(i18n("<p>Ignore date information during the compare process.</p><p><b>Note</b>: useful if the files are located on network filesystems or in archives.</p>"));
    cbAsymmetric      = new QCheckBox(i18n("Asymmetric"), optionGridWidget);
    cbAsymmetric->setChecked(group.readEntry("Asymmetric", _Asymmetric));
    optionGrid->addWidget(cbAsymmetric, 1, 1);
    cbAsymmetric->setWhatsThis(i18n("<p><b>Asymmetric mode</b></p><p>The left side is the destination, the right is the source folder. Files existing only in the left folder will be deleted, the other differing ones will be copied from right to left.</p><p><b>Note</b>: useful when updating a folder from a file server.</p>"));
    cbIgnoreCase      = new QCheckBox(i18n("Ignore Case"), optionGridWidget);
    cbIgnoreCase->setChecked(group.readEntry("Ignore Case", _IgnoreCase));
    optionGrid->addWidget(cbIgnoreCase, 1, 2);
    cbIgnoreCase->setWhatsThis(i18n("<p>Case insensitive filename compare.</p><p><b>Note</b>: useful when synchronizing Windows filesystems.</p>"));

    /* =========================== Show options groupbox ============================= */

    QGroupBox *showOptions  = new QGroupBox(optionWidget);
    optionBox->addWidget(showOptions);

    showOptions->setTitle(i18n("S&how options"));
    showOptions->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QGridLayout *showOptionsLayout = new QGridLayout(showOptions);
    showOptionsLayout->setSpacing(4);
    showOptionsLayout->setContentsMargins(11, 11, 11, 11);

    bool checked;
    QString description;

    checked = group.readEntry("LeftToRight Button", _BtnLeftToRight);
    description = i18n("Show files marked to <i>Copy from left to right</i>.");
    btnLeftToRight =
        createButton(showOptions, "arrow-right", checked, Qt::CTRL + Qt::Key_L, description);
    showOptionsLayout->addWidget(btnLeftToRight, 0, 0);

    checked = group.readEntry("Equals Button", _BtnEquals);
    description = i18n("Show files considered to be identical.");
    btnEquals =
        createButton(showOptions, "equals", checked, Qt::CTRL + Qt::Key_E, description, "=");
    showOptionsLayout->addWidget(btnEquals, 0, 1);

    checked = group.readEntry("Differents Button", _BtnDifferents);
    description = i18n("Show excluded files.");
    btnDifferents =
        createButton(showOptions, "unequals", checked, Qt::CTRL + Qt::Key_D, description, "!=");
    showOptionsLayout->addWidget(btnDifferents, 0, 2);

    checked = group.readEntry("RightToLeft Button", _BtnRightToLeft);
    description = i18n("Show files marked to <i>Copy from right to left</i>.");
    btnRightToLeft =
        createButton(showOptions, "arrow-left", checked, Qt::CTRL + Qt::Key_R, description);
    showOptionsLayout->addWidget(btnRightToLeft, 0, 3);

    checked = group.readEntry("Deletable Button", _BtnDeletable);
    description = i18n("Show files marked to delete.");
    btnDeletable =
        createButton(showOptions, "user-trash", checked, Qt::CTRL + Qt::Key_T, description);
    showOptionsLayout->addWidget(btnDeletable, 0, 4);

    checked = group.readEntry("Duplicates Button", _BtnDuplicates);
    description = i18n("Show files that exist on both sides.");
    btnDuplicates = createButton(showOptions, "arrow-up", checked, Qt::CTRL + Qt::Key_I,
                                 description, i18n("Duplicates"), true);
    showOptionsLayout->addWidget(btnDuplicates, 0, 5);

    checked = group.readEntry("Singles Button", _BtnSingles);
    description = i18n("Show files that exist on one side only.");
    btnSingles = createButton(showOptions, "arrow-down", checked, Qt::CTRL + Qt::Key_N, description,
                              i18n("Singles"), true);
    showOptionsLayout->addWidget(btnSingles, 0, 6);

    grid->addWidget(optionWidget, 2, 0, 1, 3);

    synchronizerGrid->addWidget(compareDirs, 0, 0);

    /* ========================= Synchronization list view ========================== */
    syncList = new SynchronizerListView(&synchronizer, synchronizerTab);  // create the main container
    syncList->setWhatsThis(i18n("The compare results of the synchronizer (Ctrl+M)."));
    syncList->setAutoFillBackground(true);
    syncList->installEventFilter(this);

    KConfigGroup gl(krConfig, "Look&Feel");
    syncList->setFont(gl.readEntry("Filelist Font", _FilelistFont));

    syncList->setBackgroundRole(QPalette::Window);
    syncList->setAutoFillBackground(true);

    QStringList labels;
    labels << i18nc("@title:column file name", "Name");
    labels << i18nc("@title:column", "Size");
    labels << i18nc("@title:column", "Date");
    labels << i18n("<=>");
    labels << i18nc("@title:column", "Date");
    labels << i18nc("@title:column", "Size");
    labels << i18nc("@title:column file name", "Name");
    syncList->setHeaderLabels(labels);

    syncList->header()->setSectionResizeMode(QHeaderView::Interactive);

    if (group.hasKey("State"))
        syncList->header()->restoreState(group.readEntry("State", QByteArray()));
    else {
        int i = QFontMetrics(syncList->font()).width("W");
        int j = QFontMetrics(syncList->font()).width("0");
        j = (i > j ? i : j);
        int typeWidth = j * 7 / 2;

        syncList->setColumnWidth(0, typeWidth * 4);
        syncList->setColumnWidth(1, typeWidth * 2);
        syncList->setColumnWidth(2, typeWidth * 3);
        syncList->setColumnWidth(3, typeWidth * 1);
        syncList->setColumnWidth(4, typeWidth * 3);
        syncList->setColumnWidth(5, typeWidth * 2);
        syncList->setColumnWidth(6, typeWidth * 4);

        equalSizes = true;
    }

    syncList->setAllColumnsShowFocus(true);
    syncList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    syncList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    syncList->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    syncList->header()->setSortIndicatorShown(false);
    syncList->setSortingEnabled(false);
    syncList->setRootIsDecorated(true);
    syncList->setIndentation(10);
    syncList->setDragEnabled(true);
    syncList->setAutoFillBackground(true);

    synchronizerGrid->addWidget(syncList, 1, 0);

    synchronizerTabs->addTab(synchronizerTab, i18n("&Synchronizer"));
    synchGrid->addWidget(synchronizerTabs, 0, 0);

    filterTabs = FilterTabs::addTo(synchronizerTabs, FilterTabs::HasDontSearchIn);
    generalFilter = (GeneralFilter *)filterTabs->get("GeneralFilter");
    generalFilter->searchFor->setEditText(fileFilter->currentText());
    generalFilter->searchForCase->setChecked(true);

    // creating the time shift, equality threshold, hidden files options

    QGroupBox *optionsGroup = new QGroupBox(generalFilter);
    optionsGroup->setTitle(i18n("&Options"));

    QGridLayout *optionsLayout = new QGridLayout(optionsGroup);
    optionsLayout->setAlignment(Qt::AlignTop);
    optionsLayout->setSpacing(6);
    optionsLayout->setContentsMargins(11, 11, 11, 11);

    QLabel * parallelThreadsLabel = new QLabel(i18n("Parallel threads:"), optionsGroup);
    optionsLayout->addWidget(parallelThreadsLabel, 0, 0);
    parallelThreadsSpinBox = new QSpinBox(optionsGroup);
    parallelThreadsSpinBox->setMinimum(1);
    parallelThreadsSpinBox->setMaximum(15);
    int parThreads = group.readEntry("Parallel Threads", 1);
    parallelThreadsSpinBox->setValue(parThreads);

    optionsLayout->addWidget(parallelThreadsSpinBox, 0, 1);

    QLabel * equalityLabel = new QLabel(i18n("Equality threshold:"), optionsGroup);
    optionsLayout->addWidget(equalityLabel, 1, 0);

    equalitySpinBox = new QSpinBox(optionsGroup);
    equalitySpinBox->setMaximum(9999);
    optionsLayout->addWidget(equalitySpinBox, 1, 1);

    equalityUnitCombo = new QComboBox(optionsGroup);
    equalityUnitCombo->addItem(i18n("sec"));
    equalityUnitCombo->addItem(i18n("min"));
    equalityUnitCombo->addItem(i18n("hour"));
    equalityUnitCombo->addItem(i18n("day"));
    optionsLayout->addWidget(equalityUnitCombo, 1, 2);

    QLabel * timeShiftLabel = new QLabel(i18n("Time shift (right-left):"), optionsGroup);
    optionsLayout->addWidget(timeShiftLabel, 2, 0);

    timeShiftSpinBox = new QSpinBox(optionsGroup);
    timeShiftSpinBox->setMinimum(-9999);
    timeShiftSpinBox->setMaximum(9999);
    optionsLayout->addWidget(timeShiftSpinBox, 2, 1);

    timeShiftUnitCombo = new QComboBox(optionsGroup);
    timeShiftUnitCombo->addItem(i18n("sec"));
    timeShiftUnitCombo->addItem(i18n("min"));
    timeShiftUnitCombo->addItem(i18n("hour"));
    timeShiftUnitCombo->addItem(i18n("day"));
    optionsLayout->addWidget(timeShiftUnitCombo, 2, 2);

    QFrame *line = new QFrame(optionsGroup);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    optionsLayout->addWidget(line, 3, 0, 1, 3);

    ignoreHiddenFilesCB = new QCheckBox(i18n("Ignore hidden files"), optionsGroup);
    optionsLayout->addWidget(ignoreHiddenFilesCB, 4, 0, 1, 3);

    generalFilter->middleLayout->addWidget(optionsGroup);


    /* ================================== Buttons =================================== */

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->setSpacing(6);
    buttons->setContentsMargins(0, 0, 0, 0);

    profileManager = new ProfileManager("SynchronizerProfile", this);
    profileManager->setShortcut(Qt::CTRL + Qt::Key_P);
    profileManager->setWhatsThis(i18n("Profile manager (Ctrl+P)."));
    buttons->addWidget(profileManager);

    btnSwapSides = new QPushButton(this);
    btnSwapSides->setIcon(QIcon::fromTheme("document-swap"));
    btnSwapSides->setShortcut(Qt::CTRL + Qt::Key_S);
    btnSwapSides->setWhatsThis(i18n("Swap sides (Ctrl+S)."));
    buttons->addWidget(btnSwapSides);

    statusLabel = new QLabel(this);
    buttons->addWidget(statusLabel);

    QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttons->addItem(spacer);

    btnCompareDirs = new QPushButton(this);
    btnCompareDirs->setText(i18n("Compare"));
    btnCompareDirs->setIcon(QIcon::fromTheme("kr_comparedirs"));
    btnCompareDirs->setDefault(true);
    buttons->addWidget(btnCompareDirs);

    btnScrollResults = new QPushButton(this);
    btnScrollResults->setCheckable(true);
    btnScrollResults->setChecked(group.readEntry("Scroll Results", _ScrollResults));
    btnScrollResults->hide();
    if (btnScrollResults->isChecked())
        btnScrollResults->setText(i18n("Quiet"));
    else
        btnScrollResults->setText(i18n("Scroll Results"));
    buttons->addWidget(btnScrollResults);

    btnStopComparing = new QPushButton(this);
    btnStopComparing->setText(i18n("Stop"));
    btnStopComparing->setIcon(QIcon::fromTheme("process-stop"));
    btnStopComparing->setEnabled(false);
    buttons->addWidget(btnStopComparing);

    btnFeedToListBox = new QPushButton(this);
    btnFeedToListBox->setText(i18n("Feed to listbox"));
    btnFeedToListBox->setIcon(QIcon::fromTheme("list-add"));
    btnFeedToListBox->setEnabled(false);
    btnFeedToListBox->hide();
    buttons->addWidget(btnFeedToListBox);

    btnSynchronize = new QPushButton(this);
    btnSynchronize->setText(i18n("Synchronize"));
    btnSynchronize->setIcon(QIcon::fromTheme("folder-sync"));
    btnSynchronize->setEnabled(false);
    buttons->addWidget(btnSynchronize);

    QPushButton *btnCloseSync = new QPushButton(this);
    btnCloseSync->setText(i18n("Close"));
    btnCloseSync->setIcon(QIcon::fromTheme("dialog-close"));
    buttons->addWidget(btnCloseSync);

    synchGrid->addLayout(buttons, 1, 0);

    /* =============================== Connect table ================================ */

    connect(syncList, SIGNAL(itemRightClicked(QTreeWidgetItem*,QPoint,int)),
            this, SLOT(rightMouseClicked(QTreeWidgetItem*,QPoint)));
    connect(syncList, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(doubleClicked(QTreeWidgetItem*)));

    connect(profileManager, SIGNAL(loadFromProfile(QString)), this, SLOT(loadFromProfile(QString)));
    connect(profileManager, SIGNAL(saveToProfile(QString)), this, SLOT(saveToProfile(QString)));

    connect(btnSwapSides,      SIGNAL(clicked()), this, SLOT(swapSides()));
    connect(btnCompareDirs,    SIGNAL(clicked()), this, SLOT(compare()));
    connect(btnStopComparing,  SIGNAL(clicked()), this, SLOT(stop()));
    connect(btnFeedToListBox,  SIGNAL(clicked()), this, SLOT(feedToListBox()));
    connect(btnSynchronize,    SIGNAL(clicked()), this, SLOT(synchronize()));
    connect(btnScrollResults,  SIGNAL(toggled(bool)), this, SLOT(setScrolling(bool)));
    connect(btnCloseSync,      SIGNAL(clicked()), this, SLOT(closeDialog()));

    connect(cbSubdirs,         SIGNAL(toggled(bool)), this, SLOT(subdirsChecked(bool)));
    connect(cbAsymmetric,      SIGNAL(toggled(bool)), this, SLOT(setPanelLabels()));

    connect(&synchronizer,     SIGNAL(comparedFileData(SynchronizerFileItem*)), this,
            SLOT(addFile(SynchronizerFileItem*)));
    connect(&synchronizer,     SIGNAL(markChanged(SynchronizerFileItem*,bool)), this,
            SLOT(markChanged(SynchronizerFileItem*,bool)));
    connect(&synchronizer,     SIGNAL(statusInfo(QString)), this, SLOT(statusInfo(QString)));

    connect(btnLeftToRight,    SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(btnEquals,         SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(btnDifferents,     SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(btnRightToLeft,    SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(btnDeletable,      SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(btnDuplicates,     SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(btnSingles,        SIGNAL(toggled(bool)), this, SLOT(refresh()));

    connect(fileFilter,        SIGNAL(currentTextChanged(QString)), this, SLOT(connectFilters(QString)));
    connect(generalFilter->searchFor, SIGNAL(currentTextChanged(QString)), this, SLOT(connectFilters(QString)));
    connect(generalFilter->searchFor, SIGNAL(currentTextChanged(QString)), this, SLOT(setCompletion()));
    connect(generalFilter->dontSearchIn, SIGNAL(checkValidity(QString&,QString&)),
            this, SLOT(checkExcludeURLValidity(QString&,QString&)));

    connect(profileManager, SIGNAL(loadFromProfile(QString)), filterTabs, SLOT(loadFromProfile(QString)));
    connect(profileManager, SIGNAL(saveToProfile(QString)), filterTabs, SLOT(saveToProfile(QString)));

    setPanelLabels();
    setCompletion();

    /* =============================== Loading the colors ================================ */

    KConfigGroup gc(krConfig, "Colors");

    QString COLOR_NAMES[] = { "Equals", "Differs", "LeftCopy", "RightCopy", "Delete" };
    QPalette defaultPalette = QGuiApplication::palette();

    DECLARE_SYNCHRONIZER_BACKGROUND_DEFAULTS;
    DECLARE_SYNCHRONIZER_FOREGROUND_DEFAULTS;

    for (int clr = 0; clr != TT_MAX; clr ++) {
        QString colorName = clr > 4 ? "Equals" : COLOR_NAMES[ clr ];
        QColor backgroundDefault = clr > 4 ? defaultPalette.color(QPalette::Active, QPalette::Base) : SYNCHRONIZER_BACKGROUND_DEFAULTS[ clr ];
        QColor foregroundDefault = clr > 4 ? defaultPalette.color(QPalette::Active, QPalette::Text) : SYNCHRONIZER_FOREGROUND_DEFAULTS[ clr ];

        QString foreEntry = QString("Synchronizer ") + colorName + QString(" Foreground");
        QString bckgEntry = QString("Synchronizer ") + colorName + QString(" Background");

        if (gc.readEntry(foreEntry, QString()) == "KDE default")
            foreGrounds[ clr ] = QColor();
        else if (gc.readEntry(foreEntry, QString()).isEmpty())    // KDE4 workaround, default color doesn't work
            foreGrounds[ clr ] = foregroundDefault;
        else
            foreGrounds[ clr ] = gc.readEntry(foreEntry, foregroundDefault);

        if (gc.readEntry(bckgEntry, QString()) == "KDE default")
            backGrounds[ clr ] = QColor();
        else if (gc.readEntry(foreEntry, QString()).isEmpty())    // KDE4 workaround, default color doesn't work
            backGrounds[ clr ] = backgroundDefault;
        else
            backGrounds[ clr ] = gc.readEntry(bckgEntry, backgroundDefault);
    }
    if (backGrounds[ TT_EQUALS ].isValid()) {
        QPalette pal = syncList->palette();
        pal.setColor(QPalette::Base, backGrounds[ TT_EQUALS ]);
        syncList->setPalette(pal);
    }

    int sx = group.readEntry("Window Width",  -1);
    int sy = group.readEntry("Window Height",  -1);

    if (sx != -1 && sy != -1)
        resize(sx, sy);

    if (group.readEntry("Window Maximized",  false)) {
        setWindowState(windowState() | Qt::WindowMaximized);
    }

    if (equalSizes) {
        int newSize6 = syncList->header()->sectionSize(6);
        int newSize0 = syncList->header()->sectionSize(0);
        int delta = newSize6 - newSize0 + (newSize6 & 1);

        newSize0 += (delta / 2);

        if (newSize0 > 20)
            syncList->header()->resizeSection(0, newSize0);
    }

    if (!profileName.isNull())
        profileManager->loadProfile(profileName);

    synchronizer.setParentWidget(this);
}

SynchronizerGUI::~SynchronizerGUI()
{
    syncList->clear(); // for sanity: deletes the references to the synchronizer list
}

void SynchronizerGUI::setPanelLabels()
{
    if (hasSelectedFiles && cbAsymmetric->isChecked()) {
        leftDirLabel->setText(i18n("Selected files from targ&et folder:"));
        rightDirLabel->setText(i18n("Selected files from sou&rce folder:"));
    } else if (hasSelectedFiles && !cbAsymmetric->isChecked()) {
        leftDirLabel->setText(i18n("Selected files from &left folder:"));
        rightDirLabel->setText(i18n("Selected files from &right folder:"));
    } else if (cbAsymmetric->isChecked()) {
        leftDirLabel->setText(i18n("Targ&et folder:"));
        rightDirLabel->setText(i18n("Sou&rce folder:"));
    } else {
        leftDirLabel->setText(i18n("&Left folder:"));
        rightDirLabel->setText(i18n("&Right folder:"));
    }
}

void SynchronizerGUI::setCompletion()
{
    generalFilter->dontSearchIn->setCompletionDir(Synchronizer::fsUrl(rightLocation->currentText()));
}

void SynchronizerGUI::checkExcludeURLValidity(QString &text, QString &error)
{
    QUrl url = Synchronizer::fsUrl(text);
    if (url.isRelative())
        return;

    QString leftBase = leftLocation->currentText();
    if (!leftBase.endsWith('/'))
        leftBase += '/';
    QUrl leftBaseURL = Synchronizer::fsUrl(leftBase);
    if (leftBaseURL.isParentOf(url) && !url.isParentOf(leftBaseURL)) {
        text = QDir(leftBaseURL.path()).relativeFilePath(url.path());
        return;
    }

    QString rightBase = rightLocation->currentText();
    if (!rightBase.endsWith('/'))
        rightBase += '/';
    QUrl rightBaseURL = Synchronizer::fsUrl(rightBase);
    if (rightBaseURL.isParentOf(url) && !url.isParentOf(rightBaseURL)) {
        text = QDir(rightBaseURL.path()).relativeFilePath(url.path());
        return;
    }

    error = i18n("URL must be the descendant of either the left or the right base URL.");
}

void SynchronizerGUI::doubleClicked(QTreeWidgetItem *itemIn)
{
    if (!itemIn)
        return;

    SyncViewItem *syncItem = (SyncViewItem *)itemIn;
    SynchronizerFileItem *item = syncItem->synchronizerItemRef();
    if (item && item->existsInLeft() && item->existsInRight() && !item->isDir()) {
        QString leftDirName     = item->leftDirectory().isEmpty() ? "" : item->leftDirectory() + '/';
        QString rightDirName     = item->rightDirectory().isEmpty() ? "" : item->rightDirectory() + '/';
        QUrl leftURL = Synchronizer::fsUrl(synchronizer.leftBaseDirectory()  + leftDirName + item->leftName());
        QUrl rightURL = Synchronizer::fsUrl(synchronizer.rightBaseDirectory() + rightDirName + item->rightName());

        SLOTS->compareContent(leftURL,rightURL);
    } else if (item && item->isDir()) {
        itemIn->setExpanded(!itemIn->isExpanded());
    }
}

void SynchronizerGUI::rightMouseClicked(QTreeWidgetItem *itemIn, const QPoint &pos)
{
    // these are the values that will exist in the menu
#define EXCLUDE_ID          90
#define RESTORE_ID          91
#define COPY_TO_LEFT_ID     92
#define COPY_TO_RIGHT_ID    93
#define REVERSE_DIR_ID      94
#define DELETE_ID           95
#define VIEW_LEFT_FILE_ID   96
#define VIEW_RIGHT_FILE_ID  97
#define COMPARE_FILES_ID    98
#define SELECT_ITEMS_ID     99
#define DESELECT_ITEMS_ID   100
#define INVERT_SELECTION_ID 101
#define SYNCH_WITH_KGET_ID  102
#define COPY_CLPBD_LEFT_ID  103
#define COPY_CLPBD_RIGHT_ID 104
    //////////////////////////////////////////////////////////
    if (!itemIn)
        return;

    SyncViewItem *syncItem = (SyncViewItem *)itemIn;
    if (syncItem == 0)
        return;

    SynchronizerFileItem *item = syncItem->synchronizerItemRef();

    bool    isDuplicate = item->existsInLeft() && item->existsInRight();
    bool    isDir       = item->isDir();

    // create the menu
    QMenu popup;
    QAction *myact;
    QHash< QAction *, int > actHash;

    popup.setTitle(i18n("Synchronize Folders"));

    myact = popup.addAction(i18n("E&xclude"));
    actHash[ myact ] = EXCLUDE_ID;
    myact = popup.addAction(i18n("Restore ori&ginal operation"));
    actHash[ myact ] = RESTORE_ID;
    myact = popup.addAction(i18n("Re&verse direction"));
    actHash[ myact ] = REVERSE_DIR_ID;
    myact = popup.addAction(i18n("Copy from &right to left"));
    actHash[ myact ] = COPY_TO_LEFT_ID;
    myact = popup.addAction(i18n("Copy from &left to right"));
    actHash[ myact ] = COPY_TO_RIGHT_ID;
    myact = popup.addAction(i18n("&Delete (left single)"));
    actHash[ myact ] = DELETE_ID;

    popup.addSeparator();

    myact = popup.addAction(i18n("V&iew left file"));
    myact->setEnabled(!isDir && item->existsInLeft());
    actHash[ myact ] = VIEW_LEFT_FILE_ID;
    myact = popup.addAction(i18n("Vi&ew right file"));
    myact->setEnabled(!isDir && item->existsInRight());
    actHash[ myact ] = VIEW_RIGHT_FILE_ID;
    myact = popup.addAction(i18n("&Compare Files"));
    myact->setEnabled(!isDir && isDuplicate);
    actHash[ myact ] = COMPARE_FILES_ID;

    popup.addSeparator();

    myact = popup.addAction(i18n("C&opy selected to clipboard (left)"));
    actHash[ myact ] = COPY_CLPBD_LEFT_ID;
    myact = popup.addAction(i18n("Co&py selected to clipboard (right)"));
    actHash[ myact ] = COPY_CLPBD_RIGHT_ID;

    popup.addSeparator();

    myact = popup.addAction(i18n("&Select items"));
    actHash[ myact ] = SELECT_ITEMS_ID;
    myact = popup.addAction(i18n("Deselec&t items"));
    actHash[ myact ] = DESELECT_ITEMS_ID;
    myact = popup.addAction(i18n("I&nvert selection"));
    actHash[ myact ] = INVERT_SELECTION_ID;

    QUrl leftBDir = Synchronizer::fsUrl(synchronizer.leftBaseDirectory());
    QUrl rightBDir = Synchronizer::fsUrl(synchronizer.rightBaseDirectory());

    if (KrServices::cmdExist("kget") &&
            ((!leftBDir.isLocalFile() && rightBDir.isLocalFile() && btnLeftToRight->isChecked()) ||
             (leftBDir.isLocalFile() && !rightBDir.isLocalFile() && btnRightToLeft->isChecked()))) {
        popup.addSeparator();
        myact = popup.addAction(i18n("Synchronize with &KGet"));
        actHash[ myact ] = SYNCH_WITH_KGET_ID;
    }

    QAction * res = popup.exec(pos);

    int result = -1;
    if (actHash.contains(res))
        result = actHash[ res ];

    if (result != -1)
        executeOperation(item, result);
}

void SynchronizerGUI::executeOperation(SynchronizerFileItem *item, int op)
{
    // check out the user's option
    QString leftDirName     = item->leftDirectory().isEmpty() ? "" : item->leftDirectory() + '/';
    QString rightDirName     = item->rightDirectory().isEmpty() ? "" : item->rightDirectory() + '/';

    QUrl leftURL = Synchronizer::fsUrl(synchronizer.leftBaseDirectory()  + leftDirName + item->leftName());
    QUrl rightURL = Synchronizer::fsUrl(synchronizer.rightBaseDirectory() + rightDirName + item->rightName());

    switch (op) {
    case EXCLUDE_ID:
    case RESTORE_ID:
    case COPY_TO_LEFT_ID:
    case COPY_TO_RIGHT_ID:
    case REVERSE_DIR_ID:
    case DELETE_ID: {
        unsigned              ndx = 0;
        SynchronizerFileItem  *currentItem;

        while ((currentItem = synchronizer.getItemAt(ndx++)) != 0) {
            SyncViewItem *viewItem = (SyncViewItem *)currentItem->userData();

            if (!viewItem || !viewItem->isSelected() || viewItem->isHidden())
                continue;

            switch (op) {
            case EXCLUDE_ID:
                synchronizer.exclude(viewItem->synchronizerItemRef());
                break;
            case RESTORE_ID:
                synchronizer.restore(viewItem->synchronizerItemRef());
                break;
            case REVERSE_DIR_ID:
                synchronizer.reverseDirection(viewItem->synchronizerItemRef());
                break;
            case COPY_TO_LEFT_ID:
                synchronizer.copyToLeft(viewItem->synchronizerItemRef());
                break;
            case COPY_TO_RIGHT_ID:
                synchronizer.copyToRight(viewItem->synchronizerItemRef());
                break;
            case DELETE_ID:
                synchronizer.deleteLeft(viewItem->synchronizerItemRef());
                break;
            }
        }

        refresh();
    }
    break;
    case VIEW_LEFT_FILE_ID:
        KrViewer::view(leftURL, this);   // view the file
        break;
    case VIEW_RIGHT_FILE_ID:
        KrViewer::view(rightURL, this);   // view the file
        break;
    case COMPARE_FILES_ID:
        SLOTS->compareContent(leftURL,rightURL);
        break;
    case SELECT_ITEMS_ID:
    case DESELECT_ITEMS_ID: {
        KRQuery query = KRSpWidgets::getMask((op == SELECT_ITEMS_ID ? i18n("Select items") :
                                              i18n("Deselect items")), true, this);
        if (query.isNull())
            break;

        unsigned              ndx = 0;
        SynchronizerFileItem  *currentItem;

        while ((currentItem = synchronizer.getItemAt(ndx++)) != 0) {
            SyncViewItem *viewItem = (SyncViewItem *)currentItem->userData();

            if (!viewItem || viewItem->isHidden())
                continue;

            if (query.match(currentItem->leftName()) ||
                    query.match(currentItem->rightName()))
                viewItem->setSelected(op == SELECT_ITEMS_ID);
        }
    }
    break;
    case INVERT_SELECTION_ID: {
        unsigned              ndx = 0;
        SynchronizerFileItem  *currentItem;

        while ((currentItem = synchronizer.getItemAt(ndx++)) != 0) {
            SyncViewItem *viewItem = (SyncViewItem *)currentItem->userData();

            if (!viewItem || viewItem->isHidden())
                continue;

            viewItem->setSelected(!viewItem->isSelected());
        }
    }
    break;
    case SYNCH_WITH_KGET_ID:
        synchronizer.synchronizeWithKGet();
        closeDialog();
        break;
    case COPY_CLPBD_LEFT_ID:
        copyToClipboard(true);
        break;
    case COPY_CLPBD_RIGHT_ID:
        copyToClipboard(false);
        break;
    case -1 : return;     // the user clicked outside of the menu
    }
}

void SynchronizerGUI::closeDialog()
{
    if (isComparing) {
        stop();
        wasClosed = true;
        return;
    }

    KConfigGroup group(krConfig, "Synchronize");

    QStringList list;

    foreach(const QString &item, leftLocation->historyItems()) {
        QUrl url(item);
        // make sure no passwords are saved in config
        url.setPassword(QString());
        list << url.toDisplayString(QUrl::PreferLocalFile);
    }
    group.writeEntry("Left Folder History", list);
    list.clear();
    foreach(const QString &item, rightLocation->historyItems()) {
        QUrl url(item);
        // make sure no passwords are saved in config
        url.setPassword(QString());
        list << url.toDisplayString(QUrl::PreferLocalFile);
    }
    group.writeEntry("Right Folder History", list);

    list = fileFilter->historyItems();
    group.writeEntry("File Filter", list);

    group.writeEntry("Recurse Subdirectories", cbSubdirs->isChecked());
    group.writeEntry("Follow Symlinks", cbSymlinks->isChecked());
    group.writeEntry("Compare By Content", cbByContent->isChecked());
    group.writeEntry("Ignore Date", cbIgnoreDate->isChecked());
    group.writeEntry("Asymmetric", cbAsymmetric->isChecked());
    group.writeEntry("Ignore Case", cbIgnoreCase->isChecked());
    group.writeEntry("LeftToRight Button", btnLeftToRight->isChecked());
    group.writeEntry("Equals Button", btnEquals->isChecked());
    group.writeEntry("Differents Button", btnDifferents->isChecked());
    group.writeEntry("RightToLeft Button", btnRightToLeft->isChecked());
    group.writeEntry("Deletable Button", btnDeletable->isChecked());
    group.writeEntry("Duplicates Button", btnDuplicates->isChecked());
    group.writeEntry("Singles Button", btnSingles->isChecked());

    group.writeEntry("Scroll Results", btnScrollResults->isChecked());

    group.writeEntry("Parallel Threads", parallelThreadsSpinBox->value());

    group.writeEntry("Window Width", sizeX);
    group.writeEntry("Window Height", sizeY);
    group.writeEntry("Window Maximized", isMaximized());

    group.writeEntry("State", syncList->header()->saveState());

    QDialog::reject();

    this->deleteLater();

    if (wasSync) {
        LEFT_PANEL->func->refresh();
        RIGHT_PANEL->func->refresh();
        ACTIVE_PANEL->gui->slotFocusOnMe();
    }
}

void SynchronizerGUI::compare()
{
    KRQuery query;

    if (!filterTabs->fillQuery(&query))
        return;

    // perform some previous tests
    QString leftLocationTrimmed = leftLocation->currentText().trimmed();
    QString rightLocationTrimmed = rightLocation->currentText().trimmed();
    if (leftLocationTrimmed.isEmpty()) {
        KMessageBox::error(this, i18n("The target folder must not be empty."));
        leftLocation->setFocus();
        return;
    }
    if (rightLocationTrimmed.isEmpty()) {
        KMessageBox::error(this, i18n("The source folder must not be empty."));
        rightLocation->setFocus();
        return;
    }

    if (leftLocationTrimmed == rightLocationTrimmed) {
        if (KMessageBox::warningContinueCancel(this, i18n("Warning: The left and the right side are showing the same folder."))
            != KMessageBox::Continue) {
            return;
        }
    }

    query.setNameFilter(fileFilter->currentText(), query.isCaseSensitive());
    synchronizerTabs->setCurrentIndex(0);

    syncList->clear();
    lastItem = 0;

    leftLocation->addToHistory(leftLocation->currentText());
    rightLocation->addToHistory(rightLocation->currentText());
    fileFilter->addToHistory(fileFilter->currentText());

    setMarkFlags();

    btnCompareDirs->setEnabled(false);
    profileManager->setEnabled(false);
    btnSwapSides->setEnabled(false);
    btnStopComparing->setEnabled(isComparing = true);
    btnStopComparing->show();
    btnFeedToListBox->setEnabled(false);
    btnFeedToListBox->hide();
    btnSynchronize->setEnabled(false);
    btnCompareDirs->hide();
    btnScrollResults->show();
    disableMarkButtons();

    int fileCount = synchronizer.compare(leftLocation->currentText(), rightLocation->currentText(),
                                         &query, cbSubdirs->isChecked(), cbSymlinks->isChecked(),
                                         cbIgnoreDate->isChecked(), cbAsymmetric->isChecked(), cbByContent->isChecked(),
                                         cbIgnoreCase->isChecked(), btnScrollResults->isChecked(), selectedFiles,
                                         convertToSeconds(equalitySpinBox->value(), equalityUnitCombo->currentIndex()),
                                         convertToSeconds(timeShiftSpinBox->value(), timeShiftUnitCombo->currentIndex()),
                                         parallelThreadsSpinBox->value(), ignoreHiddenFilesCB->isChecked());
    enableMarkButtons();
    btnStopComparing->setEnabled(isComparing = false);
    btnStopComparing->hide();
    btnFeedToListBox->show();
    btnCompareDirs->setEnabled(true);
    profileManager->setEnabled(true);
    btnSwapSides->setEnabled(true);
    btnCompareDirs->show();
    btnScrollResults->hide();
    if (fileCount) {
        btnSynchronize->setEnabled(true);
        btnFeedToListBox->setEnabled(true);
    }

    syncList->setFocus();

    if (wasClosed)
        closeDialog();
}

void SynchronizerGUI::stop()
{
    synchronizer.stop();
}

void SynchronizerGUI::feedToListBox()
{
    FeedToListBoxDialog listBox(this, &synchronizer, syncList, btnEquals->isChecked());
    if (listBox.isAccepted())
        closeDialog();
}

void SynchronizerGUI::reject()
{
    closeDialog();
}

void SynchronizerGUI::addFile(SynchronizerFileItem *item)
{
    QString leftName = "", rightName = "", leftDate = "", rightDate = "", leftSize = "", rightSize = "";
    bool    isDir = item->isDir();

    QColor textColor = foreGrounds[ item->task()];
    QColor baseColor = backGrounds[ item->task()];

    if (item->existsInLeft()) {
        leftName = item->leftName();
        leftSize = isDir ? dirLabel() + ' ' : KRpermHandler::parseSize(item->leftSize());
        leftDate = SynchronizerGUI::convertTime(item->leftDate());
    }

    if (item->existsInRight()) {
        rightName = item->rightName();
        rightSize = isDir ? dirLabel() + ' ' : KRpermHandler::parseSize(item->rightSize());
        rightDate = SynchronizerGUI::convertTime(item->rightDate());
    }

    SyncViewItem *listItem = 0;
    SyncViewItem *dirItem;

    if (item->parent() == 0) {
        listItem = new SyncViewItem(item, textColor, baseColor, syncList, lastItem, leftName, leftSize,
                                    leftDate, Synchronizer::getTaskTypeName(item->task()), rightDate,
                                    rightSize, rightName);
        lastItem = listItem;
    } else {
        dirItem = (SyncViewItem *)item->parent()->userData();
        if (dirItem) {
            dirItem->setExpanded(true);
            listItem = new SyncViewItem(item, textColor, baseColor, dirItem, dirItem->lastItem(), leftName,
                                        leftSize, leftDate, Synchronizer::getTaskTypeName(item->task()),
                                        rightDate, rightSize, rightName);

            dirItem->setLastItem(listItem);
        }
    }

    if (listItem) {
        listItem->setIcon(0, QIcon::fromTheme(isDir ? "folder" : "document-new"));
        if (!item->isMarked())
            listItem->setHidden(true);
        else
            syncList->scrollTo(syncList->indexOf(listItem));
    }
}

void SynchronizerGUI::markChanged(SynchronizerFileItem *item, bool ensureVisible)
{
    SyncViewItem *listItem = (SyncViewItem *)item->userData();
    if (listItem) {
        if (!item->isMarked()) {
            listItem->setHidden(true);
        } else {
            QString leftName = "", rightName = "", leftDate = "", rightDate = "", leftSize = "", rightSize = "";
            bool    isDir = item->isDir();

            if (item->existsInLeft()) {
                leftName = item->leftName();
                leftSize = isDir ? dirLabel() + ' ' : KRpermHandler::parseSize(item->leftSize());
                leftDate = SynchronizerGUI::convertTime(item->leftDate());
            }

            if (item->existsInRight()) {
                rightName = item->rightName();
                rightSize = isDir ? dirLabel() + ' ' : KRpermHandler::parseSize(item->rightSize());
                rightDate = SynchronizerGUI::convertTime(item->rightDate());
            }

            listItem->setHidden(false);
            listItem->setText(0, leftName);
            listItem->setText(1, leftSize);
            listItem->setText(2, leftDate);
            listItem->setText(3, Synchronizer::getTaskTypeName(item->task()));
            listItem->setText(4, rightDate);
            listItem->setText(5, rightSize);
            listItem->setText(6, rightName);
            listItem->setColors(foreGrounds[ item->task()], backGrounds[ item->task()]);

            if (ensureVisible)
                syncList->scrollTo(syncList->indexOf(listItem));
        }
    }
}

void SynchronizerGUI::subdirsChecked(bool isOn)
{
    cbSymlinks->setEnabled(isOn);
}

void SynchronizerGUI::disableMarkButtons()
{
    btnLeftToRight->setEnabled(false);
    btnEquals->setEnabled(false);
    btnDifferents->setEnabled(false);
    btnRightToLeft->setEnabled(false);
    btnDeletable->setEnabled(false);
    btnDuplicates->setEnabled(false);
    btnSingles->setEnabled(false);
}

void SynchronizerGUI::enableMarkButtons()
{
    btnLeftToRight->setEnabled(true);
    btnEquals->setEnabled(true);
    btnDifferents->setEnabled(true);
    btnRightToLeft->setEnabled(true);
    btnDeletable->setEnabled(true);
    btnDuplicates->setEnabled(true);
    btnSingles->setEnabled(true);
}

QString SynchronizerGUI::convertTime(time_t time) const
{
    // convert the time_t to struct tm
    struct tm* t = localtime((time_t *) & time);

    QDateTime tmp(QDate(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday), QTime(t->tm_hour, t->tm_min));
    return QLocale().toString(tmp, QLocale::ShortFormat);
}

void SynchronizerGUI::setMarkFlags()
{
    synchronizer.setMarkFlags(btnRightToLeft->isChecked(), btnEquals->isChecked(), btnDifferents->isChecked(),
                              btnLeftToRight->isChecked(), btnDuplicates->isChecked(), btnSingles->isChecked(),
                              btnDeletable->isChecked());
}

void SynchronizerGUI::refresh()
{
    if (!isComparing) {
        syncList->clearSelection();
        setMarkFlags();
        btnCompareDirs->setEnabled(false);
        profileManager->setEnabled(false);
        btnSwapSides->setEnabled(false);
        btnSynchronize->setEnabled(false);
        btnFeedToListBox->setEnabled(false);
        disableMarkButtons();
        int fileCount = synchronizer.refresh();
        enableMarkButtons();
        btnCompareDirs->setEnabled(true);
        profileManager->setEnabled(true);
        btnSwapSides->setEnabled(true);
        if (fileCount) {
            btnFeedToListBox->setEnabled(true);
            btnSynchronize->setEnabled(true);
        }
    }
}

void SynchronizerGUI::synchronize()
{
    int             copyToLeftNr, copyToRightNr, deleteNr;
    KIO::filesize_t copyToLeftSize, copyToRightSize, deleteSize;

    if (!synchronizer.totalSizes(&copyToLeftNr, &copyToLeftSize, &copyToRightNr, &copyToRightSize,
                                 &deleteNr, &deleteSize)) {
        KMessageBox::sorry(parentWidget(), i18n("Synchronizer has nothing to do."));
        return;
    }

    SynchronizeDialog *sd = new SynchronizeDialog(this, &synchronizer,
            copyToLeftNr, copyToLeftSize, copyToRightNr,
            copyToRightSize, deleteNr, deleteSize,
            parallelThreadsSpinBox->value());

    wasSync = sd->wasSyncronizationStarted();
    delete sd;

    if (wasSync)
        closeDialog();
}

void SynchronizerGUI::resizeEvent(QResizeEvent *e)
{
    if (!isMaximized()) {
        sizeX = e->size().width();
        sizeY = e->size().height();
    }

    if (!firstResize) {
        int delta = e->size().width() - e->oldSize().width() + (e->size().width() & 1);
        int newSize = syncList->header()->sectionSize(0) + delta / 2;

        if (newSize > 20)
            syncList->header()->resizeSection(0, newSize);
    }
    firstResize = false;
    QDialog::resizeEvent(e);
}

void SynchronizerGUI::statusInfo(QString info)
{
    statusLabel->setText(info);
    qApp->processEvents();
}

void SynchronizerGUI::swapSides()
{
    if (btnCompareDirs->isEnabled()) {
        QString leftCurrent = leftLocation->currentText();
        leftLocation->lineEdit()->setText(rightLocation->currentText());
        rightLocation->lineEdit()->setText(leftCurrent);
        synchronizer.swapSides();
        refresh();
    }
}

void SynchronizerGUI::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_M : {
        if (e->modifiers() == Qt::ControlModifier) {
            syncList->setFocus();
            e->accept();
        }
        break;
    }
    case Qt::Key_F3 :
    case Qt::Key_F4 : {
        e->accept();
        syncList->setFocus();
        QTreeWidgetItem *listItem =  syncList->currentItem();
        if (listItem == 0)
            break;

        bool isedit = e->key() == Qt::Key_F4;

        SynchronizerFileItem *item = ((SyncViewItem *)listItem)->synchronizerItemRef();
        QString leftDirName = item->leftDirectory().isEmpty() ? "" : item->leftDirectory() + '/';
        QString rightDirName = item->rightDirectory().isEmpty() ? "" : item->rightDirectory() + '/';

        if (item->isDir())
            return;

        if (e->modifiers() == Qt::ShiftModifier && item->existsInRight()) {
            QUrl rightURL = Synchronizer::fsUrl(synchronizer.rightBaseDirectory() + rightDirName + item->rightName());
            if (isedit)
                KrViewer::edit(rightURL, this);   // view the file
            else
                KrViewer::view(rightURL, this);   // view the file
            return;
        } else if (e->modifiers() == 0 && item->existsInLeft()) {
            QUrl leftURL = Synchronizer::fsUrl(synchronizer.leftBaseDirectory()  + leftDirName + item->leftName());
            if (isedit)
                KrViewer::edit(leftURL, this);   // view the file
            else
                KrViewer::view(leftURL, this);   // view the file
            return;
        }
    }
    break;
    case Qt::Key_U :
        if (e->modifiers() != Qt::ControlModifier)
            break;
        e->accept();
        swapSides();
        return;
    case Qt::Key_Escape:
        if (!btnStopComparing->isHidden() && btnStopComparing->isEnabled()) { // is it comparing?
            e->accept();
            btnStopComparing->animateClick(); // just click the stop button
        } else {
            e->accept();
            if (syncList->topLevelItemCount() != 0) {
                int result = KMessageBox::warningYesNo(this, i18n("The synchronizer window contains data from a previous compare. If you exit, this data will be lost. Do you really want to exit?"),
                                                       i18n("Krusader::Synchronize Folders"),
                                                       KStandardGuiItem::yes(), KStandardGuiItem::no(), "syncGUIexit");
                if (result != KMessageBox::Yes)
                    return;
            }
            QDialog::reject();
        }
        return;
    }

    QDialog::keyPressEvent(e);
}

bool SynchronizerGUI::eventFilter(QObject * /* watched */, QEvent * e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent* ke = (QKeyEvent*) e;
        switch (ke->key()) {
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Delete:
        case Qt::Key_W: {
            if (ke->key() == Qt::Key_W) {
                if (ke->modifiers() != Qt::ControlModifier)
                    return false;
            } else if (ke->modifiers() != Qt::AltModifier)
                return false;

            int op = -1;
            switch (ke->key()) {
            case Qt::Key_Down:
                op = EXCLUDE_ID;
                break;
            case Qt::Key_Left:
                op = COPY_TO_LEFT_ID;
                break;
            case Qt::Key_Right:
                op = COPY_TO_RIGHT_ID;
                break;
            case Qt::Key_Up:
                op = RESTORE_ID;
                break;
            case Qt::Key_Delete:
                op = DELETE_ID;
                break;
            case Qt::Key_W:
                op = REVERSE_DIR_ID;
                break;
            }

            ke->accept();

            QTreeWidgetItem *listItem =  syncList->currentItem();
            if (listItem == 0)
                return true;

            SynchronizerFileItem *item = ((SyncViewItem *)listItem)->synchronizerItemRef();

            bool hasSelected = false;
            QList<QTreeWidgetItem *> selected = syncList->selectedItems();
            for (int i = 0; i != selected.count(); i++)
                if (selected[ i ]->isSelected() && !selected[ i ]->isHidden())
                    hasSelected = true;
            if (!hasSelected)
                listItem->setSelected(true);

            executeOperation(item, op);
            return true;
        }
        }
    }
    return false;
}

void SynchronizerGUI::loadFromProfile(QString profile)
{
    syncList->clear();
    synchronizer.reset();
    isComparing = wasClosed = false;
    btnSynchronize->setEnabled(false);
    btnFeedToListBox->setEnabled(false);

    KConfigGroup pg(krConfig, profile);

    if (!hasSelectedFiles) {
        leftLocation->lineEdit()->setText(pg.readEntry("Left Location", QString()));
        rightLocation->lineEdit()->setText(pg.readEntry("Right Location", QString()));
    }
    fileFilter->lineEdit()->setText(pg.readEntry("Search For", QString()));

    cbSubdirs->   setChecked(pg.readEntry("Recurse Subdirectories", true));
    cbSymlinks->  setChecked(pg.readEntry("Follow Symlinks", false));
    cbByContent-> setChecked(pg.readEntry("Compare By Content", false));
    cbIgnoreDate->setChecked(pg.readEntry("Ignore Date", false));
    cbAsymmetric->setChecked(pg.readEntry("Asymmetric", false));
    cbIgnoreCase->setChecked(pg.readEntry("Ignore Case", false));

    btnScrollResults->setChecked(pg.readEntry("Scroll Results", false));

    btnLeftToRight->setChecked(pg.readEntry("Show Left To Right", true));
    btnEquals     ->setChecked(pg.readEntry("Show Equals", true));
    btnDifferents ->setChecked(pg.readEntry("Show Differents", true));
    btnRightToLeft->setChecked(pg.readEntry("Show Right To Left", true));
    btnDeletable  ->setChecked(pg.readEntry("Show Deletable", true));
    btnDuplicates ->setChecked(pg.readEntry("Show Duplicates", true));
    btnSingles    ->setChecked(pg.readEntry("Show Singles", true));

    int equalityThreshold = pg.readEntry("Equality Threshold", 0);
    int equalityCombo = 0;
    convertFromSeconds(equalityThreshold, equalityCombo, equalityThreshold);
    equalitySpinBox->setValue(equalityThreshold);
    equalityUnitCombo->setCurrentIndex(equalityCombo);

    int timeShift = pg.readEntry("Time Shift", 0);
    int timeShiftCombo = 0;
    convertFromSeconds(timeShift, timeShiftCombo, timeShift);
    timeShiftSpinBox->setValue(timeShift);
    timeShiftUnitCombo->setCurrentIndex(timeShiftCombo);

    int parallelThreads = pg.readEntry("Parallel Threads", 1);
    parallelThreadsSpinBox->setValue(parallelThreads);

    bool ignoreHidden = pg.readEntry("Ignore Hidden Files", false);
    ignoreHiddenFilesCB->setChecked(ignoreHidden);

    refresh();
}

void SynchronizerGUI::saveToProfile(QString profile)
{
    KConfigGroup group(krConfig, profile);

    group.writeEntry("Left Location", leftLocation->currentText());
    group.writeEntry("Search For", fileFilter->currentText());
    group.writeEntry("Right Location", rightLocation->currentText());

    group.writeEntry("Recurse Subdirectories", cbSubdirs->isChecked());
    group.writeEntry("Follow Symlinks", cbSymlinks->isChecked());
    group.writeEntry("Compare By Content", cbByContent->isChecked());
    group.writeEntry("Ignore Date", cbIgnoreDate->isChecked());
    group.writeEntry("Asymmetric", cbAsymmetric->isChecked());
    group.writeEntry("Ignore Case", cbIgnoreCase->isChecked());

    group.writeEntry("Scroll Results", btnScrollResults->isChecked());

    group.writeEntry("Show Left To Right", btnLeftToRight->isChecked());
    group.writeEntry("Show Equals", btnEquals->isChecked());
    group.writeEntry("Show Differents", btnDifferents->isChecked());
    group.writeEntry("Show Right To Left", btnRightToLeft->isChecked());
    group.writeEntry("Show Deletable", btnDeletable->isChecked());
    group.writeEntry("Show Duplicates", btnDuplicates->isChecked());
    group.writeEntry("Show Singles", btnSingles->isChecked());

    group.writeEntry("Equality Threshold", convertToSeconds(equalitySpinBox->value(), equalityUnitCombo->currentIndex()));
    group.writeEntry("Time Shift", convertToSeconds(timeShiftSpinBox->value(), timeShiftUnitCombo->currentIndex()));
    group.writeEntry("Parallel Threads", parallelThreadsSpinBox->value());

    group.writeEntry("Ignore Hidden Files", ignoreHiddenFilesCB->isChecked());
}

void SynchronizerGUI::connectFilters(const QString &newString)
{
    if (synchronizerTabs->currentIndex())
        fileFilter->setEditText(newString);
    else
        generalFilter->searchFor->setEditText(newString);
}

void SynchronizerGUI::setScrolling(bool isOn)
{
    if (isOn)
        btnScrollResults->setText(i18n("Quiet"));
    else
        btnScrollResults->setText(i18n("Scroll Results"));

    synchronizer.setScrolling(isOn);
}

int SynchronizerGUI::convertToSeconds(int time, int unit)
{
    switch (unit) {
    case 1:
        return time * 60;
    case 2:
        return time * 3600;
    case 3:
        return time * 86400;
    default:
        return time;
    }
}

void SynchronizerGUI::convertFromSeconds(int &time, int &unit, int second)
{
    unit = 0;
    time = second;
    int absTime = (time < 0) ? -time : time;

    if (absTime >= 86400 && (absTime % 86400) == 0) {
        time /= 86400;
        unit = 3;
    } else if (absTime >= 3600 && (absTime % 3600) == 0) {
        time /= 3600;
        unit = 2;
    } else if (absTime >= 60 && (absTime % 60) == 0) {
        time /= 60;
        unit = 1;
    }
}

QPushButton *SynchronizerGUI::createButton(QWidget *parent, const QString &iconName, bool checked,
                                           const QKeySequence &shortCut, const QString &description,
                                           const QString &text, bool textAndIcon)
{
    QPushButton *button = new QPushButton(parent);
    if (!text.isEmpty() && (textAndIcon || !QIcon::hasThemeIcon(iconName)))
        button->setText(text);
    button->setIcon(QIcon::fromTheme(iconName));
    button->setCheckable(true);
    button->setChecked(checked);
    button->setShortcut(shortCut);
    const QString infoText =
        QString("%1 (%2)").arg(description, shortCut.toString(QKeySequence::NativeText));
    button->setWhatsThis(infoText);
    button->setToolTip(infoText);
    return button;
}

void SynchronizerGUI::copyToClipboard(bool isLeft)
{
    QList<QUrl> urls;

    unsigned              ndx = 0;
    SynchronizerFileItem  *currentItem;

    while ((currentItem = synchronizer.getItemAt(ndx++)) != 0) {
        SynchronizerGUI::SyncViewItem *viewItem = (SynchronizerGUI::SyncViewItem *)currentItem->userData();

        if (!viewItem || !viewItem->isSelected() || viewItem->isHidden())
            continue;

        SynchronizerFileItem *item = viewItem->synchronizerItemRef();
        if (item) {
            if (isLeft && item->existsInLeft()) {
                QString leftDirName = item->leftDirectory().isEmpty() ? "" : item->leftDirectory() + '/';
                QUrl leftURL = Synchronizer::fsUrl(synchronizer.leftBaseDirectory()  + leftDirName + item->leftName());
                urls.push_back(leftURL);
            } else if (!isLeft && item->existsInRight()) {
                QString rightDirName = item->rightDirectory().isEmpty() ? "" : item->rightDirectory() + '/';
                QUrl rightURL = Synchronizer::fsUrl(synchronizer.rightBaseDirectory()  + rightDirName + item->rightName());
                urls.push_back(rightURL);
            }
        }
    }

    if (urls.count() == 0)
        return;

    QMimeData *mimeData = new QMimeData;
    mimeData->setImageData(FL_LOADICON(isLeft ? "arrow-left-double" : "arrow-right-double"));
    mimeData->setUrls(urls);

    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

QString SynchronizerGUI::dirLabel()
{
    //HACK add <> brackets AFTER translating - otherwise KUIT thinks it's a tag
    static QString label = QString("<") +
        i18nc("Show the string 'DIR' instead of file size in detailed view (for folders)", "DIR") + '>';
    return label;
}

