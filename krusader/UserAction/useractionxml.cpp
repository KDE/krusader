//
// C++ Implementation: useraction
//
// Description: This handles the useraction.xml
//
//
// Author: Jonas B�r (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <kstandarddirs.h>
#include "../krusader.h"
#include <klocale.h>

#include <qfile.h>
#include <qstring.h>
// for the xml-parsing
#include <qdom.h>

#include "useractionxml.h"
#include "useractionproperties.h"

#define ACTION_XML		"krusader/useractions.xml"
#define ACTION_DOCTYPE		"KrusaderUserActions"
// in well formed XML the root-element has to have the same name then the doctype:
#define ACTION_ROOT	ACTION_DOCTYPE
#define ACTION_PROCESSINSTR	"version=\"1.0\" encoding=\"UTF-8\" "


UserActionXML::UserActionXML() {
  // this return the local version of the file if this exists. else the global one is returnd
  _filename = locate( "data", ACTION_XML );
  getActionDom();
  // for further writing use only the local version!
  _filename = locateLocal( "data", ACTION_XML );
//   kdDebug() << "UserActionXML: _dom = " << _doc << endl;
}

UserActionXML::UserActionXML( QString filename ) {
  _filename = filename;
  getActionDom();
}

bool UserActionXML::validDoc() {
   if (_doc)
      return true;
   else
      return false;
}

void UserActionXML::getActionDom() {

  _doc = new QDomDocument( ACTION_DOCTYPE );
  QFile file( _filename );
  if( file.open( IO_ReadOnly ) ) {
    //kdDebug() << "UserAction::getActionDom: " << _filename << "could be opened" << endl;
    if( !_doc->setContent( &file ) ) {
      //kdDebug() << "UserAction::getActionDom: content set - failed" << endl;
      // if the file doesn't exist till now, the content CAN be set but is empty.
      // if the content can't be set, the file exists and is NOT an xml-file.
      file.close();
      delete _doc; _doc = 0;
      //TODO: replace me with a KMessageBox as soon as i18n-freeze is over!
      krOut << "There is a file called " << _filename << " which does not contain valid UserActions." << endl
      		<< "Please rename or delete it in order to get proper UserAction support" << endl;
    }
    file.close();
    
    if ( _doc ) {
      QDomElement root = _doc->documentElement();
      // check if the file got the right root-element (ACTION_ROOT) - this finds out if the xml-file read to the DOM is realy an krusader useraction-file
      if( root.tagName() != ACTION_ROOT ) {
        krOut << "UserActions: the actionfile's root-element isn't called "ACTION_ROOT", using " << _filename << endl;
        delete _doc; _doc = 0;
      }
    }

  } // if ( file.open( IO_ReadOnly ) )
  else { //create empty XML-document since no file was found
    // adding: <?xml version="1.0" encoding="UTF-8" ?>
    _doc->appendChild( _doc->createProcessingInstruction( "xml", ACTION_PROCESSINSTR ) );
    //adding root-element
    _doc->appendChild( _doc->createElement( ACTION_ROOT ) ); // create new actionfile by adding a root-element ACTION_ROOT
  }
    
}

void UserActionXML::writeActionDom() {
  writeActionDom( _filename );
}

void UserActionXML::writeActionDom( QString filename ) {
  QFile file( filename );
   if( !file.open( IO_WriteOnly ) ) {
     //TODO: create warning
     return;
   }
   
   if ( ! _doc->firstChild().isProcessingInstruction() ) {
      // adding: <?xml version="1.0" encoding="UTF-8" ?> if not already present
      QDomProcessingInstruction instr = _doc->createProcessingInstruction( "xml", ACTION_PROCESSINSTR );
      _doc->insertBefore( instr, _doc->firstChild() );
   }
 
   QTextStream ts( &file );
   ts << _doc->toString().utf8();
 
   file.close();
}

QDomElement UserActionXML::makeActionElement( UserActionProperties *prop ) {
  QDomElement property;
  QDomAttr attribute;
 
  //create child - the action itself
  QDomElement action = _doc->createElement( "action" );
  if ( ! prop->name()->simplifyWhiteSpace().isEmpty() ) {
    attribute = _doc->createAttribute( "name" );
    attribute.setValue( *prop->name() );
    action.setAttributeNode( attribute );
  }
    
  //property: <title>
  property = _doc->createElement( "title" );
  property.appendChild( _doc->createTextNode( *prop->title() ) );
  action.appendChild( property );
  
  //property: <tooltip>
  if ( ! prop->tooltip()->simplifyWhiteSpace().isEmpty() ) {
    property = _doc->createElement( "tooltip" );
    property.appendChild( _doc->createTextNode( *prop->tooltip() ) );
    action.appendChild( property );
  }
  
  //property: <icon>
  if ( ! prop->icon()->simplifyWhiteSpace().isEmpty() ) {
    property = _doc->createElement( "icon" );
    property.appendChild( _doc->createTextNode( *prop->icon() ) );
    action.appendChild( property );
  }

  //property: <category>
  if ( ! prop->category()->simplifyWhiteSpace().isEmpty() ) {
    property = _doc->createElement( "category" );
    property.appendChild( _doc->createTextNode( *prop->category() ) );
    action.appendChild( property );
  }

  //property: <descriptopn>
  if ( prop->descriptionUseTooltip() || ! prop->description()->simplifyWhiteSpace().isEmpty() ) {
    property = _doc->createElement( "description" );
    if ( prop->descriptionUseTooltip() ) {
      attribute = _doc->createAttribute( "same_as" );
      attribute.setValue( "tooltip" );
      property.setAttributeNode( attribute );
    } else
    if ( ! prop->description()->simplifyWhiteSpace().isEmpty() ) {
      property.appendChild( _doc->createTextNode( *prop->description() ) );
    }
    action.appendChild( property );
  } // </description>
  
  //property: <command>
  property = _doc->createElement( "command" );
  if (prop->execType() == UserActionProperties::Terminal ) {
    attribute = _doc->createAttribute( "executionmode" );
    attribute.setValue( "terminal" );
    property.setAttributeNode( attribute );
  } else
  if (prop->execType() == UserActionProperties::CollectOutput) {
    attribute = _doc->createAttribute( "executionmode" );
    if ( prop->separateStderr() )
      attribute.setValue( "collect_output_separate_stderr" );
    else
      attribute.setValue( "collect_output" );
    property.setAttributeNode( attribute );
  }
  if ( prop->acceptURLs() ) {
    attribute = _doc->createAttribute( "accept" );
    attribute.setValue( "url" );
    property.setAttributeNode( attribute );
  }
  if ( prop->confirmExecution() ) {
    attribute = _doc->createAttribute( "confirmexecution" );
    attribute.setValue( "true" );
    property.setAttributeNode( attribute );
  }
  if ( ! prop->user()->isEmpty() ) {
    attribute = _doc->createAttribute( "run_as" );
    attribute.setValue( *prop->user() );
    property.setAttributeNode( attribute );
  }
  property.appendChild( _doc->createTextNode( *prop->command() ) );
  action.appendChild( property );
  // </command>
  
  //property: <startpath>
  if ( ! prop->startpath()->simplifyWhiteSpace().isEmpty() ) {
    property = _doc->createElement( "startpath" );
    property.appendChild( _doc->createTextNode( *prop->startpath() ) );
    action.appendChild( property );
  }
  
  //property: <availability>
  if ( !prop->showonlyProtocol()->empty() || !prop->showonlyPath()->empty() || !prop->showonlyMime()->empty() || !prop->showonlyFile()->empty() ) {
    property = _doc->createElement( "availability" );
    QDomElement subproperty, subsubproperty;
    
    // <protocol>
    if ( !prop->showonlyProtocol()->empty() ) {
      subproperty = _doc->createElement( "protocol" );
      for ( QStringList::iterator it = prop->showonlyProtocol()->begin(); it != prop->showonlyProtocol()->end(); ++it ) {
        subsubproperty = _doc->createElement( "show" );
        subsubproperty.appendChild( _doc->createTextNode( *it ) );
        subproperty.appendChild( subsubproperty );
      }
      property.appendChild( subproperty );
    } // </protocol>
    
    // <path>
    if ( !prop->showonlyPath()->empty() ) {
      subproperty = _doc->createElement( "path" );
      for ( QStringList::iterator it = prop->showonlyPath()->begin(); it != prop->showonlyPath()->end(); ++it ) {
        subsubproperty = _doc->createElement( "show" );
        subsubproperty.appendChild( _doc->createTextNode( *it ) );
        subproperty.appendChild( subsubproperty );
      }
      property.appendChild( subproperty );
    } // </path>
    
    // <mimetype>
    if ( !prop->showonlyMime()->empty() ) {
      subproperty = _doc->createElement( "mimetype" );
      for ( QStringList::iterator it = prop->showonlyMime()->begin(); it != prop->showonlyMime()->end(); ++it ) {
        subsubproperty = _doc->createElement( "show" );
        subsubproperty.appendChild( _doc->createTextNode( *it ) );
        subproperty.appendChild( subsubproperty );
      }
      property.appendChild( subproperty );
    }  // </mimetype>
    
    // <filename>
    if ( !prop->showonlyFile()->empty() ) {
      subproperty = _doc->createElement( "filename" );
      for ( QStringList::iterator it = prop->showonlyFile()->begin(); it != prop->showonlyFile()->end(); ++it ) {
        subsubproperty = _doc->createElement( "show" );
        subsubproperty.appendChild( _doc->createTextNode( *it ) );
        subproperty.appendChild( subsubproperty );
      }
      property.appendChild( subproperty );
    }  // </filename>
    
    action.appendChild( property );
  } // </availability>
  
  //property: <defaultshortcut>
  if ( ! prop->defaultShortcut()->isNull() ) {
    property = _doc->createElement( "defaultshortcut" );
    property.appendChild( _doc->createTextNode( prop->defaultShortcut()->toStringInternal() ) );	//.toString() would return a localised string which can't be read again
    action.appendChild( property );
  }

  
  return action;
}

QDomElement* UserActionXML::findActionByName( QString name ) {
  if (!_doc)
     return 0;

  QDomElement root = _doc->documentElement();
  
  for (QDomNode node = root.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    QDomElement element = node.toElement();
    if( !element.isNull() ) {
      if( element.tagName() == "action" ) {
        if ( name == element.attribute( "name", "" ) ) {
          QDomElement *tmp = new QDomElement;
	  *tmp = element;
          return tmp;
        } // if ( name == element.attribute( "name", "" ) )
      }
    }
  } // for (QDomNode node = root.firstChild(); !node.isNull(); node = node.nextSibling() )
  
  return 0;
}

void UserActionXML::addActionToDom( UserActionProperties *prop ) {

  QDomElement root = _doc->documentElement();
  
  root.appendChild( makeActionElement( prop) );
}

void UserActionXML::removeAction( QString name ) {
  QDomElement *oldAction =  findActionByName( name );
  if ( oldAction ) {
    removeAction( oldAction );
    delete oldAction;
  }
}

void UserActionXML::removeAction( QDomElement *action ) {
  if (action == 0)
    return;
    
  QDomElement root = _doc->documentElement();
  
  root.removeChild( *action );
    
}

bool UserActionXML::updateAction( UserActionProperties *prop ) {
  return updateAction( *prop->name(), prop );
}

bool UserActionXML::updateAction( QString name, UserActionProperties *prop ) {
  QDomElement *oldAction =  findActionByName( name );
  if ( oldAction ) {
    updateAction( oldAction, prop );
    delete oldAction;
    return true;
  }
  else
    return false;
}

void UserActionXML::updateAction( QDomElement *action, UserActionProperties *prop ) {
  if (action == 0)
    return;
    
  QDomElement root = _doc->documentElement();
  
  root.replaceChild( makeActionElement( prop ), *action );
    
}

UserActionProperties* UserActionXML::readAction( QDomElement *action ) {
  if (action == 0)
    return 0;

  UserActionProperties *prop = new UserActionProperties;
  
  QString attr;
  prop->setName( action->attribute( "name", "" ) );

  for ( QDomNode node = action->firstChild(); !node.isNull(); node = node.nextSibling() ) {
    QDomElement e = node.toElement();
    
    // <title>
    if ( e.tagName() == "title" ) {
      //kdDebug() << "UserAction: title found: " << e.text() << endl;
      prop->setTitle( e.text() );
    } else
    
    // <tooltip>
    if ( e.tagName() == "tooltip" ) {
      //kdDebug() << "UserAction: tooltip found: " << e.text() << endl;
      prop->setTooltip( e.text() );
    } else
    
    // <icon>
    if ( e.tagName() == "icon" ) {
      //kdDebug() << "UserAction: icon found: " << e.text() << endl;
      prop->setIcon( e.text() );
    } else
    
    // <category>
    if ( e.tagName() == "category" ) {
      //kdDebug() << "UserAction: category found: " << e.text() << endl;
      prop->setCategory( e.text() );
    } else
    
    // <description>
    if ( e.tagName() == "description" ) {
      //kdDebug() << "UserAction: description found: " << e.text() << endl;
      attr = e.attribute( "same_as", "" ); // default: not set
      if ( attr == "" ) {
        prop->setDescription( e.text() );
        prop->setDescriptionUseTooltip( false );
      }
      else if ( attr == "tooltip") {
        prop->setDescription( *prop->tooltip() );
        prop->setDescriptionUseTooltip( true );
      }
      else
        krOut << "unrecognized attribute value in "ACTION_XML" found <action name=\"" << prop->name() << "\"><command accept=\"" << attr << "\""<< endl;
      
    } // </description>
    else
    
    // <command>
    if (e.tagName() == "command") {
      //kdDebug() << "UserAction: command found: " << e.text() << endl;
      prop->setCommand( e.text() );
      
      attr = e.attribute( "executionmode", "normal" ); // default: "normal"
      prop->setSeparateStderr( false );
      if ( attr == "terminal" )
        prop->setExecType( UserActionProperties::Terminal );
      else if ( attr == "normal")
        prop->setExecType( UserActionProperties::Normal );
      else if ( attr == "collect_output")
        prop->setExecType( UserActionProperties::CollectOutput );
      else if ( attr == "collect_output_separate_stderr") {
        prop->setExecType( UserActionProperties::CollectOutput );
        prop->setSeparateStderr( true );
      }
      else
        krOut << "unrecognized attribute value in "ACTION_XML" <action name=\"" << prop->name() << "\"><command executionmode=\"" << attr << "\""<< endl;

      attr = e.attribute( "accept", "local" ); // default: "local"
      if ( attr == "local" )
        prop->setAcceptURLs( false );
      else if ( attr == "url")
        prop->setAcceptURLs( true );
      else
        krOut << "unrecognized attribute value in "ACTION_XML" found <action name=\"" << prop->name() << "\"><command accept=\"" << attr << "\""<< endl;

      attr = e.attribute( "confirmexecution", "false" ); // default: "false"
      if ( attr == "false" )
        prop->setConfirmExecution( false );
      else
        prop->setConfirmExecution( true );

     prop->setUser( e.attribute( "run_as", QString::null ) );
    } // </command>
    else
    
    // <startpath>
    if ( e.tagName() == "startpath" ) {
      //kdDebug() << "UserAction: startpath found: " << e.text() << endl;
      prop->setStartpath( e.text() );
    } else
    
    // <availability>
    if (e.tagName() == "availability") {
      //kdDebug() << "UserAction: availability found: " << e.text() << endl;
      for ( QDomNode availabilitynode = e.firstChild(); !availabilitynode.isNull(); availabilitynode = availabilitynode.nextSibling() ) {
        QDomElement availabilityelement = availabilitynode.toElement();
        // <protocol>
        if ( availabilityelement.tagName() == "protocol" ) {
          //kdDebug() << "UserAction: availability->protocol found: " << availabilityelement.text() << endl;
          for ( QDomNode subnode = availabilityelement.firstChild(); !subnode.isNull(); subnode = subnode.nextSibling() ) {
            QDomElement subelement = subnode.toElement();
            if ( subelement.tagName() == "show" ) {
              //kdDebug() << "UserAction: availability->protocol->show found: " << subelement.text() << endl;
              prop->showonlyProtocol()->append( subelement.text() );
            }
          }
        } // </protocol>
        else

        // <path>
        if ( availabilityelement.tagName() == "path" ) {
          //kdDebug() << "UserAction: availability->path found: " << availabilityelement.text() << endl;
          for ( QDomNode subnode = availabilityelement.firstChild(); !subnode.isNull(); subnode = subnode.nextSibling() ) {
            QDomElement subelement = subnode.toElement();
            if ( subelement.tagName() == "show" ) {
              //kdDebug() << "UserAction: availability->path->show found: " << subelement.text() << endl;
              prop->showonlyPath()->append( subelement.text() );
            }
          }
        } // </path>
        else

        // <mimetype>
        if ( availabilityelement.tagName() == "mimetype" ) {
          //kdDebug() << "UserAction: availability->mimetype found: " << availabilityelement.text() << endl;
          for ( QDomNode subnode = availabilityelement.firstChild(); !subnode.isNull(); subnode = subnode.nextSibling() ) {
            QDomElement subelement = subnode.toElement();
            if ( subelement.tagName() == "show" ) {
              //kdDebug() << "UserAction: availability->mimetype->show found: " << subelement.text() << endl;
              prop->showonlyMime()->append( subelement.text() );
            }
          }
        } // </mimetype>
        else

        // <filename>
        if ( availabilityelement.tagName() == "filename" ) {
          //kdDebug() << "UserAction: availability->filename found: " << availabilityelement.text() << endl;
          for ( QDomNode subnode = availabilityelement.firstChild(); !subnode.isNull(); subnode = subnode.nextSibling() ) {
            QDomElement subelement = subnode.toElement();
            if ( subelement.tagName() == "show" ) {
              //kdDebug() << "UserAction: availability->filename->show found: " << subelement.text() << endl;
              prop->showonlyFile()->append( subelement.text() );
            }
          }
        } // </filename>
      
      }
    } // </availability>
    else
    
    // <defaultshortcut>
    if ( e.tagName() == "defaultshortcut" ) {
      //kdDebug() << "UserAction: defaultshortcut found: " << e.text() << endl;
      prop->defaultShortcut()->init( e.text() );
      //kdDebug() << "UserAction: defaultshortcut verify: " << prop->defaultShortcut.toString() << endl;
    } else

    // unknown but not empty (comments, <!-- -->, are empty)
    if (e.tagName() != "") {
      krOut << "unrecognized tag in "ACTION_XML" found: <action name=\"" << prop->name() << "\"><" << e.tagName() << ">" << endl;
    }
  } // for ( QDomNode node = action->firstChild(); !node.isNull(); node = node.nextSibling() )
  
  //kdDebug() << "UserAction: New action; distnct name: \"" << prop->name << "\"\n";
  return prop;
}

UserActionProperties* UserActionXML::readAction( QString name ) {
  QDomElement *action =  findActionByName( name );
  if ( action ) {
    UserActionProperties* prop = readAction( action );
    delete action;
    return prop;
  }
  else
    return 0;
}

QStringList UserActionXML::getActionNames() {
  if (_doc == 0)
    return 0;
    
  QStringList actionNames = QStringList();
  
  QDomElement root = _doc->documentElement();
  
  for (QDomNode node = root.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    QDomElement element = node.toElement();
    if( !element.isNull() ) {
      actionNames.append( element.attribute( "name", "" ) );
    }
  } // for (QDomNode node = root.firstChild(); !node.isNull(); node = node.nextSibling() )
      
  return actionNames;

}

QStringList UserActionXML::getActionCategories() {
  if (_doc == 0)
    return 0;
    
  QStringList actionCategories = QStringList();
  
  QDomElement root = _doc->documentElement();
  
  for (QDomNode rootnode = root.firstChild(); !rootnode.isNull(); rootnode = rootnode.nextSibling() ) {
    QDomElement element = rootnode.toElement();
    if( !element.isNull() ) {
      for ( QDomNode actionnode = element.firstChild(); !actionnode.isNull(); actionnode = actionnode.nextSibling() ) {
        QDomElement e = actionnode.toElement();
        // <category>
        if ( e.tagName() == "category" ) {
          // add only if not already in list
          if ( actionCategories.find(e.text()) == actionCategories.end() )
            actionCategories.append( e.text() );
          break;
        }
      } // for ( QDomNode actionnode = element.firstChild(); !actionnode.isNull(); actionnode = actionnode.nextSibling() )
    }
  } // for (QDomNode node = root.firstChild(); !node.isNull(); node = node.nextSibling() )
      
  return actionCategories;
}

bool UserActionXML::nameExists( QString name ) {
  if ( findActionByName( name ) )
    return true;
  else
    return false;
}

