#include "kr7zencryptionchecker.h"

Kr7zEncryptionChecker::Kr7zEncryptionChecker() : KProcess(), encrypted(false), lastData()
{
    setOutputChannelMode(KProcess::SeparateChannels); // without this output redirection has no effect!
    connect(this, SIGNAL(readyReadStandardOutput()), SLOT(receivedOutput()));
}

void Kr7zEncryptionChecker::setupChildProcess()
{
    // This function is called after the fork but for the exec. We create a process group
    // to work around a broken wrapper script of 7z. Without this only the wrapper is killed.
    setsid(); // make this process leader of a new process group
}

void Kr7zEncryptionChecker::receivedOutput()
{
    QString data =  QString::fromLocal8Bit(this->readAllStandardOutput());

    QString checkable = lastData + data;

    QStringList lines = checkable.split('\n');
    lastData = lines[ lines.count() - 1 ];
    for (int i = 0; i != lines.count(); i++) {
        QString line = lines[ i ].trimmed().toLower();
        int ndx = line.indexOf("testing");
        if (ndx >= 0)
            line.truncate(ndx);
        if (line.isEmpty())
            continue;

        if (line.contains("password") && line.contains("enter")) {
            encrypted = true;
            ::kill(- pid(), SIGKILL); // kill the whole process group by giving the negative PID
        }
    }
}

bool Kr7zEncryptionChecker::isEncrypted()
{
    return encrypted;
}
