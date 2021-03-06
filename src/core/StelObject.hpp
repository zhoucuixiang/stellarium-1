/*
 * Stellarium
 * Copyright (C) 2002 Fabien Chereau
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

#ifndef _STELOBJECT_HPP_
#define _STELOBJECT_HPP_

#include "VecMath.hpp"
#include "StelObjectType.hpp"
#include "StelRegionObject.hpp"

#include <QFlags>
#include <QString>

class StelCore;

//! The base abstract class for sky objects used in Stellarium like Stars, Planets, Constellations etc...
//! Normally you should use StelObjectP instead of StelObject* which have by default the same behaviour,
//! but which can be added reference counting if needed.
//! @sa StelObjectP
class StelObject : public StelRegionObject
{
	//Required for Q_FLAGS macro, this requires this header to be MOC'ed
	Q_GADGET
	Q_FLAGS(InfoStringGroupFlags InfoStringGroup)
public:
	//! Used as named bitfield flags as specifiers to
	//! filter results of getInfoString. The precise definition of these should
	//! be documented in the getInfoString documentation for the derived classes
	//! for all specifiers which are defined in that derivative.
	//! Use InfoStringGroup instead.
	enum InfoStringGroupFlags
	{
		Name			= 0x00000001, //!< An object's name
		CatalogNumber		= 0x00000002, //!< Catalog numbers
		Magnitude		= 0x00000004, //!< Magnitude related data
		RaDecJ2000		= 0x00000008, //!< The equatorial position (J2000 ref)
		RaDecOfDate		= 0x00000010, //!< The equatorial position (of date)
		AltAzi			= 0x00000020, //!< The position (Altitude/Azimuth)
		Distance		= 0x00000040, //!< Info about an object's distance
		Size			= 0x00000080, //!< Info about an object's size
		Extra			= 0x00000100, //!< Derived class-specific extra fields
		HourAngle		= 0x00000200, //!< The hour angle + DE (of date)
		AbsoluteMagnitude	= 0x00000400, //!< The absolute magnitude
		GalacticCoord		= 0x00000800, //!< The galactic position
		SupergalacticCoord	= 0x00001000, //!< The supergalactic position
		ObjectType		= 0x00002000, //!< The type of the object (star, planet, etc.)
		EclipticCoordJ2000	= 0x00004000, //!< The ecliptic position (J2000.0 ref) [+ XYZ of VSOP87A (used mainly for debugging, not public)]
		EclipticCoordOfDate	= 0x00008000, //!< The ecliptic position (of date)
		IAUConstellation        = 0x00010000, //!< Three-letter constellation code (And, Boo, Cas, ...)
		SiderealTime		= 0x00020000, //!< Mean and Apparent Sidereal Time
		NoFont			= 0x00040000,
		PlainText		= 0x00080000,  //!< Strip HTML tags from output
// TODO GZ
//		RaDecJ2000Planetocentric  = 0x00020000, //!< The planetocentric equatorial position (J2000 ref) [Mostly to compare with almanacs]
//		RaDecOfDatePlanetocentric = 0x00040000  //!< The planetocentric equatorial position (of date)


	};
	Q_DECLARE_FLAGS(InfoStringGroup, InfoStringGroupFlags)

	//! A pre-defined set of specifiers for the getInfoString flags argument to getInfoString
	static const InfoStringGroupFlags AllInfo = (InfoStringGroupFlags)(Name|CatalogNumber|Magnitude|RaDecJ2000|RaDecOfDate|AltAzi|Distance|Size|Extra|HourAngle|
									   AbsoluteMagnitude|GalacticCoord|SupergalacticCoord|ObjectType|EclipticCoordJ2000|
									   EclipticCoordOfDate|IAUConstellation|SiderealTime);
	//! A pre-defined set of specifiers for the getInfoString flags argument to getInfoString
	static const InfoStringGroupFlags ShortInfo = (InfoStringGroupFlags)(Name|CatalogNumber|Magnitude|RaDecJ2000);

	virtual ~StelObject() {}

	//! Default implementation of the getRegion method.
	//! Return the spatial region of the object.
	virtual SphericalRegionP getRegion() const {return SphericalRegionP(new SphericalPoint(getJ2000EquatorialPos(NULL)));}

	//! Default implementation of the getPointInRegion method.
	//! Return the J2000 Equatorial Position of the object.
	virtual Vec3d getPointInRegion() const {return getJ2000EquatorialPos(NULL);}
	
	//! Write I18n information about the object in QString.
	//! @param core the StelCore object to use
	//! @param flags a set of InfoStringGroup flags which are used to
	//! filter the return value - including specified types of information
	//! and altering the output format.
	//! @return an HTML string containing information about the StelObject.
	virtual QString getInfoString(const StelCore *core, const InfoStringGroup& flags=StelObject::AllInfo) const = 0;

	//! Return a key/value map with data about an object's position, magnitude and so on. Useful in a context like scripting.
	//! Derived objects can add their own special information tags.
	//! @param core the current StelCore
	//! @return a map of object data.  Keys:
	//! - above-horizon : true, if celestial body is above horizon
	//! - altitude : apparent altitude angle in decimal degrees
	//! - azimuth : apparent azimuth angle in decimal degrees
	//! - altitude-geometric : geometric altitude angle in decimal degrees
	//! - azimuth-geometric : geometric azimuth angle in decimal degrees
	//! - ra : right ascension angle (current date frame) in decimal degrees
	//! - dec : declination angle (current date frame) in decimal degrees
	//! - raJ2000 : right ascension angle (J2000 frame) in decimal degrees
	//! - decJ2000 : declination angle (J2000 frame) in decimal degrees
	//! - glong : galactic longitude in decimal degrees
	//! - glat : galactic latitude in decimal degrees
	//! - sglong : supergalactic longitude in decimal degrees
	//! - sglat : supergalactic latitude in decimal degrees
	//! - elong : ecliptic longitude in decimal degrees (on Earth only!)
	//! - elat : ecliptic latitude in decimal degrees (on Earth only!)
	//! - elongJ2000 : ecliptic longitude (Earth's J2000 frame) in decimal degrees
	//! - elatJ2000 : ecliptic latitude (Earth's J2000 frame) in decimal degrees
	//! - vmag : visual magnitude
	//! - vmage : visual magnitude (after atmospheric extinction)
	//! - size: angular size in radians
	//! - size-dd : angular size in decimal degrees
	//! - size-deg : angular size in decimal degrees (formatted string)
	//! - size-dms : angular size in DMS format
	//! - name : english name of the object
	//! - localized-name : localized name	
	virtual QVariantMap getInfoMap(const StelCore *core) const;

	//! Return object's type. It should be the name of the class.
	virtual QString getType() const = 0;

	//! Return object's name in english
	virtual QString getEnglishName() const = 0;

	//! Return translated object's name
	virtual QString getNameI18n() const = 0;

	//! Get observer-centered equatorial coordinates at equinox J2000
	virtual Vec3d getJ2000EquatorialPos(const StelCore* core) const = 0;

	//! Get observer-centered equatorial coordinate at the current equinox
	//! The frame has its Z axis at the planet's current rotation axis
	//! At time 2000-01-01 this frame is almost the same as J2000, but ONLY if the observer is on earth
	Vec3d getEquinoxEquatorialPos(const StelCore* core) const;
	//! Like getEquinoxEquatorialPos(core), but always adds refraction correction to the position.
	Vec3d getEquinoxEquatorialPosApparent(const StelCore* core) const;
	//! Like getEquinoxEquatorialPos(core), but adds refraction correction to the position if atmosphere is active.
	Vec3d getEquinoxEquatorialPosAuto(const StelCore* core) const;

	//! Get observer-centered galactic coordinates
	Vec3d getGalacticPos(const StelCore* core) const;

	//! Get observer-centered supergalactic coordinates
	Vec3d getSupergalacticPos(const StelCore* core) const;

	//! Get observer-centered hour angle + declination (at current equinox)
	//! It is the geometric position, i.e. without taking refraction effect into account.
	//! The frame has its Z axis at the planet's current rotation axis
	Vec3d getSiderealPosGeometric(const StelCore* core) const;

	//! Get observer-centered hour angle + declination (at current equinox)
	//! It is the apparent position, i.e. taking the refraction effect into account.
	//! The frame has its Z axis at the planet's current rotation axis
	Vec3d getSiderealPosApparent(const StelCore* core) const;

	//! Get observer-centered alt/az position
	//! It is the geometric position, i.e. without taking refraction effect into account.
	//! The frame has its Z axis at the zenith
	Vec3d getAltAzPosGeometric(const StelCore* core) const;

	//! Get observer-centered alt/az position
	//! It is the apparent position, i.e. taking the refraction effect into account.
	//! The frame has its Z axis at the zenith
	Vec3d getAltAzPosApparent(const StelCore* core) const;

	//! Get observer-centered alt/az position
	//! It is the automatic position, i.e. taking the refraction effect into account if atmosphere is on.
	//! The frame has its Z axis at the zenith
	Vec3d getAltAzPosAuto(const StelCore* core) const;

	//! Checking position an object above mathematical horizon for current location.
	//! @return true if object an above mathematical horizon
	bool isAboveHorizon(const StelCore* core) const;

	//! Checking position an object above real horizon for current location.
	//! @return true if object an above real horizon (uses test for landscapes)
	bool isAboveRealHorizon(const StelCore* core) const;

	//! Return object's apparent V magnitude as seen from observer, without including extinction.
	virtual float getVMagnitude(const StelCore* core) const;
	
	//! Return object's apparent V magnitude as seen from observer including extinction.
	//! Extinction obviously only if atmosphere=on.
	float getVMagnitudeWithExtinction(const StelCore* core) const;

	//! Return a priority value which is used to discriminate objects by priority
	//! As for magnitudes, the lower is the higher priority
	virtual float getSelectPriority(const StelCore*) const;

	//! Get a color used to display info about the object
	virtual Vec3f getInfoColor() const {return Vec3f(1,1,1);}

	//! Return the best FOV in degree to use for a close view of the object
	virtual double getCloseViewFov(const StelCore*) const {return 10.;}

	//! Return the best FOV in degree to use for a global view of the object satellite system (if there are satellites)
	virtual double getSatellitesFov(const StelCore*) const {return -1.;}
	virtual double getParentSatellitesFov(const StelCore*) const {return -1.;}

	//! Return the angular radius of a circle containing the object as seen from the observer
	//! with the circle center assumed to be at getJ2000EquatorialPos().
	//! @return radius in degree. This value is the apparent angular size of the object, and is independent of the current FOV.
	virtual double getAngularSize(const StelCore* core) const = 0;

protected:

	//! Format the positional info string contain J2000/of date/altaz/hour angle positions for the object
	QString getPositionInfoString(const StelCore *core, const InfoStringGroup& flags) const;

	//! Apply post processing on the info string
	void postProcessInfoString(QString& str, const InfoStringGroup& flags) const;
private:
	static int stelObjectPMetaTypeID;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(StelObject::InfoStringGroup)

#endif // _STELOBJECT_HPP_
