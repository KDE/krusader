/*
    SPDX-FileCopyrightText: 2005 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filterdialog.h"
#include "filtertabs.h"
#include "generalfilter.h"

// QtWidgets
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QPushButton>

#include <KLocalizedString>
#include <utility>

FilterDialog::FilterDialog(QWidget *parent, const QString &caption, QStringList extraOptions, bool modal)
    : QDialog(parent)
{
    setWindowTitle(caption.isNull() ? i18n("Krusader::Choose Files") : caption);
    setModal(modal);

    auto *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    auto *filterWidget = new QTabWidget;

    filterTabs = FilterTabs::addTo(filterWidget, FilterTabs::HasProfileHandler, std::move(extraOptions));
    generalFilter = dynamic_cast<GeneralFilter *>(filterTabs->get("GeneralFilter"));

    mainLayout->addWidget(filterWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Reset);
    mainLayout->addWidget(buttonBox);

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &FilterDialog::slotOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FilterDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &FilterDialog::slotReset);
    connect(filterTabs, &FilterTabs::closeRequest, this, &FilterDialog::slotCloseRequest);

    generalFilter->searchFor->setFocus();

    if (modal)
        exec();
}

void FilterDialog::applySettings(const FilterSettings &s)
{
    filterTabs->applySettings(s);
}

KrQuery FilterDialog::getQuery()
{
    return settings.toQuery();
}

bool FilterDialog::isExtraOptionChecked(QString name)
{
    return filterTabs->isExtraOptionChecked(std::move(name));
}

void FilterDialog::checkExtraOption(QString name, bool check)
{
    filterTabs->checkExtraOption(std::move(name), check);
}

void FilterDialog::slotCloseRequest(bool doAccept)
{
    if (doAccept) {
        slotOk();
        accept();
    } else
        reject();
}

void FilterDialog::slotReset()
{
    filterTabs->reset();
    generalFilter->searchFor->setFocus();
}

void FilterDialog::slotOk()
{
    settings = filterTabs->getSettings();
    if (settings.isValid())
        accept();
}
