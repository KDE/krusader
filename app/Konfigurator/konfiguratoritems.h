/*
    SPDX-FileCopyrightText: 2003 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KONFIGURATORITEMS_H
#define KONFIGURATORITEMS_H

// QtCore
#include <QList>
#include <QObject>
#include <QString>
// QtGui
#include <QFont>
#include <QPixmap>
// QtWidgets
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QToolButton>

#include <KUrlRequester>

#include "../GUI/krlistwidget.h"

#define FIRST_PAGE 0

class KonfiguratorExtension : public QObject
{
    Q_OBJECT

public:
    KonfiguratorExtension(QObject *obj, QString cfgGroup, QString cfgName, bool restartNeeded = false, int page = FIRST_PAGE);

    virtual void loadInitialValue();
    virtual bool apply();
    virtual void setDefaults();
    virtual bool isChanged();
    virtual void setSubPage(int page)
    {
        subpage = page;
    }
    virtual int subPage()
    {
        return subpage;
    }

    inline QObject *object()
    {
        return objectPtr;
    }

    inline QString getConfigGroup()
    {
        return configGroup;
    }
    inline QString getConfigName()
    {
        return configName;
    }

public slots:
    void setChanged()
    {
        emit sigChanged(changed = true);
    }
    void setChanged(bool chg)
    {
        emit sigChanged(changed = chg);
    }

signals:
    void applyManually(QObject *, QString, QString);
    void applyAuto(QObject *, QString, QString);
    void setDefaultsManually(QObject *);
    void setDefaultsAuto(QObject *);
    void setInitialValue(QObject *);
    void sigChanged(bool);

protected:
    QObject *objectPtr;

    bool applyConnected;
    bool setDefaultsConnected;
    bool changed;
    bool restartNeeded;
    int subpage;

    QString configGroup;
    QString configName;

    void connectNotify(const QMetaMethod &signal) override;
};

// KonfiguratorCheckBox class
///////////////////////////////

class KonfiguratorCheckBox : public QCheckBox
{
    Q_OBJECT

public:
    KonfiguratorCheckBox(QString configGroup,
                         QString name,
                         bool defaultValue,
                         const QString &text,
                         QWidget *parent = nullptr,
                         bool restart = false,
                         int page = FIRST_PAGE);
    ~KonfiguratorCheckBox() override;

    inline KonfiguratorExtension *extension()
    {
        return ext;
    }

    // indicate that a checkobox is dependent of this,
    // meaning that dep is only available if this box is checked
    void addDep(KonfiguratorCheckBox *dep);

public slots:
    virtual void loadInitialValue();
    void slotApply(QObject *, const QString &, const QString &);
    void slotSetDefaults(QObject *);

protected:
    void checkStateSet() override;
    void nextCheckState() override;
    void updateDeps();

    bool defaultValue;
    KonfiguratorExtension *ext;
    QList<KonfiguratorCheckBox *> deps;
};

// KonfiguratorSpinBox class
///////////////////////////////

class KonfiguratorSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    KonfiguratorSpinBox(QString configGroup,
                        QString configName,
                        int defaultValue,
                        int min,
                        int max,
                        QWidget *parent = nullptr,
                        bool restartNeeded = false,
                        int page = FIRST_PAGE);
    ~KonfiguratorSpinBox() override;

    inline KonfiguratorExtension *extension()
    {
        return ext;
    }

public slots:
    virtual void loadInitialValue();
    void slotApply(QObject *, const QString &, const QString &);
    void slotSetDefaults(QObject *);

protected:
    int defaultValue;
    KonfiguratorExtension *ext;
};

// KonfiguratorCheckBoxGroup class
///////////////////////////////

class KonfiguratorCheckBoxGroup : public QWidget
{
public:
    explicit KonfiguratorCheckBoxGroup(QWidget *parent = nullptr)
        : QWidget(parent)
    {
    }

    void add(KonfiguratorCheckBox *);
    qsizetype count()
    {
        return checkBoxList.count();
    }
    KonfiguratorCheckBox *find(int index);
    KonfiguratorCheckBox *find(const QString &name);

private:
    QList<KonfiguratorCheckBox *> checkBoxList;
};

// KonfiguratorRadioButtons class
///////////////////////////////

class KonfiguratorRadioButtons : public QWidget
{
    Q_OBJECT

public:
    KonfiguratorRadioButtons(QString configGroup, QString name, QString defaultValue, QWidget *parent = nullptr, bool restart = false, int page = FIRST_PAGE);
    ~KonfiguratorRadioButtons() override;

    inline KonfiguratorExtension *extension()
    {
        return ext;
    }

    void addRadioButton(QRadioButton *radioWidget, const QString &name, const QString &value);

    void selectButton(const QString &value);

    qsizetype count()
    {
        return radioButtons.count();
    }
    QString selectedValue();
    QRadioButton *find(int index);
    QRadioButton *find(const QString &name);

public slots:
    virtual void loadInitialValue();
    void slotApply(QObject *, const QString &, const QString &);
    void slotSetDefaults(QObject *);

protected:
    QList<QRadioButton *> radioButtons;
    QList<QString> radioValues;
    QList<QString> radioNames;

    QString defaultValue;

    KonfiguratorExtension *ext;
};

// KonfiguratorEditBox class
///////////////////////////////

class KonfiguratorEditBox : public QLineEdit
{
    Q_OBJECT

public:
    KonfiguratorEditBox(QString configGroup, QString name, QString defaultValue, QWidget *parent = nullptr, bool restart = false, int page = FIRST_PAGE);
    ~KonfiguratorEditBox() override;

    inline KonfiguratorExtension *extension()
    {
        return ext;
    }

public slots:
    virtual void loadInitialValue();
    void slotApply(QObject *, const QString &, const QString &);
    void slotSetDefaults(QObject *);

protected:
    QString defaultValue;
    KonfiguratorExtension *ext;
};

// KonfiguratorURLRequester class
///////////////////////////////

class KonfiguratorURLRequester : public KUrlRequester
{
    Q_OBJECT

public:
    KonfiguratorURLRequester(QString configGroup,
                             QString name,
                             QString defaultValue,
                             QWidget *parent = nullptr,
                             bool restart = false,
                             int page = FIRST_PAGE,
                             bool expansion = true);
    ~KonfiguratorURLRequester() override;

    inline KonfiguratorExtension *extension()
    {
        return ext;
    }

public slots:
    virtual void loadInitialValue();
    void slotApply(QObject *, const QString &, const QString &);
    void slotSetDefaults(QObject *);

protected:
    QString defaultValue;
    KonfiguratorExtension *ext;
    bool expansion;
};

// KonfiguratorFontChooser class
///////////////////////////////

class KonfiguratorFontChooser : public QWidget
{
    Q_OBJECT

public:
    KonfiguratorFontChooser(QString configGroup,
                            QString name,
                            const QFont &defaultValue,
                            QWidget *parent = nullptr,
                            bool restart = false,
                            int page = FIRST_PAGE);
    ~KonfiguratorFontChooser() override;

    inline KonfiguratorExtension *extension()
    {
        return ext;
    }

public slots:
    virtual void loadInitialValue();
    void slotApply(QObject *, const QString &, const QString &);
    void slotSetDefaults(QObject *);
    void slotBrowseFont();

protected:
    QFont defaultValue;
    QFont font;
    KonfiguratorExtension *ext;

    QLabel *pLabel;
    QToolButton *pToolButton;

    void setFont();
};

// KONFIGURATOR_NAME_VALUE_PAIR structure
///////////////////////////////

struct KONFIGURATOR_NAME_VALUE_PAIR {
    QString text;
    QString value;
};

// KONFIGURATOR_NAME_VALUE_TIP structure
///////////////////////////////

struct KONFIGURATOR_NAME_VALUE_TIP {
    QString text;
    QString value;
    QString tooltip;
};

// KonfiguratorComboBox class
///////////////////////////////

class KonfiguratorComboBox : public QComboBox
{
    Q_OBJECT

public:
    KonfiguratorComboBox(QString configGroup,
                         QString name,
                         QString defaultValue,
                         KONFIGURATOR_NAME_VALUE_PAIR *listIn,
                         int listInLen,
                         QWidget *parent = nullptr,
                         bool restart = false,
                         bool editable = false,
                         int page = FIRST_PAGE);
    ~KonfiguratorComboBox() override;

    inline KonfiguratorExtension *extension()
    {
        return ext;
    }

public slots:
    virtual void loadInitialValue();
    void slotApply(QObject *, const QString &, const QString &);
    void slotSetDefaults(QObject *);

protected:
    QString defaultValue;
    KONFIGURATOR_NAME_VALUE_PAIR *list;
    int listLen;
    KonfiguratorExtension *ext;

    void selectEntry(const QString &entry);
};

// KonfiguratorColorChooser class
///////////////////////////////

typedef struct {
    QString name;
    QColor color;
    QString value;
} ADDITIONAL_COLOR;

class KonfiguratorColorChooser : public QComboBox
{
    Q_OBJECT

public:
    KonfiguratorColorChooser(QString configGroup,
                             QString name,
                             const QColor &defaultValue,
                             QWidget *parent = nullptr,
                             bool restart = false,
                             ADDITIONAL_COLOR *addColPtr = nullptr,
                             int addColNum = 0,
                             int page = FIRST_PAGE);
    ~KonfiguratorColorChooser() override;

    inline KonfiguratorExtension *extension()
    {
        return ext;
    }

    void setDefaultColor(QColor dflt);
    void setDefaultText(const QString &text);
    QColor getColor();
    void changeAdditionalColor(int num, const QColor &color);
    QString getValue();
    bool isValueRGB();
    void setValue(const QString &value);

public slots:
    virtual void loadInitialValue();
    void slotApply(QObject *, const QString &, const QString &);
    void slotSetDefaults(QObject *);
    void slotCurrentChanged(int number);

signals:
    void colorChanged();

private:
    void addColor(const QString &text, const QColor &color);
    QPixmap createPixmap(const QColor &color);

protected:
    QColor defaultValue;
    QColor customValue;
    QList<QColor> palette;
    QList<ADDITIONAL_COLOR> additionalColors;
    KonfiguratorExtension *ext;
    bool disableColorChooser;
};

// KonfiguratorListBox class
///////////////////////////////

class KonfiguratorListBox : public KrListWidget
{
    Q_OBJECT

public:
    KonfiguratorListBox(QString configGroup, QString name, QStringList defaultValue, QWidget *parent = nullptr, bool restart = false, int page = FIRST_PAGE);
    ~KonfiguratorListBox() override;

    inline KonfiguratorExtension *extension()
    {
        return ext;
    }

    void addItem(const QString &);
    void removeItem(const QString &);

public slots:
    virtual void loadInitialValue();
    void slotApply(QObject *, const QString &, const QString &);
    void slotSetDefaults(QObject *);

protected:
    QStringList list();
    void setList(const QStringList &);

    QStringList defaultValue;
    KonfiguratorExtension *ext;
};

#endif /* __KONFIGURATOR_ITEMS_H__ */
