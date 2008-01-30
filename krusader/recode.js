/*
   This is an extension for the Krusader filemanager
   http://krusader.sf.net/

   Author: Vaclav Juza.
      based on mount by Jonas B�r, http://www.jonas-baehr.de/

   This script shows a dialog for recoding file names. The commandline which is
   returned by this is executed by Krusader's UserAction-system
   Krusader looks for the variable 'cmd'.

   scriptDir is a string-variable set by Krusader with the directory of the currently
   executed script. This is usefull for relative path's inside scripts.
*/

var cmd = "@CANCEL@"; //FIXME: replace this with UserAction.cancel
//if an exception occurs, do not continue

var charsetsRecode = ["UTF-8","UTF-7","ISO-8859-2","WINDOWS-1250","IBM852","Kamenicky","MacCentralEurope","ISO-8859-1"];
var charsetsIconv = ["UTF-8","UTF-7","ISO_6937","ISO-8859-2","WINDOWS-1250","IBM852","ISO-8859-1"];
var endLineTexts = ["UNIX","DOS","MAC"];
var endLineVals = ["","/CR-LF","/CR"];
var mimeTexts = ["NORMAL","BASE64","QUOTED-PRINTABLE"];
var mimeVals = ["","/Base64","/Quoted-Printable"];
var dumps = ["NORMAL","Hexadecimal-1","Hexadecimal-2","Hexadecimal-4","Decimal-1","Decimal-2","Decimal-4",
  "Octal-1","Octal-2","Octal-4"];
var methodTexts = ["Copy","Rename/move"];

if(dstDir.indexOf(srcDir) == 0 || srcDir.indexOf(dstDir) == 0){ //would do weird things
  throw new Error("Destination directory inside the source directory is not supported");
}

var ui = Factory.loadui( scriptDir + 'recode.ui' );

var bc = [ ui.child('boxInCharset'), ui.child('boxOutCharset') ];
var bl = [ ui.child('boxInLineend'), ui.child('boxOutLineend') ];
var bm = [ ui.child('boxInMime'), ui.child('boxOutMime') ];
var bd = [ ui.child('boxInDump'), ui.child('boxOutDump') ];
var bMtd = ui.child('boxMethod');
var cbName = ui.child('checkName');
var cbContent = ui.child('checkContent');
var cbDebug = ui.child('checkDebug');
var loc = [ ui.child('locInput'), ui.child('locOutput') ];

function fillCombo(kcombo, texts) {
  var t = kcombo.child('in-combo');
  for(i in texts) {
    t.insertItem(texts[i]);
  }
  kcombo.currentItem = 0;
}

if(useRecode==1){
  for(var d=0;d<=1;d++){
    fillCombo(bc[d],charsetsRecode);
    fillCombo(bl[d],endLineTexts);
    fillCombo(bm[d],mimeTexts);
    fillCombo(bd[d],dumps);
    bc[0].currentItem = 2;
  }
}else{
  for(var d=0;d<=1;d++){
    fillCombo(bc[d],charsetsIconv);
    bc[0].currentItem = 3;
    bl[0].shown = 0;
    bl[1].shown = 0;
    bm[0].shown = 0;
    bm[1].shown = 0;
    bd[0].shown = 0;
    bd[1].shown = 0;
  }
}
fillCombo(bMtd,methodTexts);

//defaults
loc[0].url = srcDir;
loc[1].url = dstDir;
cbName.checked=1;
cbContent.checked=0;
cbDebug.checked=0;

function quoteString(str) {
  return "'" + str.replace(/'/g,"'\\''") + "'";
}

function indentString(str) {
  return str.replace(/(^|\n)([^$])/g,"$1  $2");
}

function debugString(str) {
  return str.replace(/(<|>|\\|;)/g,"\\$1").replace(/"/g,"\\\"\"");
}

bc[0].setFocus();
var m = ui.exec();

if(m) {
   var isMove = ( bMtd.currentItem == 1 );
   var rcName = cbName.checked;
   var rcContent = cbContent.checked;
   var debug = cbDebug.checked;

   srcDir = loc[0].url;
   dstDir = loc[1].url;
   if(dstDir.indexOf(srcDir) == 0 || srcDir.indexOf(dstDir) == 0){ //would do weird things
     throw new Error("Destination directory inside the source directory is not supported");
   }

   var listCmd =
        "for i in " + toProcess + "\n"
       +"do\n"
       +" find \"${i}\" -name '*'\n"
       +"done";

   //the following contains user input, must be quoted
   var recodeCmd, recodeSmallCmd;
   if(useRecode==1){
     var expr = new Array(2);
     var smallExpr = new Array(2);
     for(var d=0; d<=1; d++){
       var chs = "";
       chs = bc[d].currentText;
       var le = endLineVals[ bl[d].currentItem ];
       var me = mimeVals[ bm[d].currentItem ];
       var du="";
       if( bd[d].currentItem > 0 ) {
         du = "/" + bd[d].currentText;
       }
       expr[d] = chs + le + me + du;
       smallExpr[d] = chs + me;
     }
     recodeCmd="recode " + quoteString(expr[0] + ".." + expr[1]);
     recodeSmallCmd="recode " + quoteString(smallExpr[0] + ".." + smallExpr[1]);
   }else{
     recodeCmd="iconv -f " + quoteString(bc[0].currentText) + " -t " + quoteString(bc[1].currentText);
     recodeSmallCmd=recodeCmd;
   }

   var setupVarsCmd;
   if(rcName){
     setupVarsCmd =
          "fnew=\"$(\n"
         +"  echo \"${forig}\"  | " + recodeSmallCmd +"\n"
         +")\"";
   }else{
     setupVarsCmd = "fnew=\"${forig}\"";
   }
   var dstStr = dstDir + "/\"${fnew}\"";
   var oneDirCmd = "mkdir " + dstStr; //create the directory in new location
   var oneFileCmd;
   var oneSpcCmd;
   if(isMove){
     oneSpcCmd = "mv " + "\"${forig}\" " + dstStr;
   }else{
     oneSpcCmd = "cp -dp " + "\"${forig}\" " + dstStr;
   }
   if(rcContent){
     oneFileCmd = recodeCmd + " \<\"${forig}\" \>"+dstStr;
     if(isMove){
       oneFileCmd += "&& rm -f \"${forig}\"";
     }
   }else{
     oneFileCmd = oneSpcCmd;
   }
   oneDirCmdDbg="echo directory:" + debugString(oneDirCmd);
   oneFileCmdDbg="echo file:" + debugString(oneFileCmd);
   oneSpcCmdDbg="echo special:" + debugString(oneSpcCmd);
   var oneItemCmd =
        setupVarsCmd + "\n"
       +"if [ ! -L \"${forig}\" ] && [ -d \"${forig}\" ]\n"
       +"then\n"
         +indentString(oneDirCmdDbg) + "\n"
         + ( debug ? "" : indentString(oneDirCmd) + "\n" )
       +"elif [ -f \"${forig}\" ]\n"
       +"then\n"
         +indentString(oneFileCmdDbg) + "\n"
         + ( debug ? "" : indentString(oneFileCmd) + "\n" )
       +"else\n"
         +indentString(oneSpcCmdDbg) + "\n"
         + ( debug ? "" : indentString(oneSpcCmd) + "\n" )
       +"fi";
   var processCmd =
        "while read forig;\n"
       +"do\n"
         +indentString(oneItemCmd) + "\n"
       +"done";
   if(isMove){
     var deleteOldCmd =
        "for i in " + toProcess + "\n"
       +"do\n"
       +" [ / = \"$i\" ] || rm -rf \"${i}\"\n"
       +"done";
      var realCmd = "(\n" + indentString(listCmd) + "\n) | (\n" + indentString(processCmd) + "\n)\n"+ deleteOldCmd;
   }else{
      var realCmd = "(\n" + indentString(listCmd) + "\n) | (\n" + indentString(processCmd) + "\n)";
   }
   cmd = "sh -c " + quoteString("LANG=C; " + realCmd);
   /*if(debug){
     cmd = "echo " + quoteString(cmd);
   }*/
}

