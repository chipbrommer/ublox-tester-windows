#include "uBloxGpsReceiver.h"

uBloxGPS::uBloxGPS()
{

}

uBloxGPS::~uBloxGPS()
{

}

void uBloxGPS::startInterface()
{
	m_thread.reset(new std::thread(std::bind(&uBloxGPS::interfaceTask, this)));
}

void uBloxGPS::interfaceTask()
{
	int inBytes = 0;

	// Connect
	SerialPort m_serial("\\\\.\\COM5");

	// SEND UBX::MON::VER REQUEST TO VERIFY CONNECTION ON GPS UNIT
	requestUbxData(m_outBuffer, UBX::MON::classId, UBX::MON::VER::messageId);
	m_serial.writeSerialPort(m_outBuffer, sizeof(m_outBuffer));

	while (m_serial.isConnected())
	{
		inBytes = m_serial.readSerialPort(m_inBuffer, sizeof(m_inBuffer));

		// POLL FOR DATA
		//	 ^  OR DO WE WANT TO ENABLE DATA STREAM ? 

		// PARSE DATA 
	}
}

int uBloxGPS::hexCharToInt(char c)
{
	if (c >= '0' && c <= '9')
	{
		return c - '0';
	}
	if (c >= 'A' && c <= 'F')
	{
		return c - 'A' + 10;
	}
	if (c >= 'a' && c <= 'f')
	{
		return c - 'a' + 10;
	}
	return -1;
}

int uBloxGPS::hexToInt(char* c)
{
	// int for return
	int value;

	// append the char array to the int
	value = hexCharToInt(c[0]);
	value = value << 4;
	value += hexCharToInt(c[1]);

	// return int
	return value;
}

bool uBloxGPS::validateNmeaChecksum(char* buffer)
{
	char* checksum_str;						// char array to hold the check sum
	int checksum;							// holds the int version of the checksum
	unsigned char calculated_checksum = 0;	// holds the calculated checksum

	// Checksum is postcede by *
	checksum_str = strchr(buffer, '*');
	if (checksum_str != NULL)
	{
		// Remove checksum from string
		*checksum_str = '\0';
		// Calculate checksum, starting after $ (i = 1)
		for (int i = 1; i < strlen(buffer); i++)
		{
			calculated_checksum = calculated_checksum ^ buffer[i];
		}
		checksum = hexToInt((char*)checksum_str + 1);
		//printf("Checksum Str [%s], Checksum %02X, Calculated Checksum %02X \n",(char *)checksum_str+1, checksum, calculated_checksum);
		// if cheksum is good, return success
		if (checksum == calculated_checksum)
		{
			return 1;
		}
	}
	// else checksum is bad, missing, or the nmea message is null
	return 0;
}

void uBloxGPS::handleNmeaMessage(char* buffer)
{
	int maxSatellitesUsed = 12;
	int numberGASatellitesInView = 0;
	int numberGASatellitesPrinted = 0;
	int numberGBSatellitesInView = 0;
	int numberGBSatellitesPrinted = 0;
	int numberGLSatellitesInView = 0;
	int numberGLSatellitesPrinted = 0;
	int numberGPSatellitesInView = 0;
	int numberGPSatellitesPrinted = 0;
	int numberGNSatellitesInView = 0;
	int numberGNSatellitesPrinted = 0;
	int GnssId = 0;
	int i;
	char* field[30];

	// validate check sum
	if (validateNmeaChecksum(buffer) == 0)
	{
		return;
	}

	// get the signal ID first before buffer gets split
	int signal = getNmeaMsgSignalId(buffer);
	// GPS or GNSS Messages
	if ((strncmp(buffer, "$GP", 3) == 0) | (strncmp(buffer, "$GN", 3) == 0))
	{
		// Set the GNSS ID
		GnssId = 1;

		// DATUM REFERENCE MESSAGE 
		// Gives  the difference between the current datum and the reference datum.
		if (strncmp(&buffer[3], "DTM", 3) == 0)
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// store
			latitudeOffset = atof(field[3]);									// latitudeInDeg offset
			field[4][0] == NULL ? latitudeOffsetDirection = '0' : latitudeOffsetDirection = field[4][0];							// North or south
			longitudeOffset = atof(field[5]);									// longitudeInDeg offset
			field[6][0] == NULL ? longitudeOffsetDirection = '0' : longitudeOffsetDirection = field[6][0];
			altitudeOffset = atof(field[7]);									// altitude offset
			datumReference = field[8];										// datum reference code
		}
		// GPS FIX DATA MESSAGES
		else if (strncmp(&buffer[3], "GGA", 3) == 0)
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// store
			gpsFixData.utcTime = atof(field[1]);								// utc time
			gpsFixData.latitudeInDegrees = atof(field[2]);					// latitudeInDeg in degreees
			field[3][0] == NULL ? gpsFixData.latitudeDirection = '0' : gpsFixData.latitudeDirection = field[3][0];
			gpsFixData.longitudeInDegrees = atof(field[4]);					// longitudeInDeg in degrees
			field[5][0] == NULL ? gpsFixData.longitudeDirection = '0' : gpsFixData.longitudeDirection = field[5][0];
			gpsFixData.positionFixQuality = atoi(field[6]);					// position fix quality
			gpsFixData.numberSatellitesUsed = atoi(field[7]);					// num of sats used 0-12
			gpsFixData.horizontalDOP = atof(field[8]);						// HDOP
			gpsFixData.altitudeASLinMeters = atof(field[9]);					// alt above sea level
			gpsFixData.geoidSeperationInMeters = atof(field[11]);				// difference between ellipsoid and mean sea level
			gpsFixData.differentialAge = atof(field[13]);						// age of the correction
			gpsFixData.differentialStationId = atof(field[14]);				// station id providing correction
		}
		// latitudeInDeg AND longitudeInDeg W/ TIME OR POSITION FIX AND STATUS MESSAGE
		else if (strncmp(&buffer[3], "GLL", 3) == 0)
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// store
			latitudeInDegrees = atof(field[1]);								// latitudeInDeg in degreees
			field[2][0] == NULL ? latitudeDirection = '0' : latitudeDirection = field[2][0];
			longitudeInDegrees = atof(field[3]);								// longitudeInDeg in degrees
			field[4][0] == NULL ? longitudeOffsetDirection = '0' : longitudeOffsetDirection = field[4][0];
			utcTime = atof(field[5]);											// utc time
			field[6][0] == NULL ? dataStatus = '0' : dataStatus = field[6][0];
			field[7][0] == NULL ? positionModeGps = '0' : positionModeGps = field[7][0];
		}
		// GNS FIX DATA MESSAGE
		else if (strncmp(&buffer[3], "GNS", 3) == 0)
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// print	
			gnsFixData.utcTime = atof(field[1]);								// utc time
			gnsFixData.latitudeInDegrees = atof(field[2]);					// latitudeInDeg in degreees
			field[3][0] == NULL ? gnsFixData.latitudeDirection = '0' : gnsFixData.latitudeDirection = field[3][0];
			gnsFixData.longitudeInDegrees = atof(field[4]);					// longitudeInDeg in degrees
			field[5][0] == NULL ? gnsFixData.longitudeDirection = '0' : gnsFixData.longitudeDirection = field[5][0];
			gnsFixData.numberSatellitesUsed = atoi(field[7]);					// num of sats used 0-12
			gnsFixData.horizontalDOP = atof(field[8]);						// HDOP
			gnsFixData.altitudeASLinMeters = atof(field[9]);					// alt above sea level
			gnsFixData.geoidSeperationInMeters = atof(field[10]);				// difference between ellipsoid and mean sea level
			gnsFixData.differentialAge = atof(field[11]);						// age of the correction
			gnsFixData.differentialStationId = atof(field[12]);				// station id providing correction
			field[13][0] == NULL ? navStatus = '0' : navStatus = field[13][0];

			field[6][0] == NULL ? positionModeGps = '0' : positionModeGps = field[6][0];
			field[6][1] == NULL ? positionModeGalileo = '0' : positionModeGalileo = field[6][1];
			field[6][2] == NULL ? positionModeGlonass = '0' : positionModeGlonass = field[6][2];
			field[6][3] == NULL ? positionModeBeidou = '0' : positionModeBeidou = field[6][3];
		}
		// GNSS RANGE RESIDUALS
		else if (strncmp(&buffer[3], "GRS", 3) == 0)
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// print
			utcTime = atof(field[1]);											// utc time
			// print range for each satellite used
			for (int i = 0; i < maxSatellitesUsed; i++)
			{
				updateRangeOfActiveNavigationSatellites(atof(field[3 + i]), i);
			}
		}
		// GNSS DOP + ACTIVE SATELLITES
		else if (strncmp(&buffer[3], "GSA", 3) == 0)
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// print
			// print range for each satellite used
			for (int i = 0; i < maxSatellitesUsed; i++)
			{
				updateIdOfActiveNavigationSatellites(atoi(field[3 + i]), i);

			}
			positionDOP = atof(field[8]);										// Position dilution of precision
			horizontalDOP = atof(field[8]);									// Horizontal dilution of precision
			verticalDOP = atof(field[8]);										// Vertical dilution of precision
		}
		// GNSS PSEUDO RANGE ERROR STATS
		else if (strncmp(&buffer[3], "GST", 3) == 0)
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// store data
			utcTime = atof(field[1]);											// utc time
			rangeRMS = atof(field[2]);
			majorAxisStandardDeviationInMeters = atof(field[3]);
			minorAxisStandardDeviationInMeters = atof(field[4]);
			orientationInDegrees = atof(field[5]);
			latitudeStandardDeviationInMeters = atof(field[6]);
			longitudeStandardDeviationInMeters = atof(field[7]);
			altitudeStandardDeviatioInMeters = atof(field[8]);
		}
		// GNSS SATELLITES IN VIEW
		else if (strncmp(&buffer[3], "GSV", 3) == 0)
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// print	

			// if first message in series, reset print variable and set num sats in view
			if (atoi(field[2]) == 1)
			{
				numberGPSatellitesInView = atoi(field[3]);
				numberGPSatellitesPrinted = 0;
			}

			// Set the number of satellites in this message
			int numberSatellitesThisMessage = numberGPSatellitesInView - numberGPSatellitesPrinted;
			// max is 4 - if more than 4 cap it at 4, else num is good.  
			numberSatellitesThisMessage > 4 ? numberSatellitesThisMessage = 4 : numberSatellitesThisMessage = numberSatellitesThisMessage;

			int numberSatsPrintedThisMessage = 0;

			// Print data for satellites in view in this message
			for (; numberSatsPrintedThisMessage < numberSatellitesThisMessage; numberSatsPrintedThisMessage++)
			{
				//printf("Satellite Number     :%d \n", numberGPSatellitesPrinted + 1);
				updateSatelliteData(GnssId, field[4 + (4 * numberSatsPrintedThisMessage)], field[5 + (4 * numberSatsPrintedThisMessage)], field[6 + (4 * numberSatsPrintedThisMessage)], field[7 + (4 * numberSatsPrintedThisMessage)]);
				numberGPSatellitesPrinted++;
			}
		}

		// RECOMMENDED MINIMUM DATA
		else if (strncmp(&buffer[3], "RMC", 3) == 0)
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// store data
			utcTime = atof(field[1]);
			field[2][0] == NULL ? dataStatus = '0' : dataStatus = field[2][0];
			latitudeInDegrees = atof(field[3]);
			field[4][0] == NULL ? latitudeDirection = '0' : latitudeDirection = field[4][0];
			longitudeInDegrees = atof(field[5]);
			field[6][0] == NULL ? longitudeDirection = '0' : longitudeDirection = field[6][0];
			speedOverGroundInKnots = atof(field[7]);
			courseOverGroundInDegrees = atof(field[8]);
			utcDate = atoi(field[9]);
			magneticVariationInDegrees = atof(field[10]);
			field[11][0] == NULL ? magneticVariationDirection = '0' : magneticVariationDirection = field[11][0];
			field[12][0] == NULL ? positionModeGps = '0' : positionModeGps = field[12][0];
			field[13][0] == NULL ? navStatus = '0' : navStatus = field[13][0];
		}
		// TEXT TRANSMISSION
		else if (strncmp(&buffer[3], "TXT", 3) == 0)
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// store
			lastTextTransmissionType = decodeTxtMsgType(field[3]);
			setLastTextTransmission(atoi(field[2]), field[4]);
		}
		// DUAL GROUND/WATER DISTANCE
		else if (strncmp(&buffer[3], "VLW", 3) == 0)
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// store
			totalCumulativeWaterDistanceInNautMiles = atof(field[1]);
			waterDistanceSinceResetInNautMiles = atof(field[3]);
			totalCumulativeGroundDistanceInNautMiles = atof(field[5]);
			groundDistanceSinceResetInNautMiles = atof(field[7]);
		}
		// COURSE OVER GROUND + GROUND SPEED
		else if (strncmp(&buffer[3], "VTG", 3) == 0)
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// print
			courseOverGroundInDegrees = atof(field[1]);
			magneticCourseOverGroundInDegrees = atof(field[3]);
			speedOverGroundInKnots = atof(field[5]);
			speedOverGroundInKMH = atof(field[7]);
			field[9][0] == NULL ? positionModeGps = '0' : positionModeGps = field[9][0];
		}
		// TIME AND DATE 
		else if (strncmp(&buffer[3], "ZDA", 3) == 0)
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// store
			utcTime = atof(field[1]);
			utcDate = atoi(field[2]);
			localTimeZoneHours = atoi(field[5]);
			localTimeZoneMinutes = atoi(field[6]);
		}
		else
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// print
			printf("MESSAGE NOT HANDLED: %s \n", field[0]);									// message id 
		}
	}
	// GLONASS Messages
	else if (strncmp(buffer, "$GL", 3) == 0)
	{
		GnssId = 2;

		// GL POLLING MESSAGE OUTBOUND ONLY
		if (strncmp(&buffer[3], "GLQ", 3) == 0)
		{
			//break;
		}
		// GNSS SATELLITES IN VIEW
		else if (strncmp(&buffer[3], "GSV", 3) == 0)
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// print	

			// if first message in series, reset print variable and set num sats in view
			if (atoi(field[2]) == 1)
			{
				numberGPSatellitesInView = atoi(field[3]);
				numberGPSatellitesPrinted = 0;
			}

			// Set the number of satellites in this message
			int numberSatellitesThisMessage = numberGPSatellitesInView - numberGPSatellitesPrinted;
			// max is 4 - if more than 4 cap it at 4, else num is good.  
			numberSatellitesThisMessage > 4 ? numberSatellitesThisMessage = 4 : numberSatellitesThisMessage = numberSatellitesThisMessage;

			int numberSatsPrintedThisMessage = 0;

			// Print data for satellites in view in this message
			for (; numberSatsPrintedThisMessage < numberSatellitesThisMessage; numberSatsPrintedThisMessage++)
			{
				printf("Satellite Number     :%d \n", numberGPSatellitesPrinted + 1);
				updateSatelliteData(GnssId, field[4 + (4 * numberSatsPrintedThisMessage)], field[5 + (4 * numberSatsPrintedThisMessage)], field[6 + (4 * numberSatsPrintedThisMessage)], field[7 + (4 * numberSatsPrintedThisMessage)]);
				numberGPSatellitesPrinted++;
			}
		}
		else
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// print
			printf("MESSAGE NOT HANDLED: %s \n", field[0]);									// message id 
		}
	}
	// GALILEO Messages
	else if (strncmp(buffer, "$GA", 3) == 0)
	{
		GnssId = 3;

		// GA POLLING MESSAGE OUTBOUND ONLY
		if (strncmp(&buffer[3], "GAQ", 3) == 0)
		{
			//break;
		}
		// GNSS SATELLITES IN VIEW
		else if (strncmp(&buffer[3], "GSV", 3) == 0)
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// print	

			// if first message in series, reset print variable and set num sats in view
			if (atoi(field[2]) == 1)
			{
				numberGPSatellitesInView = atoi(field[3]);
				numberGPSatellitesPrinted = 0;
			}

			// Set the number of satellites in this message
			int numberSatellitesThisMessage = numberGPSatellitesInView - numberGPSatellitesPrinted;
			// max is 4 - if more than 4 cap it at 4, else num is good.  
			numberSatellitesThisMessage > 4 ? numberSatellitesThisMessage = 4 : numberSatellitesThisMessage = numberSatellitesThisMessage;

			int numberSatsPrintedThisMessage = 0;

			// Print data for satellites in view in this message
			for (; numberSatsPrintedThisMessage < numberSatellitesThisMessage; numberSatsPrintedThisMessage++)
			{
				printf("Satellite Number     :%d \n", numberGPSatellitesPrinted + 1);
				updateSatelliteData(GnssId, field[4 + (4 * numberSatsPrintedThisMessage)], field[5 + (4 * numberSatsPrintedThisMessage)], field[6 + (4 * numberSatsPrintedThisMessage)], field[7 + (4 * numberSatsPrintedThisMessage)]);
				numberGPSatellitesPrinted++;
			}
		}
		else
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// print
			printf("MESSAGE NOT HANDLED: %s \n", field[0]);									// message id 
		}
	}
	// BEIDOU Messages
	else if (strncmp(buffer, "$GB", 3) == 0)
	{
		GnssId = 4;

		// GB POLLING MESSAGE OUTBOUND ONLY
		if (strncmp(&buffer[3], "GBQ", 3) == 0)
		{
			//break;
		}
		// GNSS SATELLITES IN VIEW
		else if (strncmp(&buffer[3], "GSV", 3) == 0)
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// print	

			// if first message in series, reset print variable and set num sats in view
			if (atoi(field[2]) == 1)
			{
				numberGPSatellitesInView = atoi(field[3]);
				numberGPSatellitesPrinted = 0;
			}

			// Set the number of satellites in this message
			int numberSatellitesThisMessage = numberGPSatellitesInView - numberGPSatellitesPrinted;
			// max is 4 - if more than 4 cap it at 4, else num is good.  
			numberSatellitesThisMessage > 4 ? numberSatellitesThisMessage = 4 : numberSatellitesThisMessage = numberSatellitesThisMessage;

			int numberSatsPrintedThisMessage = 0;

			// Print data for satellites in view in this message
			for (; numberSatsPrintedThisMessage < numberSatellitesThisMessage; numberSatsPrintedThisMessage++)
			{
				printf("Satellite Number     :%d \n", numberGPSatellitesPrinted + 1);
				updateSatelliteData(GnssId, field[4 + (4 * numberSatsPrintedThisMessage)], field[5 + (4 * numberSatsPrintedThisMessage)], field[6 + (4 * numberSatsPrintedThisMessage)], field[7 + (4 * numberSatsPrintedThisMessage)]);
				numberGPSatellitesPrinted++;
			}
		}
		else
		{
			// parse
			i = parseNmeaMessage(buffer, field, 30);
			// print
			printf("MESSAGE NOT HANDLED: %s \n", field[0]);									// message id 
		}
	}
}

void uBloxGPS::setUbxChecksum(uint8_t* buffer)
{
	// variables for calculating checksum
	unsigned i, j;
	uint8_t a = 0, b = 0;

	// find the data range.
	j = (buffer[4] + (buffer[5] << 8) + 6);

	// for each element inside the data range, store the size in hex. 
	for (i = 2; i < j; i++)
	{
		a += buffer[i];
		b += a;
	}

	// set the checksum bytes
	buffer[i + 0] = a;
	buffer[i + 1] = b;
}

bool uBloxGPS::validateUbxSyncBytes(uint8_t syncByte1, uint8_t syncByte2)
{
	// verify sync byte 1 and 2 match the UBX protocol.
	if ((syncByte1 == UBX::Header::SyncChar1) && (syncByte2 == UBX::Header::SyncChar2))
	{
		// bytes match, return good.
		return 1;
	}

	// else return bad - 0
	return 0;
}

bool uBloxGPS::validateUbxChecksum(uint8_t* buffer, int size)
{
	uint8_t CK_a = buffer[size - 2];
	uint8_t CK_b = buffer[size - 1];

	// variables for calculating checksum
	unsigned i, j;
	uint8_t a = 0, b = 0;

	// find the data range.
	j = (buffer[4] + (buffer[5] << 8) + 6);

	// for each element inside the data range, store the size in hex. 
	for (i = 2; i < j; i++)
	{
		a += buffer[i];
		b += a;
	}

	// verify the checksum bytes
	if (CK_a == a && CK_b == b)
	{
		return 1;
	}

	return 0;
}

void uBloxGPS::handleUbxMessage(uint8_t* buffer)
{
	// grab the msg id and class id 
	uint8_t classId = buffer[2];
	uint8_t msgId = buffer[3];

	// get the data length from the buffer at index 4
	uint16_t payloadSize;
	memcpy(&payloadSize, buffer + 4, sizeof(payloadSize));

	// verify the class id
	if (classId == UBX::ACK::classId)
	{
		// Check for ACK message
		if (msgId == UBX::ACK::ACK_::messageId)
		{
			// TODO
		}
		// Check for NACK massage
		else if (msgId == UBX::ACK::NACK::messageId)
		{
			// TODO
		}
	}
	else if (classId == UBX::MON::classId)
	{
		// Check for MON::VER data
		if (msgId == UBX::MON::VER::messageId)
		{
			parseMonitorVersionData(buffer);
		}
	}
	else if (classId == UBX::NAV::classId)
	{
		// Check for NAV::SAT data
		if (msgId == UBX::NAV::SAT::messageId)
		{
			parseNavSatelliteData(buffer);
		}
		// check for NAV::POSECEF data
		else if (msgId == UBX::NAV::POSECEF::messageId && payloadSize == UBX::NAV::POSECEF::payloadLength)
		{
			parseNavPositionEcefData(buffer);
		}
		// check for NAV::VELECEF data
		else if (msgId == UBX::NAV::VELECEF::messageId && payloadSize == UBX::NAV::VELECEF::payloadLength)
		{
			parseNavVelocityEcefData(buffer);
		}
		// check for NAV::PVT data
		else if (msgId == UBX::NAV::PVT::messageId && payloadSize == UBX::NAV::PVT::payloadLength)
		{
			parseNavPositionVelocityTimeData(buffer);
		}
		// check for NAV::TIMEUTC data
		else if (msgId == UBX::NAV::TIMEUTC::messageId && payloadSize == UBX::NAV::TIMEUTC::payloadLength)
		{
			parseNavTimeUtcData(buffer);
		}
		// check for NAV::STATUS data
		else if (msgId == UBX::NAV::STATUS::messageId && payloadSize == UBX::NAV::STATUS::payloadLength)
		{
			parseNavStatusData(buffer);
		}
		// check for NAV::VELNED data
		else if (msgId == UBX::NAV::VELNED::messageId && payloadSize == UBX::NAV::VELNED::payloadLength)
		{
			parseNavVelocityNedData(buffer);
		}
		// check for NAV::DOP data
		else if (msgId == UBX::NAV::DOP::messageId && payloadSize == UBX::NAV::DOP::payloadLength)
		{
			parseNavDopData(buffer);
		}
		// check for NAV::PVT data
		else if (msgId == UBX::NAV::POSLLH::messageId && payloadSize == UBX::NAV::POSLLH::payloadLength)
		{
			parseNavPositionLlhData(buffer);
		}
	}
}

void uBloxGPS::requestUbxData(uint8_t* buffer, uint8_t classId, uint8_t messageId)
{
	// set sync bytes
	buffer[0] = UBX::Header::SyncChar1;
	buffer[1] = UBX::Header::SyncChar2;

	// set class id byte
	buffer[2] = classId;

	// set msg id byte
	buffer[3] = messageId;

	// set empty msg length
	buffer[4] = 0x00;

	// set empty data
	buffer[5] = 0x00;

	// set check sum bytes - will be over riden by the setUbxCheckSum function to correctly calculate the checksum.
	buffer[6] = 0x00;
	buffer[7] = 0x00;

	// set check sum bytes
	setUbxChecksum(buffer);
}

void uBloxGPS::enableUbxMessageDataStream(uint8_t* buffer, uint8_t classId, uint8_t messageId)
{
	// set sync bytes
	buffer[0] = UBX::Header::SyncChar1;
	buffer[1] = UBX::Header::SyncChar2;

	// set class id byte
	buffer[2] = UBX::CFG::classId ;

	// set msg id byte
	buffer[3] = UBX::CFG::VALSET::messageId;

	// TODO - ADD DATA STRUCTURE

	// set check sum bytes - will be over riden by the setUbxCheckSum function to correctly calculate the checksum.
	buffer[6] = 0x00;
	buffer[7] = 0x00;

	// set check sum bytes
	setUbxChecksum(buffer);
}

void uBloxGPS::disableUbxMessageDataStream(uint8_t* buffer, uint8_t classId, uint8_t messageId)
{
	// set sync bytes
	buffer[0] = UBX::Header::SyncChar1;
	buffer[1] = UBX::Header::SyncChar2;

	// set class id byte
	buffer[2] = UBX::CFG::classId;

	// set msg id byte
	buffer[3] = UBX::CFG::VALSET::messageId;

	// TODO - ADD DATA STRUCTURE

	// set check sum bytes - will be over riden by the setUbxCheckSum function to correctly calculate the checksum.
	buffer[6] = 0x00;
	buffer[7] = 0x00;

	// set check sum bytes
	setUbxChecksum(buffer);
}

void uBloxGPS::setUbxMessageRate(uint8_t* buffer, uint8_t satelliteSource, uint8_t measurmentRate, uint8_t navigationRate)
{
	// set sync bytes
	buffer[0] = UBX::Header::SyncChar1;
	buffer[1] = UBX::Header::SyncChar2;

	// set class id byte
	buffer[2] = UBX::CFG::classId;

	// set message id byte
	buffer[3] = UBX::CFG::RATE::messageId;

	// THESE ARE EXPECTED TO BE RECEIVED AS UINT16_T SO WE WILL FINESSE
		// set message length
		buffer[4] = 0x06;
		buffer[5] = 0x00;

		// set desired measurement rate
		buffer[6] = measurmentRate;
		// set second byte based on size of first byte
		measurmentRate == UBX::CFG::RATE::MEASURE_HZ1 ? buffer[7] = 0x03 : buffer[7] = 0x00;

		// desired navigation rate
		buffer[8] = navigationRate;
		buffer[9] = 0x00;

		// desired satellite source
		buffer[10] = satelliteSource;
		buffer[11] = 0x00;

	// set check sum bytes - will be over riden by the setUbxCheckSum function to correctly calculate the checksum.
	buffer[12] = 0x00;
	buffer[13] = 0x00;

	// set check sum bytes
	setUbxChecksum(buffer);
}

void uBloxGPS::setLastTextTransmission(int msgNum, std::string text)
{
	// if first message, clear the transmission variable, and set the text
	if (msgNum == 1)
	{
		lastTextTransmission.clear();											// clear as precaution
		lastTextTransmission = text;											// set text
	}
	// else we want to append to current text. 
	else
	{
		// append text to the end of 
		lastTextTransmission.append(" ");										// spacer
		lastTextTransmission.append(text);										// append msg
	}
}

void uBloxGPS::updateSatelliteData(int gnssId, char* id, char* elevation, char* azimuth, char* signalStrength)
{
	// create a temp satellite instance and set the data to it. 
	Satellite temp;
	temp.numberID = atoi(id);
	temp.elevationInDegrees = atoi(elevation);
	temp.azimuthInDegrees = atoi(azimuth);
	temp.signalStrength = atoi(signalStrength);

	// if signal strength is 0 = not tracking. else = tracking
	signalStrength == "\0" ? temp.tracking = false : temp.tracking = true;

	// use the gnssId to decipher which vector to add satellite to. 
	switch (gnssId)
	{
	case 1:
	{
		updateSatelliteVector(temp, gpsSatellites);
		break;
	}
	case 2:
	{
		updateSatelliteVector(temp, glonassSatellites);
		break;
	}
	case 3:
	{
		updateSatelliteVector(temp, galileoSatellites);
		break;
	}
	case 4:
	{
		updateSatelliteVector(temp, beidouSatellites);
		break;
	}
	}
}

void uBloxGPS::updateSatelliteVector(const Satellite sat, std::vector<Satellite>& satList)
{
	// TODO FIX VECTOR SEARCH

	// if list is empty, push onto the back. 
	if (satList.empty())
	{
		satList.push_back(sat);
	}
	// else - if the temp.id is found, replace. 
	//		  if the temp.id is NOT found, add to the back of the vector.
	else
	{
		std::vector<Satellite>::iterator it = std::find(satList.begin(), satList.end(), sat);

		// if found, replace.
		if (it != satList.end())
		{
			std::replace(satList.begin(), satList.end(), *it, sat);
		}
		else // push to the back. 
		{
			satList.push_back(sat);
		}
	}
}

void uBloxGPS::updateSatelliteVector(const UBX_SAT_DATA sat, std::vector<UBX_SAT_DATA>& satList)
{
	// TODO FIX VECTOR SEARCH

	// if list is empty, push onto the back. 
	if (satList.empty())
	{
		satList.push_back(sat);
	}
	// else - if the temp.satelliteId is found, replace. 
	//		  if the temp.satelliteId is NOT found, add to the back of the vector.
	else
	{
		std::vector<UBX_SAT_DATA>::iterator it = std::find(satList.begin(), satList.end(), sat);

		// if found, replace.
		if (it != satList.end())
		{
			std::replace(satList.begin(), satList.end(), *it, sat);
		}
		else // push to the back. 
		{
			satList.push_back(sat);
		}
	}
}

void uBloxGPS::updateIdOfActiveNavigationSatellites(int id, int satelliteNumber)
{
	activeNavigationSatellites[satelliteNumber].numberID = id;
}

void uBloxGPS::updateRangeOfActiveNavigationSatellites(double range, int satelliteNumber)
{
	activeNavigationSatellites[satelliteNumber].range = range;
}

int uBloxGPS::getNmeaMsgSignalId(std::string buffer)
{
	// temp variable to holds the location of the *
	size_t temp;

	// check if buffer is empty
	if (buffer.empty())
	{
		return -1;
	}
	else
	{
		// get the location of the '*' indicating the check sum
		temp = buffer.find("*");

		// if the '*' is not found, means it has already been removed 
		// return -1 for not found 
		if (temp == -1)
		{
			return -1;
		}

		// return the char converted to an int. 
		return atoi(&buffer[temp - 1]);
	}
}

bool uBloxGPS::getBit(uint8_t data, int element)
{
	int mask = 1 << element;
	int masked = data & mask;
	return masked >> element;
}

bool uBloxGPS::getBit(uint16_t data, int element)
{
	int mask = 1 << element;
	int masked = data & mask;
	return masked >> element;
}

bool uBloxGPS::getBit(uint32_t data, int element)
{
	int mask = 1 << element;
	int masked = data & mask;
	return masked >> element;
}


#pragma region getters

void uBloxGPS::getUbxChecksums(uint8_t* buffer, int checkSumA, int checkSumB)
{
	// variables for calculating checksum
	int i, j;
	uint8_t a = 0, b = 0;

	// find the data range.
	j = (buffer[4] + (buffer[5] << 8) + 6);

	// for each element inside the data range, store the size in hex. 
	for (i = 2; i < j; i++)
	{
		a += buffer[i];
		b += a;
	}

	// set the checksum bytes
	checkSumA = a;
	checkSumB = b;
}

#pragma endregion

#pragma region converters

std::string uBloxGPS::convertUtcTimeToString(char* time)
{
	struct tm gpstime;								//!< holds time parts
	char tempBuf[3];								//!< temp buffer for pulling out time elements
	std::string utc;								//!< output string.

	// UTC time typically has format of hhmmss.ss, but we will allow for hhmmss
	// verify the length of the inbound char array. 
	if ((strlen(time) == 6) || (strlen(time) == 9)) {
		// Parse hour:
		strncpy_s(tempBuf, time, 2);
		tempBuf[2] = '\0';
		gpstime.tm_hour = atoi(tempBuf);

		// Parse minutes:
		strncpy_s(tempBuf, time + 2, 2);
		tempBuf[2] = '\0';
		gpstime.tm_min = atoi(tempBuf);

		// Parse seconds:
		strncpy_s(tempBuf, time + 4, 2);
		tempBuf[2] = '\0';
		gpstime.tm_sec = atoi(tempBuf);

		// store in format
		utc = std::to_string(gpstime.tm_hour) + ":" + std::to_string(gpstime.tm_min) + ":" + std::to_string(gpstime.tm_sec);
	}
	// Else, not in anticipated format
	else
	{
		utc = "Error";
	}

	// return string
	return utc;
}

std::string uBloxGPS::convertUtcDateToString(char* date)
{
	struct tm gpstime;							//!< Holds date parts
	char tempBuf[3];							//!< temp buffer for pulling out data parts
	std::string utc;							//!< output string

	// UTC date has format of ddmmyy
	// verify length of the inbound char array matches the expected size
	if ((strlen(date) == 6))
	{
		// Parse day:
		strncpy_s(tempBuf, date, 2);
		tempBuf[2] = '\0';
		gpstime.tm_mday = atoi(tempBuf);

		// Parse month:
		strncpy_s(tempBuf, date + 2, 2);
		tempBuf[2] = '\0';
		gpstime.tm_mon = atoi(tempBuf) - 1;

		// Parse year:
		strncpy_s(tempBuf, date + 4, 2);
		tempBuf[2] = '\0';
		gpstime.tm_year = atoi(tempBuf) + 100;

		// store in format
		utc = std::to_string(gpstime.tm_mon) + "/" + std::to_string(gpstime.tm_mday) + "/" + std::to_string(gpstime.tm_year);
	}
	else
	{
		// date not expected size, set string to error 
		utc = "Error";
	}

	// return string
	return utc;
}

#pragma endregion

#pragma region printers

void uBloxGPS::printMessageInHex(uint8_t* buffer, uint8_t size)
{
	// iterate through the buffer - printing
	for (int i = 0; i < size; i++)
	{
		// Print a new line every 6th iteration. 
		if (i % 10 == 0 && i != 0) printf("\n");

		// output array element in hex. 
		std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)buffer[i] << ' ';
	}
	// new line at the end. 
	std::cout << '\n';
}

void uBloxGPS::printSatelliteData(Satellite sat)
{
	printf("\tNumber ID          :%i \n", sat.numberID);							// Number ID
	printf("\tSignal ID          :%s \n", sat.signalID.c_str());					// Signal ID
	printf("\tType               :%s \n", sat.type.c_str());						// Type
	printf("\tRange              :%f \n", sat.range);								// Range
	printf("\tlatitudeInDeg      :%f %c \n", sat.latitudeInDegrees, sat.latitudeDirection);
	printf("\tlongitudeInDeg     :%f %c \n", sat.longitudeInDegrees, sat.longitudeDirection);
	printf("\tPosition Fix Qual  :%i \n", sat.positionFixQuality);				// Position Fix Quality
	printf("\tElevation          :%f \n", sat.elevationInDegrees);				// Elevation
	printf("\tAzimuth            :%f \n", sat.azimuthInDegrees);					// Azimuth
	printf("\tSignal Strength    :%f \n", sat.signalStrength);					// Signal Strength
	printf("\tTracking           :%s \n", sat.tracking ? "true" : "false");		// Tracking
	printf("\tLast Update:       :%f \n", sat.utcTimeOfLastData);					// Last Update time
	printf("\n");
}

void uBloxGPS::printSatelliteData(UBX_SAT_DATA sat)
{
	printf("\tGNSS ID            :%i \n", sat.gnssId);
	printf("\tSatellite ID       :%i \n", sat.satelliteId);
	printf("\tCarr. To Noise Rat.:%i dBHz\n", sat.carrierToNoiseRatioInDbhz);
	printf("\tElevation          :%i deg\n", sat.elevationInDeg);
	printf("\tAzimuth            :%i deg\n", sat.azimuthInDeg);
	printf("\tPseudo Range Res.  :%i m\n", sat.pseudoRangeResidualInM);
	printf("\n");
}

void uBloxGPS::printFixData(FixData data)
{
	// lock 
	std::unique_lock<std::mutex> lock(m_mutex);

	printf("\tUTC Time           :%.0f \n", data.utcTime);
	printf("\tlatitudeInDeg           :%4.2f %c\n", data.latitudeInDegrees, data.latitudeDirection);
	printf("\tlongitudeInDeg          :%4.2f %c\n", data.longitudeInDegrees, data.longitudeDirection);
	printf("\tPosition Fix Qual  :%i \n", data.positionFixQuality);
	printf("\t# Satellites Used  :%i \n", data.numberSatellitesUsed);
	printf("\tHorizontal DOP     :%4.2f \n", data.horizontalDOP);
	printf("\tAltitude ASL       :%4.2f M\n", data.altitudeASLinMeters);
	printf("\tGeoid Seperation   :%4.2f M\n", data.geoidSeperationInMeters);
	printf("\tDiff Age           :%4.2f \n", data.differentialAge);
	printf("\tDiff Station ID    :%4.2f \n", data.differentialStationId);
}

void uBloxGPS::printAllNmeaData()
{
	// lock 
	std::unique_lock<std::mutex> lock(m_mutex);

	printf("GNS FIX DATA:\n");
	printFixData(gnsFixData);

	printf("GPS FIX DATA:\n");
	printFixData(gpsFixData);

	printf("\tUTC Time           :%.0f \n", utcTime);
	printf("\tUTC Date           :%i \n", utcDate);
	printf("\tLocal Hours        :%i \n", localTimeZoneHours);
	printf("\tLocal Minutes      :%i \n", localTimeZoneMinutes);

	printf("\tData Status        :%c \n", dataStatus);
	printf("\tNav Status         :%c \n", navStatus);
	printf("\tNav Mode           :%i \n", navigationMode);
	printf("\tOperation Mode     :%i \n", operationMode);
	printf("\tGPS Pos Mode       :%c \n", positionModeGps);
	printf("\tGalileo Pos Mode   :%c \n", positionModeGalileo);
	printf("\tGlonass Pos Mode   :%c \n", positionModeGlonass);
	printf("\tBeiDou Pos Mode    :%c \n", positionModeBeidou);

	printf("\tlatitudeInDeg           :%4.2f %c\n", latitudeInDegrees, latitudeDirection);
	printf("\tlatitudeInDeg Stan Dev  :%4.2f M\n", latitudeStandardDeviationInMeters);
	printf("\tlongitudeInDeg          :%4.2f %c\n", longitudeInDegrees, longitudeDirection);
	printf("\tlongitudeInDeg Stan Dev :%4.2f M\n", latitudeStandardDeviationInMeters);
	printf("\tAltitude ASL       :%4.2f M\n", altitudeASLinMeters);
	printf("\tAltitude Stan Dev  :%4.2f M\n", altitudeStandardDeviatioInMeters);

	printf("\tGeoid Seperation   :%4.2f M\n", geoidSeperationInMeters);

	printf("\tPosition Fix Qual  :%i \n", positionFixQuality);

	printf("\t# Satellites Used  :%i \n", numberSatellitesUsed);

	printf("\tDiff Age           :%4.2f \n", differentialAge);
	printf("\tDiff Station ID    :%4.2f \n", differentialStationId);

	printf("\tDatum Reference    :%s \n", datumReference.c_str());
	printf("\tlatitude Offset    :%4.2f %c\n", latitudeOffset, latitudeOffsetDirection);
	printf("\tlongitude Offset   :%4.2f %c\n", longitudeOffset, longitudeOffsetDirection);
	printf("\tAltitude Offset    :%4.2f \n", altitudeOffset);

	printf("\tSpeed Over Ground  :%4.2f Knots\n", speedOverGroundInKnots);
	printf("\tSpeed Over Ground  :%4.2f KMH\n", speedOverGroundInKMH);

	printf("\tCourse Over Ground :%4.2f deg\n", courseOverGroundInDegrees);
	printf("\tMagnetic COG       :%4.2f deg \n", magneticCourseOverGroundInDegrees);

	//printf("\tPosition DOP       :%4.2f \n", positionDOP);
	//printf("\tHorizontal DOP     :%4.2f \n", horizontalDOP);
	//printf("\tVertical DOP       :%4.2f \n", verticalDOP);

	printf("\tRange RMS          :%4.2f \n", rangeRMS);
	printf("\tMajor Axis Stan Dev:%4.2f M\n", majorAxisStandardDeviationInMeters);
	printf("\tMinor Axis Stan Dev:%4.2f M\n", minorAxisStandardDeviationInMeters);
	printf("\tOrientation        :%4.2f deg\n", orientationInDegrees);

	printf("\tLast Text Type     :%s \n", lastTextTransmissionType.c_str());
	printf("\tLast Text Msg      :%s \n", lastTextTransmission.c_str());

	printf("\tMangetic Variation :%4.2f %c\n", magneticVariationInDegrees, magneticVariationDirection);

	printf("\tTotal Water Dist.  :%4.2f NMi\n", totalCumulativeWaterDistanceInNautMiles);
	printf("\tCurr. Water Dist.  :%4.2f NMi\n", waterDistanceSinceResetInNautMiles);
	printf("\tTotal Ground Dist. :%4.2f NMi\n", totalCumulativeGroundDistanceInNautMiles);
	printf("\tCurr. Ground Dist. :%4.2f NMi\n", groundDistanceSinceResetInNautMiles);

	// Print Data For all 12 Active Nav Satellites
	for (int i = 0; i < 12; i++)
	{
		printf("\tNav Sat. %i ID     :%i \n", i, activeNavigationSatellites[i].numberID);
		printf("\tNav Sat. %i Range  :%f \n", i, activeNavigationSatellites[i].range);
	}

	// print gps satellites
	printf("\nPRINTING GPS SAT DATA:\n");
	int x = 1;
	for (auto it = gpsSatellites.begin(); it != gpsSatellites.end(); ++it)
	{
		printf("\nSATELLITE %i:\n", x);
		printSatelliteData(*it);
		x++;
	}

	// print galileo satellites
	printf("\nPRINTING GALILEO SAT DATA:\n");
	x = 1;
	for (auto it = galileoSatellites.begin(); it != galileoSatellites.end(); ++it)
	{
		printf("\nSATELLITE %i:\n", x);
		printSatelliteData(*it);
		x++;
	}

	// print glonass satellites
	printf("\nPRINTING GLONASS SAT DATA:\n");
	x = 1;
	for (auto it = glonassSatellites.begin(); it != glonassSatellites.end(); ++it)
	{
		printf("\nSATELLITE %i:\n", x);
		printSatelliteData(*it);
		x++;
	}

	// print beidou satellites
	printf("\nPRINTING BEIDOU SAT DATA:\n");
	x = 1;
	for (auto it = beidouSatellites.begin(); it != beidouSatellites.end(); ++it)
	{
		printf("\nSATELLITE %i:\n", x);
		printSatelliteData(*it);
		x++;
	}
}

void uBloxGPS::printNavPositionEcefData()
{
	// lock 
	std::unique_lock<std::mutex> lock(m_mutex);

	printf("\tGPS Time of Week   :%i ms\n", gpsTimeOfWeekInMilliSecs);
	printf("\tECEF Position X    :%i cm\n", ecefXPositonInCm);
	printf("\tECEF Position Y    :%i cm\n", ecefYPositonInCm);
	printf("\tECEF Position Z    :%i cm\n", ecefZPositonInCm);
	printf("\tECEF Pos. Acc. Est.:%i cm\n", positionAccuraryEstimateInCm);
}

void uBloxGPS::printNavVelocityEcefData()
{
	// lock 
	std::unique_lock<std::mutex> lock(m_mutex);

	printf("\tGPS Time of Week   :%i ms\n", gpsTimeOfWeekInMilliSecs);
	printf("\tECEF Velocity X    :%i cm/s\n", ecefXVelocityInCms);
	printf("\tECEF Velocity Y    :%i cm/s\n", ecefYVelocityInCms);
	printf("\tECEF Velocity Z    :%i cm/s\n", ecefZVelocityInCms);
	printf("\tSpeed Accuracy Est.:%i cm/s\n", speedAccuracyEstimateInCms);
}

void uBloxGPS::printNavPositionVelocityTimeData()
{
	// lock 
	std::unique_lock<std::mutex> lock(m_mutex);

	printf("\tGPS Time of Week   :%i ms\n", gpsTimeOfWeekInMilliSecs);
	printf("\tYear               :%i \n", year);
	printf("\tMonth              :%i \n", month);
	printf("\tDay                :%i \n", day);
	printf("\tHour               :%i \n", hour);
	printf("\tMinute             :%i \n", minute);
	printf("\tSecond             :%i \n", seconds);
	printf("\tValid              :%i \n", valid);
	printf("\tTime Accuracy Est. :%i ns\n", timeAccuraryEstimateInNs);
	printf("\tNano               :%i ns\n", nanoSeconds);
	printf("\tGNSS Fix Type      :%i \n", gnssFixType);
	printf("\tFlags              :%i \n", flags);
	printf("\tFlags 2            :%i \n", flags2);
	printf("\tNum Satellites Used:%i \n", numSatellitesUsed);
	printf("\tLongitude          :%i deg\n", longitudeInDeg);
	printf("\tLatitude           :%i deg\n", latitudeInDeg);
	printf("\tHeight Above Elip  :%i mm\n", heightAboveEllipsoidInMm);
	printf("\tHeight Above MSL   :%i mm\n", heightAboveMeanSeaLevelInMm);
	printf("\tHorizontal Acc.    :%i mm\n", horizontalAccuracyEstimateInMm);
	printf("\tVertical Acc.      :%i mm\n", verticalAccuracyEstimateInMm);
	printf("\tVelocity North     :%i mm/s\n", velocityNorthInMms);
	printf("\tVelocuty East      :%i mm/s\n", velocityEastInMms);
	printf("\tVelocity Down      :%i mm/s\n", velocityDownInMms);
	printf("\tGround Speed       :%i mm/s\n", groundSpeedInMms);
	printf("\tHeading Of Motion  :%i deg\n", headingOfMotionInDeg);
	printf("\tSpeed Accuracy Est.:%i mm/s\n", speedAccuracyEstimateInMms);
	printf("\tHeading Acc. Est.  :%i deg\n", headingAccuracyEstimateInDeg);
	printf("\tPosition DOP       :%d \n", positionDOP);
	printf("\tHeading Of Vehicle :%i deg\n", headingOfVehicleInDeg);
	printf("\tMagnetic Decline   :%i deg\n", magneticDeclinationInDeg);
	printf("\tMag. Decline Acc.  :%i deg\n", magneticDeclinationAccuracyInDeg);
}

void uBloxGPS::printNavSatelliteData()
{
	// lock 
	std::unique_lock<std::mutex> lock(m_mutex);

	printf("\tGPS Time of Week   :%i ms\n", gpsTimeOfWeekInMilliSecs);
	printf("\tNumber Satellites  :%i \n", numberSatellites);

	// print the satellite data in vector
	int x = 1;
	for (int index = 0; index < satellites.size(); index++)
	{
		printf("\nSATELLITE %i:\n", x);
		printSatelliteData(satellites[index]);
		x++;
	}
}

void uBloxGPS::printMonitorVersionData()
{
	// lock 
	std::unique_lock<std::mutex> lock(m_mutex);

	printf("\tSoftware Version   :%s \n", softwareVersion);
	printf("\tHardware Version   :%s \n", hardwareVersion);
	printf("\tExt. Version Data  :%s \n", extendedVersionData.c_str());
}

void uBloxGPS::printNavTimeUtcData()
{
	// lock 
	std::unique_lock<std::mutex> lock(m_mutex);

	printf("\tGPS Time of Week   :%i ms\n", gpsTimeOfWeekInMilliSecs);
	printf("\tTime Accuracy Est. :%i ns\n", timeAccuraryEstimateInNs);
	printf("\tNano               :%i ns\n", nanoSeconds);
	printf("\tYear               :%i \n", year);
	printf("\tMonth              :%i \n", month);
	printf("\tDay                :%i \n", day);
	printf("\tHour               :%i \n", hour);
	printf("\tMinute             :%i \n", minute);
	printf("\tSecond             :%i \n", seconds);
	printf("\tValid              :%i \n", valid);
}

void uBloxGPS::printNavStatusData()
{
	// lock 
	std::unique_lock<std::mutex> lock(m_mutex);

	printf("\tGPS Time of Week   :%i ms\n", gpsTimeOfWeekInMilliSecs);
	printf("\tGPS Fix Type       :%i \n", timeAccuraryEstimateInNs);
	printf("\tNAV Status Flags   :%i \n", navStatusFlags);
	printf("\tFix Status Flags   :%i \n", fixStatusFlags);
	printf("\tNav Flags 2        :%i \n", navFlags2);
	printf("\tTime To First Fix  :%i ms\n", timeToFirstFixInMilliseconds);
	printf("\tTime Since Startup :%i ms\n", milliSecondsSinceStartup);
}

void uBloxGPS::printNavVelocityNedData()
{
	// lock 
	std::unique_lock<std::mutex> lock(m_mutex);

	printf("\tGPS Time of Week   :%i ms\n", gpsTimeOfWeekInMilliSecs);
	printf("\tVelocity North     :%i cm/s\n", velocityNorthInCms);
	printf("\tVelocity East      :%i cm/s\n", velocityEastInCms);
	printf("\tVelocity Down      :%i cm/s\n", velocityDownInCms);
	printf("\t3D Speed           :%i cm/s\n", speedInCms);
	printf("\tGround Speed       :%i cm/s\n", groundSpeedInCms);
	printf("\tHeading Of Motion  :%i deg\n", headingOfMotionInDeg);
	printf("\tSpeed Accuracy Est.:%i cm/s\n", speedAccuracyEstimateInCms);
	printf("\tHeading Acc. Est.  :%i deg\n", headingAccuracyEstimateInDeg);
}

void uBloxGPS::printNavDopData()
{
	// lock 
	std::unique_lock<std::mutex> lock(m_mutex);

	printf("\tGPS Time of Week   :%i ms\n", gpsTimeOfWeekInMilliSecs);
	printf("\tGeometric DOP      :%i \n", geometricDOP);
	printf("\tPosition DOP       :%i \n", positionDOP);
	printf("\tTime DOP           :%i \n", timeDOP);
	printf("\tVertical DOP       :%i \n", verticalDOP);
	printf("\tHorizontal DOP     :%i \n", horizontalDOP);
	printf("\tNorthing DOP       :%i \n", northingDOP);
	printf("\tEasting DOP        :%i \n", eastingDOP);
}

void uBloxGPS::printNavPositionLlhData()
{
	// lock
	std::unique_lock<std::mutex> lock(m_mutex);

	printf("\tGPS Time of Week   :%i ms\n", gpsTimeOfWeekInMilliSecs);
	printf("\tLongitude          :%i deg\n", longitudeInDeg);
	printf("\tLatitude           :%i deg\n", latitudeInDeg);
	printf("\tHeight Above Elip  :%i mm\n", heightAboveEllipsoidInMm);
	printf("\tHeight Above MSL   :%i mm\n", heightAboveMeanSeaLevelInMm);
	printf("\tHorizontal Acc.    :%i mm\n", horizontalAccuracyEstimateInMm);
	printf("\tVertical Acc.      :%i mm\n", verticalAccuracyEstimateInMm);
}

#pragma endregion

#pragma region parsers

int uBloxGPS::parseNmeaMessage(char* buffer, char** fields, int max_fields)
{
	// iterator for size of inbound buffer
	int i = 0;
	fields[i++] = buffer;

	// Iterate through the string and change each comma into a null to terminate the C string. 
	// Then store the address of the next location into an array so we can later reference to the start of each field.
	while ((i < max_fields) && NULL != (buffer = strchr(buffer, ',')))
	{
		*buffer = '\0';
		fields[i++] = ++buffer;
	}

	// return
	return --i;
}

void uBloxGPS::parseBitmaskFlags(uint32_t* flags)
{
	//TODO
}

void uBloxGPS::parseValidityFlags(uint8_t* flags)
{
	// TODO
}

void uBloxGPS::parseFixStatusFlags(uint8_t* flags)
{
	// TODO
}

void uBloxGPS::parseAdditionalFlags(uint8_t* flags)
{
	// TODO
}


void uBloxGPS::parseMonitorVersionData(uint8_t* buffer)
{
	// this message is variable length, 
	// so get the data length from the buffer at index 4
	uint16_t payloadSize;
	memcpy(&payloadSize, buffer + 4, sizeof(payloadSize));

	// lock 
	std::unique_lock<std::mutex> lock(m_mutex);

	// Start at Buffer+6 (sync bytes, ids, payload length are in first 6 bytes)
	memcpy(&softwareVersion, buffer + 6, sizeof(softwareVersion));
	memcpy(&hardwareVersion, buffer + 36, sizeof(hardwareVersion));	// starts at payload(index 30) + starting 6 bytes = 36
	memcpy(&extendedVersionData, buffer + 46, payloadSize - 40);	// starts at payload(index 40) + starting 6 bytes = 46
}

void uBloxGPS::parseNavSatelliteData(uint8_t* buffer)
{
	// lock 
	std::unique_lock<std::mutex> lock(m_mutex);

	// Start at Buffer+6 (sync bytes, ids, payload length are in first 6 bytes)
	memcpy(&gpsTimeOfWeekInMilliSecs, buffer + 6, sizeof(gpsTimeOfWeekInMilliSecs));

	// verify message version so we can parse correctly
	if (buffer[10] == UBX::NAV::SAT::messageVersion)
	{
		int satsInMsg = buffer[11];
		// buffer[12] && buffer[13] = reserved

		for (int i = 0; i < satsInMsg; i++)
		{
			UBX_SAT_DATA temp;

			memcpy(&temp.gnssId, buffer + (14 + (12 * i)), sizeof(temp.gnssId));
			memcpy(&temp.satelliteId, buffer + (15 + (12 * i)), sizeof(temp.satelliteId));
			memcpy(&temp.carrierToNoiseRatioInDbhz, buffer + (16 + (12 * i)), sizeof(temp.carrierToNoiseRatioInDbhz));
			memcpy(&temp.elevationInDeg, buffer + (17 + (12 * i)), sizeof(temp.elevationInDeg));
			memcpy(&temp.azimuthInDeg, buffer + (18 + (12 * i)), sizeof(temp.azimuthInDeg));
			memcpy(&temp.pseudoRangeResidualInM, buffer + (20 + (12 * i)), sizeof(temp.pseudoRangeResidualInM));
			memcpy(&temp.flags, buffer + (22 + (12 * i)), sizeof(temp.flags));

			updateSatelliteVector(temp, satellites);
		}

		numberSatellites = satellites.size();
	}
}

void uBloxGPS::parseNavPositionVelocityTimeData(uint8_t* buffer)
{
	// lock
	std::unique_lock<std::mutex> lock(m_mutex);

	// Start at Buffer+6 (sync bytes, ids, payload length are in first 6 bytes)
	memcpy(&gpsTimeOfWeekInMilliSecs, buffer + 6, sizeof(gpsTimeOfWeekInMilliSecs));
	memcpy(&year, buffer + 10, sizeof(year));
	memcpy(&month, buffer + 12, sizeof(month));
	memcpy(&day, buffer + 13, sizeof(day));
	memcpy(&hour, buffer + 14, sizeof(hour));
	memcpy(&minute, buffer + 15, sizeof(minute));
	memcpy(&seconds, buffer + 16, sizeof(seconds));
	memcpy(&valid, buffer + 17, sizeof(valid));
	memcpy(&timeAccuraryEstimateInNs, buffer + 18, sizeof(timeAccuraryEstimateInNs));
	memcpy(&nanoSeconds, buffer + 22, sizeof(nanoSeconds));
	memcpy(&gnssFixType, buffer + 26, sizeof(gnssFixType));
	memcpy(&flags, buffer + 27, sizeof(flags));
	memcpy(&flags2, buffer + 28, sizeof(flags2));
	memcpy(&numSatellitesUsed, buffer + 29, sizeof(numSatellitesUsed));
	memcpy(&longitudeInDeg, buffer + 30, sizeof(longitudeInDeg));
	memcpy(&latitudeInDeg, buffer + 34, sizeof(latitudeInDeg));
	memcpy(&heightAboveEllipsoidInMm, buffer + 38, sizeof(heightAboveEllipsoidInMm));
	memcpy(&heightAboveMeanSeaLevelInMm, buffer + 42, sizeof(heightAboveMeanSeaLevelInMm));
	memcpy(&horizontalAccuracyEstimateInMm, buffer + 46, sizeof(horizontalAccuracyEstimateInMm));
	memcpy(&verticalAccuracyEstimateInMm, buffer + 50, sizeof(verticalAccuracyEstimateInMm));
	memcpy(&velocityNorthInMms, buffer + 54, sizeof(velocityNorthInMms));
	memcpy(&velocityEastInMms, buffer + 58, sizeof(velocityEastInMms));
	memcpy(&velocityDownInMms, buffer + 62, sizeof(velocityDownInMms));
	memcpy(&groundSpeedInMms, buffer + 66, sizeof(groundSpeedInMms));
	memcpy(&headingOfMotionInDeg, buffer + 70, sizeof(headingOfMotionInDeg));
	memcpy(&speedAccuracyEstimateInMms, buffer + 74, sizeof(speedAccuracyEstimateInMms));
	memcpy(&headingAccuracyEstimateInDeg, buffer + 78, sizeof(headingAccuracyEstimateInDeg));
	memcpy(&positionDOP, buffer + 82, sizeof(positionDOP));
	memcpy(&headingOfVehicleInDeg, buffer + 90, sizeof(headingOfVehicleInDeg));
	memcpy(&magneticDeclinationInDeg, buffer + 94, sizeof(magneticDeclinationInDeg));
	memcpy(&magneticDeclinationAccuracyInDeg, buffer + 96, sizeof(magneticDeclinationAccuracyInDeg));

	// adjust for scaling
	longitudeInDeg = longitudeInDeg * 1e-7;
	latitudeInDeg = latitudeInDeg * 1e-7;
	headingOfMotionInDeg = headingOfMotionInDeg * 1e-5;
	headingAccuracyEstimateInDeg = headingAccuracyEstimateInDeg * 1e-5;
	positionDOP = positionDOP * 0.01;
	headingOfVehicleInDeg = headingOfVehicleInDeg * 1e-5;
	magneticDeclinationInDeg = magneticDeclinationInDeg * 1e-2;
	magneticDeclinationAccuracyInDeg = magneticDeclinationAccuracyInDeg * 1e-2;
}

void uBloxGPS::parseNavStatusData(uint8_t* buffer)
{
	// lock
	std::unique_lock<std::mutex> lock(m_mutex);

	// Start at Buffer+6 (sync bytes, ids, payload length are in first 6 bytes)
	memcpy(&gpsTimeOfWeekInMilliSecs, buffer + 6, sizeof(gpsTimeOfWeekInMilliSecs));
	memcpy(&gpsFixType, buffer + 10, sizeof(gpsFixType));
	memcpy(&navStatusFlags, buffer + 11, sizeof(navStatusFlags));
	memcpy(&fixStatusFlags, buffer + 12, sizeof(fixStatusFlags));
	memcpy(&navFlags2, buffer + 13, sizeof(navFlags2));
	memcpy(&timeToFirstFixInMilliseconds, buffer + 14, sizeof(timeToFirstFixInMilliseconds));
	memcpy(&milliSecondsSinceStartup, buffer + 18, sizeof(milliSecondsSinceStartup));
}

void uBloxGPS::parseNavPositionEcefData(uint8_t* buffer)
{
	// lock
	std::unique_lock<std::mutex> lock(m_mutex);

	// Start at Buffer+6 (sync bytes, ids, payload length are in first 6 bytes)
	memcpy(&gpsTimeOfWeekInMilliSecs, buffer + 6, sizeof(gpsTimeOfWeekInMilliSecs));
	memcpy(&ecefXPositonInCm, buffer + 10, sizeof(ecefXPositonInCm));
	memcpy(&ecefYPositonInCm, buffer + 14, sizeof(ecefYPositonInCm));
	memcpy(&ecefZPositonInCm, buffer + 18, sizeof(ecefZPositonInCm));
	memcpy(&positionAccuraryEstimateInCm, buffer + 22, sizeof(positionAccuraryEstimateInCm));
}

void uBloxGPS::parseNavPositionLlhData(uint8_t* buffer)
{
	// lock
	std::unique_lock<std::mutex> lock(m_mutex);

	// Start at Buffer+6 (sync bytes, ids, payload length are in first 6 bytes)
	memcpy(&gpsTimeOfWeekInMilliSecs, buffer + 6, sizeof(gpsTimeOfWeekInMilliSecs));
	memcpy(&longitudeInDeg, buffer + 10, sizeof(longitudeInDeg));
	memcpy(&latitudeInDeg, buffer + 14, sizeof(latitudeInDeg));
	memcpy(&heightAboveEllipsoidInMm, buffer + 18, sizeof(heightAboveEllipsoidInMm));
	memcpy(&heightAboveMeanSeaLevelInMm, buffer + 22, sizeof(heightAboveMeanSeaLevelInMm));
	memcpy(&horizontalAccuracyEstimateInMm, buffer + 26, sizeof(horizontalAccuracyEstimateInMm));
	memcpy(&verticalAccuracyEstimateInMm, buffer + 30, sizeof(verticalAccuracyEstimateInMm));

	// adjust for scaling
	longitudeInDeg = longitudeInDeg * 1e-7;
	latitudeInDeg = latitudeInDeg * 1e-7;
}

void uBloxGPS::parseNavVelocityEcefData(uint8_t* buffer)
{
	// lock
	std::unique_lock<std::mutex> lock(m_mutex);

	// Start at Buffer+6 (sync bytes, ids, payload length are in first 6 bytes)
	memcpy(&gpsTimeOfWeekInMilliSecs, buffer + 6, sizeof(gpsTimeOfWeekInMilliSecs));
	memcpy(&ecefXVelocityInCms, buffer + 10, sizeof(ecefXVelocityInCms));
	memcpy(&ecefYVelocityInCms, buffer + 14, sizeof(ecefYVelocityInCms));
	memcpy(&ecefZVelocityInCms, buffer + 18, sizeof(ecefZVelocityInCms));
	memcpy(&speedAccuracyEstimateInCms, buffer + 22, sizeof(speedAccuracyEstimateInCms));
}

void uBloxGPS::parseNavVelocityNedData(uint8_t* buffer)
{
	// lock
	std::unique_lock<std::mutex> lock(m_mutex);

	// Start at Buffer+6 (sync bytes, ids, payload length are in first 6 bytes)
	memcpy(&gpsTimeOfWeekInMilliSecs, buffer + 6, sizeof(gpsTimeOfWeekInMilliSecs));
	memcpy(&velocityNorthInCms, buffer + 10, sizeof(velocityNorthInCms));
	memcpy(&velocityEastInCms, buffer + 14, sizeof(velocityEastInCms));
	memcpy(&velocityDownInCms, buffer + 18, sizeof(velocityDownInCms));
	memcpy(&speedInCms, buffer + 22, sizeof(speedInCms));
	memcpy(&groundSpeedInCms, buffer + 26, sizeof(groundSpeedInCms));
	memcpy(&headingOfMotionInDeg, buffer + 30, sizeof(headingOfMotionInDeg));
	memcpy(&speedAccuracyEstimateInCms, buffer + 34, sizeof(speedAccuracyEstimateInCms));
	memcpy(&headingAccuracyEstimateInDeg, buffer + 38, sizeof(headingAccuracyEstimateInDeg));

	// adjust for scaling. 
	headingOfMotionInDeg = headingOfMotionInDeg * 1e-5;
	headingAccuracyEstimateInDeg = headingAccuracyEstimateInDeg * 1e-5;
}

void uBloxGPS::parseNavTimeUtcData(uint8_t* buffer)
{
	// lock
	std::unique_lock<std::mutex> lock(m_mutex);

	// Start at Buffer+6 (sync bytes, ids, payload length are in first 6 bytes)
	memcpy(&gpsTimeOfWeekInMilliSecs, buffer + 6, sizeof(gpsTimeOfWeekInMilliSecs));
	memcpy(&timeAccuraryEstimateInNs, buffer + 10, sizeof(timeAccuraryEstimateInNs));
	memcpy(&nanoSeconds, buffer + 14, sizeof(nanoSeconds));
	memcpy(&year, buffer + 18, sizeof(year));
	memcpy(&month, buffer + 20, sizeof(month));
	memcpy(&day, buffer + 21, sizeof(day));
	memcpy(&hour, buffer + 22, sizeof(hour));
	memcpy(&minute, buffer + 23, sizeof(minute));
	memcpy(&seconds, buffer + 24, sizeof(seconds));
	memcpy(&valid, buffer + 25, sizeof(valid));
}

void uBloxGPS::parseNavDopData(uint8_t* buffer)
{
	// lock
	std::unique_lock<std::mutex> lock(m_mutex);

	// Start at Buffer+6 (sync bytes, ids, payload length are in first 6 bytes)
	memcpy(&gpsTimeOfWeekInMilliSecs, buffer + 6, sizeof(gpsTimeOfWeekInMilliSecs));
	memcpy(&geometricDOP, buffer + 10, sizeof(geometricDOP));
	memcpy(&positionDOP, buffer + 12, sizeof(positionDOP));
	memcpy(&timeDOP, buffer + 14, sizeof(timeDOP));
	memcpy(&verticalDOP, buffer + 16, sizeof(verticalDOP));
	memcpy(&horizontalDOP, buffer + 18, sizeof(horizontalDOP));
	memcpy(&northingDOP, buffer + 20, sizeof(northingDOP));
	memcpy(&eastingDOP, buffer + 22, sizeof(eastingDOP));

	// adjust for scaling. 
	geometricDOP = geometricDOP * 0.01;
	positionDOP = positionDOP * 0.01;
	timeDOP = timeDOP * 0.01;
	verticalDOP = verticalDOP * 0.01;
	horizontalDOP = horizontalDOP * 0.01;
	northingDOP = northingDOP * 0.01;
	eastingDOP = eastingDOP * 0.01;
}


#pragma endregion

#pragma region decoders

std::string uBloxGPS::decodeSignalId(int gnssId, char* signalId)
{
	// store conversion for return
	std::string type;

	// catch empty string = QZSS
	if (gnssId == 0 && strlen(signalId) == 0)
	{
		type = "QZSS";
	}
	// else convert to appropriate type
	else
	{
		switch (gnssId)
		{
		case 1:
		{
			switch (atoi(signalId))
			{
			case 1: type = "GPS L1C/A"; break;
			case 5: type = "GPS L2 CM"; break;
			case 6: type = "GPS L2 CL "; break;
			}
			break;
		}
		case 2:
		{
			switch (atoi(signalId))
			{
			case 1: type = "GLONASS L1 OF*"; break;
			case 3: type = "GLONASS L2 OF"; break;
			}
			break;
		}
		case 3:
		{
			switch (atoi(signalId))
			{
			case 2: type = "Galileo E5"; break;
			case 7: type = "Galileo E1"; break;
			}
		}
		case 4:
		{
			switch (atoi(signalId))
			{
			case 1: type = "BeiDou B1I"; break;
			case 3: type = "BeiDou B2I"; break;
			}
			break;
		}
		default: type = "Unknown";
		}
	}

	// return
	return type;
}

std::string uBloxGPS::decodeDataStatus(char* status)
{
	// holds decoded meaning
	std::string type;

	// find the chars appropriate meaning.
	if (strcmp(status, "V") == 0)
	{
		type = "Data Invalid";
	}
	else if (strcmp(status, "A") == 0)
	{
		type = "Data Valid";
	}
	else if (status == NULL)
	{
		type = "Error";
	}

	// return string of decoded type
	return type;
}

std::string uBloxGPS::decodePosMode(char* posMode)
{
	// holds decoded meaning
	std::string type;

	// find the chars appropriate meaning.
	if (strcmp(posMode, "N") == 0)
	{
		type = "No Fix";
	}
	else if (strcmp(posMode, "E") == 0)
	{
		type = "Estimated/Dead Reckoning Fix";
	}
	else if (strcmp(posMode, "A") == 0)
	{
		type = "Autonomous GNSS Fix";
	}
	else if (strcmp(posMode, "D") == 0)
	{
		type = "Differential GNSS Fix";
	}
	else if (strcmp(posMode, "F") == 0)
	{
		type = "RTK Float";
	}
	else if (strcmp(posMode, "R") == 0)
	{
		type = "RTK Fixed";
	}
	else if (posMode == NULL)
	{
		type = "Error";
	}

	// return string of decoded type
	return type;
}

std::string uBloxGPS::decodeNavMode(char* navMode)
{
	// holds decoded meaning
	std::string type;

	// convert the char to int and find its appropriate meaning.
	switch (atoi(navMode))
	{
	case 1: type = "No Fix"; break;
	case 2: type = "2D Fix"; break;
	case 3: type = "3D Fix"; break;
	default: type = "Error";
	}

	// return string
	return type;
}

std::string uBloxGPS::decodeTxtMsgType(char* msgType)
{
	// holds decoded meaning
	std::string type;

	// convert the char to int and find its appropriate meaning.
	switch (atoi(msgType))
	{
	case 00: type = "Error"; break;
	case 01: type = "Warning"; break;
	case 02: type = "Notice"; break;
	case 07: type = "User"; break;
	default: type = "Error";
	}

	// return string
	return type;
}

std::string uBloxGPS::decodeQuality(char* quality)
{
	// holds decoded meaning
	std::string type;

	// convert the char to int and find its appropriate meaning.
	switch (atoi(quality))
	{
	case 0: type = "No Fix"; break;
	case 1: type = "Autonomous GNSS Fix"; break;
	case 2: type = "Differential GNSS Fix"; break;
	case 4: type = "RTK Fixed"; break;
	case 5: type = "RTK Float"; break;
	case 6: type = "Estimated/Dead Reckoning Fix"; break;
	default: type = "Error";
	}

	// return string
	return type;
}

std::string uBloxGPS::decodeOpMode(char* opMode)
{
	// holds decoded mode
	std::string mode;

	// Decode the opMode
	if (strcmp(opMode, "M") == 0)
	{
		mode = "Manual Mode";
	}
	else if (strcmp(opMode, "A") == 0)
	{
		mode = "Automatic Mode";
	}
	else if (opMode == NULL)
	{
		mode = "Error";
	}

	// turn string
	return mode;
}

std::string uBloxGPS::decodeGnssId(char* gnssId)
{
	// store conversion for return
	std::string type;

	// catch empty string = QZSS
	if (strlen(gnssId) == 0)
	{
		type = "QZSS";
	}
	// else convert to appropriate type
	else
	{
		switch (atoi(gnssId))
		{
		case 1: type = "GPS"; break;
		case 2: type = "GLONASS"; break;
		case 3: type = "Galileo"; break;
		case 4: type = "BeiDou"; break;
		default: type = "Error";
		}
	}

	// return converted type.
	return type;
}

#pragma endregion