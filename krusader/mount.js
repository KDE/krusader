/*
   This is an extension for the Krusader filemanager
   http://krusader.sf.net/
   
   Author: Jonas Bähr, http://www.jonas-baehr.de/
   
   This script shows a dialog for mounting a device. The commandline which is
   returned by this is executed by Krusader's UserAction-system
   Krusader looks for the variable 'cmd'.

   scriptDir is a string-variable set by Krusader with the directory of the currently
   executed script. This is usefull for relative path's inside scripts.
*/

var ui = Factory.loadui( scriptDir + 'mount.ui' );

ui.child('device').setFocus();

var m = ui.exec();

var cmd = "@CANCEL@"; //FIXME: replace this with UserAction.cancel

if (m) {
   var cmd = "mount";
   var device = ui.child('device').url;
   var mountpoint = ui.child('mountpoint').url;
   var filesystem = ui.child('filesystem').currentText;
   var options = ui.child('options').text;

   if ( filesystem != '<auto>' && filesystem != '' )
      cmd += " -t " + filesystem;
   if ( options != '' )
      cmd += " " + options;

   cmd +=  " " + device + " " + mountpoint;
}

