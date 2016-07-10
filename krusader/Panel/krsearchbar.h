/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <dehtris@yahoo.de>                          *
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

#ifndef KRSEARCHBAR_H
#define KRSEARCHBAR_H

#include <QComboBox>
#include <QToolButton>
#include <QWidget>

#include <KCompletion/KComboBox>

class KrView;

/**
 * @brief Search, select or filter files by search string
 *
 * The search bar can be used in three different modes:
 *    search: cursor jumpes with up/down arrow keys to previous/next matching file
 *    select: all files in model matching an expression are selected
 *    filter: all files in model not matching an expression are filtered (hidden)
 *
 * These actions are implemented in KrView and KrViewOperator.
 *
 * NOTE: some key events in KrView are filtered here.
 */
class KrSearchBar : public QWidget
{
    Q_OBJECT

public:
    enum SearchMode {
        MODE_LAST = -1,
        // NOTE: values used for combobox index
        MODE_SEARCH = 0,
        MODE_SELECT = 1,
        MODE_FILTER = 2,
    };

public:
    KrSearchBar(KrView *view, QWidget *parent);
    void setView(KrView *view); // set another KrView, old one must not be deleted yet!

public slots:
    void showBar(SearchMode mode = MODE_LAST);
    void hideBar();
    void resetSearch(); // reset search to empty string

protected slots:
    void onModeChange();
    void onSearchChange();
    void saveSearchString(); // save current search string to settings

protected:
    void keyPressEvent(QKeyEvent *) Q_DECL_OVERRIDE;
    // filter key events from view widget and text combo box
    bool eventFilter(QObject *, QEvent *) Q_DECL_OVERRIDE;

private:
    bool handleKeyPressEvent(QKeyEvent *e); // handle events when search bar is open or open it
    bool handleUpDownKeyPress(bool);
    void indicateMatch(bool); // set color of line edit to highlight if anything matches
    static bool caseSensitive();

private:
    KrView *_view;
    QComboBox *_modeBox;
    KComboBox *_textBox;
    QToolButton *_openSelectDialogBtn;
    SearchMode _currentMode;
};

#endif // KRSEARCHBAR_H
