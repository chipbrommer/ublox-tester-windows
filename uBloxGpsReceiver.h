#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <mutex>
#include <memory>
#include <thread>
#include <functional>
#include "uBloxGpsReceiverInfo.h"
#include "Serial.h"

#if defined(__linux__) 
const char* portName = "/dev/ttyACM0";
#else
const char* portName = "\\\\.\\COM5";
#endif

class uBloxGPS
{

public:
	//! @brief Default Constructor 
	uBloxGPS();

	//! @brief Default Deconstructor
	~uBloxGPS();

	//! @brief Starts the interface on a thread to poll for data. 
	void startInterface();

	//! @brief Validates the checksum of an NMEA message
	//! @param buffer - [in] - char array containing the NMEA message
	//! @return 1 on good checksum, 0 on fail. 
	bool validateNmeaChecksum(char* buffer);

	//! @brief handles a received NMEA message and maps the data based on message type. 
	//! @param buffer - [in] - buffer containing the inbound message
	void handleNmeaMessage(char* buffer);

	//! @brief Parses an NMEA message and splits it into an array at every ","
	//! @param buffer - [in] - Buffer containing the NMEA message  
	//! @param fields - [out] - Char array to store the parsed message cuts into
	//! @param max_fields - [in] - maximum number of fields we can parse into.
	//! @return The number of fields parsed into
	int parseNmeaMessage(char* buffer, char** fields, int max_fields);

	//! @brief Sets the UBX checksum for a UBX message buffer. 
	//! @param buffer - [in] - Message to set the checksum of. 
	void setUbxChecksum(uint8_t* buffer);

	//! @brief Sets the UBX checksum for a UBX message buffer
	//! @param checkSumA - [out] - variable to hold the first checksum
	//! @param checkSumB - [out] - variable to holds the second checksum
	void getUbxChecksums(uint8_t* buffer, int checkSumA, int checkSumB);

	//! @brief Validates the sync bytes of a UBX message
	//! @param syncByte1 - [in] - First sync byte to be validated.
	//! @param syncByte2 - [in] - Seconds sync byte to be validated.
	//! @return 1 for good sync byts, 0 for bad.
	bool validateUbxSyncBytes(uint8_t syncByte1, uint8_t syncByte2);

	//! @brief Validates the checksum of a UBX message
	//! @param buffer - [in] - Buffer containing the UBX message
	//! @param size - [in] - size of the buffer data to calculate checksum of
	//! @return 1 for good checksum, 0 for bad.
	bool validateUbxChecksum(uint8_t* buffer, int size);

	//! @brief Handles a received UBX message and maps the data accordingly
	//! @param buffer - [in] - buffer containing the received UBX message
	void handleUbxMessage(uint8_t* buffer);

	//! @brief Sets a buffer to a data message we want to request
	//! @param buffer - [out] - buffer to store the message in
	//! @param classId - [in] - classId of the message we want to request
	//! @param messageId  - [in] - messageId of the message we want to request
	void requestUbxData(uint8_t* buffer, uint8_t classId, uint8_t messageId);

	//! @brief Enables a desired message data stream from the uBlox GPS without needing to poll for it. 
	//! @param buffer - [out] - buffer to store the message in
	//! @param classId - [in] - classId of the message we want to enable
	//! @param messageId  - [in] - messageId of the message we want to enable
	void enableUbxMessageDataStream(uint8_t* buffer, uint8_t classId, uint8_t messageId);

	//! @brief Disables a desired message data stream from the uBlox GPS to only get the data when polling for it. 
	//! @param buffer - [out] - buffer to store the message in
	//! @param classId - [in] - classId of the message we want to disable
	//! @param messageId  - [in] - messageId of the message we want to disable
	void disableUbxMessageDataStream(uint8_t* buffer, uint8_t classId, uint8_t messageId);

	//! @brief Converts a char array time to a string with formatting: Ex. "03:25:58"
	//! @param time - [in] - Char array to be converted
	//! @return string formatted time. 
	std::string convertUtcTimeToString(char* time);

	//! @brief Converts a char array date to a string with formatting: Ex. "03/25/22"
	//! @param time - [in] - Char array to be converted
	//! @return string formatted date. 
	std::string convertUtcDateToString(char* date);

#pragma region Printers
	//! @brief Prints abuffer in hex
	//! @param buffer - [in] - message to be printed. 
	//! @param size - [in] - size of the buffer used
	void printMessageInHex(uint8_t* buffer, uint8_t size);

	//! @brief Prints an NMEA satellite structs contents to console.
	//! @param sat - [in] - satellite struct to be printed. 
	void printSatelliteData(Satellite sat);

	//! @brief Prints a UBX satellite structs contents to console.
	//! @param sat - [in] - satellite struct to be printed.
	void printSatelliteData(UBX_SAT_DATA sat);

	//! @brief Prints a FixData structs contentsto console.
	//! @param data - [in] - FixData struct to be printed.
	void printFixData(FixData data);

	//! @brief Prints all the received NMEA data points to console. 
	void printAllNmeaData();

	//! @brief Prints the ECEF position data - UBX::NAV::POSECEF
	void printNavPositionEcefData();

	//! @brief Prints the ECEF velocity data - UBX::NAV::VELECEF
	void printNavVelocityEcefData();

	//! @brief Prints all data associated with the UBX::NAV::PVT message
	void printNavPositionVelocityTimeData();

	//! @brief Prints all data associated with the UBX::NAV::SAT message
	void printNavSatelliteData();

	//! @brief Prints the UBLOX F9P Receiver Hardware and software versions
	void printMonitorVersionData();

	//! @brief Prints all data associated with the UBX::NAV::TIMEUTC message
	void printNavTimeUtcData();

	//! @brief Prints all data associated with the UBX::NAV::STATUS message
	void printNavStatusData();

	//! @brief Prints all data associated with the UBX::NAV::VELNED message
	void printNavVelocityNedData();

	//! @brief Prints all data associated with the UBX::NAV::DOP message
	void printNavDopData();

	//! @brief Prints all data associated with the UBX::NAV::POSLLH message
	void printNavPositionLlhData();
#pragma endregion

	//! @brief  Allows us to change the configured messaging data rates as desired. 
	//! GPS Ex. (UBX::CFG::RATE::GPS_SOURCE, UBX::CFG::RATE::MEASURE_HZ10, UBX::CFG::RATE::NAV_CYCLES2).
	//!	This would set GPS satellite measurement data rate at 10hz and satellite data rate at 2 cycles within this 10hz = 5hz.
	//! @param buffer - [out] - buffer to store the message in - 14 element size minimum
	//! @param satelliteSource - [in] - Satellite source to configure rate for. Ex. UBX::CFG::RATE::GPS_SOURCE
	//! @param measureRate - [in] - Desired measurement data rate. Ex. 10hz = UBX::CFG::RATE::MEASURE_HZ10
	//! @param navigationCycles - [in] - Desired navigation cycles rate based on desired measure rate. 
	void setUbxMessageRate(uint8_t* buffer, uint8_t satelliteSource, uint8_t measurmentRate, uint8_t navigationRate);

protected:
	//! @brief entry point for the thread that polls for data
	virtual void interfaceTask();
	std::unique_ptr<std::thread>    m_thread;       //!< the thread that polls for new data to the gps receiver
	std::mutex                      m_mutex;

private:
	unsigned char m_inBuffer[1024];
	unsigned char m_outBuffer[8];
	/*
	*	NMEA DATA POINTS
	*/
	FixData gnsFixData;
	FixData gpsFixData;

	double utcTime = 0;											//!< HHMMSS.ss		
	int utcDate = 0;											//!< DDMMYY
	int localTimeZoneHours = 0;									//!< HH
	int localTimeZoneMinutes = 0;								//!< MM

	char dataStatus = 'X';
	char navStatus = 'X';
	int navigationMode = 0;
	int operationMode = 0;
	char positionModeGps = 'X';
	char positionModeGalileo = 'X';
	char positionModeGlonass = 'X';
	char positionModeBeidou = 'X';

	double latitudeInDegrees = 0;
	char latitudeDirection = 'X';
	double latitudeStandardDeviationInMeters = 0;

	double longitudeInDegrees = 0;
	char longitudeDirection = 'X';
	double longitudeStandardDeviationInMeters = 0;

	double altitude = 0;
	double altitudeStandardDeviatioInMeters = 0;
	double altitudeASLinMeters = 0;
	double geoidSeperationInMeters = 0;

	int positionFixQuality = 0;
	int numberSatellitesUsed = 0;
	double differentialAge = 0;
	double differentialStationId = 0;

	std::string datumReference;
	double latitudeOffset = 0;
	char latitudeOffsetDirection = 'X';
	double longitudeOffset = 0;
	char longitudeOffsetDirection = 'X';
	double altitudeOffset = 0;

	double speedOverGroundInKnots = 0;
	double speedOverGroundInKMH = 0;

	double courseOverGroundInDegrees = 0;
	double magneticCourseOverGroundInDegrees = 0;

	//double positionDOP = 0;
	//double horizontalDOP = 0;
	//double verticalDOP = 0;

	double rangeRMS = 0;
	double majorAxisStandardDeviationInMeters = 0;
	double minorAxisStandardDeviationInMeters = 0;
	double orientationInDegrees = 0;

	double magneticVariationInDegrees = 0;
	char magneticVariationDirection = 'X';

	double totalCumulativeWaterDistanceInNautMiles = 0;
	double waterDistanceSinceResetInNautMiles = 0;
	double totalCumulativeGroundDistanceInNautMiles = 0;
	double groundDistanceSinceResetInNautMiles = 0;

	std::string lastTextTransmissionType;
	std::string lastTextTransmission;

	Satellite activeNavigationSatellites[12];
	std::vector<Satellite> gpsSatellites;
	std::vector<Satellite> galileoSatellites;
	std::vector<Satellite> glonassSatellites;
	std::vector<Satellite> beidouSatellites;


	/*
	*	UBX MESSAGES - DO WE WANT TO STORE HERE INSTEAD ?
	*/
	// UBX_MON_VER monitorVersion;
	// UBX_NAV_DOP dilutionOfPrecisionData;
	// UBX_NAV_SAT satelliteData;
	// UBX_NAV_STATUS navigationStatusData;
	// UBX_NAV_PVT positionVelocityTimeData;
	// UBX_NAV_POSECEF positionEcefData;
	// UBX_NAV_POSLLH positionLlhData;
	// UBX_NAV_VELECEF velocityEcefData;
	// UBX_NAV_VELNED velocityNedData;
	// UBX_NAV_TIMEUTC timeUtcData;


	/*
	*	UBX MESSAGE DATA POINTS
	*/
	char softwareVersion[30] = { 0 };
	char hardwareVersion[10] = { 0 };
	std::string extendedVersionData = { 0 };
	uint32_t gpsTimeOfWeekInMilliSecs = 0;
	uint16_t year = 0;
	uint8_t month = 0;
	uint8_t day = 0;
	uint8_t hour = 0;
	uint8_t minute = 0;
	uint8_t seconds = 0;
	uint8_t valid = 0;
	uint32_t timeAccuraryEstimateInNs = 0;
	int32_t nanoSeconds = 0;
	uint8_t gnssFixType = 0;
	uint8_t gpsFixType = 0;
	uint8_t navStatusFlags = 0;
	uint8_t fixStatusFlags = 0;
	uint8_t navFlags2 = 0;
	uint8_t flags = 0;
	uint8_t flags2 = 0;
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
	int32_t velocityNorthInCms = 0;
	int32_t velocityEastInCms = 0;
	int32_t velocityDownInCms = 0;
	uint32_t speedInCms = 0;
	int32_t groundSpeedInMms = 0;
	uint32_t groundSpeedInCms = 0;
	int32_t headingOfMotionInDeg = 0;
	uint32_t speedAccuracyEstimateInMms = 0;
	uint32_t speedAccuracyEstimateInCms = 0;
	uint32_t headingAccuracyEstimateInDeg = 0;
	uint16_t geometricDOP = 0;
	uint16_t positionDOP = 0;
	uint16_t timeDOP = 0;
	uint16_t verticalDOP = 0;
	uint16_t horizontalDOP = 0;
	uint16_t northingDOP = 0;
	uint16_t eastingDOP = 0;
	int32_t headingOfVehicleInDeg = 0;
	int16_t magneticDeclinationInDeg = 0;
	uint16_t magneticDeclinationAccuracyInDeg = 0;
	int32_t ecefXVelocityInCms = 0;
	int32_t ecefYVelocityInCms = 0;
	int32_t ecefZVelocityInCms = 0;
	int32_t ecefXPositonInCm = 0;
	int32_t ecefYPositonInCm = 0;
	int32_t ecefZPositonInCm = 0;
	uint32_t positionAccuraryEstimateInCm = 0;
	uint8_t numberSatellites = 0;
	uint32_t timeToFirstFixInMilliseconds = 0;
	uint32_t milliSecondsSinceStartup = 0;
	std::vector<UBX_SAT_DATA> satellites;

	/*
	*	UBX MESSAGE FLAGS
	*/
	bool validNavPvtData = false;
	bool navPvtFullyResolved = false;
	bool navPvtValidTime = false;
	bool navPvtValidDate = false;

	bool gnssFixOk = false;
	bool differentialCorrectionsApplied = false;
	int powerSaveModeState = 0;
	bool vehicleHeadingValid = false;
	int carrierPhaseRangeSolutionStatus = 0;

	bool confirmedTime = false;
	bool confirmedDate = false;
	bool confirmedDateTimeValidity = false;

	/*
	*	FUNCTION PROTOTYPES
	*/
	//! @brief Converts a single hex char to an integer.
	//! @param c - [in] - the character to be converted
	//! @return the int value of the converted char
	int hexCharToInt(char c);

	//! @brief Converts a char array of hex to integers
	//! @param c - [in] - The array to be converted  
	//! @return the int value of the array. 
	int hexToInt(char* c);

	//! @brief Gets the Signal ID of an NMEA message
	//! @param buffer - [in] - the buffer containing the NMEA message 
	//! @return int containing the signal id, or -1 on empty buffer or if * not found.
	int getNmeaMsgSignalId(std::string buffer);

	//! @brief Sets the last text transmission variable
	//! @param msgNum - [in] - number of message in the sequence
	//! @param text - [in] - string to store the data in. 
	void setLastTextTransmission(int msgNum, std::string text);

	//! @brief Will update or add satellite data to the list of satellites per type
	//! @param gnssId - [in] - id of the type of satellite
	//! @param id - [in] - id of the satellite itself
	//! @param elevation - [in] - elevation in degrees
	//! @param azimuth - [in] - azimuth in degrees
	//! @param signalStrength - [in] - signal strength of the satellite connection
	void updateSatelliteData(int gnssId, char* id, char* elevation, char* azimuth, char* signalStrength);

	//! @brief Updates the data within the satellite list for NMEA messages. If sat.id is not found it adds new sat to list, else it updates the values.
	//! @param sat - [in] - Satellite instance to add or update
	//! @param satList - [in] - Satellite list for the sat instance
	void updateSatelliteVector(const Satellite sat, std::vector<Satellite>& satList);

	//! @brief Updates the data within the satellite list for UBX messages. If sat.satelliteId is not found it adds new sat to list, else it updates the values.
	//! @param sat - [in] - Satellite instance to add or update
	//! @param satList - [in] - Satellite list for the sat instance
	void updateSatelliteVector(const UBX_SAT_DATA sat, std::vector<UBX_SAT_DATA>& satList);

	//! @brief Updates the list of Navigation Satellites, id's
	//! @param id - [in] - ID of the satellite. 
	//! @param satelliteNumber - [in] - array element (satellite number) to set the ID for.
	void updateIdOfActiveNavigationSatellites(int id, int satelliteNumber);

	//! @brief Updates the list of Navigation Satellites, ranges
	//! @param range - [in] - range of the satellite
	//! @param satelliteNumber - [in] - array element (satellite number) to set the range for.
	void updateRangeOfActiveNavigationSatellites(double range, int satelliteNumber);

	//! @brief Converts the status indicator into its readable type Ex: 'A' = "Data Valid"
	//! @param status - [in] - indicator to be decoded
	//! @return string of the decoded indicator
	std::string decodeDataStatus(char* status);

	//! @brief Decode the posMode data into a string 
	//! @param posMode - [in] - data code for the position mode
	//! @return - string containing the decoded value/meaning
	std::string decodePosMode(char* posMode);

	//! @brief Decode the navMode data into a string 
	//! @param navMode - [in] - data code for the navigation mode
	//! @return - string containing the decoded value/meaning
	std::string decodeNavMode(char* navMode);

	//! @brief Decode the txt data type into a string 
	//! @param msgType - [in] - data code for the text message type
	//! @return - string containing the decoded value/meaning
	std::string decodeTxtMsgType(char* msgType);

	//! @brief Decode the quality data type into a string
	//! @param quality - [in] - data code for the gps fix quality 
	//! @return - string containing the decoded value/meaning
	std::string decodeQuality(char* quality);

	//! @brief Decode the operation mode into a sting
	//! @param opMode - [in] - data code for the gps operation mode
	//! @return - string containing the decoded value/meaning
	std::string decodeOpMode(char* opMode);

	//! @brief Decode the GNSS ID into a string for the type of satellite.
	//! @param gnssId - [in] - data code of the satellite to be decoded. 
	//! @return - string containing the type of satellite.
	std::string decodeGnssId(char* gnssId);

	//! @brief Decode the satellite signal id based on the gnss id
	//! @param gnssId - [in] - int value of the gnss id of the satellite
	//! @param signalId - [in] - the signal id of the satellite to be decoded.
	//! @return - string containing the decoded value/meaning
	std::string decodeSignalId(int gnssId, char* signalId);

	//! @brief Parses a UBX message X4 BITMASK flags variable to decode the bits appropriately 
	//! @brief found in UBX::NAV::SAT message as "flags" or "Bitmask"
	//! @param flags - [in] - Bitmask flag variable to decode 
	void parseBitmaskFlags(uint32_t* flags);

	//! @brief Parses a UBX message X1 VALIDITY flags variable to decode the bits appropriately
	//! @brief found in UBX::NAV::PVT message as "valid"
	//! @param flags - [in] - Validity flag variable to decode 
	void parseValidityFlags(uint8_t* flags);

	//! @brief Parses a UBX message X1 VALIDITY flags variable to decode the bits appropriately 
	//! @brief found in UBX::NAV::PVT message as "flags" or "fix"
	//! @param flags - [in] - Fix flag variable to decode 
	void parseFixStatusFlags(uint8_t* flags);

	//! @brief Parses a UBX message X1 VALIDITY flags variable to decode the bits appropriately 
	//! @brief found in UBX::NAV::PVT message as "flags2" or "Additional flags"
	//! @param flags - [in] - flag variable to decode 
	void parseAdditionalFlags(uint8_t* flags);

	//! @brief Parses a UBX::MON::VER message to the appropriate variables.
	//! @param buffer - [in] - buffer to be parsed. 
	void parseMonitorVersionData(uint8_t* buffer);

	//! @brief Parses a UBX::NAV::SAT message to the appropriate variables.
	//! @param buffer - [in] - buffer to be parsed. 
	void parseNavSatelliteData(uint8_t* buffer);

	//! @brief Parses a UBX::NAV::PVT message to the appropriate variables.
	//! @param buffer - [in] - buffer to be parsed. 
	void parseNavPositionVelocityTimeData(uint8_t* buffer);

	//! @brief Parses a UBX::NAV::STATUS message to the appropriate variables.
	//! @param buffer - [in] - buffer to be parsed. 
	void parseNavStatusData(uint8_t* buffer);

	//! @brief Parses a UBX::NAV::POSECEF message to the appropriate variables.
	//! @param buffer - [in] - buffer to be parsed. 
	void parseNavPositionEcefData(uint8_t* buffer);

	//! @brief Parses a UBX::NAV::POSLLH message to the appropriate variables.
	//! @param buffer - [in] - buffer to be parsed. 
	void parseNavPositionLlhData(uint8_t* buffer);

	//! @brief Parses a UBX::NAV::VELECEF message to the appropriate variables.
	//! @param buffer - [in] - buffer to be parsed.  
	void parseNavVelocityEcefData(uint8_t* buffer);

	//! @brief Parses a UBX::NAV::VELNED message to the appropriate variables.
	//! @param buffer - [in] - buffer to be parsed. 
	void parseNavVelocityNedData(uint8_t* buffer);

	//! @brief Parses a UBX::NAV::TIMEUTC message to the appropriate variables.
	//! @param buffer - [in] - buffer to be parsed. 
	void parseNavTimeUtcData(uint8_t* buffer);

	//! @brief Parses a UBX::NAV::DOP message to the appropriate variables.
	//! @param buffer - [in] - buffer to be parsed. 
	void parseNavDopData(uint8_t* buffer);

	//! @brief Gets a bit of a unsigned char / uint8_t at a specified element location
	//! @param data - [in] - int to get a bit from
	//! @param element - [in] - element of the bit we want to check
	//! @return 0 or 1 based on the bit at specified location
	bool getBit(uint8_t data, int element);

	//! @brief Gets a bit of a unsigned shprt / uint16_t at a specified element location
	//! @param data - [in] - int to get a bit from
	//! @param element - [in] - element of the bit we want to check
	//! @return 0 or 1 based on the bit at specified location
	bool getBit(uint16_t data, int element);

	//! @brief Gets a bit of a unsigned long / uint32_t at a specified element location
	//! @param data - [in] - int to get a bit from
	//! @param element - [in] - element of the bit we want to check
	//! @return 0 or 1 based on the bit at specified location
	bool getBit(uint32_t data, int element);
};