/****************************************************************************
** Form interface generated from reading ui file 'newftpgui.ui'
**
** Created: Fri Oct 27 23:47:08 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef NEWFTPGUI_H
#define NEWFTPGUI_H

#include <QtCore/QVariant>
#include <QtCore/QEvent>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QDialog>

#include <KLineEdit>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QLabel;
class QPushButton;
class QSpinBox;
class KComboBox;
class KHistoryComboBox;
class KLineEdit;

class newFTPGUI : public QDialog
{
    Q_OBJECT

public:
    newFTPGUI(QWidget* parent = 0);
    ~newFTPGUI();

    QLabel* TextLabel1;
    KComboBox* prefix;
    QLabel* TextLabel1_2_2;
    QLabel* TextLabel1_22;
    QLabel* TextLabel1_2;
    QLabel* TextLabel1_3;
    QSpinBox* port;
    KLineEdit* password;
    QPushButton* connectBtn;
    QPushButton* saveBtn;
    QPushButton* cancelBtn;
    QLabel* PixmapLabel1;
    QLabel* TextLabel3;
    KLineEdit* username;
    KHistoryComboBox* url;

public slots:
    void slotTextChanged(const QString& string);

protected:
    QHBoxLayout* hbox;
    bool event(QEvent*);
};

#endif // NEWFTPGUI_H
