#include "krviewitem.h"
#include "../VFS/krpermhandler.h"
#include <klocale.h>
#include <kmimetype.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <QPixmap>

#define PROPS	static_cast<const KrViewProperties*>(_viewProperties)

static int compareNumbers(QString& aS1, int& aPos1, QString& aS2, int& aPos2);

KrViewItem::KrViewItem(vfile *vf, const KrViewProperties* properties): 
	_vf(vf), dummyVfile(false), _viewProperties(properties), _hasExtension(false), _hidden(false), _extension("") {
	if (vf) {
		// check if the file has an extension
		const QString& vfName = vf->vfile_getName();
		int loc = vfName.lastIndexOf('.');
		if (loc>0) { // avoid mishandling of .bashrc and friend
			// check if it has one of the predefined 'atomic extensions'
			for (QStringList::const_iterator i = PROPS->atomicExtensions.begin(); i != PROPS->atomicExtensions.end(); ++i) {
				if (vfName.endsWith(*i)){
					loc = vfName.length() - (*i).length();
					break;
				}
			}
			_name = vfName.left(loc);
			_extension = vfName.mid(loc+1);
			_hasExtension=true;
		}
		
		if( vfName.startsWith('.') )
			_hidden = true;
	}
}

const QString& KrViewItem::name(bool withExtension) const {
	if (!withExtension && _hasExtension) return _name;
	else return _vf->vfile_getName();
}

QString KrViewItem::description() const {
	if (dummyVfile) return i18n("Climb up the directory tree");
	// else is implied
	QString text = _vf->vfile_getName();
	QString comment;
	KMimeType::Ptr mt = KMimeType::mimeType(_vf->vfile_getMime());
	if( mt )
		comment = mt->comment(_vf->vfile_getUrl());
	QString myLinkDest = _vf->vfile_getSymDest();
	KIO::filesize_t mySize = _vf->vfile_getSize();
	
	QString text2 = text;
	mode_t m_fileMode = _vf->vfile_getMode();
	
	if (_vf->vfile_isSymLink() ){
		QString tmp;
		if ( _vf->vfile_getMime() == "Broken Link !" ) tmp = i18n("(broken link !)");
		else if ( comment.isEmpty() ) tmp = i18n ( "Symbolic Link" ) ;
		else tmp = i18n("%1 (Link)", comment);
	
		text += "->";
	text += myLinkDest;
	text += "  ";
	text += tmp;
	} else if ( S_ISREG( m_fileMode ) ){
	text = QString("%1").arg(text2)+ QString(" (%1)").arg( PROPS->humanReadableSize ?
		KRpermHandler::parseSize(_vf->vfile_getSize()) : KIO::convertSize( mySize ) );
	text += "  ";
	text += comment;
	} else if ( S_ISDIR ( m_fileMode ) ){
	text += "/  ";
		if( _vf->vfile_getSize() != 0 ) {
			text += '(' +
                                ( PROPS->humanReadableSize ? KRpermHandler::parseSize(_vf->vfile_getSize()) : KIO::convertSize( mySize ) ) + ") ";
		}
		text += comment;
	} else {
	text += "  ";
	text += comment;
	}
	return text;
}

QString KrViewItem::dateTime() const {
   // convert the time_t to struct tm
   time_t time = _vf->vfile_getTime_t();
   struct tm* t=localtime((time_t *)&time);

   QDateTime tmp(QDate(t->tm_year+1900, t->tm_mon+1, t->tm_mday), QTime(t->tm_hour, t->tm_min));
   return KGlobal::locale()->formatDateTime(tmp);
}

QPixmap KrViewItem::icon() {
#if 0  
  QPixmap *p;

  // This is bad - very bad. the function must return a valid reference,
  // This is an interface flow - shie please fix it with a function that return QPixmap*
  // this way we can return 0 - and do our error checking...
  
  // shie answers: why? what's the difference? if we return an empty pixmap, others can use it as it
  // is, without worrying or needing to do error checking. empty pixmap displays nothing
#endif
	if (dummyVfile || !_viewProperties->displayIcons)
		return QPixmap();
	else return KrView::getIcon(_vf);
}

int KrViewItem::compareTexts(QString aS1, QString aS2, int asc, bool isName) const
// if aS1 is: less, equal, greater then aS2 then return: -1, 0, 1
{
   //check empty strings
   if( aS1.length() == 0 ) {
     if( aS2.length() == 0 ) return 0;
     else return 1;
   } else {
     if( aS2.length() == 0 ) return -1;
   }
   
   if( isName )
   {
     if ( aS1 == ".." ) {
        if ( aS2 == ".." ) return 0;
        else return 1*asc;
     } else {
        if ( aS2 == ".." ) return -1*asc;
     }
   }

   if( _viewProperties->sortMode & KrViewProperties::IgnoreCase )
   {
      aS1 = aS1.toLower();
      aS2 = aS2.toLower();
   }

   switch(_viewProperties->sortMethod)
   {
      case KrViewProperties::Alphabetical:
         return compareTextsAlphabetical(aS1, aS2, false);
      case KrViewProperties::AlphabeticalNumbers:
         return compareTextsAlphabetical(aS1, aS2, true);
      case KrViewProperties::CharacterCode:
         return compareTextsCharacterCode(aS1, aS2, false);
      case KrViewProperties::CharacterCodeNumbers:
         return compareTextsCharacterCode(aS1, aS2, true);
      case KrViewProperties::Krusader:
      default:
         return compareTextsKrusader(aS1, aS2, asc, isName);
   }
}

int KrViewItem::compareTextsAlphabetical(QString& aS1, QString& aS2, bool aNumbers) const
// if aS1 is: less, equal, greater than aS2 then return: -1, 0, 1
// function which invoke (KrViewItem::compareTexts) ensure non empty aS1 and aS2 - checking is not needed
// similar to Total Commander sortings: "Alphabetical, considering accents" (aNumbers==false)
// or "Natural sorting: alphabetical and numbers" (aNumbers==true), but fixed (numbers are not before nonalfanumeric characters, but on their correct places)
{
   int lPositionS1 = 0;
   int lPositionS2 = 0;
   // sometimes, localeAwareCompare is not case sensitive. in that case, we need to fallback to a simple string compare (KDE bug #40131)
   bool lUseLocaleAware = (_viewProperties->sortMode & KrViewProperties::IgnoreCase)
      || _viewProperties->localeAwareCompareIsCaseSensitive;
   int j = 0;
   QChar lchar1;
   QChar lchar2;
   while(true)
   {
      lchar1 = aS1[lPositionS1];
      lchar2 = aS2[lPositionS2];
      // detect numbers
      if(aNumbers && lchar1.isDigit() && lchar2.isDigit() )
      {
         int j = compareNumbers(aS1, lPositionS1, aS2, lPositionS2);
         if( j != 0 ) return j;
      }
      else
      if( lUseLocaleAware
          && 
          ( ( lchar1 >= 128
              && ( (lchar2 >= 'A' && lchar2 <= 'Z') || (lchar2 >= 'a' && lchar2 <= 'z') || lchar2 >= 128 ) )
            ||
            ( lchar2 >= 128
              && ( (lchar1 >= 'A' && lchar1 <= 'Z') || (lchar1 >= 'a' && lchar1 <= 'z') || lchar1 >= 128 ) )
          )
        )
      {
         // use localeAwareCompare when a unicode character is encountered
         j = QString::localeAwareCompare(lchar1, lchar2);
         if(j != 0) return j;
         lPositionS1++;
         lPositionS2++;
      }
      else
      {
         // if characters are latin or localeAwareCompare is not case sensitive then use simple characters compare is enough
         if(lchar1 < lchar2) return -1;
         if(lchar1 > lchar2) return 1;
         lPositionS1++;
         lPositionS2++;
      }
      // at this point strings are equal, check if ends of strings are reached
      if(lPositionS1 == aS1.length() && lPositionS2 == aS2.length()) return 0;
      if(lPositionS1 == aS1.length() && lPositionS2 < aS2.length()) return -1;
      if(lPositionS1 < aS1.length() && lPositionS2 == aS2.length()) return 1;
   }
}

int KrViewItem::compareTextsCharacterCode(QString& aS1, QString& aS2, bool aNumbers) const
// if aS1 is: less, equal, greater than aS2 then return: -1, 0, 1
// The invoking function (KrViewItem::compareTexts) ensures non-empty aS1 and aS2 - checking is not needed
// similar to Total Commander sortings: "Strictly by numeric character code" (aNumbers==false)
// or "Natural sorting: by character code and numbers" (aNumbers==true), but fixed (correct unicode characters sorting)
{
   int lPositionS1 = 0;
   int lPositionS2 = 0;
   while(true)
   {
      // detect numbers
      if(aNumbers && aS1[lPositionS1].isDigit() && aS2[lPositionS2].isDigit())
      {
         int j = compareNumbers(aS1, lPositionS1, aS2, lPositionS2);
         if( j != 0 ) return j;
      }
      else
      {
         if(aS1[lPositionS1] < aS2[lPositionS2]) return -1;
         if(aS1[lPositionS1] > aS2[lPositionS2]) return 1;
         lPositionS1++;
         lPositionS2++;
      }
      // at this point strings are equal, check if ends of strings are reached
      if(lPositionS1 == aS1.length() && lPositionS2 == aS2.length()) return 0;
      if(lPositionS1 == aS1.length() && lPositionS2 < aS2.length()) return -1;
      if(lPositionS1 < aS1.length() && lPositionS2 == aS2.length()) return 1;
   }
}

int KrViewItem::compareTextsKrusader(QString& aS1, QString& aS2, int asc, bool isName) const
// if aS1 is: less, equal, greater than aS2 then return: -1, 0, 1
// The invoking function (KrViewItem::compareTexts) ensures non-empty aS1 and aS2 - checking is not needed
// this is traditional Krusader sorting algorithm using locale aware compare
// The algorithm skips nonalfanumeric characters
{
   // ensure "hidden" before others
   if( isName )
   {
      if( aS1[0] == '.' && aS2[0] != '.' ) return 1*asc;
      if( aS1[0] != '.' && aS2[0] == '.' ) return -1*asc;
   }

   // sometimes, localeAwareCompare is not case sensitive. in that case, we need to fallback to a simple string compare (KDE bug #40131)
   bool lUseLocaleAware = (_viewProperties->sortMode & KrViewProperties::IgnoreCase)
      || _viewProperties->localeAwareCompareIsCaseSensitive;

   if( lUseLocaleAware )
      return QString::localeAwareCompare(aS1, aS2);
   else
      // if localeAwareCompare is not case sensitive then use simple compare is enough
      return QString::compare(aS1, aS2);
}

static int compareNumbers(QString& aS1, int& aPos1, QString& aS2, int& aPos2)
// compares numbers within two strings
{
   int res = 0;
   int start1 = aPos1;
   int start2 = aPos2;
   while ( aPos1 < aS1.length() && aS1.at( aPos1 ).isDigit() ) aPos1++;
   while ( aPos2 < aS2.length() && aS2.at( aPos2 ).isDigit() ) aPos2++;
   // the left-most difference determines what's bigger
   int i1 = aPos1 - 1;
   int i2 = aPos2 - 1;
   for ( ; i1 >= start1 || i2 >= start2; i1--, i2--)
   {
     int c1 = 0;
     int c2 = 0;
     if ( i1 >= start1 ) c1 = aS1.at( i1 ).digitValue();
     if ( i2 >= start2 ) c2 = aS2.at( i2 ).digitValue();
     if ( c1 < c2 ) res = -1;
     else if ( c1 > c2 ) res = 1;
   }
   return res;
}
