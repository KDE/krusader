/*****************************************************************************
 * Copyright (C) 2006 Csaba Karai <cskarai@freemail.hu>                      *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "mediabutton.h"
#include "../krslots.h"
#include "../MountMan/kmountman.h"

#include <QMouseEvent>
#include <QEvent>

#include <klocale.h>
#include <kmessagebox.h>
#include <kdiskfreespace.h>
#include <kio/global.h>
#include <QtGui/QCursor>
#include <kmountpoint.h>
#include <solid/deviceinterface.h>
#include <solid/storageaccess.h>
#include <solid/storagevolume.h>
#include <solid/opticaldisc.h>
#include <solid/opticaldrive.h>
#include <solid/devicenotifier.h>

MediaButton::MediaButton( QWidget *parent ) : QToolButton( parent ),
		popupMenu( 0 ), rightMenu( 0 ), openInNewTab( false )
{
	KIconLoader * iconLoader = new KIconLoader();
	QPixmap icon = iconLoader->loadIcon( "blockdevice", KIconLoader::Toolbar, 16 );

	setFixedSize( icon.width() + 4, icon.height() + 4 );
	setIcon( QIcon( icon ) );
	setText( i18n( "Open the available media list" ) );
	setToolTip( i18n( "Open the available media list" ) );
	setPopupMode( QToolButton::InstantPopup );
	setAcceptDrops( false );

	popupMenu = new QMenu( this );
	popupMenu->installEventFilter( this );
	Q_CHECK_PTR( popupMenu );

	setMenu( popupMenu );

	connect( popupMenu, SIGNAL( aboutToShow() ), this, SLOT( slotAboutToShow() ) );
	connect( popupMenu, SIGNAL( aboutToHide() ), this, SLOT( slotAboutToHide() ) );
	connect( popupMenu, SIGNAL( triggered( QAction * ) ), this, SLOT( slotPopupActivated( QAction * ) ) );
	
	Solid::DeviceNotifier *notifier = Solid::DeviceNotifier::instance();
	connect(notifier, SIGNAL(deviceAdded(const QString&)),
	        this, SLOT(slotDeviceAdded(const QString&)));
	connect(notifier, SIGNAL(deviceRemoved(const QString&)),
	        this, SLOT(slotDeviceRemoved(const QString&)));
	
	connect( &mountCheckerTimer, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
}

MediaButton::~MediaButton() {
}

void MediaButton::slotAboutToShow() {
	emit aboutToShow();
	
	popupMenu->clear();
	udiNameMap.clear();
	
	createMediaList();
}

void MediaButton::slotAboutToHide() {
	if( rightMenu )
		rightMenu->close();
	mountCheckerTimer.stop();
}

void MediaButton::createMediaList() {
	// devices detected by solid
	storageDevices = Solid::Device::listFromType( Solid::DeviceInterface::StorageAccess );
	
	for( int p = storageDevices.count()-1 ; p >= 0; p-- ) {
		Solid::Device device = storageDevices[ p ];
		QString udi     = device.udi();
		
		QString name;
		KIcon kdeIcon;
		if( !getNameAndIcon( device, name, kdeIcon ) )
			continue;
		
		QAction * act = popupMenu->addAction( kdeIcon, name );
		act->setData( QVariant( udi ) );
		udiNameMap[ udi ] = name;
		
		connect(device.as<Solid::StorageAccess>(), SIGNAL(accessibilityChanged(bool, const QString &)),
			this, SLOT(slotAccessibilityChanged(bool, const QString &)));
	}
	
	KMountPoint::List possibleMountList = KMountPoint::possibleMountPoints();
	KMountPoint::List currentMountList = KMountPoint::currentMountPoints();
	
	for (KMountPoint::List::iterator it = possibleMountList.begin(); it != possibleMountList.end(); ++it) {
		if( (*it)->mountType() == "nfs" || (*it)->mountType() == "smb" ) {
			QString path = (*it)->mountPoint();
			bool mounted = false;
			
			for (KMountPoint::List::iterator it2 = currentMountList.begin(); it2 != currentMountList.end(); ++it2) {
				if(( (*it2)->mountType() == "nfs" || (*it2)->mountType() == "smb" ) &&
				   (*it)->mountPoint() == (*it2)->mountPoint() ) {
					mounted = true;
					break;
				}
			}
			
			QString name = i18n( "Remote Share" ) + " [" + (*it)->mountPoint() + ']';
			QStringList overlays;
			if ( mounted )
				overlays << "emblem-mounted";
			KIcon kdeIcon("network-wired", 0, overlays);
			QAction * act = popupMenu->addAction( kdeIcon, name );
			QString udi = "remote:" + (*it)->mountPoint();
			act->setData( QVariant( udi ) );
		}
	}
	
	mountCheckerTimer.setSingleShot( true );
	mountCheckerTimer.start( 1000 );
}

bool MediaButton::getNameAndIcon( Solid::Device & device, QString &name, KIcon &kicon) {
	Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
	if( access == 0 )
		return false;
		
	QString udi     = device.udi();
	QString label   = i18n( "Unknown" );
	bool    mounted = access->isAccessible();
	QString path    = access->filePath();
	QString type    = i18n( "Unknown" );
	QString icon    = device.icon();
	QString fstype;
	QString size;
	
	Solid::StorageVolume * vol = device.as<Solid::StorageVolume> ();
	if( vol ) {
		label = vol->label();
		fstype = vol->fsType();
		size = KIO::convertSize( vol->size() );
	}
	
	bool printSize = false;
	
	if( icon == "media-floppy" )
		type = i18n( "Floppy" );
	else if( icon == "drive-optical" )
		type = i18n( "CD/DVD-ROM" );
	else if( icon == "drive-removable-media-usb-pendrive" )
		type = i18n( "USB pen drive" ), printSize = true;
	else if( icon == "drive-removable-media-usb" )
		type = i18n( "USB device" ), printSize = true;
	else if( icon == "drive-removable-media" )
		type = i18n( "Removable media" ), printSize = true;
	else if( icon == "drive-harddisk" )
		type = i18n( "Hard Disk" ), printSize = true;
	else if( icon == "camera-photo" )
		type = i18n( "Camera" );
	else if( icon == "media-optical-video" )
		type = i18n( "Video CD/DVD-ROM" );
	else if( icon == "media-optical-audio" )
		type = i18n( "Audio CD/DVD-ROM" );
	else if( icon == "media-optical" )
		type = i18n( "Recordable CD/DVD-ROM" );
	
	if( printSize && !size.isEmpty() )
		name += size + ' ';
	
	if( !label.isEmpty() )
		name += label + ' ';
	else
		name += type + ' ';
	
	if( !fstype.isEmpty() )
		name += '(' + fstype + ") ";
	if( !path.isEmpty() )
		name += '[' + path + "] ";
	
	name = name.trimmed();
	
	QStringList overlays;
	if ( mounted ) {
		overlays << "emblem-mounted";
	} else {
		overlays << QString(); // We have to guarantee the placement of the next emblem
	}
	if (vol && vol->usage()==Solid::StorageVolume::Encrypted) {
		overlays << "security-high";
	}
	kicon = KIcon(icon, 0, overlays);
	return true;
}

void MediaButton::slotPopupActivated( QAction * action ) {
	if( action && action->data().canConvert<QString>() )
	{
		QString id = action->data().toString();
		if( id.startsWith( "remote:" ) )
		{
			QString mountPoint = id.mid( 7 );
			
			bool mounted = false;
			KMountPoint::List currentMountList = KMountPoint::currentMountPoints();
			for (KMountPoint::List::iterator it = currentMountList.begin(); it != currentMountList.end(); ++it) {
				if(( (*it)->mountType() == "nfs" || (*it)->mountType() == "smb" ) &&
				   (*it)->mountPoint() == mountPoint ) {
					mounted = true;
					break;
				}
			}
			
			if( !mounted )
				mount( id, true, false );
			else
				emit openUrl( KUrl( mountPoint ) );
			return;
		}
		Solid::Device device( id );
		
		Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
		if( access && !access->isAccessible() ) {
			mount( id, true );
			return;
		}
		
		if( access && !access->filePath().isEmpty() )
			emit openUrl( KUrl( access->filePath() ) );
	}
}

void MediaButton::showMenu() {
	QMenu * pP = menu();
	if ( pP ) {
		menu() ->exec( mapToGlobal( QPoint( 0, height() ) ) );
	}
}

bool MediaButton::eventFilter( QObject *o, QEvent *e ) {
	if( o == popupMenu ) {
		if( e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonRelease ) {
			QMouseEvent *m = (QMouseEvent *)e;
			if( m->button() == Qt::RightButton ) {
				if( e->type() == QEvent::MouseButtonPress ) {
					QAction * act = popupMenu->actionAt( m->pos() );
					QString id;
					if( act && act->data().canConvert<QString>() )
						id = act->data().toString();
					if( !id.isEmpty() )
						rightClickMenu( id );
				}
 				m->accept();
				return true;
			}
		}
	}
	return false;
}

void MediaButton::rightClickMenu( QString udi ) {
	if( rightMenu )
		rightMenu->close();
	
	bool network = udi.startsWith( "remote:" );
	bool ejectable = false;
	bool mounted = false;
	KUrl openURL;
	
	if( network )
	{
		QString mountPoint = udi.mid( 7 );
		openURL = KUrl( mountPoint );
		KMountPoint::List currentMountList = KMountPoint::currentMountPoints();
		for (KMountPoint::List::iterator it = currentMountList.begin(); it != currentMountList.end(); ++it) {
			if(( (*it)->mountType() == "nfs" || (*it)->mountType() == "smb" ) &&
			   (*it)->mountPoint() == mountPoint ) {
				mounted = true;
				break;
			}
		}
	} else {
		Solid::Device device( udi );
		
		Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
		Solid::OpticalDisc  *optdisc = device.as<Solid::OpticalDisc>();
		if( access )
			openURL = KUrl( access->filePath() );
		if( access && access->isAccessible() )
			mounted = true;
		if( optdisc )
			ejectable = true;
	}
	
	QMenu * myMenu = rightMenu = new QMenu( popupMenu );
	QAction * actOpen = myMenu->addAction( i18n( "Open" ) );
	actOpen->setData( QVariant( 1 ) );
	QAction * actOpenNewTab = myMenu->addAction( i18n( "Open in a new tab" ) );
	actOpenNewTab->setData( QVariant( 2 ) );

	myMenu->addSeparator();
	if( !mounted ) {
		QAction * actMount = myMenu->addAction( i18n( "Mount" ) );
		actMount->setData( QVariant( 3 ) );
	} else {
		QAction * actUnmount = myMenu->addAction( i18n( "Unmount" ) );
		actUnmount->setData( QVariant( 4 ) );
	}
	if( ejectable ) {
		QAction * actEject = myMenu->addAction( i18n( "Eject" ) );
		actEject->setData( QVariant( 5 ) );
	}

	QAction *act = myMenu->exec( QCursor::pos() );

	int result = -1;
	if( act != 0 && act->data().canConvert<int>() )
		result = act->data().toInt();

	delete myMenu;
	if( rightMenu == myMenu )
		rightMenu = 0;
	else
		return;
	
	switch ( result ) {
	case 1:
	case 2:
		popupMenu->close();
		if( mounted ) {
			if( result == 1 )
				emit openUrl( openURL );
			else
				SLOTS->newTab( openURL );
		} else {
			mount( udi, true, result == 2 ); // mount first, when mounted open the tab
		}
		break;
	case 3:
		mount( udi );
		break;
	case 4:
		umount( udi );
		break;
	case 5:
		eject( udi );
		break;
	default:
		break;
	}
}

void MediaButton::mount( QString udi, bool open, bool newtab ) {
	if( udi.startsWith( "remote:" ) ) {
		QString mp = udi.mid( 7 );
		krMtMan.mount( mp, true );
		if( newtab )
			SLOTS->newTab( KUrl( mp ) );
		else
			emit openUrl( KUrl( mp ) );
		return;
	}
	Solid::Device device( udi );
	Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
	if( access && !access->isAccessible() ) {
		if( open )
			udiToOpen = device.udi(), openInNewTab = newtab;
		connect( access, SIGNAL( setupDone(Solid::ErrorType, QVariant, const QString &) ),
		         this, SLOT( slotSetupDone(Solid::ErrorType, QVariant, const QString &) ) );
		access->setup();
	}
}

void MediaButton::slotSetupDone(Solid::ErrorType error, QVariant errorData, const QString &udi) {
	if ( error == Solid::NoError ) {
		if( udi == udiToOpen ) {
			Solid::StorageAccess *access = Solid::Device( udi ).as<Solid::StorageAccess>();
			if( access && access->isAccessible() ) {
				if( openInNewTab )
					SLOTS->newTab( KUrl( access->filePath() ) );
				else
					emit openUrl( KUrl( access->filePath() ) );
			}
			udiToOpen = QString(), openInNewTab = false;
		}
	} else {
		if( udi == udiToOpen )
			udiToOpen = QString(), openInNewTab = false;
		QString name;
		if( udiNameMap.contains( udi ) )
			name = udiNameMap[ udi ];
		
		if (errorData.isValid()) {
			KMessageBox::sorry( this, i18n("An error occurred while accessing '%1', the system responded: %2",
			                    name, errorData.toString()));
		} else {
			KMessageBox::sorry( this, i18n("An error occurred while accessing '%1'",
			                    name));
		}
	}
}

void MediaButton::umount( QString udi ) {
	if( udi.startsWith( "remote:" ) ) {
		krMtMan.unmount( udi.mid( 7 ), false );
		return;
	}
	Solid::Device device( udi );
	Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
	if( access && access->isAccessible() ) {
		connect( access, SIGNAL( teardownDone(Solid::ErrorType, QVariant, const QString &) ),
		         this, SLOT( slotTeardownDone(Solid::ErrorType, QVariant, const QString &) ) );
		access->teardown();
	}
}

void MediaButton::slotTeardownDone(Solid::ErrorType error, QVariant errorData, const QString &udi) {
    if (error != Solid::NoError && errorData.isValid()) {
        KMessageBox::sorry( this, errorData.toString());
    }
}

void MediaButton::eject( QString udi ) {
	Solid::Device device( udi );
	Solid::OpticalDrive *drive = device.parent().as<Solid::OpticalDrive>();
	
	if (drive!=0) {
		connect(drive, SIGNAL(ejectDone(Solid::ErrorType, QVariant, const QString &)),
			this, SLOT(slotTeardownDone(Solid::ErrorType, QVariant, const QString &)));
		
		drive->eject();
	}
}

void MediaButton::slotAccessibilityChanged(bool accessible, const QString & udi) {
	QList<QAction *> actionList = popupMenu->actions();
	foreach( QAction * act, actionList ) {
		if( act && act->data().canConvert<QString>() && act->data().toString() == udi ) {
			Solid::Device device(udi);
			
			QString name;
			KIcon kdeIcon;
			if( getNameAndIcon( device, name, kdeIcon ) ) {
				act->setText( name );
				act->setIcon( kdeIcon );
			}
			break;
		}
	}
}

void MediaButton::slotDeviceAdded(const QString& udi) {
	if( popupMenu->isHidden() )
		return;
	
	Solid::Device device( udi );
	Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
	if( access == 0 )
		return;
	
	QString name;
	KIcon kdeIcon;
	if( !getNameAndIcon( device, name, kdeIcon ) )
		return;
		
	QAction * act = popupMenu->addAction( kdeIcon, name );
	act->setData( QVariant( udi ) );
	udiNameMap[ udi ] = name;
		
	connect(device.as<Solid::StorageAccess>(), SIGNAL(accessibilityChanged(bool, const QString &)),
		this, SLOT(slotAccessibilityChanged(bool, const QString &)));
}

void MediaButton::slotDeviceRemoved(const QString& udi) {
	if( popupMenu->isHidden() )
		return;
	
	QList<QAction *> actionList = popupMenu->actions();
	foreach( QAction * act, actionList ) {
		if( act && act->data().canConvert<QString>() && act->data().toString() == udi ) {
			popupMenu->removeAction( act );
			delete act;
			break;
		}
	}
}

void MediaButton::slotTimeout() {
	if( isHidden() )
		return;
	
	KMountPoint::List possibleMountList = KMountPoint::possibleMountPoints();
	KMountPoint::List currentMountList = KMountPoint::currentMountPoints();
	QList<QAction *> actionList = popupMenu->actions();
	
	foreach( QAction * act, actionList ) {
		if( act && act->data().canConvert<QString>() && act->data().toString().startsWith( "remote:" ) ) {
			QString mountPoint = act->data().toString().mid( 7 );
			bool available = false;
			
			for (KMountPoint::List::iterator it = possibleMountList.begin(); it != possibleMountList.end(); ++it) {
				if(( (*it)->mountType() == "nfs" || (*it)->mountType() == "smb" ) &&
				   (*it)->mountPoint() == mountPoint ) {
					available = true;
					break;
				}
			}
			
			if( !available ) {
				popupMenu->removeAction( act );
				delete act;
			}
		}
	}
	
	for (KMountPoint::List::iterator it = possibleMountList.begin(); it != possibleMountList.end(); ++it) {
		if( (*it)->mountType() == "nfs" || (*it)->mountType() == "smb" ) {
			QString path = (*it)->mountPoint();
			bool mounted = false;
			QString udi = "remote:" + path;
			
			QAction * correspondingAct = 0;
			foreach( QAction * act, actionList ) {
				if(act && act->data().canConvert<QString>() && act->data().toString() == udi ) {
					correspondingAct = act;
					break;
				}
			}
			for (KMountPoint::List::iterator it2 = currentMountList.begin(); it2 != currentMountList.end(); ++it2) {
				if(( (*it2)->mountType() == "nfs" || (*it2)->mountType() == "smb" ) &&
				   path == (*it2)->mountPoint() ) {
					mounted = true;
					break;
				}
			}
			
			QString name = i18n( "Remote Share" ) + " [" + (*it)->mountPoint() + ']';
			QStringList overlays;
			if ( mounted )
				overlays << "emblem-mounted";
			KIcon kdeIcon("network-wired", 0, overlays);
			
			if( !correspondingAct ) {
				QAction * act = popupMenu->addAction( kdeIcon, name );
				act->setData( QVariant( udi ) );
			} else {
				correspondingAct->setText( name );
				correspondingAct->setIcon( kdeIcon );
			}
		}
	}
	
	mountCheckerTimer.setSingleShot( true );
	mountCheckerTimer.start( 1000 );
}

#include "mediabutton.moc"
