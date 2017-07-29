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

#include "krsearchbar.h"

#include "PanelView/krview.h"
#include "PanelView/krviewitem.h"
#include "../FileSystem/dirlisterinterface.h"
#include "../defaults.h"
#include "../krglobal.h"

#include <QDebug>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QToolButton>
#include <QToolTip>

#include <KConfigCore/KSharedConfig>
#include <KIconThemes/KIconLoader>
#include <KI18n/KLocalizedString>


KrSearchBar::KrSearchBar(KrView *view, QWidget *parent)
    : QWidget(parent), _view(0)
{
    // close button
    QToolButton *closeButton = new QToolButton(this);
    closeButton->setAutoRaise(true);
    closeButton->setIcon(QIcon::fromTheme(QStringLiteral("dialog-close")));
    closeButton->setToolTip(i18n("Close the search bar"));
    connect(closeButton, SIGNAL(clicked()), SLOT(hideBar()));

    // combo box for changing search mode
    _modeBox = new QComboBox(this);
    _modeBox->addItems(QStringList() << i18n("Search") << i18n("Select") << i18n("Filter"));
     int defaultIndex = KConfigGroup (krConfig, "Look&Feel")
                      .readEntry("Default Search Mode",
                                 QString::number(KrSearchBar::MODE_SEARCH)).toInt();
    _modeBox->setCurrentIndex(defaultIndex);
    _modeBox->setToolTip(i18n("Change the search mode"));
    connect(_modeBox, SIGNAL(currentIndexChanged(int)), SLOT(onModeChange()));

    _currentMode = static_cast<SearchMode>(_modeBox->currentIndex());

    // combo box for entering search string
    _textBox = new KComboBox(this);
    _textBox->setEditable(true);
    _modeBox->setToolTip(i18n("Enter or select search string"));
    QStringList savedSearches = KConfigGroup(krConfig, "Private")
                      .readEntry("Predefined Selections", QStringList());
    if (savedSearches.count() > 0)
        _textBox->addItems(savedSearches);
    _textBox->setCurrentText("");
    _textBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    connect(_textBox, SIGNAL(currentTextChanged(QString)), SLOT(onSearchChange()));

    QToolButton *saveSearchBtn = new QToolButton(this);
    saveSearchBtn->setIcon(krLoader->loadIcon("document-save", KIconLoader::Toolbar, 16));
    saveSearchBtn->setFixedSize(20, 20);
    saveSearchBtn->setToolTip(i18n("Save the current search string"));
    connect(saveSearchBtn, SIGNAL(clicked()), this, SLOT(saveSearchString()));

    _openSelectDialogBtn = new QToolButton(this);
    _openSelectDialogBtn->setIcon(krLoader->loadIcon("configure", KIconLoader::Toolbar, 16));
    _openSelectDialogBtn->setFixedSize(20, 20);
    _openSelectDialogBtn->setToolTip(i18n("Open selection dialog"));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(closeButton);
    layout->addWidget(_modeBox);
    layout->addWidget(_textBox);
    layout->addWidget(saveSearchBtn);
    layout->addWidget(_openSelectDialogBtn);

    _textBox->installEventFilter(this);

    setView(view);
}

void KrSearchBar::setView(KrView *view)
{
    if (_view) {
        _view->widget()->removeEventFilter(this);
        disconnect(_openSelectDialogBtn, 0, 0, 0);
    }

    _view = view;

    connect(_openSelectDialogBtn, &QToolButton::clicked, [this](){ _view->customSelection(true); });
    _view->widget()->installEventFilter(this);
}

// #### public slots

void KrSearchBar::showBar(SearchMode mode)
{
    if (mode != MODE_LAST) {
        _modeBox->setCurrentIndex(mode);
    }
    show();
    _textBox->setFocus();
}

void KrSearchBar::hideBar()
{
    resetSearch();
    if (_textBox->hasFocus())
        _view->widget()->setFocus();
    hide();
}

void KrSearchBar::resetSearch()
{
    _textBox->clearEditText();
    indicateMatch(true);
}

// #### protected slots

void KrSearchBar::onModeChange()
{
    if (_currentMode == MODE_FILTER) {
        _view->op()->filterSearch(QString(), true); // reset filter
    }
    _currentMode = static_cast<SearchMode>(_modeBox->currentIndex());

    onSearchChange();
}

void KrSearchBar::onSearchChange()
{
    const QString text = _textBox->currentText();

    switch(_currentMode) {
        case MODE_SEARCH: {
            const bool anyMatch = _view->op()->searchItem(text, caseSensitive());
            indicateMatch(anyMatch);
            break;
        }
        case MODE_SELECT: {
            _view->unselectAll();
            if (!text.isEmpty()) {
                const bool anyMatch = _view->changeSelection(KRQuery(text, caseSensitive()), true);
                indicateMatch(anyMatch);
            }
            break;
        }
        case MODE_FILTER: {
            const bool anyMatch =_view->op()->filterSearch(text, caseSensitive());
            indicateMatch(anyMatch);
            break;
        }
        default:
            qWarning() << "unexpected search mode: " << _currentMode;
    }

    _textBox->setFocus();
}

void KrSearchBar::saveSearchString()
{
    KConfigGroup group(krConfig, "Private");
    QStringList lst = group.readEntry("Predefined Selections", QStringList());
    QString searchString = _textBox->currentText();
    if (lst.indexOf(searchString) != -1) {
        // already saved
        return;
    }

    lst.append(searchString);
    group.writeEntry("Predefined Selections", lst);

    _textBox->addItem(searchString);
    QToolTip::showText(QCursor::pos(), i18n("Saved search text to history"));
}

// #### protected

void KrSearchBar::keyPressEvent(QKeyEvent *event)
{
    const bool handled = handleKeyPressEvent(static_cast<QKeyEvent*>(event));
    if (handled) {
        return;
    }

    QWidget::keyPressEvent(event);
}

bool KrSearchBar::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() != QEvent::ShortcutOverride && watched == _view->widget()) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        // overwrite "escape" shortcut if bar is shown
        if ((ke->key() == Qt::Key_Escape) && (ke->modifiers() == Qt::NoModifier) && !isHidden()) {
            ke->accept();
            handleKeyPressEvent(ke);
            return true;
        }
    }

    if (event->type() != QEvent::KeyPress) {
        return false;
    }

    qDebug() << "key press event=" << event;

    QKeyEvent *ke = static_cast<QKeyEvent *>(event);
    if (watched == _view->widget()) {
        KConfigGroup grpSv(krConfig, "Look&Feel");
        const bool autoShow = grpSv.readEntry("New Style Quicksearch", _NewStyleQuicksearch);

        if (isHidden() && !autoShow) {
            return false;
        }

        if (!isHidden()) {
            const bool handled = handleKeyPressEvent(ke);
            if (handled) {
                return true;
            }
        }

        if (isHidden() ||
            // view can handle its own event if user does not want to remove text or...
            !((ke->key() == Qt::Key_Backspace && !_textBox->currentText().isEmpty()) ||
              // ...insert space in search bar (even if not focused)
              (ke->key() == Qt::Key_Space && _currentMode == KrSearchBar::MODE_SEARCH))) {
            const bool handled = _view->handleKeyEvent(ke);
            if (handled) {
                return true;
            }
        }

        if (ke->text().isEmpty() || (ke->modifiers() != Qt::NoModifier &&
                                     ke->modifiers() != Qt::ShiftModifier &&
                                     ke->modifiers() != Qt::KeypadModifier)) {
            return false;
        }

        // start searching if bar is hidden?
        if (isHidden()) {
            if (autoShow) {
                showBar();
            } else {
                return false;
            }
        }

        // bar is visible and gets the key input
        _textBox->setFocus();
        if (ke->key() == Qt::Key_Backspace) {
            _textBox->lineEdit()->backspace();
        } else {
            _textBox->setEditText(_textBox->currentText().append(ke->text()));
        }
        return true;
    } else if (watched == _textBox) {
        // allow the view to handle (most) key events from the text box
        if (ke->modifiers() == Qt::NoModifier &&
            ke->key() != Qt::Key_Space &&
            ke->key() != Qt::Key_Backspace &&
            ke->key() != Qt::Key_Left &&
            ke->key() != Qt::Key_Right) {
            bool handled = _view->handleKeyEvent(ke);
            if (handled) {
                _view->widget()->setFocus();
                return true;
            }
        }
    }
    return false;
}

// #### private

bool KrSearchBar::handleKeyPressEvent(QKeyEvent *ke)
{
    if (ke->modifiers() != Qt::NoModifier) {
        return false;
    }

    switch (ke->key()) {
    case Qt::Key_Escape: {
        hideBar();
        return true;
    }
    case Qt::Key_Up:
        return handleUpDownKeyPress(true);
    case Qt::Key_Down:
        return handleUpDownKeyPress(false);
    case Qt::Key_Insert: {
        // select current item and jump to next search result
        KrViewItem * item = _view->getCurrentKrViewItem();
        if (item) {
            item->setSelected(!item->isSelected());
            _view->op()->searchItem(_textBox->currentText(), caseSensitive(), 1);
        }
        return true;
    }
    case Qt::Key_Home: {
        // jump to first search result
        KrViewItem * item = _view->getLast();
        if (item) {
            _view->setCurrentKrViewItem(_view->getLast());
            _view->op()->searchItem(_textBox->currentText(), caseSensitive(), 1);
        }
        return true;
    }
    case Qt::Key_End: {
        // jump to last search result
        KrViewItem * item = _view->getFirst();
        if (item) {
            _view->setCurrentKrViewItem(_view->getFirst());
            _view->op()->searchItem(_textBox->currentText(), caseSensitive(), -1);
        }
        return true;
    }
    }
    return false;
}

bool KrSearchBar::handleUpDownKeyPress(bool up)
{
    if (_currentMode != MODE_SEARCH) {
        return false;
    }

    const bool updownCancel = KConfigGroup(krConfig, "Look&Feel")
                        .readEntry("Up/Down Cancels Quicksearch", false);
    if (updownCancel) {
        hideBar();
        return false;
    }

    const bool anyMatch = _view->op()->searchItem(_textBox->currentText(), caseSensitive(), up ? -1 : 1);
    indicateMatch(anyMatch);
    return true;
}

void KrSearchBar::indicateMatch(bool anyMatch)
{
    KConfigGroup gc(krConfig, "Colors");
    QPalette p = QGuiApplication::palette();
    QString foreground, background;
    if (anyMatch) {
        foreground = "Quicksearch Match Foreground";
        background = "Quicksearch Match Background";
    } else {
        foreground = "Quicksearch Non-match Foreground";
        background = "Quicksearch Non-match Background";
    }

    QColor fore = Qt::black;
    QString foreSetting = gc.readEntry(foreground, QString());
    if (foreSetting == "KDE default") {
        fore = p.color(QPalette::Active, QPalette::Text);
    } else if (!foreSetting.isEmpty()) {
        fore = gc.readEntry(foreground, fore);
    }

    QColor back = anyMatch ? QColor(192, 255, 192) : QColor(255, 192, 192);
    QString backSetting = gc.readEntry(background, QString());
    if (backSetting == "KDE default") {
        back = p.color(QPalette::Active, QPalette::Base);
    } else if (!backSetting.isEmpty()) {
        back = gc.readEntry(background, back);
    }

    QPalette pal = palette();
    pal.setColor(QPalette::Base, back);
    pal.setColor(QPalette::Text, fore);
    _textBox->lineEdit()->setPalette(pal);
}

bool KrSearchBar::caseSensitive()
{
    KConfigGroup grpSvr(krConfig, "Look&Feel");
    return grpSvr.readEntry("Case Sensitive Quicksearch", _CaseSensitiveQuicksearch);
}
