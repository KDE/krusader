#include "ktar.h"
#include <stdio.h>
#include <QtCore/QFile>
#include <kcomponentdata.h>
#include <kdebug.h>

void recursive_print( const KTarDirectory * dir, const QString & path )
{
  QStringList l = dir->entries();
  QStringList::Iterator it = l.begin();
  for( ; it != l.end(); ++it )
  {
    const KTarEntry* entry = dir->entry( (*it) );
    printf("mode=%07o %s %s %s%s\n", entry->permissions(), entry->user().toLatin1(), entry->group().toLatin1(), path.toLatin1(), (*it).toLatin1());
    if (entry->isDirectory())
      recursive_print( (KTarDirectory *)entry, path+(*it)+"/" );
  }
}

static void usage()
{
    printf("\n"
 " Usage :\n"
 " ./ktartest list /path/to/existing_file.tar.gz       tests listing an existing tar.gz\n"
 " ./ktartest get /path/to/existing_file.tar.gz filename   tests extracting a file.\n"
 " ./ktartest readwrite newfile.tar.gz                 will create the tar.gz, then close and reopen it.\n"
 " ./ktartest maxlength newfile.tar.gz                 tests the maximum filename length allowed.\n"
 " ./ktartest bytearray /path/to/existing_file.tar.gz  tests KTarData\n");
}

int main( int argc, char** argv )
{
  if (argc < 3)
  {
    usage();
    return 1;
  }
  KInstance instance("ktartest");

  QString command = argv[1];
  kDebug() << "main: command=" << command << endl;
  if ( command == "list" )
  {
    KTarGz tar( argv[2] );

    if ( !tar.open( QIODevice::ReadOnly ) )
    {
      printf("Could not open %s for reading\n", argv[1] );
      return 1;
    }

    const KTarDirectory* dir = tar.directory();

    //printf("calling recursive_print\n");
    recursive_print( dir, "" );
    //printf("recursive_print called\n");

    tar.close();

    return 0;
  }
  else if ( command == "get" )
  {
    if ( argc != 4 )
    {
        usage();
        return 1;
    }

    KTarGz tar( argv[2] );

    if ( !tar.open( QIODevice::ReadOnly ) )
    {
      printf("Could not open %s for reading\n", argv[1] );
      return 1;
    }

    const KTarDirectory* dir = tar.directory();

    const KTarEntry* e = dir->entry( argv[3] );
    Q_ASSERT( e && e->isFile() );
    const KTarFile* f = (KTarFile*)e;

    QByteArray arr( f->data() );
    printf("SIZE=%i\n",arr.size() );
    QString str( arr );
    printf("DATA=%s\n", str.toLatin1());

    /*
    // This is what KGzipDev::readAll could do, if QIODevice::readAll was virtual....
    QByteArray array(1024);
    int n;
    while ( ( n = dev.read( array.data(), array.size() ) ) )
    {
        kDebug() << "read returned " << n << endl << endl;
        QCString s(array,n+1); // Terminate with 0 before printing
        printf("%s", s.data());
    }
    dev.close();
    */


    tar.close();
  }
  else if (command == "readwrite" )
  {
    kDebug() << " --- readwrite --- " << endl;
    KTarGz tar( argv[2] );

    if ( !tar.open( QIODevice::WriteOnly ) )
    {
      printf("Could not open %s for writing\n", argv[1]);
      return 1;
    }

    tar.writeFile( "empty", "weis", "users", 0, "" );
    tar.writeFile( "test1", "weis", "users", 5, "Hallo" );
    tar.writeFile( "test2", "weis", "users", 8, "Hallo Du" );
    tar.writeFile( "mydir/test3", "weis", "users", 13, "Noch so einer" );
    tar.writeFile( "my/dir/test3", "dfaure", "hackers", 29, "I don't speak German (David)" );

#define SIZE1 100
    // Now a medium file : 100 null bytes
    char medium[ SIZE1 ];
    memset( medium, 0, SIZE1 );
    tar.writeFile( "mediumfile", "user", "group", SIZE1, medium );
    // Another one, with an absolute path
    tar.writeFile( "/dir/subdir/mediumfile2", "user", "group", SIZE1, medium );

    // Now a huge file : 20000 null bytes
    int n = 20000;
    char * huge = new char[ n ];
    memset( huge, 0, n );
    tar.writeFile( "hugefile", "user", "group", n, huge );
    delete [] huge;

    tar.close();

    printf("-----------------------\n");

    if ( !tar.open( QIODevice::ReadOnly ) )
    {
      printf("Could not open %s for reading\n", argv[1] );
      return 1;
    }

    const KTarDirectory* dir = tar.directory();
    recursive_print(dir, "");

    const KTarEntry* e = dir->entry( "mydir/test3" );
    Q_ASSERT( e && e->isFile() );
    const KTarFile* f = (KTarFile*)e;

    QByteArray arr( f->data() );
    printf("SIZE=%i\n",arr.size() );
    QString str( arr );
    printf("DATA=%s\n", str.toLatin1());

    tar.close();

    return 0;
  }
  else if ( command == "maxlength" )
  {
    KTarGz tar( argv[2] );

    if ( !tar.open( QIODevice::WriteOnly ) )
    {
      printf("Could not open %s for writing\n", argv[1]);
      return 1;
    }
    // Generate long filenames of each possible length bigger than 98...
    for (int i = 98; i < 500 ; i++ )
    {
      QString str, num;
      str.fill( 'a', i-10 );
      num.setNum( i );
      num = num.rightJustified( 10, '0' );
      tar.writeFile( str+num, "testu", "testg", 3, "hum" );
    }
    // Result of this test : it fails at 482 (instead of 154 previously).
    // Ok, I think we can do with that :)
    tar.close();
    printf("Now run 'tar tvzf %s'\n", argv[2]);
    return 0;
  }
  else if ( command == "bytearray" )
  {
    QFile file( argv[2] );
    if ( !file.open( QIODevice::ReadOnly ) )
      return 1;
    KTarGz tar( &file );
    tar.open( QIODevice::ReadOnly );
    const KTarDirectory* dir = tar.directory();
    recursive_print( dir, "" );
    return 0;
  }
  else
    printf("Unknown command\n");
}

