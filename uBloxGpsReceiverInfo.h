#pragma once
#include <stdint.h>
#include <string>
#include <vector>

/*
*	UBX ASSOCIATED IDs & SIZES
*/

//! @brief UBX Structure
struct UBX
{
	struct Header;
	struct ACK;
	struct CFG;
	struct MON;
	struct NAV;
};

//! @brief UBX Header for sync chars
struct UBX::Header
{
	static const uint8_t SyncChar1 = 0xB5;
	static const uint8_t SyncChar2 = 0x62;
};

struct UBX::ACK
{
	struct ACK_;
	struct NACK;

	static const uint8_t classId = 0x05;
};

struct UBX::ACK::ACK_
{
	static const uint8_t messageId = 0x01;
};

struct UBX::ACK::NACK
{
	static const uint8_t messageId = 0x00;
};

//! @brief UBX CFG class id and messages
struct UBX::CFG
{
	struct RATE;
	struct VALSET;
	struct MSG;

	static const uint8_t classId = 0x06;
};

//! @brief holds UBX CFG RATE message id, satellite source, and message hz capabilities
struct UBX::CFG::RATE
{
	static const uint8_t messageId = 0x08;

	static const uint8_t UTC_SOURCE = 0x00;
	static const uint8_t GPS_SOURCE = 0x01;
	static const uint8_t GLO_SOURCE = 0x02;
	static const uint8_t BDS_SOURCE = 0x03;
	static const uint8_t GAL_SOURCE = 0x04;

	static const uint8_t MEASURE_HZ1 = 0xE8;
	static const uint8_t MEASURE_HZ5 = 0xC8;
	static const uint8_t MEASURE_HZ10 = 0x64;

	static const uint8_t NAV_CYCLES1 = 0x01;
	static const uint8_t NAV_CYCLES2 = 0x02;
	static const uint8_t NAV_CYCLES3 = 0x03;
};

//! @ brief holds UBX CFG VALSET message id, satellite source, and message hz capabilities
struct UBX::CFG::VALSET
{
	static const uint8_t messageId = 0x8A;
};

struct UBX::CFG::MSG
{
	static const uint8_t messageId = 0x01;
};

//! @brief UBX MON class id and messages
struct UBX::MON
{
	struct VER;

	static const uint8_t classId = 0x0A;
};

//! @brief holds UBX MON VER message id
struct UBX::MON::VER
{
	static const uint8_t messageId = 0x04;
};

//! @brief UBX NAV class id and messages
struct UBX::NAV
{
	struct COV;
	struct DOP;
	struct SAT;
	struct STATUS;
	struct PVT;
	struct POSECEF;
	struct POSLLH;
	struct VELECEF;
	struct VELNED;
	struct TIMEUTC;

	static const uint8_t classId = 0x01;
};

//! @brief holds UBX NAV COV message id, payload length, and full message length
struct UBX::NAV::COV
{
	static const uint8_t messageId = 0x36;
	static const int payloadLength = 64;
	static const int fullMessageLength = 72;
};

//! @brief holds UBX NAV DOP message id, payload length, and full message length
struct UBX::NAV::DOP
{
	static const uint8_t messageId = 0x04;
	static const int payloadLength = 18;
	static const int fullMessageLength = 26;
};

//! @brief holds UBX NAV SAT message id and supported message version for current firmware (1.00)
struct UBX::NAV::SAT
{
	static const uint8_t messageId = 0x35;
	static const uint8_t messageVersion = 0x01;
};

//! @brief holds UBX NAV STATUS message id, payload length, and full message length
struct UBX::NAV::STATUS
{
	static const uint8_t messageId = 0x03;
	static const int payloadLength = 16;
	static const int fullMessageLength = 24;
};

//! @brief holds UBX NAV PVT message id, payload length, and full message length
struct UBX::NAV::PVT
{
	static const uint8_t messageId = 0x07;
	static const int payloadLength = 92;
	static const int fullMessageLength = 100;
};

//! @brief holds UBX NAV POSECEF message id, payload length, and full message length
struct UBX::NAV::POSECEF
{
	static const uint8_t messageId = 0x1;
	static const int payloadLength = 20;
	static const int fullMessageLength = 28;
};

//! @brief holds UBX NAV POSLLH message id, payload length, and full message length
struct UBX::NAV::POSLLH
{
	static const uint8_t messageId = 0x02;
	static const int payloadLength = 28;
	static const int fullMessageLength = 36;
};

//! @brief holds UBX NAV VELECEF message id, payload length, and full message length
struct UBX::NAV::VELECEF
{
	static const uint8_t messageId = 0x11;
	static const int payloadLength = 20;
	static const int fullMessageLength = 28;
};

//! @brief holds UBX NAV VELNED message id, payload length, and full message length
struct UBX::NAV::VELNED
{
	static const uint8_t messageId = 0x12;
	static const int payloadLength = 36;
	static const int fullMessageLength = 44;
};

//! @brief holds UBX NAV TIMEUTC message id, payload length, and full message length
struct UBX::NAV::TIMEUTC
{
	static const uint8_t messageId = 0x21;
	static const int payloadLength = 20;
	static const int fullMessageLength = 28;
};

/*
*	UBX DATA & MESSAGE STRUCTURES
*/

//! @brief A UBX Satellite Variable
struct UBX_SAT_DATA
{
	uint8_t gnssId = 0;
	uint8_t satelliteId = 0;
	uint8_t carrierToNoiseRatioInDbhz = 0;
	int8_t elevationInDeg = 0;
	int16_t azimuthInDeg = 0;
	int16_t pseudoRangeResidualInM = 0;
	uint32_t flags = 0;				// BITMASK FLAGS - SEE PAGE 163 OF UBLOX MANUAL

	/*
	uint8_t signalQuality = 0;
	bool satelliteUsed = false;
	uint8_t health = 0;
	bool differentialCorrectionDataAvailable = false;
	bool carrierSmoothedPseudorangeUsed;
	uint8_t orbitSource = 0;
	bool ephemerisAvailable;
	bool almanacAvailable;
	bool assistNowOfflineDataAvailable;
	bool assistNowAutonomousDataAvailable;
	bool sbasCorrectionsUsed;
	bool rtcmCorrectionsUsed;
	bool slasCorrectionsUsed;
	bool pseudorangeCorrectionsUsed;
	bool carrierRangeCorrectionsUsed;
	bool dopplerCorrectionsUsed;
	*/

	// members
	// for finding the satellite in the vector. 
	bool operator == (const UBX_SAT_DATA& s) const
	{
		return satelliteId == s.satelliteId;
	}
};

//! @brief Holds UBX MON Version data
struct UBX_MON_VER
{
	char softwareVersion[30] = { 0 };
	char hardwareVersion[10] = { 0 };
	std::string extendedVersionData = { 0 };
};

//! @brief Holds UBX NAV Covariance data - NOTE: THIS DATA COMES ACROSS AS LITTLE ENDIAN
struct UBX_NAV_COV
{
	uint32_t gpsTimeOfWeekInMilliSecs = 0;
	uint8_t messageVersion = 0;
	uint8_t positionCovValid = 0;
	uint8_t velocityCovValid = 0;
	uint16_t reserved = 0;
	uint16_t positionCovNN = 0;
	uint16_t positionCovNE = 0;
	uint16_t positionCovND = 0;
	uint16_t positionCovEE = 0;
	uint16_t positionCovED = 0;
	uint16_t positionCovDD = 0;
	uint16_t velocityCovNN = 0;
	uint16_t velocityCovNE = 0;
	uint16_t velocityCovND = 0;
	uint16_t velocityCovEE = 0;
	uint16_t velocityCovED = 0;
	uint16_t velocityCovDD = 0;
};

//! @brief Holds UBX NAV Dilution of precision data
struct UBX_NAV_DOP
{
	uint32_t gpsTimeOfWeekInMilliSecs = 0;
	uint16_t geometricDOP = 0;
	uint16_t positionDOP = 0;
	uint16_t timeDOP = 0;
	uint16_t verticalDOP = 0;
	uint16_t horizontalDOP = 0;
	uint16_t northingDOP = 0;
	uint16_t eastingDOP = 0;
};

//! @brief Holds UBX NAV Satellite data
struct UBX_NAV_SAT
{
	uint32_t gpsTimeOfWeekInMilliSecs = 0;
	uint8_t messageVersion = 0;
	uint8_t numberSatellites = 0;
	uint8_t reserved1 = 0;
	uint8_t reserved2 = 0;
	std::vector<UBX_SAT_DATA> satelliteData;
};

//! @brief Holds UBX NAV Status data
struct UBX_NAV_STATUS
{
	uint32_t gpsTimeOfWeekInMilliSecs = 0;
	uint8_t gpsFix = 0;
	uint8_t flags = 0;
	uint8_t fixStatus = 0;
	uint8_t flasg2 = 0;
	uint8_t timeToFixFixInMs = 0;
	uint8_t milliSecondsSinceStartup = 0;
};

//! @brief Holds UBX NAV Position Velocity Time data
struct UBX_NAV_PVT
{
	uint32_t gpsTimeOfWeekInMilliSecs = 0;
	uint16_t year = 0;
	uint8_t month = 0;
	uint8_t day = 0;
	uint8_t hour = 0;
	uint8_t min = 0;
	uint8_t seconds = 0;
	uint8_t valid = 0;			// VALIDITY FLAGS - SEE PAGE 159 OF UBLOX MANUAL
	uint32_t timeAccuracyEstimate = 0;
	int32_t nanoSeconds = 0;
	uint8_t gnssFixType = 0;
	uint8_t flags = 0;			// FIX STATUS FLAGS - SEE PAGE 159 OF UBLOX MANUAL
	uint8_t flags2 = 0;			// ADDITIONAL FLAGS - SEE PAGE 160 OF UBLOX MANUAL
	uint8_t numSatellitesUsed = 0;
	int32_t longitudeInDeg = 0;
	int32_t latitudeInDeg = 0;
	int32_t heightAboveEllipsoidInMm = 0;
	int32_t heightAboveMeanSeaLevelInMm = 0;
	uint32_t horizontalAccuracyEstimateInMm = 0;
	uint32_t verticalAccuracyEstimateInMm = 0;
	int32_t velocityNorthInMms = 0;
	int32_t velocityEastInMms = 0;
	int32_t velocityDownInMms = 0;
	int32_t groundSpeedInMms = 0;
	int32_t headingOfMotionInDeg = 0;
	uint32_t speedAccuracyEstimateInMms = 0;
	uint32_t headingAccuracyEstimateInDeg = 0;
	uint16_t positionDOP = 0;
	int32_t headingOfVehicleInDeg = 0;
	int16_t magneticDeclinationInDeg = 0;
	uint16_t magneticDeclinationAccuracyInDeg = 0;
};

//! @brief Holds UBX NAV Position ECEF data
struct UBX_NAV_POSECEF
{
	uint32_t gpsTimeOfWeekInMs = 0;
	int32_t ecefXPositonInCm = 0;
	int32_t ecefYPositonInCm = 0;
	int32_t ecefZPositonInCm = 0;
	uint32_t positionAccuraryEstimateInCm = 0;
};

//! @brief Holds UBX NAV Position LLH data
struct UBX_NAV_POSLLH
{
	uint32_t gpsTimeOfWeekInMs = 0;
	int32_t heightAboveEllipsoidInMm = 0;
	int32_t heightAboveMeanSeaLevelInMm = 0;
	uint32_t horizontalAccuracyEstimateInMm = 0;
	uint32_t verticalAccuracyEstimateInMm = 0;
};

//! @brief Holds UBX NAV Velocity ECEF data
struct UBX_NAV_VELECEF
{
	uint32_t gpsTimeOfWeekInMs = 0;
	int32_t ecefXVelocityInCms = 0;
	int32_t ecefYVelocityInCms = 0;
	int32_t ecefZVelocityInCms = 0;
	uint32_t speedAccuraryEstimateInCms = 0;
};

//! @brief Holds UBX NAV Velocity NED data
struct UBX_NAV_VELNED
{
	uint32_t gpsTimeOfWeekInMilliSecs = 0;
	int32_t velocityNorthInCms = 0;
	int32_t velocityEastInCms = 0;
	int32_t velocityDownInCms = 0;
	uint32_t speedInCms = 0;
	uint32_t groundSpeedInCms = 0;
	int32_t headingOfMotionInDeg = 0;
	uint32_t speedAccuracyEstimateInCms = 0;
	uint32_t headingAccuracyEstimateInDeg = 0;
};

//! @brief Holds UBX NAV Time UTC data
struct UBX_NAV_TIMEUTC
{
	uint32_t gpsTimeOfWeekInMilliSecs = 0;
	uint32_t timeAccuraryEstimateInNs = 0;
	int32_t nanoSeconds = 0;
	uint16_t year = 0;
	uint8_t month = 0;
	uint8_t day = 0;
	uint8_t hour = 0;
	uint8_t minute = 0;
	uint8_t seconds = 0;
	uint8_t valid = 0;					// VALIDITY FLAGS - SEE PAGE 183 OF UBLOX MANUAL	
};

/*	
*	NMEA DATA & MESSAGE STRUCTURES
*/

struct NMEA
{
	static const uint8_t classId = 0xF0;

	struct GxGSV;
	struct GxGLL;
	struct GxGSA;
	struct GxGGA;
	struct GxVTG;
	struct GxRMC;
};

struct NMEA::GxGGA
{
	static const uint8_t messageId = 0x00;
};

struct NMEA::GxGLL
{
	static const uint8_t messageId = 0x01;
};

struct NMEA::GxGSA
{
	static const uint8_t messageId = 0x02;
};

struct NMEA::GxGSV
{
	static const uint8_t messageId = 0x03;
};

struct NMEA::GxRMC
{
	static const uint8_t messageId = 0x04;
};

struct NMEA::GxVTG
{
	static const uint8_t messageId = 0x05;
};

//! @brief Holds Satellite data with necessary data elements. 
struct Satellite
{
	// variables
	int numberID = 0;
	std::string signalID;
	std::string type;
	double range = 0;
	double latitudeInDegrees = 0;
	char latitudeDirection = 'X';
	double longitudeInDegrees = 0;
	char longitudeDirection = 'X';
	int positionFixQuality = 0;
	double elevationInDegrees = 0;
	double azimuthInDegrees = 0;
	double signalStrength = 0;
	bool tracking = false;
	double utcTimeOfLastData = 0;

	// members
	bool operator == (const Satellite& s) const					// for finding the satellite in the vector. 
	{
		return numberID == s.numberID;
	}
};

//! @brief Holds the GPS and GNS fix data with necessary data elements.
struct FixData
{
	double utcTime = 0;												// utc time
	double latitudeInDegrees = 0;									// latitude in degreees
	char latitudeDirection = 'X';										// north or south
	double longitudeInDegrees = 0;									// longitude in degrees
	char longitudeDirection = 'X';									// east or west
	int positionFixQuality = 0;										// position fix quality
	int numberSatellitesUsed = 0;									// num of sats used 0-12
	double horizontalDOP = 0;										// HDOP
	double altitudeASLinMeters = 0;									// alt above sea level
	double geoidSeperationInMeters = 0;								// difference between ellipsoid and mean sea level
	double differentialAge = 0;										// age of the correction
	double differentialStationId = 0;								// station id providing correction
};