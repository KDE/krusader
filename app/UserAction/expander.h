/*
    SPDX-FileCopyrightText: 2004 Jonas Bähr <jonas.baehr@web.de>
    SPDX-FileCopyrightText: 2004 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EXPANDER_H
#define EXPANDER_H

// QtCore
#include <QList>
#include <QString>
#include <QStringList>

#include "tstring.h"

class KrPanel;
class Expander;
class Error;

typedef TagString_t<QStringList> TagString;
typedef QList<TagString> TagStringList;

/**
 * This holds the information about each parameter
 */
class exp_parameter
{
public:
    exp_parameter()
    {
    }
    inline exp_parameter(QString desc, QString pre, bool ness)
    {
        _description = desc;
        _preset = pre;
        _necessary = ness;
    }
    inline QString description() const
    { ///< A description of the parameter
        return _description;
    }
    inline QString preset() const
    { ///< the default of the parameter
        return _preset;
    }
    inline bool necessary() const
    { ///< false if the parameter is optional
        return _necessary;
    }

private:
    QString _description;
    QString _preset;
    bool _necessary;
};

#define EXP_FUNC virtual TagString expFunc(const KrPanel *, const TagStringList &, const bool &, Expander &) const
#define SIMPLE_EXP_FUNC virtual TagString expFunc(const KrPanel *, const QStringList &, const bool &, Expander &) const
/**
 * Abstract baseclass for all expander-functions (which replace placeholder).
 * A Placeholder is an entry containing the expression,
 * its expanding function and Parameter.
 *
 * Not to be created on the heap
 */
class exp_placeholder
{
public:
    inline QString expression() const
    { ///< The placeholder (without '%' or panel-prefix)
        return _expression;
    }
    inline QString description() const
    { ///< A description of the placeholder
        return _description;
    }
    inline bool needPanel() const
    { ///< true if the placeholder needs a panel to operate on
        return _needPanel;
    }
    inline void addParameter(exp_parameter parameter)
    { ///< adds parameter to the placeholder
        _parameter.append(parameter);
    }
    inline qsizetype parameterCount() const
    { ///< returns the number of placeholders
        return _parameter.count();
    }
    inline const exp_parameter &parameter(int id) const
    { ///< returns a specific parameter
        return _parameter[id];
    }

    EXP_FUNC = 0;

protected:
    static void setError(Expander &exp, const Error &e);
    static void panelMissingError(const QString &s, Expander &exp);
    static QStringList splitEach(const TagString &s);
    static QStringList
    fileList(const KrPanel *const panel, const QString &type, const QString &mask, const bool omitPath, const bool useUrl, Expander &, const QString &);
    exp_placeholder();
    exp_placeholder(const exp_placeholder &p);
    virtual ~exp_placeholder()
    {
    }
    QString _expression;
    QString _description;
    QList<exp_parameter> _parameter;
    bool _needPanel;
};

class Error
{
public:
    enum Cause { exp_C_USER, exp_C_SYNTAX, exp_C_WORLD, exp_C_ARGUMENT };
    enum Severity { exp_S_OK, exp_S_WARNING, exp_S_ERROR, exp_S_FATAL };

    Error()
        : m_severity(exp_S_OK)
    {
    }
    Error(Severity severity, Cause cause)
        : m_severity(severity)
        , m_cause(cause)
        , m_description()
    {
    }
    Error(Severity severity, Cause cause, QString description)
        : m_severity(severity)
        , m_cause(cause)
        , m_description(description)
    {
    }

    operator bool() const
    {
        return m_severity != exp_S_OK;
    }
    Cause cause() const
    {
        return m_cause;
    }
    const QString &description() const
    {
        return m_description;
    }

private:
    Severity m_severity;
    Cause m_cause;
    QString m_description;
};

/**
 * The Expander expands the command of an UserAction by replacing all
 * placeholders by their current values.@n
 * Each placeholder begins with a '%'-sign, followed by one char indicating
 * the panel, followed by a command which may have some parameter enclosed
 * in brackets and also ends with a '%'-sign.
 * Examples are %aPath% or %rBookmark("/home/jonas/src/krusader_kde3", "yes")%.@n
 * The panel-indicator has to be either
 * 'a' for the active
 * 'o' for the other
 * 'r' for the right
 * 'l' for the left
 * '_' for panel-independence.
 *
 * Currently supported are these commands can be ordered in three groups
 * (children are the parameter in the right order):
 * - Placeholders for Krusaders panel-data (panel-indicator has to be 'a', 'o', 'r' or 'l')
 *    - @em Path is replaced by the panel's path
 *    - @em Count is replaced by a nomber of
 *       -# Either "All", "Files", "Dirs", "Selected"
 *       .
 *    - @em Filter is preplaced by the panels filter-mask (ex: "*.cpp *.h")
 *    - @em Current is replaced by the current item or, in case of onmultiple="call_each", by each selected item.
 *       -# If "yes", only the filename (without path) is returned
 *       .
 *    - @em List isreplaced by a list of
 *       -# Either "All", "Files", "Dirs", "Selected"
 *       -# A separator between the items (default: " " [one space])
 *       -# If "yes", only the filename (without path) is returned
 *       -# (for all but "Selected") a filter-mask (default: "*")
 *       .
 *   .
 * - Access to panel-dependent, krusader-internal, parameter-needed functions
 *   (panel-indicator has to be 'a', 'o', 'r' or 'l')
 *    - @em Select manipulates the selection of the panel
 *       -# A filter-mask (necessary)
 *       -# Either "Add", "Remove", "Set" (default)
 *       .
 *    - @em Bookmark manipulates the selection of the panel
 *       -# A path or URL (necessary)
 *       -# If "yes", the location is opened in a new tab
 *       .
 *   .
 * - Access to panel-independent, krusader-internal, parameter-needed functions
 *   (panel-indicator doesn't matter but should be set to '_')
 *    - @em Ask displays a lineedit and is replaced by its return
 *       -# The question (necessary)
 *       -# A default answer
 *       -# A cation for the popup
 *       .
 *    - @em Clipboard manipulates the system-wide clipboard
 *       -# The string copied to clip (ex: "%aCurrent%") (necessary)
 *       -# A separator. If set, parameter1 is append with this to the current clipboard content
 *       .
 *    .
 * .
 * Since all placeholders are expanded in the order they appear in the command,
 * little one-line-scripts are possible
 */
class Expander
{
public:
    inline static qsizetype placeholderCount()
    { ///< returns the number of placeholders
        return _placeholder().count();
    }
    inline static const exp_placeholder *placeholder(qsizetype id)
    {
        return _placeholder()[id];
    }

    /**
     * This expands a whole commandline
     *
     * @param stringToExpand the commandline with the placeholder
     * @param useUrl true iff the path's should be expanded to an URL instead of an local path
     * @return a list of all commands
     */
    void expand(const QString &stringToExpand, bool useUrl);

    /**
     * Returns the list of all commands to be executed, provided that #expand was called
     * before, and there was no error (see #error).
     *
     * @return The list of commands to be executed
     */
    const QStringList &result() const
    {
        assert(!error());
        return resultList;
    }

    /**
     *  Returns the error object of this Expander. You can test whether there was
     * any error by
     * \code
     * if(exp.error())
     *  error behaviour...
     * else
     *   no error...
     * \endcode
     *
     * @return The error object
     */
    const Error &error() const
    {
        return _err;
    }

protected:
    /**
     * This expands a whole commandline by calling for each Placeholder the corresponding expander
     *
     * @param stringToExpand the commandline with the placeholder
     * @param useUrl true if the path's should be expanded to an URL instead of an local path
     * @return the expanded commanline for the current item
     */
    TagString expandCurrent(const QString &stringToExpand, bool useUrl);
    /**
     * This function searches for "\@EACH"-marks to split the string in a list for each %_Each%-item
     *
     * @param stringToSplit the string which should be split
     * @return the split list
     */
    static QStringList splitEach(TagString stringToSplit);
    /**
     * @param panelIndicator either '_' for panel-independent placeholders, 'a', 'o', 'r', or 'l' for the active, other (inactive), right or left panel
     * @param pl placeholder
     * @param exp expander
     * @return a pointer to the right panel or NULL if no panel is needed.
     */
    static KrPanel *getPanel(const char panelIndicator, const exp_placeholder *, Expander &);
    /**
     *  This splits the parameter-string into separate parameter and expands each
     * @param exp the string holding all parameter
     * @param useUrl true if the path's should be expanded to an URL instead of an local path
     * @return a list of all parameter
     */
    TagStringList separateParameter(QString *const exp, bool useUrl);
    /**
     * This finds the end of a placeholder, taking care of the parameter
     * @return the position where the placeholder ends
     */
    qsizetype findEnd(const QString &str, qsizetype start);

    void setError(const Error &e)
    {
        _err = e;
    }
    friend class exp_placeholder;

private:
    static QList<const exp_placeholder *> &_placeholder();
    Error _err;
    QStringList resultList;
};

#endif // ifndef EXPANDER_H
