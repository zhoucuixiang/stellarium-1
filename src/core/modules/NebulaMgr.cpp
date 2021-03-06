/*
 * Stellarium
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2011 Alexander Wolf
 * Copyright (C) 2015 Georg Zotti
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
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335, USA.
 */

// class used to manage groups of Nebulas

#include "StelApp.hpp"
#include "NebulaMgr.hpp"
#include "Nebula.hpp"
#include "StelTexture.hpp"
#include "StelUtils.hpp"
#include "StelSkyDrawer.hpp"
#include "StelTranslator.hpp"
#include "StelTextureMgr.hpp"
#include "StelObjectMgr.hpp"
#include "StelLocaleMgr.hpp"
#include "StelSkyCultureMgr.hpp"
#include "StelFileMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelCore.hpp"
#include "StelSkyImageTile.hpp"
#include "StelPainter.hpp"
#include "RefractionExtinction.hpp"
#include "StelActionMgr.hpp"

#include <algorithm>
#include <QDebug>
#include <QFile>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QDir>

void NebulaMgr::setLabelsColor(const Vec3f& c) {Nebula::labelColor = c; emit labelsColorChanged(c);}
const Vec3f NebulaMgr::getLabelsColor(void) const {return Nebula::labelColor;}
void NebulaMgr::setCirclesColor(const Vec3f& c) {Nebula::circleColor = c; emit circlesColorChanged(c); }
const Vec3f NebulaMgr::getCirclesColor(void) const {return Nebula::circleColor;}
void NebulaMgr::setGalaxyColor(const Vec3f& c) {Nebula::galaxyColor = c; emit galaxiesColorChanged(c); }
const Vec3f NebulaMgr::getGalaxyColor(void) const {return Nebula::galaxyColor;}
void NebulaMgr::setRadioGalaxyColor(const Vec3f& c) {Nebula::radioGalaxyColor = c; emit radioGalaxiesColorChanged(c); }
const Vec3f NebulaMgr::getRadioGalaxyColor(void) const {return Nebula::radioGalaxyColor;}
void NebulaMgr::setActiveGalaxyColor(const Vec3f& c) {Nebula::activeGalaxyColor = c; emit activeGalaxiesColorChanged(c); }
const Vec3f NebulaMgr::getActiveGalaxyColor(void) const {return Nebula::activeGalaxyColor;}
void NebulaMgr::setInteractingGalaxyColor(const Vec3f& c) {Nebula::interactingGalaxyColor = c; emit interactingGalaxiesColorChanged(c); }
const Vec3f NebulaMgr::getInteractingGalaxyColor(void) const {return Nebula::interactingGalaxyColor;}
void NebulaMgr::setQuasarColor(const Vec3f& c) {Nebula::quasarColor = c; emit quasarsColorChanged(c); }
const Vec3f NebulaMgr::getQuasarColor(void) const {return Nebula::quasarColor;}
void NebulaMgr::setBrightNebulaColor(const Vec3f& c) {Nebula::nebulaColor = c; emit nebulaeColorChanged(c); }
const Vec3f NebulaMgr::getBrightNebulaColor(void) const {return Nebula::nebulaColor;}
void NebulaMgr::setNebulaColor(const Vec3f& c) {Nebula::nebulaColor = c; emit nebulaeColorChanged(c); }
const Vec3f NebulaMgr::getNebulaColor(void) const {return Nebula::nebulaColor;}
void NebulaMgr::setPlanetaryNebulaColor(const Vec3f& c) {Nebula::planetaryNebulaColor = c; emit planetaryNebulaeColorChanged(c);}
const Vec3f NebulaMgr::getPlanetaryNebulaColor(void) const {return Nebula::planetaryNebulaColor;}
void NebulaMgr::setReflectionNebulaColor(const Vec3f& c) {Nebula::reflectionNebulaColor = c; emit reflectionNebulaeColorChanged(c);}
const Vec3f NebulaMgr::getReflectionNebulaColor(void) const {return Nebula::reflectionNebulaColor;}
void NebulaMgr::setBipolarNebulaColor(const Vec3f& c) {Nebula::bipolarNebulaColor = c; emit bipolarNebulaeColorChanged(c);}
const Vec3f NebulaMgr::getBipolarNebulaColor(void) const {return Nebula::bipolarNebulaColor;}
void NebulaMgr::setEmissionNebulaColor(const Vec3f& c) {Nebula::emissionNebulaColor = c; emit emissionNebulaeColorChanged(c);}
const Vec3f NebulaMgr::getEmissionNebulaColor(void) const {return Nebula::emissionNebulaColor;}
void NebulaMgr::setDarkNebulaColor(const Vec3f& c) {Nebula::darkNebulaColor= c; emit darkNebulaeColorChanged(c);}
const Vec3f NebulaMgr::getDarkNebulaColor(void) const {return Nebula::darkNebulaColor;}
void NebulaMgr::setHydrogenRegionColor(const Vec3f& c) {Nebula::hydrogenRegionColor= c; emit hydrogenRegionsColorChanged(c);}
const Vec3f NebulaMgr::getHydrogenRegionColor(void) const {return Nebula::hydrogenRegionColor;}
void NebulaMgr::setSupernovaRemnantColor(const Vec3f& c) {Nebula::supernovaRemnantColor = c; emit supernovaRemnantsColorChanged(c);}
const Vec3f NebulaMgr::getSupernovaRemnantColor(void) const {return Nebula::supernovaRemnantColor;}
void NebulaMgr::setInterstellarMatterColor(const Vec3f& c) {Nebula::interstellarMatterColor = c; emit interstellarMatterColorChanged(c);}
const Vec3f NebulaMgr::getInterstellarMatterColor(void) const {return Nebula::interstellarMatterColor;}
void NebulaMgr::setClusterWithNebulosityColor(const Vec3f& c) {Nebula::clusterWithNebulosityColor= c; emit clusterWithNebulosityColorChanged(c);}
const Vec3f NebulaMgr::getClusterWithNebulosityColor(void) const {return Nebula::clusterWithNebulosityColor;}
void NebulaMgr::setClusterColor(const Vec3f& c) {Nebula::clusterColor= c; emit clustersColorChanged(c);}
const Vec3f NebulaMgr::getClusterColor(void) const {return Nebula::clusterColor;}
void NebulaMgr::setOpenClusterColor(const Vec3f& c) {Nebula::openClusterColor = c; emit openClustersColorChanged(c);}
const Vec3f NebulaMgr::getOpenClusterColor(void) const {return Nebula::openClusterColor;}
void NebulaMgr::setGlobularClusterColor(const Vec3f& c) {Nebula::globularClusterColor = c; emit globularClustersColorChanged(c);}
const Vec3f NebulaMgr::getGlobularClusterColor(void) const {return Nebula::globularClusterColor;}
void NebulaMgr::setStellarAssociationColor(const Vec3f& c) {Nebula::stellarAssociationColor = c; emit stellarAssociationsColorChanged(c);}
const Vec3f NebulaMgr::getStellarAssociationColor(void) const {return Nebula::stellarAssociationColor;}
void NebulaMgr::setStarCloudColor(const Vec3f& c) {Nebula::starCloudColor = c; emit starCloudsColorChanged(c);}
const Vec3f NebulaMgr::getStarCloudColor(void) const {return Nebula::starCloudColor;}
void NebulaMgr::setEmissionObjectColor(const Vec3f& c) {Nebula::emissionObjectColor = c; emit emissionObjectsColorChanged(c);}
const Vec3f NebulaMgr::getEmissionObjectColor(void) const {return Nebula::emissionObjectColor;}
void NebulaMgr::setBlLacObjectColor(const Vec3f& c) {Nebula::blLacObjectColor = c; emit blLacObjectsColorChanged(c);}
const Vec3f NebulaMgr::getBlLacObjectColor(void) const {return Nebula::blLacObjectColor;}
void NebulaMgr::setBlazarColor(const Vec3f& c) {Nebula::blazarColor = c; emit blazarsColorChanged(c);}
const Vec3f NebulaMgr::getBlazarColor(void) const {return Nebula::blazarColor;}
void NebulaMgr::setMolecularCloudColor(const Vec3f& c) {Nebula::molecularCloudColor = c; emit molecularCloudsColorChanged(c);}
const Vec3f NebulaMgr::getMolecularCloudColor(void) const {return Nebula::molecularCloudColor;}
void NebulaMgr::setYoungStellarObjectColor(const Vec3f& c) {Nebula::youngStellarObjectColor = c; emit youngStellarObjectsColorChanged(c);}
const Vec3f NebulaMgr::getYoungStellarObjectColor(void) const {return Nebula::youngStellarObjectColor;}
void NebulaMgr::setPossibleQuasarColor(const Vec3f& c) {Nebula::possibleQuasarColor = c; emit possibleQuasarsColorChanged(c);}
const Vec3f NebulaMgr::getPossibleQuasarColor(void) const {return Nebula::possibleQuasarColor;}
void NebulaMgr::setPossiblePlanetaryNebulaColor(const Vec3f& c) {Nebula::possiblePlanetaryNebulaColor = c; emit possiblePlanetaryNebulaeColorChanged(c);}
const Vec3f NebulaMgr::getPossiblePlanetaryNebulaColor(void) const {return Nebula::possiblePlanetaryNebulaColor;}
void NebulaMgr::setProtoplanetaryNebulaColor(const Vec3f& c) {Nebula::protoplanetaryNebulaColor = c; emit protoplanetaryNebulaeColorChanged(c);}
const Vec3f NebulaMgr::getProtoplanetaryNebulaColor(void) const {return Nebula::protoplanetaryNebulaColor;}
void NebulaMgr::setStarColor(const Vec3f& c) {Nebula::starColor = c; emit starsColorChanged(c);}
const Vec3f NebulaMgr::getStarColor(void) const {return Nebula::starColor;}
void NebulaMgr::setHintsProportional(const bool proportional) {if(Nebula::drawHintProportional!=proportional){ Nebula::drawHintProportional=proportional; emit hintsProportionalChanged(proportional);}}
bool NebulaMgr::getHintsProportional(void) const {return Nebula::drawHintProportional;}
void NebulaMgr::setDesignationUsage(const bool flag) {if(Nebula::designationUsage!=flag){ Nebula::designationUsage=flag; emit designationUsageChanged(flag);}}
bool NebulaMgr::getDesignationUsage(void) const {return Nebula::designationUsage; }

NebulaMgr::NebulaMgr(void)
	: nebGrid(200)
	, hintsAmount(0)
	, labelsAmount(0)
	, flagConverter(false)
	, flagDecimalCoordinates(true)
{
	setObjectName("NebulaMgr");
}

NebulaMgr::~NebulaMgr()
{
	Nebula::texCircle = StelTextureSP();
	Nebula::texGalaxy = StelTextureSP();
	Nebula::texOpenCluster = StelTextureSP();
	Nebula::texGlobularCluster = StelTextureSP();
	Nebula::texPlanetaryNebula = StelTextureSP();
	Nebula::texDiffuseNebula = StelTextureSP();
	Nebula::texDarkNebula = StelTextureSP();
	Nebula::texOpenClusterWithNebulosity = StelTextureSP();
}

/*************************************************************************
 Reimplementation of the getCallOrder method
*************************************************************************/
double NebulaMgr::getCallOrder(StelModuleActionName actionName) const
{
	if (actionName==StelModule::ActionDraw)
		return StelApp::getInstance().getModuleMgr().getModule("MilkyWay")->getCallOrder(actionName)+10;
	return 0;
}

// read from stream
void NebulaMgr::init()
{
	QSettings* conf = StelApp::getInstance().getSettings();
	Q_ASSERT(conf);

	nebulaFont.setPixelSize(StelApp::getInstance().getBaseFontSize());
	Nebula::texCircle			= StelApp::getInstance().getTextureManager().createTexture(StelFileMgr::getInstallationDir()+"/textures/neb.png");	// Load circle texture
	Nebula::texGalaxy			= StelApp::getInstance().getTextureManager().createTexture(StelFileMgr::getInstallationDir()+"/textures/neb_gal.png");	// Load ellipse texture
	Nebula::texOpenCluster			= StelApp::getInstance().getTextureManager().createTexture(StelFileMgr::getInstallationDir()+"/textures/neb_ocl.png");	// Load open cluster marker texture
	Nebula::texGlobularCluster		= StelApp::getInstance().getTextureManager().createTexture(StelFileMgr::getInstallationDir()+"/textures/neb_gcl.png");	// Load globular cluster marker texture
	Nebula::texPlanetaryNebula		= StelApp::getInstance().getTextureManager().createTexture(StelFileMgr::getInstallationDir()+"/textures/neb_pnb.png");	// Load planetary nebula marker texture
	Nebula::texDiffuseNebula		= StelApp::getInstance().getTextureManager().createTexture(StelFileMgr::getInstallationDir()+"/textures/neb_dif.png");	// Load diffuse nebula marker texture
	Nebula::texDarkNebula			= StelApp::getInstance().getTextureManager().createTexture(StelFileMgr::getInstallationDir()+"/textures/neb_drk.png");	// Load dark nebula marker texture
	Nebula::texOpenClusterWithNebulosity	= StelApp::getInstance().getTextureManager().createTexture(StelFileMgr::getInstallationDir()+"/textures/neb_ocln.png");	// Load Ocl/Nebula marker texture
	texPointer = StelApp::getInstance().getTextureManager().createTexture(StelFileMgr::getInstallationDir()+"/textures/pointeur5.png");   // Load pointer texture

	setFlagShow(conf->value("astro/flag_nebula",true).toBool());
	setFlagHints(conf->value("astro/flag_nebula_name",false).toBool());
	setHintsAmount(conf->value("astro/nebula_hints_amount", 3).toFloat());
	setLabelsAmount(conf->value("astro/nebula_labels_amount", 3).toFloat());
	setHintsProportional(conf->value("astro/flag_nebula_hints_proportional", false).toBool());
	setDesignationUsage(conf->value("gui/flag_dso_designation_usage", false).toBool());
	setFlagSurfaceBrightnessUsage(conf->value("astro/flag_surface_brightness_usage", false).toBool());

	// Load colors from config file
	// Upgrade config keys
	if (conf->contains("color/nebula_label_color"))
	{
		conf->setValue("color/dso_label_color", conf->value("color/nebula_label_color", "0.4,0.3,0.5").toString());
		conf->remove("color/nebula_label_color");
	}
	if (conf->contains("color/nebula_circle_color"))
	{
		conf->setValue("color/dso_circle_color", conf->value("color/nebula_circle_color", "0.8,0.8,0.1").toString());
		conf->remove("color/nebula_circle_color");
	}
	if (conf->contains("color/nebula_galaxy_color"))
	{
		conf->setValue("color/dso_galaxy_color", conf->value("color/nebula_galaxy_color", "1.0,0.2,0.2").toString());
		conf->remove("color/nebula_galaxy_color");
	}
	if (conf->contains("color/nebula_radioglx_color"))
	{
		conf->setValue("color/dso_radio_galaxy_color", conf->value("color/nebula_radioglx_color", "0.3,0.3,0.3").toString());
		conf->remove("color/nebula_radioglx_color");
	}
	if (conf->contains("color/nebula_activeglx_color"))
	{
		conf->setValue("color/dso_active_galaxy_color", conf->value("color/nebula_activeglx_color", "1.0,0.5,0.2").toString());
		conf->remove("color/nebula_activeglx_color");
	}
	if (conf->contains("color/nebula_intglx_color"))
	{
		conf->setValue("color/dso_interacting_galaxy_color", conf->value("color/nebula_intglx_color", "0.2,0.5,1.0").toString());
		conf->remove("color/nebula_intglx_color");
	}
	if (conf->contains("color/nebula_brightneb_color"))
	{
		conf->setValue("color/dso_nebula_color", conf->value("color/nebula_brightneb_color", "0.1,1.0,0.1").toString());
		conf->remove("color/nebula_brightneb_color");
	}
	if (conf->contains("color/nebula_darkneb_color"))
	{
		conf->setValue("color/dso_dark_nebula_color", conf->value("color/nebula_darkneb_color", "0.3,0.3,0.3").toString());
		conf->remove("color/nebula_darkneb_color");
	}
	if (conf->contains("color/nebula_hregion_color"))
	{
		conf->setValue("color/dso_hydrogen_region_color", conf->value("color/nebula_hregion_color", "0.1,1.0,0.1").toString());
		conf->remove("color/nebula_hregion_color");
	}
	if (conf->contains("color/nebula_snr_color"))
	{
		conf->setValue("color/dso_supernova_remnant_color", conf->value("color/nebula_snr_color", "0.1,1.0,0.1").toString());
		conf->remove("color/nebula_snr_color");
	}
	if (conf->contains("color/nebula_cluster_color"))
	{
		conf->setValue("color/dso_cluster_color", conf->value("color/nebula_cluster_color", "0.8,0.8,0.1").toString());
		conf->remove("color/nebula_cluster_color");
	}

	// Set colors for markers
	setLabelsColor(StelUtils::strToVec3f(conf->value("color/dso_label_color", "0.2,0.6,0.7").toString()));
	setCirclesColor(StelUtils::strToVec3f(conf->value("color/dso_circle_color", "1.0,0.7,0.2").toString()));

	QString defaultGalaxyColor = conf->value("color/dso_galaxy_color", "1.0,0.2,0.2").toString();
	setGalaxyColor(StelUtils::strToVec3f(defaultGalaxyColor));
	setRadioGalaxyColor(StelUtils::strToVec3f(conf->value("color/dso_radio_galaxy_color", "0.3,0.3,0.3").toString()));
	setActiveGalaxyColor(StelUtils::strToVec3f(conf->value("color/dso_active_galaxy_color", "1.0,0.5,0.2").toString()));
	setInteractingGalaxyColor(StelUtils::strToVec3f(conf->value("color/dso_interacting_galaxy_color", "0.2,0.5,1.0").toString()));
	setQuasarColor(StelUtils::strToVec3f(conf->value("color/dso_quasar_color", defaultGalaxyColor).toString()));
	setPossibleQuasarColor(StelUtils::strToVec3f(conf->value("color/dso_possible_quasar_color", defaultGalaxyColor).toString()));
	setBlLacObjectColor(StelUtils::strToVec3f(conf->value("color/dso_bl_lac_color", defaultGalaxyColor).toString()));
	setBlazarColor(StelUtils::strToVec3f(conf->value("color/dso_blazar_color", defaultGalaxyColor).toString()));

	QString defaultNebulaColor = conf->value("color/dso_nebula_color", "0.1,1.0,0.1").toString();
	setNebulaColor(StelUtils::strToVec3f(defaultNebulaColor));
	setPlanetaryNebulaColor(StelUtils::strToVec3f(conf->value("color/dso_planetary_nebula_color", defaultNebulaColor).toString()));
	setReflectionNebulaColor(StelUtils::strToVec3f(conf->value("color/dso_reflection_nebula_color", defaultNebulaColor).toString()));
	setBipolarNebulaColor(StelUtils::strToVec3f(conf->value("color/dso_bipolar_nebula_color", defaultNebulaColor).toString()));
	setEmissionNebulaColor(StelUtils::strToVec3f(conf->value("color/dso_emission_nebula_color", defaultNebulaColor).toString()));
	setDarkNebulaColor(StelUtils::strToVec3f(conf->value("color/dso_dark_nebula_color", "0.3,0.3,0.3").toString()));
	setHydrogenRegionColor(StelUtils::strToVec3f(conf->value("color/dso_hydrogen_region_color", defaultNebulaColor).toString()));
	setSupernovaRemnantColor(StelUtils::strToVec3f(conf->value("color/dso_supernova_remnant_color", defaultNebulaColor).toString()));
	setInterstellarMatterColor(StelUtils::strToVec3f(conf->value("color/dso_interstellar_matter_color", defaultNebulaColor).toString()));
	setClusterWithNebulosityColor(StelUtils::strToVec3f(conf->value("color/dso_cluster_with_nebulosity_color", defaultNebulaColor).toString()));
	setMolecularCloudColor(StelUtils::strToVec3f(conf->value("color/dso_molecular_cloud_color", defaultNebulaColor).toString()));
	setPossiblePlanetaryNebulaColor(StelUtils::strToVec3f(conf->value("color/dso_possible_planetary_nebula_color", defaultNebulaColor).toString()));
	setProtoplanetaryNebulaColor(StelUtils::strToVec3f(conf->value("color/dso_protoplanetary_nebula_color", defaultNebulaColor).toString()));

	QString defaultClusterColor = conf->value("color/dso_cluster_color", "1.0,1.0,0.1").toString();
	setClusterColor(StelUtils::strToVec3f(defaultClusterColor));
	setOpenClusterColor(StelUtils::strToVec3f(conf->value("color/dso_open_cluster_color", defaultClusterColor).toString()));
	setGlobularClusterColor(StelUtils::strToVec3f(conf->value("color/dso_globular_cluster_color", defaultClusterColor).toString()));
	setStellarAssociationColor(StelUtils::strToVec3f(conf->value("color/dso_stellar_association_color", defaultClusterColor).toString()));
	setStarCloudColor(StelUtils::strToVec3f(conf->value("color/dso_star_cloud_color", defaultClusterColor).toString()));

	QString defaultStellarColor = conf->value("color/dso_star_color", "1.0,0.7,0.2").toString();
	setStarColor(StelUtils::strToVec3f(defaultStellarColor));
	setEmissionObjectColor(StelUtils::strToVec3f(conf->value("color/dso_emission_object_color", defaultStellarColor).toString()));
	setYoungStellarObjectColor(StelUtils::strToVec3f(conf->value("color/dso_young_stellar_object_color", defaultStellarColor).toString()));

	// for DSO convertor (for developers!)
	flagConverter = conf->value("devel/convert_dso_catalog", false).toBool();
	flagDecimalCoordinates = conf->value("devel/convert_dso_decimal_coord", true).toBool();

	setFlagUseTypeFilters(conf->value("astro/flag_use_type_filter", false).toBool());

	Nebula::CatalogGroup catalogFilters = Nebula::CatalogGroup(0);

	conf->beginGroup("dso_catalog_filters");
	if (conf->value("flag_show_ngc", true).toBool())
		catalogFilters	|= Nebula::CatNGC;
	if (conf->value("flag_show_ic", true).toBool())
		catalogFilters	|= Nebula::CatIC;
	if (conf->value("flag_show_m", true).toBool())
		catalogFilters	|= Nebula::CatM;
	if (conf->value("flag_show_c", false).toBool())
		catalogFilters	|= Nebula::CatC;
	if (conf->value("flag_show_b", false).toBool())
		catalogFilters	|= Nebula::CatB;
	if (conf->value("flag_show_sh2", false).toBool())
		catalogFilters	|= Nebula::CatSh2;
	if (conf->value("flag_show_vdb", false).toBool())
		catalogFilters	|= Nebula::CatVdB;
	if (conf->value("flag_show_lbn", false).toBool())
		catalogFilters	|= Nebula::CatLBN;
	if (conf->value("flag_show_ldn", false).toBool())
		catalogFilters	|= Nebula::CatLDN;
	if (conf->value("flag_show_rcw", false).toBool())
		catalogFilters	|= Nebula::CatRCW;
	if (conf->value("flag_show_cr", false).toBool())
		catalogFilters	|= Nebula::CatCr;
	if (conf->value("flag_show_mel", false).toBool())
		catalogFilters	|= Nebula::CatMel;
	if (conf->value("flag_show_pgc", false).toBool())
		catalogFilters	|= Nebula::CatPGC;
	if (conf->value("flag_show_ced", false).toBool())
		catalogFilters	|= Nebula::CatCed;
	if (conf->value("flag_show_ugc", false).toBool())
		catalogFilters	|= Nebula::CatUGC;
	conf->endGroup();

	// NB: nebula set loaded inside setter of catalog filter
	setCatalogFilters(catalogFilters);

	Nebula::TypeGroup typeFilters = Nebula::TypeGroup(0);

	conf->beginGroup("dso_type_filters");
	if (conf->value("flag_show_galaxies", true).toBool())
		typeFilters	|= Nebula::TypeGalaxies;
	if (conf->value("flag_show_active_galaxies", true).toBool())
		typeFilters	|= Nebula::TypeActiveGalaxies;
	if (conf->value("flag_show_interacting_galaxies", true).toBool())
		typeFilters	|= Nebula::TypeInteractingGalaxies;
	if (conf->value("flag_show_clusters", true).toBool())
		typeFilters	|= Nebula::TypeStarClusters;
	if (conf->value("flag_show_bright_nebulae", true).toBool())
		typeFilters	|= Nebula::TypeBrightNebulae;
	if (conf->value("flag_show_dark_nebulae", true).toBool())
		typeFilters	|= Nebula::TypeDarkNebulae;
	if (conf->value("flag_show_planetary_nebulae", true).toBool())
		typeFilters	|= Nebula::TypePlanetaryNebulae;
	if (conf->value("flag_show_hydrogen_regions", true).toBool())
		typeFilters	|= Nebula::TypeHydrogenRegions;
	if (conf->value("flag_show_supernova_remnants", true).toBool())
		typeFilters	|= Nebula::TypeSupernovaRemnants;
	if (conf->value("flag_show_other", true).toBool())
		typeFilters	|= Nebula::TypeOther;
	conf->endGroup();

	setTypeFilters(typeFilters);

	// TODO: mechanism to specify which sets get loaded at start time.
	// candidate methods:
	// 1. config file option (list of sets to load at startup)
	// 2. load all
	// 3. flag in nebula_textures.fab (yuk)
	// 4. info.ini file in each set containing a "load at startup" item
	// For now (0.9.0), just load the default set
	// NB: nebula set loaded inside setter of catalog filter --AW
	// loadNebulaSet("default");

	updateI18n();
	
	StelApp *app = &StelApp::getInstance();
	connect(app, SIGNAL(languageChanged()), this, SLOT(updateI18n()));	
	connect(&app->getSkyCultureMgr(), SIGNAL(currentSkyCultureChanged(QString)), this, SLOT(updateSkyCulture(const QString&)));
	GETSTELMODULE(StelObjectMgr)->registerStelObjectMgr(this);
	connect(this, SIGNAL(catalogFiltersChanged(Nebula::CatalogGroup)), this, SLOT(updateDSONames()));

	addAction("actionShow_Nebulas", N_("Display Options"), N_("Deep-sky objects"), "flagHintDisplayed", "D", "N");
	addAction("actionSet_Nebula_TypeFilterUsage", N_("Display Options"), N_("Toggle DSO type filter"), "flagTypeFiltersUsage");
}

struct DrawNebulaFuncObject
{
	DrawNebulaFuncObject(float amaxMagHints, float amaxMagLabels, StelPainter* p, StelCore* aCore, bool acheckMaxMagHints)
		: maxMagHints(amaxMagHints)
		, maxMagLabels(amaxMagLabels)
		, sPainter(p)
		, core(aCore)
		, checkMaxMagHints(acheckMaxMagHints)
	{
		angularSizeLimit = 5.f/sPainter->getProjector()->getPixelPerRadAtCenter()*180.f/M_PI;
	}
	void operator()(StelRegionObject* obj)
	{
		Nebula* n = static_cast<Nebula*>(obj);
		StelSkyDrawer *drawer = core->getSkyDrawer();
		// filter out DSOs which are too dim to be seen (e.g. for bino observers)
		if ((drawer->getFlagNebulaMagnitudeLimit()) && (n->vMag > drawer->getCustomNebulaMagnitudeLimit())) return;

		if (n->majorAxisSize>angularSizeLimit || n->majorAxisSize==0.f || (checkMaxMagHints && n->vMag <= maxMagHints))
		{
			float refmag_add=0; // value to adjust hints visibility threshold.
			sPainter->getProjector()->project(n->XYZ,n->XY);
			n->drawLabel(*sPainter, maxMagLabels-refmag_add);
			n->drawHints(*sPainter, maxMagHints -refmag_add);
		}
	}
	float maxMagHints;
	float maxMagLabels;
	StelPainter* sPainter;
	StelCore* core;
	float angularSizeLimit;
	bool checkMaxMagHints;
};

void NebulaMgr::setCatalogFilters(Nebula::CatalogGroup cflags)
{
	if(static_cast<int>(cflags) != static_cast<int>(Nebula::catalogFilters))
	{
		Nebula::catalogFilters = cflags;

		dsoArray.clear();
		dsoIndex.clear();
		nebGrid.clear();
		bool status = getFlagShow();

		StelApp::getInstance().getStelObjectMgr().unSelect();

		qWarning() << "Reloading DSO data...";
		setFlagShow(false);
		loadNebulaSet("default");		
		setFlagShow(status);

		updateI18n(); // OK, update localized names of DSO

		emit catalogFiltersChanged(cflags);
	}
}

void NebulaMgr::setTypeFilters(Nebula::TypeGroup tflags)
{
	if(static_cast<int>(tflags) != static_cast<int>(Nebula::typeFilters))
	{
		Nebula::typeFilters = tflags;
		emit typeFiltersChanged(tflags);
	}
}

float NebulaMgr::computeMaxMagHint(const StelSkyDrawer* skyDrawer) const
{
	return skyDrawer->getLimitMagnitude()*1.2f-2.f+(hintsAmount *1.2f)-2.f;
}

// Draw all the Nebulae
void NebulaMgr::draw(StelCore* core)
{
	const StelProjectorP prj = core->getProjection(StelCore::FrameJ2000);
	StelPainter sPainter(prj);

	StelSkyDrawer* skyDrawer = core->getSkyDrawer();

	Nebula::hintsBrightness = hintsFader.getInterstate()*flagShow.getInterstate();

	sPainter.setBlending(true, GL_ONE, GL_ONE);

	// Use a 1 degree margin
	const double margin = 1.*M_PI/180.*prj->getPixelPerRadAtCenter();
	const SphericalRegionP& p = prj->getViewportConvexPolygon(margin, margin);

	// Print all the nebulae of all the selected zones
	float maxMagHints  = computeMaxMagHint(skyDrawer);
	float maxMagLabels = skyDrawer->getLimitMagnitude()-2.f+(labelsAmount*1.2f)-2.f;
	sPainter.setFont(nebulaFont);
	DrawNebulaFuncObject func(maxMagHints, maxMagLabels, &sPainter, core, hintsFader.getInterstate()>0.0001);
	nebGrid.processIntersectingPointInRegions(p.data(), func);

	if (GETSTELMODULE(StelObjectMgr)->getFlagSelectedObjectPointer())
		drawPointer(core, sPainter);
}

void NebulaMgr::drawPointer(const StelCore* core, StelPainter& sPainter)
{
	const StelProjectorP prj = core->getProjection(StelCore::FrameJ2000);

	const QList<StelObjectP> newSelected = GETSTELMODULE(StelObjectMgr)->getSelectedObject("Nebula");	
	if (!newSelected.empty())
	{
		const StelObjectP obj = newSelected[0];
		Vec3d pos=obj->getJ2000EquatorialPos(core);

		// Compute 2D pos and return if outside screen
		if (!prj->projectInPlace(pos)) return;
		sPainter.setColor(0.4f,0.5f,0.8f);

		texPointer->bind();

		sPainter.setBlending(true);

		// Size on screen
		float size = obj->getAngularSize(core)*M_PI/180.*prj->getPixelPerRadAtCenter();
		if (size>120.f) // avoid oversized marker
			size = 120.f;

		if (Nebula::drawHintProportional)
			size*=1.2f;
		size+=20.f + 10.f*std::sin(3.f * StelApp::getInstance().getAnimationTime());
		sPainter.drawSprite2dMode(pos[0]-size/2, pos[1]-size/2, 10, 90);
		sPainter.drawSprite2dMode(pos[0]-size/2, pos[1]+size/2, 10, 0);
		sPainter.drawSprite2dMode(pos[0]+size/2, pos[1]+size/2, 10, -90);
		sPainter.drawSprite2dMode(pos[0]+size/2, pos[1]-size/2, 10, -180);
	}
}

// Search by name
NebulaP NebulaMgr::search(const QString& name)
{
	QString uname = name.toUpper();

	foreach (const NebulaP& n, dsoArray)
	{
		QString testName = n->getEnglishName().toUpper();
		if (testName==uname) return n;
	}

	// If no match found, try search by catalog reference
	static QRegExp catNumRx("^(M|NGC|IC|C|B|VDB|RCW|LDN|LBN|CR|MEL|PGC|UGC)\\s*(\\d+)$");
	if (catNumRx.exactMatch(uname))
	{
		QString cat = catNumRx.capturedTexts().at(1);
		int num = catNumRx.capturedTexts().at(2).toInt();

		if (cat == "M") return searchM(num);
		if (cat == "NGC") return searchNGC(num);
		if (cat == "IC") return searchIC(num);
		if (cat == "C") return searchC(num);
		if (cat == "B") return searchB(num);
		if (cat == "VDB") return searchVdB(num);
		if (cat == "RCW") return searchRCW(num);
		if (cat == "LDN") return searchLDN(num);
		if (cat == "LBN") return searchLBN(num);
		if (cat == "CR") return searchCr(num);
		if (cat == "MEL") return searchMel(num);
		if (cat == "PGC") return searchPGC(num);
		if (cat == "UGC") return searchUGC(num);
	}
	static QRegExp dCatNumRx("^(SH)\\s*\\d-\\s*(\\d+)$");
	if (dCatNumRx.exactMatch(uname))
	{
		QString dcat = dCatNumRx.capturedTexts().at(1);
		int dnum = dCatNumRx.capturedTexts().at(2).toInt();

		if (dcat == "SH") return searchSh2(dnum);
	}
	static QRegExp sCatNumRx("^(CED)\\s*(.+)$");
	if (sCatNumRx.exactMatch(uname))
	{
		QString cat = catNumRx.capturedTexts().at(1);
		QString num = catNumRx.capturedTexts().at(2).trimmed();

		if (cat == "CED") return searchCed(num);
	}
	return NebulaP();
}

void NebulaMgr::loadNebulaSet(const QString& setName)
{
	QString srcCatalogPath		= StelFileMgr::findFile("nebulae/" + setName + "/catalog.txt");
	QString dsoCatalogPath		= StelFileMgr::findFile("nebulae/" + setName + "/catalog.dat");


	if (flagConverter)
	{
		if (!srcCatalogPath.isEmpty())
			convertDSOCatalog(srcCatalogPath, dsoCatalogPath, flagDecimalCoordinates);
		else
			qWarning() << "ERROR convert catalogue, because source data set is not exists for " << setName;			

	}

	if (dsoCatalogPath.isEmpty())
	{
		qWarning() << "ERROR while loading deep-sky catalog data set " << setName;
		return;
	}

	loadDSOCatalog(dsoCatalogPath);		
}

// Look for a nebulae by XYZ coords
NebulaP NebulaMgr::search(const Vec3d& apos)
{
	Vec3d pos = apos;
	pos.normalize();
	NebulaP plusProche;
	float anglePlusProche=0.0f;
	foreach (const NebulaP& n, dsoArray)
	{
		if (n->XYZ*pos>anglePlusProche)
		{
			anglePlusProche=n->XYZ*pos;
			plusProche=n;
		}
	}
	if (anglePlusProche>0.999f)
	{
		return plusProche;
	}
	else return NebulaP();
}


QList<StelObjectP> NebulaMgr::searchAround(const Vec3d& av, double limitFov, const StelCore*) const
{
	QList<StelObjectP> result;
	if (!getFlagShow())
		return result;

	Vec3d v(av);
	v.normalize();
	double cosLimFov = cos(limitFov * M_PI/180.);
	Vec3d equPos;
	foreach (const NebulaP& n, dsoArray)
	{
		equPos = n->XYZ;
		equPos.normalize();
		if (equPos*v>=cosLimFov)
		{
			result.push_back(qSharedPointerCast<StelObject>(n));
		}
	}
	return result;
}

NebulaP NebulaMgr::searchDSO(unsigned int DSO)
{
	if (dsoIndex.contains(DSO))
		return dsoIndex[DSO];
	return NebulaP();
}


NebulaP NebulaMgr::searchM(unsigned int M)
{
	foreach (const NebulaP& n, dsoArray)
		if (n->M_nb == M)
			return n;
	return NebulaP();
}

NebulaP NebulaMgr::searchNGC(unsigned int NGC)
{
	foreach (const NebulaP& n, dsoArray)
		if (n->NGC_nb == NGC)
			return n;
	return NebulaP();
}

NebulaP NebulaMgr::searchIC(unsigned int IC)
{
	foreach (const NebulaP& n, dsoArray)
		if (n->IC_nb == IC)
			return n;
	return NebulaP();
}

NebulaP NebulaMgr::searchC(unsigned int C)
{
	foreach (const NebulaP& n, dsoArray)
		if (n->C_nb == C)
			return n;
	return NebulaP();
}

NebulaP NebulaMgr::searchB(unsigned int B)
{
	foreach (const NebulaP& n, dsoArray)
		if (n->B_nb == B)
			return n;
	return NebulaP();
}

NebulaP NebulaMgr::searchSh2(unsigned int Sh2)
{
	foreach (const NebulaP& n, dsoArray)
		if (n->Sh2_nb == Sh2)
			return n;
	return NebulaP();
}

NebulaP NebulaMgr::searchVdB(unsigned int VdB)
{
	foreach (const NebulaP& n, dsoArray)
		if (n->VdB_nb == VdB)
			return n;
	return NebulaP();
}

NebulaP NebulaMgr::searchRCW(unsigned int RCW)
{
	foreach (const NebulaP& n, dsoArray)
		if (n->RCW_nb == RCW)
			return n;
	return NebulaP();
}

NebulaP NebulaMgr::searchLDN(unsigned int LDN)
{
	foreach (const NebulaP& n, dsoArray)
		if (n->LDN_nb == LDN)
			return n;
	return NebulaP();
}

NebulaP NebulaMgr::searchLBN(unsigned int LBN)
{
	foreach (const NebulaP& n, dsoArray)
		if (n->LBN_nb == LBN)
			return n;
	return NebulaP();
}

NebulaP NebulaMgr::searchCr(unsigned int Cr)
{
	foreach (const NebulaP& n, dsoArray)
		if (n->Cr_nb == Cr)
			return n;
	return NebulaP();
}

NebulaP NebulaMgr::searchMel(unsigned int Mel)
{
	foreach (const NebulaP& n, dsoArray)
		if (n->Mel_nb == Mel)
			return n;
	return NebulaP();
}

NebulaP NebulaMgr::searchPGC(unsigned int PGC)
{
	foreach (const NebulaP& n, dsoArray)
		if (n->PGC_nb == PGC)
			return n;
	return NebulaP();
}

NebulaP NebulaMgr::searchUGC(unsigned int UGC)
{
	foreach (const NebulaP& n, dsoArray)
		if (n->UGC_nb == UGC)
			return n;
	return NebulaP();
}

NebulaP NebulaMgr::searchCed(QString Ced)
{
	foreach (const NebulaP& n, dsoArray)
		if (n->Ced_nb.trimmed().toUpper() == Ced.trimmed().toUpper())
			return n;
	return NebulaP();
}

QString NebulaMgr::getLatestSelectedDSODesignation()
{
	QString result = "";

	const QList<StelObjectP> selected = GETSTELMODULE(StelObjectMgr)->getSelectedObject("Nebula");
	if (!selected.empty())
	{
		foreach (const NebulaP& n, dsoArray)
			if (n==selected[0])
				result = n->getDSODesignation(); // Get designation for latest selected DSO
	}

	return result;
}

void NebulaMgr::convertDSOCatalog(const QString &in, const QString &out, bool decimal=false)
{
	QFile dsoIn(in);
	if (!dsoIn.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QFile dsoOut(out);
	if (!dsoOut.open(QIODevice::WriteOnly))
		return;

	int totalRecords=0;
	QString record;
	while (!dsoIn.atEnd())
	{
		dsoIn.readLine();
		++totalRecords;
	}

	// rewind the file to the start
	dsoIn.seek(0);

	QDataStream dsoOutStream(&dsoOut);
	dsoOutStream.setVersion(QDataStream::Qt_5_2);

	int	id, orientationAngle, NGC, IC, M, C, B, Sh2, VdB, RCW, LDN, LBN, Cr, Mel, PGC, UGC;
	float	raRad, decRad, bMag, vMag, majorAxisSize, minorAxisSize, dist, distErr, z, zErr, plx, plxErr;
	QString oType, mType, Ced, ra, dec;

	unsigned int nType;

	int currentLineNumber = 0;	// what input line we are on
	int currentRecordNumber = 0;	// what record number we are on
	int readOk = 0;			// how many records weree rad without problems
	while (!dsoIn.atEnd())
	{
		record = QString::fromUtf8(dsoIn.readLine());
		++currentLineNumber;

		// skip comments
		if (record.startsWith("//") || record.startsWith("#"))
			continue;
		++currentRecordNumber;

		if (!record.isEmpty())
		{
			QStringList list=record.split("\t", QString::KeepEmptyParts);

			id			= list.at(0).toInt();	 // ID (inner identification number)
			ra			= list.at(1).trimmed();
			dec			= list.at(2).trimmed();
			bMag			= list.at(3).toFloat();  // B magnitude
			vMag			= list.at(4).toFloat();	 // V magnitude
			oType			= list.at(5).trimmed();  // Object type
			mType			= list.at(6).trimmed();  // Morphological type of object
			majorAxisSize		= list.at(7).toFloat();  // major axis size (arcmin)
			minorAxisSize		= list.at(8).toFloat();	 // minor axis size (arcmin)
			orientationAngle	= list.at(9).toInt();	 // orientation angle (degrees)
			z			= list.at(10).toFloat(); // redshift
			zErr			= list.at(11).toFloat(); // error of redshift
			plx			= list.at(12).toFloat(); // parallax (mas)
			plxErr			= list.at(13).toFloat(); // error of parallax (mas)
			dist			= list.at(14).toFloat(); // distance (Mpc for galaxies, kpc for other objects)
			distErr			= list.at(15).toFloat(); // distance error (Mpc for galaxies, kpc for other objects)
			// -----------------------------------------------
			// cross-identification data
			// -----------------------------------------------
			NGC			= list.at(16).toInt();	 // NGC number
			IC			= list.at(17).toInt();	 // IC number
			M			= list.at(18).toInt();	 // M number
			C			= list.at(19).toInt();	 // C number
			B			= list.at(20).toInt();	 // B number
			Sh2			= list.at(21).toInt();	 // Sh2 number
			VdB			= list.at(22).toInt();	 // VdB number
			RCW			= list.at(23).toInt();	 // RCW number
			LDN			= list.at(24).toInt();	 // LDN number
			LBN			= list.at(25).toInt();	 // LBN number
			Cr			= list.at(26).toInt();	 // Cr number (alias: Col)
			Mel			= list.at(27).toInt();	 // Mel number
			PGC			= list.at(28).toInt();	 // PGC number (subset)
			UGC			= list.at(29).toInt();	 // UGC number (subset)
			Ced			= list.at(30).trimmed(); // Ced number			

			if (decimal)
			{
				// Convert from deg to rad
				raRad	= ra.toFloat()*M_PI/180.;
				decRad	= dec.toFloat()*M_PI/180.;
			}
			else
			{
				QStringList raLst;
				if (ra.contains(":"))
					raLst	= ra.split(":");
				else
					raLst	= ra.split(" ");

				QStringList decLst;
				if (dec.contains(":"))
					decLst = dec.split(":");
				else
					decLst = dec.split(" ");

				raRad	= raLst.at(0).toFloat() + raLst.at(1).toFloat()/60.f + raLst.at(2).toFloat()/3600.f;
				decRad	= qAbs(decLst.at(0).toFloat()) + decLst.at(1).toFloat()/60.f + decLst.at(2).toFloat()/3600.f;
				if (dec.startsWith("-")) decRad *= -1.f;

				raRad  *= M_PI/12.;	// Convert from hours to rad
				decRad *= M_PI/180.;    // Convert from deg to rad
			}

			majorAxisSize /= 60.f;	// Convert from arcmin to degrees
			minorAxisSize /= 60.f;	// Convert from arcmin to degrees

			// Warning: Hyades and LMC has visual magnitude less than 1.0 (0.5^m and 0.9^m)
			if (bMag <= 0.f) bMag = 99.f;
			if (vMag <= 0.f) vMag = 99.f;

			QStringList oTypes;
			oTypes << "G" << "GX" << "GC" << "OC" << "NB" << "PN" << "DN" << "RN" << "C+N"
			       << "RNE" << "HII" << "SNR" << "BN" << "EN" << "SA" << "SC" << "CL" << "IG"
			       << "RG" << "AGX" << "QSO" << "ISM" << "EMO" << "GNE" << "RAD" << "LIN"
			       << "BLL" << "BLA" << "MOC" << "YSO" << "Q?" << "PN?" << "*" << "SFR"
			       << "IR" << "**" << "MUL" << "PPN" << "GIG";

			switch (oTypes.indexOf(oType.toUpper()))
			{
				case 0:
				case 1:				
					nType = (unsigned int)Nebula::NebGx;
					break;
				case 2:
					nType = (unsigned int)Nebula::NebGc;
					break;
				case 3:
					nType = (unsigned int)Nebula::NebOc;
					break;
				case 4:
					nType = (unsigned int)Nebula::NebN;
					break;
				case 5:
					nType = (unsigned int)Nebula::NebPn;
					break;
				case 6:
				case 34:
					nType = (unsigned int)Nebula::NebDn;
					break;
				case 7:
				case 9:
					nType = (unsigned int)Nebula::NebRn;
					break;
				case 8:
					nType = (unsigned int)Nebula::NebCn;
					break;				
				case 10:
				case 23:
					nType = (unsigned int)Nebula::NebHII;
					break;
				case 11:
					nType = (unsigned int)Nebula::NebSNR;
					break;
				case 12:
					nType = (unsigned int)Nebula::NebBn;
					break;
				case 13:
					nType = (unsigned int)Nebula::NebEn;
					break;
				case 14:
					nType = (unsigned int)Nebula::NebSA;
					break;
				case 15:
					nType = (unsigned int)Nebula::NebSC;
					break;
				case 16:
					nType = (unsigned int)Nebula::NebCl;
					break;
				case 17:
				case 38:
					nType = (unsigned int)Nebula::NebIGx;
					break;
				case 18:
					nType = (unsigned int)Nebula::NebRGx;
					break;
				case 19:
				case 25: // LINER-type active galaxies
					nType = (unsigned int)Nebula::NebAGx;
					break;
				case 20:
					nType = (unsigned int)Nebula::NebQSO;
					break;
				case 21:
				case 24:
					nType = (unsigned int)Nebula::NebISM;
					break;
				case 22:
					nType = (unsigned int)Nebula::NebEMO;
					break;
				case 26:
					nType = (unsigned int)Nebula::NebBLL;
					break;
				case 27:
					nType = (unsigned int)Nebula::NebBLA;
					break;
				case 28:
				case 33:
					nType = (unsigned int)Nebula::NebMolCld;
					break;
				case 29:
					nType = (unsigned int)Nebula::NebYSO;
					break;
				case 30:
					nType = (unsigned int)Nebula::NebPossQSO;
					break;
				case 31:
					nType = (unsigned int)Nebula::NebPossPN;
					break;
				case 32:
				case 35:
				case 36:
					nType = (unsigned int)Nebula::NebStar;
					break;
				case 37:
					nType = (unsigned int)Nebula::NebPPN;
					break;
				default:
					nType = (unsigned int)Nebula::NebUnknown;
					break;
			}

			++readOk;

			dsoOutStream << id << raRad << decRad << bMag << vMag << nType << mType << majorAxisSize << minorAxisSize
				     << orientationAngle << z << zErr << plx << plxErr << dist  << distErr << NGC << IC << M << C
				     << B << Sh2 << VdB << RCW  << LDN << LBN << Cr << Mel << PGC << UGC << Ced;
		}
	}
	dsoIn.close();
	dsoOut.flush();
	dsoOut.close();
	qDebug() << "Converted" << readOk << "/" << totalRecords << "DSO records";
}

bool NebulaMgr::loadDSOCatalog(const QString &filename)
{
	QFile in(filename);
	if (!in.open(QIODevice::ReadOnly))
		return false;

	// TODO: Let's begin use gzipped data
	// QDataStream ins(StelUtils::uncompress(in.readAll()));
	QDataStream ins(&in);
	ins.setVersion(QDataStream::Qt_5_2);

	int totalRecords=0;
	while (!ins.atEnd())
	{
		// Create a new Nebula record
		NebulaP e = NebulaP(new Nebula);
		e->readDSO(ins);

		if (!objectInDisplayedCatalog(e)) continue;

		dsoArray.append(e);
		nebGrid.insert(qSharedPointerCast<StelRegionObject>(e));
		if (e->DSO_nb!=0)
			dsoIndex.insert(e->DSO_nb, e);
		++totalRecords;
	}
	in.close();
	qDebug() << "Loaded" << totalRecords << "DSO records";
	return true;
}

bool NebulaMgr::objectInDisplayedCatalog(NebulaP n)
{
	bool r = false;
	Nebula::CatalogGroup catalogFilters = getCatalogFilters();
	if ((catalogFilters&Nebula::CatM) && (n->M_nb>0))
		r = true;
	else if ((catalogFilters&Nebula::CatC) && (n->C_nb>0))
		r = true;
	else if ((catalogFilters&Nebula::CatNGC) && (n->NGC_nb>0))
		r = true;
	else if ((catalogFilters&Nebula::CatIC) && (n->IC_nb>0))
		r = true;
	else if ((catalogFilters&Nebula::CatB) && (n->B_nb>0))
		r = true;
	else if ((catalogFilters&Nebula::CatSh2) && (n->Sh2_nb>0))
		r = true;
	else if ((catalogFilters&Nebula::CatVdB) && (n->VdB_nb>0))
		r = true;
	else if ((catalogFilters&Nebula::CatRCW) && (n->RCW_nb>0))
		r = true;
	else if ((catalogFilters&Nebula::CatLDN) && (n->LDN_nb>0))
		r = true;
	else if ((catalogFilters&Nebula::CatLBN) && (n->LBN_nb>0))
		r = true;
	else if ((catalogFilters&Nebula::CatCr) && (n->Cr_nb>0))
		r = true;
	else if ((catalogFilters&Nebula::CatMel) && (n->Mel_nb>0))
		r = true;
	else if ((catalogFilters&Nebula::CatPGC) && (n->PGC_nb>0))
		r = true;
	else if ((catalogFilters&Nebula::CatUGC) && (n->UGC_nb>0))
		r = true;
	else if ((catalogFilters&Nebula::CatCed) && !(n->Ced_nb.isEmpty()))
		r = true;

	// Special case: objects without ID from current catalogs
	if (catalogFilters==Nebula::AllCatalogs)
		r = true;

	return r;
}

bool NebulaMgr::loadDSONames(const QString &filename)
{
	qDebug() << "Loading DSO name data ...";
	QFile dsoNameFile(filename);
	if (!dsoNameFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qWarning() << "DSO name data file" << QDir::toNativeSeparators(filename) << "not found.";
		return false;
	}

	// Read the names of the NGC objects
	QString name, record, ref, cdes;
	int totalRecords=0;
	int lineNumber=0;
	int readOk=0;
	int nb;
	NebulaP e;
	QRegExp commentRx("^(\\s*#.*|\\s*)$");
	QRegExp transRx("_[(]\"(.*)\"[)]");
	while (!dsoNameFile.atEnd())
	{
		record = QString::fromUtf8(dsoNameFile.readLine());
		lineNumber++;
		if (commentRx.exactMatch(record))
			continue;

		totalRecords++;

		// bytes 1 - 5, designator for catalogue (prefix)
		ref  = record.left(5).trimmed();
		// bytes 6 -20, identificator for object in the catalog
		cdes = record.mid(5, 15).trimmed().toUpper();
		// bytes 21-80, proper name of the object (translatable)
		name = record.mid(21).trimmed(); // Let gets the name with trimmed whitespaces

		nb = cdes.toInt();

		QStringList catalogs;		
		catalogs << "IC" << "M" << "C" << "CR" << "MEL" << "B" << "SH2" << "VDB" << "RCW" << "LDN" << "LBN"
			 << "NGC" << "PGC" << "UGC" << "CED";

		switch (catalogs.indexOf(ref.toUpper()))
		{
			case 0:
				e = searchIC(nb);
				break;
			case 1:
				e = searchM(nb);
				break;
			case 2:
				e = searchC(nb);
				break;
			case 3:
				e = searchCr(nb);
				break;
			case 4:
				e = searchMel(nb);
				break;
			case 5:
				e = searchB(nb);
				break;
			case 6:
				e = searchSh2(nb);
				break;
			case 7:
				e = searchVdB(nb);
				break;
			case 8:
				e = searchRCW(nb);
				break;
			case 9:
				e = searchLDN(nb);
				break;
			case 10:
				e = searchLBN(nb);
				break;
			case 11:
				e = searchNGC(nb);
				break;
			case 12:
				e = searchPGC(nb);
				break;
			case 13:
				e = searchUGC(nb);
				break;
			case 14:
				e = searchCed(cdes);
				break;			
			default:
				e = searchDSO(nb);
				break;
		}

		if (e)
		{
			if (transRx.exactMatch(name))
			{
				QString propName = transRx.capturedTexts().at(1).trimmed();
				QString currName = e->getEnglishName();
				if (currName.isEmpty())
					e->setProperName(propName);
				else if (currName!=propName)
					e->addNameAlias(propName);
			}


			readOk++;
		}
		//else
		//	qWarning() << "no position data for " << name << "at line" << lineNumber << "of" << QDir::toNativeSeparators(filename);
	}
	dsoNameFile.close();
	qDebug() << "Loaded" << readOk << "/" << totalRecords << "DSO name records successfully";
	return true;
}


void NebulaMgr::updateDSONames()
{
	updateSkyCulture(StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID());
	updateI18n();
}

void NebulaMgr::updateSkyCulture(const QString& skyCultureDir)
{
	QString namesFile = StelFileMgr::findFile("skycultures/" + skyCultureDir + "/dso_names.fab");

	foreach (const NebulaP& n, dsoArray)
		n->removeAllNames();

	if (namesFile.isEmpty())
	{
		QString setName = "default";
		QString dsoNamesPath = StelFileMgr::findFile("nebulae/" + setName + "/names.dat");
		if (dsoNamesPath.isEmpty())
		{
			qWarning() << "ERROR while loading deep-sky names data set " << setName;
			return;
		}
		loadDSONames(dsoNamesPath);
	}
	else
	{
		// Open file
		QFile dsoNamesFile(namesFile);
		if (!dsoNamesFile.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			qDebug() << "Cannot open file" << QDir::toNativeSeparators(namesFile);
			return;
		}

		// Now parse the file
		// lines to ignore which start with a # or are empty
		QRegExp commentRx("^(\\s*#.*|\\s*)$");

		// lines which look like records - we use the RE to extract the fields
		// which will be available in recRx.capturedTexts()
		QRegExp recRx("^\\s*([\\w\\s]+)\\s*\\|_[(]\"(.*)\"[)]\\s*([\\,\\d\\s]*)\\n");

		QString record, dsoId, nativeName;
		int totalRecords=0;
		int readOk=0;
		int lineNumber=0;
		while (!dsoNamesFile.atEnd())
		{
			record = QString::fromUtf8(dsoNamesFile.readLine());
			lineNumber++;

			// Skip comments
			if (commentRx.exactMatch(record))
				continue;

			totalRecords++;

			if (!recRx.exactMatch(record))
			{
				qWarning() << "ERROR - cannot parse record at line" << lineNumber << "in native deep-sky object names file" << QDir::toNativeSeparators(namesFile);
			}
			else
			{
				dsoId = recRx.capturedTexts().at(1).trimmed();
				nativeName = recRx.capturedTexts().at(2).trimmed(); // Use translatable text
				NebulaP e = search(dsoId);
				QString currentName = e->getEnglishName();
				if (currentName.isEmpty()) // Set native name of DSO
					e->setProperName(nativeName);
				else if (currentName!=nativeName) // Add traditional (well-known?) name of DSO as alias
					e->addNameAlias(nativeName);
				readOk++;
			}
		}
		dsoNamesFile.close();
		qDebug() << "Loaded" << readOk << "/" << totalRecords << "native names of deep-sky objects";
	}

	updateI18n();
}

void NebulaMgr::updateI18n()
{
	const StelTranslator& trans = StelApp::getInstance().getLocaleMgr().getSkyTranslator();
	foreach (NebulaP n, dsoArray)
		n->translateName(trans);
}


//! Return the matching Nebula object's pointer if exists or NULL
StelObjectP NebulaMgr::searchByNameI18n(const QString& nameI18n) const
{
	QString objw = nameI18n.toUpper();

	// Search by NGC numbers (possible formats are "NGC31" or "NGC 31")
	if (objw.left(3) == "NGC")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("NGC%1").arg(n->NGC_nb) == objw || QString("NGC %1").arg(n->NGC_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by common names
	foreach (const NebulaP& n, dsoArray)
	{
		QString objwcap = n->nameI18.toUpper();
		if (objwcap==objw)
			return qSharedPointerCast<StelObject>(n);
	}

	// Search by aliases of common names
	foreach (const NebulaP& n, dsoArray)
	{
		foreach(QString objwcapa, n->nameI18Aliases)
		{
			if (objwcapa.toUpper()==objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by IC numbers (possible formats are "IC466" or "IC 466")
	if (objw.left(2) == "IC")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("IC%1").arg(n->IC_nb) == objw || QString("IC %1").arg(n->IC_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}


	// Search by Messier numbers (possible formats are "M31" or "M 31")
	if (objw.left(1) == "M" && objw.left(3) != "MEL")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("M%1").arg(n->M_nb) == objw || QString("M %1").arg(n->M_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by Caldwell numbers (possible formats are "C31" or "C 31")
	if (objw.left(1) == "C" && objw.left(2) != "CR" && objw.left(3) != "CED")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("C%1").arg(n->C_nb) == objw || QString("C %1").arg(n->C_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by Barnard numbers (possible formats are "B31" or "B 31")
	if (objw.left(1) == "B")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("B%1").arg(n->B_nb) == objw || QString("B %1").arg(n->B_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by Sharpless numbers (possible formats are "Sh2-31" or "Sh 2-31")
	if (objw.left(2) == "SH")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("SH2-%1").arg(n->Sh2_nb) == objw || QString("SH 2-%1").arg(n->Sh2_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by Van den Bergh numbers (possible formats are "VdB31" or "VdB 31")
	if (objw.left(3) == "VDB")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("VDB%1").arg(n->VdB_nb) == objw || QString("VDB %1").arg(n->VdB_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by RCW numbers (possible formats are "RCW31" or "RCW 31")
	if (objw.left(3) == "RCW")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("RCW%1").arg(n->RCW_nb) == objw || QString("RCW %1").arg(n->RCW_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by LDN numbers (possible formats are "LDN31" or "LDN 31")
	if (objw.left(3) == "LDN")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("LDN%1").arg(n->LDN_nb) == objw || QString("LDN %1").arg(n->LDN_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by LBN numbers (possible formats are "LBN31" or "LBN 31")
	if (objw.left(3) == "LBN")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("LBN%1").arg(n->LBN_nb) == objw || QString("LBN %1").arg(n->LBN_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by Collinder numbers (possible formats are "Cr31" or "Cr 31")
	if (objw.left(2) == "CR")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("CR%1").arg(n->Cr_nb) == objw || QString("CR %1").arg(n->Cr_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by Melotte numbers (possible formats are "Mel31" or "Mel 31")
	if (objw.left(3) == "MEL")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("MEL%1").arg(n->Mel_nb) == objw || QString("MEL %1").arg(n->Mel_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by PGC numbers (possible formats are "PGC31" or "PGC 31")
	if (objw.left(3) == "PGC")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("PGC%1").arg(n->PGC_nb) == objw || QString("PGC %1").arg(n->PGC_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by UGC numbers (possible formats are "UGC31" or "UGC 31")
	if (objw.left(3) == "UGC")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("UGC%1").arg(n->UGC_nb) == objw || QString("UGC %1").arg(n->UGC_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by Cederblad numbers (possible formats are "Ced31" or "Ced 31")
	if (objw.left(3) == "CED")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("CED%1").arg(n->Ced_nb.trimmed().toUpper()) == objw.trimmed() || QString("CED %1").arg(n->Ced_nb.trimmed().toUpper()) == objw.trimmed())
				return qSharedPointerCast<StelObject>(n);
		}
	}

	return StelObjectP();
}


//! Return the matching Nebula object's pointer if exists or NULL
//! TODO split common parts of this and I18 fn above into a separate fn.
StelObjectP NebulaMgr::searchByName(const QString& name) const
{
	QString objw = name.toUpper();

	// Search by NGC numbers (possible formats are "NGC31" or "NGC 31")
	if (objw.startsWith("NGC"))
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("NGC%1").arg(n->NGC_nb) == objw || QString("NGC %1").arg(n->NGC_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by common names
	foreach (const NebulaP& n, dsoArray)
	{
		QString objwcap = n->englishName.toUpper();
		if (objwcap==objw)
			return qSharedPointerCast<StelObject>(n);
	}

	// Search by aliases of common names
	foreach (const NebulaP& n, dsoArray)
	{
		foreach(QString objwcapa, n->englishAliases)
		{
			if (objwcapa.toUpper()==objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by IC numbers (possible formats are "IC466" or "IC 466")
	if (objw.startsWith("IC"))
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("IC%1").arg(n->IC_nb) == objw || QString("IC %1").arg(n->IC_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by Messier numbers (possible formats are "M31" or "M 31")
	if (objw.startsWith("M") && !objw.startsWith("ME"))
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("M%1").arg(n->M_nb) == objw || QString("M %1").arg(n->M_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by Caldwell numbers (possible formats are "C31" or "C 31")
	if (objw.startsWith("C") && !objw.startsWith("CR") && !objw.startsWith("CE"))
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("C%1").arg(n->C_nb) == objw || QString("C %1").arg(n->C_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by Barnard numbers (possible formats are "B31" or "B 31")
	if (objw.startsWith("B"))
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("B%1").arg(n->B_nb) == objw || QString("B %1").arg(n->B_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by Sharpless numbers (possible formats are "Sh2-31" or "Sh 2-31")
	if (objw.startsWith("SH"))
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("SH2-%1").arg(n->Sh2_nb) == objw || QString("SH 2-%1").arg(n->Sh2_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by Van den Bergh numbers (possible formats are "VdB31" or "VdB 31")
	if (objw.startsWith("VDB"))
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("VDB%1").arg(n->VdB_nb) == objw || QString("VDB %1").arg(n->VdB_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by RCW numbers (possible formats are "RCW31" or "RCW 31")
	if (objw.startsWith("RCW"))
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("RCW%1").arg(n->RCW_nb) == objw || QString("RCW %1").arg(n->RCW_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by LDN numbers (possible formats are "LDN31" or "LDN 31")
	if (objw.startsWith("LDN"))
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("LDN%1").arg(n->LDN_nb) == objw || QString("LDN %1").arg(n->LDN_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by LBN numbers (possible formats are "LBN31" or "LBN 31")
	if (objw.startsWith("LBN"))
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("LBN%1").arg(n->LBN_nb) == objw || QString("LBN %1").arg(n->LBN_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by Collinder numbers (possible formats are "Cr31" or "Cr 31")
	if (objw.startsWith("CR") || objw.startsWith("COL"))
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("CR%1").arg(n->Cr_nb) == objw || QString("CR %1").arg(n->Cr_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by Melotte numbers (possible formats are "Mel31" or "Mel 31")
	if (objw.startsWith("MEL"))
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("MEL%1").arg(n->Mel_nb) == objw || QString("MEL %1").arg(n->Mel_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by PGC numbers (possible formats are "PGC31" or "PGC 31")
	if (objw.startsWith("PGC"))
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("PGC%1").arg(n->PGC_nb) == objw || QString("PGC %1").arg(n->PGC_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by UGC numbers (possible formats are "UGC31" or "UGC 31")
	if (objw.startsWith("UGC"))
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("UGC%1").arg(n->UGC_nb) == objw || QString("UGC %1").arg(n->UGC_nb) == objw)
				return qSharedPointerCast<StelObject>(n);
		}
	}

	// Search by Cederblad numbers (possible formats are "Ced31" or "Ced 31")
	if (objw.startsWith("CED"))
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (QString("CED%1").arg(n->Ced_nb.trimmed().toUpper()) == objw.trimmed() || QString("CED %1").arg(n->Ced_nb.trimmed().toUpper()) == objw.trimmed())
				return qSharedPointerCast<StelObject>(n);
		}
	}

	return NULL;
}

//! Find and return the list of at most maxNbItem objects auto-completing the passed object name
QStringList NebulaMgr::listMatchingObjects(const QString& objPrefix, int maxNbItem, bool useStartOfWords, bool inEnglish) const
{
	QStringList result;
	if (maxNbItem <= 0)
	{
		return result;
	}

	QString objw = objPrefix.toUpper();

	// Search by Messier objects number (possible formats are "M31" or "M 31")
	if (objw.size()>=1 && objw.left(1)=="M" && objw.left(3)!="MEL")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (n->M_nb==0) continue;
			QString constw = QString("M%1").arg(n->M_nb);
			QString constws = constw.mid(0, objw.size());
			if (constws==objw)
			{
				result << constws;
				continue;	// Prevent adding both forms for name
			}
			constw = QString("M %1").arg(n->M_nb);
			constws = constw.mid(0, objw.size());
			if (constws==objw)
				result << constw;
		}
	}

	// Search by Melotte objects number (possible formats are "Mel31" or "Mel 31")
	if (objw.size()>=1 && objw.left(3)=="MEL")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (n->Mel_nb==0) continue;
			QString constw = QString("MEL%1").arg(n->Mel_nb);
			QString constws = constw.mid(0, objw.size());
			if (constws==objw)
			{
				result << constws;
				continue;	// Prevent adding both forms for name
			}
			constw = QString("MEL %1").arg(n->Mel_nb);
			constws = constw.mid(0, objw.size());
			if (constws==objw)
				result << constw;
		}
	}

	// Search by IC objects number (possible formats are "IC466" or "IC 466")
	if (objw.size()>=1 && objw.left(2)=="IC")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (n->IC_nb==0) continue;
			QString constw = QString("IC%1").arg(n->IC_nb);
			QString constws = constw.mid(0, objw.size());
			if (constws==objw)
			{
				result << constws;
				continue;	// Prevent adding both forms for name
			}
			constw = QString("IC %1").arg(n->IC_nb);
			constws = constw.mid(0, objw.size());
			if (constws==objw)
				result << constw;
		}
	}

	// Search by NGC numbers (possible formats are "NGC31" or "NGC 31")
	foreach (const NebulaP& n, dsoArray)
	{
		if (n->NGC_nb==0) continue;
		QString constw = QString("NGC%1").arg(n->NGC_nb);
		QString constws = constw.mid(0, objw.size());
		if (constws==objw)
		{
			result << constws;
			continue;
		}
		constw = QString("NGC %1").arg(n->NGC_nb);
		constws = constw.mid(0, objw.size());
		if (constws==objw)
			result << constw;
	}

	// Search by PGC object numbers (possible formats are "PGC31" or "PGC 31")
	if (objw.size()>=1 && objw.left(3)=="PGC")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (n->PGC_nb==0) continue;
			QString constw = QString("PGC%1").arg(n->PGC_nb);
			QString constws = constw.mid(0, objw.size());
			if (constws==objw)
			{
				result << constws;	// Prevent adding both forms for name
				continue;
			}
			constw = QString("PGC %1").arg(n->PGC_nb);
			constws = constw.mid(0, objw.size());
			if (constws==objw)
				result << constw;
		}
	}

	// Search by UGC object numbers (possible formats are "UGC31" or "UGC 31")
	if (objw.size()>=1 && objw.left(3)=="UGC")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (n->UGC_nb==0) continue;
			QString constw = QString("UGC%1").arg(n->UGC_nb);
			QString constws = constw.mid(0, objw.size());
			if (constws==objw)
			{
				result << constws;
				continue;	// Prevent adding both forms for name
			}
			constw = QString("UGC %1").arg(n->UGC_nb);
			constws = constw.mid(0, objw.size());
			if (constws==objw)
				result << constw;
		}
	}

	// Search by Caldwell objects number (possible formats are "C31" or "C 31")
	if (objw.size()>=1 && objw.left(1)=="C" && objw.left(2)!="CR" && objw.left(2)!="CE")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (n->C_nb==0) continue;
			QString constw = QString("C%1").arg(n->C_nb);
			QString constws = constw.mid(0, objw.size());
			if (constws==objw)
			{
				result << constws;
				continue;	// Prevent adding both forms for name
			}
			constw = QString("C %1").arg(n->C_nb);
			constws = constw.mid(0, objw.size());
			if (constws==objw)
				result << constw;
		}
	}

	// Search by Collinder objects number (possible formats are "Cr31" or "Cr 31")
	if (objw.size()>=1 && objw.left(2)=="CR")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (n->Cr_nb==0) continue;
			QString constw = QString("CR%1").arg(n->Cr_nb);
			QString constws = constw.mid(0, objw.size());
			if (constws==objw)
			{
				result << constws;
				continue;	// Prevent adding both forms for name
			}
			constw = QString("CR %1").arg(n->Cr_nb);
			constws = constw.mid(0, objw.size());
			if (constws==objw)
				result << constw;
		}
	}

	// Search by Ced objects number (possible formats are "Ced31" or "Ced 31")
	if (objw.size()>=1 && objw.left(3)=="CED")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (n->Ced_nb==0) continue;
			QString constw = QString("Ced%1").arg(n->Ced_nb.trimmed());
			QString constws = constw.mid(0, objw.size());
			if (constws.toUpper()==objw.toUpper())
			{
				result << constws;
				continue;	// Prevent adding both forms for name
			}
			constw = QString("Ced %1").arg(n->Ced_nb.trimmed());
			constws = constw.mid(0, objw.size());
			if (constws.toUpper()==objw.toUpper())
				result << constw;
		}
	}

	// Search by Barnard objects number (possible formats are "B31" or "B 31")
	if (objw.size()>=1 && objw.left(1)=="B")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (n->B_nb==0) continue;
			QString constw = QString("B%1").arg(n->B_nb);
			QString constws = constw.mid(0, objw.size());
			if (constws==objw)
			{
				result << constws;
				continue;	// Prevent adding both forms for name
			}
			constw = QString("B %1").arg(n->B_nb);
			constws = constw.mid(0, objw.size());
			if (constws==objw)
				result << constw;
		}
	}

	// Search by Sharpless objects number (possible formats are "Sh2-31" or "Sh 2-31")
	if (objw.size()>=1 && objw.left(2)=="SH")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (n->Sh2_nb==0) continue;
			QString constw = QString("SH2-%1").arg(n->Sh2_nb);
			QString constws = constw.mid(0, objw.size());
			if (constws==objw)
			{
				result << constws;
				continue;	// Prevent adding both forms for name
			}
			constw = QString("SH 2-%1").arg(n->Sh2_nb);
			constws = constw.mid(0, objw.size());
			if (constws==objw)
				result << constw;
		}
	}

	// Search by Van den Bergh objects number (possible formats are "VdB31" or "VdB 31")
	if (objw.size()>=1 && objw.left(3)=="VDB")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (n->VdB_nb==0) continue;
			QString constw = QString("VDB%1").arg(n->VdB_nb);
			QString constws = constw.mid(0, objw.size());
			if (constws==objw)
			{
				result << constws;
				continue;	// Prevent adding both forms for name
			}
			constw = QString("VDB %1").arg(n->VdB_nb);
			constws = constw.mid(0, objw.size());
			if (constws==objw)
				result << constw;
		}
	}

	// Search by RCW objects number (possible formats are "RCW31" or "RCW 31")
	if (objw.size()>=1 && objw.left(3)=="RCW")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (n->RCW_nb==0) continue;
			QString constw = QString("RCW%1").arg(n->RCW_nb);
			QString constws = constw.mid(0, objw.size());
			if (constws==objw)
			{
				result << constws;
				continue;	// Prevent adding both forms for name
			}
			constw = QString("RCW %1").arg(n->RCW_nb);
			constws = constw.mid(0, objw.size());
			if (constws==objw)
				result << constw;
		}
	}

	// Search by LDN objects number (possible formats are "LDN31" or "LDN 31")
	if (objw.size()>=1 && objw.left(3)=="LDN")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (n->LDN_nb==0) continue;
			QString constw = QString("LDN%1").arg(n->LDN_nb);
			QString constws = constw.mid(0, objw.size());
			if (constws==objw)
			{
				result << constws;
				continue;	// Prevent adding both forms for name
			}
			constw = QString("LDN %1").arg(n->LDN_nb);
			constws = constw.mid(0, objw.size());
			if (constws==objw)
				result << constw;
		}
	}

	// Search by LBN objects number (possible formats are "LBN31" or "LBN 31")
	if (objw.size()>=1 && objw.left(3)=="LBN")
	{
		foreach (const NebulaP& n, dsoArray)
		{
			if (n->LBN_nb==0) continue;
			QString constw = QString("LBN%1").arg(n->LBN_nb);
			QString constws = constw.mid(0, objw.size());
			if (constws==objw)
			{
				result << constws;
				continue;	// Prevent adding both forms for name
			}
			constw = QString("LBN %1").arg(n->LBN_nb);
			constws = constw.mid(0, objw.size());
			if (constws==objw)
				result << constw;
		}
	}

	// Search by common names
	foreach (const NebulaP& n, dsoArray)
	{
		QString name = inEnglish ? n->englishName : n->nameI18;
		if (matchObjectName(name, objPrefix, useStartOfWords))
		{
			result.append(name);
		}
	}

	// Search by aliases of common names
	foreach (const NebulaP& n, dsoArray)
	{
		QStringList nameList = inEnglish ? n->englishAliases : n->nameI18Aliases;
		foreach(QString name, nameList)
		{
			if (matchObjectName(name, objPrefix, useStartOfWords))
				result.append(name);
		}
	}

	result.sort();
	if (result.size() > maxNbItem)
	{
		result.erase(result.begin() + maxNbItem, result.end());
	}

	return result;
}

QStringList NebulaMgr::listAllObjects(bool inEnglish) const
{
	QStringList result;
	foreach(const NebulaP& n, dsoArray)
	{		
		if (!n->getEnglishName().isEmpty())
		{
			if (inEnglish)
				result << n->getEnglishName();
			else
				result << n->getNameI18n();
		}
	}
	return result;
}

QStringList NebulaMgr::listAllObjectsByType(const QString &objType, bool inEnglish) const
{
	QStringList result;
	int type = objType.toInt();
	switch (type)
	{
		case 0: // Bright galaxies?
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->nType==type && qMin(n->vMag, n->bMag)<=10.)
				{
					if (!n->getEnglishName().isEmpty())
					{
						if (inEnglish)
							result << n->getEnglishName();
						else
							result << n->getNameI18n();
					}
					else if (n->NGC_nb>0)
						result << QString("NGC %1").arg(n->NGC_nb);
					else if (n->IC_nb>0)
						result << QString("IC %1").arg(n->IC_nb);
					else if (n->M_nb>0)
						result << QString("M %1").arg(n->M_nb);
					else if (n->C_nb>0)
						result << QString("C %1").arg(n->C_nb);
				}
			}
			break;
		case 100: // Messier Catalogue?
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->M_nb>0)
					result << QString("M%1").arg(n->M_nb);
			}
			break;
		case 101: // Caldwell Catalogue?
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->C_nb>0)
					result << QString("C%1").arg(n->C_nb);
			}
			break;
		case 102: // Barnard Catalogue?
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->B_nb>0)
					result << QString("B %1").arg(n->B_nb);
			}
			break;
		case 103: // Sharpless Catalogue?
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->Sh2_nb>0)
					result << QString("SH 2-%1").arg(n->Sh2_nb);
			}
			break;
		case 104: // Van den Bergh Catalogue
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->VdB_nb>0)
					result << QString("VdB %1").arg(n->VdB_nb);
			}
			break;
		case 105: // RCW Catalogue
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->RCW_nb>0)
					result << QString("RCW %1").arg(n->RCW_nb);
			}
			break;
		case 106: // Collinder Catalogue
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->Cr_nb>0)
					result << QString("Cr %1").arg(n->Cr_nb);
			}
			break;
		case 107: // Melotte Catalogue
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->Mel_nb>0)
					result << QString("Mel %1").arg(n->Mel_nb);
			}
			break;
		case 108: // New General Catalogue
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->NGC_nb>0)
					result << QString("NGC %1").arg(n->NGC_nb);
			}
			break;
		case 109: // Index Catalogue
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->IC_nb>0)
					result << QString("IC %1").arg(n->IC_nb);
			}
			break;
		case 110: // Lynds' Catalogue of Bright Nebulae
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->LBN_nb>0)
					result << QString("LBN %1").arg(n->LBN_nb);
			}
			break;
		case 111: // Lynds' Catalogue of Dark Nebulae
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->LDN_nb>0)
					result << QString("LDN %1").arg(n->LDN_nb);
			}
			break;
		case 114: // Cederblad Catalog
			foreach(const NebulaP& n, dsoArray)
			{
				if (!n->Ced_nb.isEmpty())
					result << QString("Ced %1").arg(n->Ced_nb);
			}
			break;
		case 150: // Dwarf galaxies
		{
			QStringList dwarfGalaxies;
			dwarfGalaxies  << "PGC 3589" << "PGC 3792" << "PGC 6830" << "PGC 10074" << "PGC 19441"
				       << "PGC 28913" << "PGC 29194" << "PGC 29653" << "PGC 50779" << "PGC 54074"
				       << "PGC 60095" << "PGC 63287" << "PGC 69519" << "PGC 88608" << "PGC 2807155"
				       << "PGC 3097691";
			result = dwarfGalaxies;
			break;
		}
		case 151: // Herschel 400 Catalogue
		{
			QList<int> h400list;
			h400list <<   40 <<  129 <<  136 <<  157 <<  185 <<  205 <<  225 <<  246 <<  247 <<  253
				 <<  278 <<  288 <<  381 <<  404 <<  436 <<  457 <<  488 <<  524 <<  559 <<  584
				 <<  596 <<  598 <<  613 <<  615 <<  637 <<  650 <<  654 <<  659 <<  663 <<  720
				 <<  752 <<  772 <<  779 <<  869 <<  884 <<  891 <<  908 <<  936 << 1022 << 1023
				 << 1027 << 1052 << 1055 << 1084 << 1245 << 1342 << 1407 << 1444 << 1501 << 1502
				 << 1513 << 1528 << 1535 << 1545 << 1647 << 1664 << 1788 << 1817 << 1857 << 1907
				 << 1931 << 1961 << 1964 << 1980 << 1999 << 2022 << 2024 << 2126 << 2129 << 2158
				 << 2169 << 2185 << 2186 << 2194 << 2204 << 2215 << 2232 << 2244 << 2251 << 2264
				 << 2266 << 2281 << 2286 << 2301 << 2304 << 2311 << 2324 << 2335 << 2343 << 2353
				 << 2354 << 2355 << 2360 << 2362 << 2371 << 2372 << 2392 << 2395 << 2403 << 2419
				 << 2420 << 2421 << 2422 << 2423 << 2438 << 2440 << 2479 << 2482 << 2489 << 2506
				 << 2509 << 2527 << 2539 << 2548 << 2567 << 2571 << 2613 << 2627 << 2655 << 2681
				 << 2683 << 2742 << 2768 << 2775 << 2782 << 2787 << 2811 << 2841 << 2859 << 2903
				 << 2950 << 2964 << 2974 << 2976 << 2985 << 3034 << 3077 << 3079 << 3115 << 3147
				 << 3166 << 3169 << 3184 << 3190 << 3193 << 3198 << 3226 << 3227 << 3242 << 3245
				 << 3277 << 3294 << 3310 << 3344 << 3377 << 3379 << 3384 << 3395 << 3412 << 3414
				 << 3432 << 3486 << 3489 << 3504 << 3521 << 3556 << 3593 << 3607 << 3608 << 3610
				 << 3613 << 3619 << 3621 << 3626 << 3628 << 3631 << 3640 << 3655 << 3665 << 3675
				 << 3686 << 3726 << 3729 << 3810 << 3813 << 3877 << 3893 << 3898 << 3900 << 3912
				 << 3938 << 3941 << 3945 << 3949 << 3953 << 3962 << 3982 << 3992 << 3998 << 4026
				 << 4027 << 4030 << 4036 << 4039 << 4041 << 4051 << 4085 << 4088 << 4102 << 4111
				 << 4143 << 4147 << 4150 << 4151 << 4179 << 4203 << 4214 << 4216 << 4245 << 4251
				 << 4258 << 4261 << 4273 << 4274 << 4278 << 4281 << 4293 << 4303 << 4314 << 4346
				 << 4350 << 4361 << 4365 << 4371 << 4394 << 4414 << 4419 << 4429 << 4435 << 4438
				 << 4442 << 4448 << 4449 << 4450 << 4459 << 4473 << 4477 << 4478 << 4485 << 4490
				 << 4494 << 4526 << 4527 << 4535 << 4536 << 4546 << 4548 << 4550 << 4559 << 4565
				 << 4570 << 4594 << 4596 << 4618 << 4631 << 4636 << 4643 << 4654 << 4656 << 4660
				 << 4665 << 4666 << 4689 << 4697 << 4698 << 4699 << 4725 << 4753 << 4754 << 4762
				 << 4781 << 4800 << 4845 << 4856 << 4866 << 4900 << 4958 << 4995 << 5005 << 5033
				 << 5054 << 5195 << 5248 << 5273 << 5322 << 5363 << 5364 << 5466 << 5473 << 5474
				 << 5557 << 5566 << 5576 << 5631 << 5634 << 5676 << 5689 << 5694 << 5746 << 5846
				 << 5866 << 5897 << 5907 << 5982 << 6118 << 6144 << 6171 << 6207 << 6217 << 6229
				 << 6235 << 6284 << 6287 << 6293 << 6304 << 6316 << 6342 << 6355 << 6356 << 6369
				 << 6401 << 6426 << 6440 << 6445 << 6451 << 6514 << 6517 << 6520 << 6522 << 6528
				 << 6540 << 6543 << 6544 << 6553 << 6568 << 6569 << 6583 << 6624 << 6629 << 6633
				 << 6638 << 6642 << 6645 << 6664 << 6712 << 6755 << 6756 << 6781 << 6802 << 6818
				 << 6823 << 6826 << 6830 << 6834 << 6866 << 6882 << 6885 << 6905 << 6910 << 6934
				 << 6939 << 6940 << 6946 << 7000 << 7006 << 7008 << 7009 << 7044 << 7062 << 7086
				 << 7128 << 7142 << 7160 << 7209 << 7217 << 7243 << 7296 << 7331 << 7380 << 7448
				 << 7479 << 7510 << 7606 << 7662 << 7686 << 7723 << 7727 << 7789 << 7790 << 7814;
			for (int i=0; i < h400list.size(); i++)
				result << QString("NGC %1").arg(h400list.at(i));
			break;
		}
		default:
		{
			foreach (const NebulaP& n, dsoArray)
			{
				if (n->nType==type)
				{
					if (!n->getEnglishName().isEmpty())
					{
						if (inEnglish)
							result << n->getEnglishName();
						else
							result << n->getNameI18n();
					}
					else if (n->NGC_nb>0)
						result << QString("NGC %1").arg(n->NGC_nb);
					else if (n->IC_nb>0)
						result << QString("IC %1").arg(n->IC_nb);
					else if (n->M_nb>0)
						result << QString("M %1").arg(n->M_nb);
					else if (n->C_nb>0)
						result << QString("C %1").arg(n->C_nb);
					else if (n->B_nb>0)
						result << QString("B %1").arg(n->B_nb);
					else if (n->Sh2_nb>0)
						result << QString("SH 2-%1").arg(n->Sh2_nb);
					else if (n->VdB_nb>0)
						result << QString("VdB %1").arg(n->VdB_nb);
					else if (n->RCW_nb>0)
						result << QString("RCW %1").arg(n->RCW_nb);
					else if (n->LBN_nb>0)
						result << QString("LBN %1").arg(n->LBN_nb);
					else if (n->LDN_nb>0)
						result << QString("LDN %1").arg(n->LDN_nb);
					else if (n->Cr_nb>0)
						result << QString("Cr %1").arg(n->Cr_nb);
					else if (n->Mel_nb>0)
						result << QString("Mel %1").arg(n->Mel_nb);

				}
			}
			break;
		}
	}

	result.removeDuplicates();
	return result;
}

QList<NebulaP> NebulaMgr::getDeepSkyObjectsByType(const QString &objType)
{
	QList<NebulaP> dso;
	int type = objType.toInt();
	switch (type)
	{
		case 100: // Messier Catalogue?
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->M_nb>0)
					dso.append(n);
			}
			break;
		case 101: // Caldwell Catalogue?
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->C_nb>0)
					dso.append(n);
			}
			break;
		case 102: // Barnard Catalogue?
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->B_nb>0)
					dso.append(n);
			}
			break;
		case 103: // Sharpless Catalogue?
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->Sh2_nb>0)
					dso.append(n);
			}
			break;
		case 104: // Van den Bergh Catalogue
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->VdB_nb>0)
					dso.append(n);
			}
			break;
		case 105: // RCW Catalogue
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->RCW_nb>0)
					dso.append(n);
			}
			break;
		case 106: // Collinder Catalogue
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->Cr_nb>0)
					dso.append(n);
			}
			break;
		case 107: // Melotte Catalogue
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->Mel_nb>0)
					dso.append(n);
			}
			break;
		case 108: // New General Catalogue
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->NGC_nb>0)
					dso.append(n);
			}
			break;
		case 109: // Index Catalogue
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->IC_nb>0)
					dso.append(n);
			}
			break;
		case 110: // Lynds' Catalogue of Bright Nebulae
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->LBN_nb>0)
					dso.append(n);
			}
			break;
		case 111: // Lynds' Catalogue of Dark Nebulae
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->LDN_nb>0)
					dso.append(n);
			}
			break;
		case 112: // Principal Galaxy Catalog
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->PGC_nb>0)
					dso.append(n);
			}
			break;
		case 113: // The Uppsala General Catalogue of Galaxies
			foreach(const NebulaP& n, dsoArray)
			{
				if (n->UGC_nb>0)
					dso.append(n);
			}
			break;
		case 114: // Cederblad Catalog
			foreach(const NebulaP& n, dsoArray)
			{
				if (!n->Ced_nb.isEmpty())
					dso.append(n);
			}
			break;
		case 150: // Dwarf galaxies
		{
			QList<int> dwarfGalaxies;
			dwarfGalaxies  <<    3589 <<    3792 <<    6830 <<   10074 <<   19441 <<   28913
				       <<   29194 <<   29653 <<   50779 <<   54074 <<   60095 <<   63287
				       <<   69519 <<   88608 << 2807155 << 3097691;
			NebulaP ds;
			for (int i=0; i < dwarfGalaxies.size(); i++)
			{
				ds = searchPGC(dwarfGalaxies.at(i));
				if (!ds.isNull())
					dso.append(ds);
			}
			break;
		}
		case 151: // Herschel 400 Catalogue
		{
			QList<int> h400list;
			h400list <<   40 <<  129 <<  136 <<  157 <<  185 <<  205 <<  225 <<  246 <<  247 <<  253
				 <<  278 <<  288 <<  381 <<  404 <<  436 <<  457 <<  488 <<  524 <<  559 <<  584
				 <<  596 <<  598 <<  613 <<  615 <<  637 <<  650 <<  654 <<  659 <<  663 <<  720
				 <<  752 <<  772 <<  779 <<  869 <<  884 <<  891 <<  908 <<  936 << 1022 << 1023
				 << 1027 << 1052 << 1055 << 1084 << 1245 << 1342 << 1407 << 1444 << 1501 << 1502
				 << 1513 << 1528 << 1535 << 1545 << 1647 << 1664 << 1788 << 1817 << 1857 << 1907
				 << 1931 << 1961 << 1964 << 1980 << 1999 << 2022 << 2024 << 2126 << 2129 << 2158
				 << 2169 << 2185 << 2186 << 2194 << 2204 << 2215 << 2232 << 2244 << 2251 << 2264
				 << 2266 << 2281 << 2286 << 2301 << 2304 << 2311 << 2324 << 2335 << 2343 << 2353
				 << 2354 << 2355 << 2360 << 2362 << 2371 << 2372 << 2392 << 2395 << 2403 << 2419
				 << 2420 << 2421 << 2422 << 2423 << 2438 << 2440 << 2479 << 2482 << 2489 << 2506
				 << 2509 << 2527 << 2539 << 2548 << 2567 << 2571 << 2613 << 2627 << 2655 << 2681
				 << 2683 << 2742 << 2768 << 2775 << 2782 << 2787 << 2811 << 2841 << 2859 << 2903
				 << 2950 << 2964 << 2974 << 2976 << 2985 << 3034 << 3077 << 3079 << 3115 << 3147
				 << 3166 << 3169 << 3184 << 3190 << 3193 << 3198 << 3226 << 3227 << 3242 << 3245
				 << 3277 << 3294 << 3310 << 3344 << 3377 << 3379 << 3384 << 3395 << 3412 << 3414
				 << 3432 << 3486 << 3489 << 3504 << 3521 << 3556 << 3593 << 3607 << 3608 << 3610
				 << 3613 << 3619 << 3621 << 3626 << 3628 << 3631 << 3640 << 3655 << 3665 << 3675
				 << 3686 << 3726 << 3729 << 3810 << 3813 << 3877 << 3893 << 3898 << 3900 << 3912
				 << 3938 << 3941 << 3945 << 3949 << 3953 << 3962 << 3982 << 3992 << 3998 << 4026
				 << 4027 << 4030 << 4036 << 4039 << 4041 << 4051 << 4085 << 4088 << 4102 << 4111
				 << 4143 << 4147 << 4150 << 4151 << 4179 << 4203 << 4214 << 4216 << 4245 << 4251
				 << 4258 << 4261 << 4273 << 4274 << 4278 << 4281 << 4293 << 4303 << 4314 << 4346
				 << 4350 << 4361 << 4365 << 4371 << 4394 << 4414 << 4419 << 4429 << 4435 << 4438
				 << 4442 << 4448 << 4449 << 4450 << 4459 << 4473 << 4477 << 4478 << 4485 << 4490
				 << 4494 << 4526 << 4527 << 4535 << 4536 << 4546 << 4548 << 4550 << 4559 << 4565
				 << 4570 << 4594 << 4596 << 4618 << 4631 << 4636 << 4643 << 4654 << 4656 << 4660
				 << 4665 << 4666 << 4689 << 4697 << 4698 << 4699 << 4725 << 4753 << 4754 << 4762
				 << 4781 << 4800 << 4845 << 4856 << 4866 << 4900 << 4958 << 4995 << 5005 << 5033
				 << 5054 << 5195 << 5248 << 5273 << 5322 << 5363 << 5364 << 5466 << 5473 << 5474
				 << 5557 << 5566 << 5576 << 5631 << 5634 << 5676 << 5689 << 5694 << 5746 << 5846
				 << 5866 << 5897 << 5907 << 5982 << 6118 << 6144 << 6171 << 6207 << 6217 << 6229
				 << 6235 << 6284 << 6287 << 6293 << 6304 << 6316 << 6342 << 6355 << 6356 << 6369
				 << 6401 << 6426 << 6440 << 6445 << 6451 << 6514 << 6517 << 6520 << 6522 << 6528
				 << 6540 << 6543 << 6544 << 6553 << 6568 << 6569 << 6583 << 6624 << 6629 << 6633
				 << 6638 << 6642 << 6645 << 6664 << 6712 << 6755 << 6756 << 6781 << 6802 << 6818
				 << 6823 << 6826 << 6830 << 6834 << 6866 << 6882 << 6885 << 6905 << 6910 << 6934
				 << 6939 << 6940 << 6946 << 7000 << 7006 << 7008 << 7009 << 7044 << 7062 << 7086
				 << 7128 << 7142 << 7160 << 7209 << 7217 << 7243 << 7296 << 7331 << 7380 << 7448
				 << 7479 << 7510 << 7606 << 7662 << 7686 << 7723 << 7727 << 7789 << 7790 << 7814;
			NebulaP ds;
			for (int i=0; i < h400list.size(); i++)
			{
				ds = searchNGC(h400list.at(i));
				if (!ds.isNull())
					dso.append(ds);
			}
			break;
		}
		default:
		{
			foreach (const NebulaP& n, dsoArray)
			{
				if (n->nType==type)
					dso.append(n);
			}
			break;
		}
	}

	return dso;
}
