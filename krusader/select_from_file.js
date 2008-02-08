/*
   This is an extension for the Krusader filemanager
   http://krusader.sourceforge.net/
   
   Author: Dirk Eschler <deschler@users.sourceforge.net>
   
   Reads a file that contains a list of files separated by newline and builds
   a list separated by whitespace from it.
   
   The script is supposed to be used in conjunction with the select_from_file
   user action: http://www.kde-files.org/content/show.php?content=54518   
*/
var filelist = shell("cat " + file).replace(/\n/g, ' ') || "";
