/* **************************************************************************
                       konfiguratorpage.h  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __KONFIGURATOR_PAGE_H__
#define __KONFIGURATOR_PAGE_H__
 
#include "konfiguratoritems.h"
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <QGridLayout>

struct KONFIGURATOR_CHECKBOX_PARAM;
struct KONFIGURATOR_NAME_VALUE_TIP;
struct KONFIGURATOR_NAME_VALUE_PAIR;

/**
  * KonfiguratorPage is responsible for handling pages in Konfigurator.
  * It provides simple methods for create and manage Konfigurator pages.
  *
  * @short The base class of a page in Konfigurator
  */  
class KonfiguratorPage : public QFrame
{
  Q_OBJECT
   
public:
/**
  * The constructor of the KonfiguratorPage class.
  *
  * @param firstTime    this parameter is true if it is the first call of Konfigurator
  * @param parent       reference to the parent widget
  */
  KonfiguratorPage( bool firstTime, QWidget* parent );

  /**
    * Applies the changes in the Konfigurator page.
    *
    * Writes out all relevent information to the konfiguration object and synchronizes
    * it with the file storage (hard disk, krusaderrc file). This function calls the apply()
    * method of each konfigurator item and finally performs the synchronization.
    *
    * @return             a boolean value indicates that Krusader restart is needed for the correct change
    */
  virtual bool apply();

  /**
    * Sets every konfigurator item to its default value on the page.
    *
    * This method calls the setDefaults() method of each konfigurator item. This function
    * doesn't modify the current configuration, only the values of the GUI items. The
    * apply() method must be called for finalizing the changes.
    */
  virtual void setDefaults();

  /**
    * Reloads the original value of each konfigurator item from the configuration object.
    *
    * This function calls the loadInitialValue() method of each konfigurator item.
    * Used to rollback the changes on the konfigurator page. Called if the user
    * responds 'No' to the "Apply changes" question.
    */
  virtual void loadInitialValues();

  /**
    * Checks whether the page was changed.
    *
    * This function calls the isChanged() method of each konfigurator item and
    * performs logical OR operation on them. Actually, this function returns true
    * if any of the konfigurator items was changed.
    *
    * @return             true if at least one of the konfigurator items was changed
    */
  virtual bool isChanged();

  /**
    * Flag, indicates the first call of Konfigurator
    * @return             true if konfigurator was started at the first time
    */
  inline  bool isFirst()   {return firstCall;}

  /**
    * This method is used to query the active subpage from the Konfigurator
    * @return             the active page (by default the first page)
    */
  virtual int activeSubPage() {return FIRST_PAGE;}

  /**
    * Adds a new checkbox item to the page.
    * <br>The checkbox widget's name is QString(cls + "/" + name).ascii()<br>
    *
    * Sample:<br><br>
    * KonfiguratorCheckBox *myCheckBox = createCheckBox( "class", "name", false, parentWidget );<br>
    * myLayout->addWidget( myCheckBox, 0, 0 );
    *
    * @param  cls         The class name used in KConfig (ex. "Archives")
    * @param  name        The item name used in KConfig (ex. "Do Tar")
    * @param  dflt        The default value of the checkbox
    * @param  text        The text field of the checkbox
    * @param  parent      Reference to the parent widget
    * @param  rst         The change of this parameter requires Krusader restart
    * @param  toolTip     Tooltip used for this checkbox
    * @param  pg          The subpage of a Konfigurator page (because of setDefaults)
    *
    * @return             reference to the newly created checkbox
    */
  KonfiguratorCheckBox    *createCheckBox( QString cls, QString name, bool dflt,
                                           QString text, QWidget *parent=0, bool rst=false,
                                           QString toolTip = QString(), int pg=FIRST_PAGE );

  /**
    * Adds a new spinbox item to the page.
    * <br>The spinbox widget's name is QString(cls + "/" + name).ascii()<br>
    *
    * Sample:<br><br>
    * KonfiguratorSpinBox *mySpinBox = createSpinBox( "class", "name", 10, 1, 100, parentWidget );<br>
    * myLayout->addWidget( mySpinBox, 0, 0 );
    *
    * @param  cls         The class name used in KConfig (ex. "Archives")
    * @param  name        The item name used in KConfig (ex. "Do Tar")
    * @param  dflt        The default value of the spinbox
    * @param  min         The minimum value of the spinbox
    * @param  max         The maximum value of the spinbox
    * @param  parent      Reference to the parent widget
    * @param  rst         The change of this parameter requires Krusader restart
    * @param  pg          The subpage of a Konfigurator page (because of setDefaults)
    *
    * @return             reference to the newly created spinbox
    */
  KonfiguratorSpinBox     *createSpinBox(  QString cls, QString name, int dflt, int min,
                                           int max, QWidget *parent = 0, bool rst = false, int pg=FIRST_PAGE );

  /**
    * Adds a new editbox item to the page.
    * <br>The editbox widget's name is QString(cls + "/" + name).ascii()<br>
    *
    * Sample:<br><br>
    * KonfiguratorEditBox *myEditBox = createEditBox( "class", "name", "default", parentWidget );<br>
    * myLayout->addWidget( myEditBox, 0, 0 );
    *
    * @param  cls         The class name used in KConfig (ex. "Archives")
    * @param  name        The itemname used in KConfig (ex. "Do Tar")
    * @param  dflt        The default value of the editbox
    * @param  parent      Reference to the parent widget
    * @param  rst         The change of this parameter requires Krusader restart
    * @param  pg          The subpage of a Konfigurator page (because of setDefaults)
    *
    * @return             reference to the newly created editbox
    */
  KonfiguratorEditBox     *createEditBox(  QString cls, QString name, QString dflt,
                                           QWidget *parent=0, bool rst=false, int pg=FIRST_PAGE );

  /**
    * Adds a new listbox item to the page.
    * <br>The listbox widget's name is QString(cls + "/" + name).ascii()<br>
    *
    * Sample:<br><br>
    * QStringList valueList;<br>
    * valueList += "item";<br>
    * KonfiguratorListBox *myListBox = createListBox( "class", "name", valueList, parentWidget );<br>
    * myLayout->addWidget( myListBox, 0, 0 );
    *
    * @param  cls         The class name used in KConfig (ex. "Archives")
    * @param  name        The itemname used in KConfig (ex. "Do Tar")
    * @param  dflt        The default value of the listbox
    * @param  parent      Reference to the parent widget
    * @param  rst         The change of this parameter requires Krusader restart
    * @param  pg          The subpage of a Konfigurator page (because of setDefaults)
    *
    * @return             reference to the newly created editbox
    */
  KonfiguratorListBox     *createListBox(  QString cls, QString name, QStringList dflt,
                                           QWidget *parent=0, bool rst=false, int pg=FIRST_PAGE );

  /**
    * Adds a new URL requester item to the page.
    * <br>The URL requester widget's name is QString(cls + "/" + name).ascii()<br>
    *
    * Sample:<br><br>
    * KonfiguratorURLRequester *myURLRequester = createURLRequester( "class", "name", "default", parentWidget );<br>
    * myLayout->addWidget( myURLRequester, 0, 0 );
    *
    * @param  cls         The class name used in KConfig (ex. "Archives")
    * @param  name        The itemname used in KConfig (ex. "Do Tar")
    * @param  dflt        The default value of the URL requester
    * @param  text        The text field of the URL requester
    * @param  parent      Reference to the parent widget
    * @param  rst         The change of this parameter requires Krusader restart
    * @param  pg          The subpage of a Konfigurator page (because of setDefaults)
    *
    * @return             reference to the newly created URL requester
    */
  KonfiguratorURLRequester *createURLRequester(  QString cls, QString name,
                                           QString dflt, QWidget *parent, bool rst, int pg=FIRST_PAGE );

  /**
    * Adds a new font chooser item to the page.
    * <br>The font chooser widget's name is QString(cls + "/" + name).ascii()<br>
    *
    * Sample:<br><br>
    * KonfiguratorFontChooser *myFontChooser = createFontChooser( "class", "name", new QFont(), parentWidget );<br>
    * myLayout->addWidget( myFontChooser, 0, 0 );
    *
    * @param  cls         The class name used in KConfig (ex. "Archives")
    * @param  name        The item name used in KConfig (ex. "Do Tar")
    * @param  dflt        The default value of the font chooser
    * @param  parent      Reference to the parent widget
    * @param  rst         The change of this parameter requires Krusader restart
    * @param  pg          The subpage of a Konfigurator page (because of setDefaults)
    *
    * @return             reference to the newly created font chooser
    */
  KonfiguratorFontChooser *createFontChooser(  QString cls, QString name, QFont *dflt,
                                           QWidget *parent=0, bool rst=false, int pg=FIRST_PAGE );

  /**
    * Adds a new combobox item to the page.
    * <br>The combobox widget's name is QString(cls + "/" + name).ascii()<br>
    *
    * Sample:<br><br>
    * KONFIGURATOR_NAME_VALUE_PAIR comboInfo[] =<br>
    * &nbsp;{{ i18n( "combo text1" ), "value1" },<br>
    * &nbsp;&nbsp;{ i18n( "combo text2" ), "value2" },<br>
    * &nbsp;&nbsp;{ i18n( "combo text3" ), "value3" }};<br><br>
    * KonfiguratorComboBox *myComboBox = createComboBox( "class", "name", "value2", comboInfo, 3, parentWidget );<br>
    * myLayout->addWidget( myComboBox, 0, 0 );
    *
    * @param  cls         The class name used in KConfig (ex. "Archives")
    * @param  name        The item name used in KConfig (ex. "Do Tar")
    * @param  dflt        The default value of the combobox
    * @param  params      Pointer to the name-value pair array (combo elements)
    * @param  paramNum    Number of the combobox elements
    * @param  text        The text field of the combobox
    * @param  parent      Reference to the parent widget
    * @param  rst         The change of this parameter requires Krusader restart
    * @param  editable    Flag indicates that the combo can be edited
    * @param  pg          The subpage of a Konfigurator page (because of setDefaults)
    *
    * @return             reference to the newly created combobox
    */
  KonfiguratorComboBox    *createComboBox(  QString cls, QString name, QString dflt,
                                           KONFIGURATOR_NAME_VALUE_PAIR *params, int paramNum,
                                           QWidget *parent=0, bool rst=false, bool editable=false, int pg=FIRST_PAGE );

  /**
    * Creates a frame on the page.
    *
    * Sample:<br><br>
    * QGroupBox *myGroup = createFrame( i18n( "MyFrameName" ), parentWidget, "frameName" );<br>
    * myLayout->addWidget( myGroup, 0, 0 );
    *
    * @param  text        The text written out onto the frame
    * @param  parent      Reference to the parent widget
    *
    * @return             reference to the newly created frame
    */
  QGroupBox               *createFrame( QString text = QString(), QWidget *parent=0 );

  /**
    * Creates a new QGridLayout element and sets its margins.
    *
    * Sample:<br><br>
    * QGroupBox *myGroup = createFrame( i18n( "MyFrameName" ), parentWidget, "frameName" );<br>
    * QGridLayout *myLayout = createGridLayout( myGroup ) );<br>
    * myLayout->addWidget( myGroup, 0, 0 );
    *
    * @param  parent      Reference to the parent layout
    *
    * @return             reference to the newly created QGridLayout
    */
  QGridLayout             *createGridLayout( QWidget *parent );

  /**
    * Adds a new label to a grid layout.
    *
    * Sample:<br><br>
    * QGroupBox *myGroup = createFrame( i18n( "MyFrameName" ), parentWidget, "frameName" );<br>
    * QGridLayout *myLayout = createGridLayout( myGroup->layout() );<br>
    * addLabel( myLayout, 0, 0, i18n( "Hello world!" ), myGroup, "myLabel" );<br>
    * mainLayout->addWidget( myGroup, 0, 0 );
    *
    * @param  layout      The grid layout on which the item will be placed
    * @param  x           the column to which the label will be placed
    * @param  y           the row to which the label will be placed
    * @param  label       the text of the label
    * @param  parent      Reference to the parent widget
    *
    * @return             reference to the newly created label
    */
  QLabel                  *addLabel( QGridLayout *layout, int x, int y, QString label,
                                           QWidget *parent=0 );

  /**
    * Creates a spacer object (for justifying in QHBox).
    *
    * Sample:<br><br>
    * QHBox *hbox = new QHBox( myParent, "hbox" );<br>
    * createSpinBox( "class", "spin", 5, 1, 10, hbox );<br>
    * createSpacer( hbox );<br>
    * myLayout->addWidget( hbox, 0, 0 );
    *
    * @param  parent      Reference to the parent widget
    *
    * @return             reference to the newly created spacer widget
    */
  QWidget                 *createSpacer( QWidget *parent=0 );

  /**
    * Creates a separator line.
    *
    * Sample:<br><br>
    * QFrame *myLine = createLine( myParent, "myLine" );<br>
    * myLayout->addWidget( myLine, 1, 0 );<br>
    *
    * @param  parent      Reference to the parent widget
    * @param  vertical    Means vertical line
    *
    * @return             reference to the newly created spacer widget
    */
  QFrame                   *createLine( QWidget *parent=0, bool vertical = false );

  /**
    * Creates a checkbox group. A checkbox group contains a lot of checkboxes.
    * The grouped checkboxes are embedded into one widget, which can be placed anywhere
    * on the GUI. The placing of the elements can be horizontal or vertical in the group.
    * At horizontal placing the sizex integer defines the maximum element number in
    * one row, sizey is 0. At vertical placing sizex is 0, and sizey defines the
    * maximum row number in one column. <br>
    *
    * One specific element can be reached by its name or index with the find methods.
    * The first element is checkBoxGroup->find( 0 ), "myCb" element is checkBoxGroup->find( "myCb" ) ...
    *
    * Sample:<br><br>
    * KONFIGURATOR_CHECKBOX_PARAM myCBArray[] =<br>
    * &nbsp;{{"CbClass","CbName1", false, i18n( "name1" ), false, "tooltip1"},<br>
    * &nbsp;&nbsp;{"CbClass","CbName2", true, i18n( "name2" ), false, "tooltip2"},<br>
    * &nbsp;&nbsp;{"CbClass","CbName3", true, i18n( "name3" ), false, "tooltip3"}};<br><br>
    * KonfiguratorCheckBoxGroup *myCheckBoxGroup = createCheckBoxGroup( 1, 0, myCBArray, 3, myParent, "myCheckboxGroup" );<br>
    * myCheckBoxGroup->find( 0 )->setEnabled( false );<br><br>
    * myLayout->addWidget( myCheckBoxGroup, 0, 0 );<br>
    *
    * @param  sizex       the maximum column number at horizontal placing
    * @param  sizey       the maximum row number at vertical placing
    * @param  params      pointer to the checkbox array
    * @param  paramNum    number of the checkbox elements
    * @param  parent      Reference to the parent widget
    * @param  pg          The subpage of a Konfigurator page (because of setDefaults)
    *
    * @return             reference to the newly created checkbox group widget
    */
  KonfiguratorCheckBoxGroup *createCheckBoxGroup( int sizex, int sizey,
                                           KONFIGURATOR_CHECKBOX_PARAM *params, int paramNum,
                                           QWidget *parent=0, int pg=FIRST_PAGE );
  /**
    * Creates a radio button group. A radio button group contains a lot of radio buttons.
    * The grouped buttons are embedded into one widget, which can be placed anywhere
    * on the GUI. The placing of the elements can be horizontal or vertical in the group.
    * At horizontal placing the sizex integer defines the maximum element number in
    * one row, sizey is 0. At vertical placing sizex is 0, and sizey defines the
    * maximum row number in one column.<br>
    *
    * The references of the buttons can be accessed by the find methods of KonfiguratorRadioButtons.
    * The first element is myRadioGrp->find( 0 ), "myRadio" element is myRadioGrp->find( "myRadio" ) ...
    *
    * Sample:<br><br>
    * KONFIGURATOR_NAME_VALUE_TIP radioInfo[] =<br>
    * &nbsp;{{ i18n( "radio text1" ), "value1", i18n( "tooltip1" ) },<br>
    * &nbsp;&nbsp;{ i18n( "radio text2" ), "value2", i18n( "tooltip2" ) },<br>
    * &nbsp;&nbsp;{ i18n( "radio text3" ), "value3", i18n( "tooltip3" ) }};<br><br>
    * KonfiguratorRadioButtons *myRadioGroup = createRadioButtonGroup( "class", "name", "value1", 1, 0, radioInfo, 3, myParent, "myRadioGroup" );<br>
    * myRadioGroup->find( i18n( "radio text1" ) )->setEnabled( false );<br>
    * myLayout->addWidget( myRadioGroup, 0, 0 );<br>
    *
    * @param  cls         The class name used in KConfig (ex. "Archives")
    * @param  name        The item name used in KConfig (ex. "Do Tar")
    * @param  dflt        The default value of the radio buttons
    * @param  sizex       the maximum column number at horizontal placing
    * @param  sizey       the maximum row number at vertical placing
    * @param  params      pointer to the checkbox array
    * @param  paramNum    number of the checkbox elements
    * @param  parent      Reference to the parent widget
    * @param  rst         The change of this parameter requires Krusader restart
    * @param  pg          The subpage of a Konfigurator page (because of setDefaults)
    *
    * @return             reference to the newly created radio button group widget
    */
  KonfiguratorRadioButtons *createRadioButtonGroup( QString cls, QString name, 
                                           QString dflt, int sizex, int sizey,
                                           KONFIGURATOR_NAME_VALUE_TIP *params, int paramNum,
                                           QWidget *parent=0, bool rst=false, int pg=FIRST_PAGE );

  /**
    * This function is used to insert new, unknown items into KonfiguratorPage. The
    * item must be derived from KonfiguratorExtension class, which have
    * isChanged(), apply(), setDefaults, loadInitialValue() methods. After that, the
    * object is properly handled by Konfigurator page.
    *
    *
    * @param  item        The item to be added to KonfiguratorPage
    */
  void                    registerObject( KonfiguratorExtension *item );

  /**
    * This function is used to remove elements from KonfiguratorPage.
    *
    * Sample:<br><br>
    * KonfiguratorEditBox *myEditBox = createEditBox( "class", "name", "default", parentWidget );<br>
    * myLayout->addWidget( myEditBox, 0, 0 );<br>
    * removeObject( myEditBox->extension() );
    *
    * After the removeObject myEditBox will be untouched at apply(), setDefaults(), isChanged(),
    * loadInitialValues() methods of the KonfiguratorPage.
    *
    * @param  item        The item to be removed from KonfiguratorPage
    */
  void                    removeObject( KonfiguratorExtension *item );

  /**
    * Adds a new color chooser combobox item to the page.
    * <br>The chooser's widget's name is QString(cls + "/" + name).ascii()<br>
    *
    * Sample:<br><br>
    * KonfiguratorColorChooser *myColorChooser = createColorChooser( "class", "name", QColor( 255, 0, 255 ), parentWidget );<br>
    * myLayout->addWidget( myColorChooser, 0, 0 );
    *
    * @param  cls         The class name used in KConfig (ex. "Archives")
    * @param  name        The item name used in KConfig (ex. "Do Tar")
    * @param  dflt        The default value of the color chooser
    * @param  parent      Reference to the parent widget
    * @param  rst         The change of this parameter requires Krusader restart
    * @param  addColPtr   The additional color values
    * @param  rst         Number of additional colors
    * @param  pg          The subpage of a Konfigurator page (because of setDefaults)
    *
    * @return             reference to the newly created combobox
    */
  KonfiguratorColorChooser *createColorChooser(  QString cls, QString name, QColor dflt,
                                                 QWidget *parent=0, bool rst=false,
                                                 ADDITIONAL_COLOR *addColPtr = 0, int addColNum = 0, int pg=FIRST_PAGE );
signals:
  /**
    * The signal is emitted if the changed flag was modified in any konfigurator item.
    * Used for enabling/disabling the apply button.
    */
  void  sigChanged();
  
protected:
  QList<KonfiguratorExtension *> itemList;

private:
  bool  firstCall;
};

/**
  * KONFIGURATOR_CHECKBOX_PARAM is the basic item of checkbox arrays. It contains
  * every information related to a checkbox.
  */
struct KONFIGURATOR_CHECKBOX_PARAM
{
  /**
    * The class used in KConfig (ex. "Archives")
    */
  QString configClass;
  /**
    * The item name used in KConfig (ex. "Do Tar")
    */
  QString configName;
  /**
    * The default value of the checkbox
    */
  bool    defaultValue;
  /**
    * The text field of the checkbox
    */
  QString text;
  /**
    * The change of this parameter requires Krusader restart
    */
  bool    restart;
  /**
    * The checkbox's tooltip
    */
  QString toolTip;
};

#endif /* __KONFIGURATOR_PAGE_H__ */
