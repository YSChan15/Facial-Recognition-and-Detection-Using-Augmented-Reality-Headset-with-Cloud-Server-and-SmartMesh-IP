#ifndef __API_H
#define __API_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "SERIALCOM.h"

// API Packet Parsing Defines
#define CONTROL_ESCAPE 	0x7D
#define FLAG_SEQ		0x7E
#define ETX				0x03
#define	XON				0x11
#define XOFF			0x13
#define	XON_PARITY		0x91
#define XOFF_PARITY		0x93

#define GET_BYTE(x) x ^ (1 << 5)

// API Inverted Parsing Defines
#define ICONTROL_ESCAPE 0x5D
#define IFLAG_SEQ		0x5E
#define IETX			0x23
#define	IXON			0x31
#define IXOFF			0x33
#define	IXON_PARITY		0xB1
#define IXOFF_PARITY	0xB3

// API and firmware versions
#define API_APP_VER 4
#define SW_VER 143// (1.43)

// Command Defines
#define USR_HELLO 0x01
#define MGR_HELLO_RESPONSE 0x02
#define MGR_HELLO 0x03
#define NOTIF 0x14
#define RESET_CMD 0x15
#define SUBSCRIBE 0x16
#define GET_TIME 0x17
#define SET_NTWK_CONFIG 0x1A
#define CLEAR_STAT 0x1F
#define EXCH_MOTE_JKEY 0x21
#define EXCH_NETID 0x22
#define RADIO_TEST_TX 0x23
#define RADIO_TEST_RX 0x25
#define GET_RADIO_TEST_STAT 0x26
#define SET_ACL_ENTRY 0x27
#define GET_NEXT_ACL_ENTRY 0x28
#define DELETE_ACL_ENTRY 0x29
#define PING_MOTE 0x2A
#define GET_LOG 0x2B
#define SEND_DATA 0x2C
#define START_NETWORK 0x2D
#define GET_SYS_INFO 0x2E
#define GET_MOTE_CONFIG 0x2F
#define GET_PATH_INFO 0x30
#define GET_NEXT_PATH_INFO 0x31
#define SET_ADVERTISING 0x32
#define SETz_DOWNSTREAM_FRAME 0x33
#define GET_MANAGER_STAT 0x35
#define SET_TIME 0x36
#define GET_LICENSE 0x37
#define SET_LICENSE 0x38
#define SET_CLI_USER 0x3A
#define SEND_IP 0x3B
#define RESTORE_FACTORY 0x3D
#define GET_MOTE_INFO 0x3E
#define GET_NET_CONFIG 0x3F
#define GET_NETWORK_INFO 0x40
#define GET_MOTE_CFG_BY_ID 0x41
#define SET_COMMON_JKEY 0x42
#define GET_IP_CONFIG 0x43
#define SET_IP_CONFIG 0x44
#define DELETE_MOTE 0x45
#define GET_MOTE_LINKS 0x46

// Notification Types
#define NOTIF_EVENT 0x1
#define NOTIF_LOG 0x2
#define NOTIF_DATA 0x4
#define NOTIF_IPDATA 0x5
#define NOTIF_HEALTH 0x6

// Subscription filters
#define SUB_EVENT 0x02
#define SUB_LOG 0x04
#define SUB_DATA 0x10
#define SUB_IPDATA 0x20
#define SUB_HEALTH 0x40

// Event Types
#define MOTE_RESET 0x00
#define NETWQRK_RESET 0x01
#define COMMAND_FINISHED 0x02
#define MOTE_JOIN 0x03
#define MOTE_OPERATIONAL 0x04
#define MOTE_LOST 0x5
#define NETWORK_TIME 0x06
#define PING_RESPONSE 0x07
#define PATH_CREATE 0x0A
#define PATH_DELETE 0x0B
#define PACKET_SENT 0x0C
#define MOTE_CREATE 0x0D
#define MOTE_DELETE 0x0E
#define JOIN_FAILED 0x0F
#define INVALID_MIC 0x20

// Response Codes
#define RC_OK 0x00
#define RC_INVALID_COMMAND 0x01
#define RC_INVALID_ARGUMENT 0x02
#define RC_END_OF_LIST 0x0B
#define RC_NO_RESOURCES 0x0C
#define RC_IN_PROGRESS 0x0D
#define RC_NACK 0x0E
#define RC_WRITE_FAIL 0x0F
#define RC_VALIDATION_ERROR 0x10
#define RC_INV_STATE 0x11
#define RC_NOT_FOUND 0x12
#define RC_UNSUPPORTED 0x13

// Packet priority
#define PACKET_PRIORITY_LOW 0x0
#define PACKET_PRIORITY_MED 0x1
#define PACKET_PRIORITY_HIGH 0x2

#define CMD_FAIL 0x0
#define CMD_SUCCESS 0x1

struct command_params{
  int commandType;
  uint8_t error_code;
  uint64_t time;// Since 1970
  uint8_t ipv6_addr[16];// IPV6 address of mote
  uint8_t mac_addr[8];// Mote MAC address
  uint8_t payload[128];// Payload recieved => maximum size is 128 bytes
};

struct send_params{
  uint8_t commandType;
  uint8_t macAddr;
  uint8_t *data;// Optional data to transmit => if sending packet
  uint8_t *payload;// Payload for manager packet
  uint32_t sub_filter;
  uint32_t unpack_filter;
};

// Data notification strcuture
typedef struct{
	uint64_t seconds;
	uint32_t us;
	uint64_t macAddr;
	uint16_t src;
	uint16_t dest;
	uint8_t data[90];
	uint8_t length;
}data_notif;

// Network Information Structure
typedef struct{
	uint8_t rc_code;// 0 = good
	uint16_t num_motes;// Total motes in system
	uint16_t asnSize;
	uint8_t adv_state;
	uint8_t downStreamFrame;
	uint8_t network_reliability;// Percentage
	uint8_t netPathStability;// Percentage
	uint32_t latency;// In ms
	uint8_t netState;
	uint64_t ipv6AddrHigh;
	uint64_t ipv6AddrLow;
	#if SW_VER >= 130// In version 1.3(130) and later
	uint32_t numLostPackets;
	uint64_t numArrivedPackets;
	uint8_t maxHops;
	#endif
}network_info;

// Hardware Information Structure
typedef struct{
	uint8_t rc_code;
	uint64_t mac_address;
	uint8_t hw_model;
	uint8_t hw_rev;
	uint8_t sw_major;
	uint8_t sw_minor;
	uint8_t sw_patch;
	uint16_t sw_build;
}system_info;

// Mote info structure
typedef struct{
	uint8_t rc_code;
	uint64_t mac_address;
	uint8_t state;
	uint8_t numNbrs;// Number of motes that could possibly be connected to this mote
	uint8_t numGoodNbrs;// Number of motes that have a good connection to this mote
	uint32_t requestedBw;
	uint32_t totalNeededBw;
	uint32_t assignedBw;
	uint32_t packetsReceived;
	uint32_t packetsLost;
	uint32_t avgLatency;
	uint32_t stateTime;
	uint8_t numJoins;
	uint8_t hopDepth;// Value is multiplied by 10
}mote_info;

// Mote configuration from mote ID structure
typedef struct{
	uint8_t rc_code;
	uint64_t mac_address;
	uint16_t mote_id;
	uint8_t isAP;
	uint8_t state;
	uint8_t reserved;
	uint8_t routing;
}mote_config_from_id;

// Network configuration struct
typedef struct{
	uint8_t rc_code;
	uint16_t networkId;
	uint8_t apTxPower;
	uint8_t frameProfile;
	uint16_t maxMotes;
	uint16_t baseBandwidth;
	uint8_t downFrameMultVal;
	uint8_t numParents;
	uint8_t ccaMode;
	uint16_t channelList;
	uint8_t autoStartNetwork;
	uint8_t locMode;
	uint8_t bbMode;
	uint8_t bbSize;
	uint8_t isRadioTest;
	uint16_t bwMult;
	uint8_t oneChannel;
}network_config;

// Class declaration
class Smartmesh_API{
  	public:
		uint8_t send_data[128];
		bool commandRecieved;// Is set whenever data was recieved from manager
	
		Smartmesh_API(UART *uart);
		bool sendCommand(send_params params);
		bool mgr_init();
		bool subscribe(uint32_t sub, uint32_t unpack);
		bool sendData(uint64_t macAddr, uint8_t priority, const uint8_t *src, const uint8_t *dst, uint8_t *data, uint8_t length);
		uint8_t packetType(uint8_t *data);
		bool getNetworkInfo(void);
		bool parseDataNotification(data_notif *notif, uint8_t *data);
		bool parseNetworkInfo(network_info *info, uint8_t *data);
		bool clearStatistics(void);
		bool getHardwareInfo(void);
		bool parseHardwareInfo(system_info *info, uint8_t *data);
		bool setNetworkConfig(uint8_t *network_id);
		bool getNetworkConfig(void);
		bool setJoinKey(uint8_t *jkey);
    	bool getMoteConfigFromMoteId(uint16_t moteid);
		bool getMoteInfo(uint8_t *mac_address);
		bool parseGetMoteInfo(mote_info *info, uint8_t *data);
		bool resetManager(void);
		bool parseNetworkConfig(network_config *config, uint8_t *data);
		bool getMoteConfigFromMac(uint8_t *mac_addr);

  	private:
		UART *sendUart; // Uart port for where to send data to (a seperate class)
		uint8_t mgrSeqNum;
    	uint8_t cliSeqNum;
	
		static SemaphoreHandle_t usingApi;

    	void init_packet(uint8_t length, uint8_t command);
    	void checksumData(uint16_t fcs, uint8_t *data, uint16_t len);
    	uint16_t verifyPacket(uint16_t fcs, uint8_t *data, uint16_t len);
};

#endif
