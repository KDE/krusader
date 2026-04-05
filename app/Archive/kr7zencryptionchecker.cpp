/*
    SPDX-FileCopyrightText: 2001 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kr7zencryptionchecker.h"

Kr7zEncryptionChecker::Kr7zEncryptionChecker()
    : encrypted(false)
    , lastData()
{
    setOutputChannelMode(KProcess::SeparateChannels); // without this output redirection has no effect!
    this->setChildProcessModifier([]() noexcept {
        // This function is called after the fork but for the exec. We create a process group
        // to work around a broken wrapper script of 7z. Without this only the wrapper is killed.
        setsid(); // make this process leader of a new process group
    });
    connect(this, &Kr7zEncryptionChecker::readyReadStandardOutput, this, [=]() {
        receivedOutput();
    });
}

void Kr7zEncryptionChecker::receivedOutput()
{
    // Reminder: If that function is modified, it's important to research if the
    // changes must also be applied to `kio_krarcProtocol::check7zOutputForPassword()`

    QString data = QString::fromLocal8Bit(this->readAllStandardOutput());

    QString checkable = lastData + data;

    QStringList lines = checkable.split('\n');
    lastData = lines[lines.count() - 1];
    for (QString &line : lines) {
        line = line.trimmed();
        if (line.isEmpty())
            continue;

        // If it seems a 7z archive with an encrypted header (then, e.g. a file name can't be seen without the password) or
        // if it seems a 7z archive without an encrypted header (although the content of its encrypted files can't be seen
        // without the password, the file name (and other data) can be seen)
        if ((line.startsWith("Enter password") && line.endsWith(":")) || line == QStringLiteral("Encrypted = +")) {
            encrypted = true;
            ::kill(static_cast<pid_t>(-processId()), SIGKILL); // kill the whole process group by giving the negative PID
            break;
        }
    }
}

bool Kr7zEncryptionChecker::isEncrypted()
{
    return encrypted;
}
