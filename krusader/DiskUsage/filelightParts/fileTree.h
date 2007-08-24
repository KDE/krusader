//Author:    Max Howell <max.howell@methylblue.com>, (C) 2004
//Copyright: See COPYING file that comes with this distribution

#ifndef FILETREE_H
#define FILETREE_H

#include <stdlib.h>
#include <sys/types.h>
#include <kio/global.h>

//TODO these are pointlessly general purpose now, make them incredibly specific



typedef KIO::filesize_t FileSize;

template <class T> class Iterator;
template <class T> class ConstIterator;
template <class T> class Chain;

template <class T>
class Link
{
public:
    Link( T* const t ) : prev( this ), next( this ), data( t ) {}
    Link() : prev( this ), next( this ), data( 0 ) {}

  //TODO unlinking is slow and you don't use it very much in this context.
  //  ** Perhaps you can make a faster deletion system that doesn't bother tidying up first
  //  ** and then you MUST call some kind of detach() function when you remove elements otherwise
    ~Link() { delete data; unlink(); }

    friend class Iterator<T>;
    friend class ConstIterator<T>;
    friend class Chain<T>;

private:
    void unlink() { prev->next = next; next->prev = prev; prev = next = this; }

    Link<T>* prev;
    Link<T>* next;

    T* data; //ensure only iterators have access to this
};


template <class T>
class Iterator
{
public:
    Iterator() : link( 0 ) { } //**** remove this, remove this REMOVE THIS!!! dangerous as your implementation doesn't test for null links, always assumes they can be derefenced
    Iterator( Link<T> *p ) : link( p ) { }

    bool operator==( const Iterator<T>& it ) const { return link == it.link; }
    bool operator!=( const Iterator<T>& it ) const { return link != it.link; }
    bool operator!=( const Link<T> *p ) const { return p != link; }

    //here we have a choice, really I should make two classes one const the other not
    const T* operator*() const { return link->data; }
    T* operator*() { return link->data; }

    Iterator<T>& operator++() { link = link->next; return *this; } //**** does it waste time returning in places where we don't use the retval?

    bool isNull() const { return (link == 0); } //REMOVE WITH ABOVE REMOVAL you don't want null iterators to be possible

    void transferTo( Chain<T> &chain )
    {
        chain.append( remove() );
    }

    T* const remove() //remove from list, delete Link, data is returned NOT deleted
    {
        T* const d = link->data;
        Link<T>* const p = link->prev;

        link->data = 0;
        delete link;
        link = p; //make iterator point to previous element, YOU must check this points to an element

        return d;
    }

private:
    Link<T> *link;
};


template <class T>
class ConstIterator
{
public:
    ConstIterator( Link<T> *p ) : link( p ) { }

    bool operator==( const Iterator<T>& it ) const { return link == it.link; }
    bool operator!=( const Iterator<T>& it ) const { return link != it.link; }
    bool operator!=( const Link<T> *p ) const { return p != link; }

    const T* operator*() const { return link->data; }

    ConstIterator<T>& operator++() { link = link->next; return *this; }

private:
    const Link<T> *link;
};

//**** try to make a generic list class and then a brief full list template that inlines
//     thus reducing code bloat
template <class T>
class Chain
{
public:
    Chain() { }
    virtual ~Chain() { empty(); }

    void append( T* const data )
    {
        Link<T>* const link = new Link<T>( data );

        link->prev = head.prev;
        link->next = &head;

        head.prev->next = link;
        head.prev = link;
    }

    void transferTo( Chain &c )
    {
        if( isEmpty() ) return;

        Link<T>* const first = head.next;
        Link<T>* const last  = head.prev;

        head.unlink();

        first->prev = c.head.prev;
        c.head.prev->next = first;

        last->next = &c.head;
        c.head.prev = last;
    }

    void empty() { while( head.next != &head ) { delete head.next; } }

    Iterator<T>      iterator() const { return Iterator<T>( head.next ); }
    ConstIterator<T> constIterator() const { return ConstIterator<T>( head.next ); }
    const Link<T>   *end() const { return &head; }
    bool             isEmpty() const { return ( head.next == &head ); }

private:
    Link<T> head;
    void operator=( const Chain& ) {}
};


class Directory;
class QString;
class KUrl;

class File
{
protected:
  Directory        *m_parent;   //0 if this is treeRoot  
  QString           m_name;     //< file name
  QString           m_directory;//< the directory of the file
  FileSize          m_size;     //< size with subdirectories
  FileSize          m_ownSize;  //< size without subdirectories
  mode_t            m_mode;     //< file mode
  QString           m_owner;    //< file owner name
  QString           m_group;    //< file group name
  QString           m_perm;     //< file permissions string
  time_t            m_time;     //< file modification in time_t format
  bool              m_symLink;  //< true if the file is a symlink
  QString           m_mimeType; //< file mimetype
  bool              m_excluded; //< flag if the file is excluded from du
  int               m_percent;  //< percent flag

public:
  File( Directory *parentIn, const QString &nameIn, const QString &dir, FileSize sizeIn, mode_t modeIn,
        const QString &ownerIn, const QString &groupIn, const QString &permIn, time_t timeIn, bool symLinkIn,
        const QString &mimeTypeIn )
  : m_parent( parentIn ), m_name( nameIn ), m_directory( dir ), m_size( sizeIn ), m_ownSize( sizeIn ), m_mode( modeIn ), 
    m_owner( ownerIn ), m_group( groupIn ), m_perm( permIn ), m_time( timeIn ), m_symLink( symLinkIn ), 
    m_mimeType( mimeTypeIn ), m_excluded( false ), m_percent( -1 ) {}
    
  File( const QString &nameIn, FileSize sizeIn )
  : m_parent( 0 ), m_name( nameIn ), m_directory( QString() ), m_size( sizeIn ), m_ownSize( sizeIn ), m_mode( 0 ), 
    m_owner( QString() ), m_group( QString() ), m_perm( QString() ), m_time( -1 ), 
    m_symLink( false ), m_mimeType( QString() ), m_excluded( false ), m_percent( -1 ) 
  {
  }
  
  virtual ~File() {}
      
  inline const QString &  name()                const  {return m_name;}
  inline const QString &  directory()           const  {return m_directory;}
  inline const FileSize   size()                const  {return m_excluded ? 0 : m_size;}
  inline const FileSize   ownSize()             const  {return m_excluded ? 0 : m_ownSize;}
  inline const mode_t     mode()                const  {return m_mode;}
  inline const QString &  owner()               const  {return m_owner;}
  inline const QString &  group()               const  {return m_group;}
  inline const QString &  perm()                const  {return m_perm;}
  inline const time_t     time()                const  {return m_time;}
  inline const QString &  mime()                const  {return m_mimeType;}
  inline const bool       isSymLink()           const  {return m_symLink;}
  virtual const bool      isDir()               const  {return false;}
  inline const bool       isExcluded()          const  {return m_excluded;}
  inline void             exclude( bool flag )         {m_excluded = flag;}
  inline const int        intPercent()          const  {return m_percent;}
  inline const QString    percent()             const  {if( m_percent < 0 )
                                                         return "INV";
                                                        QString buf; 
                                                        buf.sprintf( "%d.%02d%%", m_percent / 100, m_percent % 100 );
                                                        return buf;}
  inline void             setPercent( int p )          {m_percent = p;}  
  inline const Directory* parent()              const  {return m_parent;}
  
  inline void setSizes( KIO::filesize_t totalSize, KIO::filesize_t ownSize )
  {
    m_ownSize = ownSize;
    m_size = totalSize;
  }  

  enum UnitPrefix { kilo, mega, giga, tera };

  static const FileSize DENOMINATOR[4];
  static const char PREFIX[5][2];

  QString fullPath( const Directory* = 0 ) const;
  QString humanReadableSize( UnitPrefix key = mega ) const;

  static QString humanReadableSize( FileSize size, UnitPrefix Key = mega );
  
  friend class Directory;
};


//TODO when you modify this to take into account hardlinks you should make the Chain layered not inherited
class Directory : public Chain<File>, public File
{
public:
  Directory( Directory *parentIn, const QString &nameIn, const QString &dir, FileSize sizeIn, mode_t modeIn,
             const QString &ownerIn, const QString &groupIn, const QString &permIn, time_t timeIn, bool symLinkIn, 
             const QString &mimeTypeIn )
  : File( parentIn, nameIn, dir, sizeIn, modeIn, ownerIn, groupIn, permIn, timeIn, symLinkIn, mimeTypeIn ),
    m_fileCount( 0 ) 
  {}
 
  Directory( const QString &name, QString url ) : File( name, 0 ), m_fileCount( 0 )
  {
    m_directory = url;
  }
  
  virtual ~Directory() {}  
  virtual const bool      isDir()               const  {return true;}
  
  void append( File *p )
  {
     ++m_fileCount;
     
     Directory *parent = m_parent;
     while( parent )
     {
       parent->m_fileCount++;
       parent = parent->m_parent;
     }
     
     Chain<File>::append( p );
     p->m_parent = this;
  }
  
  void remove( File *p )
  {
    for( Iterator<File> it = Chain<File>::iterator(); it != Chain<File>::end(); ++it )
      if( (*it) == p )
      {
        --m_fileCount;
        
        Directory *parent = m_parent;
        while( parent )
        {
          parent->m_fileCount--;
          parent = parent->m_parent;
        }
        
        it.remove();
        break;
      }
  }
    
  uint fileCount() const { return m_fileCount; }

private:
  Directory( const Directory& );
  void operator=( const Directory& );

  uint m_fileCount;
};

#endif
