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

#include <qvariant.h>
#include <qdialog.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <QEvent>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>
class Q3VBoxLayout; 
class Q3HBoxLayout; 
class Q3GridLayout; 
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class KComboBox;
class KHistoryCombo;

class newFTPGUI : public QDialog {
    Q_OBJECT
public:
    newFTPGUI( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~newFTPGUI();

    QLabel* TextLabel1;
    KComboBox* prefix;
    QLabel* TextLabel1_2_2;
	 QLabel* TextLabel1_22;
    QLabel* TextLabel1_2;
    QLabel* TextLabel1_3;
    QSpinBox* port;
    QLineEdit* password;
    QPushButton* connectBtn;
    QPushButton* saveBtn;
    QPushButton* cancelBtn;
    QLabel* PixmapLabel1;
    QLabel* TextLabel3;
    QLineEdit* username;
    KHistoryCombo* url;

public slots:
    void slotTextChanged(const QString& string);

protected:
    Q3HBoxLayout* hbox;
    bool event( QEvent* );
};

#endif // NEWFTPGUI_H
