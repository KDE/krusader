/***************************************************************************
                                 krsearchdialog.cpp
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
    web site		 : http://krusader.sourceforge.net
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

#define USERSFILE  QString("/etc/passwd")
#define GROUPSFILE QString("/etc/group")


#include "../krusader.h"
#include "../VFS/vfs.h"
#include "../krusaderview.h"
#include "../Panel/listpanel.h"
#include "../Panel/panelfunc.h"
#include "../resources.h"
#include "../defaults.h"
#include "../Dialogs/krdialogs.h"
#include "../VFS/krpermhandler.h"
#include "krsearchmod.h"
#include "krsearchdialog.h"
#include <time.h>
#include <pwd.h>
#include <sys/types.h>
#include <kglobal.h>
#include <qtabwidget.h>
#include <qstring.h>
#include <qradiobutton.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qmultilineedit.h>
#include <qdatetime.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qlistview.h>
#include <qfontmetrics.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <kfiledialog.h>
#include <kdatetbl.h>
#include <klocale.h>
#include <kdeversion.h>

// private functions - used as services /////////////////////
void changeDate(QLineEdit *p) {
  // check if the current date is valid
  QDate d = KGlobal::locale()->readDate(p->text());
  if (!d.isValid()) d = QDate::currentDate();

  KRGetDate *gd = new KRGetDate(d);
  d = gd->getDate();
  // if a user pressed ESC or closed the dialog, we'll return an invalid date
  if (d.isValid())
    p->setText(KGlobal::locale()->formatDate(d, true));
  delete gd;
}

// bool start: set it to true if this date is the beginning of the search,
// if it's the end, set it to false
void qdate2time_t(time_t *dest, QDate d, bool start) {
  struct tm t;
  t.tm_sec   = (start ? 0 : 59);
  t.tm_min   = (start ? 0 : 59);
  t.tm_hour  = (start ? 0 : 23);
  t.tm_mday  = d.day();
  t.tm_mon   = d.month() - 1;
  t.tm_year  = d.year() - 1900;
  t.tm_wday  = d.dayOfWeek() - 1; // actually ignored by mktime
  t.tm_yday  = d.dayOfYear() - 1; // actually ignored by mktime
  t.tm_isdst = -1; // daylight saving time information isn't availble

  (*dest) = mktime( &t );
}

// class starts here /////////////////////////////////////////
KrSearchDialog::KrSearchDialog(QWidget *parent, const char *name ) :
                KrSearchBase(parent,name), query(0), searcher(0) {
  prepareGUI();
  show();
  // disable the search action ... no 2 searchers !
  krFind->setEnabled(false);
  searchFor->setFocus();
	resultsList->setColumnAlignment(2, AlignRight);
}

KrSearchDialog::~KrSearchDialog(){
}

void KrSearchDialog::invalidDateMessage(QLineEdit *p) {
	KMessageBox::detailedError(0, i18n("Invalid date entered."),
                             i18n("The date '") + p->text() + i18n("' is not valid according to your locale.\n"
                             "Please re-enter a valid date (use the date button of easy access)."));

	TabWidget2->setCurrentPage(1); // set page to advanced
  p->setFocus();
}

void KrSearchDialog::prepareGUI() {
  //=======> to be moved ...
  resultsList->setSorting(1); // sort by location
  belongsToUserData->setEditable(false); belongsToGroupData->setEditable(false);
	searchInArchives->setEnabled(true);
	connect(searchInArchives, SIGNAL(toggled(bool)), containsText, SLOT(setDisabled(bool)));
	connect(searchInArchives, SIGNAL(toggled(bool)), containsTextCase, SLOT(setDisabled(bool)));
	
  ofType->insertItem(i18n("All Files"));
  ofType->insertItem(i18n("Archives"));
  ofType->insertItem(i18n("Directories"));
  ofType->insertItem(i18n("Image Files"));
  ofType->insertItem(i18n("Text Files"));
  ofType->insertItem(i18n("Video Files"));
  ofType->insertItem(i18n("Audio Files"));

  ofType->setEnabled(true);
  // ========================================

  // additional connections ... adds history support etc.
  connect(searchFor, SIGNAL(activated(const QString&)),
          searchFor, SLOT(addToHistory(const QString&)));
  connect(containsText, SIGNAL(activated(const QString&)),
          containsText, SLOT(addToHistory(const QString&)));

  // add shell completion
  completion.setMode(KURLCompletion::FileCompletion);
  searchInEdit->setCompletionObject(&completion);
  dontSearchInEdit->setCompletionObject(&completion);

  // fix the results list
  // => make the results font smaller
  QFont resultsFont(  resultsList->font() );
  resultsFont.setPointSize(resultsFont.pointSize()-1);
  resultsList->setFont(resultsFont);

  resultsList->setAllColumnsShowFocus(true);
  for (int i=0; i<5; ++i) // don't let it resize automatically
    resultsList->setColumnWidthMode(i, QListView::Manual);
  int i=QFontMetrics(resultsList->font()).width("W");
	int j=QFontMetrics(resultsList->font()).width("0");
	j=(i>j ? i : j);
	resultsList->setColumnWidth(0, j*14);
	resultsList->setColumnWidth(1, j*25);
	resultsList->setColumnWidth(2, j*6);
	resultsList->setColumnWidth(3, j*7);
  resultsList->setColumnWidth(4, j*7);

  // clear the searchin lists
  searchIn->clear(); dontSearchIn->clear();

	// the path in the active panel should be the default search location
	if (krApp->mainView->activePanel->func->files()->vfs_getType() != "ftp") {
		QString path = krApp->mainView->activePanel->getPath();
		// if we're inside an archive, show its directory
		int i = path.find('\\');
		if (i>=0) path = path.left(path.findRev('/', i));
		searchInEdit->setText(path);
	}

  // load the completion and history lists
  // ==> search for
  krConfig->setGroup("Search");
  QStringList list = krConfig->readListEntry("SearchFor Completion");
  searchFor->completionObject()->setItems(list);
  list = krConfig->readListEntry("SearchFor History");
  searchFor->setHistoryItems(list);
  // ==> grep
  krConfig->setGroup("Search");
  list = krConfig->readListEntry("ContainsText Completion");
  containsText->completionObject()->setItems(list);
  list = krConfig->readListEntry("ContainsText History");
  containsText->setHistoryItems(list);

  // fill the users and groups list
  fillList(belongsToUserData, USERSFILE);
  fillList(belongsToGroupData, GROUPSFILE);

  // fill the saved searches list
  refreshSavedSearches();
}

void KrSearchDialog::refreshSavedSearches() {
}

void KrSearchDialog::closeDialog() {
  // stop the search if it's on-going
  if (searcher!=0) {
    delete searcher;
    searcher = 0;
  }
  hide();
  // save the history combos
  // ==> search for
  QStringList list = searchFor->completionObject()->items();
  krConfig->setGroup("Search");
  krConfig->writeEntry("SearchFor Completion", list);
  list = searchFor->historyItems();
  krConfig->writeEntry("SearchFor History", list);
  // ==> grep text
  list = containsText->completionObject()->items();
  krConfig->setGroup("Search");
  krConfig->writeEntry("ContainsText Completion", list);
  list = containsText->historyItems();
  krConfig->writeEntry("ContainsText History", list);

  krConfig->sync();
  // re-enable the search action
  krFind->setEnabled(true);
  accept();
}

void KrSearchDialog::reject() {
  closeDialog();
  KrSearchBase::reject();
}

void KrSearchDialog::addToSearchIn() {
  QString dir = KFileDialog::getExistingDirectory();
  if (dir==QString::null) return;
  searchInEdit->setText(dir);
  searchInEdit->setFocus();
}

void KrSearchDialog::addToSearchInManually() {
  searchIn->append(searchInEdit->text());
  searchInEdit->clear();
}

void KrSearchDialog::addToDontSearchIn() {
  QString dir = KFileDialog::getExistingDirectory();
  if (dir==QString::null) return;
  dontSearchInEdit->setText(dir);
  dontSearchInEdit->setFocus();
}

void KrSearchDialog::addToDontSearchInManually() {
  dontSearchIn->append(dontSearchInEdit->text());
  dontSearchInEdit->clear();
}

void KrSearchDialog::found(QString what, QString where, KIO::filesize_t size, time_t mtime, QString perm){
  // convert the time_t to struct tm
  struct tm* t=localtime((time_t *)&mtime);
  QDateTime tmp(QDate(t->tm_year+1900, t->tm_mon+1, t->tm_mday), QTime(t->tm_hour, t->tm_min));
  new QListViewItem(resultsList, what, where.replace(QRegExp("\\\\"),"#"),
                    KRpermHandler::parseSize(size), KGlobal::locale()->formatDateTime(tmp), perm);
  QString totals = QString("Found %1 matches.").arg(resultsList->childCount());
  foundLabel->setText(totals);
}

void KrSearchDialog::query2gui() {
  QString tmp;
  // matches

  for ( QStringList::Iterator it = query->matches.begin();
        it != query->matches.end(); ++it )
    tmp += ((tmp == QString::null ? QString::null : QChar(' ')) + (*it));
  searchFor->insertItem(tmp,0);
}

bool KrSearchDialog::gui2query() {
  // prepare the query ...
  /////////////////// names, locations and greps
  if (query!=0) { delete query; query = 0; }
  query = new KRQuery();
  query->matches = QStringList::split(QChar(' '),searchFor->currentText());
  query->matchesCaseSensitive = searchForCase->isChecked();
  if (containsText->isEnabled())
    query->contain = containsText->currentText();
  query->containCaseSensetive = containsTextCase->isChecked();
  query->inArchive = searchInArchives->isChecked();
  query->recurse = searchInDirs->isChecked();
  query->followLinks = followLinks->isChecked();
  if (ofType->currentText()!=i18n("All Files"))
    query->type = ofType->currentText();
  else query->type = QString::null;
  // create the lists
  query->whereToSearch = QStringList::split(QChar('\n'), searchIn->text().simplifyWhiteSpace());
  if (!searchInEdit->text().simplifyWhiteSpace().isEmpty())
    query->whereToSearch.append(searchInEdit->text().simplifyWhiteSpace());
  query->whereNotToSearch = QStringList::split(QChar('\n'), dontSearchIn->text());
  if (!dontSearchInEdit->text().simplifyWhiteSpace().isEmpty())
    query->whereNotToSearch.append(dontSearchInEdit->text().simplifyWhiteSpace());

  // check that we have (at least) what to search, and where to search in
  if (searchFor->currentText().simplifyWhiteSpace().isEmpty()) {
    KMessageBox::error(0,i18n("No search criteria entered !"));
    TabWidget2->setCurrentPage(0); // set page to general
    searchFor->setFocus();
    return false;
  }
  if (query->whereToSearch.isEmpty()) { // we need a place to search in
    KMessageBox::error(0,i18n("Please specify a location to search in."));
    TabWidget2->setCurrentPage(0); // set page to general
    searchInEdit->setFocus();
    return false;
  }

  // size calculations ////////////////////////////////////////////////
  if ( biggerThanEnabled->isChecked() &&
      !(biggerThanAmount->text().simplifyWhiteSpace()).isEmpty() ) {
    query->minSize = biggerThanAmount->text().toULong();
    switch (biggerThanType->currentItem()) {
      case 1 : query->minSize *= 1024;
               break;
      case 2 : query->minSize *= (1024*1024);
               break;
    }
  }
  if ( smallerThanEnabled->isChecked() &&
      !(smallerThanAmount->text().simplifyWhiteSpace()).isEmpty()) {
    query->maxSize = smallerThanAmount->text().toULong();
    switch (smallerThanType->currentItem()) {
      case 1 : query->maxSize *= 1024;
               break;
      case 2 : query->maxSize *= (1024*1024);
               break;
    }
  }
  // check that minSize is smaller than maxSize
  if ((query->minSize > 0) && (query->maxSize > 0) && (query->maxSize < query->minSize)) {
		KMessageBox::detailedError(0, i18n("Specified sizes are inconsistent !"),
      i18n("Please re-enter the values, so that the leftmost size will\n"
           "be smaller (or equal) to the right size."));
		TabWidget2->setCurrentPage(1); // set page to advanced
    biggerThanAmount->setFocus();
    return false;
  }

  // date calculations ////////////////////////////////////////////////////
  if (modifiedBetweenEnabled->isChecked()) {
    // first, if both dates are empty, than don't use them
    if ( !(modifiedBetweenData1->text().simplifyWhiteSpace().isEmpty() &&
          modifiedBetweenData2->text().simplifyWhiteSpace().isEmpty()) ) {
      // check if date is valid
      QDate d1 = KGlobal::locale()->readDate(modifiedBetweenData1->text());
      if (!d1.isValid()) { invalidDateMessage(modifiedBetweenData1); return false; }
      QDate d2 = KGlobal::locale()->readDate(modifiedBetweenData2->text());
      if (!d2.isValid()) { invalidDateMessage(modifiedBetweenData2); return false; }

      if (d1 > d2) {
        KMessageBox::detailedError(0, i18n("Dates are inconsistent !"),
          i18n("The date on the left side is later than the date on the right.\n"
               "Please re-enter the dates, so that the leftmost date will be\n"
               "earlier than the right one."));
        TabWidget2->setCurrentPage(1); // set page to advanced
        modifiedBetweenData1->setFocus();
        return false;
      }
      // all seems to be ok, create time_t
      qdate2time_t(&(query->newerThen), d1, true);
      qdate2time_t(&(query->olderThen), d2, false);
    }
  } else if (notModifiedAfterEnabled->isChecked()) {
    if ( !notModifiedAfterData->text().simplifyWhiteSpace().isEmpty() ) {
      QDate d = KGlobal::locale()->readDate(notModifiedAfterData->text());
      if (!d.isValid()) { invalidDateMessage(notModifiedAfterData); return false; }
      qdate2time_t(&(query->olderThen), d, false);
    }
  } else if (modifiedInTheLastEnabled->isChecked()) {
    if ( !(modifiedInTheLastData->text().simplifyWhiteSpace().isEmpty() &&
          notModifiedInTheLastData->text().simplifyWhiteSpace().isEmpty()) ) {
      QDate d1 = QDate::currentDate(); QDate d2 = QDate::currentDate();
      if (!modifiedInTheLastData->text().simplifyWhiteSpace().isEmpty()) {
        int tmp1 = modifiedInTheLastData->text().simplifyWhiteSpace().toInt();
        switch (modifiedInTheLastType->currentItem()) {
          case 1 : tmp1 *= 7;
                   break;
          case 2 : tmp1 *= 30;
                   break;
          case 3 : tmp1 *= 365;
                   break;
        }
        d1 = d1.addDays((-1) * tmp1);
        qdate2time_t(&(query->newerThen), d1, true);
      }
      if (!notModifiedInTheLastData->text().simplifyWhiteSpace().isEmpty()) {
        int tmp2 = notModifiedInTheLastData->text().simplifyWhiteSpace().toInt();
        switch (notModifiedInTheLastType->currentItem()) {
          case 1 : tmp2 *= 7;
                   break;
          case 2 : tmp2 *= 30;
                   break;
          case 3 : tmp2 *= 365;
                   break;
        }
        d2 = d2.addDays((-1) * tmp2);
        qdate2time_t(&(query->olderThen), d2, true);
      }
      if ( !modifiedInTheLastData->text().simplifyWhiteSpace().isEmpty() &&
           !notModifiedInTheLastData->text().simplifyWhiteSpace().isEmpty() ) {
        if (d1 > d2) {
          KMessageBox::detailedError(0, i18n("Dates are inconsistent !"),
            i18n("The date on the top is later than the date on the bottom.\n"
                 "Please re-enter the dates, so that the top date will be\n"
                 "earlier than the bottom one."));
          TabWidget2->setCurrentPage(1); // set page to advanced
          modifiedInTheLastData->setFocus();
          return false;
        }
      }
    }
  }

  // permissions and ownership /////////////////////////////////////
  if (permissionsEnabled->isChecked()) {
    QString perm = ownerR->currentText() + ownerW->currentText() + ownerX->currentText() +
                   groupR->currentText() + groupW->currentText() + groupX->currentText() +
                   allR->currentText()   + allW->currentText()   + allX->currentText();
    query->perm = perm;
  }
  if (belongsToUserEnabled->isChecked())
    query->owner = belongsToUserData->currentText();
  if (belongsToGroupEnabled->isChecked())
    query->group = belongsToGroupData->currentText();

  return true;
}

void KrSearchDialog::startSearch() {
  // first, informative messages
  if (searchInArchives->isChecked()) {
    KMessageBox::information(0, i18n("Since you chose to also search in archives, "
                                     "note the following limitations:\n"
                                     "1. Krusader will search in all of the supported"
                                       " archives, NOT INCLUDING arj, and ace.\n"
                                     "2. You cannot search for text (grep) while doing"
                                       " a search that includes archives."), 0, "searchInArchives");
  }

  // prepare the query /////////////////////////////////////////////
  if (!gui2query()) return;
  // else implied
  searchFor->addToHistory(searchFor->currentText());
  containsText->addToHistory(containsText->currentText());

  // prepare the gui ///////////////////////////////////////////////
  mainSearchBtn->setEnabled(false);
  mainCloseBtn->setEnabled(false);
  mainStopBtn->setEnabled(true);
  resultsList->clear(); searchingLabel->setText("");
  foundLabel->setText(i18n("Found 0 matches."));
  TabWidget2->setCurrentPage(2); // show the results page
  qApp->processEvents();

  // start the search.
  if (searcher != 0) {
    delete searcher;
    searcher = 0;
  }
  searcher  = new KRSearchMod(query);
  connect(searcher, SIGNAL(searching(const QString&)),
          searchingLabel, SLOT(setText(const QString&)));
  connect(searcher, SIGNAL(found(QString,QString,KIO::filesize_t,time_t,QString)),
                this, SLOT(found(QString,QString,KIO::filesize_t,time_t,QString)));
  connect(searcher, SIGNAL(finished()), this, SLOT(stopSearch()));

  searcher->start();
}

void KrSearchDialog::stopSearch() {
  if (searcher!=0) {
    searcher->stop();
    disconnect(searcher,0,0,0);
    delete query;
    query = 0;
  }

  // gui stuff
  mainSearchBtn->setEnabled(true);
  mainCloseBtn->setEnabled(true);
  mainStopBtn->setEnabled(false);
  searchingLabel->setText(i18n("Finished searching."));
}

void KrSearchDialog::modifiedBetweenSetDate1() {
  changeDate(modifiedBetweenData1);
}

void KrSearchDialog::modifiedBetweenSetDate2() {
  changeDate(modifiedBetweenData2);
}

void KrSearchDialog::notModifiedAfterSetDate() {
  changeDate(notModifiedAfterData);
}

void KrSearchDialog::fillList(QComboBox *list, QString filename) {
  QFile data(filename);
	if (!data.open(IO_ReadOnly)) {
    kdWarning() << "Search: Unable to read " << filename << " !!!" << endl;
    return;
  }
  // and read it into the temporary array
	QTextStream t(&data);
  while (!data.atEnd()) {
    QString s = t.readLine();
    QString name = s.left(s.find(':'));
    list->insertItem(name);
  }
}

void KrSearchDialog::resultClicked(QListViewItem* i) {
 	krApp->mainView->activePanel->func->openUrl((i->text(1)).replace(QRegExp("#"),"\\")+"/"+i->text(0));
  showMinimized();
}

void KrSearchDialog::saveSearch() {
}

void KrSearchDialog::loadSearch() {
}

void KrSearchDialog::loadSearch(QListViewItem *) {
}

#include "krsearchdialog.moc"
