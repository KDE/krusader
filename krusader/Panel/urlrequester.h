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

#ifndef __URLREQUESTER_H__
#define __URLREQUESTER_H__

#include <kurlrequester.h>

class UrlRequester : public KUrlRequester
{
    Q_OBJECT
public:
    UrlRequester(KLineEdit *le, QWidget *parent);

    bool eventFilter(QObject * watched, QEvent * e);
    void setActive(bool active);
    void edit();

public slots:
    void slotTextChanged(const QString &text);

private:
    class PathLabel;

    PathLabel*_path;
};

#endif // __URLREQUESTER_H__
