/***************************************************************************
                      profilemanager.cpp  -  description
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

#include "../krusader.h"
#include "profilemanager.h"

#include <klocale.h>
#include <qtooltip.h>
#include <kpopupmenu.h>
#include <qcursor.h>
#include <kinputdialog.h>

static const char * const profiles_data[] = {
"20 20 111 2",
"  	g None",
". 	g #B9B9B9",
"+ 	g #BEBEBE",
"@ 	g #C7C7C7",
"# 	g #C4C4C4",
"$ 	g #CECECE",
"% 	g #B7B7B7",
"& 	g #A1A1A1",
"* 	g #C1C1C1",
"= 	g #ADADAD",
"- 	g #A5A5A5",
"; 	g #AAAAAA",
"> 	g #CACACA",
", 	g #B0B0B0",
"' 	g #ACACAC",
") 	g #CCCCCC",
"! 	g #ABABAB",
"~ 	g #B3B3B3",
"{ 	g #AFAFAF",
"] 	g #CDCDCD",
"^ 	g #B6B6B6",
"/ 	g #A9A9A9",
"( 	g #B4B4B4",
"_ 	g #888888",
": 	g #7A7A7A",
"< 	g #858585",
"[ 	g #9B9B9B",
"} 	g #898989",
"| 	g #9C9C9C",
"1 	g #A8A8A8",
"2 	g #B1B1B1",
"3 	g #989898",
"4 	g #959595",
"5 	g #8B8B8B",
"6 	g #8E8E8E",
"7 	g #9D9D9D",
"8 	g #8C8C8C",
"9 	g #808080",
"0 	g #7F7F7F",
"a 	g #B5B5B5",
"b 	g #848484",
"c 	g #5C5C5C",
"d 	g #565656",
"e 	g #656565",
"f 	g #999999",
"g 	g #9E9E9E",
"h 	g #8A8A8A",
"i 	g #B8B8B8",
"j 	g #C8C8C8",
"k 	g #A4A4A4",
"l 	g #868686",
"m 	g #B2B2B2",
"n 	g #111111",
"o 	g #000000",
"p 	g #020202",
"q 	g #353535",
"r 	g #9F9F9F",
"s 	g #969696",
"t 	g #8D8D8D",
"u 	g #919191",
"v 	g #9A9A9A",
"w 	g #090909",
"x 	g #212121",
"y 	g #A6A6A6",
"z 	g #646464",
"A 	g #929292",
"B 	g #4F4F4F",
"C 	g #181818",
"D 	g #C5C5C5",
"E 	g #7C7C7C",
"F 	g #191919",
"G 	g #0F0F0F",
"H 	g #BDBDBD",
"I 	g #909090",
"J 	g #C0C0C0",
"K 	g #BABABA",
"L 	g #333333",
"M 	g #1A1A1A",
"N 	g #AEAEAE",
"O 	g #D1D1D1",
"P 	g #555555",
"Q 	g #585858",
"R 	g #A7A7A7",
"S 	g #626262",
"T 	g #242424",
"U 	g #C3C3C3",
"V 	g #A2A2A2",
"W 	g #7E7E7E",
"X 	g #151515",
"Y 	g #CBCBCB",
"Z 	g #7D7D7D",
"` 	g #040404",
" .	g #010101",
"..	g #CFCFCF",
"+.	g #A3A3A3",
"@.	g #979797",
"#.	g #030303",
"$.	g #939393",
"%.	g #8F8F8F",
"&.	g #C9C9C9",
"*.	g #7B7B7B",
"=.	g #878787",
"-.	g #414141",
";.	g #3E3E3E",
">.	g #3F3F3F",
",.	g #949494",
"'.	g #838383",
").	g #C6C6C6",
"!.	g #D4D4D4",
"~.	g #D2D2D2",
"{.	g #BCBCBC",
"        .       +       @       #       ",
"$ % & * = - ; > , ' ; ) ! ~ { ] ^ / '   ",
"  ( _ : < [ } | 1 % / 2 3 & _ [ 4 5 6 ) ",
"  7 8 9 0 5 1 a b c d e f + g h } 6 i   ",
"j k l h } m h n o o o o p q ( r l < s # ",
"  [ t u v 1 w o o o o o o o x ^ 7 : y   ",
"  ; 5 b ; z o o o o o o o o o d , 9 A j ",
"  y < l ^ B o o o o o o o o o C a t a   ",
"D y E & . F o o o o o o o o o G H I | J ",
"  A h K L o o o o o o o o o o M J I N   ",
"O ( 4 K P o o o o o o o o o o Q ^ 6 - j ",
"  R t 2 S o o o o o o o o o T H 7 5 K   ",
"U V W y W o o o o o o o o X , R l s u Y ",
"  | I N Z `  .o o o o o o < ; < h I g   ",
"..+.Z @.i ( t X o o o o #.; $.b _ %.@.&.",
"  3 *.9 %.u { Z o o o o o =.V u 6 t ~   ",
"* f _ 0 9 %.f 2 -.;.>.;.;.: ~ ,.t A V &.",
"  $.l ,.'.< ,.J % a ( U K i ' f : < +.  ",
"..{ 8 + - | 9 + V 3 u ).{ k %.H +.g 1   ",
"    + ..#   U !.)   D ~.]   {.] ..      "};

ProfileManager::ProfileManager( QString profileType, QWidget * parent, const char * name ) 
  : QPushButton( parent, name )
{
  QPixmap profiles( ( const char** ) profiles_data );
  setText( "" );
  setPixmap( profiles );
  QToolTip::add( this, i18n( "Profiles" ) );
  
  this->profileType = profileType;
  
  connect( this, SIGNAL( clicked() ), this, SLOT( profilePopup() ) );

  krConfig->setGroup("Private");
  profileList = krConfig->readListEntry( profileType );
}

void ProfileManager::profilePopup()
{
  // profile menu identifiers
  #define ADD_NEW_ENTRY_ID    1000
  #define LOAD_ENTRY_ID       2000
  #define REMOVE_ENTRY_ID     3000
  #define OVERWRITE_ENTRY_ID  4000
  
  // create the menu
  KPopupMenu popup, removePopup, overwritePopup;
  popup.insertTitle(i18n("Profiles"));
  
  for( unsigned i=0; i != profileList.count() ; i++ )
  {
    krConfig->setGroup( profileType + " - " + profileList[i] ); 
    QString name = krConfig->readEntry( "Name" );
    popup.insertItem( name, LOAD_ENTRY_ID + i );
    removePopup.insertItem( name, REMOVE_ENTRY_ID + i );
    overwritePopup.insertItem( name, OVERWRITE_ENTRY_ID + i );
  }

  popup.insertSeparator();
  
  if( profileList.count() )
  {
    popup.insertItem( i18n("Remove entry"), &removePopup );
    popup.insertItem( i18n("Overwrite entry"), &overwritePopup );
  }
  
  popup.insertItem(i18n("Add new entry"),ADD_NEW_ENTRY_ID);

  unsigned result=popup.exec(QCursor::pos());

  // check out the user's selection
  if( result == ADD_NEW_ENTRY_ID )
  {
    QString profile = KInputDialog::getText( i18n( "Krusader::ProfileManager" ), i18n( "Enter the profile name:" ) );  
    if( !profile.isEmpty() )
    {
      int profileNum = 1;
      while( profileList.contains( QString( "%1" ).arg( profileNum ) ) )
        profileNum++;

      QString profileString = QString( "%1" ).arg( profileNum );
      QString profileName = profileType + " - " + profileString;
      profileList.append( QString( "%1" ).arg( profileString ) );
  
      krConfig->setGroup("Private");
      krConfig->writeEntry( profileType, profileList );
      
      krConfig->setGroup( profileName );
      krConfig->writeEntry( "Name", profile );
      emit saveToProfile( profileName );
      krConfig->sync();
    }
  }else if( result >= LOAD_ENTRY_ID && result < LOAD_ENTRY_ID + profileList.count() )
  {
    emit loadFromProfile( profileType + " - " + profileList[ result - LOAD_ENTRY_ID ] );
  }else if( result >= REMOVE_ENTRY_ID && result < REMOVE_ENTRY_ID + profileList.count() )
  { 
    krConfig->deleteGroup( profileType + " - " + profileList[ result - REMOVE_ENTRY_ID ] );    
    profileList.remove( profileList[ result - REMOVE_ENTRY_ID ] );
  
    krConfig->setGroup("Private");
    krConfig->writeEntry( profileType, profileList );
    krConfig->sync();
  }else if( result >= OVERWRITE_ENTRY_ID && result < OVERWRITE_ENTRY_ID + profileList.count() )
  {
    emit saveToProfile( profileType + " - " + profileList[ result - OVERWRITE_ENTRY_ID ] );
  }  
}

#include "profilemanager.moc"
