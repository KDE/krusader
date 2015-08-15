/***************************************************************************
                           krlinecountingprocess.h
                           -----------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
