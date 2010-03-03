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
#include "../VFS/krpermhandler.h"
#include "../KViewer/krviewer.h"
#include "../Dialogs/krspwidgets.h"
#include "../VFS/krquery.h"
#include "../krservices.h"
#include "../krslots.h"
#include "../kicons.h"
#include "synchronizedialog.h"
#include "feedtolistboxdialog.h"
#include <QtGui/QLayout>
#include <QtGui/QGroupBox>
#include <QResizeEvent>
#include <QLabel>
#include <QPixmap>
#include <QMouseEvent>
#include <QGridLayout>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QFrame>
#include <kurlrequester.h>
#include <klocale.h>
#include <kmenu.h>
#include <QtGui/QCursor>
#include <time.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <QtCore/QEventLoop>
#include <QtCore/QRegExp>
#include <qheaderview.h>
#include <qspinbox.h>
#include <kinputdialog.h>
#include <QDrag>
#include <QMimeData>
#include <QtGui/QClipboard>
#include <QtCore/QHash>

static const char * const right_arrow_button_data[] = {
    "18 18 97 2",
    "   c None",
    ".  c #000000",
    "+  c #030D03",
    "@  c #184F16",
    "#  c #5DB45A",
    "$  c #2E6C2A",
    "%  c #90D28D",
    "&  c #9CD59A",
    "*  c #32732E",
    "=  c #92CF8F",
    "-  c #BDE9BB",
    ";  c #C4E5C3",
    ">  c #447F41",
    ",  c #108F08",
    "'  c #0F8E08",
    ")  c #108E08",
    "!  c #14970B",
    "~  c #8DD289",
    "{  c #87DF7F",
    "]  c #D6F1D3",
    "^  c #C3E1C1",
    "/  c #488844",
    "(  c #73D56C",
    "_  c #D1F4D0",
    ":  c #F3FCF3",
    "<  c #F7FEF7",
    "[  c #EFFCEF",
    "}  c #DCF6DB",
    "|  c #B4EAB0",
    "1  c #70D965",
    "2  c #2AC71B",
    "3  c #68D85D",
    "4  c #CFF1CB",
    "5  c #DCEFDB",
    "6  c #589955",
    "7  c #74D46D",
    "8  c #9DDF98",
    "9  c #ABE8A5",
    "0  c #B8EEB4",
    "a  c #9EE797",
    "b  c #74DD6A",
    "c  c #62D758",
    "d  c #23C512",
    "e  c #15BC07",
    "f  c #27C519",
    "g  c #73DC69",
    "h  c #BAEDB6",
    "i  c #B8E7B6",
    "j  c #499145",
    "k  c #77D671",
    "l  c #7BD874",
    "m  c #3AC72C",
    "n  c #42C437",
    "o  c #34C526",
    "p  c #1FC40F",
    "q  c #24C516",
    "r  c #1AB70E",
    "s  c #1ABC0D",
    "t  c #20C411",
    "u  c #5AD94B",
    "v  c #5DE24A",
    "w  c #36C229",
    "x  c #177B11",
    "y  c #7DDA75",
    "z  c #84E07B",
    "A  c #2BC519",
    "B  c #2FBF1F",
    "C  c #33C623",
    "D  c #28C716",
    "E  c #22CC11",
    "F  c #20C511",
    "G  c #23CC13",
    "H  c #31D81C",
    "I  c #4FE03B",
    "J  c #34C725",
    "K  c #137F0D",
    "L  c #157C0E",
    "M  c #126B0F",
    "N  c #126A0E",
    "O  c #116C0C",
    "P  c #13760E",
    "Q  c #179A0E",
    "R  c #37DC22",
    "S  c #4DDF38",
    "T  c #2AB71D",
    "U  c #12720D",
    "V  c #010C00",
    "W  c #28C715",
    "X  c #47DF32",
    "Y  c #2DBD1F",
    "Z  c #116B0D",
    "`  c #2CB122",
    " . c #23AD18",
    ".. c #10620C",
    "+. c #289023",
    "@. c #125B0E",
    "#. c #094706",
    "                                    ",
    "                . +                 ",
    "                . @ .               ",
    "                . # $ .             ",
    "                . % & * .           ",
    "  . . . . . . . . = - ; > .         ",
    "  . , ' ' ' ) , ! ~ { ] ^ / .       ",
    "  . ( _ : < [ } | 1 2 3 4 5 6 .     ",
    "  . 7 8 9 0 a b c d e f g h i j .   ",
    "  . k l m n o p q r s t u v w x .   ",
    "  . y z A B C D E F G H I J K .     ",
    "  . L M N M O P Q p R S T U .       ",
    "  V . . . . . . . W X Y Z .         ",
    "                . `  ....           ",
    "                . +.@..             ",
    "                . #..               ",
    "                . .                 ",
    "                                    "
};

static const char * const equals_button_data[] = {
    "18 18 5 1",
    "  c None",
    ". c #414100",
    "+ c #E0E0E0",
    "@ c #A8A8A8",
    "# c #808080",
    "                  ",
    "                  ",
    "                  ",
    "  ..............  ",
    "  .++++++++++++.  ",
    "  .@@@@@@@@@@@@.  ",
    "  .############.  ",
    "  ..............  ",
    "                  ",
    "  ..............  ",
    "  .++++++++++++.  ",
    "  .@@@@@@@@@@@@.  ",
    "  .############.  ",
    "  ..............  ",
    "                  ",
    "                  ",
    "                  ",
    "                  "
};

static const char * const differents_button_data[] = {
    "18 18 5 1",
    "  c None",
    ". c #FF0000",
    "+ c #FFC0C0",
    "@ c #FF8080",
    "# c #FF4040",
    "                  ",
    "           ...    ",
    "           ...    ",
    "  ..............  ",
    "  .+++++++...++.  ",
    "  .@@@@@@...@@@.  ",
    "  .######...###.  ",
    "  ..............  ",
    "        ...       ",
    "  ..............  ",
    "  .++++...+++++.  ",
    "  .@@@...@@@@@@.  ",
    "  .###...######.  ",
    "  ..............  ",
    "     ...          ",
    "    ...           ",
    "    ...           ",
    "                  "
};

static const char * const left_arrow_button_data[] = {
    "18 18 137 2",
    "   c None",
    ".  c #03090E",
    "+  c #0D3A57",
    "@  c #041F2B",
    "#  c #073347",
    "$  c #0D3C5A",
    "%  c #051C26",
    "&  c #0F455C",
    "*  c #237191",
    "=  c #104363",
    "-  c #04121A",
    ";  c #0C4A62",
    ">  c #198AAD",
    ",  c #2291B2",
    "'  c #104564",
    ")  c #062332",
    "!  c #0D506B",
    "~  c #209FBD",
    "{  c #33CBDF",
    "]  c #16ACC8",
    "^  c #0C4968",
    "/  c #061F2D",
    "(  c #031721",
    "_  c #041621",
    ":  c #051721",
    "<  c #021621",
    "[  c #031B27",
    "}  c #01090D",
    "|  c #04151E",
    "1  c #0D5672",
    "2  c #1E99B8",
    "3  c #39CEDF",
    "4  c #22C5DC",
    "5  c #10A1C4",
    "6  c #0E799B",
    "7  c #0E5976",
    "8  c #0D516D",
    "9  c #0F4E6B",
    "0  c #0F4D6A",
    "a  c #0F607D",
    "b  c #031D25",
    "c  c #052837",
    "d  c #0D617F",
    "e  c #25ABC7",
    "f  c #3BD0E1",
    "g  c #1DC0D9",
    "h  c #14A8CC",
    "i  c #11A3C5",
    "j  c #11ABCC",
    "k  c #17AAC8",
    "l  c #23ACC6",
    "m  c #1FA8C0",
    "n  c #1AAAC5",
    "o  c #7CCDE1",
    "p  c #76C4DB",
    "q  c #032832",
    "r  c #061D28",
    "s  c #125F7C",
    "t  c #29A6C3",
    "u  c #4BD4E3",
    "v  c #4BC5DA",
    "w  c #129FC4",
    "x  c #0D95BC",
    "y  c #0F90B7",
    "z  c #16A2C5",
    "A  c #0FA3C4",
    "B  c #26A8C5",
    "C  c #37A8C4",
    "D  c #2DA9C7",
    "E  c #75C1D9",
    "F  c #71BED6",
    "G  c #0A212C",
    "H  c #467B92",
    "I  c #B6D9E8",
    "J  c #B6E2ED",
    "K  c #69C7DC",
    "L  c #19A2C5",
    "M  c #0796BC",
    "N  c #13A5C5",
    "O  c #59BBD7",
    "P  c #6BC5DD",
    "Q  c #98D8E8",
    "R  c #B4E2EE",
    "S  c #A6DCE9",
    "T  c #98CFDF",
    "U  c #6DBCD4",
    "V  c #143341",
    "W  c #56859A",
    "X  c #DCEAF0",
    "Y  c #CCEAF2",
    "Z  c #5EC2D9",
    "`  c #1BA7C8",
    " . c #66C4DA",
    ".. c #B1DBEB",
    "+. c #DBEEF6",
    "@. c #EFF6FC",
    "#. c #F7FAFE",
    "$. c #F3F8FC",
    "%. c #D0EAF4",
    "&. c #6CBCD5",
    "*. c #091A21",
    "=. c #457589",
    "-. c #C2D8E2",
    ";. c #D4ECF2",
    ">. c #80CCDF",
    ",. c #8ABFD3",
    "'. c #0C7497",
    "). c #086E90",
    "!. c #086C8E",
    "~. c #086B8E",
    "{. c #086D90",
    "]. c #021E27",
    "^. c #0D2B38",
    "/. c #426D80",
    "(. c #C3DAE5",
    "_. c #BCDCEA",
    ":. c #90BDD0",
    "<. c #144361",
    "[. c #002745",
    "}. c #00213F",
    "|. c #001F3E",
    "1. c #00203F",
    "2. c #002643",
    "3. c #000B13",
    "4. c #03161D",
    "5. c #2F5F73",
    "6. c #9AC3D5",
    "7. c #8EBED3",
    "8. c #1A4A68",
    "9. c #0C222B",
    "0. c #2B5A6D",
    "a. c #5A98B4",
    "b. c #164867",
    "c. c #0F2731",
    "d. c #163E50",
    "e. c #0E3E5C",
    "f. c #0C3652",
    "                                    ",
    "                . f.                ",
    "              c.d.e.                ",
    "            9.0.a.b.                ",
    "          4.5.6.7.8.                ",
    "        ^./.(._.:.<.[.}.|.|.1.2.3.  ",
    "      *.=.-.;.>.,.'.).!.~.~.~.{.].  ",
    "    V W X Y Z `  ...+.@.#.$.%.&.q   ",
    "  G H I J K L M N O P Q R S T U q   ",
    "  r s t u v w x y z A B C D E F q   ",
    "    c d e f g h i j k l m n o p q   ",
    "      | 1 2 3 4 5 6 7 8 9 0 9 a b   ",
    "        ) ! ~ { ] ^ / ( _ : < [ }   ",
    "          - ; > , '                 ",
    "            % & * =                 ",
    "              @ # $                 ",
    "                . +                 ",
    "                                    "
};

static const char * const trash_button_data[] = {
    "18 18 140 2",
    "   c None",
    ".  c #BFBFBF",
    "+  c #BABAB9",
    "@  c #AEAEAE",
    "#  c #A2A2A3",
    "$  c #959595",
    "%  c #8B8B8C",
    "&  c #868687",
    "*  c #D3D5D5",
    "=  c #E1E1E1",
    "-  c #CCCCCD",
    ";  c #BDBEBD",
    ">  c #B1B2B1",
    ",  c #A3A2A2",
    "'  c #959597",
    ")  c #8E8E8F",
    "!  c #818282",
    "~  c #727171",
    "{  c #838384",
    "]  c #D1D1D1",
    "^  c #F3F3F3",
    "/  c #C6C7C6",
    "(  c #B8B9B9",
    "_  c #ABABAB",
    ":  c #9F9FA0",
    "<  c #949394",
    "[  c #8E8E8E",
    "}  c #7E8080",
    "|  c #717071",
    "1  c #5C5C5B",
    "2  c #555556",
    "3  c #A7A7A7",
    "4  c #FAFAFA",
    "5  c #CACACA",
    "6  c #BABBBB",
    "7  c #B5B6B6",
    "8  c #A9A9AA",
    "9  c #9E9E9D",
    "0  c #929293",
    "a  c #8E8C8D",
    "b  c #7F7F7F",
    "c  c #6F6F70",
    "d  c #525151",
    "e  c #414141",
    "f  c #A1A2A2",
    "g  c #C3C3C2",
    "h  c #D5D4D4",
    "i  c #ECECEC",
    "j  c #E7E7E7",
    "k  c #D6D6D6",
    "l  c #C5C5C6",
    "m  c #B0B0B0",
    "n  c #AAAAAA",
    "o  c #989898",
    "p  c #6D6D6E",
    "q  c #494949",
    "r  c #9E9E9E",
    "s  c #C0C1C1",
    "t  c #BABABA",
    "u  c #B2B2B2",
    "v  c #AEAEAD",
    "w  c #A4A4A4",
    "x  c #9B9B9B",
    "y  c #8E8F8F",
    "z  c #888888",
    "A  c #767676",
    "B  c #616161",
    "C  c #B3B3B3",
    "D  c #B9B9BA",
    "E  c #A4A5A4",
    "F  c #979797",
    "G  c #888788",
    "H  c #6D6D6D",
    "I  c #4D4D4D",
    "J  c #4B4A4B",
    "K  c #F6F6F6",
    "L  c #B1B1B1",
    "M  c #A7A8A7",
    "N  c #939394",
    "O  c #8D8D8E",
    "P  c #727272",
    "Q  c #505050",
    "R  c #484848",
    "S  c #EEEEEE",
    "T  c #EBEBEB",
    "U  c #9D9D9C",
    "V  c #919292",
    "W  c #8C8C8C",
    "X  c #808080",
    "Y  c #6C6B6C",
    "Z  c #E5E5E5",
    "`  c #AFAFAF",
    " . c #A6A6A6",
    ".. c #8F908F",
    "+. c #888989",
    "@. c #7B7B7B",
    "#. c #676667",
    "$. c #4F4F4F",
    "%. c #AFB0AF",
    "&. c #D8D8D8",
    "*. c #D4D4D4",
    "=. c #A5A5A4",
    "-. c #9A9A9A",
    ";. c #8D8D8D",
    ">. c #777777",
    ",. c #5F5F5F",
    "'. c #4B4B4B",
    "). c #B0AFAF",
    "!. c #D3D2D2",
    "~. c #CDCDCD",
    "{. c #A4A5A5",
    "]. c #8D8D8B",
    "^. c #868685",
    "/. c #747474",
    "(. c #5D5D5D",
    "_. c #AEAFAE",
    ":. c #C4C4C4",
    "<. c #BEBEBE",
    "[. c #A3A4A3",
    "}. c #8B8A8A",
    "|. c #838282",
    "1. c #707070",
    "2. c #565656",
    "3. c #444444",
    "4. c #000000",
    "5. c #B2B1B1",
    "6. c #ADADAD",
    "7. c #A3A3A3",
    "8. c #979697",
    "9. c #6C6C6C",
    "0. c #403F3F",
    "a. c #B1B2B2",
    "b. c #9F9F9F",
    "c. c #A6A6A7",
    "d. c #9E9F9F",
    "e. c #949494",
    "f. c #828282",
    "g. c #787979",
    "h. c #3D3D3C",
    "i. c #2F2F2F",
    "                                    ",
    "        . + @ # $ % & %             ",
    "    * = - ; > , ' ) ! ~ {           ",
    "  ] ^ / ; ( _ : < [ } | 1 2         ",
    "  3 4 5 6 7 8 9 0 a b c d e         ",
    "  f g h i j k l m n o p q e         ",
    "  r s t u v w x y z A B e 2         ",
    "    [ C D C E F G b H I J           ",
    "    C K ^ L M r N O P Q R           ",
    "    L S T m 3 U V W X Y R           ",
    "    m Z = `  .x ..+.@.#.$.          ",
    "    %.&.*.@ =.-.;.& >.,.'.          ",
    "    ).!.~.@ {.-.].^./.(.R           ",
    "    _.:.<.@ [.o }.|.1.2.3.4.4.4.    ",
    "    @ D 5.6.7.8.z b 9.Q 0.4.4.4.4.  ",
    "    a.b.c.3 d.e.f.g.(.h.i.4.4.4.4.  ",
    "            4.4.4.4.4.4.4.4.        ",
    "                                    "
};

static const char * const file_data[] = {
    "16 16 42 1",
    "  c None",
    ". c #D0D0DF",
    "+ c #9C9CB6",
    "@ c #FFFFFF",
    "# c #F9F9FE",
    "$ c #F5F5FC",
    "% c #E9E9F2",
    "& c #EBEBF4",
    "* c #FCFCFF",
    "= c #F8F8FE",
    "- c #ECECF4",
    "; c #D3D3E1",
    "> c #EFEFF6",
    ", c #FDFDFF",
    "' c #F1F1F8",
    ") c #E6E6F0",
    "! c #D7D7E5",
    "~ c #C9C9DA",
    "{ c #FEFEFF",
    "] c #F2F2F9",
    "^ c #EEEEF5",
    "/ c #DADAE7",
    "( c #CECEDD",
    "_ c #CCCCDB",
    ": c #F3F3F9",
    "< c #D5D5E4",
    "[ c #D2D2E0",
    "} c #E7E7F0",
    "| c #E0E0EC",
    "1 c #DCDCE9",
    "2 c #DBDBE8",
    "3 c #D8D8E6",
    "4 c #F6F6FD",
    "5 c #E5E5EF",
    "6 c #DEDEEB",
    "7 c #F0F0F7",
    "8 c #EAEAF3",
    "9 c #E8E8F1",
    "0 c #E1E1ED",
    "a c #F4F4FA",
    "b c #E4E4EE",
    "c c #FAFAFF",
    "  ........+     ",
    "  .@@@#$%&.+    ",
    "  .@@@*=-&.;+   ",
    "  .@@@*#>&++++  ",
    "  .@@@,#'&)!~+  ",
    "  .@@@{*]^/(_+  ",
    "  .@@@@*:&<[.+  ",
    "  .@@{#$}|123+  ",
    "  .@{,4^)5|61+  ",
    "  .@@#7^8950|+  ",
    "  .@,a'>&8)b|+  ",
    "  .@c:'>&8}50+  ",
    "  .@$:'7^8}50+  ",
    "  .@:]7>-8)50+  ",
    "  .*::'>&8}50+  ",
    "  ++++++++++++  "
};

static const char * const folder_data[] = {
    "16 16 132 2",
    "   c None",
    ".  c #2C87EF",
    "+  c #64A6F7",
    "@  c #357CE3",
    "#  c #D9E1F8",
    "$  c #B1BADE",
    "%  c #8BA2D9",
    "&  c #4392E3",
    "*  c #B8D9F8",
    "=  c #4F9BED",
    "-  c #126EE0",
    ";  c #0A43A8",
    ">  c #C6CBDA",
    ",  c #FFFFFF",
    "'  c #F1F0F6",
    ")  c #80ADE2",
    "!  c #4F95E4",
    "~  c #1876DF",
    "{  c #0C6ADF",
    "]  c #0C4CB9",
    "^  c #CCCFDA",
    "/  c #F8F4F7",
    "(  c #89B7E7",
    "_  c #78ACE4",
    ":  c #B4D8F8",
    "<  c #97CAFF",
    "[  c #6FAEFC",
    "}  c #3175D8",
    "|  c #0E51B4",
    "1  c #002374",
    "2  c #D0D0D7",
    "3  c #8EC1F0",
    "4  c #85BBED",
    "5  c #DAF0F9",
    "6  c #BDE2FB",
    "7  c #9CCCF8",
    "8  c #84BBF8",
    "9  c #6FAAF4",
    "0  c #4780D4",
    "a  c #0851AA",
    "b  c #0035A9",
    "c  c #CFD0D4",
    "d  c #51A2E9",
    "e  c #FFFFFE",
    "f  c #EEFCFD",
    "g  c #CEECFB",
    "h  c #B1D9F9",
    "i  c #9AC9F9",
    "j  c #7EB3F2",
    "k  c #568CDA",
    "l  c #1156BA",
    "m  c #004595",
    "n  c #003293",
    "o  c #EFEFEE",
    "p  c #84BCEE",
    "q  c #E2F8FC",
    "r  c #C9E8FB",
    "s  c #B0D8FA",
    "t  c #90C0F3",
    "u  c #6B9FE5",
    "v  c #3375CC",
    "w  c #2A71C7",
    "x  c #003B96",
    "y  c #0651AE",
    "z  c #0E3DAC",
    "A  c #E5E3E0",
    "B  c #DFD8D5",
    "C  c #FFF7F2",
    "D  c #DEEFFE",
    "E  c #BEDCF6",
    "F  c #E5FCFD",
    "G  c #C4E6FB",
    "H  c #A8D4F8",
    "I  c #85B6EC",
    "J  c #437DCE",
    "K  c #2170C9",
    "L  c #397CC8",
    "M  c #A3B6D4",
    "N  c #E3D3D2",
    "O  c #295BC3",
    "P  c #4AACFF",
    "Q  c #74A7D5",
    "R  c #CBC7CE",
    "S  c #7EB0E7",
    "T  c #F5FFFF",
    "U  c #C1E7FE",
    "V  c #9AC8F3",
    "W  c #4B84D3",
    "X  c #5490D9",
    "Y  c #B3C7E3",
    "Z  c #E9DFDF",
    "`  c #D3CED8",
    " . c #D7CFD3",
    ".. c #7488C1",
    "+. c #002A95",
    "@. c #6FCDFF",
    "#. c #CBEAFF",
    "$. c #D9F9FF",
    "%. c #70AAE1",
    "&. c #C7E0EE",
    "*. c #9DCBF1",
    "=. c #84AEE4",
    "-. c #D0E1F4",
    ";. c #FFF9F4",
    ">. c #EEE9EB",
    ",. c #E6E2E5",
    "'. c #D9D1D6",
    "). c #637DC0",
    "!. c #2D63D3",
    "~. c #001251",
    "{. c #439FF0",
    "]. c #B4DBF9",
    "^. c #D6F8FF",
    "/. c #75ACE3",
    "(. c #F0FAFF",
    "_. c #FCF9F8",
    ":. c #91A5D4",
    "<. c #2A5FCE",
    "[. c #002B91",
    "}. c #3E9DEF",
    "|. c #A4CEF4",
    "1. c #77AFE8",
    "2. c #E1EBFC",
    "3. c #3F73D7",
    "4. c #043BAE",
    "5. c #3188DE",
    "6. c #75A9E3",
    "7. c #A8C6EC",
    "8. c #6195E7",
    "9. c #1450C5",
    "0. c #7AB0FB",
    "a. c #155ED2",
    "                                ",
    "                  . + @         ",
    "          # $ % & * = - ;       ",
    "        > , , ' ) ! ~ { ]       ",
    "      ^ , / ( _ : < [ } | 1     ",
    "    2 , 3 4 5 6 7 8 9 0 a b     ",
    "  c , d e f g h i j k l m n     ",
    "o , , p , q r s t u v w x y z   ",
    "A B C D E F G H I J K L M N O   ",
    "  P Q R S T U V W X Y Z `  ...+.",
    "  @.#.$.%.&.*.=.-.;.>.,.'.).!.~.",
    "    {.].^./.(., , _.C :.<.[.    ",
    "      }.|.1., , , 2.3.4.        ",
    "        5.6.7., 8.9.            ",
    "            0.a.                ",
    "                                "
};

static const char * const swap_sides_data[] = {
    "27 20 228 2",
    "   c None",
    ".  c #03090E",
    "+  c #0C3652",
    "@  c #000000",
    "#  c #030D03",
    "$  c #0F2731",
    "%  c #163E50",
    "&  c #0E3E5C",
    "*  c #184F16",
    "=  c #0C222B",
    "-  c #2B5A6D",
    ";  c #5A98B4",
    ">  c #164867",
    ",  c #5DB45A",
    "'  c #2E6C2A",
    ")  c #03161D",
    "!  c #2F5F73",
    "~  c #9AC3D5",
    "{  c #8EBED3",
    "]  c #1A4A68",
    "^  c #90D28D",
    "/  c #9CD59A",
    "(  c #32732E",
    "_  c #0D2B38",
    ":  c #426D80",
    "<  c #C3DAE5",
    "[  c #BCDCEA",
    "}  c #90BDD0",
    "|  c #144361",
    "1  c #002745",
    "2  c #00213F",
    "3  c #001F3E",
    "4  c #00203F",
    "5  c #002643",
    "6  c #92CF8F",
    "7  c #BDE9BB",
    "8  c #C4E5C3",
    "9  c #447F41",
    "0  c #091A21",
    "a  c #457589",
    "b  c #C2D8E2",
    "c  c #D4ECF2",
    "d  c #80CCDF",
    "e  c #8ABFD3",
    "f  c #0C7497",
    "g  c #086E90",
    "h  c #086C8E",
    "i  c #086B8E",
    "j  c #086D90",
    "k  c #108F08",
    "l  c #0F8E08",
    "m  c #108E08",
    "n  c #14970B",
    "o  c #8DD289",
    "p  c #87DF7F",
    "q  c #D6F1D3",
    "r  c #C3E1C1",
    "s  c #488844",
    "t  c #143341",
    "u  c #56859A",
    "v  c #DCEAF0",
    "w  c #CCEAF2",
    "x  c #5EC2D9",
    "y  c #1BA7C8",
    "z  c #66C4DA",
    "A  c #B1DBEB",
    "B  c #DBEEF6",
    "C  c #EFF6FC",
    "D  c #F7FAFE",
    "E  c #F3F8FC",
    "F  c #D0EAF4",
    "G  c #6CBCD5",
    "H  c #73D56C",
    "I  c #D1F4D0",
    "J  c #F3FCF3",
    "K  c #F7FEF7",
    "L  c #EFFCEF",
    "M  c #DCF6DB",
    "N  c #B4EAB0",
    "O  c #70D965",
    "P  c #2AC71B",
    "Q  c #68D85D",
    "R  c #CFF1CB",
    "S  c #DCEFDB",
    "T  c #589955",
    "U  c #0A212C",
    "V  c #467B92",
    "W  c #B6D9E8",
    "X  c #B6E2ED",
    "Y  c #69C7DC",
    "Z  c #19A2C5",
    "`  c #0796BC",
    " . c #13A5C5",
    ".. c #59BBD7",
    "+. c #6BC5DD",
    "@. c #98D8E8",
    "#. c #B4E2EE",
    "$. c #A6DCE9",
    "%. c #98CFDF",
    "&. c #6DBCD4",
    "*. c #74D46D",
    "=. c #9DDF98",
    "-. c #ABE8A5",
    ";. c #B8EEB4",
    ">. c #9EE797",
    ",. c #74DD6A",
    "'. c #62D758",
    "). c #23C512",
    "!. c #15BC07",
    "~. c #27C519",
    "{. c #73DC69",
    "]. c #BAEDB6",
    "^. c #B8E7B6",
    "/. c #499145",
    "(. c #061D28",
    "_. c #125F7C",
    ":. c #29A6C3",
    "<. c #4BD4E3",
    "[. c #4BC5DA",
    "}. c #129FC4",
    "|. c #0D95BC",
    "1. c #0F90B7",
    "2. c #16A2C5",
    "3. c #0FA3C4",
    "4. c #26A8C5",
    "5. c #37A8C4",
    "6. c #2DA9C7",
    "7. c #75C1D9",
    "8. c #71BED6",
    "9. c #77D671",
    "0. c #7BD874",
    "a. c #3AC72C",
    "b. c #42C437",
    "c. c #34C526",
    "d. c #1FC40F",
    "e. c #24C516",
    "f. c #1AB70E",
    "g. c #1ABC0D",
    "h. c #20C411",
    "i. c #5AD94B",
    "j. c #5DE24A",
    "k. c #36C229",
    "l. c #177B11",
    "m. c #052837",
    "n. c #0D617F",
    "o. c #25ABC7",
    "p. c #3BD0E1",
    "q. c #1DC0D9",
    "r. c #14A8CC",
    "s. c #11A3C5",
    "t. c #11ABCC",
    "u. c #17AAC8",
    "v. c #23ACC6",
    "w. c #1FA8C0",
    "x. c #1AAAC5",
    "y. c #7CCDE1",
    "z. c #76C4DB",
    "A. c #7DDA75",
    "B. c #84E07B",
    "C. c #2BC519",
    "D. c #2FBF1F",
    "E. c #33C623",
    "F. c #28C716",
    "G. c #22CC11",
    "H. c #20C511",
    "I. c #23CC13",
    "J. c #31D81C",
    "K. c #4FE03B",
    "L. c #34C725",
    "M. c #137F0D",
    "N. c #04151E",
    "O. c #0D5672",
    "P. c #1E99B8",
    "Q. c #39CEDF",
    "R. c #22C5DC",
    "S. c #10A1C4",
    "T. c #0E799B",
    "U. c #0E5976",
    "V. c #0D516D",
    "W. c #0F4E6B",
    "X. c #0F4D6A",
    "Y. c #0F607D",
    "Z. c #157C0E",
    "`. c #126B0F",
    " + c #126A0E",
    ".+ c #116C0C",
    "++ c #13760E",
    "@+ c #179A0E",
    "#+ c #37DC22",
    "$+ c #4DDF38",
    "%+ c #2AB71D",
    "&+ c #12720D",
    "*+ c #062332",
    "=+ c #0D506B",
    "-+ c #209FBD",
    ";+ c #33CBDF",
    ">+ c #16ACC8",
    ",+ c #0C4968",
    "'+ c #061F2D",
    ")+ c #031721",
    "!+ c #041621",
    "~+ c #051721",
    "{+ c #021621",
    "]+ c #031B27",
    "^+ c #010C00",
    "/+ c #28C715",
    "(+ c #47DF32",
    "_+ c #2DBD1F",
    ":+ c #116B0D",
    "<+ c #04121A",
    "[+ c #0C4A62",
    "}+ c #198AAD",
    "|+ c #2291B2",
    "1+ c #104564",
    "2+ c #2CB122",
    "3+ c #23AD18",
    "4+ c #10620C",
    "5+ c #051C26",
    "6+ c #0F455C",
    "7+ c #237191",
    "8+ c #104363",
    "9+ c #289023",
    "0+ c #125B0E",
    "a+ c #041F2B",
    "b+ c #073347",
    "c+ c #0D3C5A",
    "d+ c #094706",
    "e+ c #0D3A57",
    "                                                      ",
    "                                                      ",
    "              . +                   @ #               ",
    "            $ % &                   @ * @             ",
    "          = - ; >                   @ , ' @           ",
    "        ) ! ~ { ]                   @ ^ / ( @         ",
    "      _ : < [ } | 1 3 4 5 @ @ @ @ @ @ 6 7 8 9 @       ",
    "    0 a b c d e f g i i j @ k l l k n o p q r s @     ",
    "  t u v w x y z A B E F G @ H I K M N O P Q R S T @   ",
    "U V W X Y Z `  ...+.$.%.&.@ *.=.;.,.'.).!.~.{.].^./.@ ",
    "(._.:.<.[.}.|.1.2.3.6.7.8.@ 9.0.b.d.e.f.g.h.i.j.k.l.@ ",
    "  m.n.o.p.q.r.s.t.u.x.y.z.@ A.B.D.F.G.H.I.J.K.L.M.@   ",
    "    N.O.P.Q.R.S.T.U.X.W.Y.@ Z.`.`.++@+d.#+$+%+&+@     ",
    "      *+=+-+;+>+,+'+~+{+]+^+@ @ @ @ @ /+(+_+:+@       ",
    "        <+[+}+|+1+                  @ 2+3+4+@         ",
    "          5+6+7+8+                  @ 9+0+@           ",
    "            a+b+c+                  @ d+@             ",
    "              . e+                  @ @               ",
    "                                                      ",
    "                                                      "
};

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
        KUrl::List urls;

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
                    KUrl leftURL = KUrl(synchronizer->leftBaseDirectory()  + leftDirName + item->leftName());
                    urls.push_back(leftURL);
                } else if (!isLeft && item->existsInRight()) {
                    QString rightDirName = item->rightDirectory().isEmpty() ? "" : item->rightDirectory() + '/';
                    KUrl rightURL = KUrl(synchronizer->rightBaseDirectory()  + rightDirName + item->rightName());
                    urls.push_back(rightURL);
                }
            }
        }

        if (urls.count() == 0)
            return;

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setImageData(FL_LOADICON(isLeft ? "arrow-left-double" : "arrow-right-double"));
        urls.populateMimeData(mimeData);
        drag->setMimeData(mimeData);
        drag->start();
    }
};

SynchronizerGUI::SynchronizerGUI(QWidget* parent,  KUrl leftURL, KUrl rightURL, QStringList selList) :
        QDialog(parent)
{
    initGUI(parent, QString(), leftURL, rightURL, selList);
}

SynchronizerGUI::SynchronizerGUI(QWidget* parent,  QString profile) :
        QDialog(parent)
{
    initGUI(parent, profile, KUrl(), KUrl(), QStringList());
}

void SynchronizerGUI::initGUI(QWidget* /* parent */, QString profileName, KUrl leftURL, KUrl rightURL, QStringList selList)
{
    selectedFiles = selList;
    isComparing = wasClosed = wasSync = false;
    firstResize = true;
    sizeX = sizeY = -1;

    bool equalSizes = false;

    hasSelectedFiles = (selectedFiles.count() != 0);

    if (leftURL.isEmpty())
        leftURL = KUrl(ROOT_DIR);
    if (rightURL.isEmpty())
        rightURL = KUrl(ROOT_DIR);

    setWindowTitle(i18n("Krusader::Synchronize Directories"));
    QGridLayout *synchGrid = new QGridLayout(this);
    synchGrid->setSpacing(6);
    synchGrid->setContentsMargins(11, 11, 11, 11);

    folderIcon =   QPixmap((const char**) folder_data);
    fileIcon   =   QPixmap((const char**) file_data);

    synchronizerTabs = new KTabWidget(this);

    /* ============================== Compare groupbox ============================== */

    QWidget *synchronizerTab = new QWidget(this);
    QGridLayout *synchronizerGrid = new QGridLayout(synchronizerTab);
    synchronizerGrid->setSpacing(6);
    synchronizerGrid->setContentsMargins(11, 11, 11, 11);

    QGroupBox *compareDirs = new QGroupBox(synchronizerTab);
    compareDirs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    compareDirs->setTitle(i18n("Directory Comparison"));

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
    QStringList list = group.readEntry("Left Directory History", QStringList());
    leftLocation->setHistoryItems(list);
    KUrlRequester *leftUrlReq = new KUrlRequester(leftLocation, compareDirs);
    leftUrlReq->setUrl(leftURL.pathOrUrl());
    leftUrlReq->setMode(KFile::Directory);
    leftUrlReq->setMinimumWidth(250);
    grid->addWidget(leftUrlReq, 1 , 0);
    leftLocation->setWhatsThis(i18n("The left base directory used during the synchronization process."));
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

    QString wtFilter = "<p><img src='toolbar|find'></p>" + i18n("<p>The filename filtering criteria is defined here.</p><p>You can make use of wildcards. Multiple patterns are separated by space (means logical OR) and patterns are excluded from the search using the pipe symbol.</p><p>If the pattern is ended with a slash (<code>*pattern*/</code>), that means that pattern relates to recursive search of directories.<ul><li><code>pattern</code> - means to search those files/directories that name is <code>pattern</code>, recursive search goes through all subdirectories independently of the value of <code>pattern</code></li><li><code>pattern/</code> - means to search all files/directories, but recursive search goes through/excludes the directories that name is <code>pattern</code></li></ul><p></p><p>It's allowed to use quotation marks for names that contain space. Filter <code>\"Program&nbsp;Files\"</code> searches out those files/directories that name is <code>Program&nbsp;Files</code>.</p><p>Examples:<ul><code><li>*.o</li><li>*.h *.c\?\?</li><li>*.cpp *.h | *.moc.cpp</li><li>* | CVS/ .svn/</li></code></ul><b>Note</b>: the search term '<code>text</code>' is equivalent to '<code>*text*</code>'.</p>");
    fileFilter->setWhatsThis(wtFilter);
    filterLabel->setWhatsThis(wtFilter);

    rightLocation = new KHistoryComboBox(compareDirs);
    rightLocation->setMaxCount(25);  // remember 25 items
    rightLocation->setDuplicatesEnabled(false);
    rightLocation->setEditable(true);
    rightLocation->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    list = group.readEntry("Right Directory History", QStringList());
    rightLocation->setHistoryItems(list);
    KUrlRequester *rightUrlReq = new KUrlRequester(rightLocation, compareDirs);
    rightUrlReq->setUrl(rightURL.pathOrUrl());
    rightUrlReq->setMode(KFile::Directory);
    rightUrlReq->setMinimumWidth(250);
    grid->addWidget(rightUrlReq, 1 , 2);
    rightLocation->setWhatsThis(i18n("The right base directory used during the synchronization process."));
    rightUrlReq->setEnabled(!hasSelectedFiles);
    rightLocation->setEnabled(!hasSelectedFiles);
    rightDirLabel->setBuddy(rightLocation);

    QWidget *optionWidget  = new QWidget(compareDirs);
    QHBoxLayout *optionBox = new QHBoxLayout(optionWidget);
    optionBox->setContentsMargins(0, 0, 0, 0);

    QWidget *optionGridWidget = new QWidget(optionWidget);
    QGridLayout *optionGrid = new QGridLayout(optionGridWidget);

    optionBox->addWidget(optionGridWidget);

    cbSubdirs         = new QCheckBox(i18n("Recurse subdirectories"), optionGridWidget);
    cbSubdirs->setChecked(group.readEntry("Recurse Subdirectories", _RecurseSubdirs));
    optionGrid->addWidget(cbSubdirs, 0, 0);
    cbSubdirs->setWhatsThis(i18n("Compare not only the base directories but their subdirectories as well."));
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
    cbAsymmetric->setWhatsThis(i18n("<p><b>Asymmetric mode</b></p><p>The left side is the destination, the right is the source directory. Files existing only in the left directory will be deleted, the other differing ones will be copied from right to left.</p><p><b>Note</b>: useful when updating a directory from a file server.</p>"));
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

    QPixmap showLeftToRight((const char**) right_arrow_button_data);
    QPixmap showEquals((const char**) equals_button_data);
    QPixmap showDifferents((const char**) differents_button_data);
    QPixmap showRightToLeft((const char**) left_arrow_button_data);
    QPixmap showDeletable((const char**) trash_button_data);

    btnLeftToRight = new QPushButton(showOptions);
    btnLeftToRight->setText("");
    btnLeftToRight->setIcon(QIcon(showLeftToRight));
    btnLeftToRight->setCheckable(true);
    btnLeftToRight->setChecked(group.readEntry("LeftToRight Button", _BtnLeftToRight));
    btnLeftToRight->setShortcut(Qt::CTRL + Qt::Key_L);
    btnLeftToRight->setWhatsThis(i18n("Show files marked to <i>Copy from left to right</i> (CTRL+L)."));
    btnLeftToRight->setFixedSize(showLeftToRight.width() + 15, showLeftToRight.height() + 15);
    showOptionsLayout->addWidget(btnLeftToRight, 0, 0);

    btnEquals = new QPushButton(showOptions);
    btnEquals->setText("");
    btnEquals->setIcon(QIcon(showEquals));
    btnEquals->setCheckable(true);
    btnEquals->setChecked(group.readEntry("Equals Button", _BtnEquals));
    btnEquals->setShortcut(Qt::CTRL + Qt::Key_E);
    btnEquals->setWhatsThis(i18n("Show files considered to be identical (CTRL+E)."));
    btnEquals->setFixedSize(showEquals.width() + 15, showEquals.height() + 15);
    showOptionsLayout->addWidget(btnEquals, 0, 1);

    btnDifferents = new QPushButton(showOptions);
    btnDifferents->setText("");
    btnDifferents->setIcon(QIcon(showDifferents));
    btnDifferents->setCheckable(true);
    btnDifferents->setChecked(group.readEntry("Differents Button", _BtnDifferents));
    btnDifferents->setShortcut(Qt::CTRL + Qt::Key_D);
    btnDifferents->setWhatsThis(i18n("Show excluded files (CTRL+D)."));
    btnDifferents->setFixedSize(showDifferents.width() + 15, showDifferents.height() + 15);
    showOptionsLayout->addWidget(btnDifferents, 0, 2);

    btnRightToLeft = new QPushButton(showOptions);
    btnRightToLeft->setText("");
    btnRightToLeft->setIcon(QIcon(showRightToLeft));
    btnRightToLeft->setCheckable(true);
    btnRightToLeft->setChecked(group.readEntry("RightToLeft Button", _BtnRightToLeft));
    btnRightToLeft->setShortcut(Qt::CTRL + Qt::Key_R);
    btnRightToLeft->setWhatsThis(i18n("Show files marked to <i>Copy from right to left</i> (CTRL+R)."));
    btnRightToLeft->setFixedSize(showRightToLeft.width() + 15, showRightToLeft.height() + 15);
    showOptionsLayout->addWidget(btnRightToLeft, 0, 3);

    btnDeletable = new QPushButton(showOptions);
    btnDeletable->setText("");
    btnDeletable->setIcon(QIcon(showDeletable));
    btnDeletable->setCheckable(true);
    btnDeletable->setChecked(group.readEntry("Deletable Button", _BtnDeletable));
    btnDeletable->setShortcut(Qt::CTRL + Qt::Key_T);
    btnDeletable->setWhatsThis(i18n("Show files marked to delete (CTRL+T)."));
    btnDeletable->setFixedSize(showDeletable.width() + 15, showDeletable.height() + 15);
    showOptionsLayout->addWidget(btnDeletable, 0, 4);

    btnDuplicates = new QPushButton(showOptions);
    btnDuplicates->setText(i18n("Duplicates"));
    btnDuplicates->setFixedWidth(QFontMetrics(btnDuplicates->font()).width(btnDuplicates->text()) + 15);
    btnDuplicates->setMinimumHeight(btnLeftToRight->height());
    btnDuplicates->setCheckable(true);
    btnDuplicates->setChecked(group.readEntry("Duplicates Button", _BtnDuplicates));
    btnDuplicates->setWhatsThis(i18n("Show files that exist on both sides."));
    showOptionsLayout->addWidget(btnDuplicates, 0, 5);

    btnSingles = new QPushButton(showOptions);
    btnSingles->setText(i18n("Singles"));
    btnSingles->setFixedWidth(QFontMetrics(btnSingles->font()).width(btnSingles->text()) + 15);
    btnSingles->setMinimumHeight(btnLeftToRight->height());
    btnSingles->setCheckable(true);
    btnSingles->setChecked(group.readEntry("Singles Button", _BtnSingles));
    btnSingles->setWhatsThis(i18n("Show files that exist on one side only."));
    showOptionsLayout->addWidget(btnSingles, 0, 6);

    grid->addWidget(optionWidget, 2, 0, 1, 3);

    synchronizerGrid->addWidget(compareDirs, 0, 0);

    /* ========================= Synchronization list view ========================== */
    syncList = new SynchronizerListView(&synchronizer, synchronizerTab);  // create the main container
    syncList->setWhatsThis(i18n("The compare results of the synchronizer (CTRL+M)."));
    syncList->setAutoFillBackground(true);
    syncList->installEventFilter(this);

    KConfigGroup gl(krConfig, "Look&Feel");
    syncList->setFont(gl.readEntry("Filelist Font", *_FilelistFont));

    syncList->setBackgroundRole(QPalette::Window);
    syncList->setAutoFillBackground(true);

    QStringList labels;
    labels << i18n("Name");
    labels << i18n("Size");
    labels << i18n("Date");
    labels << i18n("<=>");
    labels << i18n("Date");
    labels << i18n("Size");
    labels << i18n("Name");
    syncList->setHeaderLabels(labels);

    syncList->header()->setResizeMode(0, QHeaderView::Interactive);
    syncList->header()->setResizeMode(1, QHeaderView::Interactive);
    syncList->header()->setResizeMode(2, QHeaderView::Interactive);
    syncList->header()->setResizeMode(3, QHeaderView::Interactive);
    syncList->header()->setResizeMode(4, QHeaderView::Interactive);
    syncList->header()->setResizeMode(5, QHeaderView::Interactive);
    syncList->header()->setResizeMode(6, QHeaderView::Interactive);

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

    QPixmap swapSides((const char**) swap_sides_data);
    btnSwapSides = new QPushButton(this);
    btnSwapSides->setIcon(QIcon(swapSides));
    btnSwapSides->setShortcut(Qt::CTRL + Qt::Key_S);
    btnSwapSides->setWhatsThis(i18n("Swap sides (Ctrl+S)."));
    btnSwapSides->setFixedWidth(swapSides.width() + 15);
    buttons->addWidget(btnSwapSides);

    statusLabel = new QLabel(this);
    buttons->addWidget(statusLabel);

    QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttons->addItem(spacer);

    btnCompareDirs = new QPushButton(this);
    btnCompareDirs->setText(i18n("Compare"));
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
    btnStopComparing->setEnabled(false);
    buttons->addWidget(btnStopComparing);

    btnFeedToListBox = new QPushButton(this);
    btnFeedToListBox->setText(i18n("Feed to listbox"));
    btnFeedToListBox->setEnabled(false);
    btnFeedToListBox->hide();
    buttons->addWidget(btnFeedToListBox);

    btnSynchronize = new QPushButton(this);
    btnSynchronize->setText(i18n("Synchronize"));
    btnSynchronize->setEnabled(false);
    buttons->addWidget(btnSynchronize);

    QPushButton *btnCloseSync = new QPushButton(this);
    btnCloseSync->setText(i18n("Close"));
    buttons->addWidget(btnCloseSync);

    synchGrid->addLayout(buttons, 1, 0);

    /* =============================== Connect table ================================ */

    connect(syncList, SIGNAL(itemRightClicked(QTreeWidgetItem *, const QPoint &, int)),
            this, SLOT(rightMouseClicked(QTreeWidgetItem *, const QPoint &)));
    connect(syncList, SIGNAL(itemActivated(QTreeWidgetItem *, int)),
            this, SLOT(doubleClicked(QTreeWidgetItem *)));

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

    connect(&synchronizer,     SIGNAL(comparedFileData(SynchronizerFileItem *)), this,
            SLOT(addFile(SynchronizerFileItem *)));
    connect(&synchronizer,     SIGNAL(markChanged(SynchronizerFileItem *, bool)), this,
            SLOT(markChanged(SynchronizerFileItem *, bool)));
    connect(&synchronizer,     SIGNAL(statusInfo(QString)), this, SLOT(statusInfo(QString)));

    connect(btnLeftToRight,    SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(btnEquals,         SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(btnDifferents,     SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(btnRightToLeft,    SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(btnDeletable,      SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(btnDuplicates,     SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(btnSingles,        SIGNAL(toggled(bool)), this, SLOT(refresh()));

    connect(fileFilter,        SIGNAL(textChanged(const QString &)), this, SLOT(connectFilters(const QString &)));
    connect(generalFilter->searchFor, SIGNAL(textChanged(const QString &)), this, SLOT(connectFilters(const QString &)));
    connect(generalFilter->searchFor, SIGNAL(textChanged(const QString &)), this, SLOT(setCompletion()));
    connect(generalFilter->dontSearchIn, SIGNAL(checkValidity(QString &, QString &)),
            this, SLOT(checkExcludeURLValidity(QString &, QString &)));

    connect(profileManager, SIGNAL(loadFromProfile(QString)), filterTabs, SLOT(loadFromProfile(QString)));
    connect(profileManager, SIGNAL(saveToProfile(QString)), filterTabs, SLOT(saveToProfile(QString)));

    setPanelLabels();
    setCompletion();

    /* =============================== Loading the colors ================================ */

    KConfigGroup gc(krConfig, "Colors");

    DECLARE_COLOR_NAME_ARRAY;
    DECLARE_BACKGROUND_DFLTS;
    DECLARE_FOREGROUND_DFLTS;

    for (int clr = 0; clr != TT_MAX; clr ++) {
        QString foreEntry = QString("Synchronizer ") + COLOR_NAMES[ clr ] + QString(" Foreground");
        QString bckgEntry = QString("Synchronizer ") + COLOR_NAMES[ clr ] + QString(" Background");

        if (gc.readEntry(foreEntry, QString()) == "KDE default")
            foreGrounds[ clr ] = QColor();
        else if (gc.readEntry(foreEntry, QString()).isEmpty())    // KDE4 workaround, default color doesn't work
            foreGrounds[ clr ] = FORE_DFLTS[ clr ];
        else
            foreGrounds[ clr ] = gc.readEntry(foreEntry, FORE_DFLTS[ clr ]);

        if (gc.readEntry(bckgEntry, QString()) == "KDE default")
            backGrounds[ clr ] = QColor();
        else if (gc.readEntry(foreEntry, QString()).isEmpty())    // KDE4 workaround, default color doesn't work
            backGrounds[ clr ] = BCKG_DFLTS[ clr ];
        else
            backGrounds[ clr ] = gc.readEntry(bckgEntry, BCKG_DFLTS[ clr ]);
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

    if (group.readEntry("Window Maximized",  false))
        showMaximized();
    else
        show();

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
        leftDirLabel->setText(i18n("Selected files from targ&et directory:"));
        rightDirLabel->setText(i18n("Selected files from sou&rce directory:"));
    } else if (hasSelectedFiles && !cbAsymmetric->isChecked()) {
        leftDirLabel->setText(i18n("Selected files from &left directory:"));
        rightDirLabel->setText(i18n("Selected files from &right directory:"));
    } else if (cbAsymmetric->isChecked()) {
        leftDirLabel->setText(i18n("Targ&et directory:"));
        rightDirLabel->setText(i18n("Sou&rce directory:"));
    } else {
        leftDirLabel->setText(i18n("&Left directory:"));
        rightDirLabel->setText(i18n("&Right directory:"));
    }
}

void SynchronizerGUI::setCompletion()
{
    generalFilter->dontSearchIn->setCompletionDir(rightLocation->currentText());
}

void SynchronizerGUI::checkExcludeURLValidity(QString &text, QString &error)
{
    KUrl url = KUrl(text);
    if (KUrl::isRelativeUrl(url.url()))
        return;

    QString leftBase = leftLocation->currentText();
    if (!leftBase.endsWith('/'))
        leftBase += '/';
    KUrl leftBaseURL = KUrl(leftBase);
    if (leftBaseURL.isParentOf(url) && !url.isParentOf(leftBaseURL)) {
        text = KUrl::relativeUrl(leftBaseURL, url);
        return;
    }

    QString rightBase = rightLocation->currentText();
    if (!rightBase.endsWith('/'))
        rightBase += '/';
    KUrl rightBaseURL = KUrl(rightBase);
    if (rightBaseURL.isParentOf(url) && !url.isParentOf(rightBaseURL)) {
        text = KUrl::relativeUrl(rightBaseURL, url);
        return;
    }

    error = i18n("URL must be the descendant of either the left or the right base URL!");
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
        KUrl leftURL  = KUrl(synchronizer.leftBaseDirectory()  + leftDirName + item->leftName());
        KUrl rightURL = KUrl(synchronizer.rightBaseDirectory() + rightDirName + item->rightName());

        SLOTS->compareContent(leftURL, rightURL);
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
    KMenu popup;
    QAction *myact;
    QHash< QAction *, int > actHash;

    popup.setTitle(i18n("Synchronize Directories"));

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

    KUrl leftBDir = KUrl(synchronizer.leftBaseDirectory());
    KUrl rightBDir = KUrl(synchronizer.rightBaseDirectory());

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

    KUrl leftURL  = KUrl(synchronizer.leftBaseDirectory()  + leftDirName + item->leftName());
    KUrl rightURL = KUrl(synchronizer.rightBaseDirectory() + rightDirName + item->rightName());

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
        SLOTS->compareContent(leftURL, rightURL);
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

    QStringList list = leftLocation->historyItems();
    KConfigGroup group(krConfig, "Synchronize");
    group.writeEntry("Left Directory History", list);
    list = rightLocation->historyItems();
    group.writeEntry("Right Directory History", list);
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
        ListPanel *p = ACTIVE_PANEL;
        MAIN_VIEW->left->func->refresh();
        MAIN_VIEW->right->func->refresh();
        p->slotFocusOnMe();
    }
}

void SynchronizerGUI::compare()
{
    KRQuery query;

    if (!filterTabs->fillQuery(&query))
        return;

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
        leftSize = isDir ? i18n("<DIR>") + ' ' : KRpermHandler::parseSize(item->leftSize());
        leftDate = SynchronizerGUI::convertTime(item->leftDate());
    }

    if (item->existsInRight()) {
        rightName = item->rightName();
        rightSize = isDir ? i18n("<DIR>") + ' ' : KRpermHandler::parseSize(item->rightSize());
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
        listItem->setIcon(0, isDir ? folderIcon : fileIcon);
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
                leftSize = isDir ? i18n("<DIR>") + ' ' : KRpermHandler::parseSize(item->leftSize());
                leftDate = SynchronizerGUI::convertTime(item->leftDate());
            }

            if (item->existsInRight()) {
                rightName = item->rightName();
                rightSize = isDir ? i18n("<DIR>") + ' ' : KRpermHandler::parseSize(item->rightSize());
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
    return KGlobal::locale()->formatDateTime(tmp);
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
        KMessageBox::sorry(parentWidget(), i18n("Synchronizer has nothing to do!"));
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
            KUrl rightURL = KUrl(synchronizer.rightBaseDirectory() + rightDirName + item->rightName());
            if (isedit)
                KrViewer::edit(rightURL, this);   // view the file
            else
                KrViewer::view(rightURL, this);   // view the file
            return;
        } else if (e->modifiers() == 0 && item->existsInLeft()) {
            KUrl leftURL  = KUrl(synchronizer.leftBaseDirectory()  + leftDirName + item->leftName());
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
                                                       i18n("Krusader::Synchronize Directories"),
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

void SynchronizerGUI::copyToClipboard(bool isLeft)
{
    KUrl::List urls;

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
                KUrl leftURL = KUrl(synchronizer.leftBaseDirectory()  + leftDirName + item->leftName());
                urls.push_back(leftURL);
            } else if (!isLeft && item->existsInRight()) {
                QString rightDirName = item->rightDirectory().isEmpty() ? "" : item->rightDirectory() + '/';
                KUrl rightURL = KUrl(synchronizer.rightBaseDirectory()  + rightDirName + item->rightName());
                urls.push_back(rightURL);
            }
        }
    }

    if (urls.count() == 0)
        return;

    QMimeData *mimeData = new QMimeData;
    mimeData->setImageData(FL_LOADICON(isLeft ? "arrow-left-double" : "arrow-right-double"));
    urls.populateMimeData(mimeData);

    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

#include "synchronizergui.moc"
