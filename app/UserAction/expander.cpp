/*
    SPDX-FileCopyrightText: 2004 Jonas Bähr <jonas.baehr@web.de>
    SPDX-FileCopyrightText: 2004 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "expander.h"

#include "../FileSystem/filesystemprovider.h"
#include "../GUI/profilemanager.h"
#include "../KViewer/krviewer.h"
#include "../Panel/PanelView/krview.h"
#include "../Panel/listpanel.h"
#include "../Panel/panelfunc.h"
#include "../Search/krsearchdialog.h"
#include "../krservices.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "../panelmanager.h"

#ifdef SYNCHRONIZER_ENABLED
#include "../Synchronizer/synchronizergui.h"
#endif

// QtCore
#include <QDebug>
#include <QList>
#include <QStringList>
#include <QTemporaryFile>
#include <QTextStream>
// QtGui
#include <QClipboard>
// QtWidgets
#include <QApplication>
#include <QInputDialog>

#include <KLocalizedString>
#include <KMessageBox>

#include <algorithm>
#include <functional>
using namespace std;

#define NEED_PANEL                                                                                                                                             \
    if (panel == nullptr) {                                                                                                                                    \
        panelMissingError(_expression, exp);                                                                                                                   \
        return QString();                                                                                                                                      \
    }

inline void exp_placeholder::setError(Expander &exp, const Error &e)
{
    exp.setError(e);
}
inline QStringList exp_placeholder::splitEach(const TagString &s)
{
    return Expander::splitEach(s);
}
inline exp_placeholder::exp_placeholder() = default;

void exp_placeholder::panelMissingError(const QString &s, Expander &exp)
{
    exp.setError(Error(Error::exp_S_FATAL, Error::exp_C_ARGUMENT, i18n("Needed panel specification missing in expander %1", s)));
}

QStringList exp_placeholder::fileList(const KrPanel *const panel,
                                      const QString &type,
                                      const QString &mask,
                                      const bool omitPath,
                                      const bool useUrl,
                                      Expander &exp,
                                      const QString &error)
{
    QStringList items;
    if (type.isEmpty() || type == "all")
        panel->view->getItemsByMask(mask, &items);
    else if (type == "files")
        panel->view->getItemsByMask(mask, &items, false, true);
    else if (type == "dirs")
        panel->view->getItemsByMask(mask, &items, true, false);
    else if (type == "selected")
        panel->view->getSelectedItems(&items);
    else {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_ARGUMENT, i18n("Expander: Bad argument to %1: %2 is not valid item specifier", error, type)));
        return QStringList();
    }
    if (!omitPath) { // add the current path
        // translate to urls using filesystem
        const QList<QUrl> list = panel->func->files()->getUrls(items);
        items.clear();
        // parse everything to a single qstring
        for (const QUrl &url : list) {
            items.push_back(useUrl ? url.url() : url.path());
        }
    }

    return items;
}

namespace
{

class exp_simpleplaceholder : public exp_placeholder
{
public:
    EXP_FUNC override;
    virtual TagString expFunc(const KrPanel *, const QStringList &, const bool &, Expander &) const = 0;
};

#define PLACEHOLDER_CLASS(name)                                                                                                                                \
    class name : public exp_placeholder                                                                                                                        \
    {                                                                                                                                                          \
    public:                                                                                                                                                    \
        name();                                                                                                                                                \
        virtual TagString expFunc(const KrPanel *, const TagStringList &, const bool &, Expander &) const override;                                            \
    };

#define SIMPLE_PLACEHOLDER_CLASS(name)                                                                                                                         \
    class name : public exp_simpleplaceholder                                                                                                                  \
    {                                                                                                                                                          \
    public:                                                                                                                                                    \
        using exp_simpleplaceholder::expFunc;                                                                                                                  \
        name();                                                                                                                                                \
        virtual TagString expFunc(const KrPanel *, const QStringList &, const bool &, Expander &) const override;                                              \
    };

/**
 * expands %_Path% ('_' is replaced by 'a', 'o', 'r' or 'l' to indicate the active, other, right or left panel) with the path of the specified panel
 */
SIMPLE_PLACEHOLDER_CLASS(exp_Path)

/**
 * expands %_Count% ('_' is replaced by 'a', 'o', 'r' or 'l' to indicate the active, other, right or left panel) with the number of items, which type is
 * specified by the first Parameter
 */
SIMPLE_PLACEHOLDER_CLASS(exp_Count)

/**
 * expands %_Filter% ('_' is replaced by 'a', 'o', 'r' or 'l' to indicate the active, other, right or left panel) with the correspondent filter (ie: "*.cpp")
 */
SIMPLE_PLACEHOLDER_CLASS(exp_Filter)

/**
 * expands %_Current% ('_' is replaced by 'a', 'o', 'r' or 'l' to indicate the active, other, right or left panel) with the current item ( != the selected ones)
 */
SIMPLE_PLACEHOLDER_CLASS(exp_Current)

/**
 * expands %_List% ('_' is replaced by 'a', 'o', 'r' or 'l' to indicate the active, other, right or left panel) with a list of items, which type is specified by
 * the first Parameter
 */
SIMPLE_PLACEHOLDER_CLASS(exp_List)

/**
 * expands %_ListFile% ('_' is replaced by 'a', 'o', 'r' or 'l' to indicate
 * the active, other, right or left panel) with the name of a temporary file,
 * containing a list of items, which type is specified by the first Parameter
 */
SIMPLE_PLACEHOLDER_CLASS(exp_ListFile)

/**
 * expands %_Ask% ('_' is necessary because there is no panel needed)
 * with the return of an input-dialog
 */
SIMPLE_PLACEHOLDER_CLASS(exp_Ask)

/**
 * This copies it's first Parameter to the clipboard
 */
PLACEHOLDER_CLASS(exp_Clipboard)

/**
 * This selects all items by the mask given with the first Parameter
 */
SIMPLE_PLACEHOLDER_CLASS(exp_Select)

/**
 * This changes the panel'spath to the value given with the first Parameter.
 */
SIMPLE_PLACEHOLDER_CLASS(exp_Goto)

/**
 * This is equal to 'cp <first Parameter> <second Parameter>'.
 */
PLACEHOLDER_CLASS(exp_Copy)

/**
 * This is equal to 'mv <first Parameter> <second Parameter>'.
 */
PLACEHOLDER_CLASS(exp_Move)

#ifdef SYNCHRONIZER_ENABLED
/**
 * This opens the synchronizer with a given profile
 */
SIMPLE_PLACEHOLDER_CLASS(exp_Sync)
#endif

/**
 * This opens the searchmodule with a given profile
 */
SIMPLE_PLACEHOLDER_CLASS(exp_NewSearch)

/**
 * This loads the panel-profile with a given name
 */
SIMPLE_PLACEHOLDER_CLASS(exp_Profile)

/**
 * This is setting marks in the string where he is later split up for each {all, selected, files, dirs}
 */
SIMPLE_PLACEHOLDER_CLASS(exp_Each)

/**
 * This sets the sorting on a specific column
 */
SIMPLE_PLACEHOLDER_CLASS(exp_ColSort)

/**
 * This sets relation between the left and right panel
 */
SIMPLE_PLACEHOLDER_CLASS(exp_PanelSize)

/**
 * This loads a file in the internal viewer
 */
SIMPLE_PLACEHOLDER_CLASS(exp_View)

////////////////////////////////////////////////////////////
//////////////////////// utils ////////////////////////
////////////////////////////////////////////////////////////

/**
 * escapes everything that confuses bash in filenames
 * @param s String to manipulate
 * @return escaped string
 */
QString bashquote(QString s)
{
    /*
    // we _can_not_ use this function because it _encloses_ the sting in single-quotes!
    // In this case quotes strings could not be concatenated anymore
    return KrServices::quote(s);
    */

    static const QString evilstuff = "\\\"'`()[]{}!?;$&<>| \t\r\n"; // stuff that should get escaped

    for (auto i : evilstuff)
        s.replace(i, ('\\' + i));

    return s;
}

QString separateAndQuote(QStringList list, const QString &separator, const bool quote)
{
    if (quote)
        transform(list.begin(), list.end(), list.begin(), bashquote);

    // QLineEdit::text() always escapes special characters, revert this for newline and tab
    QString decodedSeparator = separator;
    decodedSeparator.replace("\\n", "\n").replace("\\t", "\t");
    return list.join(decodedSeparator);
}
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// expander classes ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

exp_Path::exp_Path()
{
    _expression = "Path";
    _description = i18n("Panel's Path...");
    _needPanel = true;

    addParameter(exp_parameter(i18n("Automatically escape spaces"), "__yes", false));
}
TagString exp_Path::expFunc(const KrPanel *panel, const QStringList &parameter, const bool &useUrl, Expander &exp) const
{
    NEED_PANEL

    QString result;

    if (useUrl)
        result = panel->func->files()->currentDirectory().url() + '/';
    else
        result = panel->func->files()->currentDirectory().path() + '/';

    if (parameter.count() > 0 && parameter[0].toLower() == "no") // don't escape spaces
        return TagString(result);
    else
        return TagString(bashquote(result));
}

exp_Count::exp_Count()
{
    _expression = "Count";
    _description = i18n("Number of...");
    _needPanel = true;

    addParameter(exp_parameter(i18n("Count:"), "__choose:All;Files;Dirs;Selected", false));
}
TagString exp_Count::expFunc(const KrPanel *panel, const QStringList &parameter, const bool &, Expander &exp) const
{
    NEED_PANEL

    int n = -1;
    if (parameter.count() == 0 || parameter[0].isEmpty() || parameter[0].toLower() == "all")
        n = panel->view->numDirs() + panel->view->numFiles();
    else if (parameter[0].toLower() == "files")
        n = panel->view->numFiles();
    else if (parameter[0].toLower() == "dirs")
        n = panel->view->numDirs();
    else if (parameter[0].toLower() == "selected")
        n = panel->view->numSelected();
    else {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_ARGUMENT, i18n("Expander: Bad argument to Count: %1 is not valid item specifier", parameter[0])));
        return QString();
    }

    return TagString(QString("%1").arg(n));
}

exp_Filter::exp_Filter()
{
    _expression = "Filter";
    _description = i18n("Filter Mask (*.h, *.cpp, etc.)");
    _needPanel = true;
}
TagString exp_Filter::expFunc(const KrPanel *panel, const QStringList &, const bool &, Expander &exp) const
{
    NEED_PANEL

    return panel->view->filterMask().nameFilter();
}

exp_Current::exp_Current()
{
    _expression = "Current";
    _description = i18n("Current File (!= Selected File)...");
    _needPanel = true;

    addParameter(exp_parameter(i18n("Omit the current path (optional)"), "__no", false));
    addParameter(exp_parameter(i18n("Automatically escape spaces"), "__yes", false));
}
TagString exp_Current::expFunc(const KrPanel *panel, const QStringList &parameter, const bool &useUrl, Expander &exp) const
{
    NEED_PANEL

    QString item = panel->view->getCurrentItem();
    if (item == "..") {
        // if ".." is current, treat this as nothing is current
        return QString();
    }

    QString result;
    if (parameter.count() > 0 && parameter[0].toLower() == "yes") // omit the current path
        result = item;
    else {
        const QUrl url = panel->func->files()->getUrl(item);
        result = useUrl ? url.url() : url.path();
    }

    const bool escapeSpaces = parameter.count() < 2 || parameter[1].toLower() != "no";
    return escapeSpaces ? bashquote(result) : result;
}

exp_List::exp_List()
{
    _expression = "List";
    _description = i18n("Item List of...");
    _needPanel = true;

    addParameter(exp_parameter(i18n("Which items:"), "__choose:All;Files;Dirs;Selected", false));
    addParameter(exp_parameter(i18n("Separator between the items (optional):"), " ", false));
    addParameter(exp_parameter(i18n("Omit the current path (optional)"), "__no", false));
    addParameter(exp_parameter(i18n("Mask (optional, all but 'Selected'):"), "__select", false));
    addParameter(exp_parameter(i18n("Automatically escape spaces"), "__yes", false));
}
TagString exp_List::expFunc(const KrPanel *panel, const QStringList &parameter, const bool &useUrl, Expander &exp) const
{
    NEED_PANEL

    // get selected items from view
    QStringList items;
    QString mask;

    if (parameter.count() <= 3 || parameter[3].isEmpty())
        mask = '*';
    else
        mask = parameter[3];

    return separateAndQuote(fileList(panel,
                                     parameter.isEmpty() ? QString() : parameter[0].toLower(),
                                     mask,
                                     parameter.count() > 2 ? parameter[2].toLower() == "yes" : false,
                                     useUrl,
                                     exp,
                                     "List"),
                            parameter.count() > 1 ? parameter[1] : " ",
                            parameter.count() > 4 ? parameter[4].toLower() == "yes" : true);
}

exp_ListFile::exp_ListFile()
{
    _expression = "ListFile";
    _description = i18n("Filename of an Item List...");
    _needPanel = true;

    addParameter(exp_parameter(i18n("Which items:"), "__choose:All;Files;Dirs;Selected", false));
    addParameter(exp_parameter(i18n("Separator between the items (optional)"), "\n", false));
    addParameter(exp_parameter(i18n("Omit the current path (optional)"), "__no", false));
    addParameter(exp_parameter(i18n("Mask (optional, all but 'Selected'):"), "__select", false));
    addParameter(exp_parameter(i18n("Automatically escape spaces"), "__no", false));
}
TagString exp_ListFile::expFunc(const KrPanel *panel, const QStringList &parameter, const bool &useUrl, Expander &exp) const
{
    NEED_PANEL

    // get selected items from view
    QStringList items;
    QString mask;

    if (parameter.count() <= 3 || parameter[3].isEmpty())
        mask = '*';
    else
        mask = parameter[3];
    QTemporaryFile tmpFile(QDir::tempPath() + QLatin1String("/krusader_XXXXXX.itemlist"));
    tmpFile.setAutoRemove(false);

    if (!tmpFile.open()) {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_WORLD, i18n("Expander: temporary file could not be opened (%1)", tmpFile.errorString())));
        return QString();
    }

    QTextStream stream(&tmpFile);
    stream << separateAndQuote(fileList(panel,
                                        parameter.isEmpty() ? QString() : parameter[0].toLower(),
                                        mask,
                                        parameter.count() > 2 ? parameter[2].toLower() == "yes" : false,
                                        useUrl,
                                        exp,
                                        "ListFile"),
                               parameter.count() > 1 ? parameter[1] : "\n",
                               parameter.count() > 4 ? parameter[4].toLower() == "yes" : true)
           << "\n";
    tmpFile.close();

    return tmpFile.fileName();
}

exp_Select::exp_Select()
{
    _expression = "Select";
    _description = i18n("Manipulate the Selection...");
    _needPanel = true;

    addParameter(exp_parameter(i18n("Selection mask:"), "__select", true));
    addParameter(exp_parameter(i18n("Manipulate in which way:"), "__choose:Set;Add;Remove", false));
}
TagString exp_Select::expFunc(const KrPanel *panel, const QStringList &parameter, const bool &, Expander &exp) const
{
    NEED_PANEL

    KrQuery mask;
    if (parameter.count() <= 0 || parameter[0].isEmpty())
        mask = KrQuery("*");
    else
        mask = KrQuery(parameter[0]);

    if (parameter.count() > 1 && parameter[1].toLower() == "list-add")
        panel->view->select(mask);
    else if (parameter.count() > 1 && parameter[1].toLower() == "list-remove")
        panel->view->unselect(mask);
    else { // parameter[1].toLower() == "set" or isEmpty() or whatever
        panel->view->unselect(KrQuery("*"));
        panel->view->select(mask);
    }

    return QString(); // this doesn't return anything, that's normal!
}

exp_Goto::exp_Goto()
{
    _expression = "Goto";
    _description = i18n("Jump to a Location...");
    _needPanel = true;

    addParameter(exp_parameter(i18n("Choose a path:"), "__goto", true));
    addParameter(exp_parameter(i18n("Open location in a new tab"), "__no", false));
}
TagString exp_Goto::expFunc(const KrPanel *panel, const QStringList &parameter, const bool &, Expander &exp) const
{
    NEED_PANEL

    bool newTab = false;
    if (parameter.count() > 1 && parameter[1].toLower() == "yes")
        newTab = true;

    if (parameter.count() == 0) {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_ARGUMENT, i18n("Expander: at least 1 parameter is required for Goto.")));
        return QString();
    }

    QUrl url = QUrl::fromUserInput(parameter[0], QString(), QUrl::AssumeLocalFile);
    if (newTab) {
        if (panel == LEFT_PANEL)
            MAIN_VIEW->leftManager()->slotNewTab(url);
        else
            MAIN_VIEW->rightManager()->slotNewTab(url);
    } else {
        panel->func->openUrl(url, "");
        panel->gui->slotFocusOnMe();
    }

    return QString(); // this doesn't return anything, that's normal!
}

/*
exp_Search::exp_Search() {
   _expression = "Search";
   _description = i18n("Search for files");
   _needPanel = true;

   addParameter( new exp_parameter( i18n("please choose the setting"), "__searchprofile", true ) );
   addParameter( new exp_parameter( i18n("open the search in a new tab"), "__yes", false ) );  //TODO: add this also to panel-dependent as soon as filesystem
support the display of search-results
}
*/

exp_Ask::exp_Ask()
{
    _expression = "Ask";
    _description = i18n("Ask Parameter from User...");
    _needPanel = false;

    addParameter(exp_parameter(i18n("Question:"), "Where do you want do go today?", true));
    addParameter(exp_parameter(i18n("Preset (optional):"), "", false));
    addParameter(exp_parameter(i18n("Caption (optional):"), "", false));
}
TagString exp_Ask::expFunc(const KrPanel *, const QStringList &parameter, const bool &, Expander &exp) const
{
    QString caption, preset, result;

    if (parameter.count() == 0) {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_ARGUMENT, i18n("Expander: at least 1 parameter is required for Ask.")));
        return QString();
    }

    if (parameter.count() <= 2 || parameter[2].isEmpty())
        caption = i18n("User Action");
    else
        caption = parameter[2];
    if (parameter.count() <= 1 || parameter[1].isEmpty())
        preset.clear();
    else
        preset = parameter[1];

    bool ok;
    result = QInputDialog::getText(krMainWindow, caption, parameter[0], QLineEdit::Normal, preset, &ok);

    if (ok)
        return result;
    else {
        // user cancelled
        setError(exp, Error(Error::exp_S_ERROR, Error::exp_C_USER));
        return QString();
    }
}

exp_Clipboard::exp_Clipboard()
{
    _expression = "Clipboard";
    _description = i18n("Copy to Clipboard...");
    _needPanel = false;

    addParameter(exp_parameter(i18n("What to copy:"), "__placeholder", true));
    addParameter(exp_parameter(i18n("Append to current clipboard content with this separator (optional):"), "", false));
}
TagString exp_Clipboard::expFunc(const KrPanel *, const TagStringList &parameter, const bool &, Expander &exp) const
{
    //    qDebug() << "Expander::exp_Clipboard, parameter[0]: '" << parameter[0] << "', Clipboard: " << QApplication::clipboard()->text();
    if (parameter.count() == 0) {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_ARGUMENT, i18n("Expander: at least 1 parameter is required for Clipboard.")));
        return QString();
    }

    QStringList lst = splitEach(parameter[0]);
    if (parameter.count() > 1 && !parameter[1].isSimple()) {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_SYNTAX, i18n("Expander: %Each% may not be in the second argument of %Clipboard%")));
        return QString();
    }
    if (parameter.count() <= 1 || parameter[1].string().isEmpty() || QApplication::clipboard()->text().isEmpty())
        QApplication::clipboard()->setText(lst.join("\n"));
    else
        QApplication::clipboard()->setText(QApplication::clipboard()->text() + parameter[1].string() + lst.join("\n"));

    return QString(); // this doesn't return anything, that's normal!
}

exp_Copy::exp_Copy()
{
    _expression = "Copy";
    _description = i18n("Copy a File/Folder...");
    _needPanel = false;

    addParameter(exp_parameter(i18n("What to copy:"), "__placeholder", true));
    addParameter(exp_parameter(i18n("Where to copy:"), "__placeholder", true));
}
TagString exp_Copy::expFunc(const KrPanel *, const TagStringList &parameter, const bool &, Expander &exp) const
{
    if (parameter.count() < 2) {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_ARGUMENT, i18n("Expander: at least 2 parameter is required for Copy.")));
        return QString();
    }

    // basically the parameter can already be used as URL, but since QUrl has problems with ftp-proxy-urls (like ftp://username@proxyusername@url...) this is
    // necessary:
    const QStringList sourceList = splitEach(parameter[0]);
    QList<QUrl> sourceURLs;
    for (const QString &source : sourceList) {
        sourceURLs.append(QUrl::fromUserInput(source, QString(), QUrl::AssumeLocalFile));
    }

    if (!parameter[1].isSimple()) {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_SYNTAX, i18n("Expander: %Each% may not be in the second argument of %Copy%")));
        return QString();
    }

    // or transform(...) ?
    const QUrl dest = QUrl::fromUserInput(parameter[1].string(), QString(), QUrl::AssumeLocalFile);

    if (!dest.isValid() || find_if(sourceURLs.constBegin(), sourceURLs.constEnd(), std::not_fn(std::bind(std::mem_fn(&QUrl::isValid), std::placeholders::_1))) != sourceURLs.constEnd()) {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_ARGUMENT, i18n("Expander: invalid URLs in %_Copy(\"src\", \"dest\")%")));
        return QString();
    }

    FileSystemProvider::instance().startCopyFiles(sourceURLs, dest);

    return QString(); // this doesn't return everything, that's normal!
}

exp_Move::exp_Move()
{
    _expression = "Move";
    _description = i18n("Move/Rename a File/Folder...");
    _needPanel = false;

    addParameter(exp_parameter(i18n("What to move/rename:"), "__placeholder", true));
    addParameter(exp_parameter(i18n("New target/name:"), "__placeholder", true));
}
TagString exp_Move::expFunc(const KrPanel *, const TagStringList &parameter, const bool &, Expander &exp) const
{
    if (parameter.count() < 2) {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_ARGUMENT, i18n("Expander: at least 2 parameter is required for Move.")));
        return QString();
    }

    // basically the parameter can already be used as URL, but since QUrl has problems with ftp-proxy-urls (like ftp://username@proxyusername@url...) this is
    // necessary:
    QStringList lst = splitEach(parameter[0]);
    if (!parameter[1].isSimple()) {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_SYNTAX, i18n("%Each% may not be in the second argument of %Move%")));
        return QString();
    }
    QList<QUrl> src;
    for (QStringList::const_iterator it = lst.constBegin(), end = lst.constEnd(); it != end; ++it)
        src.push_back(QUrl::fromUserInput(*it, QString(), QUrl::AssumeLocalFile));
    // or transform(...) ?
    QUrl dest = QUrl::fromUserInput(parameter[1].string(), QString(), QUrl::AssumeLocalFile);

    if (!dest.isValid() || find_if(src.constBegin(), src.constEnd(), std::not_fn(std::bind(std::mem_fn(&QUrl::isValid), std::placeholders::_1))) != src.constEnd()) {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_ARGUMENT, i18n("Expander: invalid URLs in %_Move(\"src\", \"dest\")%")));
        return QString();
    }

    FileSystemProvider::instance().startCopyFiles(src, dest, KIO::CopyJob::Move);

    return QString(); // this doesn't return anything, that's normal!
}

#ifdef SYNCHRONIZER_ENABLED
exp_Sync::exp_Sync()
{
    _expression = "Sync";
    _description = i18n("Load a Synchronizer Profile...");
    _needPanel = false;

    addParameter(exp_parameter(i18n("Choose a profile:"), "__syncprofile", true));
}
TagString exp_Sync::expFunc(const KrPanel *, const QStringList &parameter, const bool &, Expander &exp) const
{
    if (parameter.count() == 0 || parameter[0].isEmpty()) {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_ARGUMENT, i18n("Expander: no profile specified for %_Sync(profile)%")));
        return QString();
    }

    SynchronizerGUI *synchronizerDialog = new SynchronizerGUI(MAIN_VIEW, parameter[0]);
    synchronizerDialog->show(); // destroyed on close

    return QString(); // this doesn't return everything, that's normal!
}
#endif

exp_NewSearch::exp_NewSearch()
{
    _expression = "NewSearch";
    _description = i18n("Load a Searchmodule Profile...");
    _needPanel = false;

    addParameter(exp_parameter(i18n("Choose a profile:"), "__searchprofile", true));
}
TagString exp_NewSearch::expFunc(const KrPanel *, const QStringList &parameter, const bool &, Expander &exp) const
{
    if (parameter.count() == 0 || parameter[0].isEmpty()) {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_ARGUMENT, i18n("Expander: no profile specified for %_NewSearch(profile)%")));
        return QString();
    }

    new KrSearchDialog(parameter[0], krApp);

    return QString(); // this doesn't return everything, that's normal!
}

exp_Profile::exp_Profile()
{
    _expression = "Profile";
    _description = i18n("Load a Panel Profile...");
    _needPanel = false;

    addParameter(exp_parameter(i18n("Choose a profile:"), "__panelprofile", true));
}
TagString exp_Profile::expFunc(const KrPanel *, const QStringList &parameter, const bool &, Expander &exp) const
{
    if (parameter.count() == 0 || parameter[0].isEmpty()) {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_ARGUMENT, i18n("Expander: no profile specified for %_Profile(profile)%; abort...")));
        return QString();
    }

    MAIN_VIEW->profiles(parameter[0]);

    return QString(); // this doesn't return everything, that's normal!
}

exp_Each::exp_Each()
{
    _expression = "Each";
    _description = i18n("Separate Program Call for Each...");
    _needPanel = true;

    addParameter(exp_parameter(i18n("Which items:"), "__choose:All;Files;Dirs;Selected", false));
    addParameter(exp_parameter(i18n("Omit the current path (optional)"), "__no", false));
    addParameter(exp_parameter(i18n("Mask (optional, all but 'Selected'):"), "__select", false));
    addParameter(exp_parameter(i18n("Automatically escape spaces"), "__yes", false));
}
TagString exp_Each::expFunc(const KrPanel *panel, const QStringList &parameter, const bool &useUrl, Expander &exp) const
{
    NEED_PANEL

    QString mask;
    if (parameter.count() <= 2 || parameter[2].isEmpty())
        mask = '*';
    else
        mask = parameter[2];

    TagString ret;
    QStringList l = fileList(panel,
                             parameter.empty() ? QString() : parameter[0].toLower(),
                             mask,
                             parameter.count() > 1 && parameter[1].toLower() == "yes",
                             useUrl,
                             exp,
                             "Each");

    if (!(parameter.count() <= 3 || parameter[3].toLower() != "yes"))
        transform(l.begin(), l.end(), l.begin(), bashquote);

    ret.insertTag(0, l);
    return ret;
}

exp_ColSort::exp_ColSort()
{
    _expression = "ColSort";
    _description = i18n("Set Sorting for This Panel...");
    _needPanel = true;

    addParameter(exp_parameter(i18n("Choose a column:"), "__choose:Name;Ext;Type;Size;Modified;Perms;rwx;Owner;Group", true));
    addParameter(exp_parameter(i18n("Choose a sort sequence:"), "__choose:Toggle;Asc;Desc", false));
}
TagString exp_ColSort::expFunc(const KrPanel *panel, const QStringList &parameter, const bool &, Expander &exp) const
{
    NEED_PANEL

    if (parameter.count() == 0 || parameter[0].isEmpty()) {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_ARGUMENT, i18n("Expander: no column specified for %_ColSort(column)%")));
        return QString();
    }

    KrViewProperties::ColumnType oldColumn = panel->view->properties()->sortColumn;
    KrViewProperties::ColumnType column = oldColumn;

    if (parameter[0].toLower() == "name") {
        column = KrViewProperties::Name;
    } else if (parameter[0].toLower() == "ext") {
        column = KrViewProperties::Ext;
    } else if (parameter[0].toLower() == "type") {
        column = KrViewProperties::Type;
    } else if (parameter[0].toLower() == "size") {
        column = KrViewProperties::Size;
    } else if (parameter[0].toLower() == "modified") {
        column = KrViewProperties::Modified;
    } else if (parameter[0].toLower() == "changed") {
        column = KrViewProperties::Changed;
    } else if (parameter[0].toLower() == "accessed") {
        column = KrViewProperties::Accessed;
    } else if (parameter[0].toLower() == "perms") {
        column = KrViewProperties::Permissions;
    } else if (parameter[0].toLower() == "rwx") {
        column = KrViewProperties::KrPermissions;
    } else if (parameter[0].toLower() == "owner") {
        column = KrViewProperties::Owner;
    } else if (parameter[0].toLower() == "group") {
        column = KrViewProperties::Group;
    } else {
        setError(exp, Error(Error::exp_S_WARNING, Error::exp_C_ARGUMENT, i18n("Expander: unknown column specified for %_ColSort(%1)%", parameter[0])));
        return QString();
    }

    bool descending = panel->view->properties()->sortOptions & KrViewProperties::Descending;

    if (parameter.count() <= 1 || (parameter[1].toLower() != "asc" && parameter[1].toLower() != "desc")) { // no sortdir parameter
        if (column == oldColumn) // reverse direction if column is unchanged
            descending = !descending;
        else // otherwise set to ascending
            descending = false;
    } else { // sortdir specified
        if (parameter[1].toLower() == "asc")
            descending = false;
        else // == desc
            descending = true;
    }

    panel->view->setSortMode(column, descending);

    return QString(); // this doesn't return anything, that's normal!
}

exp_PanelSize::exp_PanelSize()
{
    _expression = "PanelSize";
    _description = i18n("Set Relation Between the Panels...");
    _needPanel = true;

    addParameter(exp_parameter(i18n("Set the new size in percent:"), "__int:0;100;5;50", true));
}
TagString exp_PanelSize::expFunc(const KrPanel *panel, const QStringList &parameter, const bool &, Expander &exp) const
{
    NEED_PANEL
    int newSize;

    if (parameter.count() == 0 || parameter[0].isEmpty())
        newSize = 50; // default is 50%
    else
        newSize = parameter[0].toInt();

    if (newSize < 0 || newSize > 100) {
        setError(exp,
                 Error(Error::exp_S_FATAL,
                       Error::exp_C_ARGUMENT,
                       i18n("Expander: Value %1 out of range for %_PanelSize(percent)%. The first parameter has to be >0 and <100", newSize)));
        return QString();
    }

    MAIN_VIEW->setPanelSize(panel->isLeft(), newSize);

    return QString(); // this doesn't return everything, that's normal!
}

exp_View::exp_View()
{
    _expression = "View";
    _description = i18n("View File with Krusader's Internal Viewer...");
    _needPanel = false;

    addParameter(exp_parameter(i18n("Which file to view (normally '%aCurrent%'):"), "__placeholder", true));
    addParameter(exp_parameter(i18n("Choose a view mode:"), "__choose:generic;text;hex", false));
    // addParameter( exp_parameter( i18n("Choose a window-mode"), "__choose:tab;window;panel", false ) );
    // TODO: window-mode 'panel' should open the file in the third-hand viewer
    addParameter(exp_parameter(i18n("Choose a window mode:"), "__choose:tab;window", false));
}
TagString exp_View::expFunc(const KrPanel *, const QStringList &parameter, const bool &, Expander &exp) const
{
    if (parameter.count() == 0 || parameter[0].isEmpty()) {
        setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_ARGUMENT, i18n("Expander: no file to view in %_View(filename)%")));
        return QString();
    }

    QString viewMode, windowMode;
    if (parameter.count() <= 1 || parameter[1].isEmpty())
        viewMode = "generic";
    else
        viewMode = parameter[1];

    if (parameter.count() <= 2 || parameter[2].isEmpty())
        windowMode = "tab";
    else
        windowMode = parameter[2];

    KrViewer::Mode mode = KrViewer::Generic;
    if (viewMode == "text")
        mode = KrViewer::Text;
    else if (viewMode == "hex")
        mode = KrViewer::Hex;

    QUrl url = QUrl::fromUserInput(parameter[0], QString(), QUrl::AssumeLocalFile);
    KrViewer::view(url, mode, (windowMode == "window"));
    // TODO: Call the viewer with viewMode and windowMode. Filename is in parameter[0].
    //  It would be nice if parameter[0] could also be a space-separated filename-list (provided if the first parameter is %aList(selected)%)

    return QString(); // this doesn't return everything, that's normal!
}

/////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// end of expander classes ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

TagString exp_simpleplaceholder::expFunc(const KrPanel *p, const TagStringList &parameter, const bool &useUrl, Expander &exp) const
{
    QStringList lst;
    for (const auto &it : parameter)
        if (it.isSimple())
            lst.push_back(it.string());
        else {
            setError(exp, Error(Error::exp_S_FATAL, Error::exp_C_SYNTAX, i18n("%Each% is not allowed in parameter to %1", description())));
            return QString();
        }
    return expFunc(p, lst, useUrl, exp);
}

}

KrPanel *Expander::getPanel(const char panelIndicator, const exp_placeholder *pl, Expander &exp)
{
    switch (panelIndicator) {
    case 'a':
        return ACTIVE_PANEL;
    case 'o':
        return OTHER_PANEL;
    case 'l':
        return LEFT_PANEL;
    case 'r':
        return RIGHT_PANEL;
    case '_':
        return nullptr;
    default:
        exp.setError(
            Error(Error::exp_S_FATAL, Error::exp_C_SYNTAX, i18n("Expander: Bad panel specifier %1 in placeholder %2", panelIndicator, pl->description())));
        return nullptr;
    }
}

void Expander::expand(const QString &stringToExpand, bool useUrl)
{
    TagString result = expandCurrent(stringToExpand, useUrl);
    if (error())
        return;

    if (!result.isSimple())
        resultList = splitEach(result);
    else
        resultList.append(result.string());

    //    qWarning() << resultList[0];
}

TagString Expander::expandCurrent(const QString &stringToExpand, bool useUrl)
{
    TagString result;
    QString exp;
    TagString tmpResult;
    qsizetype begin, end, i;
    //    int brackets = 0;
    //    bool inQuotes = false;
    qsizetype idx = 0;
    while (idx < stringToExpand.length()) {
        if ((begin = stringToExpand.indexOf('%', idx)) == -1)
            break;
        if ((end = findEnd(stringToExpand, begin)) == -1) {
            // xgettext:no-c-format
            setError(Error(Error::exp_S_FATAL, Error::exp_C_SYNTAX, i18n("Error: unterminated % in Expander")));
            return QString();
        }

        result += stringToExpand.mid(idx, begin - idx); // copy until the start of %exp%

        // get the expression, and expand it using the correct expander function
        exp = stringToExpand.mid(begin + 1, end - begin - 1);
        //       qDebug() << "------------- exp: '" << exp << "'";
        if (exp.isEmpty())
            result += QString(QChar('%'));
        else {
            TagStringList parameter = separateParameter(&exp, useUrl);
            if (error())
                return QString();
            char panelIndicator = exp.toLower()[0].toLatin1();
            exp.replace(0, 1, "");
            for (i = 0; i < placeholderCount(); ++i)
                if (exp == placeholder(i)->expression()) {
                    //               qDebug() << "---------------------------------------";
                    tmpResult = placeholder(i)->expFunc(getPanel(panelIndicator, placeholder(i), *this), parameter, useUrl, *this);
                    if (error()) {
                        return QString();
                    } else
                        result += tmpResult;
                    //               qDebug() << "---------------------------------------";
                    break;
                }
            if (i == placeholderCount()) { // didn't find an expander
                setError(Error(Error::exp_S_FATAL, Error::exp_C_SYNTAX, i18n("Error: unrecognized %%%1%2%% in Expander", panelIndicator, exp)));
                return QString();
            }
        } // else
        idx = end + 1;
    }
    // copy the rest of the string
    result += stringToExpand.mid(idx);
    //    qDebug() << "============== result '" << result << "'";
    return result;
}

QStringList Expander::splitEach(TagString stringToSplit)
{
    if (stringToSplit.isSimple()) {
        //   qWarning() << stringToSplit.string();
        QStringList l;
        l << stringToSplit.string();
        return l;
    }
    pair<uint, QStringList> pl = *stringToSplit.tagsBegin();
    stringToSplit.eraseTag(stringToSplit.tagsBegin());
    QStringList ret;
    for (QStringList::const_iterator it = pl.second.constBegin(), end = pl.second.constEnd(); it != end; ++it) {
        TagString s = stringToSplit;
        s.insert(pl.first, *it);
        ret += splitEach(s);
    }
    return ret;
    //    qDebug() << "stringToSplit: " << stringToSplit;
}

TagStringList Expander::separateParameter(QString *const exp, bool useUrl)
{
    TagStringList parameter;
    QStringList parameter1;
    QString result;
    qsizetype begin, end;
    if ((begin = exp->indexOf('(')) != -1) {
        if ((end = exp->lastIndexOf(')')) == -1) {
            setError(Error(Error::exp_S_FATAL, Error::exp_C_SYNTAX, i18n("Error: missing ')' in Expander")));
            return TagStringList();
        }
        result = exp->mid(begin + 1, end - begin - 1);
        *exp = exp->left(begin);

        bool inQuotes = false;
        int idx = 0;
        begin = 0;
        while (idx < result.length()) {
            if (result[idx].toLatin1() == '\\') {
                if (result[idx + 1].toLatin1() == '"')
                    result.replace(idx, 1, "");
            }
            if (result[idx].toLatin1() == '"')
                inQuotes = !inQuotes;
            if (result[idx].toLatin1() == ',' && !inQuotes) {
                parameter1.append(result.mid(begin, idx - begin));
                begin = idx + 1;
                //             qWarning() << " ---- parameter: " << parameter.join(";");
            }
            idx++;
        }
        parameter1.append(result.mid(begin, idx - begin)); // don't forget the last one

        for (auto &it : parameter1) {
            it = it.trimmed();
            if (it.left(1) == "\"")
                it = it.mid(1, it.length() - 2);
            parameter.push_back(expandCurrent(it, useUrl));
            if (error())
                return TagStringList();
        }
    }

    //    qWarning() << "------- exp: " << *exp << " ---- parameter: " << parameter.join(";");
    return parameter;
}

qsizetype Expander::findEnd(const QString &str, qsizetype start)
{
    qsizetype end = str.indexOf('%', start + 1);
    if (end == -1)
        return end;
    qsizetype bracket = str.indexOf('(', start + 1);
    if (end < bracket || bracket == -1)
        return end;

    qsizetype idx = bracket + 1;
    bool inQuotes = false;
    int depth = 1;
    while (idx < str.length()) {
        switch (str[idx].toLatin1()) {
        case '\\':
            idx++;
            break;
        case '"':
            inQuotes = !inQuotes;
            break;
        case '(':
            if (!inQuotes)
                depth++;
            break;
        case ')':
            if (!inQuotes)
                --depth;
            break;
        case '%':
            if (depth == 0)
                return idx;
        } // switch
        idx++;
    } // while
    // failsafe
    return -1;
}

QList<const exp_placeholder *> &Expander::_placeholder()
{
    static QList<const exp_placeholder *> ret;
    if (!ret.count()) {
        ret << new exp_View;
        ret << new exp_PanelSize;
        ret << new exp_ColSort;
        ret << new exp_Each;
        ret << new exp_Profile;
        ret << new exp_NewSearch;
#ifdef SYNCHRONIZER_ENABLED
        ret << new exp_Sync;
#endif
        ret << new exp_Move;
        ret << new exp_Copy;
        ret << new exp_Goto;
        ret << new exp_Select;
        ret << new exp_Clipboard;
        ret << new exp_Ask;
        ret << new exp_ListFile;
        ret << new exp_List;
        ret << new exp_Current;
        ret << new exp_Filter;
        ret << new exp_Count;
        ret << new exp_Path;
    }
    return ret;
}
