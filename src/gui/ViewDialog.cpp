/*
 * Stellarium
 * Copyright (C) 2008 Fabien Chereau
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "Dialog.hpp"
#include "ViewDialog.hpp"
#include "StelMainWindow.hpp"
#include "ui_viewDialog.h"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelSkyCultureMgr.hpp"
#include "StelFileMgr.hpp"
#include "StelLocaleMgr.hpp"
#include "Mapping.hpp"
#include "Projector.hpp"
#include "LandscapeMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StarMgr.hpp"
#include "SkyDrawer.hpp"
#include "SolarSystem.hpp"
#include "NebulaMgr.hpp"
#include "MeteorMgr.hpp"

#include <QDebug>
#include <QFrame>
#include <QFile>

ViewDialog::ViewDialog() : dialog(NULL)
{
	ui = new Ui_viewDialogForm;
}

ViewDialog::~ViewDialog()
{
	delete ui;
}

void ViewDialog::close()
{
	emit closed();
}

void ViewDialog::setVisible(bool v)
{
	if (v) 
	{
		dialog = new DialogFrame(&StelMainWindow::getInstance());
		ui->setupUi(dialog);
		dialog->raise();
		dialog->move(200, 100);	// TODO: put in the center of the screen
		dialog->setVisible(true);
		connect(ui->closeView, SIGNAL(clicked()), this, SLOT(close()));
//		connect(ui->longitudeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(spinBoxChanged(void)));
		
		// Fill the culture list widget from the available list
		QListWidget* l = ui->culturesListWidget;
		l->clear();
		l->addItems(StelApp::getInstance().getSkyCultureMgr().getSkyCultureListI18());
		updateSkyCultureText();
		l->setCurrentItem(l->findItems(StelApp::getInstance().getSkyCultureMgr().getSkyCultureNameI18(), Qt::MatchExactly).at(0));
		connect(l, SIGNAL(currentTextChanged(const QString&)), this, SLOT(skyCultureChanged(const QString&)));
		
		// Fill the projection list
		l = ui->projectionListWidget;
		l->clear();
		const QMap<QString, const Mapping*>& mappings = StelApp::getInstance().getCore()->getProjection()->getAllMappings();
		QMapIterator<QString, const Mapping*> i(mappings);
		while (i.hasNext())
		{
			i.next();
			l->addItem(q_(i.value()->getNameI18()));
		}
		l->setCurrentItem(l->findItems(StelApp::getInstance().getCore()->getProjection()->getCurrentMapping().getNameI18(), Qt::MatchExactly).at(0));
		ui->projectionTextBrowser->setHtml(StelApp::getInstance().getCore()->getProjection()->getCurrentMapping().getHtmlSummary());
		connect(l, SIGNAL(currentTextChanged(const QString&)), this, SLOT(projectionChanged(const QString&)));
		
		// Fill the landscape list
		l = ui->landscapesListWidget;
		l->clear();
		LandscapeMgr* lmgr = (LandscapeMgr*)GETSTELMODULE("LandscapeMgr");
		l->addItems(lmgr->getAllLandscapeNames());
		l->setCurrentItem(l->findItems(lmgr->getCurrentLandscapeName(), Qt::MatchExactly).at(0));
		ui->landscapeTextBrowser->setHtml(lmgr->getCurrentLandscapeHtmlDescription());
		connect(l, SIGNAL(currentTextChanged(const QString&)), this, SLOT(landscapeChanged(const QString&)));
		
		// Connect and initialize checkboxes and other widgets
		
		// Stars section
		StarMgr* smgr = (StarMgr*)GETSTELMODULE("StarMgr");
		ui->showStarsCheckBox->setChecked(smgr->getFlagStars());
		QAction* a = StelMainWindow::getInstance().findChild<QAction*>("actionShow_Stars");
		connect(a, SIGNAL(toggled(bool)), ui->showStarsCheckBox, SLOT(setChecked(bool)));
		connect(ui->showStarsCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
		
		ui->starLabelCheckBox->setChecked(smgr->getFlagNames());
		connect(ui->starLabelCheckBox, SIGNAL(toggled(bool)), smgr, SLOT(setFlagNames(bool)));
		
		ui->starTwinkleCheckBox->setChecked(StelApp::getInstance().getCore()->getSkyDrawer()->getFlagTwinkle());
		connect(ui->starTwinkleCheckBox, SIGNAL(toggled(bool)), StelApp::getInstance().getCore()->getSkyDrawer(), SLOT(setFlagTwinkle(bool)));
		
		ui->starScaleRadiusDoubleSpinBox->setValue(StelApp::getInstance().getCore()->getSkyDrawer()->getScale());
		connect(ui->starScaleRadiusDoubleSpinBox, SIGNAL(valueChanged(double)), StelApp::getInstance().getCore()->getSkyDrawer(), SLOT(setScale(double)));
		
		ui->starTwinkleAmountDoubleSpinBox->setValue(StelApp::getInstance().getCore()->getSkyDrawer()->getTwinkleAmount());
		connect(ui->starTwinkleAmountDoubleSpinBox, SIGNAL(valueChanged(double)), StelApp::getInstance().getCore()->getSkyDrawer(), SLOT(setTwinkleAmount(double)));
		
		// Planets section
		SolarSystem* ssmgr = (SolarSystem*)GETSTELMODULE("SolarSystem");
		ui->showPlanetCheckBox->setChecked(ssmgr->getFlagPlanets());
		connect(ui->showPlanetCheckBox, SIGNAL(toggled(bool)), ssmgr, SLOT(setFlagPlanets(bool)));
		
		ui->planetMarkerCheckBox->setChecked(ssmgr->getFlagHints());
		a = StelMainWindow::getInstance().findChild<QAction*>("actionShow_Planets_Hints");
		connect(a, SIGNAL(toggled(bool)), ui->planetMarkerCheckBox, SLOT(setChecked(bool)));
		connect(ui->planetMarkerCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
		
		ui->planetLabelCheckBox->setChecked(ssmgr->getFlagLabels());
		connect(ui->planetLabelCheckBox, SIGNAL(toggled(bool)), ssmgr, SLOT(setFlagLabels(bool)));
		
		ui->planetScaleMoonCheckBox->setChecked(ssmgr->getFlagMoonScale());
		connect(ui->planetScaleMoonCheckBox, SIGNAL(toggled(bool)), ssmgr, SLOT(setFlagMoonScale(bool)));
		
		ui->planetOrbitCheckBox->setChecked(ssmgr->getFlagOrbits());
		connect(ui->planetOrbitCheckBox, SIGNAL(toggled(bool)), ssmgr, SLOT(setFlagOrbits(bool)));
		
		ui->planetLightSpeedCheckBox->setChecked(ssmgr->getFlagLightTravelTime());
		connect(ui->planetLightSpeedCheckBox, SIGNAL(toggled(bool)), ssmgr, SLOT(setFlagLightTravelTime(bool)));
		
		// Nebula section
		NebulaMgr* nmgr = (NebulaMgr*)GETSTELMODULE("NebulaMgr");
		ui->showNebulaCheckBox->setChecked(nmgr->getFlagShow());
		connect(ui->showNebulaCheckBox, SIGNAL(toggled(bool)), nmgr, SLOT(setFlagShow(bool)));
		
		// Shouting stars section
		MeteorMgr* mmgr = (MeteorMgr*)GETSTELMODULE("MeteorMgr");
		ui->showShoutingStarsCheckBox->setChecked(mmgr->getFlagShow());
		connect(ui->showShoutingStarsCheckBox, SIGNAL(toggled(bool)), mmgr, SLOT(setFlagShow(bool)));
		
		connect(ui->zhr10, SIGNAL(clicked()), this, SLOT(shoutingStarsZHRChanged()));
		connect(ui->zhr80, SIGNAL(clicked()), this, SLOT(shoutingStarsZHRChanged()));
		connect(ui->zhr10000, SIGNAL(clicked()), this, SLOT(shoutingStarsZHRChanged()));
		connect(ui->zhr144000, SIGNAL(clicked()), this, SLOT(shoutingStarsZHRChanged()));
	}
	else
	{
		dialog->deleteLater();
		dialog = NULL;
	}
}

void ViewDialog::skyCultureChanged(const QString& cultureName)
{
	StelApp::getInstance().getSkyCultureMgr().setSkyCulture(cultureName);
	updateSkyCultureText();
}

void ViewDialog::updateSkyCultureText()
{	
	StelFileMgr& fileMan(StelApp::getInstance().getFileMgr());
	QString descPath;
	try
	{
		descPath = fileMan.findFile("skycultures/" + StelApp::getInstance().getSkyCultureMgr().getSkyCultureDir() + "/description."+StelApp::getInstance().getLocaleMgr().getAppLanguage()+".utf8");
	}
	catch (exception& e)
	{
		try
		{
			descPath = fileMan.findFile("skycultures/" + StelApp::getInstance().getSkyCultureMgr().getSkyCultureDir() + "/description.en.utf8");
		}
		catch (exception& e)
		{
			qWarning() << "WARNING: can't find description for skyculture" << StelApp::getInstance().getSkyCultureMgr().getSkyCultureDir();
		}
	}
	if (descPath.isEmpty())
	{
		ui->skyCultureTextBrowser->setHtml("No description");
	}
	else
	{
		QFile f(descPath);
		f.open(QIODevice::ReadOnly);
		ui->skyCultureTextBrowser->setHtml(QString::fromUtf8(f.readAll()));
	}
}

void ViewDialog::projectionChanged(const QString& projectionName)
{
	const QMap<QString, const Mapping*>& mappings = StelApp::getInstance().getCore()->getProjection()->getAllMappings();
	QMapIterator<QString, const Mapping*> i(mappings);
	while (i.hasNext())
	{
		i.next();
		if (q_(i.value()->getNameI18()) == projectionName)
			break;
	}

	StelApp::getInstance().getCore()->getProjection()->setCurrentMapping(i.value()->getId());
	ui->projectionTextBrowser->setHtml(StelApp::getInstance().getCore()->getProjection()->getCurrentMapping().getHtmlSummary());
}

void ViewDialog::landscapeChanged(const QString& landscapeName)
{
	LandscapeMgr* lmgr = (LandscapeMgr*)GETSTELMODULE("LandscapeMgr");
	lmgr->setLandscapeByName(landscapeName);
	ui->landscapeTextBrowser->setHtml(lmgr->getCurrentLandscapeHtmlDescription());
}

void ViewDialog::shoutingStarsZHRChanged()
{
	MeteorMgr* mmgr = (MeteorMgr*)GETSTELMODULE("MeteorMgr");
	int zhr;
	if (ui->zhr10->isChecked())
		zhr = 10;
	if (ui->zhr80->isChecked())
		zhr = 80;
	if (ui->zhr10000->isChecked())
		zhr = 10000;
	if (ui->zhr144000->isChecked())
		zhr = 144000;
	if (zhr==mmgr->getZHR())
		return;
	mmgr->setZHR(zhr);
	switch (zhr)
	{
		case 10:
			ui->zhrLabel->setText(q_("Normal rate"));
			break;
		case 80:
			ui->zhrLabel->setText(q_("Standard Perseids rate"));
			break;
		case 10000:
			ui->zhrLabel->setText(q_("Exceptional Leonid rate"));
			break;
		case 144000:
			ui->zhrLabel->setText(q_("Highest rate ever (1966 Leonids)"));
			break;
	}
}
