//
// C++ Implementation: useraction
//
// Description: This manages all useractions
//
//
// Author: Shie Erlich and Rafi Yanai <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "useractionproperties.h"


UserActionProperties::UserActionProperties() {
  //fill with defaults
  _execType = Normal;
  _descriptionUseTooltip = false;
  _separateStderr = false;
  _acceptURLs = false;
  _callEach = false;
  _confirmExecution = false;
}


void UserActionProperties::copyFrom( UserActionProperties* from ) {
  _name = *from->name();
  _title = *from->title();
  _category = *from->category();
  _icon = *from->icon();
  _tooltip = *from->tooltip();
  _description = *from->description();
  _command = *from->command();
  _startpath = *from->startpath();
  _showonlyProtocol = *from->showonlyProtocol();
  _showonlyPath = *from->showonlyPath();
  _showonlyMime = *from->showonlyMime();
  _showonlyFile = *from->showonlyFile();
  _execType = from->execType();
  _descriptionUseTooltip = from->descriptionUseTooltip();
  _separateStderr = from->separateStderr();
  _acceptURLs = from->acceptURLs();
  _callEach = from->callEach();
  _confirmExecution = from->confirmExecution();
  _defaultShortcut = *from->defaultShortcut();
}


////////////////////////// access functions //////////////////////////

QString* UserActionProperties::name() { return &_name; }
void UserActionProperties::setName(const QString& name) { _name = name; }

QString* UserActionProperties::title() { return &_title; }
void UserActionProperties::setTitle(const QString& title) { _title = title;}

QString* UserActionProperties::category() { return &_category; }
void UserActionProperties::setCategory(const QString& category) { _category = category;}

QString* UserActionProperties::icon() { return &_icon; }
void UserActionProperties::setIcon(const QString& icon) { _icon = icon;}

QString* UserActionProperties::tooltip() { return &_tooltip; }
void UserActionProperties::setTooltip(const QString& tooltip) { _tooltip = tooltip;}

QString* UserActionProperties::description() { return &_description; }
void UserActionProperties::setDescription(const QString& description) { _description = description;}

QString* UserActionProperties::command() { return &_command; }
void UserActionProperties::setCommand(const QString& command) { _command = command;}

QString* UserActionProperties::startpath() { return &_startpath; }
void UserActionProperties::setStartpath(const QString& startpath) { _startpath = startpath;}

QStringList* UserActionProperties::showonlyProtocol() { return &_showonlyProtocol; }
void UserActionProperties::setShowonlyProtocol(const QStringList& showonlyProtocol) { _showonlyProtocol = showonlyProtocol;}

QStringList* UserActionProperties::showonlyPath() { return &_showonlyPath; }
void UserActionProperties::setShowonlyPath(const QStringList& showonlyPath) { _showonlyPath = showonlyPath;}

QStringList* UserActionProperties::showonlyMime() { return &_showonlyMime; }
void UserActionProperties::setShowonlyMime(const QStringList& showonlyMime) { _showonlyMime = showonlyMime;}

QStringList* UserActionProperties::showonlyFile() { return &_showonlyFile; }
void UserActionProperties::setShowonlyFile(const QStringList& showonlyFile) { _showonlyFile = showonlyFile;}

UserActionProperties::ExecType UserActionProperties::execType() { return _execType; }
void UserActionProperties::setExecType(const ExecType& execType) { _execType = execType;}

bool UserActionProperties::descriptionUseTooltip() { return _descriptionUseTooltip; }
void UserActionProperties::setDescriptionUseTooltip(const bool& descriptionUseTooltip) { _descriptionUseTooltip = descriptionUseTooltip;}

bool UserActionProperties::separateStderr() { return _separateStderr; }
void UserActionProperties::setSeparateStderr(const bool& separateStderr) { _separateStderr = separateStderr;}

bool UserActionProperties::acceptURLs() { return _acceptURLs; }
void UserActionProperties::setAcceptURLs(const bool& acceptURLs) { _acceptURLs = acceptURLs;}

bool UserActionProperties::callEach() { return _callEach; }
void UserActionProperties::setCallEach(const bool& callEach) { _callEach = callEach;}

bool UserActionProperties::confirmExecution() { return _confirmExecution; }
void UserActionProperties::setConfirmExecution(const bool& confirmExecution) { _confirmExecution = confirmExecution;}

KShortcut* UserActionProperties::defaultShortcut() { return &_defaultShortcut; }
void UserActionProperties::setDefaultShortcut(const KShortcut& defaultShortcut) { _defaultShortcut = defaultShortcut;}

