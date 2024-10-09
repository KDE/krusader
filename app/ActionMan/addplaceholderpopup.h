/*
    SPDX-FileCopyrightText: 2004 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004 Jonas Bähr <jonas.baehr@web.de>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ADDPLACEHOLDERPOPUP_H
#define ADDPLACEHOLDERPOPUP_H

// QtCore
#include <QList>
#include <QString>
// QtWidgets
#include <QDialog>
#include <QMenu>

#include "../UserAction/expander.h"

class KLineEdit;
class QToolButton;
class QCheckBox;
class KComboBox;
class QSpinBox;

/**
 * This reads Expander::placeholder[] and
 * fills a popup for easy access to the UserAction Placeholder
 */
class AddPlaceholderPopup : public QMenu
{
public:
    explicit AddPlaceholderPopup(QWidget *parent);

    /**
     * Use this to exec the popup.
     * @param pos Position where the popup should appear
     * @return the expression which can be placed in the UserAction commandline
     */
    QString getPlaceholder(const QPoint &pos);

protected:
    /**
     * This is called when a Placeholder got parameter.
     * @param currentPlaceholder A pointer to the Placeholder the user has chosen
     * @return a parameter-string
     */
    QString getParameter(exp_placeholder *currentPlaceholder);

private:
    QMenu *_activeSub, *_otherSub, *_leftSub, *_rightSub, *_independentSub;
};

////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Parameter Widgets ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

/**
 *  abstract baseclass for all Parameter widgets
 */
class ParameterBase : public QWidget
{
public:
    inline ParameterBase(const exp_parameter &parameter, QWidget *parent)
        : QWidget(parent)
    {
        _necessary = parameter.necessary();
    }
    /**
     * @return the text for the parameter
     */
    virtual QString text() = 0;
    /**
     * @return the default of the parameter
     */
    virtual QString preset() = 0;
    /**
     * re-init the parameter with the default
     */
    virtual void reset() = 0;
    /**
     * @return true if the Parameter as a valid value
     */
    virtual bool valid() = 0;
    /**
     * @return true if the Placeholder really needs this parameter
     */
    inline bool necessary()
    {
        return _necessary;
    }

private:
    bool _necessary;
};

/**
 *  The simple Parameter widgets: a line-edit with the description above
 *  used by default
 */
class ParameterText : public ParameterBase
{
public:
    ParameterText(const exp_parameter &parameter, QWidget *parent);
    QString text() override;
    QString preset() override;
    void reset() override;
    bool valid() override;

private:
    KLineEdit *_lineEdit;
    QString _preset;
};

/**
 *  A line-edit with the "addPlaceholder"-button
 *  used with default = "__placeholder"
 */
class ParameterPlaceholder : public ParameterBase
{
    Q_OBJECT
public:
    ParameterPlaceholder(const exp_parameter &parameter, QWidget *parent);
    QString text() override;
    QString preset() override;
    void reset() override;
    bool valid() override;

private:
    KLineEdit *_lineEdit;
    QToolButton *_button;
private slots:
    void addPlaceholder();
};

/**
 *  A Checkbox, default: checked; retuns "No" if unchecked
 *  used with default = "__yes"
 */
class ParameterYes : public ParameterBase
{
public:
    ParameterYes(const exp_parameter &parameter, QWidget *parent);
    QString text() override;
    QString preset() override;
    void reset() override;
    bool valid() override;

private:
    QCheckBox *_checkBox;
};

/**
 *  A Checkbox, default: unchecked; retuns "Yes" if checked
 *  used with default = "__no"
 */
class ParameterNo : public ParameterBase
{
public:
    ParameterNo(const exp_parameter &parameter, QWidget *parent);
    QString text() override;
    QString preset() override;
    void reset() override;
    bool valid() override;

private:
    QCheckBox *_checkBox;
};

/**
 *  A line-edit with the "file open"-button
 *  used with default = "__file"
 */
class ParameterFile : public ParameterBase
{
    Q_OBJECT
public:
    ParameterFile(const exp_parameter &parameter, QWidget *parent);
    QString text() override;
    QString preset() override;
    void reset() override;
    bool valid() override;

private:
    KLineEdit *_lineEdit;
    QToolButton *_button;
private slots:
    void addFile();
};

/**
 *  A ComboBox with the description above
 *  used with default = "__choose:item1;item2;..."
 */
class ParameterChoose : public ParameterBase
{
public:
    ParameterChoose(const exp_parameter &parameter, QWidget *parent);
    QString text() override;
    QString preset() override;
    void reset() override;
    bool valid() override;

private:
    KComboBox *_combobox;
};

/**
 *  An editable ComboBox with the predefined selections
 *  used with default = "__select"
 */
class ParameterSelect : public ParameterBase
{
public:
    ParameterSelect(const exp_parameter &parameter, QWidget *parent);
    QString text() override;
    QString preset() override;
    void reset() override;
    bool valid() override;

private:
    KComboBox *_combobox;
};

/**
 *  A line-edit with a "choose dir"- and a bookmark-button
 *  used with default = "__goto"
 */
class ParameterGoto : public ParameterBase
{
    Q_OBJECT
public:
    ParameterGoto(const exp_parameter &parameter, QWidget *parent);
    QString text() override;
    QString preset() override;
    void reset() override;
    bool valid() override;

private:
    KLineEdit *_lineEdit;
    QToolButton *_dirButton, *_placeholderButton;
private slots:
    void setDir();
    void addPlaceholder();
};

/**
 *  A ComboBox with all profiles available for the Synchronizer
 *  used with default = "__syncprofile"
 */
class ParameterSyncprofile : public ParameterBase
{
public:
    ParameterSyncprofile(const exp_parameter &parameter, QWidget *parent);
    QString text() override;
    QString preset() override;
    void reset() override;
    bool valid() override;

private:
    KComboBox *_combobox;
};

/**
 *  A ComboBox with all profiles available for the panels
 *  used with default = "__panelprofile"
 */
class ParameterPanelprofile : public ParameterBase
{
public:
    ParameterPanelprofile(const exp_parameter &parameter, QWidget *parent);
    QString text() override;
    QString preset() override;
    void reset() override;
    bool valid() override;

private:
    KComboBox *_combobox;
};

/**
 *  A ComboBox with all profiles available for the Searchmodule
 *  used with default = "__searchprofile"
 */
class ParameterSearch : public ParameterBase
{
public:
    ParameterSearch(const exp_parameter &parameter, QWidget *parent);
    QString text() override;
    QString preset() override;
    void reset() override;
    bool valid() override;

private:
    KComboBox *_combobox;
};

/**
 *  A SpinBox for integer
 *  used with default = "__int:min;max;step;value"
 */
class ParameterInt : public ParameterBase
{
public:
    ParameterInt(const exp_parameter &parameter, QWidget *parent);
    QString text() override;
    QString preset() override;
    void reset() override;
    bool valid() override;

private:
    QSpinBox *_spinbox;
    int _default;
};

////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// ParameterDialog ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

/**
 *  Opens a dialog for the parameter. Depending on the default (preset) a different widget is used.
 *  See Parameter-Classes for details
 */
class ParameterDialog : public QDialog
{
    Q_OBJECT
public:
    ParameterDialog(const exp_placeholder *currentPlaceholder, QWidget *parent);

    /**
     * Use this to execute the dialog.
     * @return a QString with all parameters; omitting the optional ones if they have the default-value.
     */
    QString getParameter();

private:
    typedef QList<ParameterBase *> ParameterList;
    ParameterList _parameter;
    qsizetype _parameterCount;
private slots:
    void reset();
    void slotOk();
};

#endif // ADDPLACEHOLDERPOPUP_H
