/*****************************************************************************
 * Copyright (C) 2005 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2007-2008 Csaba Karai <cskarai@freemail.hu>                 *
 * Copyright (C) 2008 Jonas Bähr <jonas.baehr@web.de>                        *
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

#ifndef CHECKSUMDLG_H
#define CHECKSUMDLG_H

// QtWidgets
#include <QDialog>

class KUrlRequester;
class QCheckBox;

extern void initChecksumModule();

class CreateChecksumDlg: public QDialog
{
public:
    CreateChecksumDlg(const QStringList& files, bool containFolders, const QString& path);
};


class MatchChecksumDlg: public QDialog
{
public:
    MatchChecksumDlg(const QStringList& files, bool containFolders,
                     const QString& path, const QString& checksumFile = QString());

    static QString checksumTypesFilter;

protected:
    bool verifyChecksumFile(QString path, QString& extension);
};


class ChecksumResultsDlg: public QDialog
{
public:
    ChecksumResultsDlg(const QStringList &stdOut, const QStringList &stdErr,
                       const QString& suggestedFilename, bool standardFormat);

public slots:
    virtual void accept() Q_DECL_OVERRIDE;

protected:
    bool saveChecksum(const QStringList& data, QString filename);
    bool savePerFile();

private:
    QCheckBox *_onePerFile;
    KUrlRequester *_checksumFileSelector;
    QStringList _data;
    QString _suggestedFilename;
};


class VerifyResultDlg: public QDialog
{
public:
    explicit VerifyResultDlg(const QStringList& failed);
};

#endif // CHECKSUMDLG_H
