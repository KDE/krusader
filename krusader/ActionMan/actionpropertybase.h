/********************************************************************************
** Form generated from reading ui file 'actionpropertybase.ui'
**
** Created: Fri Sep 14 23:54:20 2007
**      by: Qt User Interface Compiler version 4.3.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef ACTIONPROPERTYBASE_H
#define ACTIONPROPERTYBASE_H

#include <Qt3Support/Q3ButtonGroup>
#include <Qt3Support/Q3GroupBox>
#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTabWidget>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "k3listbox.h"
#include "kcombobox.h"
#include "kicondialog.h"
#include "kkeysequencewidget.h"
#include "klineedit.h"
#include "ktextedit.h"

class Ui_ActionPropertyBase: public QWidget
{
public:
    QGridLayout *gridLayout;
    QTabWidget *tabWidget3;
    QWidget *tab;
    QGridLayout *gridLayout1;
    QToolButton *ButtonAddStartpath;
    QLabel *LabelDescription;
    Q3ButtonGroup *bgAccept;
    QGridLayout *gridLayout2;
    QRadioButton *radioLocal;
    QRadioButton *radioUrl;
    KLineEdit *leTitle;
    QLabel *LabelTitle;
    QHBoxLayout *hboxLayout;
    QVBoxLayout *vboxLayout;
    KLineEdit *leDistinctName;
    KComboBox *cbCategory;
    KIconButton *ButtonIcon;
    QLabel *LabelDistinctName;
    QLabel *LabelCommandline;
    KLineEdit *leTooltip;
    KLineEdit *leStartpath;
    QLabel *LabelTooltip;
    KLineEdit *leCommandline;
    QLabel *LabelCategory;
    QToolButton *ButtonAddPlaceholder;
    KTextEdit *textDescription;
    QLabel *LabelStartpath;
    QSpacerItem *spacerItem;
    QHBoxLayout *hboxLayout1;
    QLabel *LabelShortcut;
    QSpacerItem *spacerItem1;
    KKeySequenceWidget *KeyButtonShortcut;
    Q3ButtonGroup *bgExecType;
    QGridLayout *gridLayout3;
    QRadioButton *radioCollectOutput;
    QCheckBox *chkSeparateStdError;
    QRadioButton *radioNormal;
    QRadioButton *radioTerminal;
    QWidget *tab1;
    QGridLayout *gridLayout4;
    Q3GroupBox *gbShowonly;
    QGridLayout *gridLayout5;
    QTabWidget *tabShowonly;
    QWidget *TabPage;
    QGridLayout *gridLayout6;
    QToolButton *ButtonNewProtocol;
    QToolButton *ButtonEditProtocol;
    QSpacerItem *spacerItem2;
    QToolButton *ButtonRemoveProtocol;
    K3ListBox *lbShowonlyProtocol;
    QWidget *tab2;
    QGridLayout *gridLayout7;
    K3ListBox *lbShowonlyPath;
    QToolButton *ButtonAddPath;
    QToolButton *ButtonEditPath;
    QToolButton *ButtonRemovePath;
    QSpacerItem *spacerItem3;
    QWidget *tab3;
    QGridLayout *gridLayout8;
    K3ListBox *lbShowonlyMime;
    QToolButton *ButtonAddMime;
    QToolButton *ButtonEditMime;
    QToolButton *ButtonRemoveMime;
    QSpacerItem *spacerItem4;
    QWidget *TabPage1;
    QGridLayout *gridLayout9;
    K3ListBox *lbShowonlyFile;
    QToolButton *ButtonNewFile;
    QToolButton *ButtonEditFile;
    QToolButton *ButtonRemoveFile;
    QSpacerItem *spacerItem5;
    QCheckBox *chkConfirmExecution;
    QCheckBox *chkDifferentUser;
    KLineEdit *leDifferentUser;
    QSpacerItem *spacerItem6;

	Ui_ActionPropertyBase(QWidget *parent, const char *name): QWidget(parent, name) {}

    void setupUi(QWidget *ActionPropertyBase)
    {
    if (ActionPropertyBase->objectName().isEmpty())
        ActionPropertyBase->setObjectName(QString::fromUtf8("ActionPropertyBase"));
    QSize size(485, 497);
    size = size.expandedTo(ActionPropertyBase->minimumSizeHint());
    ActionPropertyBase->resize(size);
    gridLayout = new QGridLayout(ActionPropertyBase);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(11);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    gridLayout->setHorizontalSpacing(0);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    tabWidget3 = new QTabWidget(ActionPropertyBase);
    tabWidget3->setObjectName(QString::fromUtf8("tabWidget3"));
    tab = new QWidget();
    tab->setObjectName(QString::fromUtf8("tab"));
    gridLayout1 = new QGridLayout(tab);
    gridLayout1->setSpacing(6);
    gridLayout1->setMargin(11);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    ButtonAddStartpath = new QToolButton(tab);
    ButtonAddStartpath->setObjectName(QString::fromUtf8("ButtonAddStartpath"));

    gridLayout1->addWidget(ButtonAddStartpath, 8, 3, 1, 1);

    LabelDescription = new QLabel(tab);
    LabelDescription->setObjectName(QString::fromUtf8("LabelDescription"));
    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(LabelDescription->sizePolicy().hasHeightForWidth());
    LabelDescription->setSizePolicy(sizePolicy);
    LabelDescription->setWordWrap(false);

    gridLayout1->addWidget(LabelDescription, 4, 0, 1, 1);

    bgAccept = new Q3ButtonGroup(tab);
    bgAccept->setObjectName(QString::fromUtf8("bgAccept"));
    QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Minimum);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(bgAccept->sizePolicy().hasHeightForWidth());
    bgAccept->setSizePolicy(sizePolicy1);
    bgAccept->setColumnLayout(0, Qt::Vertical);
    bgAccept->layout()->setSpacing(6);
    bgAccept->layout()->setMargin(11);
    gridLayout2 = new QGridLayout();
    QBoxLayout *boxlayout = qobject_cast<QBoxLayout *>(bgAccept->layout());
    if (boxlayout)
        boxlayout->addLayout(gridLayout2);
    gridLayout2->setAlignment(Qt::AlignTop);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    radioLocal = new QRadioButton(bgAccept);
    radioLocal->setObjectName(QString::fromUtf8("radioLocal"));
    radioLocal->setChecked(true);

    gridLayout2->addWidget(radioLocal, 0, 0, 1, 1);

    radioUrl = new QRadioButton(bgAccept);
    radioUrl->setObjectName(QString::fromUtf8("radioUrl"));

    gridLayout2->addWidget(radioUrl, 1, 0, 1, 1);


    gridLayout1->addWidget(bgAccept, 9, 2, 1, 2);

    leTitle = new KLineEdit(tab);
    leTitle->setObjectName(QString::fromUtf8("leTitle"));

    gridLayout1->addWidget(leTitle, 2, 1, 1, 3);

    LabelTitle = new QLabel(tab);
    LabelTitle->setObjectName(QString::fromUtf8("LabelTitle"));
    QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Fixed);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(LabelTitle->sizePolicy().hasHeightForWidth());
    LabelTitle->setSizePolicy(sizePolicy2);
    LabelTitle->setWordWrap(false);

    gridLayout1->addWidget(LabelTitle, 2, 0, 1, 1);

    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    vboxLayout = new QVBoxLayout();
    vboxLayout->setSpacing(6);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    leDistinctName = new KLineEdit(tab);
    leDistinctName->setObjectName(QString::fromUtf8("leDistinctName"));

    vboxLayout->addWidget(leDistinctName);

    cbCategory = new KComboBox(tab);
    cbCategory->setObjectName(QString::fromUtf8("cbCategory"));
    cbCategory->setEditable(true);

    vboxLayout->addWidget(cbCategory);


    hboxLayout->addLayout(vboxLayout);

    ButtonIcon = new KIconButton(tab);
    ButtonIcon->setObjectName(QString::fromUtf8("ButtonIcon"));
    QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(ButtonIcon->sizePolicy().hasHeightForWidth());
    ButtonIcon->setSizePolicy(sizePolicy3);
    ButtonIcon->setMinimumSize(QSize(50, 50));
    ButtonIcon->setMaximumSize(QSize(50, 50));

    hboxLayout->addWidget(ButtonIcon);


    gridLayout1->addLayout(hboxLayout, 0, 1, 2, 3);

    LabelDistinctName = new QLabel(tab);
    LabelDistinctName->setObjectName(QString::fromUtf8("LabelDistinctName"));
    sizePolicy2.setHeightForWidth(LabelDistinctName->sizePolicy().hasHeightForWidth());
    LabelDistinctName->setSizePolicy(sizePolicy2);
    LabelDistinctName->setWordWrap(false);

    gridLayout1->addWidget(LabelDistinctName, 0, 0, 1, 1);

    LabelCommandline = new QLabel(tab);
    LabelCommandline->setObjectName(QString::fromUtf8("LabelCommandline"));
    sizePolicy3.setHeightForWidth(LabelCommandline->sizePolicy().hasHeightForWidth());
    LabelCommandline->setSizePolicy(sizePolicy3);
    LabelCommandline->setWordWrap(false);

    gridLayout1->addWidget(LabelCommandline, 7, 0, 1, 1);

    leTooltip = new KLineEdit(tab);
    leTooltip->setObjectName(QString::fromUtf8("leTooltip"));

    gridLayout1->addWidget(leTooltip, 3, 1, 1, 3);

    leStartpath = new KLineEdit(tab);
    leStartpath->setObjectName(QString::fromUtf8("leStartpath"));

    gridLayout1->addWidget(leStartpath, 8, 1, 1, 2);

    LabelTooltip = new QLabel(tab);
    LabelTooltip->setObjectName(QString::fromUtf8("LabelTooltip"));
    sizePolicy2.setHeightForWidth(LabelTooltip->sizePolicy().hasHeightForWidth());
    LabelTooltip->setSizePolicy(sizePolicy2);
    LabelTooltip->setWordWrap(false);

    gridLayout1->addWidget(LabelTooltip, 3, 0, 1, 1);

    leCommandline = new KLineEdit(tab);
    leCommandline->setObjectName(QString::fromUtf8("leCommandline"));

    gridLayout1->addWidget(leCommandline, 7, 1, 1, 2);

    LabelCategory = new QLabel(tab);
    LabelCategory->setObjectName(QString::fromUtf8("LabelCategory"));
    sizePolicy2.setHeightForWidth(LabelCategory->sizePolicy().hasHeightForWidth());
    LabelCategory->setSizePolicy(sizePolicy2);
    LabelCategory->setWordWrap(false);

    gridLayout1->addWidget(LabelCategory, 1, 0, 1, 1);

    ButtonAddPlaceholder = new QToolButton(tab);
    ButtonAddPlaceholder->setObjectName(QString::fromUtf8("ButtonAddPlaceholder"));
    sizePolicy3.setHeightForWidth(ButtonAddPlaceholder->sizePolicy().hasHeightForWidth());
    ButtonAddPlaceholder->setSizePolicy(sizePolicy3);
    ButtonAddPlaceholder->setMinimumSize(QSize(0, 0));

    gridLayout1->addWidget(ButtonAddPlaceholder, 7, 3, 1, 1);

    textDescription = new KTextEdit(tab);
    textDescription->setObjectName(QString::fromUtf8("textDescription"));

    gridLayout1->addWidget(textDescription, 4, 1, 3, 3);

    LabelStartpath = new QLabel(tab);
    LabelStartpath->setObjectName(QString::fromUtf8("LabelStartpath"));
    sizePolicy3.setHeightForWidth(LabelStartpath->sizePolicy().hasHeightForWidth());
    LabelStartpath->setSizePolicy(sizePolicy3);
    LabelStartpath->setWordWrap(false);

    gridLayout1->addWidget(LabelStartpath, 8, 0, 1, 1);

    spacerItem = new QSpacerItem(80, 19, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout1->addItem(spacerItem, 6, 0, 1, 1);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    LabelShortcut = new QLabel(tab);
    LabelShortcut->setObjectName(QString::fromUtf8("LabelShortcut"));
    LabelShortcut->setWordWrap(false);

    hboxLayout1->addWidget(LabelShortcut);

    spacerItem1 = new QSpacerItem(161, 21, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout1->addItem(spacerItem1);

    KeyButtonShortcut = new KKeySequenceWidget(tab);
    KeyButtonShortcut->setObjectName(QString::fromUtf8("KeyButtonShortcut"));

    hboxLayout1->addWidget(KeyButtonShortcut);


    gridLayout1->addLayout(hboxLayout1, 10, 2, 1, 2);

    bgExecType = new Q3ButtonGroup(tab);
    bgExecType->setObjectName(QString::fromUtf8("bgExecType"));
    sizePolicy1.setHeightForWidth(bgExecType->sizePolicy().hasHeightForWidth());
    bgExecType->setSizePolicy(sizePolicy1);
    bgExecType->setColumnLayout(0, Qt::Vertical);
    bgExecType->layout()->setSpacing(6);
    bgExecType->layout()->setMargin(11);
    gridLayout3 = new QGridLayout();
    QBoxLayout *boxlayout1 = qobject_cast<QBoxLayout *>(bgExecType->layout());
    if (boxlayout1)
        boxlayout1->addLayout(gridLayout3);
    gridLayout3->setAlignment(Qt::AlignTop);
    gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
    radioCollectOutput = new QRadioButton(bgExecType);
    radioCollectOutput->setObjectName(QString::fromUtf8("radioCollectOutput"));

    gridLayout3->addWidget(radioCollectOutput, 2, 0, 1, 1);

    chkSeparateStdError = new QCheckBox(bgExecType);
    chkSeparateStdError->setObjectName(QString::fromUtf8("chkSeparateStdError"));
    chkSeparateStdError->setEnabled(false);

    gridLayout3->addWidget(chkSeparateStdError, 3, 0, 1, 1);

    radioNormal = new QRadioButton(bgExecType);
    radioNormal->setObjectName(QString::fromUtf8("radioNormal"));
    radioNormal->setChecked(true);

    gridLayout3->addWidget(radioNormal, 0, 0, 1, 1);

    radioTerminal = new QRadioButton(bgExecType);
    radioTerminal->setObjectName(QString::fromUtf8("radioTerminal"));

    gridLayout3->addWidget(radioTerminal, 1, 0, 1, 1);


    gridLayout1->addWidget(bgExecType, 9, 0, 2, 2);

    tabWidget3->addTab(tab, QString());
    tab1 = new QWidget();
    tab1->setObjectName(QString::fromUtf8("tab1"));
    gridLayout4 = new QGridLayout(tab1);
    gridLayout4->setSpacing(6);
    gridLayout4->setMargin(11);
    gridLayout4->setObjectName(QString::fromUtf8("gridLayout4"));
    gbShowonly = new Q3GroupBox(tab1);
    gbShowonly->setObjectName(QString::fromUtf8("gbShowonly"));
    QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(gbShowonly->sizePolicy().hasHeightForWidth());
    gbShowonly->setSizePolicy(sizePolicy4);
    gbShowonly->setColumnLayout(0, Qt::Vertical);
    gbShowonly->layout()->setSpacing(6);
    gbShowonly->layout()->setMargin(11);
    gridLayout5 = new QGridLayout();
    QBoxLayout *boxlayout2 = qobject_cast<QBoxLayout *>(gbShowonly->layout());
    if (boxlayout2)
        boxlayout2->addLayout(gridLayout5);
    gridLayout5->setAlignment(Qt::AlignTop);
    gridLayout5->setObjectName(QString::fromUtf8("gridLayout5"));
    tabShowonly = new QTabWidget(gbShowonly);
    tabShowonly->setObjectName(QString::fromUtf8("tabShowonly"));
    tabShowonly->setTabShape(QTabWidget::Triangular);
    TabPage = new QWidget();
    TabPage->setObjectName(QString::fromUtf8("TabPage"));
    gridLayout6 = new QGridLayout(TabPage);
    gridLayout6->setSpacing(6);
    gridLayout6->setMargin(11);
    gridLayout6->setObjectName(QString::fromUtf8("gridLayout6"));
    ButtonNewProtocol = new QToolButton(TabPage);
    ButtonNewProtocol->setObjectName(QString::fromUtf8("ButtonNewProtocol"));
    sizePolicy2.setHeightForWidth(ButtonNewProtocol->sizePolicy().hasHeightForWidth());
    ButtonNewProtocol->setSizePolicy(sizePolicy2);
    ButtonNewProtocol->setMinimumSize(QSize(0, 0));
    ButtonNewProtocol->setMaximumSize(QSize(32767, 32767));

    gridLayout6->addWidget(ButtonNewProtocol, 0, 1, 1, 1);

    ButtonEditProtocol = new QToolButton(TabPage);
    ButtonEditProtocol->setObjectName(QString::fromUtf8("ButtonEditProtocol"));
    sizePolicy2.setHeightForWidth(ButtonEditProtocol->sizePolicy().hasHeightForWidth());
    ButtonEditProtocol->setSizePolicy(sizePolicy2);
    ButtonEditProtocol->setMinimumSize(QSize(0, 0));
    ButtonEditProtocol->setMaximumSize(QSize(32767, 32767));

    gridLayout6->addWidget(ButtonEditProtocol, 1, 1, 1, 1);

    spacerItem2 = new QSpacerItem(21, 58, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout6->addItem(spacerItem2, 3, 1, 1, 1);

    ButtonRemoveProtocol = new QToolButton(TabPage);
    ButtonRemoveProtocol->setObjectName(QString::fromUtf8("ButtonRemoveProtocol"));
    sizePolicy2.setHeightForWidth(ButtonRemoveProtocol->sizePolicy().hasHeightForWidth());
    ButtonRemoveProtocol->setSizePolicy(sizePolicy2);
    ButtonRemoveProtocol->setMinimumSize(QSize(0, 0));
    ButtonRemoveProtocol->setMaximumSize(QSize(32767, 32767));

    gridLayout6->addWidget(ButtonRemoveProtocol, 2, 1, 1, 1);

    lbShowonlyProtocol = new K3ListBox(TabPage);
    lbShowonlyProtocol->setObjectName(QString::fromUtf8("lbShowonlyProtocol"));

    gridLayout6->addWidget(lbShowonlyProtocol, 0, 0, 4, 1);

    tabShowonly->addTab(TabPage, QString());
    tab2 = new QWidget();
    tab2->setObjectName(QString::fromUtf8("tab2"));
    gridLayout7 = new QGridLayout(tab2);
    gridLayout7->setSpacing(6);
    gridLayout7->setMargin(11);
    gridLayout7->setObjectName(QString::fromUtf8("gridLayout7"));
    lbShowonlyPath = new K3ListBox(tab2);
    lbShowonlyPath->setObjectName(QString::fromUtf8("lbShowonlyPath"));

    gridLayout7->addWidget(lbShowonlyPath, 0, 0, 4, 1);

    ButtonAddPath = new QToolButton(tab2);
    ButtonAddPath->setObjectName(QString::fromUtf8("ButtonAddPath"));
    sizePolicy2.setHeightForWidth(ButtonAddPath->sizePolicy().hasHeightForWidth());
    ButtonAddPath->setSizePolicy(sizePolicy2);
    ButtonAddPath->setMinimumSize(QSize(0, 0));
    ButtonAddPath->setMaximumSize(QSize(32767, 32767));

    gridLayout7->addWidget(ButtonAddPath, 0, 1, 1, 1);

    ButtonEditPath = new QToolButton(tab2);
    ButtonEditPath->setObjectName(QString::fromUtf8("ButtonEditPath"));
    sizePolicy2.setHeightForWidth(ButtonEditPath->sizePolicy().hasHeightForWidth());
    ButtonEditPath->setSizePolicy(sizePolicy2);
    ButtonEditPath->setMinimumSize(QSize(0, 0));
    ButtonEditPath->setMaximumSize(QSize(32767, 32767));

    gridLayout7->addWidget(ButtonEditPath, 1, 1, 1, 1);

    ButtonRemovePath = new QToolButton(tab2);
    ButtonRemovePath->setObjectName(QString::fromUtf8("ButtonRemovePath"));
    sizePolicy2.setHeightForWidth(ButtonRemovePath->sizePolicy().hasHeightForWidth());
    ButtonRemovePath->setSizePolicy(sizePolicy2);
    ButtonRemovePath->setMinimumSize(QSize(0, 0));
    ButtonRemovePath->setMaximumSize(QSize(32767, 32767));

    gridLayout7->addWidget(ButtonRemovePath, 2, 1, 1, 1);

    spacerItem3 = new QSpacerItem(21, 61, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout7->addItem(spacerItem3, 3, 1, 1, 1);

    tabShowonly->addTab(tab2, QString());
    tab3 = new QWidget();
    tab3->setObjectName(QString::fromUtf8("tab3"));
    gridLayout8 = new QGridLayout(tab3);
    gridLayout8->setSpacing(6);
    gridLayout8->setMargin(11);
    gridLayout8->setObjectName(QString::fromUtf8("gridLayout8"));
    lbShowonlyMime = new K3ListBox(tab3);
    lbShowonlyMime->setObjectName(QString::fromUtf8("lbShowonlyMime"));

    gridLayout8->addWidget(lbShowonlyMime, 0, 0, 4, 1);

    ButtonAddMime = new QToolButton(tab3);
    ButtonAddMime->setObjectName(QString::fromUtf8("ButtonAddMime"));
    sizePolicy2.setHeightForWidth(ButtonAddMime->sizePolicy().hasHeightForWidth());
    ButtonAddMime->setSizePolicy(sizePolicy2);
    ButtonAddMime->setMinimumSize(QSize(0, 0));
    ButtonAddMime->setMaximumSize(QSize(32767, 32767));

    gridLayout8->addWidget(ButtonAddMime, 0, 1, 1, 1);

    ButtonEditMime = new QToolButton(tab3);
    ButtonEditMime->setObjectName(QString::fromUtf8("ButtonEditMime"));
    sizePolicy2.setHeightForWidth(ButtonEditMime->sizePolicy().hasHeightForWidth());
    ButtonEditMime->setSizePolicy(sizePolicy2);
    ButtonEditMime->setMinimumSize(QSize(0, 0));
    ButtonEditMime->setMaximumSize(QSize(32767, 32767));

    gridLayout8->addWidget(ButtonEditMime, 1, 1, 1, 1);

    ButtonRemoveMime = new QToolButton(tab3);
    ButtonRemoveMime->setObjectName(QString::fromUtf8("ButtonRemoveMime"));
    sizePolicy2.setHeightForWidth(ButtonRemoveMime->sizePolicy().hasHeightForWidth());
    ButtonRemoveMime->setSizePolicy(sizePolicy2);
    ButtonRemoveMime->setMinimumSize(QSize(0, 0));
    ButtonRemoveMime->setMaximumSize(QSize(32767, 32767));

    gridLayout8->addWidget(ButtonRemoveMime, 2, 1, 1, 1);

    spacerItem4 = new QSpacerItem(21, 41, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout8->addItem(spacerItem4, 3, 1, 1, 1);

    tabShowonly->addTab(tab3, QString());
    TabPage1 = new QWidget();
    TabPage1->setObjectName(QString::fromUtf8("TabPage1"));
    gridLayout9 = new QGridLayout(TabPage1);
    gridLayout9->setSpacing(6);
    gridLayout9->setMargin(11);
    gridLayout9->setObjectName(QString::fromUtf8("gridLayout9"));
    lbShowonlyFile = new K3ListBox(TabPage1);
    lbShowonlyFile->setObjectName(QString::fromUtf8("lbShowonlyFile"));

    gridLayout9->addWidget(lbShowonlyFile, 0, 0, 4, 1);

    ButtonNewFile = new QToolButton(TabPage1);
    ButtonNewFile->setObjectName(QString::fromUtf8("ButtonNewFile"));
    sizePolicy2.setHeightForWidth(ButtonNewFile->sizePolicy().hasHeightForWidth());
    ButtonNewFile->setSizePolicy(sizePolicy2);
    ButtonNewFile->setMinimumSize(QSize(0, 0));
    ButtonNewFile->setMaximumSize(QSize(32767, 32767));

    gridLayout9->addWidget(ButtonNewFile, 0, 1, 1, 1);

    ButtonEditFile = new QToolButton(TabPage1);
    ButtonEditFile->setObjectName(QString::fromUtf8("ButtonEditFile"));
    sizePolicy2.setHeightForWidth(ButtonEditFile->sizePolicy().hasHeightForWidth());
    ButtonEditFile->setSizePolicy(sizePolicy2);
    ButtonEditFile->setMinimumSize(QSize(0, 0));
    ButtonEditFile->setMaximumSize(QSize(32767, 32767));

    gridLayout9->addWidget(ButtonEditFile, 1, 1, 1, 1);

    ButtonRemoveFile = new QToolButton(TabPage1);
    ButtonRemoveFile->setObjectName(QString::fromUtf8("ButtonRemoveFile"));
    sizePolicy2.setHeightForWidth(ButtonRemoveFile->sizePolicy().hasHeightForWidth());
    ButtonRemoveFile->setSizePolicy(sizePolicy2);
    ButtonRemoveFile->setMinimumSize(QSize(0, 0));
    ButtonRemoveFile->setMaximumSize(QSize(32767, 32767));

    gridLayout9->addWidget(ButtonRemoveFile, 2, 1, 1, 1);

    spacerItem5 = new QSpacerItem(21, 41, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout9->addItem(spacerItem5, 3, 1, 1, 1);

    tabShowonly->addTab(TabPage1, QString());

    gridLayout5->addWidget(tabShowonly, 0, 0, 1, 1);


    gridLayout4->addWidget(gbShowonly, 0, 0, 1, 2);

    chkConfirmExecution = new QCheckBox(tab1);
    chkConfirmExecution->setObjectName(QString::fromUtf8("chkConfirmExecution"));

    gridLayout4->addWidget(chkConfirmExecution, 1, 0, 1, 2);

    chkDifferentUser = new QCheckBox(tab1);
    chkDifferentUser->setObjectName(QString::fromUtf8("chkDifferentUser"));

    gridLayout4->addWidget(chkDifferentUser, 2, 0, 1, 1);

    leDifferentUser = new KLineEdit(tab1);
    leDifferentUser->setObjectName(QString::fromUtf8("leDifferentUser"));
    leDifferentUser->setEnabled(false);

    gridLayout4->addWidget(leDifferentUser, 2, 1, 1, 1);

    spacerItem6 = new QSpacerItem(161, 102, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout4->addItem(spacerItem6, 3, 0, 1, 2);

    tabWidget3->addTab(tab1, QString());

    gridLayout->addWidget(tabWidget3, 0, 0, 1, 1);


    retranslateUi(ActionPropertyBase);
    QObject::connect(radioCollectOutput, SIGNAL(toggled(bool)), chkSeparateStdError, SLOT(setEnabled(bool)));
    QObject::connect(chkDifferentUser, SIGNAL(toggled(bool)), leDifferentUser, SLOT(setEnabled(bool)));

    QMetaObject::connectSlotsByName(ActionPropertyBase);
    } // setupUi

    void retranslateUi(QWidget *ActionPropertyBase)
    {
    ActionPropertyBase->setWindowTitle(QApplication::translate("ActionPropertyBase", "Action Property", 0, QApplication::UnicodeUTF8));
    ButtonAddStartpath->setText(QApplication::translate("ActionPropertyBase", "...", 0, QApplication::UnicodeUTF8));
    LabelDescription->setWhatsThis(QApplication::translate("ActionPropertyBase", "A detailed description of the <b>Useraction</b>. It is only displayed in the <i>Konfigurator</i> and via <code>Shift-F1</code>.", 0, QApplication::UnicodeUTF8));
    LabelDescription->setText(QApplication::translate("ActionPropertyBase", "Description:", 0, QApplication::UnicodeUTF8));
    bgAccept->setTitle(QApplication::translate("ActionPropertyBase", "Command accepts", 0, QApplication::UnicodeUTF8));
    radioLocal->setWhatsThis(QApplication::translate("ActionPropertyBase", "Substitute the <b>Placeholders</b> with local filenames.", 0, QApplication::UnicodeUTF8));
    radioLocal->setText(QApplication::translate("ActionPropertyBase", "Local files only (no URL's)", 0, QApplication::UnicodeUTF8));
    radioUrl->setWhatsThis(QApplication::translate("ActionPropertyBase", "Substitute the <b>Placeholders</b> with valid URL's.", 0, QApplication::UnicodeUTF8));
    radioUrl->setText(QApplication::translate("ActionPropertyBase", "URL's (remote and local)", 0, QApplication::UnicodeUTF8));
    leTitle->setWhatsThis(QApplication::translate("ActionPropertyBase", "The title displayed in the <b>Usermenu</b>.", 0, QApplication::UnicodeUTF8));
    LabelTitle->setWhatsThis(QApplication::translate("ActionPropertyBase", "The title displayed in the <b>Usermenu</b>.", 0, QApplication::UnicodeUTF8));
    LabelTitle->setText(QApplication::translate("ActionPropertyBase", "Title:", 0, QApplication::UnicodeUTF8));
    leDistinctName->setWhatsThis(QApplication::translate("ActionPropertyBase", "Unique name of the <b>Useraction</b>. It is only used in the <i>Konfigurator</i> and doesn't appear in any other menu.<p><b>Note</b>: The <i>Title</i> shown in the <b>Usermenu</b> can be set below.", 0, QApplication::UnicodeUTF8));
    cbCategory->setWhatsThis(QApplication::translate("ActionPropertyBase", "<b>Useractions</b> can be grouped in categories for better distinction. Choose a existing <i>Category</i> or create a new one by entering a name.", 0, QApplication::UnicodeUTF8));
    ButtonIcon->setWhatsThis(QApplication::translate("ActionPropertyBase", "Each <b>Useraction</b> can have its own icon. It will appear in front of the title in the <b>Usermenu</b>.", 0, QApplication::UnicodeUTF8));
    ButtonIcon->setText(QApplication::translate("ActionPropertyBase", "Icon", 0, QApplication::UnicodeUTF8));
    LabelDistinctName->setWhatsThis(QApplication::translate("ActionPropertyBase", "<p>Unique name of the <b>Useraction</b>. It is only used in the <i>Konfigurator</i> and doesn't appear in any other menu.</p><p><b>Note</b>: The <i>Title</i> shown in the <b>Usermenu</b> can be set below.</p>", 0, QApplication::UnicodeUTF8));
    LabelDistinctName->setText(QApplication::translate("ActionPropertyBase", "Identifier:", 0, QApplication::UnicodeUTF8));
    LabelCommandline->setWhatsThis(QApplication::translate("ActionPropertyBase", "<p>The <i>Command</i> defines the command that will be executed when the <b>Useraction</b> is used. It can be a simple shell command or a complex sequence of multiple commands with <b>Placeholders</b>.</p><p>Examples:<ul><code><li>eject /mnt/cdrom</li><li>amarok --append %aList(\"Selected\")%</li></code></ul>\n"
"Please consult the handbook to learn more about the syntax.</p>", 0, QApplication::UnicodeUTF8));
    LabelCommandline->setText(QApplication::translate("ActionPropertyBase", "Command:", 0, QApplication::UnicodeUTF8));
    leTooltip->setWhatsThis(QApplication::translate("ActionPropertyBase", "The <i>Tooltip</i> is shown when the mouse cursor is hold over an entry of the <b>Useraction Toolbar</b>.", 0, QApplication::UnicodeUTF8));
    leStartpath->setWhatsThis(QApplication::translate("ActionPropertyBase", "The <i>Workdir</i> defines in which directory the <i>Command</i> will be executed.", 0, QApplication::UnicodeUTF8));
    LabelTooltip->setWhatsThis(QApplication::translate("ActionPropertyBase", "The <i>Tooltip</i> is shown when the mouse cursor is hold over an entry of the <b>Useraction Toolbar</b>.", 0, QApplication::UnicodeUTF8));
    LabelTooltip->setText(QApplication::translate("ActionPropertyBase", "Tooltip:", 0, QApplication::UnicodeUTF8));
    leCommandline->setWhatsThis(QApplication::translate("ActionPropertyBase", "The <i>Command</i> defines the command that will be executed when the <b>Useraction</b> is used. It can be a simple shell command or a complex sequence of multiple commands with <b>Placeholders</b>.<p>\n"
"Examples:<ul><code><li>eject /mnt/cdrom</li><li>amarok --append %aList(\"Selected\")%</li></code></ul>\n"
"Please consult the handbook to learn more about the syntax.", 0, QApplication::UnicodeUTF8));
    leCommandline->setText(QString());
    LabelCategory->setWhatsThis(QApplication::translate("ActionPropertyBase", "<b>Useractions</b> can be grouped in categories for better distinction. Choose a existing <i>Category</i> or create a new one by entering a name.", 0, QApplication::UnicodeUTF8));
    LabelCategory->setText(QApplication::translate("ActionPropertyBase", "Category:", 0, QApplication::UnicodeUTF8));
    ButtonAddPlaceholder->setWhatsThis(QApplication::translate("ActionPropertyBase", "Add <b>Placeholders</b> for the selected files in the panel.", 0, QApplication::UnicodeUTF8));
    ButtonAddPlaceholder->setText(QApplication::translate("ActionPropertyBase", "&Add", 0, QApplication::UnicodeUTF8));
    textDescription->setWhatsThis(QApplication::translate("ActionPropertyBase", "A detailed description of the <b>Useraction</b>. It is only displayed in the <i>Konfigurator</i> and via <code>Shift-F1</code>.", 0, QApplication::UnicodeUTF8));
    LabelStartpath->setWhatsThis(QApplication::translate("ActionPropertyBase", "The <i>Workdir</i> defines in which directory the <i>Command</i> will be executed.", 0, QApplication::UnicodeUTF8));
    LabelStartpath->setText(QApplication::translate("ActionPropertyBase", "Workdir:", 0, QApplication::UnicodeUTF8));
    LabelShortcut->setText(QApplication::translate("ActionPropertyBase", "Default shortcut:", 0, QApplication::UnicodeUTF8));
    KeyButtonShortcut->setWhatsThis(QApplication::translate("ActionPropertyBase", "Set a default keyboard shortcut.", 0, QApplication::UnicodeUTF8));
    KeyButtonShortcut->setProperty("text", QVariant(QApplication::translate("ActionPropertyBase", "None", 0, QApplication::UnicodeUTF8)));
    bgExecType->setTitle(QApplication::translate("ActionPropertyBase", "Execution mode", 0, QApplication::UnicodeUTF8));
    radioCollectOutput->setWhatsThis(QApplication::translate("ActionPropertyBase", "Collect the output of the executed program.", 0, QApplication::UnicodeUTF8));
    radioCollectOutput->setText(QApplication::translate("ActionPropertyBase", "Collect output", 0, QApplication::UnicodeUTF8));
    chkSeparateStdError->setWhatsThis(QApplication::translate("ActionPropertyBase", "Separate standard out and standard error in the output collection.", 0, QApplication::UnicodeUTF8));
    chkSeparateStdError->setText(QApplication::translate("ActionPropertyBase", "Separate standard error", 0, QApplication::UnicodeUTF8));
    radioNormal->setText(QApplication::translate("ActionPropertyBase", "Normal", 0, QApplication::UnicodeUTF8));
    radioTerminal->setWhatsThis(QApplication::translate("ActionPropertyBase", "Run the command in a terminal.", 0, QApplication::UnicodeUTF8));
    radioTerminal->setText(QApplication::translate("ActionPropertyBase", "Run in terminal", 0, QApplication::UnicodeUTF8));
    tabWidget3->setTabText(tabWidget3->indexOf(tab), QApplication::translate("ActionPropertyBase", "Basic Properties", 0, QApplication::UnicodeUTF8));
    gbShowonly->setTitle(QApplication::translate("ActionPropertyBase", "The Useraction is only available for", 0, QApplication::UnicodeUTF8));
    ButtonNewProtocol->setText(QApplication::translate("ActionPropertyBase", "&New...", 0, QApplication::UnicodeUTF8));
    ButtonEditProtocol->setText(QApplication::translate("ActionPropertyBase", "Chan&ge...", 0, QApplication::UnicodeUTF8));
    ButtonRemoveProtocol->setText(QApplication::translate("ActionPropertyBase", "De&lete", 0, QApplication::UnicodeUTF8));
    lbShowonlyProtocol->setWhatsThis(QApplication::translate("ActionPropertyBase", "Show the <b>Useraction</b> only for the values defined here.", 0, QApplication::UnicodeUTF8));
    tabShowonly->setTabText(tabShowonly->indexOf(TabPage), QApplication::translate("ActionPropertyBase", "Protocol", 0, QApplication::UnicodeUTF8));
    lbShowonlyPath->setWhatsThis(QApplication::translate("ActionPropertyBase", "Show the <b>Useraction</b> only for the values defined here.", 0, QApplication::UnicodeUTF8));
    ButtonAddPath->setText(QApplication::translate("ActionPropertyBase", "&New...", 0, QApplication::UnicodeUTF8));
    ButtonEditPath->setText(QApplication::translate("ActionPropertyBase", "Chan&ge...", 0, QApplication::UnicodeUTF8));
    ButtonRemovePath->setText(QApplication::translate("ActionPropertyBase", "De&lete", 0, QApplication::UnicodeUTF8));
    tabShowonly->setTabText(tabShowonly->indexOf(tab2), QApplication::translate("ActionPropertyBase", "Path", 0, QApplication::UnicodeUTF8));
    lbShowonlyMime->setWhatsThis(QApplication::translate("ActionPropertyBase", "Show the <b>Useraction</b> only for the values defined here.", 0, QApplication::UnicodeUTF8));
    ButtonAddMime->setText(QApplication::translate("ActionPropertyBase", "&New...", 0, QApplication::UnicodeUTF8));
    ButtonEditMime->setText(QApplication::translate("ActionPropertyBase", "Chan&ge...", 0, QApplication::UnicodeUTF8));
    ButtonRemoveMime->setText(QApplication::translate("ActionPropertyBase", "De&lete", 0, QApplication::UnicodeUTF8));
    tabShowonly->setTabText(tabShowonly->indexOf(tab3), QApplication::translate("ActionPropertyBase", "Mime-type", 0, QApplication::UnicodeUTF8));
    lbShowonlyFile->setWhatsThis(QApplication::translate("ActionPropertyBase", "Show the <b>Useraction</b> only for the filenames defined here. The wildcards '<code>?</code>' and '<code>*</code>' can be used.", 0, QApplication::UnicodeUTF8));
    ButtonNewFile->setText(QApplication::translate("ActionPropertyBase", "&New...", 0, QApplication::UnicodeUTF8));
    ButtonEditFile->setText(QApplication::translate("ActionPropertyBase", "Chan&ge...", 0, QApplication::UnicodeUTF8));
    ButtonRemoveFile->setText(QApplication::translate("ActionPropertyBase", "De&lete", 0, QApplication::UnicodeUTF8));
    tabShowonly->setTabText(tabShowonly->indexOf(TabPage1), QApplication::translate("ActionPropertyBase", "Filename", 0, QApplication::UnicodeUTF8));
    chkConfirmExecution->setWhatsThis(QApplication::translate("ActionPropertyBase", "Allows to tweak the <i>Command</i> before it is executed.", 0, QApplication::UnicodeUTF8));
    chkConfirmExecution->setText(QApplication::translate("ActionPropertyBase", "Confirm each program call separately", 0, QApplication::UnicodeUTF8));
    chkDifferentUser->setWhatsThis(QApplication::translate("ActionPropertyBase", "Execute the <i>Command</i> under a different user-id.", 0, QApplication::UnicodeUTF8));
    chkDifferentUser->setText(QApplication::translate("ActionPropertyBase", "Run as different user:", 0, QApplication::UnicodeUTF8));
    leDifferentUser->setWhatsThis(QApplication::translate("ActionPropertyBase", "Execute the <i>Command</i> under a different user-id.", 0, QApplication::UnicodeUTF8));
    tabWidget3->setTabText(tabWidget3->indexOf(tab1), QApplication::translate("ActionPropertyBase", "Advanced Properties", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(ActionPropertyBase);
    } // retranslateUi

};

namespace Ui {
    class ActionPropertyBase: public Ui_ActionPropertyBase {};
} // namespace Ui

#endif // ACTIONPROPERTYBASE_H
