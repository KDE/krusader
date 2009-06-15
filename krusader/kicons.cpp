/***************************************************************************
                                kicons.cpp
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
 this file is the Krusader's icon library.
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

#include "kicons.h"
#include "krusader.h"
#include "krusaderview.h"
#include "defaults.h"
#include <QtCore/QString>
#include <QPixmap>
#include <QStyle>

QPixmap FL_LOADICON(QString name)
{
    KConfigGroup group(krConfig, "Look&Feel");
    int size = (group.readEntry("Filelist Icon Size", _FilelistIconSize)).toInt();
    return krLoader->loadIcon(name, KIconLoader::Desktop, size);
}

const char * no_xpm[] = {
    "14 14 5 1",
    "  c None",
    ". c #F70713",
    "+ c #800309",
    "@ c #7F0309",
    "# c #EC0612",
    "              ",
    "  ..       .. ",
    "  ...     ... ",
    "   ...   ...  ",
    "    ... ...   ",
    "     .....    ",
    "      ...     ",
    "     .....    ",
    "    ....#..   ",
    "   ...  ...   ",
    "  ...    ...  ",
    " ...      ... ",
    " ..        .. ",
    "              "
};

const char * yes_xpm[] = {
    "14 14 4 1",
    "  c None",
    ". c #07F907",
    "+ c #06F806",
    "@ c #06F706",
    "              ",
    "             .",
    "           ...",
    "          ....",
    "         ...  ",
    "        ...   ",
    "        ..    ",
    "..     ..     ",
    "....  ...     ",
    "  .. ...      ",
    "   ....       ",
    "   ...+       ",
    "    ..        ",
    "    @.        "
};

const char * link_xpm[] = {
    "32 32 28 1",
    "  c None",
    ". c #000000",
    "+ c #FEFEFE",
    "@ c #FFFFFF",
    "# c #D7D7D7",
    "$ c #F6F6F6",
    "% c #646464",
    "& c #373737",
    "* c #2B2B2B",
    "= c #BEBEBE",
    "- c #393939",
    "; c #363636",
    "> c #F0F0F0",
    ", c #AFAFAF",
    "' c #2A2A2A",
    ") c #606060",
    "! c #474747",
    "~ c #868686",
    "{ c #262626",
    "] c #1E1E1E",
    "^ c #DEDEDE",
    "/ c #666666",
    "( c #222222",
    "_ c #1C1C1C",
    ": c #DCDCDC",
    "< c #4F4F4F",
    "[ c #1A1A1A",
    "} c #525252",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "..........                      ",
    ".+@##$++@.                      ",
    ".@%&*#+++.                      ",
    ".@=@-;@@+.                      ",
    ".@>@,')!+.                      ",
    ".@@~!*{]@.                      ",
    ".@@^/'(_@.                      ",
    ".@@@:<][@.                      ",
    ".++++:}[+.                      ",
    ".@@@@@@#@.                      ",
    "..........                      "
};

const char * black_xpm[] = {
    "32 32 2 1",
    "  c None",
    ". c #000000",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "..........                      ",
    "..........                      ",
    "..........                      ",
    "..........                      ",
    "..........                      ",
    "..........                      ",
    "..........                      ",
    "..........                      ",
    "..........                      ",
    "..........                      ",
    "..........                      "
};

const char * yellow_xpm[] = {
    "32 32 3 1",
    "  c None",
    ". c #000000",
    "+ c #FFEE00",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "..........                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    "..........                      "
};

const char * red_xpm[] = {
    "32 32 3 1",
    "  c None",
    ". c #000000",
    "+ c #FF0000",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "..........                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    "..........                      "
};

const char * white_xpm[] = {
    "32 32 3 1",
    "  c None",
    ". c #000000",
    "+ c #FFFFFF",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "..........                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    "..........                      "
};

const char * green_xpm[] = {
    "32 32 3 1",
    "  c None",
    ". c #000000",
    "+ c #15FF00",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "..........                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    "..........                      "
};

const char * blue_xpm[] = {
    "32 32 3 1",
    "  c None",
    ". c #000000",
    "+ c #1100FF",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "..........                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    ".++++++++.                      ",
    "..........                      "
};

