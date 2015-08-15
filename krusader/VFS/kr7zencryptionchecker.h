#ifndef KR7ZENCRYPTIONCHECKER_H
#define KR7ZENCRYPTIONCHECKER_H

#include <unistd.h> // for setsid, see Kr7zEncryptionChecker::setupChildProcess
#include <signal.h> // for kill

#include <KCoreAddons/KProcess>

#include "../krservices.h"

class Kr7zEncryptionChecker : public KProcess
{
    Q_OBJECT

public:
    Kr7zEncryptionChecker();

protected:
    virtual void setupChildProcess() Q_DECL_OVERRIDE;

public slots:
    void receivedOutput();
    bool isEncrypted();

private:
    QString fileName;
    bool encrypted;
    QString lastData;
};

#endif // KR7ZENCRYPTIONCHECKER_H
