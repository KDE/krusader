#ifndef KRPOPUPMENU_H
#define KRPOPUPMENU_H

#include <kpopupmenu.h>
#include <kurl.h>
#include <kuserprofile.h>
#include "listpanel.h"
#include "krpreviewpopup.h"
#include "../UserAction/useractionpopupmenu.h"
//Added by qt3to4:
#include <Q3ValueList>
#ifdef __LIBKONQ__
#include <konq_popupmenu.h>
#include <konqbookmarkmanager.h>
#endif

// should be renamed to KrContextMenu or similar
class KrPopupMenu : public KPopupMenu {
	Q_OBJECT
public:
	static void run(const QPoint &pos, ListPanel *panel);

protected:	
	KrPopupMenu(ListPanel *thePanel, QWidget *parent=0);
	~KrPopupMenu();
	void performAction(int id);
	void addEmptyMenuEntries(); // adds the choices for a menu without selected items
	void addCreateNewMenu(); // adds a "create new" submenu

	enum ID {
		OPEN_ID,
		OPEN_WITH_ID,
		OPEN_KONQ_ID,
		OPEN_TERM_ID,
		OPEN_TAB_ID,
		PREVIEW_ID,
		KONQ_MENU_ID,
		CHOOSE_ID,
		DELETE_ID,
		COPY_ID,
		MOVE_ID,
		RENAME_ID,
		PROPERTIES_ID,
		MOUNT_ID,
		UNMOUNT_ID,
		TRASH_ID,
		SHRED_ID,
		NEW_LINK_ID,
		NEW_SYMLINK_ID,
		REDIRECT_LINK_ID,
		SYNC_SELECTED_ID,
		SEND_BY_EMAIL_ID,
		LINK_HANDLING_ID,
		EJECT_ID,
		COPY_CLIP_ID,
		MOVE_CLIP_ID,
		PASTE_CLIP_ID,
		MKDIR_ID,
		NEW_TEXT_FILE_ID,
		CREATE_NEW_ID,
		SERVICE_LIST_ID // ALWAYS KEEP THIS ONE LAST!!!
	};

private:
	ListPanel *panel;
	bool empty, multipleSelections;
	KPopupMenu openWith, linkPopup, createNewPopup;
   KrPreviewPopup preview;
   KActionCollection *actions;
   KrViewItemList items; // list of selected items
   KrViewItem *item; // the (first) selected item
   KFileItemList _items;
   Q3ValueList<KServiceOffer> offers;
#ifdef __LIBKONQ__
   KonqPopupMenu *konqMenu;
#endif
};

#endif
