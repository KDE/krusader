/***************************************************************************
                             krresulttable.cpp
                             -------------------
    copyright            : (C) 2005 by Dirk Eschler & Krusader Krew
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

#include "krresulttable.h"
#include <iostream>
using namespace std;

#define PS(x) _supported.contains(x)>0

KrResultTable::KrResultTable(QWidget* parent)
  : QWidget(parent),
    _numRows(1)
{
}

KrResultTable::~KrResultTable()
{
}


QGridLayout* KrResultTable::initTable()
{
  _grid = new QGridLayout(this, _numRows, _numColumns);
  _grid->setColStretch(_numColumns-1, 1); // stretch last column

  // +++ Build and add table header +++
  int column = 0;
  for( QStringList::Iterator it=_tableHeaders.begin(); it!=_tableHeaders.end(); ++it )
  {
    _label = new QLabel(*it, this);
    _label->setMargin(5);
    _grid->addWidget(_label, 0, column);

    // Set font
    QFont defFont = KGlobalSettings::generalFont();
    defFont.setPointSize(defFont.pointSize()-1);
    defFont.setBold(true);
    _label->setFont(defFont);

    ++column;
  }

  return _grid;
}


void KrResultTable::adjustRow(QGridLayout* grid)
{
  QLayoutIterator it = grid->iterator();
  QLayoutItem *child;
  int col = 0;

  while( (child = it.current()) != 0 )
  {
    // Add some space between columns
    child->widget()->setMinimumWidth( child->widget()->sizeHint().width() + 15 );

    // Paint uneven rows in alternate color
    if( ((col/_numColumns)%2) )
      child->widget()->setPaletteBackgroundColor( KGlobalSettings::baseColor() );

    ++it;
    ++col;
  }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

KrArchiverResultTable::KrArchiverResultTable(QWidget* parent)
 : KrResultTable(parent)
{
  _supported = KRarcHandler::supportedPackers(); // get list of available packers

  Archiver* tar   = new Archiver("tar",   "http://www.gnu.org",      PS("tar"),   true,  true);
  Archiver* gzip  = new Archiver("gzip",  "http://www.gnu.org",      PS("gzip"),  true,  true);
  Archiver* bzip2 = new Archiver("bzip2", "http://www.gnu.org",      PS("bzip2"), true,  true);
  Archiver* lha   = new Archiver("lha",   "http://www.gnu.org",      PS("lha"),   true,  true);
  Archiver* zip   = new Archiver("zip",   "http://www.info-zip.org", PS("zip"),   true,  false);
  Archiver* unzip = new Archiver("unzip", "http://www.info-zip.org", PS("unzip"), false, true);
  Archiver* arj   = new Archiver("arj",   "http://www.arjsoftware.com",  PS("arj"),   true,  true);
  Archiver* unarj = new Archiver("unarj", "http://www.arjsoftware.com",  PS("unarj"), false, true);
  Archiver* unace = new Archiver("unace", "http://www.winace.com",   PS("unace"), false, true);
  Archiver* rar   = new Archiver("rar",   "http://www.rarsoft.com",  PS("rar"),   true,  true);
  Archiver* unrar = new Archiver("unrar", "http://www.rarsoft.com",  PS("unrar"), false, true);
  Archiver* rpm   = new Archiver("rpm",   "http://www.gnu.org",      PS("rpm"),   false, true);

  // Special case: arj can unpack, but unarj is prefered
  if(PS("arj") && PS("unarj"))
    arj->setIsUnpacker(false);
  if(PS("arj") && !PS("unarj"))
    unarj->setNote( i18n("unarj not found, but arj found, which will be used for unpacking") );
  // Special case: rar can unpack, but unrar is prefered
  if(PS("rar") && PS("unrar"))
     rar->setIsUnpacker(false);
  // Special case: rpm needs cpio for unpacking
  if(PS("rpm") && !PS("cpio"))
    rpm->setNote( i18n("rpm found, but cpio not found which is required for unpacking") );

  _tableHeaders.append( i18n("Name") );
  _tableHeaders.append( i18n("Found") );
  _tableHeaders.append( i18n("Packing") );
  _tableHeaders.append( i18n("Unpacking") );
  _tableHeaders.append( i18n("Note") );
  _numColumns = _tableHeaders.size();

  _grid = initTable();

  addRow(tar, _grid);
  addRow(gzip, _grid);
  addRow(bzip2, _grid);
  addRow(lha, _grid);
  addRow(zip, _grid);
  addRow(unzip, _grid);
  addRow(arj, _grid);
  addRow(unarj, _grid);
  addRow(unace, _grid);
  addRow(rar, _grid);
  addRow(unrar, _grid);
  addRow(rpm, _grid);

  delete tar;
  delete gzip;
  delete bzip2;
  delete lha;
  delete zip;
  delete unzip;
  delete arj;
  delete unarj;
  delete unace;
  delete rar;
  delete unrar;
  delete rpm;
}

KrArchiverResultTable::~KrArchiverResultTable()
{
}


bool KrArchiverResultTable::addRow(SearchObject* search, QGridLayout* grid)
{
  Archiver* arch = dynamic_cast<Archiver*>(search);

  // Name column
  _label = new KURLLabel(arch->getWebsite(), arch->getSearchName(), this);
  _label->setMargin(5);
  _label->setAlignment(Qt::AlignTop);
  grid->addWidget(_label, _numRows, 0);
  connect(_label, SIGNAL(leftClickedURL(const QString&)),
                      SLOT(website(const QString&)));

  // Found column
  _label = new QLabel( arch->getPath(), this );
  _label->setMargin(5);
  grid->addWidget(_label, _numRows, 1);

  // Packing column
  _label = new QLabel(this);
  _label->setMargin(5);
  _label->setAlignment( Qt::AlignTop );
  if( arch->getIsPacker() && arch->getFound() ) {
    _label->setText( i18n("enabled") );
    _label->setPaletteForegroundColor("darkgreen");
  } else if( arch->getIsPacker() && !arch->getFound() ) {
    _label->setText( i18n("disabled") );
    _label->setPaletteForegroundColor("red");
  } else
    _label->setText( "" );
  grid->addWidget(_label, _numRows, 2);

  // Unpacking column
  _label = new QLabel(this);
  _label->setMargin(5);
  _label->setAlignment( Qt::AlignTop );
  if( arch->getIsUnpacker() && arch->getFound() ) {
    _label->setText( i18n("enabled") );
    _label->setPaletteForegroundColor("darkgreen");
  } else if( arch->getIsUnpacker() && !arch->getFound() ) {
    _label->setText( i18n("disabled") );
    _label->setPaletteForegroundColor("red");
  } else
    _label->setText( "" );
  grid->addWidget(_label, _numRows, 3);

  // Note column
  _label = new QLabel(arch->getNote(), this);
  _label->setMargin(5);
  _label->setAlignment( Qt::AlignTop | Qt::WordBreak ); // wrap words
  grid->addWidget(_label, _numRows, 4);

  // Apply shared design elements
  adjustRow(_grid);

  // Ensure the last column takes more space
  _label->setMinimumWidth(300);

  ++_numRows;
  return true;
}


void KrArchiverResultTable::website(const QString& url)
{
  (void) new KRun(url);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

KrToolResultTable::KrToolResultTable(QWidget* parent)
 : KrResultTable(parent)
{
  _supported = Krusader::supportedTools(); // get list of available tools

  QValueVector<Application*> vecDiff, vecMail, vecRename, vecChecksum;
  Application* kdiff3         = new Application("kdiff3",        "http://kdiff3.sourceforge.net/", KrServices::cmdExist("kdiff3"));
  Application* kompare        = new Application("kompare",       "http://www.caffeinated.me.uk/kompare/", KrServices::cmdExist("kompare"));
  Application* xxdiff         = new Application("xxdiff",        "http://xxdiff.sourceforge.net/", KrServices::cmdExist("xxdiff"));
  Application* kmail          = new Application("kmail",         "http://kmail.kde.org/", KrServices::cmdExist("kmail"));
  Application* krename        = new Application("krename",       "http://www.krename.net/", KrServices::cmdExist("krename"));
  Application* md5sum         = new Application("md5sum",        "http://www.gnu.org/software/textutils/textutils.html", KrServices::cmdExist("md5sum"));
  Application* md5deep        = new Application("md5deep",       "http://md5deep.sourceforge.net/", KrServices::cmdExist("md5deep"));
  Application* sha1deep       = new Application("sha1deep",      "http://md5deep.sourceforge.net/", KrServices::cmdExist("sha1deep"));
  Application* sha256deep     = new Application("sha256deep",    "http://md5deep.sourceforge.net/", KrServices::cmdExist("sha256deep"));
  Application* tigerdeep      = new Application("tigerdeep",     "http://md5deep.sourceforge.net/", KrServices::cmdExist("tigerdeep"));
  Application* whirlpooldeep  = new Application("whirlpooldeep", "http://md5deep.sourceforge.net/", KrServices::cmdExist("whirlpooldeep"));
  Application* cfv            = new Application("cfv",           "http://cfv.sourceforge.net/", KrServices::cmdExist("cfv"));

  vecDiff.push_back(kdiff3);
  vecDiff.push_back(kompare);
  vecDiff.push_back(xxdiff);
  vecMail.push_back(kmail);
  vecRename.push_back(krename);
  vecChecksum.push_back(md5sum);
  vecChecksum.push_back(md5deep);
  vecChecksum.push_back(sha1deep);
  vecChecksum.push_back(sha256deep);
  vecChecksum.push_back(tigerdeep);
  vecChecksum.push_back(whirlpooldeep);
  vecChecksum.push_back(cfv);

  ApplicationGroup* diff     = new ApplicationGroup( i18n("diff utility"),     PS("DIFF"),   vecDiff);
  ApplicationGroup* mail     = new ApplicationGroup( i18n("email client"),     PS("MAIL"),   vecMail);
  ApplicationGroup* rename   = new ApplicationGroup( i18n("batch renamer"),    PS("RENAME"), vecRename);
  ApplicationGroup* checksum = new ApplicationGroup( i18n("checksum utility"), PS("MD5"),    vecChecksum);

  _tableHeaders.append( i18n("Group") );
  _tableHeaders.append( i18n("Tool") );
  _tableHeaders.append( i18n("Found") );
  _tableHeaders.append( i18n("Status") );
  _numColumns = _tableHeaders.size();

  _grid = initTable();

  addRow(diff, _grid);
  addRow(mail, _grid);
  addRow(rename, _grid);
  addRow(checksum, _grid);

  delete kmail;
  delete kompare;
  delete kdiff3;
  delete xxdiff;
  delete krename;
  delete md5sum;
  delete md5deep;
  delete sha1deep;
  delete sha256deep;
  delete tigerdeep;
  delete whirlpooldeep;
  delete cfv;

  delete diff;
  delete mail;
  delete rename;
  delete checksum;
}

KrToolResultTable::~KrToolResultTable()
{
}


bool KrToolResultTable::addRow(SearchObject* search, QGridLayout* grid)
{
  ApplicationGroup* appGroup = dynamic_cast<ApplicationGroup*>(search);
  QValueVector<Application*> _apps = appGroup->getAppVec();

  // Name column
  _label = new QLabel(appGroup->getSearchName(), this);
  _label->setMargin(5);
  _label->setAlignment( Qt::AlignTop );
  grid->addWidget(_label, _numRows, 0);

  // Tool column
  QVBox* toolBox = new QVBox(this);
  for( QValueVector<Application*>::Iterator it=_apps.begin(); it!=_apps.end(); it++ )
  {
    KURLLabel* l = new KURLLabel( (*it)->getWebsite(), (*it)->getAppName(), toolBox);
    l->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    l->setMargin(5);
    connect(l, SIGNAL(leftClickedURL(const QString&)),
               SLOT(website(const QString&)));
  }
  grid->addWidget(toolBox, _numRows, 1);

  // Found column
  QVBox* vbox = new QVBox(this);
  for( QValueVector<Application*>::Iterator it=_apps.begin(); it!=_apps.end(); it++ )
  {
    _label = new QLabel( (*it)->getPath(), vbox);
    _label->setMargin(5);
    _label->setAlignment( Qt::AlignTop );
  }
  grid->addWidget(vbox, _numRows, 2);

  // Status column
  _label = new QLabel(this);
  _label->setMargin(5);
  _label->setAlignment( Qt::AlignTop );
  if( appGroup->getFoundGroup() ) {
    _label->setText( i18n("enabled") );
    _label->setPaletteForegroundColor("darkgreen");
  } else {
    _label->setText( i18n("disabled") );
    _label->setPaletteForegroundColor("red");
  }
  grid->addWidget(_label, _numRows, 3);

  // Apply shared design elements
  adjustRow(_grid);

  // Ensure the last column takes more space
  _label->setMinimumWidth(300);

  ++_numRows;
  return true;
}

void KrToolResultTable::website(const QString& url)
{
  (void) new KRun(url);
}
