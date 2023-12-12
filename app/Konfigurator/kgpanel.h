/*
    SPDX-FileCopyrightText: 2003 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPANEL_H
#define KGPANEL_H

#include "konfiguratorpage.h"

class KrTreeWidget;
class KrViewInstance;

class KgPanel : public KonfiguratorPage
{
    Q_OBJECT

public:
    explicit KgPanel(bool first, QWidget *parent = nullptr);

    int activeSubPage() override;
    bool apply() override;

protected:
    KonfiguratorCheckBoxGroup *panelToolbarButtonsCheckboxes;
    KonfiguratorCheckBoxGroup *buttonsCheckboxes;
    KonfiguratorRadioButtons *mouseRadio;
    KonfiguratorCheckBoxGroup *mouseCheckboxes;
    KrTreeWidget *mousePreview;

protected slots:
    void slotEnablePanelToolbar();
    void slotSelectionModeChanged();
    void slotMouseCheckBoxChanged();

private:
    void setupGeneralTab();
    void setupPanelTab();
    void setupButtonsTab();
    void setupMouseModeTab();
    void setupMediaMenuTab();
    void setupLayoutTab();
    void setupView(KrViewInstance *instance, QWidget *parent);
    void createIgnoredMountpointsList(QWidget *tab, QBoxLayout *tabLayout);
    void slotAddMountpoint();
    void slotRemoveMountpoint();
    QTabWidget *tabWidget;
    KonfiguratorListBox *listBox;
};

#endif /* __KGPANEL_H__ */
