//
// AdsbWrapper.cpp: ADSB decoding 
//
// Copyright (c) 2019 Bruce Clay

// Portions of thei code was adapted from dump1090 and dump978
// Copyright (c) 2017 Oliver Jowett <oliver@mutability.co.uk>

// This file is free software: you may copy, redistribute and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 2 of the License, or (at your
// option) any later version.
//
// This file is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//

#include <string.h>
#include <algorithm>
#include <time.h>
#include <QTime>

#include "AdsbWrapper.h"


#define CRC16 0x8005

uint16_t gen_crc16(const uint8_t *data, uint16_t size)
{
	uint16_t out = 0;
	int bits_read = 0, bit_flag;

	/* Sanity check: */
	if (data == NULL)
		return 0;

	while (size > 0)
	{
		bit_flag = out >> 15;

		/* Get next bit: */
		out <<= 1;
		out |= (*data >> bits_read) & 1; // item a) work from the least significant bits

		/* Increment bit counter: */
		bits_read++;
		if (bits_read > 7)
		{
			bits_read = 0;
			data++;
			size--;
		}

		/* Cycle check: */
		if (bit_flag)
			out ^= CRC16;

	}

	// item b) "push out" the last 16 bits
	int i;
	for (i = 0; i < 16; ++i) {
		bit_flag = out >> 15;
		out <<= 1;
		if (bit_flag)
			out ^= CRC16;
	}

	// item c) reverse the bits
	uint16_t crc = 0;
	i = 0x8000;
	int j = 0x0001;
	for (; i != 0; i >>= 1, j <<= 1) {
		if (i & out) crc |= j;
	}

	return crc;
}

/*
std::map<int, std::string> AdsbWrapper::mFisbProductNameMap =
{
	0, "METAR",
1, "TAF",
2, "SIGMET",
3, "Conv SIGMET",
4, "AIRMET",
5, "PIREP",
6, "Severe Wx",
7, "Winds Aloft",
8, "NOTAM",           //"NOTAM (Including TFRs) and Service Status"; 
9, "D-ATIS",          //"Aerodrome and Airspace – D-ATIS"; 
10, "Terminal Wx",     //"Aerodrome and Airspace - TWIP"; 
11, "AIRMET",          //"Aerodrome and Airspace - AIRMET"; 
12, "SIGMET",          //"Aerodrome and Airspace - SIGMET/Convective SIGMET"; 
13, "SUA",             //"Aerodrome and Airspace - SUA Status"; 
20, "METAR",           //"METAR and SPECI"; 
21, "TAF",             //"TAF and Amended TAF"; 
22, "SIGMET",          //"SIGMET"; 
23, "Conv SIGMET",     //"Convective SIGMET"; 
24, "AIRMET",          //"AIRMET"; 
25, "PIREP",           //"PIREP"; 
26, "Severe Wx",       //"AWW"; 
27, "Winds Aloft",     //"Winds and Temperatures Aloft"; 
51, "NEXRAD",          //"National NEXRAD, Type 0 - 4 level"; 
52, "NEXRAD",          //"National NEXRAD, Type 1 - 8 level (quasi 6-level VIP)"; 
53, "NEXRAD",          //"National NEXRAD, Type 2 - 8 level"; 
54, "NEXRAD",          //"National NEXRAD, Type 3 - 16 level"; 
55, "NEXRAD",          //"Regional NEXRAD, Type 0 - low dynamic range"; 
56, "NEXRAD",          //"Regional NEXRAD, Type 1 - 8 level (quasi 6-level VIP)"; 
57, "NEXRAD",          //"Regional NEXRAD, Type 2 - 8 level"; 
58, "NEXRAD",          //"Regional NEXRAD, Type 3 - 16 level"; 
59, "NEXRAD",          //"Individual NEXRAD, Type 0 - low dynamic range"; 
60, "NEXRAD",          //"Individual NEXRAD, Type 1 - 8 level (quasi 6-level VIP)"; 
61, "NEXRAD",          //"Individual NEXRAD, Type 2 - 8 level"; 
62, "NEXRAD",          //"Individual NEXRAD, Type 3 - 16 level"; 
63, "NEXRAD Regional", //"Global Block Representation - Regional NEXRAD, Type 4 – 8 level"; 
64, "NEXRAD CONUS",    //"Global Block Representation - CONUS NEXRAD, Type 4 - 8 level"; 
81, "Tops",            //"Radar echo tops graphic, scheme 1,16-level"; 
82, "Tops",            //"Radar echo tops graphic, scheme 2,8-level"; 
83, "Tops",            //"Storm tops and velocity"; 
101, "Lightning",       //"Lightning strike type 1 (pixel level)"; 
102, "Lightning",       //"Lightning strike type 2 (grid element level)"; 
151, "Lightning",       //"Point phenomena, vector format"; 
201, "Surface",         //"Surface conditions/winter precipitation graphic"; 
202, "Surface",         //"Surface weather systems"; 
254, "G-AIRMET",        //"AIRMET, SIGMET,Bitmap encoding"; 
351, "Time",            //"System Time"; 
352, "Status",          //"Operational Status"; 
353, "Status",          //"Ground Station Status"; 
401, "Imagery",         //"Generic Raster Scan Data Product APDU Payload Format Type 1"; 
402, "Text",
403, "Vector Imagery", //"Generic Vector Data Product APDU Payload Format Type 1"; 
404, "Symbols",
405, "Text",
411, "Text",    //"Generic Textual Data Product APDU Payload Format Type 1"; 
412, "Symbols", //"Generic Symbolic Product APDU Payload Format Type 1"; 
413, "Text"    //"Generic Textual Data Product APDU Payload Format Type 2"; 
};
*/

AdsbWrapper::AdsbWrapper()
{
//	ClearAhrsData(mAhrsData);

//	ClearHeartbeatInfo(mLatetestHeartbeat);
//	ClearHeartbeatInfo(mTestHeartbeat);

	mLastDataIndex = -1;
	mLastCallsign = "none";

	CrcInit();

}

///////////////////////////////////////////////////////////////////////////////
void AdsbWrapper::ClearAhrsData(struct ahrsDataRec &data)
{
	data.msgId = 0;
	data.subMsgId = 0;
	data.roll = 0.0f;
	data.pitch = 0.0f;
	data.heading = 0.0f;
	data.headingIsTrue = false;
	data.indicatedAirspeed = 0.0f;
	data.trueAirspeed = 0.0f;
};


///////////////////////////////////////////////////////////////////////////////
void AdsbWrapper::ClearHeartbeatInfo(struct heartbeatMsgRec &msg)
{
	msg.msgId = 0;;
	msg.statusByte1.statusByte = 0;
	msg.statusByte2.statusByte = 0;
	msg.timestamp = 0;
	msg.msgCounts = 0;
	msg.crc = 0;

}

///////////////////////////////////////////////////////////////////////////////
void AdsbWrapper::ClearTrafficDataList()
{
	QList<struct trafficReportNumRec *> ::iterator infoIter;

	infoIter = mAircraftInfoList.begin();
	while (infoIter != mAircraftInfoList.end())
	{
		delete *infoIter;
		infoIter++;
	}

	mAircraftInfoList.clear();
}

///////////////////////////////////////////////////////////////////////////////
unsigned int AdsbWrapper::GetUint32(unsigned char *dataBuf)
{
	return((dataBuf[0] << 24) + (dataBuf[1] << 16) + (dataBuf[2] << 8) + dataBuf[3]);
}
///////////////////////////////////////////////////////////////////////////////
unsigned int AdsbWrapper::GetUint24(unsigned char *dataBuf)
{
	return((dataBuf[0] << 16) + (dataBuf[1] << 8) + dataBuf[0]);
}
///////////////////////////////////////////////////////////////////////////////
unsigned short AdsbWrapper::GetUint16(unsigned char *dataBuf)
{
	return((dataBuf[0] << 8) + dataBuf[1]);
}

///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::GetGeodeticLocation(unsigned char *dataBuf, double &location)
{
	int status = -1;

	unsigned int locFraction = dataBuf[0] << 16;

	locFraction += dataBuf[1] << 8;
	locFraction += dataBuf[2];

	locFraction = locFraction & 0x00ffffff;  // mask off the high byte

	if (locFraction > 0x7fffffff)
	{
		locFraction -= 0x01000000;
	}

	location = locFraction * GDL90_LAT_LONG_RES;

	return(status);
}

///////////////////////////////////////////////////////////////////////////////
// if filterData is set only save the latest data for a particular aircraft
///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::DecodeMessage(unsigned int msgSize, char *msgPtr, bool filterData)
{
	int status = -1;
	unsigned char msgId = 0;
	unsigned char subMsgId = 0;
	struct trafficReportNumRec *tempDataPtr = NULL;
	int timeOfReception = 0;
	int reportLen = 0;
	bool setOwnshipCallsign = false;

	unsigned char *msgBuf  = (unsigned char *)msgPtr;

	if (msgSize == 2)
	{
		status = status;
	}
	if (msgSize == 27)
	{
		status = status;
	}

	if ((msgSize > 0) && (msgBuf != 0))
	{
		if ((msgBuf[0] == GDL90_FLAGBYTE) && (msgBuf[msgSize - 1] == GDL90_FLAGBYTE))
		{
			// convert the flag bytes to zero so we can look for control escape

			msgBuf[0] = 0;
			msgBuf[msgSize - 1] = 0;

			bool checkStuff = true;
			QByteArray byteArray;

			//			byteArray.remove(dataIndex, 1);

			int srcIndex = 0;
			int shiftCnt = 0;
			int dstIndex = 0;

			bool shiftRemaining = false;

			while (srcIndex < msgSize)
			{
				if (msgBuf[srcIndex] == 0x7D)
				{
					shiftCnt++;

					msgBuf[dstIndex] = msgBuf[srcIndex] ^ 0x20;

					srcIndex++;

					shiftRemaining = true;
				}
				else
				{
					if (shiftRemaining == true)
					{
						msgBuf[dstIndex] = msgBuf[srcIndex];
					}
				}
				srcIndex++;
				dstIndex++;
			}

			msgId = msgBuf[1];

			mLastMsgType = msgId;

			// watch for control-escape fields used to allow 7d or 7f to be passed as data

			switch (msgId)
			{
			case  GDL90_ID_HEARTBEAT:
			{
				statusByteRec1 test;

				test.statusByte = 1;

				test.statusByte = 0x8;

				mLatetestHeartbeat.msgId = msgBuf[1];
				mLatetestHeartbeat.statusByte1.statusByte = msgBuf[2];
				mLatetestHeartbeat.statusByte2.statusByte = msgBuf[3];

				mLatetestHeartbeat.timestamp = (mLatetestHeartbeat.statusByte2.bits.timestamp << 16) + (msgBuf[5] << 8) + msgBuf[4];

				mLatetestHeartbeat.msgCounts = (msgBuf[6] << 8) + msgBuf[7];

				mLatetestHeartbeat.crc = (msgBuf[8] << 8) + msgBuf[9];
			}
			break;

			case  GDL90_ID_INIT:
				status = status;

				break;

			case  GDL90_ID_UPLINK_DATA:
			{
				// length expecected to be 436 bytes long
				// possibly 24 bytes of uplink msgType, UAT Heder, then data

				timeOfReception = (msgBuf[2] << 16) + (msgBuf[3] << 8) + msgBuf[4];

				reportLen = msgSize - 5;  // 5 becuase of the flag byte

				double latitude;
				double longitude;

				int raw_lat = (msgBuf[5] << 15) | (msgBuf[6] << 7) | (msgBuf[7] >> 1);
				latitude = raw_lat * GDL90_LAT_LONG_RES;

				int raw_lon = ((msgBuf[7] & 0x01) << 23) | (msgBuf[8] << 15) | (msgBuf[9] << 7) | (msgBuf[10] >> 1);
				longitude = raw_lon * GDL90_LAT_LONG_RES;

				latitude = (msgBuf[5] << 15) + (msgBuf[6] << 7) + ((msgBuf[7] >> 1) & 0x7f);
				latitude *= GDL90_LAT_LONG_RES;

				longitude = ((msgBuf[7] & 1) << 23) + (msgBuf[8] << 15) + (msgBuf[9] & 0xFE);



				longitude *= GDL90_LAT_LONG_RES;

				if (longitude > 180.0)
				{
					longitude = 360.0 - longitude;
				}

				bool validPos = (msgBuf[10] & 0x80) == 0x80;
				bool utcCoupled = (msgBuf[11] & 0x01) == 0x01;
				bool appDataValid = (msgBuf[11] & 0x04) == 0x04;

				bool position_valid = (msgBuf[10] & 0x01) ? 1 : 0;
				bool app_data_valid = (msgBuf[11] & 0x20) ? 1 : 0;
				int slot_id = (msgBuf[11] & 0x1f);
				int tisb_site_id = (msgBuf[12] >> 4);

				unsigned char slotId = msgBuf[11] & 0x1F;
				unsigned char tisbSiteId = (msgBuf[12] >> 4) & 0x0F;

				if (app_data_valid == true)
				{
#ifdef LATER
					status = ParseApplicationData(reportLen - 8, &msgBuf[13]);
#endif // LATER
				}
				status = status;
			}

			break;

			case  GDL90_ID_OWNSHIP:
				setOwnshipCallsign = true;
		// no break here fall through to GDL90_ID_TRAFFIC

			case  GDL90_ID_TRAFFIC:
			{
				struct trafficReportNumRec trafficData;
				unsigned int dataIndex = 0;

				status = DecodeTrafficMessage(msgSize, &msgBuf[1], trafficData);

				if (status == 0)
				{
					if (filterData == true)
					{
						tempDataPtr = GetAircraftInfo(trafficData.callsign, dataIndex);
					}

					if (tempDataPtr == NULL)
					{
						tempDataPtr = new struct trafficReportNumRec;

						mAircraftInfoList.push_back(tempDataPtr);

						dataIndex = mAircraftInfoList.size() - 1;

						if (setOwnshipCallsign == true)
						{
							setOwnshipCallsign = false;

							mOwnshipCallsign = trafficData.callsign;
						}
					}

					if (tempDataPtr != NULL)
					{
						CopyAircraftData(trafficData, *tempDataPtr);

						mLastCallsign = trafficData.callsign;

						mLastDataIndex = dataIndex;
					}
				}
			}
			break;


			case  GDL90_ID_STRATUX_HEARTBEAT0:
			{
				status = 0;

				int gpsValid = (msgBuf[2] & 0x02) >> 1;
				int ahrsValid = (msgBuf[2] & 0x01);
				int protVerValid = (msgBuf[2] & 0x04) >> 2;

				int protVer = msgBuf[3];
			}
			break;

			case  GDL90_ID_STRATUX_HEARTBEAT1:
				status = 0;

				break;


			case  GDL90_ID_STRATUX_AHRS:
			{
				subMsgId = msgBuf[2];

				mStatuxAhrsData.roll = ((msgBuf[3] << 8) + msgBuf[4]) * 0.1f;
				mStatuxAhrsData.pitch = ((msgBuf[5] << 8) + msgBuf[6]) * 0.1f;

				mStatuxAhrsData.heading = ((msgBuf[7] << 8) + msgBuf[8]) * 0.1f;

				//			mStatuxAhrsData.trueAirspeed = (msgBuf[9] << 8) + msgBuf[10];
			//				mStatuxAhrsData.indicatedAirspeed = (msgBuf[11] << 8) + msgBuf[12];

				float AHRSRoll = ((msgBuf[3] << 8) + msgBuf[4]) * 0.1f;
				float AHRSPitch = ((msgBuf[5] << 8) + msgBuf[6]) * 0.1f;
				float AHRSGyroHeading = ((msgBuf[7] << 8) + msgBuf[8]);
				float AHRSMagHeading = ((msgBuf[9] << 8) + msgBuf[10]);
				float AHRSSlipSkid = ((msgBuf[11] << 8) + msgBuf[12]);
				float AHRSTurnRate = ((msgBuf[13] << 8) + msgBuf[14]);
				float AHRSGLoad = ((msgBuf[5] << 8) + msgBuf[16]);
				float AHRSGLoadMin = ((msgBuf[17] << 8) + msgBuf[18]);
				float AHRSGLoadMax = ((msgBuf[19] << 8) + msgBuf[20]);

				status = status;
			}
			break;

			case 0x53:  // GDL90_ID_STRATUX_HEARTBEAT1  aka status message


				if (msgBuf[2] == 0x58)
				{
					mLastMsgType = 0x5358;

					mStratuxStatusMessage.msgVersion = msgBuf[4];

					memcpy(mStratuxStatusMessage.versionBuf, &msgBuf[5], 4);

					mStratuxStatusMessage.hardwareRevCode = GetUint32(&msgBuf[9]);

					mStratuxStatusMessage.validAndEnableFlags = GetUint16(&msgBuf[13]);
					mStratuxStatusMessage.connectHardwareFlags = GetUint16(&msgBuf[15]);

					mStratuxStatusMessage.numSatsLocked = msgBuf[17];
					mStratuxStatusMessage.numSatsConnected = msgBuf[18];

					mStratuxStatusMessage.num978Targets = GetUint16(&msgBuf[19]);

					mStratuxStatusMessage.num1090Targets = GetUint16(&msgBuf[21]);

					mStratuxStatusMessage.num978MsgRate = GetUint16(&msgBuf[23]);

					mStratuxStatusMessage.num1090MsgRate = GetUint16(&msgBuf[25]);

					mStratuxStatusMessage.cpuTemp = (float)GetUint16(&msgBuf[27])  * 0.1f;

					mStratuxStatusMessage.numAdsbTowers = msgBuf[29];

					double towerLat;
					double towerLon;

					int bufIndex = 30;
					for (int index = 0; index < mStratuxStatusMessage.numAdsbTowers; index++)
					{
						GetGeodeticLocation(&msgBuf[bufIndex], towerLat);

						bufIndex += 3;

						GetGeodeticLocation(&msgBuf[bufIndex], towerLon);

						bufIndex += 3;
					}

					// adsb tower locations would follow if we ever saw any

				}
				else
				{
					status = status;
				}
			break;

			case 0x65:  // ForeFront AHRS message  -- or GPS time -f 12 bytes see _messages.py _parseGpsTime
			{
				unsigned short tempShrt;

				if (msgBuf[2] == 0x1)  // AHRS message
				{
					struct ahrsMsgRec ahrsMsgBuf;

					//						memcpy(&ahrsMsgBuf, &msgBuf[1], sizeof(struct ahrsMsgRec));

					// bad values in pitch and roll needs more debugging

					tempShrt = GetUint16(&msgBuf[3]);
					mAhrsData.roll = (float)tempShrt * 0.1f;

					tempShrt = GetUint16(&msgBuf[5]);
					mAhrsData.pitch = (float)tempShrt * 0.1f;

					tempShrt = GetUint16(&msgBuf[7]);

					bool useTrueHeading = false;

					if (tempShrt != 0xffff)
					{
						mAhrsData.heading = tempShrt & 0x7ffff;


						if ((tempShrt & 0x8000) == 0x8000)
						{
							useTrueHeading = true;
						}
					}
					else
					{
						mAhrsData.heading = -99999.0;
					}

					mAhrsData.indicatedAirspeed = GetUint16(&msgBuf[9]);
					mAhrsData.trueAirspeed = GetUint16(&msgBuf[11]);

					/*
					_swab(&msgBuf[3], &msgBuf[3], 10);



					ushortPtr = (unsigned short *)&msgBuf[3];

					float tempRoll =  (float)*ushortPtr * 0.1f;

//						_swab(&msgBuf[1], &msgBuf[1], 2);
						ushortPtr = (unsigned short *)&msgBuf[5];
						mAhrsData.pitch = (float)*ushortPtr * 0.1f;

						ushortPtr = (unsigned short *)&msgBuf[7];

						if (*ushortPtr & 0x8000)
						{
							mAhrsData.headingIsTrue = false;
						}
						else
						{
							mAhrsData.headingIsTrue = true;
						}

						mAhrsData.heading = (float)(*ushortPtr & 0x7FFF) * 0.1f;

						ushortPtr = (unsigned short *)&msgBuf[9];
						mAhrsData.indicatedAirspeed = (float)*ushortPtr;

						ushortPtr = (unsigned short *)&msgBuf[11];
						mAhrsData.trueAirspeed = (float)*ushortPtr;
*/
				}
				else
				{
					if (msgBuf[2] == 0x0)
					{
						status = status;
					}
				}
			}
			break;

			case GDL90_ID_OWNSHIP_ALTITUDE:  // height above WGS-84 ellipsoid = MSL
			{
				// altitude is in increments of 5ft

				short altitude = GetUint16(&msgBuf[2]) * 5;
				short metrics = GetUint16(&msgBuf[4]);

				status = status;

			}
			break;

			case GDL90_ID_BASIC_REPORT:
			{
				// this should be 22 bytes

				timeOfReception = (msgBuf[2] << 16) + (msgBuf[3] << 8) + msgBuf[4];

				status = DecodePayloadHeader(&msgBuf[5]);


	// format types  0 - 15
	// 5 bits format type
	// 27 bits surv & comm control
	// 24 bits address

	// format types 16 - 21
	// 5 bits format type
	// 27 bits surv & comm control
	// 56 bits message field
	// 24 bits address

	// format type 22 - military use only 

	// format type 24
	// 5 bits format type
	// 6 bits surv & comm control
	// 80 bits message field
	// 24 bits address


				break;

			case GDL90_ID_LONG_REPORT:
			{
				// this should be 38 bytes

				timeOfReception = (msgBuf[2] << 16) + (msgBuf[3] << 8) + msgBuf[4];

				reportLen = msgSize - 5;  // 5 because of the flag byte

				status = DecodePayloadHeader(&msgBuf[5]);

				unsigned char format = msgBuf[5] & 0x1F;

//				DecodeAirPositionReport(&msgBuf[5]);

		// format types  0 - 15
		// 5 bits format type
		// 27 bits surv & comm control
		// 24 bits address

		// format types 16 - 21
		// 5 bits format type
		// 27 bits surv & comm control
		// 56 bits message field
		// 24 bits address

		// format type 22 - military use only 

		// format type 24
		// 5 bits format type
		// 6 bits surv & comm control
		// 80 bits message field
		// 24 bits address


			}
			break;

			default:
				status = status;
				break;
			}

			}
		}
	}
	return(status);
}

///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::DecodeTrafficMessage(unsigned int msgSize,  unsigned char *msgBuf,
									struct trafficReportNumRec &trafficData)
{
	int status = -1;

	ClearAircraftData(trafficData);

	if ((msgBuf[0] == GDL90_ID_OWNSHIP) || (msgBuf[0] == GDL90_ID_TRAFFIC))
	{
		trafficData.lastUpdate = mLatetestHeartbeat.timestamp;

		trafficData.addressType = (msgBuf[1] & 0xF);
		trafficData.alertStatus = (msgBuf[1] >> 4) & 0xF;
		
		trafficData.participantAddr = (msgBuf[2] << 16) + (msgBuf[3] << 8) + msgBuf[4];

		if (trafficData.participantAddr < 0x100)
		{
			status = status;
		}

		double location;

		GetGeodeticLocation(&msgBuf[5], trafficData.latitude);

		if (trafficData.latitude > 90.0)
		{
			trafficData.latitude -= 90.0;
		}

		GetGeodeticLocation(&msgBuf[8], trafficData.longitude);

		if (trafficData.longitude > 180.0)
		{
			trafficData.longitude -= 360.0;
		}

		int altitude = ((int32_t)(msgBuf[11])) << 4;
		altitude += (msgBuf[12] & 0xF0) >> 4;
		trafficData.altitude = (altitude * 25) - 1000;

		trafficData.miscIndicators = msgBuf[12] & 0xF;
		trafficData.integrityCode = (msgBuf[13] >> 4) & 0xF;
		trafficData.accuracyCode = msgBuf[13] & 0xF;

		int tempInt = 0;

		if (msgBuf[14] > 0)
		{
			status = status;
		}
// horiz velocity
		tempInt = ((msgBuf[14] << 4) + ((msgBuf[15] & 0xf0) >> 4));

		if (tempInt == 0xfff)
		{
			trafficData.horzVelocity = 4094;
		}
		else if (tempInt == 0xffe)
		{
			trafficData.horzVelocity = 4095;  // max value
		}
		else
		{
			trafficData.horzVelocity = tempInt;
		}

// vertical velocity
		tempInt = ((msgBuf[15] & 0x0f) << 8) + msgBuf[16];

		if (tempInt <= 0x1FD)
		{
			trafficData.vertVelocity = tempInt * 64;
		}
		else if ((tempInt | 0x800) == 0x800)
		{
			if (tempInt == 0x800)
			{
				trafficData.vertVelocity = 0;
			}
			else
			{
				int tempInt2 = tempInt & 0x7fff;

				trafficData.vertVelocity = tempInt2 * -64;
			}
		}

		trafficData.trackHeading = msgBuf[17] * 360.0f / 256.0f;
		trafficData.emitterCategory = msgBuf[18];

		char tempBuf[9];
		memcpy(tempBuf, &msgBuf[19], 8);
		tempBuf[8] = 0; // add EOS

		int tempIndex = 7;

		while ((tempIndex > 0) && (msgBuf[tempIndex] == ' '))
		{
			msgBuf[tempIndex] = 0;
			tempIndex--;
		}
		trafficData.callsign = tempBuf;

		trafficData.emergencyPriorityCode = (msgBuf[27] & 0xf0) >> 4;

		// 2 bytes CRC at end of message followed by 7E flag byte

		// need to add code to test crc and other data then set status;

		unsigned int msgCrc = (msgBuf[28] << 8) + msgBuf[29];

		unsigned int calcCrc = CrcCompute(&msgBuf[1], 28);

		unsigned int calcCrc2 = gen_crc16(&msgBuf[1], 28);

		status = 0;
	}

	return(status);
}


///////////////////////////////////////////////////////////////////////////////
void AdsbWrapper::CopyAircraftData(struct trafficReportNumRec &srcData,
	struct trafficReportNumRec&tgtData)
{
	tgtData.alertStatus = srcData.alertStatus;
	tgtData.addressType = srcData.addressType;
	tgtData.participantAddr = srcData.participantAddr;
	tgtData.latitude = srcData.latitude;
	tgtData.longitude = srcData.longitude;
	tgtData.altitude = srcData.altitude;
	tgtData.miscIndicators = srcData.miscIndicators;
	tgtData.integrityCode = srcData.integrityCode;
	tgtData.accuracyCode = srcData.accuracyCode;
	tgtData.horzVelocity = srcData.horzVelocity;
	tgtData.vertVelocity = srcData.vertVelocity;
	tgtData.trackHeading = srcData.trackHeading;
	tgtData.emitterCategory = srcData.emitterCategory;
	tgtData.callsign = srcData.callsign;

	tgtData.name = srcData.name;
	tgtData.nNumber = srcData.nNumber;
	tgtData.typeAircraft = srcData.typeAircraft;
	tgtData.typeEngine = srcData.typeEngine;

	tgtData.emergencyPriorityCode = srcData.emergencyPriorityCode;

	tgtData.lastUpdate = time(NULL);
}

///////////////////////////////////////////////////////////////////////////////
void AdsbWrapper::ClearAircraftData(struct trafficReportNumRec &tgtData)
{
	tgtData.alertStatus = 0;
	tgtData.addressType = 0;
	tgtData.participantAddr = 0;
	tgtData.latitude = 0.0;
	tgtData.longitude = 0.0;
	tgtData.miscIndicators = 0;
	tgtData.integrityCode = 0;
	tgtData.accuracyCode = 0;
	tgtData.horzVelocity = 0;
	tgtData.vertVelocity = 0;
	tgtData.trackHeading = 0;
	tgtData.emitterCategory = 0;
	tgtData.callsign.erase();
	tgtData.emergencyPriorityCode = 0;
	tgtData.lastUpdate = 0;

	tgtData.name.clear();
	tgtData.nNumber.clear();
	tgtData.typeAircraft = 0;
	tgtData.typeEngine = 0;

}

///////////////////////////////////////////////////////////////////////////////
struct AdsbWrapper::trafficReportNumRec *AdsbWrapper::GetAircraftInfo(
	std::string callsign, unsigned int &dataIndex)
{
	struct trafficReportNumRec *reportPtr = NULL;
	QList<struct trafficReportNumRec *> ::iterator infoIter;
	bool found = false;
	int tempIndex = 0;

	dataIndex = 0;
	infoIter = mAircraftInfoList.begin();
	while ((found != true) && (infoIter != mAircraftInfoList.end()))
	{
		if ((*infoIter)->callsign == callsign)
		{
			found = true;

			reportPtr = *infoIter;

			dataIndex = tempIndex;
		}
		else
		{
			infoIter++;

			tempIndex++;
		}
	}
	return(reportPtr);
}
///////////////////////////////////////////////////////////////////////////////
void AdsbWrapper::CrcInit(void)
{
	unsigned short i, bitctr, crc;
	for (i = 0; i < 256; i++)
	{
		crc = (i << 8);
		for (bitctr = 0; bitctr < 8; bitctr++)
		{
			crc = (crc << 1) ^ ((crc & 0x8000) ? 0x1021 : 0);
		}
		mCrc16Table[i] = crc;
	}
}
///////////////////////////////////////////////////////////////////////////////
unsigned int AdsbWrapper::CrcCompute( // Return – CRC of the block
	unsigned char *block, // i – Starting address of message
	unsigned int length // i – Length of message
	)
{
	unsigned int i;
	unsigned short crc = 0;

	for (i = 0; i < length; i++)
	{
		crc = mCrc16Table[crc >> 8] ^ (crc << 8) ^ block[i];
	}
	return crc;
}

///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::SerializeTrafficData(unsigned int dataIndex, char delimiter,
	std::string &serializedData)
{
	int status = -1;

	if (dataIndex < mAircraftInfoList.size())
	{
		struct trafficReportNumRec *dataPtr = mAircraftInfoList.at(dataIndex);

		if (dataPtr != NULL)
		{
			status = SerializeTrafficData(dataPtr, delimiter, serializedData);
		}
	}

	return(status);
}

///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::SerializeTrafficData(struct AdsbWrapper::trafficReportNumRec *dataPtr,
	char delimiter, std::string &serializedData)
{
	int status = -1;
	char dataBuf[512];

	if (dataPtr != NULL)
	{
		sprintf(dataBuf,
			"%d%c"  // timestamp
			"%s%c" // callsign
			"%d%c" // address type
			"%x%c" // participant address 
			"%s%c" // n number
			"%3.12lf%c" // latitude
			"%3.12lf%c" // longitude
			"%d%c"  // altitude
			"%d%c" // alert status 
			"%d%c" // misc indicator
			"%d%c" // integrity code
			"%d%c" // accuracy
			"%d%c" // horiz vel
			"%d%c" // vert velocity
			"%f%c" // track heading
			"%d%c" // emitter category
			"%d%c" // prioity code
			"%f%c" // range
			"%f\n",  // bearing
			mLatetestHeartbeat.timestamp, delimiter,
			dataPtr->callsign.c_str(), delimiter,
			dataPtr->addressType, delimiter,
			dataPtr->participantAddr, delimiter,
			dataPtr->nNumber.c_str(), delimiter,
			dataPtr->latitude, delimiter,
			dataPtr->longitude, delimiter,
			dataPtr->altitude, delimiter,
			dataPtr->alertStatus, delimiter,
			dataPtr->miscIndicators, delimiter,
			dataPtr->integrityCode, delimiter,
			dataPtr->accuracyCode, delimiter,
			dataPtr->horzVelocity, delimiter,
			dataPtr->vertVelocity, delimiter,
			dataPtr->trackHeading, delimiter,
			dataPtr->emitterCategory, delimiter,
			dataPtr->emergencyPriorityCode, delimiter,
			dataPtr->range, delimiter,
			dataPtr->bearing
		);

		serializedData = dataBuf;

		status = 0;
	}
	return(status);
}

///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::GetNumTrafficReports()
{
	return(mAircraftInfoList.size());
}

///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::GetLastMsgType()
{
	return(mLastMsgType);
}

///////////////////////////////////////////////////////////////////////////////
void AdsbWrapper::GetSatelliteCnt(int &numConnected, int &numLocked)
{
	numLocked = mStratuxStatusMessage.numSatsLocked;
	numConnected  = mStratuxStatusMessage.numSatsConnected;
}

///////////////////////////////////////////////////////////////////////////////
void AdsbWrapper::GetTargetCnt(int &num978Targets, int &num1090Targets)
{
	num978Targets = mStratuxStatusMessage.num978Targets;

	num1090Targets = mStratuxStatusMessage.num1090Targets;
}

///////////////////////////////////////////////////////////////////////////////
void AdsbWrapper::GetTowerCnt(int &numTowers)
{
	numTowers = mStratuxStatusMessage.numAdsbTowers;
}

///////////////////////////////////////////////////////////////////////////////
void AdsbWrapper::GetLastCallsign(std::string &callsign)
{
	callsign = mLastCallsign;
}

///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::GetLastDataIndex()
{
	return(mLastDataIndex);
}
///////////////////////////////////////////////////////////////////////////////
struct AdsbWrapper::trafficReportNumRec *AdsbWrapper::GetTrafficInfo(
							std::string callsign, unsigned int &dataIndex)
{
	bool found = false;
	struct trafficReportNumRec *acInfoPtr = NULL;
	QList<struct trafficReportNumRec *> ::iterator acIter;
	int tempIndex = 0;

	dataIndex = 0;

	acIter = mAircraftInfoList.begin();
	while ((found != true) && (acIter != mAircraftInfoList.end()))
	{
		if ((*acIter)->callsign == callsign)
		{
			found = true;

			acInfoPtr = *acIter;

			dataIndex = tempIndex;
		}
		else
		{
			acIter++;

			tempIndex++;
		}
	}

	return(acInfoPtr);
}
///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::GetParticipantAddress(std::string callsign, unsigned char &addrType,
										unsigned int &address)
{
	int status = -1;
	struct trafficReportNumRec *infoPtr;
	unsigned int dataIndex = 0;

	infoPtr = GetTrafficInfo(callsign, dataIndex);

	if (infoPtr != NULL)
	{
		addrType = infoPtr->addressType;
		address = infoPtr->participantAddr;

		status = 0;
	}
	return(status);
}
///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::GetLastUpdate(std::string callsign, unsigned int &updateTime)
{
	int status = -1;
	struct trafficReportNumRec *infoPtr;
	unsigned int dataIndex = 0;

	infoPtr = GetTrafficInfo(callsign, dataIndex);

	if (infoPtr != NULL)
	{
		updateTime = infoPtr->lastUpdate;

		status = 0;
	}
	return(status);
}

///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::GetLocation(std::string callsign,
	double &latitude, double &longitude, double &altitude)
{
	int status = -1;
	struct trafficReportNumRec *infoPtr;
	unsigned int dataIndex = 0;

	infoPtr = GetTrafficInfo(callsign, dataIndex);

	if (infoPtr != NULL)
	{
		latitude = infoPtr->latitude;
		longitude = infoPtr->longitude;
		altitude = infoPtr->altitude;
		status = 0;
	}
	return(status);
}

///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::GetHeading(std::string callsign, float &heading)
{
	int status = -1;
	struct trafficReportNumRec *infoPtr;
	unsigned int dataIndex = 0;

	infoPtr = GetTrafficInfo(callsign, dataIndex);

	if (infoPtr != NULL)
	{
		heading = infoPtr->trackHeading;

		status = 0;
	}
	return(status);
}
///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::GetHorzVelocity(std::string callsign, float& velocity)
{
	int status = -1;
	struct trafficReportNumRec *infoPtr;
	unsigned int dataIndex = 0;

	infoPtr = GetTrafficInfo(callsign, dataIndex);

	if (infoPtr != NULL)
	{
		velocity = infoPtr->horzVelocity;

		status = 0;
	}
	return(status);
}
///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::GetVertVelocity(std::string callsign, float &velocity)
{
	int status = -1;
	struct trafficReportNumRec *infoPtr;
	unsigned int dataIndex = 0;

	infoPtr = GetTrafficInfo(callsign, dataIndex);

	if (infoPtr != NULL)
	{
		velocity = infoPtr->vertVelocity;

		status = 0;
	}
	return(status);
}

///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::GetAircraftOwnerInfo(std::string callsign, unsigned int &address,
	std::string &nNumber, std::string &name, int &typeAircraft, int &typeEngine)
{
	int status = -1;
	struct trafficReportNumRec *infoPtr;
	unsigned int dataIndex = 0;

	infoPtr = GetTrafficInfo(callsign, dataIndex);

	if (infoPtr != NULL)
	{
		address = infoPtr->address;
		nNumber = infoPtr->nNumber;
		name = infoPtr->name;
		typeAircraft = infoPtr->typeAircraft;
		typeEngine = infoPtr->typeEngine;

		status = 0;
	}
	return(status);
}
///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::SetAircraftOwnerInfo(std::string callsign, unsigned int address,
	std::string nNumber, std::string name, int typeAircraft, int typeEngine)
{
	int status = -1;
	struct trafficReportNumRec *infoPtr;
	unsigned int dataIndex = 0;

	infoPtr = GetTrafficInfo(callsign, dataIndex);

	if (infoPtr != NULL)
	{
		infoPtr->address = address;
		infoPtr->nNumber = nNumber;
		infoPtr->name = name;
		infoPtr->typeAircraft = typeAircraft;
		infoPtr->typeEngine = typeEngine;

		status = 0;
	}
	return(status);
}

///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::SetRangeValues(std::string callsign, float range, float bearing)
{
	int status = -1;
	struct trafficReportNumRec *infoPtr;
	unsigned int dataIndex = 0;

	infoPtr = GetTrafficInfo(callsign, dataIndex);

	if (infoPtr != NULL)
	{
		infoPtr->range = range;
		infoPtr->bearing = bearing;

		status = 0;
	}
	return(status);
}
///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::GetRangeValues(std::string callsign, float &range, float &bearing)
{
	int status = -1;
	struct trafficReportNumRec *infoPtr;
	unsigned int dataIndex = 0;

	infoPtr = GetTrafficInfo(callsign, dataIndex);

	if (infoPtr != NULL)
	{
		range = infoPtr->range;
		bearing = infoPtr->bearing;

		status = 0;

	}
	return(status);
}
///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::ParseApplicationData(int appDataLen, unsigned char *msgBuf)
{
	int status = -1;
	int dataIndex = 0;

	FILE *fdes;
	char filename[60];

	sprintf(filename, "appDataDump_%Id.txt", time(NULL));

	fdes = fopen(filename, "wb");

if (fdes != NULL)
{
	fwrite(msgBuf, 1, appDataLen, fdes);

	fclose(fdes);
}

	if ((appDataLen > 0) && (msgBuf != NULL))
	{
		int iFrameLen = (msgBuf[dataIndex] << 1) + ((msgBuf[dataIndex + 1] >> 7) & 0x01);
		int frameType = msgBuf[dataIndex + 1] & 0x0F;

		while ((dataIndex < appDataLen))
		{
			if ((msgBuf[dataIndex] == 0xff) && (msgBuf[dataIndex + 1] == 0xfe))
			{
				status = status;
			}
			dataIndex += iFrameLen;
			
			iFrameLen = (msgBuf[dataIndex] << 1) + ((msgBuf[dataIndex + 1] >> 7) & 0x01);
			frameType = msgBuf[dataIndex + 1] & 0x0F;

//			dataIndex++;
		}
	}
	return(status);
}

///////////////////////////////////////////////////////////////////////////////
unsigned int AdsbWrapper::GetLatestTimestamp()
{
	return(mLatetestHeartbeat.timestamp);
}

///////////////////////////////////////////////////////////////////////////////
void AdsbWrapper::GetOwnshipCallsign(std::string &callsign)
{
	callsign = mOwnshipCallsign;
}
///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::DecodeAirPositionReport(unsigned char *msgBuf)
{
	int status = -1;

	unsigned char survStatus = (msgBuf[0] >> 5) & 0x03;
	unsigned char nicSup = (msgBuf[0] >> 7) & 0x01;
	int alt = (msgBuf[1] << 4) + ((msgBuf[2] >> 4) & 0x0F);
	unsigned char timeVal = (msgBuf[2] >> 5) & 0x01;
	unsigned char cprOdd = (msgBuf[2] >> 6) & 0x01;
	unsigned int latCpr = (((msgBuf[2] >> 7) & 0x03) << 17) + (msgBuf[3] << 16) + ((msgBuf[4] >> 1) & 0x7f);
	unsigned int lonCpr = (((msgBuf[4] >> 7) & 0x01) << 17) + (msgBuf[5] << 16) + (msgBuf[6] << 8) + msgBuf[7];

	return(status);
}
///////////////////////////////////////////////////////////////////////////////

static char base40_alphabet[40] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::DecodeCallsign(unsigned char *msgBuf,
			unsigned char &emitterCategory, std::string &callsign)
{
	int status = 0;

	if (msgBuf != NULL)
	{
	char localBuf[9];

	unsigned short value = (msgBuf[0] << 8) | (msgBuf[1]);

	emitterCategory = (value / 1600) % 40;

	localBuf[0] = base40_alphabet[(value / 40) % 40];
	localBuf[1] = base40_alphabet[value % 40];
	
	value = (msgBuf[2] << 8) | (msgBuf[3]);
	localBuf[2] = base40_alphabet[(value / 1600) % 40];
	localBuf[3] = base40_alphabet[(value / 40) % 40];
	localBuf[4] = base40_alphabet[value % 40];
	
	value = (msgBuf[4] << 8) | (msgBuf[5]);
	localBuf[5] = base40_alphabet[(value / 1600) % 40];
	localBuf[6] = base40_alphabet[(value / 40) % 40];
	localBuf[7] = base40_alphabet[value % 40];
	localBuf[8] = 0;

	callsign = localBuf;

	}

	return(status);
}

///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::DecodePayloadHeader(unsigned char *msgBuf)
{
	int status = -1;

	if (msgBuf != NULL)
	{
		int addressQualifier = msgBuf[0] & 0x07;

		if (addressQualifier != 0)
		{
			switch (addressQualifier)
			{
				
				case 0:
					// 0 ADS - B target with ICAO 24 - bit address 3.2.1.5.1.3.1				

					status = status;
				break;
			
				case 1:
					// 1 ADS - B target with self - assigned temporary address 3.2.1.5.1.3.2

					status = status;
				break;
			
				case 2:
					//2 TIS - B target with ICAO 24 - bit address 3.2.1.5.1.3.3
					status = status;
				break;
			
				case 3:
					// 3 TIS - B target with track file identifier 3.2.1.5.1.3.4
					status = status;
				break;

				case 4:
					// 4 Surface Vehicle 3.2.1.5.1.3.5
					status = status;
				break;
			
				case 5:
					// 5 Fixed ADS - B Beacon 3.2.1.5.1.3.6
					status = status;
				break;

				case 6:
				case 7:
					// 6 (Reserved)
					// 7 (Reserved)
				break;

				default:
					status = status;
				break;
			}
		}

		int payloadTypeCode = (msgBuf[0] >> 3) & 0x1f;

		unsigned int address = (msgBuf[1] << 16) + (msgBuf[2] << 8) + msgBuf[3];

		if ((payloadTypeCode <= 10) && ((addressQualifier == 0) ||
			(addressQualifier == 1) || (addressQualifier == 4) || (addressQualifier == 5)))
		{
			status = DecodeStateVector(&msgBuf[4]);
		}
		else
		{
			status = status;
		}

		if ((payloadTypeCode == 1) || (payloadTypeCode == 3))
		{
			// decode Mode State message and Aux SV message -- refer to table 2-8 for rest of the payload types

			DecodeModeStatus(&msgBuf[17]);
		}

		if ((payloadTypeCode == 1) || (payloadTypeCode == 2) || (payloadTypeCode == 5) || (payloadTypeCode == 6))
		{
			DecodeAuxiliaryStateVector(&msgBuf[29]);
		}

		if ((payloadTypeCode == 3) || (payloadTypeCode == 4))
		{
			DecodeTargetState(&msgBuf[29]);
		}

		if (payloadTypeCode == 6)
		{
			DecodeTargetState(&msgBuf[24]);
		}

		status = status;

	}

	return(status);
}

///////////////////////////////////////////////////////////////////////////////
// used the following doc to decode this
// UAT-SWG02-WP04 - Draft Tech Manual V0-1.pdf
// and libmodes-master\src\mode-s.c
// UAT State vector
///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::DecodeStateVector(unsigned char *msgBuf)
{
	int status = -1;

	if (msgBuf != NULL)
	{
		unsigned int latValue = (msgBuf[0] << 15) + (msgBuf[1] << 7) + ((msgBuf[2] >> 1) & 0x7F);
		double latitude = latValue * GDL90_LAT_LONG_RES;

		if (latitude > 90.0)
		{
			latitude -= 90.0;
		}

		unsigned lonValue = ((msgBuf[2] & 0x01) << 23) + (msgBuf[3] << 15) + (msgBuf[4] << 7) + ((msgBuf[5] >> 1) & 0x7f);
		double longitude = lonValue * GDL90_LAT_LONG_RES;

		if (longitude > 180.0f)
		{
			longitude -= 360.0;
		}

		unsigned char altType = msgBuf[5] & 0x01;  // 0 = Barametric Preasure Alt, 1 = geometric Alt.

// altitude based on 12 bit code
		unsigned int altValue = (msgBuf[6] << 4) + ((msgBuf[7] >> 4) & 0x0f);

		unsigned int altitude = (altValue * 25) - 1000;

		unsigned char nic = msgBuf[7] & 0x0f;

	// agState - 00 airborne subsonic, 01 = airborne supersonic,  10 on ground, 11 reserved
		unsigned char agState = (msgBuf[8] >> 6) & 0x03;

		bool southVelocity = false;

		int northHorzVel = ((msgBuf[8] & 0x1F) << 6) + (msgBuf[9] >> 2);

		if (((northHorzVel >> 11) & 0x01) == 0x01)
		{
			southVelocity = true;
		}

		int eastHorzVel = ((msgBuf[9] & 0x03) << 9) + (msgBuf[10] << 1) + ((msgBuf[11] >> 7) & 0x01);

		bool westVelocity = false;

		if (((eastHorzVel >> 11) & 0x01) == 0x01)
		{
			westVelocity = true;
		}

		int vertVel = ((msgBuf[11] & 0x7F) << 4) + ((msgBuf[12] >> 4) & 0x0F);

		unsigned char utc = (msgBuf[12] >> 3) & 0x01;

		status = status;
	}
	return(status);
}

///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::DecodeModeStatus(unsigned char *msgBuf)
{
	int status = -1;
	std::string callsign;
	unsigned char emitterCategory;

	// callsign uses bytes 0 -> 5

	DecodeCallsign(msgBuf, emitterCategory, callsign);

	unsigned char priorityStatus = (msgBuf[6] >> 5) & 0x07;  // 3 MSB bits
	unsigned char mopsVersion = ((msgBuf[6] >> 2) & 0x07);  // next 3 bits
	unsigned char sil = msgBuf[6] & 0x03;  // the 2 LSBs

	unsigned char transmitMso = (msgBuf[7] >> 2) & 0x3F;  // high 6 bits

	unsigned char nacp = (msgBuf[8] >> 4) & 0x0F;  // 4 MSBs
	unsigned char nacv = (msgBuf[8] >> 1) & 0x07;
	unsigned char nicBaro = msgBuf[8] & 0x01;

	unsigned char capabilityCodes = (msgBuf[9] >> 6) & 0x03; // 2 MSBs
	unsigned char opModes = (msgBuf[9] >> 3) & 0x07;  // next 3 bits
	unsigned char trueMag = (msgBuf[9] >> 2) & 0x01;
	unsigned char csid = (msgBuf[9] >> 1) & 0x01;
	// last bit is reserved

	// last 2 bytes are reserved


	return(status);
}

///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::DecodeCapabilityCodes(unsigned char *msgBuf)
{
	int status = -1;

	return(status);
}
///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::DecodeAuxiliaryStateVector(unsigned char *msgBuf)
{
	int status = -1;

	if (msgBuf != NULL)
	{
		unsigned int altValue = (msgBuf[0] << 4) + ((msgBuf[1] >> 4) & 0x0F);

		unsigned int altitude = (altValue * 25) - 1000;

		status = 0;
	}

	return(status);
}
///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::DecodeTargetState(unsigned char *msgBuf)
{
	int status = -1;

	return(status);
}

///////////////////////////////////////////////////////////////////////////////
int AdsbWrapper::DecodeTrajectoryChange(unsigned char *msgBuf)
{
	int status = -1;

	return(status);
}
