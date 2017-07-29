/***************************************************************************
                         locate.cpp  -  description
                             -------------------
    copyright            : (C) 2004 by Csaba Karai
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

#include "locate.h"
#include "../kractions.h"
#include "../krglobal.h"
#include "../krslots.h"
#include "../krusaderview.h"
#include "../Panel/krpanel.h"
#include "../Panel/panelfunc.h"
#include "../GUI/krtreewidget.h"
#include "../defaults.h"
#include "../krservices.h"
#include "../FileSystem/filesystem.h"
#include "../FileSystem/virtualfilesystem.h"
#include "../KViewer/krviewer.h"
#include "../panelmanager.h"
#include "../kicons.h"

// QtCore
#include <QRegExp>
#include <QEventLoop>
#include <QDir>
#include <QMimeData>
// QtGui
#include <QFontMetrics>
#include <QKeyEvent>
#include <QCursor>
#include <QClipboard>
#include <QFont>
#include <QDrag>
// QtWidgets
#include <QApplication>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QTreeWidget>

#include <KConfigCore/KConfig>
#include <KCoreAddons/KProcess>
#include <KI18n/KLocalizedString>
#include <KIOCore/KFileItem>
#include <KWidgetsAddons/KMessageBox>
#include <KCoreAddons/KShell>
#include <KTextWidgets/KFindDialog>
#include <KTextWidgets/KFind>

// these are the values that will exist in the menu
#define VIEW_ID                     90
#define EDIT_ID                     91
#define FIND_ID                     92
#define FIND_NEXT_ID                93
#define FIND_PREV_ID                94
#define COPY_SELECTED_TO_CLIPBOARD  95
#define COMPARE_ID                  96
//////////////////////////////////////////////////////////

class LocateListView : public KrTreeWidget
{
public:
    explicit LocateListView(QWidget * parent) : KrTreeWidget(parent) {
        setAlternatingRowColors(true);
    }

    void startDrag(Qt::DropActions supportedActs) {
        Q_UNUSED(supportedActs);

        QList<QUrl> urls;
        QList<QTreeWidgetItem *> list = selectedItems();

        QListIterator<QTreeWidgetItem *> it(list);

        while (it.hasNext()) {
            QTreeWidgetItem * item = it.next();

            urls.push_back(QUrl::fromLocalFile(item->text(0)));
        }

        if (urls.count() == 0)
            return;


        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setImageData(FL_LOADICON("file"));
        mimeData->setUrls(urls);
        drag->setMimeData(mimeData);
        drag->start();
    }
};

KProcess *  LocateDlg::updateProcess = 0;
LocateDlg * LocateDlg::LocateDialog = 0;

LocateDlg::LocateDlg() : QDialog(0), isFeedToListBox(false)
{
    setWindowTitle(i18n("Krusader::Locate"));
    setWindowModality(Qt::NonModal);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(6);
    grid->setContentsMargins(11, 11, 11, 11);

    QWidget *hboxWidget = new QWidget(this);
    QHBoxLayout *hbox = new QHBoxLayout(hboxWidget);
    hbox->setContentsMargins(0, 0, 0, 0);

    QLabel *label = new QLabel(i18n("Search for:"), hboxWidget);
    hbox->addWidget(label);

    locateSearchFor = new KHistoryComboBox(false, hboxWidget);
    locateSearchFor->setMinimumContentsLength(10);
    hbox->addWidget(locateSearchFor);

    label->setBuddy(locateSearchFor);
    KConfigGroup group(krConfig, "Locate");
    QStringList list = group.readEntry("Search For", QStringList());
    locateSearchFor->setMaxCount(25);  // remember 25 items
    locateSearchFor->setHistoryItems(list);
    locateSearchFor->setEditable(true);
    locateSearchFor->setDuplicatesEnabled(false);
    locateSearchFor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    locateSearchFor->lineEdit()->setFocus();

    grid->addWidget(hboxWidget, 0, 0);

    QWidget *hboxWidget2 = new QWidget(this);
    QHBoxLayout * hbox2 = new QHBoxLayout(hboxWidget2);
    hbox2->setContentsMargins(0, 0, 0, 0);

    QSpacerItem* spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hbox2->addItem(spacer);

    dontSearchInPath = new QCheckBox(i18n("Do not search in path"), hboxWidget2);
    hbox2->addWidget(dontSearchInPath);
    dontSearchInPath->setChecked(group.readEntry("Don't Search In Path", false));

    existingFiles = new QCheckBox(i18n("Show only the existing files"), hboxWidget2);
    existingFiles->setChecked(group.readEntry("Existing Files", false));
    hbox2->addWidget(existingFiles);

    caseSensitive = new QCheckBox(i18n("Case Sensitive"), hboxWidget2);
    caseSensitive->setChecked(group.readEntry("Case Sensitive", false));
    hbox2->addWidget(caseSensitive);

    grid->addWidget(hboxWidget2, 1, 0);

    QFrame *line1 = new QFrame(this);
    line1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    grid->addWidget(line1, 2, 0);

    resultList = new LocateListView(this);  // create the main container
    resultList->setColumnCount(1);
    resultList->setHeaderLabel(i18n("Results"));

    resultList->setColumnWidth(0, QFontMetrics(resultList->font()).width("W") * 60);

    KConfigGroup gl(krConfig, "Look&Feel");
    resultList->setFont(gl.readEntry("Filelist Font", _FilelistFont));

    resultList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    resultList->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    resultList->header()->setSortIndicatorShown(false);
    resultList->setSortingEnabled(false);
    resultList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    resultList->setDragEnabled(true);

    connect(resultList, SIGNAL(itemRightClicked(QTreeWidgetItem*,QPoint,int)),
            this, SLOT(slotRightClick(QTreeWidgetItem*,QPoint)));
    connect(resultList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotDoubleClick(QTreeWidgetItem*)));
    connect(resultList, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(slotDoubleClick(QTreeWidgetItem*)));

    grid->addWidget(resultList, 3, 0);

    QFrame *line2 = new QFrame(this);
    line2->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    grid->addWidget(line2, 4, 0);

    mainLayout->addLayout(grid);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    mainLayout->addWidget(buttonBox);

    locateButton = new QPushButton(i18n("Locate"));
    locateButton->setIcon(QIcon::fromTheme(QStringLiteral("system-search")));
    locateButton->setDefault(true);
    buttonBox->addButton(locateButton, QDialogButtonBox::ActionRole);

    updateDbButton = new QPushButton(i18n("Update DB"));
    updateDbButton->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    buttonBox->addButton(updateDbButton, QDialogButtonBox::ActionRole);

    feedStopButton = new QPushButton;
    buttonBox->addButton(feedStopButton, QDialogButtonBox::ActionRole);

    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(locateButton, SIGNAL(clicked()), this, SLOT(slotLocate()));
    connect(updateDbButton, SIGNAL(clicked()), this, SLOT(slotUpdateDb()));
    connect(feedStopButton, SIGNAL(clicked()), this, SLOT(slotFeedStop()));

    updateButtons(false);

    if (updateProcess) {
        if (updateProcess->state() == QProcess::Running) {
            connect(updateProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(updateFinished()));
            updateDbButton->setEnabled(false);
        } else
            updateFinished();
    }

    show();

    LocateDialog = this;
}

void LocateDlg::slotFeedStop()   /* The stop / feed to listbox button */
{
    if (isFeedToListBox)
        feedToListBox();
    else
        locateProc->kill();
}

void LocateDlg::slotUpdateDb()   /* The Update DB button */
{
    if (!updateProcess) {
        KConfigGroup group(krConfig, "Locate");

        updateProcess = new KProcess(); // don't set the parent to 'this'! That would cause this process to be deleted once the dialog is closed
        *updateProcess << KrServices::fullPathName("updatedb");
        *updateProcess << KShell::splitArgs(group.readEntry("UpdateDB Arguments"));

        connect(updateProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(updateFinished()));
        updateProcess->start();
        updateDbButton->setEnabled(false);
    }
}

void LocateDlg::updateFinished()
{
    delete updateProcess;
    updateProcess = 0;
    updateDbButton->setEnabled(true);
}

void LocateDlg::slotLocate()   /* The locate button */
{
    locateSearchFor->addToHistory(locateSearchFor->currentText());
    QStringList list = locateSearchFor->historyItems();
    KConfigGroup group(krConfig, "Locate");
    group.writeEntry("Search For", list);
    group.writeEntry("Don't Search In Path", dontSearchPath = dontSearchInPath->isChecked());
    group.writeEntry("Existing Files", onlyExist = existingFiles->isChecked());
    group.writeEntry("Case Sensitive", isCs = caseSensitive->isChecked());

    if (!KrServices::cmdExist("locate")) {
        KMessageBox::error(0,
                           i18n("Cannot start 'locate'. Check the 'Dependencies' page in konfigurator."));
        return;
    }

    resultList->clear();
    lastItem = 0;
    remaining = "";

    updateButtons(true);

    isFeedToListBox = false;
    resultList->setFocus();

    qApp->processEvents(); //FIXME - whats's this for ?

    locateProc = new KProcess(this);
    locateProc->setOutputChannelMode(KProcess::SeparateChannels); // default is forwarding to the parent channels
    connect(locateProc, SIGNAL(readyReadStandardOutput()), SLOT(processStdout()));
    connect(locateProc, SIGNAL(readyReadStandardError()), SLOT(processStderr()));
    connect(locateProc, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(locateFinished()));
    connect(locateProc, SIGNAL(error(QProcess::ProcessError)), SLOT(locateError()));

    *locateProc << KrServices::fullPathName("locate");
    if (!isCs)
        *locateProc << "-i";
    *locateProc << (pattern = locateSearchFor->currentText());

    if (!pattern.startsWith('*'))
        pattern = '*' + pattern;
    if (!pattern.endsWith('*'))
        pattern = pattern + '*';

    collectedErr = "";
    locateProc->start();
}

void LocateDlg::locateError()
{
    if (locateProc->error() == QProcess::FailedToStart)
        KMessageBox::error(krMainWindow, i18n("Error during the start of 'locate' process."));
}

void LocateDlg::locateFinished()
{
    if (locateProc->exitStatus() != QProcess::NormalExit || locateProc->exitStatus()) {
        if (!collectedErr.isEmpty())
            KMessageBox::error(krMainWindow, i18n("Locate produced the following error message:\n\n%1", collectedErr));
    }

    if (resultList->topLevelItemCount() == 0) {
        locateSearchFor->setFocus();
        isFeedToListBox = false;
    } else {
        isFeedToListBox = true;
    }

    updateButtons(false);
}

void LocateDlg::processStdout()
{
    remaining += QString::fromLocal8Bit(locateProc->readAllStandardOutput());

    QStringList list = remaining.split('\n');
    int items = list.size();

    for (QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
        if (--items == 0 && !remaining.endsWith('\n'))
            remaining = *it;
        else {
            if (dontSearchPath) {
                QRegExp regExp(pattern, isCs ? Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::Wildcard);
                QString fileName = (*it).trimmed();
                if (fileName.endsWith(QLatin1String("/")) && fileName != "/") {
                    fileName.truncate(fileName.length() - 1);
                }
                fileName = fileName.mid(fileName.lastIndexOf('/') + 1);

                if (!regExp.exactMatch(fileName))
                    continue;
            }
            if (onlyExist) {
                KFileItem file(QUrl::fromLocalFile((*it).trimmed()));
                if (!file.isReadable())
                    continue;
            }

            if (lastItem)
                lastItem = new QTreeWidgetItem(resultList, lastItem);
            else
                lastItem = new QTreeWidgetItem(resultList);

            lastItem->setText(0, *it);
        }
    }
}

void LocateDlg::processStderr()
{
    collectedErr += QString::fromLocal8Bit(locateProc->readAllStandardError());
}

void LocateDlg::slotRightClick(QTreeWidgetItem *item, const QPoint &pos)
{
    if (!item)
        return;

    // create the menu
    QMenu popup;
    popup.setTitle(i18nc("@title:menu", "Locate"));

    QAction * actView = popup.addAction(i18n("View (F3)"));
    QAction * actEdit = popup.addAction(i18n("Edit (F4)"));
    QAction * actComp = popup.addAction(i18n("Compare by content (F10)"));
    if (resultList->selectedItems().count() != 2)
        actComp->setEnabled(false);
    popup.addSeparator();

    QAction * actFind = popup.addAction(i18n("Find (Ctrl+F)"));
    QAction * actNext = popup.addAction(i18n("Find next (Ctrl+N)"));
    QAction * actPrev = popup.addAction(i18n("Find previous (Ctrl+P)"));
    popup.addSeparator();

    QAction * actClip = popup.addAction(i18n("Copy selected to clipboard"));


    QAction * result = popup.exec(pos);

    int ret = -1;

    if (result == actView)
        ret = VIEW_ID;
    else if (result == actEdit)
        ret = EDIT_ID;
    else if (result == actFind)
        ret = FIND_ID;
    else if (result == actNext)
        ret = FIND_NEXT_ID;
    else if (result == actPrev)
        ret = FIND_PREV_ID;
    else if (result == actClip)
        ret = COPY_SELECTED_TO_CLIPBOARD;
    else if (result == actComp)
        ret = COMPARE_ID;

    if (ret != - 1)
        operate(item, ret);
}

void LocateDlg::slotDoubleClick(QTreeWidgetItem *item)
{
    if (!item)
        return;

    QString dirName = item->text(0);
    QString fileName;

    if (!QDir(dirName).exists()) {
        fileName = dirName.mid(dirName.lastIndexOf('/') + 1);
        dirName.truncate(dirName.lastIndexOf('/'));
    }

    ACTIVE_FUNC->openUrl(QUrl::fromLocalFile(dirName), fileName);
    QDialog::accept();
}

void LocateDlg::keyPressEvent(QKeyEvent *e)
{
    if (KrGlobal::copyShortcut == QKeySequence(e->key() | e->modifiers())) {
        operate(0, COPY_SELECTED_TO_CLIPBOARD);
        e->accept();
        return;
    }

    switch (e->key()) {
    case Qt::Key_M :
        if (e->modifiers() == Qt::ControlModifier) {
            resultList->setFocus();
            e->accept();
        }
        break;
    case Qt::Key_F3 :
        if (resultList->currentItem())
            operate(resultList->currentItem(), VIEW_ID);
        break;
    case Qt::Key_F4 :
        if (resultList->currentItem())
            operate(resultList->currentItem(), EDIT_ID);
        break;
    case Qt::Key_F10 :
        operate(0, COMPARE_ID);
        break;
    case Qt::Key_N :
        if (e->modifiers() == Qt::ControlModifier)
            operate(resultList->currentItem(), FIND_NEXT_ID);
        break;
    case Qt::Key_P :
        if (e->modifiers() == Qt::ControlModifier)
            operate(resultList->currentItem(), FIND_PREV_ID);
        break;
    case Qt::Key_F :
        if (e->modifiers() == Qt::ControlModifier)
            operate(resultList->currentItem(), FIND_ID);
        break;
    }

    QDialog::keyPressEvent(e);
}

void LocateDlg::operate(QTreeWidgetItem *item, int task)
{
    QUrl name;
    if (item != 0)
        name = QUrl::fromLocalFile(item->text(0));

    switch (task) {
    case VIEW_ID:
        KrViewer::view(name, this);   // view the file
        break;
    case EDIT_ID:
        KrViewer::edit(name, this);   // view the file
        break;
    case COMPARE_ID: {
        QList<QTreeWidgetItem *> list = resultList->selectedItems();
        if (list.count() != 2)
            break;

        QUrl url1 = QUrl::fromLocalFile(list[ 0 ]->text(0));
        QUrl url2 = QUrl::fromLocalFile(list[ 1 ]->text(0));

        SLOTS->compareContent(url1,url2);
    }
    break;
    case FIND_ID: {
        KConfigGroup group(krConfig, "Locate");
        long options = group.readEntry("Find Options", (long long)0);
        QStringList list = group.readEntry("Find Patterns", QStringList());

        QPointer<KFindDialog> dlg = new KFindDialog(this, options, list);
        if (dlg->exec() != QDialog::Accepted) {
            delete dlg;
            return;
        }

        QString first;
        if (list.count() != 0) {
            first = list.first();
        }
        if (first != (findPattern = dlg->pattern())) {
            list.push_front(dlg->pattern());
        }

        group.writeEntry("Find Options", (long long)(findOptions = dlg->options()));
        group.writeEntry("Find Patterns", list);

        if (!(findOptions & KFind::FromCursor) && resultList->topLevelItemCount())
            resultList->setCurrentItem((findOptions & KFind::FindBackwards) ?
                                       resultList->topLevelItem(resultList->topLevelItemCount() - 1) : resultList->topLevelItem(0));

        findCurrentItem = resultList->currentItem();

        if (find() && findCurrentItem) {
            resultList->selectionModel()->clearSelection(); // HACK: QT 4 is not able to paint the focus frame because of a bug
            resultList->setCurrentItem(findCurrentItem);
        } else {
            KMessageBox::information(this, i18n("Search string not found."));
        }

        resultList->scrollTo(resultList->currentIndex());

        delete dlg;
    }
    break;
    case FIND_NEXT_ID:
    case FIND_PREV_ID: {
        if (task == FIND_PREV_ID)
            findOptions ^= KFind::FindBackwards;

        findCurrentItem = resultList->currentItem();
        nextLine();

        if (find() && findCurrentItem) {
            resultList->selectionModel()->clearSelection(); // HACK: QT 4 is not able to paint the focus frame because of a bug
            resultList->setCurrentItem(findCurrentItem);
        } else
            KMessageBox::information(this, i18n("Search string not found."));

        resultList->scrollTo(resultList->currentIndex());

        if (task == FIND_PREV_ID)
            findOptions ^= KFind::FindBackwards;
    }
    break;
    case COPY_SELECTED_TO_CLIPBOARD: {
        QList<QUrl> urls;

        QTreeWidgetItemIterator it(resultList);
        while (*it) {
            if ((*it)->isSelected())
                urls.push_back(QUrl::fromLocalFile((*it)->text(0)));

            it++;
        }

        if (urls.count() == 0)
            return;

        QMimeData *mimeData = new QMimeData;
        mimeData->setImageData(FL_LOADICON("file"));
        mimeData->setUrls(urls);

        QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
    }
    break;
    }
}

void LocateDlg::nextLine()
{
    if (findOptions & KFind::FindBackwards)
        findCurrentItem = resultList->itemAbove(findCurrentItem);
    else
        findCurrentItem = resultList->itemBelow(findCurrentItem);
}

bool LocateDlg::find()
{
    while (findCurrentItem) {
        QString item = findCurrentItem->text(0);

        if (findOptions & KFind::RegularExpression) {
            if (item.contains(QRegExp(findPattern, ((findOptions & KFind::CaseSensitive) != 0) ? Qt::CaseSensitive : Qt::CaseInsensitive)))
                return true;
        } else {
            if (item.contains(findPattern, ((findOptions & KFind::CaseSensitive) != 0) ? Qt::CaseSensitive : Qt::CaseInsensitive))
                return true;
        }

        nextLine();
    }

    return false;
}

void LocateDlg::feedToListBox()
{
    VirtualFileSystem virtFilesystem;
    virtFilesystem.scanDir(QUrl::fromLocalFile(QStringLiteral("/")));

    KConfigGroup group(krConfig, "Locate");
    int listBoxNum = group.readEntry("Feed To Listbox Counter", 1);
    QString queryName;
    do {
        queryName = i18n("Locate results") + QString(" %1").arg(listBoxNum++);
    } while (virtFilesystem.getFileItem(queryName) != 0);
    group.writeEntry("Feed To Listbox Counter", listBoxNum);

    KConfigGroup ga(krConfig, "Advanced");
    if (ga.readEntry("Confirm Feed to Listbox",  _ConfirmFeedToListbox)) {
        bool ok;
        queryName = QInputDialog::getText(this, i18n("Query Name"), i18n("Here you can name the file collection:"),
                                          QLineEdit::Normal, queryName, &ok);
        if (! ok)
            return;
    }

    QList<QUrl> urlList;
    QTreeWidgetItemIterator it(resultList);
    while (*it) {
        QTreeWidgetItem * item = *it;
        urlList.push_back(QUrl::fromLocalFile(item->text(0)));
        it++;
    }
    QUrl url = QUrl(QStringLiteral("virt:/") + queryName);
    virtFilesystem.refresh(url); // create directory if it does not exist
    virtFilesystem.addFiles(urlList);
    //ACTIVE_FUNC->openUrl(url);
    ACTIVE_MNG->slotNewTab(url);
    accept();
}

void LocateDlg::reset()
{
    locateSearchFor->lineEdit()->setFocus();
    locateSearchFor->lineEdit()->selectAll();
}

void LocateDlg::updateButtons(bool locateIsRunning)
{
    locateButton->setEnabled(!locateIsRunning);

    if (locateIsRunning) {
        feedStopButton->setEnabled(true);
        feedStopButton->setText(i18n("Stop"));
        feedStopButton->setIcon(QIcon::fromTheme(QStringLiteral("process-stop")));
    } else {
        if (resultList->topLevelItemCount() == 0) {
            feedStopButton->setEnabled(false);
            feedStopButton->setText(i18n("Stop"));
            feedStopButton->setIcon(QIcon::fromTheme(QStringLiteral("process-stop")));
        } else {
            feedStopButton->setEnabled(true);
            feedStopButton->setText(i18n("Feed to listbox"));
            feedStopButton->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
        }
    }
}

