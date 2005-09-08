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
//   _grid->setAlignment(Qt::AlignTop);
  _grid->setColStretch(_numColumns-1, 1); // stretch last column
//   _grid->setSpacing(5);
//   _grid->setMargin(5);
//   _grid->set

  // +++ Build and add table header +++
  int column = 0;
  for( QStringList::Iterator it=_tableHeaders.begin(); it!=_tableHeaders.end(); ++it )
  {
    _label = new QLabel(*it, this);

    _grid->addWidget(_label, 0, column);
    // FIXME iterate over qgridlayout and set to maximum width plus some extra space
    _grid->setColSpacing(column, _label->sizeHint().width()+25);
    _label->setMargin(5);

    // Set font
    QFont defFont = KGlobalSettings::generalFont();
    defFont.setPointSize(defFont.pointSize()-1);
    defFont.setBold(true);
    _label->setFont(defFont);

    ++column;
  }

  return _grid;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

KrArchiverResultTable::KrArchiverResultTable(QWidget* parent)
 : KrResultTable(parent)
{
  _supported = KRarcHandler::supportedPackers(); // get list of available packers

  Archiver* tar   = new Archiver("tar",   "http://www.gnu.org",      PS("tar"),   true,  true);
  Archiver* gzip  = new Archiver("gzip",  "http://www.gnu.org",      PS("gzip"),  true,  true);
  Archiver* bzip2 = new Archiver("bzip2", "http://www.gnu.org",      PS("bzip2"), true,  false);
  Archiver* lha   = new Archiver("lha",   "http://www.gnu.org",      PS("lha"),   true,  true);
  Archiver* zip   = new Archiver("zip",   "http://www.info-zip.org", PS("zip"),   true,  false);
  Archiver* unzip = new Archiver("unzip", "http://www.info-zip.org", PS("unzip"), false, true);
  Archiver* arj   = new Archiver("arj",   "http://www.arjsoft.com",  PS("arj"),   true,  true);
  Archiver* unarj = new Archiver("unarj", "http://www.arjsoft.com",  PS("unarj"), false, true);
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
  ++_numRows;

  // Name column
  _nameLabel = new KURLLabel(arch->getWebsite(), arch->getSearchName(), this);
  _nameLabel->setMargin(5);
  _nameLabel->setAlignment(Qt::AlignTop);
  grid->addWidget(_nameLabel, _numRows, 0);
  connect(_nameLabel, SIGNAL(leftClickedURL(const QString&)),
                      SLOT(website(const QString&)));

  // Found column
  _foundLabel = new QLabel(this);
  _foundLabel->setMargin(5);
  _foundLabel->setAlignment(Qt::AlignTop);
  if(arch->getFound())
    _foundLabel->setPixmap(krLoader->loadIcon("button_ok", KIcon::Desktop, 16));
  else
    _foundLabel->setPixmap(krLoader->loadIcon("no", KIcon::Desktop, 16));
  grid->addWidget(_foundLabel, _numRows, 1);

  // Packing column
  _packingLabel = new QLabel(this);
  _packingLabel->setMargin(5);
  _packingLabel->setAlignment(Qt::AlignTop);
  if( arch->getIsPacker() && arch->getFound() ) {
    _packingLabel->setText( i18n("enabled") );
    _packingLabel->setPaletteForegroundColor("darkgreen");
  } else if( arch->getIsPacker() && !arch->getFound() ) {
    _packingLabel->setText( i18n("disabled") );
    _packingLabel->setPaletteForegroundColor("red");
  } else
    _packingLabel->setText( "" );
  grid->addWidget(_packingLabel, _numRows, 2);

  // Unpacking column
  _unpackingLabel = new QLabel(this);
  _unpackingLabel->setMargin(5);
  _unpackingLabel->setAlignment(Qt::AlignTop);
  if( arch->getIsUnpacker() && arch->getFound() ) {
    _unpackingLabel->setText( i18n("enabled") );
    _unpackingLabel->setPaletteForegroundColor("darkgreen");
  } else if( arch->getIsUnpacker() && !arch->getFound() ) {
    _unpackingLabel->setText( i18n("disabled") );
    _unpackingLabel->setPaletteForegroundColor("red");
  } else
    _unpackingLabel->setText( "" );
  grid->addWidget(_unpackingLabel, _numRows, 3);

  // Note column
  _noteLabel = new QLabel(arch->getNote(), this);
  _noteLabel->setMargin(5);
  _noteLabel->setMinimumWidth(300);
//   _noteLabel->setMinimumWidth( _noteLabel->fontMetrics().maxWidth()*30 );
  _noteLabel->setAlignment(Qt::AlignTop | Qt::WordBreak); // wrap words
  grid->addWidget(_noteLabel, _numRows, 4);

  // Paint uneven rows in alternate color
  if(!(_numRows % 2)) {
    _nameLabel->setPaletteBackgroundColor( KGlobalSettings::baseColor() );
    _foundLabel->setPaletteBackgroundColor( KGlobalSettings::baseColor() );
    _packingLabel->setPaletteBackgroundColor( KGlobalSettings::baseColor() );
    _unpackingLabel->setPaletteBackgroundColor( KGlobalSettings::baseColor() );
    _noteLabel->setPaletteBackgroundColor( KGlobalSettings::baseColor() );
  }

  return true;
}


void KrArchiverResultTable::website(const QString& url)
{
  (void) new KRun(url);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/**
* TODO We can search for application groups, but not for single tools.
*      class Application is capable of holding the tool->found information already
*/
KrToolResultTable::KrToolResultTable(QWidget* parent)
 : KrResultTable(parent)
{
  _supported = Krusader::supportedTools(); // get list of available tools

  QValueVector<Application*> vecDiff, vecMail, vecRename;
  Application* kdiff3  = new Application("kdiff3",  "http://kdiff3.sourceforge.net/", KrServices::cmdExist("kdiff3"));
  Application* kompare = new Application("kompare", "http://www.caffeinated.me.uk/kompare/", KrServices::cmdExist("kompare"));
  Application* xxdiff  = new Application("xxdiff",  "http://xxdiff.sourceforge.net/", KrServices::cmdExist("xxdiff"));
  Application* kmail   = new Application("kmail",   "http://kmail.kde.org/", KrServices::cmdExist("kmail"));
  Application* krename = new Application("krename", "http://www.krename.net/", KrServices::cmdExist("krename"));

  vecDiff.push_back(kdiff3);
  vecDiff.push_back(kompare);
  vecDiff.push_back(xxdiff);
  vecMail.push_back(kmail);
  vecRename.push_back(krename);

  ApplicationGroup* diff   = new ApplicationGroup( i18n("diff utility"),  PS("DIFF"),   vecDiff);
  ApplicationGroup* mail   = new ApplicationGroup( i18n("email client"),  PS("MAIL"),   vecMail);
  ApplicationGroup* rename = new ApplicationGroup( i18n("batch renamer"), PS("RENAME"), vecRename);

  _tableHeaders.append( i18n("Name") );
  _tableHeaders.append( i18n("Found") );
  _tableHeaders.append( i18n("Status") );
  _numColumns = _tableHeaders.size();

  _grid = initTable();

  addRow(diff, _grid);
  addRow(mail, _grid);
  addRow(rename, _grid);

  delete kmail;
  delete kompare;
  delete kdiff3;
  delete xxdiff;
  delete krename;

  delete diff;
  delete mail;
  delete rename;
}

KrToolResultTable::~KrToolResultTable()
{
}


bool KrToolResultTable::addRow(SearchObject* search, QGridLayout* grid)
{
  ApplicationGroup* appGroup = dynamic_cast<ApplicationGroup*>(search);
  ++_numRows;

  // Name column
  _nameLabel = new QLabel(appGroup->getSearchName(), this);
  _nameLabel->setMargin(5);
  _nameLabel->setAlignment(Qt::AlignTop);
  grid->addWidget(_nameLabel, _numRows, 0);

  // Found column
  QValueVector<Application*> _apps = appGroup->getAppVec();
  QVBox* vbox = new QVBox(this);
  vbox->setMargin(5);
  vbox->setSpacing(5);
  vbox->layout()->setAlignment(Qt::AlignTop);
  for( QValueVector<Application*>::Iterator it=_apps.begin(); it!=_apps.end(); it++ )
  {
    QHBox* hbox = new QHBox(vbox); // [icon][label]
    _foundLabel = new QLabel(hbox, "iconLabel");
    if( (*it)->getFound() )
      _foundLabel->setPixmap(krLoader->loadIcon("button_ok", KIcon::Desktop, 16));
    else
      _foundLabel->setPixmap(krLoader->loadIcon("no", KIcon::Desktop, 16));
    _foundLabel->setFixedSize( _foundLabel->sizeHint().width()+10, _foundLabel->sizeHint().height() ); // add some space between icon and label

    KURLLabel* l = new KURLLabel( (*it)->getWebsite(), (*it)->getAppName(), hbox);
    l->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    connect(l, SIGNAL(leftClickedURL(const QString&)),
               SLOT(website(const QString&)));
  }
  grid->addWidget(vbox, _numRows, 1);

  // Status column
  _statusLabel = new QLabel(this);
  _statusLabel->setMargin(5);
  _statusLabel->setAlignment(Qt::AlignTop);
  if( appGroup->getFoundGroup() ) {
    _statusLabel->setText( i18n("enabled") );
    _statusLabel->setPaletteForegroundColor("darkgreen");
  } else {
    _statusLabel->setText( i18n("disabled") );
    _statusLabel->setPaletteForegroundColor("red");
  }
  grid->addWidget(_statusLabel, _numRows, 2);


  // Paint uneven rows in alternate color
  if(!(_numRows % 2)) {
    _nameLabel->setPaletteBackgroundColor( KGlobalSettings::baseColor() );
    _foundLabel->setPaletteBackgroundColor( KGlobalSettings::baseColor() );
    _statusLabel->setPaletteBackgroundColor( KGlobalSettings::baseColor() );
    vbox->setPaletteBackgroundColor( KGlobalSettings::baseColor() );
  }

  return true;
}

void KrToolResultTable::website(const QString& url)
{
  (void) new KRun(url);
}
