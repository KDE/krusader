/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2000 Rafi Yanai <yanai@users.sourceforge.net>               *
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

#ifndef KRDIALOGS_H
#define KRDIALOGS_H

#include <QtGui/QLineEdit>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtCore/QDateTime>
#include <QtGui/QPixmap>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>

#include <kdialog.h>
#include <kanimatedbutton.h>
#include <kurlrequesterdialog.h>
#include <kdatepicker.h>

/** \class KChooseDir
 * Used for asking the user for a folder.
 * example:
 * \code
 * KUrl u = KChooseDir::getDir("target folder", "/suggested/path", ACTIVE_PANEL->virtualPath());
 * if (u.isEmpty()) {
 *   // user canceled (either by pressing cancel, or esc
 * } else {
 *   // do you thing here: you've got a safe url to use
 * }
 * \endcode
 */
class KChooseDir
{
public:
    /**
     * \param text - description of the info requested from the user
     * \param url - a suggested url to appear in the box as a default choice
     * \param cwd - a path which is the current working directory (usually ACTIVE_PANEL->virtualPath()).
     *              this is used for completion of partial urls
     */
    static KUrl getDir(QString text, const KUrl& url, const KUrl& cwd);
    static KUrl getDir(QString text, const KUrl& url, const KUrl& cwd, bool & queue);
    static KUrl getDir(QString text, const KUrl& url, const KUrl& cwd, bool & queue, bool & preserveAttrs);
    static KUrl getDir(QString text, const KUrl& url, const KUrl& cwd, bool & queue, bool & preserveAttrs, KUrl &baseURL);
};

class KUrlRequesterDlgForCopy : public KDialog
{
    Q_OBJECT
public:
    KUrlRequesterDlgForCopy(const QString& url, const QString& text, bool presAttrs,
                            QWidget *parent, bool modal = true, KUrl baseURL = KUrl());
    KUrlRequesterDlgForCopy();

    KUrl selectedURL() const;
    KUrl baseURL() const;
    bool preserveAttrs();
    bool enqueue() {
        return queue;
    }
    bool copyDirStructure();
    void hidePreserveAttrs() {
//         preserveAttrsCB->hide();
    }

    KUrlRequester *urlRequester();

protected:
    virtual void keyPressEvent(QKeyEvent *e);

private slots:
    void slotClear();
    void slotQueue();
    void slotTextChanged(const QString &);
    void slotDirStructCBChanged();
private:
    KUrlRequester *urlRequester_;
    QComboBox *baseUrlCombo;
//     QCheckBox *preserveAttrsCB;
    QCheckBox *copyDirStructureCB;
    bool queue;
};

class KRGetDate : public KDialog
{
    Q_OBJECT
public:
    KRGetDate(QDate date = QDate::currentDate(), QWidget *parent = 0);
    QDate getDate();

private slots:
    void setDate(QDate);

private:
    KDatePicker *dateWidget;
    QDate chosenDate, originalDate;
};

#endif
