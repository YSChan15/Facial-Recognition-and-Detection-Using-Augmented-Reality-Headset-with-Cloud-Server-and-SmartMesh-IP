#include "LTE.h"
#include <string.h>
#include <string>
#include <algorithm>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "RTCClass.h"

bool LTE::serverConnected = false;

// Constructor. Takes an instance of uart class (see SERCOM.h)
LTE::LTE(UART *uart)
{
	 // Get a uart instance for the module (see main.cpp for port used)
	_usart = uart;

	// Create semaphores needed
	sendingData = xSemaphoreCreateBinary(); // Used only when sending data (see LTE::send_to_firebase())
	LTE_used = xSemaphoreCreateBinary(); // Used when sending commands
	giveSem(false); // Give LTE_used semaphore so commands can be sent
							  // sendingData not given until module is initialized and ready to send
}

bool LTE::init()
{
	// Synchronize baud rate by sending "AT" repeatedly until module responds
	// Module uses these repeated "AT" commands to synchronize baude rate
	do{
		giveSem(false); // Give the semaphore
		sendCommand("AT\r\n"); // Because sendCommand will try to take it when sending
		vTaskDelay(100); // Wait 100ms before sending again

	}while(xSemaphoreTake(LTE_used, 250) == pdFALSE); // Attempt to take semaphore and if not 
													  // Possible within 250ms, go back to the loop
	giveSem(false);
	return true;
}

// Connect module to hologram network
bool LTE::connectToNetwork()
{
	sendCommand("ATE1\r\n"); // Disable echo. "AT1" enables echo and is useful for debugging
	sendCommand("AT+CFUN=1\r\n"); // Set the SIM7000 to full functionality mode
	sendCommand("AT+CIPSHUT\r\n"); // Shut down the PDP context
	sendCommand("AT+CNMP=38\r\n"); // Set the network mode to LTE
	sendCommand("AT+CMNB=1\r\n"); // Set device preference to CAT-M1 over Nb-IoT
	sendCommand("AT+CSTT=\"hologram\"\r\n"); // Set the APN to hologram
	sendCommand("AT+CIICR\r\n"); // Bring up wireless connection
	sendCommand("AT+CLTS=1\r\n"); //get local timestamp
	sendCommand("AT+CNTP\r\n"); //synchronize network time
	sendCommand("AT+CCLK?\r\n"); // Get current time
 
	xSemaphoreGive(sendingData);
	return true;
}


bool LTE::connectToServer(){
	
	sendCommand("ATE0\r\n"); // Disable echo. "AT1" enables echo and is useful for debugging
	sendCommand("AT+CFUN=1\r\n"); // Set the SIM7000 to full functionality mode
	sendCommand("AT+CNMP=38\r\n"); // Set the network mode to LTE
	sendCommand("AT+CMNB=1\r\n"); // Set device preference to CAT-M1 over Nb-IoT
	sendCommand("AT+CIPSTATUS\r\n"); //Query current connection status
	sendCommand("AT+CSTT=\"hologram\"\r\n"); // Set the APN to hologram
	sendCommand("AT+CIICR\r\n"); // Bring up wireless connection
	sendCommand("AT+CIFSR\r\n");
	sendCommand("AT+CIPSTART=\"TCP\",\"3.14.248.206\",3420\r\n");	//connect to AWS using TCP connection

	return true;
}


// Disconnect from network. Call connect to reconnect
bool LTE::disconnect()
{
	sendCommand("AT+COPS=2\r\n"); // Deregister from network
	sendCommand("AT+COPS=0\r\n"); // Set automatic operator selection
	sendCommand("AT_CGACT=0,1\r\n"); // Deactivate PDP context
	sendCommand("AT_CFUN=0\r\n"); // Set the mobile to minimum functionality
	return true;
}

// Check if LTE module is connected to network. INCOMPLETE
bool LTE::isConnected()
{
	sendCommand("AT+CGATT?\r\n");	
	return false;
}

// Send a string/command to the LTE module 
bool LTE::sendCommand(const char *cmd)
{
	vTaskDelay(10); // Wait for 10ms so previous command is sent successfully

	takeSem(false); // Take semaphore. Response of current command must be parsed
						// before the semaphore is available to be taken again

	_usart->_printf(cmd); // Send the command
	
	if (strstr(cmd, "AT+SHCONN")) // If command is for connecting to url, wait for 10ms
		vTaskDelay(10);				// so module has time to connect

	return true;
}

// Parse the LTE module response. NOTE: "AT+CCLK?" command used 
// to get time is parsed separately because time is extracted
// from the string and used to setup the MCU's RTC
bool LTE::parse(std::string &response)
{
	if (response == "CONNECT OK\r\n") {
		serverConnected = true;
		giveSem(false);
		return true;
	}
	else if (response == "CLOSED\r\n") {
		// Signal completion of the LTE data parsing task
		serverConnected = false;
		giveSem(false);
		return true;
	}

	return false;
}

// Check if the response is for getting clock from module
bool LTE::isClockResponse(const char* response)
{
	if (strstr(response, "+CCLK") != NULL){
		giveSem(false);
		return true;
	}
	giveSem(false);
	return false;
}

// Check if string contains a valid IP address
// The "AT+CIICR" command returns IP address
// of the network module is connected to
bool LTE::isValidIP(std::string &s)
{
	// Ignore any non-digit characters before the IP address
	int len = s.length(); // strlen(s.c_str())
	int count = 0;
	while (!isdigit(s[count]))
		count++;

	// Extract the IP address from the string
	len = len - (count + 1); // Adjust length to not include non-digit chars
	if (len < 7 || len > 15) // Make sure IP is valid length (between 7 and 15 chars)
		return false;

	// Extract the IP address
	char tail[16];
	tail[0] = 0;
	unsigned int d[4];
	int c = sscanf(s.c_str(), "%3u.%3u.%3u.%3u%s", &d[0], &d[1], &d[2], &d[3], tail); // change to string stream?

	// Check if the IP address is valid
	if (c != 4 || tail[0])
		return false;
	for (int i = 0; i < 4; i++)
		if (d[i] > 255)
			return false;

	return true;
}

bool LTE::sendHTTPMessage(const char *message, const char *url)
{
	xSemaphoreTake(sendingData, portMAX_DELAY);
	sendCommand("AT+CSSLCFG=\"sslversion\",1,3\r\n");
	sendCommand("AT+SHSSL=1,\"\"\r\n");
	sendCommand("AT+CNACT=1\r\n");
	sendCommand("AT+SHDISC\r\n");
	sendCommand(concat_strings(4, "AT+SHCONF=\"URL\",\"", url, "\"", "\r\n"));
	sendCommand("AT+SHCHEAD\r\n");
	sendCommand("AT+SHCONF=\"BODYLEN\",1024\r\n");
	sendCommand("AT+SHCONF=\"HEADERLEN\",350\r\n");
	sendCommand("AT+SHCONN\r\n");
	sendCommand("AT+SHCHEAD\r\n");
	sendCommand("AT+SHAHEAD=\"Connection\",\"keep-alive\"\r\n");

	// Construct a JSON with timestamp and message
	char *msg = concat_strings(5, "AT+SHBOD=\"{\\\"", rtc.get_string_timestamp(), "\\\":\\\"", message, "\\\"}\",");
	int length = strlen(msg) + 2;
	char lenstr[20];
	sprintf(lenstr, "%d", length);
	sendCommand(concat_strings(3, msg, lenstr, "\r\n"));
	free(msg);

	sendCommand(concat_strings(3, "AT+SHREQ=\"/", "mac", ".json\",4\r\n"));
	sendCommand("AT+SHDISC\r\n");

	// xSemaphoreTake(sendingData, portMAX_DELAY);
	return true;
}

//sends AT commands that return information for debugging; used after connection
void LTE::checkDebugInfo(){
	sendCommand("AT+CGMR\r\n"); //check firmware version
	sendCommand("AT+CPAS\r\n"); //check phone activity status
	sendCommand("AT+CSQ\r\n");  //check signal quality
	sendCommand("AT+CNSMOD?\r\n"); //Show Network System Mode
	sendCommand("AT+CPSI?\r\n"); //Inquiring UE System Information
	sendCommand("AT+CGREG?\r\n"); //Network Registration Status
	sendCommand("AT+SHSTATE?\r\n"); //Query HTTP(S) Connection Status
}


// Sends a packet to firebase
bool LTE::send_to_firebase(uint8_t *data, uint8_t* mac){
	xSemaphoreTake(sendingData, portMAX_DELAY);
	sendCommand("AT+CSSLCFG=\"sslversion\",1,3\r\n"); // Set SSL version to TLS 1.2
	sendCommand("AT+SHSSL=1,\"\"\r\n"); // Set up secure SSL connection
	sendCommand("AT+CNACT=1\r\n"); // Activate PDP context
	sendCommand("AT+SHDISC\r\n");// Make sure it is not connected to anything
	sendCommand("AT+SHCONF=\"URL\",\"https://wsn-demo-56e2f-default-rtdb.firebaseio.com\"\r\n"); // Set the URL to send data to https://wirelesssensornetwork-1585a-default-rtdb.firebaseio.com
	sendCommand("AT+SHCONF=\"BODYLEN\",1024\r\n"); // Set the max body length to 1024 bytes
	sendCommand("AT+SHCONF=\"HEADERLEN\",350\r\n"); // Set max header length
	sendCommand("AT+SHCONN\r\n"); // Connect to server
	sendCommand("AT+SHCHEAD\r\n"); // Setup the header
	sendCommand("AT+SHAHEAD=\"Connection\",\"keep-alive\"\r\n"); // Header: Connection, keep-alive. Send "AT+SHDISC" to disconnect
	
	// Convert data to ascii
	uint32_t val = ((uint64_t)data[0] << 56) | ((uint64_t)data[1] << 48) | ((uint64_t)data[2] << 40) | ((uint64_t)data[3] << 32)\
			 | (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];

	// Loop over each decimal place of the input value, starting with the largest decimal place
	char asciiData[10];
	int dataCount = 0;
	for(int i = 100000;i >= 1; i /= 10){
		char x = (val - (val % i))/i % 10 + 0x30; // Compute the value of the current decimal place
                                                    // and convert it to an ascii character
		asciiData[dataCount] = x;
		dataCount++;
	}
	
	char* timestampStr  = rtc.get_string_timestamp();
	int strLength = 17; // Length of timestampStr
	
	// Setup main body command with JSON embedded. 
	char buffer[128];
	memcpy(buffer, "AT+SHBOD=\"{\\\"", 13);
	memcpy(buffer+13, timestampStr  , strLength);
	memcpy(buffer+13+strLength, "\\\":\\\"", 5); // NOTE: double '\' used to ignore the ' " ' char
													// one '\' is for MCU to ignore that char
													  // and the other '\' used for LTE module to ignore the char
													   // so MCU only sends a "\\":..." (see SIM7000 datasheet)
	memcpy(buffer+18+strLength, asciiData, dataCount);
	memcpy(buffer+18+strLength+dataCount, "\\\"}\",", 5);
	delete timestampStr ; // buffer now looks like: "AT+SHBOD="{\"05-03-23 07:31:19\":\"data\"}",
	
	// Get size of characters to send
	char cntChar[2];
	int charCount = 7 + strLength + dataCount;
	int countPos = 0;
	for(int i = 10;i >= 1; i /= 10){
		char x = (charCount - (charCount % i))/i % 10 + 0x30;
		cntChar[countPos] = x;
		countPos++;
	}
	
	// Add character count and /r/n to buffer string
	memcpy(buffer+23+strLength+dataCount, cntChar, 2); 
	memcpy(buffer+25+strLength+dataCount, "\r\n\0", 3); // "AT+SHBOD="{\"dd-mm-yy hh:mm:ss\":\"data\"}",cntChar\r\n\0"
	sendCommand(buffer); // Send the buffer string

	// Specify the HTTP request type and use the sub url "/mac.json"
	memcpy(buffer, "AT+SHREQ=\"/", 11);
	memcpy(buffer+11, mac, 23);
	memcpy(buffer+34, ".json\",4\r\n\0",11); // "AT+SHREQ=\"/xx:xx:xx:xx:xx:xx.json\",4\r\n\0", 
												// where xx:xx:xx:xx:xx:xx is the value of the mac string.
	sendCommand(buffer);// The '4' after .json specifies the HTTP request type as put
	sendCommand("AT+SHDISC\r\n");// Disconnect from the server
	
	xSemaphoreGive(sendingData); // Give semaphore indicating next packet can now be sent if ready
	return true;
}

// Give the LTE_used semaphore
BaseType_t LTE::giveSem(bool fromISR)
{
	return !fromISR ? xSemaphoreGive(LTE_used) : xSemaphoreGiveFromISR(LTE_used, nullptr);
}

// Take the LTE_used semaphore
BaseType_t LTE::takeSem(bool fromISR)
{
	return !fromISR ? xSemaphoreTake(LTE_used, portMAX_DELAY) : xSemaphoreTakeFromISR(LTE_used, nullptr);
}