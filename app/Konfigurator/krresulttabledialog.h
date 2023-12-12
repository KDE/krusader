/*
    SPDX-FileCopyrightText: 2005 Dirk Eschler <deschler@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRRESULTTABLEDIALOG_H
#define KRRESULTTABLEDIALOG_H

// QtWidgets
#include <QDialog>

#include "../Konfigurator/krresulttable.h"

class KrResultTableDialog : public QDialog
{
public:
    enum DialogType { Archiver = 1, Tool = 2 };

    KrResultTableDialog(QWidget *parent,
                        DialogType type,
                        const QString &caption,
                        const QString &heading,
                        const QString &headerIcon = QString(),
                        const QString &hint = QString());
    virtual ~KrResultTableDialog();

    const QString &getHeading() const
    {
        return _heading;
    }
    const QString &getHint() const
    {
        return _hint;
    }
    void setHeading(const QString &s)
    {
        _heading = s;
    }
    void setHint(const QString &s)
    {
        _hint = s;
    }

public slots:
    void showHelp();

protected:
    QString _heading;
    QString _hint;
    QString helpAnchor;

    KrResultTable *_resultTable;
};

#endif
