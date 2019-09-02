#ifndef ADSBWRAPPER_H
#define ADSBWRAPPER_H

#include <qlist.h>
#include <map>

//#include "adsbwrapper_global.h"
//class ADSBWRAPPERSHARED_EXPORT AdsbWrapper

#define GDL90_FLAGBYTE 0x7E
#define GDL90_ESCAPEBYTE 0x7D

#define GDL90_ID_HEARTBEAT 0x00
#define GDL90_ID_HEARTBEAT_SIZE 0x09

#define GDL90_ID_INIT 0x02
#define GDL90_ID_INIT_SIZE 0x03

#define GDL90_ID_UPLINK_DATA 0x07
#define GDL90_ID_UPLINK_DATA_SIZE 0x01B6 /* 438 bytes */

#define GDL90_ID_HEIGHT_AGL 0x09

#define GDL90_ID_OWNSHIP 0x0A  // decimal 10
#define GDL90_ID_OWNSHIP_SIZE 0x1E /* 30 bytes */  // spec states 28 bytes

#define GDL90_ID_OWNSHIP_ALTITUDE 0x0B  // decimal 11

#define GDL90_ID_BASIC_REPORT 0x1E // Basic UAT report
#define GDL90_ID_LONG_REPORT 0x1F // Long report

#define GDL90_ID_TRAFFIC 0x14  // decimal 20
#define GDL90_ID_TRAFFIC_SIZE 0x1E /* 30 bytes */ // spec states 28 bytes

#define GDL90_ID_BASIC_REPORT 0x1E
#define GDL90_ID_LONG_REPORT 0x1F

#define GDL90_ID_STRATUX_HEARTBEAT0 0xCC
#define GDL90_ID_STRATUX_HEARTBEAT1 0x5358

#define GDL90_ID_STRATUX_AHRS 0x4C
#define GDL90_ID_STRATUX_AHRS_SIZE 0x1A /* 26 bytes */

#define GDL90_ID_GPS_TIME 0x65 // 101 decimal
#define FOREFRONT_AHRS 0x65 // 101 decimal

class AdsbWrapper
{

#define GDL90_LAT_LONG_RES (180.0f / 8388608.0f)
	/*
	message types
	  heartbeat
	  initialization
	  uplink data
	  ownship report
	  traffic report
	  pass through report
	  height above terrain
	  ownship geometric altitude
	*/

	enum emergencyOrPriorityCodeKinds
	{
		noEmergency,
		generalEmergency,
		medicalEmergency,
		minimumFuel,
		noCommunication,
		unlawfulInterference,
		downedAircraft
	};

	enum addressKinds
	{
		adsbWithIcaoAddress,
		adsbWithSelfAssignedAddress,
		tisbWithIcaoAddress,
		tisbWithTrackFileId,
		surfaceVehicle,
		groundStationBeacon
	};

	struct statusBitsRec1
	{
		unsigned uatInitialized : 1;
		unsigned reserved : 1;
		unsigned statRatcs : 1;
		unsigned statGpsBattLow : 1;
		unsigned stataddrType : 1;
		unsigned statIdent : 1;
		unsigned statMaintReqd : 1;
		unsigned statGpsPosValid : 1;
	};

	enum emitterCategoryKinds
	{
		noAircraftTypeInformation = 0,

		// ICAO < 15,500 lbs
		light = 1,

		// [15000 lbs, 75000 lbs]
		small = 2,

		// [75000 lbs, 300000 lbs]
		large = 3,

		// Aircraft such as a B757
		highVortexLarge = 4,

		// > 300,000 lbs
		heavy = 5,

		// > 5g acceleration and high speed
		highlyManeuverable = 6,

		rotorcraft = 7,
		gliderOrSailplane = 9,
		lighterThanAir = 10,
		parachutistOrSkyDiver = 11,
		ultraLightOrHangGliderOrParaGlider = 12,
		uav = 14,
		space = 15,
		surfaceEmergencyVehicle = 17,
		surfaceServiceVehicle = 18,
		pointObstacle = 19,
		clusterObstacle = 20,
		lineObstacle = 21
	};

	enum trafficAlertStatusKind
	{
		noAlert,
		trafficAlert
	};

	union statusByteRec1
	{
		struct statusBitsRec1 bits;
		unsigned char statusByte;
	};

	struct statusBitsRec2
	{
		unsigned utcOk : 1;
		unsigned reserved4 : 1;
		unsigned reserved3 : 1;
		unsigned reserved2 : 1;
		unsigned reserved1 : 1;
		unsigned csaNotAvail : 1;
		unsigned csaRequested : 1;
		unsigned timestamp : 1;
	};

	union statusByteRec2
	{
		struct statusBitsRec2 bits;
		unsigned char statusByte;
	};

	struct heartbeatMsgRec
	{
		unsigned char msgId;
		statusByteRec1 statusByte1;
		statusByteRec2 statusByte2;
		unsigned int timestamp;
		unsigned short msgCounts;
		unsigned short crc;
	};

/*
	struct trafficReportRec
	{
		unsigned alertStatus : 4;
		unsigned addressType : 4;
		unsigned char participantAddr[3];
		unsigned char latFraction[3];
		unsigned char lonFraction[3];
		unsigned char altFraction[3];
		unsigned miscIndicators : 4;
		unsigned integrityCode : 4;
		unsigned accuracyCode : 4;
		unsigned horzVelocity : 12;
		unsigned vertVelocity : 12;
		unsigned char trackHeading;
		unsigned char emitterCategory;  // what size
		char callsign[8];
		unsigned char emergencyPriorityCode;  // what size
		unsigned char spare;  // what size
		unsigned char crc[2];
	};
*/
	struct trafficReportNumRec
	{
		unsigned char alertStatus;
		unsigned char addressType;
		unsigned int participantAddr;
//		unsigned int latFraction;
//		unsigned int lonFraction;
		double latitude;
		double longitude;
//		unsigned int altFraction;
		int altitude;
		unsigned char miscIndicators;
		unsigned char integrityCode;
		unsigned char accuracyCode;
		int horzVelocity;
		int vertVelocity;
		float trackHeading;
		unsigned char emitterCategory;  // what size
		std::string callsign;
		unsigned char emergencyPriorityCode;  // what size
		unsigned char crc[2];

		int address;
		std::string nNumber;
		std::string name;
		int typeAircraft;
		int typeEngine;

		float range;
		float bearing;
		time_t lastUpdate;
	};


	struct ahrsMsgRec
	{
		unsigned char msgId;
		unsigned char subMsgId;
		unsigned char roll[2];
		unsigned char pitch[2];
		unsigned char heading[2];
		unsigned char indicatedAirspeed[2];
		unsigned char trueAirspeed[2];
	};
	struct ahrsDataRec
	{
		unsigned char msgId;
		unsigned char subMsgId;
		float roll;
		float pitch;
		float heading;
		bool headingIsTrue;
		unsigned short indicatedAirspeed;
		unsigned short trueAirspeed;
	};

	struct stratuxStatusMsgRec
	{
		unsigned char msgVersion;
		unsigned char versionBuf[4];
		unsigned int hardwareRevCode;
		unsigned short validAndEnableFlags;
		unsigned short connectHardwareFlags;
		unsigned char numSatsLocked;
		unsigned char numSatsConnected;
		unsigned short num978Targets;
		unsigned short num1090Targets;
		unsigned short num978MsgRate;
		unsigned short num1090MsgRate;
		float cpuTemp;
		unsigned char numAdsbTowers;
	};

public:
	AdsbWrapper();

	void CrcInit(void);
	unsigned int CrcCompute(unsigned char *block, unsigned int length);
	unsigned int CalculateCrc(unsigned char *msgBuf, int msgSize);

	void ClearAhrsData(struct ahrsDataRec &data);
	void ClearAircraftData(struct trafficReportNumRec &srcData);
	void ClearHeartbeatInfo(struct heartbeatMsgRec &msg);
	void ClearTrafficDataList();

	void CopyAircraftData(struct trafficReportNumRec &srcData, struct trafficReportNumRec&tgtData);

	unsigned int GetUint32(unsigned char *dataBuf);
	unsigned int GetUint24(unsigned char *dataBuf);
	unsigned short GetUint16(unsigned char *dataBuf);

	int DecodeMessage(unsigned int msgSize, char *msgBuf, bool filterData = true);
	int GetGeodeticLocation(unsigned char *dataBuf, double &location);

	int DecodeTrafficMessage(unsigned int msgSize, unsigned char *msgBuf, struct trafficReportNumRec &trafficData);
	
	int DecodeAirPositionReport(unsigned char *msgBuf);
	int DecodeCallsign(unsigned char *msgBuf, 
					unsigned char &emitterCategory, std::string &callsign);

	int DecodeModeS0(unsigned char *msgBuf);
	int DecodeModeS16(unsigned char *msgBuf);

	int DecodePayloadHeader(unsigned char *msgBuf);
	int DecodeStateVector(unsigned char *msgBuf);

	int DecodeModeStatus(unsigned char *msgBuf);
	int DecodeCapabilityCodes(unsigned char *msgBuf);
	int DecodeAuxiliaryStateVector(unsigned char *msgBuf);
	int DecodeTargetState(unsigned char *msgBuf);
	int DecodeTrajectoryChange(unsigned char *msgBuf);

	int GetLastMsgType();
	void GetLastCallsign(std::string &callsign);

	int GetNumTrafficReports();

	int GetLocation(std:: string callsign,
		double &latitude, double &longitude, double &altitude);

	int GetHeading(std::string callsign, float &heading);
	int GetHorzVelocity(std::string callsign, float &velocity);
	int GetVertVelocity(std::string callsign, float &velocity);

	int GetAircraftOwnerInfo(std::string callsign, unsigned int &address,
		std::string &nNumber, std::string &name, int &typeAircraft, int &typeEngine);

	int SetAircraftOwnerInfo(std::string callsign, unsigned int address,
		std::string nNumber, std::string name, int typeAircraft, int typeEngine);

	int GetLastUpdate(std::string callsign, unsigned int &updateTime);

	int GetParticipantAddress(std::string callsign, unsigned char &addrType,
		unsigned int &address);

	int GetLastDataIndex();
	unsigned int GetLatestTimestamp();

	void GetSatelliteCnt(int &numConnected, int &numLocked);
	void GetTargetCnt(int &num978Targets, int &num1090Targets);
	void GetTowerCnt(int &numTowers);

	void GetOwnshipCallsign(std::string &callsign);

	int ParseApplicationData(int appDataLen, unsigned char *appData);

	int SerializeTrafficData(unsigned int dataIndex, char delimiter,
		std::string &serializedData);

	int SerializeTrafficData(struct trafficReportNumRec *dataPtr,
		char delimiter, std::string &serializedData);

	int SetRangeValues(std::string callsign, float range, float bearing);
	int GetRangeValues(std::string callsign, float &range, float &bearing);

protected:
	struct trafficReportNumRec *GetAircraftInfo(std::string callsign, unsigned int &dataIndex);
	struct trafficReportNumRec *GetTrafficInfo(std::string callsign, unsigned int &dataIndex);


private:
	struct heartbeatMsgRec mLatetestHeartbeat;
	struct heartbeatMsgRec mTestHeartbeat;

	struct ahrsDataRec mAhrsData;
	struct ahrsDataRec mStatuxAhrsData;

	struct trafficReportNumRec mOwnshipData;

	QList<struct trafficReportNumRec *> mAircraftInfoList;

	struct stratuxStatusMsgRec mStratuxStatusMessage;

	int mLastMsgType;
	std::string mLastCallsign;

	unsigned short mCrc16Table[256];

//	std::map<int, std::string> mFisbProductNameMap;

	int mLastDataIndex;

	std::string mOwnshipCallsign;
};

#endif // ADSBWRAPPER_H