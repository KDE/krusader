/****************************************************************************
** Form implementation generated from reading ui file 'newftpgui.ui'
**
** Created: Fri Oct 27 23:47:10 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "newftpgui.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kcombobox.h>
#include "../krusader.h"

static const char* const image0_data[] = { 
"48 48 594 2",
".a c None",
".# c None",
"Qt c None",
".q c #000000",
"f2 c #000408",
"gN c #000418",
"f3 c #008100",
"f4 c #008508",
"fV c #080408",
"fW c #080820",
"g. c #088508",
"f9 c #088908",
"gf c #088910",
"gj c #101008",
"g# c #108d10",
"gu c #181410",
"gU c #181c20",
"aR c #181c39",
"fx c #181c41",
"#M c #182041",
"eg c #182439",
"bw c #183452",
"aH c #183862",
"bv c #18407b",
"aF c #184094",
"eO c #202041",
"gX c #202420",
"cj c #202439",
"dV c #202441",
"#. c #202820",
"fL c #202829",
"#5 c #202841",
"bE c #20284a",
"e8 c #202c29",
".2 c #202c31",
"d6 c #202c41",
"aI c #203029",
"fo c #203031",
"b. c #203452",
"#2 c #20385a",
"aj c #203862",
"b7 c #203c6a",
"a9 c #203c73",
"a8 c #20407b",
"bQ c #204094",
"#1 c #204462",
"eG c #20447b",
"dr c #204483",
"aG c #20448b",
"bu c #2044a4",
"d5 c #204ca4",
"#Z c #2059ac",
".3 c #292418",
"g8 c #292829",
".p c #292c29",
"dL c #292c41",
"d7 c #293041",
"fn c #293052",
"b8 c #293441",
"#o c #29384a",
"#n c #293c4a",
"#H c #294c6a",
"#0 c #2950b4",
"fy c #29598b",
"ah c #2959ac",
"cN c #295db4",
"#Y c #295dbd",
"a7 c #2961bd",
"dt c #312c18",
"db c #312c20",
"dM c #312c4a",
"cP c #313018",
"b9 c #313020",
".o c #313031",
"es c #313052",
"cQ c #313429",
".m c #313431",
"#3 c #314052",
"#I c #31446a",
"ak c #314862",
"er c #3150b4",
"cu c #3155a4",
"ai c #31598b",
"dq c #3159b4",
"#l c #315db4",
"fd c #31619c",
"cO c #3161bd",
"eP c #31659c",
"e2 c #3165a4",
"ct c #3165bd",
"d4 c #3169b4",
"cC c #316d9c",
"bR c #393018",
"ds c #393420",
".n c #393439",
".1 c #393829",
".l c #393839",
"da c #393852",
"#b c #393c31",
".j c #393c39",
"## c #394439",
"fM c #394452",
"a6 c #3965b4",
"bf c #3965bd",
"bP c #3965c5",
"cs c #3969b4",
"fz c #396da4",
".4 c #396dac",
"bs c #396db4",
"bt c #396dc5",
"fA c #3971a4",
"cM c #3971c5",
"#F c #3975ac",
"fD c #3975c5",
".k c #413c41",
"#t c #414039",
".i c #414041",
".g c #414441",
"e9 c #41485a",
"eF c #41558b",
"#J c #416162",
"eE c #416594",
"a4 c #41697b",
"fe c #4171a4",
"fB c #4171b4",
"eh c #4175ac",
"#k c #4175b4",
"bO c #4175c5",
"fC c #4175cd",
"fE c #4175d5",
"ck c #4179b4",
"cq c #4179c5",
"bF c #4179cd",
"b0 c #4179d5",
"#j c #4179de",
"d# c #4179e6",
"#N c #417db4",
"b6 c #417dd5",
"cL c #417dee",
"e7 c #4a4429",
".h c #4a444a",
".f c #4a484a",
".d c #4a4c4a",
"aE c #4a6183",
"a5 c #4a69bd",
"dK c #4a75c5",
"cr c #4a7dd5",
"#E c #4a81b4",
"d3 c #4a85bd",
"e3 c #4a85c5",
"fK c #524431",
"et c #52485a",
"bx c #524c4a",
".e c #524c52",
".c c #525052",
".b c #525552",
"af c #525d83",
"aJ c #52617b",
".V c #52697b",
"eq c #526da4",
"ep c #52758b",
"b5 c #52759c",
"d2 c #52818b",
"#X c #5281c5",
"c1 c #5281d5",
"#D c #5281ee",
"ek c #5285c5",
".7 c #528994",
"cK c #5289d5",
"cp c #5289de",
"cJ c #528de6",
"cI c #5291d5",
"fJ c #5a4c31",
"cv c #5a4c52",
"#m c #5a5031",
".9 c #5a504a",
".S c #5a595a",
"#a c #5a5d5a",
"#p c #5a615a",
"ag c #5a69c5",
".Y c #5a6d52",
"ff c #5a85d5",
"#T c #5a899c",
"aS c #5a89c5",
"d. c #5a8dcd",
"#C c #5a91d5",
"c2 c #5a91de",
"#U c #5a95e6",
"bg c #5a99d5",
"ar c #5a9de6",
"fI c #625529",
".X c #625931",
".L c #625962",
".r c #625d62",
".W c #626141",
"#r c #626162",
"#K c #626562",
"co c #62799c",
"en c #627d9c",
"bN c #62816a",
"cH c #6281ac",
".5 c #62899c",
"dB c #6289bd",
"aA c #628dac",
"dW c #628dcd",
"eo c #6291e6",
"c9 c #6295f6",
"dC c #6299e6",
"dY c #629dde",
"ej c #62a1e6",
"dX c #62a1f6",
"eU c #6a5d20",
"fm c #6a5d39",
"fl c #6a694a",
"#4 c #6a696a",
"e6 c #6a6d52",
".8 c #6a816a",
"br c #6a8573",
"ad c #6a91cd",
"dH c #6a95ac",
"ac c #6a9de6",
"ei c #6aa1e6",
"dZ c #6aa1ee",
"dD c #6aa5f6",
"b1 c #6ab2d5",
"be c #736973",
"eD c #739173",
"dl c #73aade",
"dJ c #73aae6",
"c3 c #73aaee",
"dk c #73b2ee",
"e5 c #7b694a",
"fk c #7b6d52",
".Z c #7b6d62",
"aq c #7b6d73",
"#G c #7b715a",
"bD c #7b716a",
"fj c #7b754a",
"ap c #7b7973",
"c7 c #7b9dbd",
"bG c #7bbae6",
"c4 c #7bbaee",
"fH c #83694a",
"aQ c #83797b",
"fG c #837d4a",
".K c #838583",
"cl c #83b2ee",
"do c #83b6d5",
"aP c #8b857b",
".P c #8b858b",
"fh c #947d4a",
"ci c #94857b",
"bZ c #948583",
"fp c #948983",
"bd c #948d83",
".C c #948d94",
"fO c #949183",
".B c #949594",
"dn c #94999c",
"eT c #9c7152",
"hi c #9c856a",
"hh c #9c8573",
"hf c #9c8973",
"he c #9c897b",
"gY c #9c898b",
"hc c #9c8d7b",
"ha c #9c8d83",
"fN c #9c8d8b",
"fF c #9c915a",
"c0 c #9c9183",
"ao c #9c918b",
"eW c #9c958b",
"dN c #9c9994",
"c6 c #9c999c",
"eC c #9c9d4a",
".6 c #a4855a",
"hk c #a4856a",
"fi c #a48962",
"hg c #a48d7b",
"hd c #a49183",
"cB c #a4918b",
"eS c #a4955a",
"hb c #a4958b",
"eV c #a49594",
"d8 c #a4959c",
"cn c #a49941",
"dA c #a49983",
"fq c #a49994",
"aO c #a4999c",
"eH c #a49d94",
"an c #a49d9c",
"d9 c #a4a19c",
"dc c #a4a1a4",
"#V c #a4aa4a",
"aT c #a4b6a4",
"e4 c #ac8152",
"hj c #ac8d73",
"fU c #ac957b",
"h# c #ac958b",
"eB c #ac9962",
"eN c #ac997b",
"ef c #ac9983",
"dj c #ac998b",
"h. c #ac9994",
"g9 c #ac999c",
"fP c #ac9d9c",
"bC c #ac9da4",
"aD c #aca152",
"a3 c #aca16a",
"f. c #aca19c",
"aN c #aca5a4",
".A c #aca5ac",
"du c #acaaa4",
".z c #acaaac",
"c. c #acaeac",
"bK c #acb26a",
"aB c #acba62",
"bh c #acbeb4",
"#u c #b4914a",
"hm c #b4916a",
"fg c #b4954a",
"gT c #b4956a",
"f8 c #b49573",
"hl c #b4957b",
"gG c #b4996a",
"bq c #b49d62",
"eR c #b49d6a",
"gI c #b49d83",
"ez c #b49d8b",
"g7 c #b4a183",
"gB c #b4a18b",
"g5 c #b4a58b",
"g3 c #b4a594",
"bc c #b4a5a4",
"g0 c #b4aa9c",
"dO c #b4aaac",
"bo c #b4ae6a",
"fQ c #b4ae9c",
"bB c #b4aea4",
"bb c #b4aeac",
"cR c #b4aeb4",
"eQ c #b4b273",
"cw c #b4b2b4",
".J c #b4b6b4",
"ae c #b4ba62",
"#h c #b4be6a",
"cD c #b4c2ee",
"bH c #b4cabd",
"bM c #bd814a",
"ho c #bd996a",
"gt c #bd9973",
"gi c #bd9d73",
"fw c #bd9d8b",
"bp c #bda16a",
"e1 c #bda18b",
"dU c #bda194",
"gH c #bda594",
"g6 c #bda59c",
"g2 c #bdaea4",
"fr c #bdaeac",
"eX c #bdb2ac",
"am c #bdb2b4",
"eA c #bdb66a",
"gZ c #bdb6ac",
".s c #bdb6bd",
"cS c #bdbab4",
".w c #bdbabd",
"#W c #bdc26a",
"as c #bdd2bd",
"hn c #c5996a",
"hp c #c59973",
"gM c #c59d7b",
"gA c #c5a17b",
"ge c #c5a183",
"f1 c #c5a18b",
"a2 c #c5a573",
"gO c #c5a594",
"gP c #c5aa8b",
"gJ c #c5aa94",
"gV c #c5ae8b",
"gv c #c5ae9c",
"g4 c #c5aea4",
"g1 c #c5aeac",
"gp c #c5b294",
"eu c #c5b2ac",
"fX c #c5b6ac",
"eI c #c5b6b4",
".x c #c5b6c5",
"#d c #c5ba62",
"#A c #c5ba94",
"e. c #c5bab4",
"dd c #c5babd",
".u c #c5bac5",
".y c #c5bec5",
"dG c #c5c2a4",
".v c #c5c2c5",
"d1 c #c5c673",
"#i c #c5c67b",
"aM c #c5c6c5",
"dp c #cda562",
"bL c #cda56a",
"go c #cda583",
"gC c #cdb69c",
"fs c #cdbaac",
"f# c #cdbab4",
"em c #cdbe6a",
"eJ c #cdc2b4",
"bY c #cdc2bd",
"ba c #cdc2c5",
".t c #cdc2cd",
"cg c #cdc6bd",
"dv c #cdc6c5",
"c# c #cdc6cd",
"#S c #cdca8b",
"bS c #cdcacd",
"#P c #cdd283",
"#f c #d5ae73",
"gw c #d5b69c",
"gD c #d5be9c",
"ab c #d5c2a4",
"f5 c #d5c2b4",
"ch c #d5c2bd",
"bn c #d5c673",
"fa c #d5c6bd",
"bA c #d5c6c5",
"de c #d5c6cd",
"a1 c #d5ca83",
"bX c #d5cac5",
".R c #d5cad5",
"#R c #d5ce7b",
"b# c #d5cecd",
".O c #d5ced5",
"dI c #d5d27b",
"dw c #d5d2c5",
".I c #d5d2d5",
"b2 c #d5d6cd",
"#e c #dea14a",
"aC c #deae6a",
"c8 c #deae73",
"#g c #deb67b",
"gR c #debe9c",
"gQ c #debea4",
"#O c #dec26a",
"gq c #dec2ac",
"cG c #dec694",
"gr c #dec6a4",
"gg c #dec6b4",
"f6 c #decab4",
"ga c #decabd",
"ev c #decac5",
"fR c #decebd",
"cZ c #decec5",
"#c c #ded262",
"el c #ded283",
"a# c #ded294",
"cY c #ded2c5",
"cT c #ded2d5",
".N c #ded2de",
"dz c #ded6c5",
"aK c #ded6de",
"bJ c #deda94",
"cW c #dedacd",
"ce c #dedad5",
"#s c #dedede",
"#y c #e6ba7b",
"#B c #e6ba83",
"gW c #e6be9c",
"ay c #e6c283",
"dF c #e6c673",
"gS c #e6c69c",
"gF c #e6c6a4",
"gx c #e6c6ac",
"az c #e6ca83",
"gs c #e6caa4",
"gk c #e6caac",
"ax c #e6ce94",
"gh c #e6ceac",
"gb c #e6ceb4",
"a0 c #e6d28b",
"f7 c #e6d2b4",
"dT c #e6d2bd",
"ft c #e6d2c5",
"cf c #e6d2cd",
"dP c #e6d2d5",
"ed c #e6d6bd",
"ee c #e6d6c5",
"cx c #e6d6d5",
"bW c #e6d6de",
"d0 c #e6da9c",
"aL c #e6dade",
"al c #e6dae6",
"bm c #e6deb4",
"dx c #e6decd",
"bz c #e6dede",
"#L c #e6dee6",
"dE c #e6e2cd",
".H c #e6e2e6",
".Q c #e6e6e6",
"gL c #eec69c",
"gK c #eec6a4",
"#z c #eeca7b",
"gy c #eecaa4",
"gE c #eecaac",
"aa c #eece94",
"gm c #eeceac",
"e0 c #eed2b4",
"ey c #eed2c5",
"fc c #eed6bd",
"fu c #eed6c5",
"di c #eed6cd",
"a. c #eeda9c",
"fS c #eedabd",
"eM c #eedac5",
"dS c #eedacd",
"cX c #eedad5",
"dQ c #eedade",
"fb c #eedec5",
"eb c #eedecd",
"e# c #eeded5",
"df c #eedede",
"bV c #eedee6",
"eK c #eee2cd",
"dy c #eee2d5",
"cc c #eee2de",
"cd c #eee2e6",
".M c #eee2ee",
"#Q c #eee683",
"bl c #eee6c5",
"cU c #eee6de",
"aU c #eee6ee",
"ca c #eeeae6",
"aw c #eeeec5",
"#q c #eeeeee",
"#w c #f6b65a",
"#v c #f6ba62",
"cm c #f6ca83",
"#x c #f6ce83",
"gd c #f6ceac",
"gz c #f6ceb4",
"b4 c #f6d28b",
"#6 c #f6d294",
"gc c #f6d2b4",
"gl c #f6d2bd",
"#9 c #f6d6a4",
"f0 c #f6d6ac",
"gn c #f6d6b4",
"fT c #f6d6bd",
"fZ c #f6d6c5",
"fv c #f6dac5",
"eZ c #f6dacd",
"cE c #f6deb4",
"ew c #f6dec5",
"ex c #f6decd",
"ec c #f6ded5",
"eL c #f6e2cd",
"bi c #f6e2d5",
"dh c #f6e2de",
"cA c #f6e2e6",
"au c #f6e6cd",
"dR c #f6e6d5",
"cV c #f6e6e6",
"bU c #f6e6ee",
"dm c #f6eac5",
"cy c #f6eaee",
".U c #f6eaf6",
"by c #f6eef6",
".F c #f6f2f6",
".0 c #f6f6f6",
"cF c #ffd69c",
"#7 c #ffdab4",
"#8 c #ffdeb4",
"fY c #ffdecd",
"aW c #ffded5",
"aV c #ffe2d5",
"c5 c #ffe2de",
"at c #ffe6c5",
"aY c #ffe6cd",
"eY c #ffe6d5",
"bj c #ffe6de",
"aZ c #ffeaa4",
"ea c #ffeade",
"aX c #ffeae6",
"dg c #ffeaee",
"av c #ffeede",
"bk c #ffeee6",
"cz c #ffeeee",
"bT c #ffeef6",
"cb c #fff2f6",
".G c #fff2ff",
"b3 c #fff6ee",
"bI c #fff6f6",
".T c #fff6ff",
".D c #fffaff",
".E c #ffffff",
"Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#",
".aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt",
"Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#",
".aQt.aQt.aQt.aQt.aQt.aQt.aQt.b.b.c.c.d.e.f.f.g.h.i.i.j.k.l.l.m.n.o.o.p.q.aQt.aQt.aQt.aQt.aQt.aQt",
"Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#.r.s.t.u.v.w.t.u.v.w.t.u.v.w.t.x.y.z.A.B.C.h.q.#Qt.#Qt.#Qt.#Qt.#Qt.#",
".aQt.aQt.aQt.aQt.aQt.aQt.aQt.b.y.D.E.D.E.D.E.D.E.D.E.D.E.D.E.F.G.H.I.J.d.K.q.aQt.aQt.aQt.aQt.aQt",
"Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#.L.u.E.D.E.D.E.D.E.D.E.D.E.D.E.D.E.G.G.M.N.e.O.P.q.#Qt.#Qt.#Qt.#Qt.#",
".aQt.aQt.aQt.aQt.aQt.aQt.aQt.c.y.D.E.D.E.D.E.D.E.D.E.D.E.D.E.D.E.F.G.H.c.Q.R.K.q.aQt.aQt.aQt.aQt",
"Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#.S.w.E.D.E.D.E.D.E.D.E.D.E.D.E.D.E.D.E.T.G.b.E.U.O.P.q.#Qt.#Qt.#Qt.#",
".aQt.aQt.aQt.aQt.aQt.aQt.aQt.c.y.j.V.W.X.Y.Z.F.D.0.E.0.E.D.E.D.E.D.E.F.S.D.E.Q.R.K.q.aQt.aQt.aQt",
"Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.1.2.3.4.5.6.7.4.8.9#.##.G.D.T.E.D.E.D.E.D.E#a.G.D.E.U.O.P.q.#Qt.#Qt.#",
".aQt.aQt.aQt.aQt.aQt.a#b.1#c#d#e#f#g#h#i#j#k#l#m#n#o#p.F#q.D.0.E.D.E.D#r#s.G.D.E.Q.R.K.q.aQt.aQt",
"Qt.#Qt.#Qt.#Qt.#Qt.##t#u#v#w#x#y#z#A#B#C#D#E#F#l#G#H#I#J.U.U.T.T.E.D.E#K.R#L.G.D.E.U.O.P.q.#Qt.#",
".aQt.aQt.aQt.aQt.a#M#N#g#O#z#P#Q#R#S#T#U#V#W#X#Y#Z#0#1#2#3#L.H.G.F.E.D#4.c.e.f.h.k.k.m.m.p.q.aQt",
"Qt.#Qt.#Qt.#Qt.##5#E#B#6#7#8#9a.a#aaabacadae#iafagahaiajajakal.M.G.T.E.U.R.wamanaoapaq.S.i.qQt.#",
".aQt.aQt.aQt.a#M#NarasatauavauawaxayazaAaBaCaBaDaE#YaFaGaHaIaJaK.H.G.0.D.HaLaM.yaNaOaPaQ.c.q.aQt",
"Qt.#Qt.#Qt.#QtaRaSaTaUaVavaWaXaYaZa0#xa1a2#Va3a4a5a6a7a8a9b..l.val.M.T.T.E.Malb#babbbcbdbe.qQt.#",
".aQt.aQt.aQtaRbfbgbhbibjbk.Ebiblazbmazbnbobpbqbrbsbt#Zbubv#2bwbxaM#L.Q.T.FbybzaLb#bAbBbCbD.q.aQt",
"Qt.#Qt.#Qt.#bEbFbGbHbIbI.EbI.E#8bmaabJbKbLbMbNbsbOa6bPbQaGajajbR.ybSaU.U.TbTbUbVbWbXbYbBbZ.qQt.#",
".aQt.aQt.aQtaRb0b1b2bkbIb3.Ebiataxb4azaCbqb5#jb6bsbt#l#0bvb7b8b9c.c#.IaUcacbcccdcecfcgchci.q.aQt",
"Qt.#Qt.#Qtcjckbgcl#AbIaV.EbIbIcm#z#6#6cncocp#DcqcrcsctcuaGa8b9.mcvcw.RcxcycyczcAcccxcfcgcB.qQt.#",
".aQt.aQt.a#5cC#UarcDbkbIbkaXcEcFazcG#fcHcIcJcKcLcqcMcNcOaFaGcPb9cQcRcScTbzczcUcVcWcXcYcZc0.q.aQt",
"Qt.#Qt.#QtaRc1c2c3c4aUc5av#8#6c6c7c6c8c2c9cKd.d#c1bsbP#lbua8dabRdbdcdddedfcAdgcVdhdicXcfdj.qQt.#",
".aQt.aQt.a#5#N#UcIdkdldmcEb4dndob1dkdp#UcJ#UcJcLcqbt#ldqbvdrbwdsdtbCdudvdwcVcUaXdxdydzdidA.q.aQt",
"Qt.#Qt.#QtaRb6dBdCacdDbgdEdFdGdHdIdJc3#Cc9cp#DcqdK#la7bQaGb.ajdLdMdNdOcSdPdQaXdRaXcXdSdTdU.qQt.#",
".aQt.aQt.adVbsdWcKdXdYdZcId0#Pd1d2cJcJcJd3cLcqb6d4cOd5buaH#2d6d7b8d8d9e.cge#e#eaebecedeeef.q.aQt",
"Qt.#Qt.#QtegehcpdC#Ceiejc3bgekelemeneoepeqepcrcscMaherbQajb.es#.etaobceuevdidhc5bjewexeyez.qQt.#",
".aQt.aQt.aQtaRb6#EcJcIardYdXdYar#EeAeneBeCeDeEeFcN#0eGb7bwd7d6#5bdd8eHeIeJcXebbjeKeLdTeMeN.q.aQt",
"Qt.#Qt.#Qt.#eOePcrc2dC#Ceiejc3aceicpeQ#ueReSeTeUbueG#Ib.esdL#5cjeVeWbceXevdibiexeYeZexe0e1.qQt.#",
".aQt.aQt.aQt.qe2cCe3cKcJcIarbgar#Ne4eCeBeC.6e5e6e7b7bw#2d6e8ege9bdd8f.f#fadSeMbifbeLfcfceN.q.aQt",
"Qt.#Qt.#Qt.#Qt.qfdfeek#Eff#Xeocpc1fga3fhfifjfkflfmb8ajdLfn.3fofpeVfqfrfsftfubiexaVfvewe0fw.qQt.#",
".aQt.aQt.aQt.afxfyfzfAfBcqfCfDfEd4.6cnfFfGfHfIfJfK#bd6fLegcjfMfNfOfPfQchfRexeMeLfSewfcfTfU.q.aQt",
"Qt.#Qt.#Qt.qfV.qfV.qfV.qfV.qfV.qfV.qfV.qfV.qfV.qfV.qfV.qfV.qfV.qfW.qfXeJdidieLfvfYfZfvf0f1.qQt.#",
".aQt.aQt.af2f3f4f3f4f3f4f3f4f3f4f3f4f3f4f3f4f3f4f3f4f3f4f3f4f3f4f3f4.qf5f6dSfcewfcfvf7fTf8.q.aQt",
"Qt.#Qt.#Qt.qf9f3f9.E.E.E.Ef3.E.E.E.E.Ef3.E.E.E.Ef9g.f9f3g#f3f9g.f9g.f9.qgagbfvfZfvgcfTgdge.qQt.#",
".aQt.aQt.af2f3gfg..Eg.f4g.gfg.gf.Egfg.gf.Egfg.gf.Ef4g.gfg.gfg.gfg.gff3f4.qggf6fTf7fTghgcgi.q.aQt",
"Qt.#Qt.#Qtgjf9f3f9.Eg#f3g#f3g#g..Eg.f9g..Eg.f9g..Eg.g#f3g#f3g#f3g#g.g#f3f9.qgggkglgmgngmgo.qQt.#",
".aQt.aQt.af2f3gff3.E.E.Eg.gff3gf.Egff3gf.E.E.E.Eg.gfg.gfg.gfg.gfg.f4g.gff3.qgpgqgre0gsgmgt.q.aQt",
"Qt.#Qt.#Qtguf9g.g#.Eg#f3g#g.g#f3.Eg.g#g..Ef3g#f3g#f3g#g.g#f3g#f3g#g.f9f3fV.qgvgwgxgygzgygA.qQt.#",
".aQt.aQt.af2f3gff3.Eg.gfg.gfg.f4.Egfg.f4.Egfg.f4g.gfg.f4g.gfg.gfg.f4f3f2.qe1gBgCgDgEgFgEgG.q.aQt",
"Qt.#Qt.#Qtgjf9f3f9f3f9f3f9f3f9f3f9f3f9f3f9f3f9f3f9f3f9f3f9f3f9f3f9f3fV.qgHgIgJgpgxgKgEgLgM.qQt.#",
".aQt.aQt.af2.qf2.qfW.qf2.qf2.qf2.qf2.qf2.qf2.qf2.qf2.qf2.qfWgNf2.qf2.qfwgIgOgPgQgRgygSgEgT.q.aQt",
"Qt.#Qt.#Qt.#gU.qfV.qfV.qfV.qfV.qfV.qfV.qfW.qfV.qfV.qfV.qfV.qfV.qfV.qgHgIgJgVgQgWgygLgdgLgM.qQt.#",
".aQt.aQt.aQt.aQt.aQt.aQt.aQtgXgYgZddeXeIbBeug0g1g0g2g3g4g3gvg5g6gBe1g7gOgPgRgDgKgSgEgSgEgT.q.aQt",
"Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#g8aog9eWh.c0h#hahbhchdhehdhfhghhhghihjhkhjfihlhmgMgGgMhngMhohp.qQt.#",
".aQt.aQt.aQt.aQt.aQt.aQt.aQt.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.q.aQt",
"Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#Qt.#",
".aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt.aQt"};


/* 
 *  Constructs a newFTPGUI which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
newFTPGUI::newFTPGUI( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl ){

    QPixmap image0( ( const char** ) image0_data );
    if ( !name )
    setName( "newFTPGUI" );
    resize( 342, 261 );
    setCaption( i18n( "New FTP Connection"  ) );
    setSizeGripEnabled( TRUE );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, sizePolicy().hasHeightForWidth() ) );
    setMinimumSize( QSize( 342, 261 ) );

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setGeometry( QRect( 10, 50, 60, 20 ) );
    TextLabel1->setText( i18n( "Host:"  ) );

    QStringList protocols = KProtocolInfo::protocols();

    prefix = new KComboBox( FALSE, this, "protocol" );
    if( protocols.contains("ftp") )
      prefix->insertItem( i18n( "ftp://" ) );
    if( protocols.contains("smb") )
      prefix->insertItem( i18n( "smb://" ) );
    if( protocols.contains("fish") )
      prefix->insertItem( i18n( "fish://" ));
    if( protocols.contains("sftp") )
      prefix->insertItem( i18n( "sftp://" ));
    prefix->setGeometry( QRect( 10, 70, 70, 22 ) );
    prefix->setAcceptDrops( FALSE );
    prefix->setEnabled( TRUE );
    connect( prefix,SIGNAL(activated(const QString& )),
               this,SLOT(slotTextChanged(const QString& )));

    TextLabel1_2_2 = new QLabel( this, "TextLabel1_2_2" );
    TextLabel1_2_2->setGeometry( QRect( 10, 150, 150, 20 ) );
    TextLabel1_2_2->setText( i18n( "Password:"  ) );

    TextLabel1_2 = new QLabel( this, "TextLabel1_2" );
    TextLabel1_2->setGeometry( QRect( 10, 100, 150, 20 ) );
    TextLabel1_2->setText( i18n( "Username:"  ) );

    TextLabel1_3 = new QLabel( this, "TextLabel1_3" );
    TextLabel1_3->setGeometry( QRect( 230, 50, 80, 20 ) );
    TextLabel1_3->setText( i18n( "Port:"  ) );

    port = new QSpinBox( this, "port" );
    port->setGeometry( QRect( 280, 70, 50, 21 ) );
    port->setMaxValue( 999 );
#if QT_VERSION < 300
    port->setFrameShadow( QSpinBox::Sunken );
#endif
    port->setValue( 21 );

    password = new QLineEdit( this, "password" );
    password->setGeometry( QRect( 10, 170, 320, 22 ) );
    password->setEchoMode( QLineEdit::Password );

    QWidget* Layout6 = new QWidget( this, "Layout6" );
    Layout6->setGeometry( QRect( 10, 210, 320, 34 ) );
    hbox = new QHBoxLayout( Layout6 );
    hbox->setSpacing( 6 );
    hbox->setMargin( 0 );

    connectBtn = new QPushButton( Layout6, "connectBtn" );
    connectBtn->setText( i18n( "&Connect"  ) );
    connectBtn->setAutoDefault( TRUE );
    connectBtn->setDefault( TRUE );
    hbox->addWidget( connectBtn );

    saveBtn = new QPushButton( Layout6, "saveBtn" );
    saveBtn->setText( i18n( "&Save"  ) );
    saveBtn->setAutoDefault( TRUE );
    hbox->addWidget( saveBtn );

    cancelBtn = new QPushButton( Layout6, "cancelBtn" );
    cancelBtn->setText( i18n( "&Cancel"  ) );
    cancelBtn->setAutoDefault( TRUE );
    hbox->addWidget( cancelBtn );

    PixmapLabel1 = new QLabel( this, "PixmapLabel1" );
    PixmapLabel1->setGeometry( QRect( 10, 10, 40, 40 ) );
    PixmapLabel1->setPixmap( image0 );
    PixmapLabel1->setScaledContents( TRUE );

    TextLabel3 = new QLabel( this, "TextLabel3" );
    TextLabel3->setGeometry( QRect( 60, 10, 150, 31 ) );
    TextLabel3->setText( i18n( "About to connect to..."  ) );
    QFont TextLabel3_font(  TextLabel3->font() );
    TextLabel3_font.setBold( TRUE );
    TextLabel3->setFont( TextLabel3_font ); 

    username = new QLineEdit( this, "username" );
    username->setGeometry( QRect( 10, 120, 320, 22 ) );

    url = new KHistoryCombo( this, "url" );
    url->setGeometry( QRect( 80, 70, 200, 22 ) );
    connect( url, SIGNAL( activated( const QString& )),
             url, SLOT( addToHistory( const QString& )));
    // load the history and completion list after creating the history combo
    QStringList list = krConfig->readListEntry( "newFTP Completion list" );
    url->completionObject()->setItems( list );
    list = krConfig->readListEntry( "newFTP History list" );
    url->setHistoryItems( list );

    // signals and slots connections
    connect( connectBtn, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( cancelBtn, SIGNAL( clicked() ), this, SLOT( reject() ) );

    // tab order
    setTabOrder( url, username );
    setTabOrder( username, password );
    setTabOrder( password, connectBtn );
    setTabOrder( connectBtn, cancelBtn );
    setTabOrder( cancelBtn, prefix );
    setTabOrder( prefix, port );
    setTabOrder( port, saveBtn );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
newFTPGUI::~newFTPGUI(){
    // no need to delete child widgets, Qt does it all for us
}

void newFTPGUI::slotTextChanged(const QString& string){
   if( string.startsWith("ftp") )
     port->setEnabled(true);
   else
     port->setEnabled(false);
}

/*
 *  Main event handler. Reimplemented to handle application
 *  font changes
 */
bool newFTPGUI::event( QEvent* ev ) {
    bool ret = QDialog::event( ev ); 
    if ( ev->type() == QEvent::ApplicationFontChange ) {
	QFont TextLabel3_font(  TextLabel3->font() );
	TextLabel3_font.setBold( TRUE );
	TextLabel3->setFont( TextLabel3_font ); 
    }
    return ret;
}

#include "newftpgui.moc"
