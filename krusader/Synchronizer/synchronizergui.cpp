/***************************************************************************
                       synchronizergui.cpp  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
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
#include "../krusader.h"
#include "../defaults.h"
#include "../VFS/krpermhandler.h"
#include "synchronizedialog.h"
#include <qlayout.h>
#include <kurlrequester.h>
#include <qlabel.h>
#include <klocale.h>
#include <qgrid.h>
#include <kpopupmenu.h>
#include <qcursor.h>
#include <time.h>
#include <kmessagebox.h>

/* TODO TODO TODO TODO TODO */
#define  _RecurseSubdirs    true
#define  _FollowSymlinks    false
#define  _CompareByContent  false
#define  _IgnoreDate        false
#define  _Asymmetric        false
#define  _BtnLeftToRight    true
#define  _BtnEquals         true
#define  _BtnDifferents     true
#define  _BtnRightToLeft    true
#define  _BtnDeletable      true
#define  _BtnDuplicates     true
#define  _BtnSingles        true
/* TODO TODO TODO TODO TODO */

static const char * const right_arrow_button_data[] = {
"18 18 97 2",
"  	c None",
". 	c #000000",
"+ 	c #030D03",
"@ 	c #184F16",
"# 	c #5DB45A",
"$ 	c #2E6C2A",
"% 	c #90D28D",
"& 	c #9CD59A",
"* 	c #32732E",
"= 	c #92CF8F",
"- 	c #BDE9BB",
"; 	c #C4E5C3",
"> 	c #447F41",
", 	c #108F08",
"' 	c #0F8E08",
") 	c #108E08",
"! 	c #14970B",
"~ 	c #8DD289",
"{ 	c #87DF7F",
"] 	c #D6F1D3",
"^ 	c #C3E1C1",
"/ 	c #488844",
"( 	c #73D56C",
"_ 	c #D1F4D0",
": 	c #F3FCF3",
"< 	c #F7FEF7",
"[ 	c #EFFCEF",
"} 	c #DCF6DB",
"| 	c #B4EAB0",
"1 	c #70D965",
"2 	c #2AC71B",
"3 	c #68D85D",
"4 	c #CFF1CB",
"5 	c #DCEFDB",
"6 	c #589955",
"7 	c #74D46D",
"8 	c #9DDF98",
"9 	c #ABE8A5",
"0 	c #B8EEB4",
"a 	c #9EE797",
"b 	c #74DD6A",
"c 	c #62D758",
"d 	c #23C512",
"e 	c #15BC07",
"f 	c #27C519",
"g 	c #73DC69",
"h 	c #BAEDB6",
"i 	c #B8E7B6",
"j 	c #499145",
"k 	c #77D671",
"l 	c #7BD874",
"m 	c #3AC72C",
"n 	c #42C437",
"o 	c #34C526",
"p 	c #1FC40F",
"q 	c #24C516",
"r 	c #1AB70E",
"s 	c #1ABC0D",
"t 	c #20C411",
"u 	c #5AD94B",
"v 	c #5DE24A",
"w 	c #36C229",
"x 	c #177B11",
"y 	c #7DDA75",
"z 	c #84E07B",
"A 	c #2BC519",
"B 	c #2FBF1F",
"C 	c #33C623",
"D 	c #28C716",
"E 	c #22CC11",
"F 	c #20C511",
"G 	c #23CC13",
"H 	c #31D81C",
"I 	c #4FE03B",
"J 	c #34C725",
"K 	c #137F0D",
"L 	c #157C0E",
"M 	c #126B0F",
"N 	c #126A0E",
"O 	c #116C0C",
"P 	c #13760E",
"Q 	c #179A0E",
"R 	c #37DC22",
"S 	c #4DDF38",
"T 	c #2AB71D",
"U 	c #12720D",
"V 	c #010C00",
"W 	c #28C715",
"X 	c #47DF32",
"Y 	c #2DBD1F",
"Z 	c #116B0D",
"` 	c #2CB122",
" .	c #23AD18",
"..	c #10620C",
"+.	c #289023",
"@.	c #125B0E",
"#.	c #094706",
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
"                                    "};
 
static const char * const equals_button_data[] = {
"18 18 5 1",
" 	c None",
".	c #414100",
"+	c #E0E0E0",
"@	c #A8A8A8",
"#	c #808080",
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
"                  "};

static const char * const differents_button_data[] = {
"18 18 5 1",
" 	c None",
".	c #FF0000",
"+	c #FFC0C0",
"@	c #FF8080",
"#	c #FF4040",
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
"                  "};

static const char * const left_arrow_button_data[] = {
"18 18 137 2",
"  	c None",
". 	c #03090E",
"+ 	c #0D3A57",
"@ 	c #041F2B",
"# 	c #073347",
"$ 	c #0D3C5A",
"% 	c #051C26",
"& 	c #0F455C",
"* 	c #237191",
"= 	c #104363",
"- 	c #04121A",
"; 	c #0C4A62",
"> 	c #198AAD",
", 	c #2291B2",
"' 	c #104564",
") 	c #062332",
"! 	c #0D506B",
"~ 	c #209FBD",
"{ 	c #33CBDF",
"] 	c #16ACC8",
"^ 	c #0C4968",
"/ 	c #061F2D",
"( 	c #031721",
"_ 	c #041621",
": 	c #051721",
"< 	c #021621",
"[ 	c #031B27",
"} 	c #01090D",
"| 	c #04151E",
"1 	c #0D5672",
"2 	c #1E99B8",
"3 	c #39CEDF",
"4 	c #22C5DC",
"5 	c #10A1C4",
"6 	c #0E799B",
"7 	c #0E5976",
"8 	c #0D516D",
"9 	c #0F4E6B",
"0 	c #0F4D6A",
"a 	c #0F607D",
"b 	c #031D25",
"c 	c #052837",
"d 	c #0D617F",
"e 	c #25ABC7",
"f 	c #3BD0E1",
"g 	c #1DC0D9",
"h 	c #14A8CC",
"i 	c #11A3C5",
"j 	c #11ABCC",
"k 	c #17AAC8",
"l 	c #23ACC6",
"m 	c #1FA8C0",
"n 	c #1AAAC5",
"o 	c #7CCDE1",
"p 	c #76C4DB",
"q 	c #032832",
"r 	c #061D28",
"s 	c #125F7C",
"t 	c #29A6C3",
"u 	c #4BD4E3",
"v 	c #4BC5DA",
"w 	c #129FC4",
"x 	c #0D95BC",
"y 	c #0F90B7",
"z 	c #16A2C5",
"A 	c #0FA3C4",
"B 	c #26A8C5",
"C 	c #37A8C4",
"D 	c #2DA9C7",
"E 	c #75C1D9",
"F 	c #71BED6",
"G 	c #0A212C",
"H 	c #467B92",
"I 	c #B6D9E8",
"J 	c #B6E2ED",
"K 	c #69C7DC",
"L 	c #19A2C5",
"M 	c #0796BC",
"N 	c #13A5C5",
"O 	c #59BBD7",
"P 	c #6BC5DD",
"Q 	c #98D8E8",
"R 	c #B4E2EE",
"S 	c #A6DCE9",
"T 	c #98CFDF",
"U 	c #6DBCD4",
"V 	c #143341",
"W 	c #56859A",
"X 	c #DCEAF0",
"Y 	c #CCEAF2",
"Z 	c #5EC2D9",
"` 	c #1BA7C8",
" .	c #66C4DA",
"..	c #B1DBEB",
"+.	c #DBEEF6",
"@.	c #EFF6FC",
"#.	c #F7FAFE",
"$.	c #F3F8FC",
"%.	c #D0EAF4",
"&.	c #6CBCD5",
"*.	c #091A21",
"=.	c #457589",
"-.	c #C2D8E2",
";.	c #D4ECF2",
">.	c #80CCDF",
",.	c #8ABFD3",
"'.	c #0C7497",
").	c #086E90",
"!.	c #086C8E",
"~.	c #086B8E",
"{.	c #086D90",
"].	c #021E27",
"^.	c #0D2B38",
"/.	c #426D80",
"(.	c #C3DAE5",
"_.	c #BCDCEA",
":.	c #90BDD0",
"<.	c #144361",
"[.	c #002745",
"}.	c #00213F",
"|.	c #001F3E",
"1.	c #00203F",
"2.	c #002643",
"3.	c #000B13",
"4.	c #03161D",
"5.	c #2F5F73",
"6.	c #9AC3D5",
"7.	c #8EBED3",
"8.	c #1A4A68",
"9.	c #0C222B",
"0.	c #2B5A6D",
"a.	c #5A98B4",
"b.	c #164867",
"c.	c #0F2731",
"d.	c #163E50",
"e.	c #0E3E5C",
"f.	c #0C3652",
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
"                                    "};

static const char * const trash_button_data[] = {
"18 18 140 2",
"  	c None",
". 	c #BFBFBF",
"+ 	c #BABAB9",
"@ 	c #AEAEAE",
"# 	c #A2A2A3",
"$ 	c #959595",
"% 	c #8B8B8C",
"& 	c #868687",
"* 	c #D3D5D5",
"= 	c #E1E1E1",
"- 	c #CCCCCD",
"; 	c #BDBEBD",
"> 	c #B1B2B1",
", 	c #A3A2A2",
"' 	c #959597",
") 	c #8E8E8F",
"! 	c #818282",
"~ 	c #727171",
"{ 	c #838384",
"] 	c #D1D1D1",
"^ 	c #F3F3F3",
"/ 	c #C6C7C6",
"( 	c #B8B9B9",
"_ 	c #ABABAB",
": 	c #9F9FA0",
"< 	c #949394",
"[ 	c #8E8E8E",
"} 	c #7E8080",
"| 	c #717071",
"1 	c #5C5C5B",
"2 	c #555556",
"3 	c #A7A7A7",
"4 	c #FAFAFA",
"5 	c #CACACA",
"6 	c #BABBBB",
"7 	c #B5B6B6",
"8 	c #A9A9AA",
"9 	c #9E9E9D",
"0 	c #929293",
"a 	c #8E8C8D",
"b 	c #7F7F7F",
"c 	c #6F6F70",
"d 	c #525151",
"e 	c #414141",
"f 	c #A1A2A2",
"g 	c #C3C3C2",
"h 	c #D5D4D4",
"i 	c #ECECEC",
"j 	c #E7E7E7",
"k 	c #D6D6D6",
"l 	c #C5C5C6",
"m 	c #B0B0B0",
"n 	c #AAAAAA",
"o 	c #989898",
"p 	c #6D6D6E",
"q 	c #494949",
"r 	c #9E9E9E",
"s 	c #C0C1C1",
"t 	c #BABABA",
"u 	c #B2B2B2",
"v 	c #AEAEAD",
"w 	c #A4A4A4",
"x 	c #9B9B9B",
"y 	c #8E8F8F",
"z 	c #888888",
"A 	c #767676",
"B 	c #616161",
"C 	c #B3B3B3",
"D 	c #B9B9BA",
"E 	c #A4A5A4",
"F 	c #979797",
"G 	c #888788",
"H 	c #6D6D6D",
"I 	c #4D4D4D",
"J 	c #4B4A4B",
"K 	c #F6F6F6",
"L 	c #B1B1B1",
"M 	c #A7A8A7",
"N 	c #939394",
"O 	c #8D8D8E",
"P 	c #727272",
"Q 	c #505050",
"R 	c #484848",
"S 	c #EEEEEE",
"T 	c #EBEBEB",
"U 	c #9D9D9C",
"V 	c #919292",
"W 	c #8C8C8C",
"X 	c #808080",
"Y 	c #6C6B6C",
"Z 	c #E5E5E5",
"` 	c #AFAFAF",
" .	c #A6A6A6",
"..	c #8F908F",
"+.	c #888989",
"@.	c #7B7B7B",
"#.	c #676667",
"$.	c #4F4F4F",
"%.	c #AFB0AF",
"&.	c #D8D8D8",
"*.	c #D4D4D4",
"=.	c #A5A5A4",
"-.	c #9A9A9A",
";.	c #8D8D8D",
">.	c #777777",
",.	c #5F5F5F",
"'.	c #4B4B4B",
").	c #B0AFAF",
"!.	c #D3D2D2",
"~.	c #CDCDCD",
"{.	c #A4A5A5",
"].	c #8D8D8B",
"^.	c #868685",
"/.	c #747474",
"(.	c #5D5D5D",
"_.	c #AEAFAE",
":.	c #C4C4C4",
"<.	c #BEBEBE",
"[.	c #A3A4A3",
"}.	c #8B8A8A",
"|.	c #838282",
"1.	c #707070",
"2.	c #565656",
"3.	c #444444",
"4.	c #000000",
"5.	c #B2B1B1",
"6.	c #ADADAD",
"7.	c #A3A3A3",
"8.	c #979697",
"9.	c #6C6C6C",
"0.	c #403F3F",
"a.	c #B1B2B2",
"b.	c #9F9F9F",
"c.	c #A6A6A7",
"d.	c #9E9F9F",
"e.	c #949494",
"f.	c #828282",
"g.	c #787979",
"h.	c #3D3D3C",
"i.	c #2F2F2F",
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
"                                    "};

static const char * const file_data[] = {
"16 16 42 1",
" 	c None",
".	c #D0D0DF",
"+	c #9C9CB6",
"@	c #FFFFFF",
"#	c #F9F9FE",
"$	c #F5F5FC",
"%	c #E9E9F2",
"&	c #EBEBF4",
"*	c #FCFCFF",
"=	c #F8F8FE",
"-	c #ECECF4",
";	c #D3D3E1",
">	c #EFEFF6",
",	c #FDFDFF",
"'	c #F1F1F8",
")	c #E6E6F0",
"!	c #D7D7E5",
"~	c #C9C9DA",
"{	c #FEFEFF",
"]	c #F2F2F9",
"^	c #EEEEF5",
"/	c #DADAE7",
"(	c #CECEDD",
"_	c #CCCCDB",
":	c #F3F3F9",
"<	c #D5D5E4",
"[	c #D2D2E0",
"}	c #E7E7F0",
"|	c #E0E0EC",
"1	c #DCDCE9",
"2	c #DBDBE8",
"3	c #D8D8E6",
"4	c #F6F6FD",
"5	c #E5E5EF",
"6	c #DEDEEB",
"7	c #F0F0F7",
"8	c #EAEAF3",
"9	c #E8E8F1",
"0	c #E1E1ED",
"a	c #F4F4FA",
"b	c #E4E4EE",
"c	c #FAFAFF",
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
"  ++++++++++++  "};

static const char * const folder_data[] = {
"16 16 132 2",
"  	c None",
". 	c #2C87EF",
"+ 	c #64A6F7",
"@ 	c #357CE3",
"# 	c #D9E1F8",
"$ 	c #B1BADE",
"% 	c #8BA2D9",
"& 	c #4392E3",
"* 	c #B8D9F8",
"= 	c #4F9BED",
"- 	c #126EE0",
"; 	c #0A43A8",
"> 	c #C6CBDA",
", 	c #FFFFFF",
"' 	c #F1F0F6",
") 	c #80ADE2",
"! 	c #4F95E4",
"~ 	c #1876DF",
"{ 	c #0C6ADF",
"] 	c #0C4CB9",
"^ 	c #CCCFDA",
"/ 	c #F8F4F7",
"( 	c #89B7E7",
"_ 	c #78ACE4",
": 	c #B4D8F8",
"< 	c #97CAFF",
"[ 	c #6FAEFC",
"} 	c #3175D8",
"| 	c #0E51B4",
"1 	c #002374",
"2 	c #D0D0D7",
"3 	c #8EC1F0",
"4 	c #85BBED",
"5 	c #DAF0F9",
"6 	c #BDE2FB",
"7 	c #9CCCF8",
"8 	c #84BBF8",
"9 	c #6FAAF4",
"0 	c #4780D4",
"a 	c #0851AA",
"b 	c #0035A9",
"c 	c #CFD0D4",
"d 	c #51A2E9",
"e 	c #FFFFFE",
"f 	c #EEFCFD",
"g 	c #CEECFB",
"h 	c #B1D9F9",
"i 	c #9AC9F9",
"j 	c #7EB3F2",
"k 	c #568CDA",
"l 	c #1156BA",
"m 	c #004595",
"n 	c #003293",
"o 	c #EFEFEE",
"p 	c #84BCEE",
"q 	c #E2F8FC",
"r 	c #C9E8FB",
"s 	c #B0D8FA",
"t 	c #90C0F3",
"u 	c #6B9FE5",
"v 	c #3375CC",
"w 	c #2A71C7",
"x 	c #003B96",
"y 	c #0651AE",
"z 	c #0E3DAC",
"A 	c #E5E3E0",
"B 	c #DFD8D5",
"C 	c #FFF7F2",
"D 	c #DEEFFE",
"E 	c #BEDCF6",
"F 	c #E5FCFD",
"G 	c #C4E6FB",
"H 	c #A8D4F8",
"I 	c #85B6EC",
"J 	c #437DCE",
"K 	c #2170C9",
"L 	c #397CC8",
"M 	c #A3B6D4",
"N 	c #E3D3D2",
"O 	c #295BC3",
"P 	c #4AACFF",
"Q 	c #74A7D5",
"R 	c #CBC7CE",
"S 	c #7EB0E7",
"T 	c #F5FFFF",
"U 	c #C1E7FE",
"V 	c #9AC8F3",
"W 	c #4B84D3",
"X 	c #5490D9",
"Y 	c #B3C7E3",
"Z 	c #E9DFDF",
"` 	c #D3CED8",
" .	c #D7CFD3",
"..	c #7488C1",
"+.	c #002A95",
"@.	c #6FCDFF",
"#.	c #CBEAFF",
"$.	c #D9F9FF",
"%.	c #70AAE1",
"&.	c #C7E0EE",
"*.	c #9DCBF1",
"=.	c #84AEE4",
"-.	c #D0E1F4",
";.	c #FFF9F4",
">.	c #EEE9EB",
",.	c #E6E2E5",
"'.	c #D9D1D6",
").	c #637DC0",
"!.	c #2D63D3",
"~.	c #001251",
"{.	c #439FF0",
"].	c #B4DBF9",
"^.	c #D6F8FF",
"/.	c #75ACE3",
"(.	c #F0FAFF",
"_.	c #FCF9F8",
":.	c #91A5D4",
"<.	c #2A5FCE",
"[.	c #002B91",
"}.	c #3E9DEF",
"|.	c #A4CEF4",
"1.	c #77AFE8",
"2.	c #E1EBFC",
"3.	c #3F73D7",
"4.	c #043BAE",
"5.	c #3188DE",
"6.	c #75A9E3",
"7.	c #A8C6EC",
"8.	c #6195E7",
"9.	c #1450C5",
"0.	c #7AB0FB",
"a.	c #155ED2",
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
"                                "};

SynchronizerGUI::SynchronizerGUI(QWidget* parent,  QString leftDirectory, QString rightDirectory ) :
    QDialog( parent, "Krusader::SynchronizerGUI", true, 0 ), isComparing( false ), wasClosed( false )
{
  setCaption( i18n("Krusader::Synchronize Directories") );
  QGridLayout *synchGrid = new QGridLayout( this );
  synchGrid->setSpacing( 6 );
  synchGrid->setMargin( 11 );

  folderIcon =   QPixmap( ( const char** ) folder_data );
  fileIcon   =   QPixmap( ( const char** ) file_data );

  /* ============================== Compare groupbox ============================== */
  
  QGroupBox *compareDirs = new QGroupBox( this, "SyncCompareDirectories" );
  compareDirs->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed);
  compareDirs->setTitle( i18n( "Directory Comparation" ) );
  compareDirs->setColumnLayout(0, Qt::Vertical );
  compareDirs->layout()->setSpacing( 0 );
  compareDirs->layout()->setMargin( 0 );
  
  QGridLayout *grid = new QGridLayout( compareDirs->layout() );
  grid->setSpacing( 6 );
  grid->setMargin( 11 );

   
  QLabel *leftDirLabel = new QLabel( compareDirs, "leftDirLabel" );
  leftDirLabel->setText( i18n( "Left directory:" ) );
  leftDirLabel->setAlignment( Qt::AlignHCenter );
  grid->addWidget( leftDirLabel, 0 ,0 );

  QLabel *filterLabel = new QLabel( compareDirs, "filterLabel" );
  filterLabel->setText( i18n( "File Filter:" ) );
  filterLabel->setAlignment( Qt::AlignHCenter );
  grid->addWidget( filterLabel, 0 ,1 );

  QLabel *rightDirLabel = new QLabel( compareDirs, "rightDirLabel" );
  rightDirLabel->setText( i18n( "Right directory:" ) );
  rightDirLabel->setAlignment( Qt::AlignHCenter );
  grid->addWidget( rightDirLabel, 0 ,2 );

  krConfig->setGroup("Synchronize");
  
  leftLocation = new KHistoryCombo(false, compareDirs, "SynchronizerHistoryLeft");
  leftLocation->setEditable( true );
  leftLocation->setMinimumWidth( 250 );  
  QStringList list = krConfig->readListEntry("Left Directory History");
  leftLocation->setHistoryItems(list);
  KURLRequester *leftUrlReq = new KURLRequester( leftLocation, compareDirs, "LeftDirectory" );
  leftUrlReq->setURL( leftDirectory );
  leftUrlReq->setMode( KFile::Directory );
  grid->addWidget( leftUrlReq, 1 ,0 );

  fileFilter = new KHistoryCombo(false, compareDirs, "SynchronizerFilter");
  fileFilter->setMinimumWidth( 100 );
  fileFilter->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
  list = krConfig->readListEntry("File Filter");
  fileFilter->setHistoryItems(list);
  fileFilter->setEditText("*");
  grid->addWidget( fileFilter, 1 ,1 );

  rightLocation = new KHistoryCombo(compareDirs, "SynchronizerHistoryRight");
  rightLocation->setEditable( true );
  rightLocation->setMinimumWidth( 250 );  
  list = krConfig->readListEntry("Right Directory History");
  rightLocation->setHistoryItems(list);
  KURLRequester *rightUrlReq = new KURLRequester( rightLocation, compareDirs, "RightDirectory" );
  rightUrlReq->setURL( rightDirectory );
  rightUrlReq->setMode( KFile::Directory );
  grid->addWidget( rightUrlReq, 1 ,2 );

  QHBox *optionBox  = new QHBox( compareDirs );
  QGrid *optionGrid = new QGrid( 3, optionBox );
  cbSubdirs         = new QCheckBox( i18n( "Recurse subdirectories" ), optionGrid, "cbSubdirs" );
  cbSubdirs->setChecked( krConfig->readBoolEntry( "Recurse Subdirectories", _RecurseSubdirs  ) );
  cbSymlinks        = new QCheckBox( i18n( "Follow symlinks" ), optionGrid, "cbSymlinks" );
  cbSymlinks->setChecked( krConfig->readBoolEntry( "Follow Symlinks", _FollowSymlinks  ) );
  cbSymlinks->setEnabled( cbSubdirs->isChecked() );
  cbByContent       = new QCheckBox( i18n( "Compare by content" ), optionGrid, "cbByContent" );
  cbByContent->setChecked( krConfig->readBoolEntry( "Compare By Content", _CompareByContent  ) );
  cbByContent->setEnabled( false ); /* TODO TODO TODO TODO TODO */
  cbIgnoreDate      = new QCheckBox( i18n( "Ignore Date" ), optionGrid, "cbIgnoreDate" );
  cbIgnoreDate->setChecked( krConfig->readBoolEntry( "Ignore Date", _IgnoreDate  ) );
  cbAsymmetric      = new QCheckBox( i18n( "Asymmetric" ), optionGrid, "cbAsymmetric" );
  cbAsymmetric->setChecked( krConfig->readBoolEntry( "Asymmetric", _Asymmetric  ) );

  /* =========================== Show options groupbox ============================= */
  
  QGroupBox *showOptions  = new QGroupBox( optionBox, "SyncOptionBox" );
  showOptions->setTitle( i18n( "Show options" ) );
  showOptions->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed);
  showOptions->setColumnLayout(0, Qt::Vertical );
  showOptions->layout()->setSpacing( 0 );
  showOptions->layout()->setMargin( 0 );
  QGridLayout *showOptionsLayout = new QGridLayout( showOptions->layout() );
  showOptionsLayout->setSpacing( 6 );
  showOptionsLayout->setMargin( 11 );

  QPixmap showLeftToRight( ( const char** ) right_arrow_button_data );
  QPixmap showEquals     ( ( const char** ) equals_button_data );
  QPixmap showDifferents ( ( const char** ) differents_button_data );
  QPixmap showRightToLeft( ( const char** ) left_arrow_button_data );
  QPixmap showDeletable  ( ( const char** ) trash_button_data );

  btnLeftToRight = new QPushButton( showOptions, "btnLeftToRight" );
  btnLeftToRight->setText( "" );
  btnLeftToRight->setPixmap( showLeftToRight );
  btnLeftToRight->setToggleButton( true );
  btnLeftToRight->setOn( krConfig->readBoolEntry( "LeftToRight Button", _BtnLeftToRight ) );
  showOptionsLayout->addWidget( btnLeftToRight, 0, 0);
  
  btnEquals = new QPushButton( showOptions, "btnEquals" );
  btnEquals->setText( "" );
  btnEquals->setPixmap( showEquals );
  btnEquals->setToggleButton( true );
  btnEquals->setOn( krConfig->readBoolEntry( "Equals Button", _BtnEquals ) );
  showOptionsLayout->addWidget( btnEquals, 0, 1);

  btnDifferents = new QPushButton( showOptions, "btnDifferents" );
  btnDifferents->setText( "" );
  btnDifferents->setPixmap( showDifferents );
  btnDifferents->setToggleButton( true );
  btnDifferents->setOn( krConfig->readBoolEntry( "Differents Button", _BtnDifferents ) );
  showOptionsLayout->addWidget( btnDifferents, 0, 2);

  btnRightToLeft = new QPushButton( showOptions, "btnRightToLeft" );
  btnRightToLeft->setText( "" );
  btnRightToLeft->setPixmap( showRightToLeft );
  btnRightToLeft->setToggleButton( true );
  btnRightToLeft->setOn( krConfig->readBoolEntry( "RightToLeft Button", _BtnRightToLeft ) );
  showOptionsLayout->addWidget( btnRightToLeft, 0, 3);

  btnDeletable = new QPushButton( showOptions, "btnDeletable" );
  btnDeletable->setText( "" );
  btnDeletable->setPixmap( showDeletable );
  btnDeletable->setToggleButton( true );
  btnDeletable->setOn( krConfig->readBoolEntry( "Deletable Button", _BtnDeletable ) );
  showOptionsLayout->addWidget( btnDeletable, 0, 4);

  btnDuplicates = new QPushButton( showOptions, "btnDuplicates" );
  btnDuplicates->setText( i18n("Duplicates") );
  btnDuplicates->setMinimumHeight( btnLeftToRight->height() );
  btnDuplicates->setToggleButton( true );
  btnDuplicates->setOn( krConfig->readBoolEntry( "Duplicates Button", _BtnDuplicates ) );
  showOptionsLayout->addWidget( btnDuplicates, 0, 5);

  btnSingles = new QPushButton( showOptions, "btnSingles" );
  btnSingles->setText( i18n("Singles") );
  btnSingles->setMinimumHeight( btnLeftToRight->height() );
  btnSingles->setToggleButton( true );
  btnSingles->setOn( krConfig->readBoolEntry( "Singles Button", _BtnSingles ) );
  showOptionsLayout->addWidget( btnSingles, 0, 6);

  grid->addMultiCellWidget( optionBox, 2, 2, 0, 2 );
  
  synchGrid->addWidget( compareDirs, 0, 0 );

  /* ========================= Synchronization list view ========================== */

  syncList=new QListView( this );  // create the main container
  
  krConfig->setGroup("Look&Feel");
  syncList->setFont(krConfig->readFontEntry("Filelist Font",_FilelistFont));

  syncList->setAllColumnsShowFocus(true);
  syncList->setMultiSelection(true);
  syncList->setSelectionMode(QListView::Extended);
  syncList->setVScrollBarMode(QScrollView::Auto);
  syncList->setHScrollBarMode(QScrollView::Auto);
  syncList->setShowSortIndicator(false);
  syncList->setSorting(-1);
  syncList->setRootIsDecorated( true );
  syncList->setTreeStepSize( 10 );
  int i=QFontMetrics(syncList->font()).width("W");
  int j=QFontMetrics(syncList->font()).width("0");
  j=(i>j ? i : j);
  int typeWidth = j*7/2;

  int leftNameWidth  = 9*typeWidth/2;
  int rightNameWidth = 4*typeWidth;
  int sizeWidth = 2*typeWidth;
  int dateWidth = 3*typeWidth;
  
  syncList->addColumn(i18n("Name"),leftNameWidth);
  syncList->addColumn(i18n("Size"),sizeWidth);
  syncList->addColumn(i18n("Date"),dateWidth);
  syncList->addColumn(i18n("<=>") ,typeWidth);
  syncList->addColumn(i18n("Date"),dateWidth);
  syncList->addColumn(i18n("Size"),sizeWidth);
  syncList->addColumn(i18n("Name"),rightNameWidth);
  syncList->setColumnWidthMode(0,QListView::Manual);
  syncList->setColumnWidthMode(1,QListView::Manual);
  syncList->setColumnWidthMode(2,QListView::Manual);
  syncList->setColumnWidthMode(3,QListView::Manual);
  syncList->setColumnWidthMode(4,QListView::Manual);
  syncList->setColumnWidthMode(5,QListView::Manual);
  syncList->setColumnWidthMode(6,QListView::Manual);
  syncList->setColumnAlignment(1, Qt::AlignRight );
  syncList->setColumnAlignment(3, Qt::AlignHCenter );
  syncList->setColumnAlignment(5, Qt::AlignRight );

  synchGrid->addWidget(syncList,1,0);
  
  /* ================================== Buttons =================================== */

  QHBoxLayout *buttons = new QHBoxLayout;
  buttons->setSpacing( 6 );
  buttons->setMargin( 0 );

  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  buttons->addItem( spacer );

  btnCompareDirs = new QPushButton( this, "btnCompareDirs" );
  btnCompareDirs->setText( i18n( "Compare" ) );
  btnCompareDirs->setDefault(true);
  buttons->addWidget( btnCompareDirs );

  btnStopComparing = new QPushButton( this, "btnStopComparing" );
  btnStopComparing->setText( i18n( "Stop" ) );
  btnStopComparing->setEnabled(false);
  buttons->addWidget( btnStopComparing );

  btnSynchronize = new QPushButton( this, "btnSynchronize" );
  btnSynchronize->setText( i18n( "Synchronize" ) );
  btnSynchronize->setEnabled(false);
  buttons->addWidget( btnSynchronize );

  QPushButton *btnCloseSync = new QPushButton( this, "btnCloseSync" );
  btnCloseSync->setText( i18n( "Close" ) );
  buttons->addWidget( btnCloseSync );
  
  synchGrid->addLayout( buttons, 2, 0 );

  /* =============================== Connect table ================================ */

  connect( syncList,SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
           this, SLOT(rightMouseClicked(QListViewItem *)));

  connect( btnCompareDirs,    SIGNAL( clicked() ), this, SLOT( compare() ) );
  connect( btnStopComparing,  SIGNAL( clicked() ), this, SLOT( stop() ) );
  connect( btnSynchronize,    SIGNAL( clicked() ), this, SLOT( synchronize() ) );
  connect( btnCloseSync,      SIGNAL( clicked() ), this, SLOT( closeDialog() ) );

  connect( cbSubdirs,         SIGNAL( toggled(bool) ), this, SLOT( subdirsChecked( bool ) ) );
  
  connect( &synchronizer,     SIGNAL( comparedFileData( SynchronizerFileItem * ) ), this,
                              SLOT( addFile( SynchronizerFileItem * ) ) );

  connect( btnLeftToRight,    SIGNAL( toggled(bool) ), this, SLOT( refresh() ) );
  connect( btnEquals,         SIGNAL( toggled(bool) ), this, SLOT( refresh() ) );
  connect( btnDifferents,     SIGNAL( toggled(bool) ), this, SLOT( refresh() ) );
  connect( btnRightToLeft,    SIGNAL( toggled(bool) ), this, SLOT( refresh() ) );
  connect( btnDeletable,      SIGNAL( toggled(bool) ), this, SLOT( refresh() ) );
  connect( btnDuplicates,     SIGNAL( toggled(bool) ), this, SLOT( refresh() ) );
  connect( btnSingles,        SIGNAL( toggled(bool) ), this, SLOT( refresh() ) );
  exec();
}

SynchronizerGUI::~SynchronizerGUI()
{
  syncList->clear(); // for sanity: deletes the references to the synchronizer list
}

void SynchronizerGUI::rightMouseClicked(QListViewItem *itemIn)
{  
  // these are the values that will exist in the menu
  #define TODO_ID        90
  //////////////////////////////////////////////////////////
  if (!itemIn)
    return;

//  SyncViewItem *item = (SyncViewItem *)itemIn;

  // create the menu
  KPopupMenu popup;
  popup.insertTitle(i18n("Synchronize Directories"));

  popup.insertItem(i18n("TODO"),TODO_ID);
  popup.setItemEnabled(TODO_ID,false);

  int result=popup.exec(QCursor::pos());

  // check out the user's option
  switch (result)
  {
    case -1 : return;     // the user clicked outside of the menu
    case TODO_ID   :
      break;
  }  
}

void SynchronizerGUI::closeDialog()
{
  if( isComparing )
  {
    stop();
    wasClosed = true;
    return;
  }
  
  QStringList list = leftLocation->historyItems();
  krConfig->setGroup("Synchronize");
  krConfig->writeEntry("Left Directory History", list);
  list = rightLocation->historyItems();
  krConfig->writeEntry("Right Directory History", list);
  list = fileFilter->historyItems();
  krConfig->writeEntry("File Filter", list);

  krConfig->writeEntry("Recurse Subdirectories", cbSubdirs->isChecked() );
  krConfig->writeEntry("Follow Symlinks", cbSymlinks->isChecked() );
  krConfig->writeEntry("Compare By Content", cbByContent->isChecked() );
  krConfig->writeEntry("Ignore Date", cbIgnoreDate->isChecked() );
  krConfig->writeEntry("Asymmetric", cbAsymmetric->isChecked() );
  krConfig->writeEntry("LeftToRight Button", btnLeftToRight->isOn() );
  krConfig->writeEntry("Equals Button", btnEquals->isOn() );
  krConfig->writeEntry("Differents Button", btnDifferents->isOn() );
  krConfig->writeEntry("RightToLeft Button", btnRightToLeft->isOn() );
  krConfig->writeEntry("Deletable Button", btnDeletable->isOn() );
  krConfig->writeEntry("Duplicates Button", btnDuplicates->isOn() );
  krConfig->writeEntry("Singles Button", btnSingles->isOn() );
 
  QDialog::reject();
}

void SynchronizerGUI::compare()
{
  syncList->clear();
  lastItem = 0;
  
  leftLocation->addToHistory(leftLocation->currentText());
  rightLocation->addToHistory(rightLocation->currentText());
  fileFilter->addToHistory(fileFilter->currentText());

  setMarkFlags();
  
  btnCompareDirs->setEnabled( false );
  btnStopComparing->setEnabled( isComparing = true );
  btnSynchronize->setEnabled( false );
  synchronizer.compare(leftLocation->currentText(), rightLocation->currentText(), fileFilter->currentText(),
                       cbSubdirs->isChecked(), cbSymlinks->isChecked(), cbIgnoreDate->isChecked(),
                       cbAsymmetric->isChecked() );
  btnStopComparing->setEnabled( isComparing = false );
  btnCompareDirs->setEnabled( true );
  if( syncList->childCount() )
    btnSynchronize->setEnabled( true );

  if( wasClosed )
    closeDialog();
}

void SynchronizerGUI::stop()
{
  synchronizer.stop();
}

void SynchronizerGUI::reject()
{
  closeDialog();
}

void SynchronizerGUI::addFile( SynchronizerFileItem *item )
{
  QString leftName="", rightName="", leftDate="", rightDate="", leftSize="", rightSize="";
  bool    isDir = item->isDir();

  if( item->existsInLeft() )
  {
    leftName = item->name();
    leftSize = isDir ? i18n("<DIR>")+" " : KRpermHandler::parseSize( item->leftSize() );
    leftDate = SynchronizerGUI::convertTime( item->leftDate() );
  }
  
  if( item->existsInRight() )
  {
    rightName = item->name();
    rightSize = isDir ? i18n("<DIR>")+" " : KRpermHandler::parseSize( item->rightSize() );
    rightDate = SynchronizerGUI::convertTime( item->rightDate() );
  }

  SyncViewItem *listItem;
  SyncViewItem *dirItem;
  
  if( item->parent() == 0 )
  {
    listItem = new SyncViewItem(item, syncList, lastItem, leftName, leftSize, leftDate,
                                Synchronizer::getTaskTypeName( item->task() ), rightDate,
                                rightSize, rightName );
    lastItem = listItem;
  }
  else
  {
    dirItem = (SyncViewItem *)item->parent()->userData();
    if( dirItem )
    {
      dirItem->setOpen( true );
      listItem = new SyncViewItem(item, dirItem, dirItem->lastItem(), leftName, leftSize,
                                  leftDate, Synchronizer::getTaskTypeName( item->task() ),
                                  rightDate, rightSize, rightName );
    }

    dirItem->setLastItem( listItem );
  }

  listItem->setPixmap(0, isDir ? folderIcon : fileIcon);
  syncList->ensureItemVisible( listItem );
}

void SynchronizerGUI::subdirsChecked( bool isOn )
{
  cbSymlinks->setEnabled( isOn );
}

QString SynchronizerGUI::convertTime(time_t time) const
{
   // convert the time_t to struct tm
   struct tm* t=localtime((time_t *)&time);

   QDateTime tmp(QDate(t->tm_year+1900, t->tm_mon+1, t->tm_mday), QTime(t->tm_hour, t->tm_min));
   return KGlobal::locale()->formatDateTime(tmp);
}

void SynchronizerGUI::setMarkFlags()
{
  synchronizer.setMarkFlags( btnRightToLeft->isOn(), btnEquals->isOn(), btnDifferents->isOn(),
                             btnLeftToRight->isOn(), btnDuplicates->isOn(), btnSingles->isOn(),
                             btnDeletable->isOn() );
}

void SynchronizerGUI::refresh()
{
  if( !isComparing )
  {
    syncList->clear();
    setMarkFlags();
    btnCompareDirs->setEnabled( false );
    btnSynchronize->setEnabled( false );
    synchronizer.refresh();
    btnCompareDirs->setEnabled( true );
    if( syncList->childCount() )
      btnSynchronize->setEnabled( true );    
  }
}

void SynchronizerGUI::synchronize()
{
  int             copyToLeftNr, copyToRightNr, deleteNr;
  KIO::filesize_t copyToLeftSize, copyToRightSize, deleteSize;

  if( !synchronizer.totalSizes( &copyToLeftNr, &copyToLeftSize, &copyToRightNr, &copyToRightSize,
                                &deleteNr, &deleteSize ) )
  {
    KMessageBox::sorry(0, i18n("The directories are identical!"));
    return;
  }
  
  SynchronizeDialog *sd = new SynchronizeDialog( this, "SychDialog", true, 0, &synchronizer,
                                                 copyToLeftNr, copyToLeftSize, copyToRightNr,
                                                 copyToRightSize, deleteNr, deleteSize );

  bool syncStarted = sd->wasSyncronizationStarted();
  delete sd;

  if( syncStarted )
    closeDialog();
}

