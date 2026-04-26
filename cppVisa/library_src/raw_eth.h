/////////////////////////////////////////////////////////////////////////////
// Author:      Alexandre CARPENTIER
// Modified by:
// Created:     02/04/2026
// Copyright:   (c) Alexandre CARPENTIER
// Licence:     LGPL-2.1-or-later
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <iostream>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using socket_t = SOCKET;
#define INVALID_SOCKET_VALUE INVALID_SOCKET
#define SOCKET_ERROR_VALUE SOCKET_ERROR
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
using socket_t = int;
#define INVALID_SOCKET_VALUE -1
#define SOCKET_ERROR_VALUE -1
#define closesocket close
#endif

class RawEthProtocol
{
public:
	RawEthProtocol(const std::string& address, uint32_t timeout);
	~RawEthProtocol();

	bool connect();
	bool disconnect();

	bool write(const std::string& command);

	bool read(std::string& response);

	void setTimeout(uint32_t timeout);

	void setAddress(const std::string& address);

	uint32_t getTimeout() const { return m_timeout; }
	std::string getAddress() const { return m_address; }

private:
	std::string m_address;
	uint32_t m_timeout;
	socket_t m_socket;
	std::string m_host;
	uint16_t m_port;

	bool parseAddress();
	bool initializeWinsock();
	void cleanupWinsock();
};