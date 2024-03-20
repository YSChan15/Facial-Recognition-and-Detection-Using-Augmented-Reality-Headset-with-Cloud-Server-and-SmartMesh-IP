#include "API.h"

using namespace std;

SemaphoreHandle_t Smartmesh_API::usingApi;

#define START_CHECKSUM 0xFFFF// Start value for a new FCS

// Lookup table for the 16-bit FCS
static uint16_t fcstab[256] = {
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
  	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
  	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
  	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

/**
	\brief  Constructor for SMartmesh IP API wrapper class
	
	\description API Constructor
	
	\param  uart UART port to send API commands through
	
	\retval None
**/
Smartmesh_API::Smartmesh_API(UART *uart){
	sendUart = uart;
	cliSeqNum = 0;
	
	if(usingApi == nullptr)
	{
		usingApi = xSemaphoreCreateBinary();
		xSemaphoreGive(usingApi);
	}
}

/**
	\brief Runs a RFC 1662 CRC on an array of bytes and stores the checksum according to Smartmesh IP docs

	\description Runs the CRC on the array starting off with the fcs checksum. Once the checksum calculation
								is complete, auto stores the checksum in the two bytes before last flag bit. 

	\param fcs - Current checksum if none use 0xFFFF
				 data - pointer to array of bytes
				 len - length of the array not counting flag bytes

	\retval None
**/
void Smartmesh_API::checksumData(uint16_t fcs, uint8_t *data, uint16_t len){// Pass in 0xFFFF for fcs if not checksum yet
	data++;// Bypass the flag bit
	while(len--)
		fcs = (fcs >> 8) ^ fcstab[(fcs ^ *data++) & 0xFF];// Update the checksum for the current data 
	
	fcs ^= 0xFFFF;// Must XOR the checksum calculation result
	*data = fcs & 0xFF;// MSB
	data++;
	*data = (fcs >> 8) & 0xFF;// LSB
}

uint16_t Smartmesh_API::verifyPacket(uint16_t fcs, uint8_t *data, uint16_t len){
	data++;
	while(len--)
		fcs = (fcs >> 8) ^ fcstab[(fcs ^ *data++) & 0xFF];
	
	fcs ^= 0xFFFF;
	return fcs;
}

/**
	\brief Initializes API packet flags and header

	\decription Sets basic packet information according to the Smartmesh API protocol. Currently,
					all packets sent will require the network manager to send an 'ack' packet back

	\params length - length of data portion of packet
					command - Command to place within packet header refer to defines/docs

	\retval None
**/
void Smartmesh_API::init_packet(uint8_t length, uint8_t command){
	send_data[0] = 0x7E;
	send_data[1] = 0x02;
	send_data[2] = command;
	send_data[3] = cliSeqNum;
	send_data[4] = length;
	send_data[7+length] = 0x7E;// End of packet transmittion
	cliSeqNum++;// Increment user sending numbers
}

/**
	\brief Initialize a connection with a Smartmesh IP network manager

	\description Sets up initial communication with the network manager(hello packet). Must be run
								before any other command can be executed. Default API version if 4.0

	\params None

	\retval	CMD_SUCCESS - command excecuted successfully
					CMD_FAIL - command failed => TODO
**/
bool Smartmesh_API::mgr_init(){
	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	cliSeqNum = 0;
	
	init_packet(3, USR_HELLO);
	send_data[1] = 0x0;
	send_data[5] = API_APP_VER;
	send_data[6] = cliSeqNum - 1;// Must be same as one that was sent first
	send_data[7] = 0;
	checksumData(START_CHECKSUM, send_data, 7);
	sendUart->send_array(send_data, 11);
	
	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);
	return CMD_SUCCESS;
}

/**
	\brief Subscribe to certain notifications for the network manager

	\description This command will subscribe to notifications specified within the sub parameter. The
								unpack filter if set to 1 will not require the application to send an 'ack' packet 
								back to the network manager.

	\params sub - the subscribe filter
					unpack - the unpack filter

	\retval	CMD_SUCCESS - command excecuted successfully
					CMD_FAIL - command failed => TODO: add sub/unpack verification
**/
bool Smartmesh_API::subscribe(uint32_t sub, uint32_t unpack){
	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	init_packet(8, SUBSCRIBE);
	
	memcpy(send_data + 5, &sub, 4);// Copy bytes over to array
	memcpy(send_data + 9, &unpack, 4);
	checksumData(START_CHECKSUM, send_data, 12);
	sendUart->send_array(send_data, 16);// Send whole payload including flags/checksum
	
	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);
	return CMD_SUCCESS;
}

/**
	\brief Send a packet to a mote by mac address(0xFFFFFFFF for all motes)

	\description This function sends a packet to the specified mac address. Destination UDP port must be 
								correct for packet to be recieved

	\params macAddr - mac address of mote 
					priority - either PACKET_PRIORITY_LOW, PACKET_PRIORITY_MED, PACKET_PRIORITY_HIGH
					src - source UDP port 
					dst - destination UDP port
					data - array of data to send to the mote
					length - length of the array that will be sent

	\retval CMD_SUCCESS - command excecuted successfully
					CMD_FAIL - command failed => TODO
**/
bool Smartmesh_API::sendData(uint64_t macAddr, uint8_t priority, const uint8_t *src, const uint8_t *dst, uint8_t *data, uint8_t length){
	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	
	init_packet(length + 14, SEND_DATA);
	memcpy(send_data + 5, &macAddr, 8);
	send_data[13] = priority;
	memcpy(send_data + 14, src, 2);
	memcpy(send_data + 16, dst, 2);
	send_data[18] = 0;
	memcpy(send_data + 19, data, length);
	checksumData(START_CHECKSUM, send_data, length + 18);
	
	sendUart->send_array(send_data, length + 22);
	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);
	
	return CMD_SUCCESS;
}

/**
	\brief Get type of packet recieved

	\description Returns the packet type from the packet header(3rd byte after flag)

	\params data - array of bytes recieved

	\retval Packet type
**/
uint8_t Smartmesh_API::packetType(uint8_t *data){
	data+=2;
	return *data;
}

/**
	\brief Parse a data notification that come in

	\description Parses a data notification from packet form to a struct where the data can be more easily
								accessed and manipulated. ALL data is copied in big endian format

	\params data - array of bytes recieved

	\retval bool - CMD_SUCCESS/CMD_FAIL
**/
bool Smartmesh_API::parseDataNotification(data_notif *notif, uint8_t *data){
	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	
	uint8_t *dataOriginal = data;
	data+=2;
	if(*data != NOTIF)
		return CMD_FAIL;
	
	memcpy(&notif->seconds, data+=4, sizeof(notif->seconds));
	memcpy(&notif->us, data+=sizeof(notif->seconds), sizeof(notif->us));
	memcpy(&notif->macAddr, data+=sizeof(notif->us), sizeof(notif->macAddr));
	memcpy(&notif->src, data+=sizeof(notif->macAddr), sizeof(notif->src));
	memcpy(&notif->dest, data+=sizeof(notif->src), sizeof(notif->dest));
	data+=sizeof(notif->dest);
	int i = 0;
	while(1){
		if(*data == 0x7E){// Verify checksum
			verifyPacket(START_CHECKSUM, dataOriginal, i-2);
//			uint16_t fcs_temp = *(data - 1) | *(data - 2) << 8;
			notif->length = i-2;
			break;
		}
		if(i > 90)
			return CMD_FAIL;
		
		memset(&notif->data[i], *data, 1);// Copy data byte over
		data++;
		i++;
	}
	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);
	
	return CMD_SUCCESS;
}

// Sends the get network info command
bool Smartmesh_API::getNetworkInfo(){
	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	
	init_packet(0, GET_NETWORK_INFO);
	checksumData(START_CHECKSUM, send_data, 4);
	sendUart->send_array(send_data, 8);
	
	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);
	
	return CMD_SUCCESS;
}

// Parses the get network info command response
bool Smartmesh_API::parseNetworkInfo(network_info *info, uint8_t *data){
	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	
	uint8_t *dataOriginal = data;
	if(*(data+2) != 0x40)// Not a data packet
		return CMD_FAIL;
	
	// Copy data over to data struct
	memcpy((uint8_t*)&info->rc_code, data+=5, sizeof(info->rc_code));
	memcpy((uint8_t*)&info->num_motes, data+=sizeof(info->rc_code), sizeof(info->num_motes));
	memcpy((uint8_t*)&info->asnSize, data+=sizeof(info->num_motes), sizeof(info->asnSize));
	memcpy((uint8_t*)&info->adv_state, data+=sizeof(info->asnSize), sizeof(info->adv_state));
	memcpy((uint8_t*)&info->downStreamFrame, data+=sizeof(info->adv_state), sizeof(info->downStreamFrame));
	memcpy((uint8_t*)&info->network_reliability, data+=sizeof(info->downStreamFrame), sizeof(info->network_reliability));
	memcpy((uint8_t*)&info->netPathStability, data+=sizeof(info->network_reliability), sizeof(info->netPathStability));
	memcpy((uint8_t*)&info->latency, data+=sizeof(info->netPathStability), sizeof(info->latency));
	memcpy((uint8_t*)&info->netState, data+=sizeof(info->latency), sizeof(info->netState));
	memcpy((void*)&info->ipv6AddrHigh, data+=sizeof(info->netState), sizeof(info->ipv6AddrHigh));
	memcpy((void*)&info->ipv6AddrLow, data+=sizeof(info->ipv6AddrHigh), sizeof(info->ipv6AddrLow));
	memcpy((uint8_t*)&info->numLostPackets, data+=sizeof(info->ipv6AddrLow), sizeof(info->numLostPackets));
	memcpy((uint8_t*)&info->numArrivedPackets, data+=sizeof(info->numLostPackets), sizeof(info->numArrivedPackets));
	memcpy((uint8_t*)&info->maxHops, data+=sizeof(info->numArrivedPackets), sizeof(info->maxHops));	
	data+=sizeof(info->maxHops);
	// Verify the CRC
	data += 48;
	verifyPacket(START_CHECKSUM, dataOriginal, 43+4);
//	uint16_t fcs_temp = *(data - 1) | *(data - 2) << 8;
	//if(temp != fcs_temp)
	//	return CMD_FAIL;
	
	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);
	
	return CMD_SUCCESS;
}

// Clear network statistics
bool Smartmesh_API::clearStatistics(){
	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	
	init_packet(0, CLEAR_STAT);
	checksumData(START_CHECKSUM, send_data, 4);
	sendUart->send_array(send_data, 8);
	
	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);
	return CMD_SUCCESS;
}

// Get Hardware Information
bool Smartmesh_API::getHardwareInfo(){
	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	
	init_packet(0, GET_SYS_INFO);
	checksumData(START_CHECKSUM, send_data, 4);
	sendUart->send_array(send_data, 8);
	
	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);
	
	return CMD_SUCCESS;
}

// Parse hardware information
bool Smartmesh_API::parseHardwareInfo(system_info *info, uint8_t *data){
	if(*(data+5) != RC_OK)
		return CMD_FAIL;

	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	
	memcpy((uint8_t*)&info->rc_code, data+=5, sizeof(info->rc_code));
	memcpy((uint8_t*)&info->mac_address, data+=sizeof(info->rc_code), sizeof(info->mac_address));
	memcpy((uint8_t*)&info->hw_model, data+=sizeof(info->mac_address), sizeof(info->hw_model));
	memcpy((uint8_t*)&info->hw_rev, data+=sizeof(info->hw_model), sizeof(info->hw_rev));
	memcpy((uint8_t*)&info->sw_major, data+=sizeof(info->hw_rev), sizeof(info->sw_major));
	memcpy((uint8_t*)&info->sw_minor, data+=sizeof(info->sw_major), sizeof(info->sw_minor));
	memcpy((uint8_t*)&info->sw_patch, data+=sizeof(info->sw_minor), sizeof(info->sw_patch));
	memcpy((uint8_t*)&info->sw_build, data+=sizeof(info->sw_patch), sizeof(info->sw_build));

	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);
	
	return CMD_SUCCESS;
}

// Setup network manager
bool Smartmesh_API::setNetworkConfig(uint8_t *network_id){
	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	
	init_packet(21, SET_NTWK_CONFIG);
	memcpy(send_data + 5, network_id, 2);
	send_data[7] = 8;// Tx power(8dbm)
	send_data[8] = 1;// Frame profile
	send_data[9] = 0;// Max motes
	send_data[10] = 10;
	send_data[11] = 23;// Base bandwidth(9s between each packet default)
	send_data[12] = 28;
	send_data[13] = 1;// Downstream frame multiplier
	send_data[14] = 3;// Number of parents for each mote
	send_data[15] = 0;// Both energy and carrier detect if network has congestion
	send_data[16] = 0x7F;// Use all channels
	send_data[17] = 0xFF;
	send_data[18] = 0x1;// Autostart network
	send_data[19] = 0;// Reserved
	send_data[20] = 0;// Backbone frame mode
	send_data[21] = 1;// Backbone size
	send_data[22] = 0;// Radiotest is off
	send_data[23] = 1;// Bandwidth over-provisioning multiplier
	send_data[24] = 0x2C;
	send_data[25] = 0xFF;// Use all channels
	checksumData(START_CHECKSUM, send_data, 25);
	sendUart->send_array(send_data, 29);// Send the packet to the network manager
	
	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);
	
	return CMD_SUCCESS;
}

// Gets the current network configuration
bool Smartmesh_API::getNetworkConfig(){
	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	
	init_packet(0, GET_NET_CONFIG);
	
	checksumData(START_CHECKSUM, send_data, 4);
	sendUart->send_array(send_data, 8);
	
	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);
	
	return CMD_SUCCESS;
}

// Set the join key of the network manager
bool Smartmesh_API::setJoinKey(uint8_t *jkey){
	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	
	init_packet(16, SET_COMMON_JKEY);
	memcpy(send_data+5, jkey, 16);
	checksumData(START_CHECKSUM, send_data, 20);
	sendUart->send_array(send_data, 24);

	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);

	return CMD_SUCCESS;
}

// Gets the mote configuration from its mote id
bool Smartmesh_API::getMoteConfigFromMoteId(uint16_t moteid){
	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	
	init_packet(2, GET_MOTE_CFG_BY_ID);
	send_data[5] = (moteid >> 8)&0xFF;
	send_data[6] = moteid&0xFF;
	
	checksumData(START_CHECKSUM, send_data, 6);
	sendUart->send_array(send_data, 10);

	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);
	
	return CMD_SUCCESS;
}

// Get mote information from its mac address
bool Smartmesh_API::getMoteInfo(uint8_t *mac_address){
	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	
	init_packet(8, GET_MOTE_INFO);
	memcpy(send_data+5, mac_address, 8);
	
	checksumData(START_CHECKSUM, send_data, 12);
	sendUart->send_array(send_data, 16);
	
	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);
	
	return CMD_SUCCESS;
}

// Parses the get mote info command
bool Smartmesh_API::parseGetMoteInfo(mote_info *info, uint8_t *data){
	if(*(data+5) != RC_OK)
		return CMD_FAIL;
	
	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	
	// Copy the data over to the struct
	memcpy((uint8_t*)&info->rc_code, data+=5, sizeof(info->rc_code));
	memcpy((uint8_t*)&info->mac_address, data+=sizeof(info->rc_code), sizeof(info->mac_address));
	memcpy((uint8_t*)&info->state, data+=sizeof(info->mac_address), sizeof(info->state));
	memcpy((uint8_t*)&info->numNbrs, data+=sizeof(info->state), sizeof(info->numNbrs));
	memcpy((uint8_t*)&info->numGoodNbrs, data+=sizeof(info->numNbrs), sizeof(info->numGoodNbrs));
	memcpy((uint8_t*)&info->requestedBw, data+=sizeof(info->numGoodNbrs), sizeof(info->requestedBw));
	memcpy((uint8_t*)&info->totalNeededBw, data+=sizeof(info->requestedBw), sizeof(info->totalNeededBw));
	memcpy((uint8_t*)&info->assignedBw, data+=sizeof(info->totalNeededBw), sizeof(info->assignedBw));
	memcpy((uint8_t*)&info->packetsReceived, data+=sizeof(info->assignedBw), sizeof(info->packetsReceived));
	memcpy((uint8_t*)&info->packetsLost, data+=sizeof(info->packetsReceived), sizeof(info->packetsLost));
	memcpy((uint8_t*)&info->avgLatency, data+=sizeof(info->packetsLost), sizeof(info->avgLatency));
	memcpy((uint8_t*)&info->stateTime, data+=sizeof(info->avgLatency), sizeof(info->stateTime));
	memcpy((uint8_t*)&info->numJoins, data+=sizeof(info->stateTime), sizeof(info->numJoins));
	memcpy((uint8_t*)&info->hopDepth, data+=sizeof(info->numJoins), sizeof(info->hopDepth));
	
	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);
	
	return CMD_SUCCESS;
}

bool Smartmesh_API::resetManager(){
	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	
	init_packet(9, RESET_CMD);
	memset(send_data + 5, 0, 9);
	
	checksumData(START_CHECKSUM, send_data, 13);
	sendUart->send_array(send_data, 17);
	
	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);

	return true;
}


bool Smartmesh_API::parseNetworkConfig(network_config *config, uint8_t *data){
	if(data[5] != RC_OK)
		return CMD_FAIL;

	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	
	// Copy the data over to the struct
	memcpy((uint8_t*)&config->rc_code, data+=5, sizeof(config->rc_code));
	memcpy((uint8_t*)&config->networkId, data+=sizeof(config->rc_code), sizeof(config->networkId));
	memcpy((uint8_t*)&config->apTxPower, data+=sizeof(config->networkId), sizeof(config->apTxPower));
	memcpy((uint8_t*)&config->frameProfile, data+=sizeof(config->apTxPower), sizeof(config->frameProfile));
	memcpy((uint8_t*)&config->maxMotes, data+=sizeof(config->frameProfile), sizeof(config->maxMotes));
	memcpy((uint8_t*)&config->baseBandwidth, data+=sizeof(config->maxMotes), sizeof(config->baseBandwidth));
	memcpy((uint8_t*)&config->downFrameMultVal, data+=sizeof(config->baseBandwidth), sizeof(config->downFrameMultVal));
	memcpy((uint8_t*)&config->numParents, data+=sizeof(config->downFrameMultVal), sizeof(config->numParents));
	memcpy((uint8_t*)&config->ccaMode, data+=sizeof(config->numParents), sizeof(config->ccaMode));
	memcpy((uint8_t*)&config->channelList, data+=sizeof(config->ccaMode), sizeof(config->channelList));
	memcpy((uint8_t*)&config->autoStartNetwork, data+=sizeof(config->channelList), sizeof(config->autoStartNetwork));
	memcpy((uint8_t*)&config->locMode, data+=sizeof(config->autoStartNetwork), sizeof(config->locMode));
	memcpy((uint8_t*)&config->bbMode, data+=sizeof(config->locMode), sizeof(config->bbMode));
	memcpy((uint8_t*)&config->bbSize, data+=sizeof(config->bbMode), sizeof(config->bbSize));
	memcpy((uint8_t*)&config->isRadioTest, data+=sizeof(config->bbSize), sizeof(config->isRadioTest));
	memcpy((uint8_t*)&config->bwMult, data+=sizeof(config->isRadioTest), sizeof(config->bwMult));
	memcpy((uint8_t*)&config->oneChannel, data+=sizeof(config->bwMult), sizeof(config->oneChannel));
	
	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);

	return CMD_SUCCESS;
}

bool Smartmesh_API::getMoteConfigFromMac(uint8_t *mac_addr){
	configASSERT(xSemaphoreTake(usingApi, 1000) == pdTRUE);
	
	init_packet(9, GET_MOTE_CONFIG);
	memcpy(send_data+5, mac_addr, 8);
	memset(send_data+13, 0, 1);
	
	checksumData(START_CHECKSUM, send_data, 13);
	sendUart->send_array(send_data, 17);
	
	configASSERT(xSemaphoreGive(usingApi) == pdTRUE);
	
	return CMD_SUCCESS;
}
