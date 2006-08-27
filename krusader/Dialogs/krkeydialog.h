//
// C++ Interface: krkeydialog
//
// Description: 
//
//
// Author: Jonas Bähr <http://jonas-baehr.de/>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KRKEYDIALOG_H
#define KRKEYDIALOG_H

#include <kkeydialog.h>

/**
 * @short KDE's KKeyDialog extended by the ability to export/import shortcuts
 * @author Jonas Bähr
 */
class KrKeyDialog : protected KKeyDialog
{
Q_OBJECT
public:
    KrKeyDialog( QWidget* parent = 0 );
    ~KrKeyDialog();

protected slots:
    void slotImportShortcuts();
    void slotExportShortcuts();
};

#endif
