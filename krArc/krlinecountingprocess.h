/*****************************************************************************
 * Copyright (C) 2001 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2001 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2019 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#ifndef KRLINECOUNTINGPROCESS_H
#define KRLINECOUNTINGPROCESS_H

#include <KCoreAddons/KProcess>

/**
 * A KProcess which emits how many lines it is writing to stdout or stderr.
 */
class KrLinecountingProcess : public KProcess
{
    Q_OBJECT
public:
    KrLinecountingProcess();
    void setMerge(bool);
    QString getErrorMsg();

public slots:
    void receivedError();
    void receivedOutput(QByteArray = QByteArray());

signals:
    void newOutputLines(int);
    void newErrorLines(int);
    void newOutputData(KProcess *, QByteArray &);

private:
    QByteArray errorData;
    QByteArray outputData;

    bool mergedOutput;
};

#endif // KRLINECOUNTINGPROCESS_H
