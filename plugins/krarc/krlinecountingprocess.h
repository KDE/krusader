/*
    SPDX-FileCopyrightText: 2001 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRLINECOUNTINGPROCESS_H
#define KRLINECOUNTINGPROCESS_H

#include <KProcess>

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
    void newOutputLines(qsizetype);
    void newErrorLines(qsizetype);
    void newOutputData(KProcess *, QByteArray &);

private:
    QByteArray errorData;
    QByteArray outputData;

    bool mergedOutput;
};

#endif // KRLINECOUNTINGPROCESS_H
