/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <dehtris@yahoo.de>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRSEARCHBAR_H
#define KRSEARCHBAR_H

#include <QComboBox>
#include <QToolButton>
#include <QWidget>

#include <KComboBox>

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
        MODE_DEFAULT = -1,
        // NOTE: values used for combobox index
        MODE_SEARCH = 0,
        MODE_SELECT = 1,
        MODE_FILTER = 2,
    };

public:
    KrSearchBar(KrView *view, QWidget *parent);
    /**
     * Set another KrView, old one must not be deleted yet!
     */
    void setView(KrView *view);
    /**
     * Hide the search bar if its mode was the search one.
     *
     * This function is normally used to hide bar after a user has quicksearched for something and found it.
     */
    void hideBarIfSearching();

public slots:
    void showBar(SearchMode mode = MODE_DEFAULT);
    void hideBar();
    /**
     * Reset search to empty string.
     */
    void resetSearch();

protected slots:
    void onModeChange();
    void onSearchChange();
    /**
     * Save current search string to settings
     */
    void saveSearchString();

protected:
    void keyPressEvent(QKeyEvent *) override;
    /**
     * Filter key events from view widget and text combo box
     */
    bool eventFilter(QObject *, QEvent *) override;

private:
    /**
     * Handle events when search bar is open or open it.
     */
    bool handleKeyPressEvent(QKeyEvent *ke);
    bool handleUpDownKeyPress(bool);
    bool handleLeftRightKeyPress(QKeyEvent *ke);
    /**
     * Set color of line edit to highlight if anything matches.
     */
    void indicateMatch(bool);
    static bool caseSensitive();

private:
    KrView *_view;
    QComboBox *_modeBox;
    KComboBox *_textBox;
    QToolButton *_openSelectDialogBtn;
    SearchMode _currentMode;
    bool _rightArrowEntersDirFlag;
};

#endif // KRSEARCHBAR_H
