/////////////////////////////////////////////////////////////////////////////
// Author:      Alexandre CARPENTIER
// Modified by:
// Created:     02/04/2026
// Copyright:   (c) Alexandre CARPENTIER
// Licence:     LGPL-2.1-or-later
/////////////////////////////////////////////////////////////////////////////
#include "raw_eth.h"
#include <sstream>
#include <cstring>

RawEthProtocol::RawEthProtocol(const std::string& address, uint32_t timeout)
	: m_address(address), m_timeout(timeout), m_socket(INVALID_SOCKET_VALUE), m_host(""), m_port(5025)
{
	std::cout << "[RAWETH] Initializing with address: " << address << "\n";

	if (!initializeWinsock()) {
		throw std::runtime_error("Failed to initialize Winsock");
	}

	if (!parseAddress()) {
		cleanupWinsock();
		throw std::runtime_error("Failed to parse address: " + address);
	}
}

RawEthProtocol::~RawEthProtocol() {
	if (m_socket != INVALID_SOCKET_VALUE) {
		disconnect();
	}
	cleanupWinsock();
}

bool RawEthProtocol::parseAddress() {
	size_t colonPos = m_address.find(':');
	if (colonPos != std::string::npos) {
		m_host = m_address.substr(0, colonPos);
		try {
			m_port = static_cast<uint16_t>(std::stoi(m_address.substr(colonPos + 1)));
		} catch (...) {
			std::cerr << "[RAWETH] Invalid port number in address\n";
			return false;
		}
	} else {
		m_host = m_address;
		m_port = 5025;
	}

	if (m_host.empty()) {
		std::cerr << "[RAWETH] Empty host address\n";
		return false;
	}

	std::cout << "[RAWETH] Parsed address - Host: " << m_host << ", Port: " << m_port << "\n";
	return true;
}

bool RawEthProtocol::initializeWinsock() {
#ifdef _WIN32
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		std::cerr << "[RAWETH] WSAStartup failed: " << result << "\n";
		return false;
	}
#endif
	return true;
}

void RawEthProtocol::cleanupWinsock() {
#ifdef _WIN32
	WSACleanup();
#endif
}

bool RawEthProtocol::connect() {
	if (m_socket != INVALID_SOCKET_VALUE) {
		std::cout << "[RAWETH] Already connected\n";
		return true;
	}

	struct addrinfo hints{}, *result = nullptr, *ptr = nullptr;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	std::string portStr = std::to_string(m_port);
	int iResult = getaddrinfo(m_host.c_str(), portStr.c_str(), &hints, &result);
	if (iResult != 0) {
		std::cerr << "[RAWETH] getaddrinfo failed: " << iResult << "\n";
		return false;
	}

	for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
		m_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (m_socket == INVALID_SOCKET_VALUE) {
			continue;
		}

		iResult = ::connect(m_socket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen));
		if (iResult == SOCKET_ERROR_VALUE) {
			closesocket(m_socket);
			m_socket = INVALID_SOCKET_VALUE;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (m_socket == INVALID_SOCKET_VALUE) {
		std::cerr << "[RAWETH] Unable to connect to server\n";
		return false;
	}

#ifdef _WIN32
	DWORD timeout_ms = m_timeout;
	setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms));
	setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms));
#else
	struct timeval tv;
	tv.tv_sec = m_timeout / 1000;
	tv.tv_usec = (m_timeout % 1000) * 1000;
	setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const void*)&tv, sizeof(tv));
	setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (const void*)&tv, sizeof(tv));
#endif

	std::cout << "[RAWETH] Connected to " << m_host << ":" << m_port << "\n";
	return true;
}

bool RawEthProtocol::disconnect() {
	if (m_socket == INVALID_SOCKET_VALUE) {
		return true;
	}

	closesocket(m_socket);
	m_socket = INVALID_SOCKET_VALUE;

	std::cout << "[RAWETH] Disconnected from " << m_host << ":" << m_port << "\n";
	return true;
}

bool RawEthProtocol::write(const std::string& command) {
	if (m_socket == INVALID_SOCKET_VALUE) {
		std::cerr << "[RAWETH] Not connected\n";
		return false;
	}

	std::string fullCommand = command;
	if (!command.empty() && command.back() != '\n') {
		fullCommand += '\n';
	}

	int totalSent = 0;
	int remaining = static_cast<int>(fullCommand.length());
	const char* ptr = fullCommand.c_str();

	while (remaining > 0) {
		int sent = send(m_socket, ptr + totalSent, remaining, 0);
		if (sent == SOCKET_ERROR_VALUE) {
#ifdef _WIN32
			std::cerr << "[RAWETH] Send failed: " << WSAGetLastError() << "\n";
#else
			std::cerr << "[RAWETH] Send failed: " << strerror(errno) << "\n";
#endif
			return false;
		}
		totalSent += sent;
		remaining -= sent;
	}

	std::cout << "[RAWETH] Command sent (" << totalSent << " bytes): " << command << "\n";
	return true;
}

bool RawEthProtocol::read(std::string& response) {
	if (m_socket == INVALID_SOCKET_VALUE) {
		std::cerr << "[RAWETH] Not connected\n";
		return false;
	}

	std::vector<char> buffer(4096);
	response.clear();

	while (true) {
		int bytesReceived = recv(m_socket, buffer.data(), static_cast<int>(buffer.size() - 1), 0);
		
		if (bytesReceived > 0) {
			buffer[bytesReceived] = '\0';
			response.append(buffer.data(), bytesReceived);
			
			if (response.find('\n') != std::string::npos) {
				break;
			}
		} else if (bytesReceived == 0) {
			std::cerr << "[RAWETH] Connection closed by remote host\n";
			return false;
		} else {
#ifdef _WIN32
			int error = WSAGetLastError();
			if (error == WSAETIMEDOUT) {
				std::cerr << "[RAWETH] Receive timeout\n";
			} else {
				std::cerr << "[RAWETH] Receive failed: " << error << "\n";
			}
#else
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				std::cerr << "[RAWETH] Receive timeout\n";
			} else {
				std::cerr << "[RAWETH] Receive failed: " << strerror(errno) << "\n";
			}
#endif
			return false;
		}
	}

	while (!response.empty() && (response.back() == '\n' || response.back() == '\r')) {
		response.pop_back();
	}

	std::cout << "[RAWETH] Response received (" << response.length() << " bytes): " << response << "\n";
	return true;
}

void RawEthProtocol::setTimeout(uint32_t timeout) {
	m_timeout = timeout;
	if (m_socket != INVALID_SOCKET_VALUE) {
#ifdef _WIN32
		DWORD timeout_ms = m_timeout;
		setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms));
		setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms));
#else
		struct timeval tv;
		tv.tv_sec = m_timeout / 1000;					
		tv.tv_usec = (m_timeout % 1000) * 1000;
		setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const void*)&tv, sizeof(tv));
		setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (const void*)&tv, sizeof(tv));
#endif
	}
}

void RawEthProtocol::setAddress(const std::string& address) {
	m_address = address;
	parseAddress();
}