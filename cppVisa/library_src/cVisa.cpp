/////////////////////////////////////////////////////////////////////////////
// Author:      Alexandre CARPENTIER
// Modified by:
// Created:     26/03/2026
// Copyright:   (c) Alexandre CARPENTIER
// Licence:     LGPL-2.1-or-later
/////////////////////////////////////////////////////////////////////////////

#include "cVisa.h"
#include <variant>
#include <string>
#include <vector>

// Helper function to check VISA status
inline bool checkVisaStatus(ViStatus status, const std::string& operation) {
	if (status < VI_SUCCESS) {
		char errMsg[256];
		viStatusDesc(VI_NULL, status, errMsg);
		std::cerr << "[VISA Error] " << operation << ": " << errMsg << " (0x" 
		          << std::hex << status << std::dec << ")\n";
		return false;
	}
	return true;
}

// Protocol implementations using static polymorphism with NIVISA
class UsbProtocol
{
public:
	UsbProtocol(const std::string& address, uint32_t timeout)
		: m_address(address), m_timeout(timeout), m_session(VI_NULL), m_defaultRM(VI_NULL)
	{
		std::cout << "[USB] Initializing with address: " << address << "\n";
		ViStatus status = viOpenDefaultRM(&m_defaultRM);
		if (!checkVisaStatus(status, "viOpenDefaultRM")) {
			throw std::runtime_error("Failed to open VISA resource manager");
		}
	}

	~UsbProtocol() {
		if (m_session != VI_NULL) {
			disconnect();
		}
		if (m_defaultRM != VI_NULL) {
			viClose(m_defaultRM);
		}
	}

	bool connect() {
		if (m_session != VI_NULL) {
			std::cout << "[USB] Already connected\n";
			return true;
		}

		ViStatus status = viOpen(m_defaultRM, const_cast<ViRsrc>(m_address.c_str()), 
		                         VI_NO_LOCK, m_timeout, &m_session);
		
		if (!checkVisaStatus(status, "viOpen")) {
			return false;
		}

		status = viSetAttribute(m_session, VI_ATTR_TMO_VALUE, m_timeout);
		checkVisaStatus(status, "viSetAttribute VI_ATTR_TMO_VALUE");

		std::cout << "[USB] Connected to " << m_address << "\n";
		return true;
	}

	bool disconnect() {
		if (m_session == VI_NULL) {
			return true;
		}

		ViStatus status = viClose(m_session);
		m_session = VI_NULL;
		
		if (!checkVisaStatus(status, "viClose")) {
			return false;
		}

		std::cout << "[USB] Disconnected from " << m_address << "\n";
		return true;
	}

	bool write(const std::string& command) {
		if (m_session == VI_NULL) {
			std::cerr << "[USB] Not connected\n";
			return false;
		}

		ViUInt32 retCount;
		ViStatus status = viWrite(m_session, 
		                          reinterpret_cast<ViBuf>(const_cast<char*>(command.c_str())),
		                          static_cast<ViUInt32>(command.length()), 
		                          &retCount);

		if (!checkVisaStatus(status, "viWrite")) {
			return false;
		}

		std::cout << "[USB] Command sent (" << retCount << " bytes): " << command << "\n";
		return true;
	}

	bool read(std::string& response) {
		if (m_session == VI_NULL) {
			std::cerr << "[USB] Not connected\n";
			return false;
		}

		std::vector<char> buffer(4096);
		ViUInt32 retCount;
		ViStatus status = viRead(m_session, 
		                         reinterpret_cast<ViBuf>(buffer.data()),
		                         static_cast<ViUInt32>(buffer.size()), 
		                         &retCount);

		if (!checkVisaStatus(status, "viRead")) {
			return false;
		}

		response.assign(buffer.data(), retCount);
		std::cout << "[USB] Response received (" << retCount << " bytes): " << response << "\n";
		return true;
	}

	void setTimeout(uint32_t timeout) { 
		m_timeout = timeout;
		if (m_session != VI_NULL) {
			viSetAttribute(m_session, VI_ATTR_TMO_VALUE, timeout);
		}
	}

	void setAddress(const std::string& address) { 
		m_address = address;
	}

	uint32_t getTimeout() const { return m_timeout; }
	std::string getAddress() const { return m_address; }

private:
	std::string m_address;
	uint32_t m_timeout;
	ViSession m_session;
	ViSession m_defaultRM;
};

class ComProtocol
{
public:
	ComProtocol(const std::string& address, uint32_t timeout)
		: m_address(address), m_timeout(timeout), m_session(VI_NULL), m_defaultRM(VI_NULL)
	{
		std::cout << "[COM] Initializing with address: " << address << "\n";
		ViStatus status = viOpenDefaultRM(&m_defaultRM);
		if (!checkVisaStatus(status, "viOpenDefaultRM")) {
			throw std::runtime_error("Failed to open VISA resource manager");
		}
	}

	~ComProtocol() {
		if (m_session != VI_NULL) {
			disconnect();
		}
		if (m_defaultRM != VI_NULL) {
			viClose(m_defaultRM);
		}
	}

	bool connect() {
		if (m_session != VI_NULL) {
			std::cout << "[COM] Already connected\n";
			return true;
		}

		ViStatus status = viOpen(m_defaultRM, const_cast<ViRsrc>(m_address.c_str()), 
		                         VI_NO_LOCK, m_timeout, &m_session);
		
		if (!checkVisaStatus(status, "viOpen")) {
			return false;
		}

		status = viSetAttribute(m_session, VI_ATTR_TMO_VALUE, m_timeout);
		checkVisaStatus(status, "viSetAttribute VI_ATTR_TMO_VALUE");

		status = viSetAttribute(m_session, VI_ATTR_ASRL_BAUD, 9600);
		checkVisaStatus(status, "viSetAttribute VI_ATTR_ASRL_BAUD");

		status = viSetAttribute(m_session, VI_ATTR_ASRL_DATA_BITS, 8);
		checkVisaStatus(status, "viSetAttribute VI_ATTR_ASRL_DATA_BITS");

		status = viSetAttribute(m_session, VI_ATTR_ASRL_STOP_BITS, VI_ASRL_STOP_ONE);
		checkVisaStatus(status, "viSetAttribute VI_ATTR_ASRL_STOP_BITS");

		status = viSetAttribute(m_session, VI_ATTR_ASRL_PARITY, VI_ASRL_PAR_NONE);
		checkVisaStatus(status, "viSetAttribute VI_ATTR_ASRL_PARITY");

		std::cout << "[COM] Connected to " << m_address << "\n";
		return true;
	}

	bool disconnect() {
		if (m_session == VI_NULL) {
			return true;
		}

		ViStatus status = viClose(m_session);
		m_session = VI_NULL;
		
		if (!checkVisaStatus(status, "viClose")) {
			return false;
		}

		std::cout << "[COM] Disconnected from " << m_address << "\n";
		return true;
	}

	bool write(const std::string& command) {
		if (m_session == VI_NULL) {
			std::cerr << "[COM] Not connected\n";
			return false;
		}

		ViUInt32 retCount;
		ViStatus status = viWrite(m_session, 
		                          reinterpret_cast<ViBuf>(const_cast<char*>(command.c_str())),
		                          static_cast<ViUInt32>(command.length()), 
		                          &retCount);

		if (!checkVisaStatus(status, "viWrite")) {
			return false;
		}

		std::cout << "[COM] Command sent (" << retCount << " bytes): " << command << "\n";
		return true;
	}

	bool read(std::string& response) {
		if (m_session == VI_NULL) {
			std::cerr << "[COM] Not connected\n";
			return false;
		}

		std::vector<char> buffer(4096);
		ViUInt32 retCount;
		ViStatus status = viRead(m_session, 
		                         reinterpret_cast<ViBuf>(buffer.data()),
		                         static_cast<ViUInt32>(buffer.size()), 
		                         &retCount);

		if (!checkVisaStatus(status, "viRead")) {
			return false;
		}

		response.assign(buffer.data(), retCount);
		std::cout << "[COM] Response received (" << retCount << " bytes): " << response << "\n";
		return true;
	}

	void setTimeout(uint32_t timeout) { 
		m_timeout = timeout;
		if (m_session != VI_NULL) {
			viSetAttribute(m_session, VI_ATTR_TMO_VALUE, timeout);
		}
	}

	void setAddress(const std::string& address) { 
		m_address = address;
	}

	uint32_t getTimeout() const { return m_timeout; }
	std::string getAddress() const { return m_address; }

private:
	std::string m_address;
	uint32_t m_timeout;
	ViSession m_session;
	ViSession m_defaultRM;
};

class EthProtocol
{
public:
	EthProtocol(const std::string& address, uint32_t timeout)
		: m_address(address), m_timeout(timeout), m_session(VI_NULL), m_defaultRM(VI_NULL)
	{
		std::cout << "[ETH] Initializing with address: " << address << "\n";
		ViStatus status = viOpenDefaultRM(&m_defaultRM);
		if (!checkVisaStatus(status, "viOpenDefaultRM")) {
			throw std::runtime_error("Failed to open VISA resource manager");
		}
	}

	~EthProtocol() {
		if (m_session != VI_NULL) {
			disconnect();
		}
		if (m_defaultRM != VI_NULL) {
			viClose(m_defaultRM);
		}
	}

	bool connect() {
		if (m_session != VI_NULL) {
			std::cout << "[ETH] Already connected\n";
			return true;
		}

		ViStatus status = viOpen(m_defaultRM, const_cast<ViRsrc>(m_address.c_str()), 
		                         VI_NO_LOCK, m_timeout, &m_session);
		
		if (!checkVisaStatus(status, "viOpen")) {
			return false;
		}

		status = viSetAttribute(m_session, VI_ATTR_TMO_VALUE, m_timeout);
		checkVisaStatus(status, "viSetAttribute VI_ATTR_TMO_VALUE");

		status = viSetAttribute(m_session, VI_ATTR_TERMCHAR_EN, VI_TRUE);
		checkVisaStatus(status, "viSetAttribute VI_ATTR_TERMCHAR_EN");

		status = viSetAttribute(m_session, VI_ATTR_TERMCHAR, '\n');
		checkVisaStatus(status, "viSetAttribute VI_ATTR_TERMCHAR");

		std::cout << "[ETH] Connected to " << m_address << "\n";
		return true;
	}

	bool disconnect() {
		if (m_session == VI_NULL) {
			return true;
		}

		ViStatus status = viClose(m_session);
		m_session = VI_NULL;
		
		if (!checkVisaStatus(status, "viClose")) {
			return false;
		}

		std::cout << "[ETH] Disconnected from " << m_address << "\n";
		return true;
	}

	bool write(const std::string& command) {
		if (m_session == VI_NULL) {
			std::cerr << "[ETH] Not connected\n";
			return false;
		}

		ViUInt32 retCount;
		ViStatus status = viWrite(m_session, 
		                          reinterpret_cast<ViBuf>(const_cast<char*>(command.c_str())),
		                          static_cast<ViUInt32>(command.length()), 
		                          &retCount);

		if (!checkVisaStatus(status, "viWrite")) {
			return false;
		}

		std::cout << "[ETH] Command sent (" << retCount << " bytes): " << command << "\n";
		return true;
	}

	bool read(std::string& response) {
		if (m_session == VI_NULL) {
			std::cerr << "[ETH] Not connected\n";
			return false;
		}

		std::vector<char> buffer(4096);
		ViUInt32 retCount;
		ViStatus status = viRead(m_session, 
		                         reinterpret_cast<ViBuf>(buffer.data()),
		                         static_cast<ViUInt32>(buffer.size()), 
		                         &retCount);

		if (!checkVisaStatus(status, "viRead")) {
			return false;
		}

		response.assign(buffer.data(), retCount);
		std::cout << "[ETH] Response received (" << retCount << " bytes): " << response << "\n";
		return true;
	}

	void setTimeout(uint32_t timeout) { 
		m_timeout = timeout;
		if (m_session != VI_NULL) {
			viSetAttribute(m_session, VI_ATTR_TMO_VALUE, timeout);
		}
	}

	void setAddress(const std::string& address) { 
		m_address = address;
	}

	uint32_t getTimeout() const { return m_timeout; }
	std::string getAddress() const { return m_address; }

private:
	std::string m_address;
	uint32_t m_timeout;
	ViSession m_session;
	ViSession m_defaultRM;
};

class GpibProtocol
{
public:
	GpibProtocol(const std::string& address, uint32_t timeout)
		: m_address(address), m_timeout(timeout), m_session(VI_NULL), m_defaultRM(VI_NULL)
	{
		std::cout << "[GPIB] Initializing with address: " << address << "\n";
		ViStatus status = viOpenDefaultRM(&m_defaultRM);
		if (!checkVisaStatus(status, "viOpenDefaultRM")) {
			throw std::runtime_error("Failed to open VISA resource manager");
		}
	}

	~GpibProtocol() {
		if (m_session != VI_NULL) {
			disconnect();
		}
		if (m_defaultRM != VI_NULL) {
			viClose(m_defaultRM);
		}
	}

	bool connect() {
		if (m_session != VI_NULL) {
			std::cout << "[GPIB] Already connected\n";
			return true;
		}

		ViStatus status = viOpen(m_defaultRM, const_cast<ViRsrc>(m_address.c_str()), 
		                         VI_NO_LOCK, m_timeout, &m_session);
		
		if (!checkVisaStatus(status, "viOpen")) {
			return false;
		}

		status = viSetAttribute(m_session, VI_ATTR_TMO_VALUE, m_timeout);
		checkVisaStatus(status, "viSetAttribute VI_ATTR_TMO_VALUE");

		viClear(m_session);

		std::cout << "[GPIB] Connected to " << m_address << "\n";
		return true;
	}

	bool disconnect() {
		if (m_session == VI_NULL) {
			return true;
		}

		ViStatus status = viClose(m_session);
		m_session = VI_NULL;
		
		if (!checkVisaStatus(status, "viClose")) {
			return false;
		}

		std::cout << "[GPIB] Disconnected from " << m_address << "\n";
		return true;
	}

	bool write(const std::string& command) {
		if (m_session == VI_NULL) {
			std::cerr << "[GPIB] Not connected\n";
			return false;
		}

		ViUInt32 retCount;
		ViStatus status = viWrite(m_session, 
		                          reinterpret_cast<ViBuf>(const_cast<char*>(command.c_str())),
		                          static_cast<ViUInt32>(command.length()), 
		                          &retCount);

		if (!checkVisaStatus(status, "viWrite")) {
			return false;
		}

		std::cout << "[GPIB] Command sent (" << retCount << " bytes): " << command << "\n";
		return true;
	}

	bool read(std::string& response) {
		if (m_session == VI_NULL) {
			std::cerr << "[GPIB] Not connected\n";
			return false;
		}

		std::vector<char> buffer(4096);
		ViUInt32 retCount;
		ViStatus status = viRead(m_session, 
		                         reinterpret_cast<ViBuf>(buffer.data()),
		                         static_cast<ViUInt32>(buffer.size()), 
		                         &retCount);

		if (!checkVisaStatus(status, "viRead")) {
			return false;
		}

		response.assign(buffer.data(), retCount);
		std::cout << "[GPIB] Response received (" << retCount << " bytes): " << response << "\n";
		return true;
	}

	void setTimeout(uint32_t timeout) { 
		m_timeout = timeout;
		if (m_session != VI_NULL) {
			viSetAttribute(m_session, VI_ATTR_TMO_VALUE, timeout);
		}
	}

	void setAddress(const std::string& address) { 
		m_address = address;
	}

	uint32_t getTimeout() const { return m_timeout; }
	std::string getAddress() const { return m_address; }

private:
	std::string m_address;
	uint32_t m_timeout;
	ViSession m_session;
	ViSession m_defaultRM;
};

// Variant type for all protocols
using ProtocolVariant = std::variant<UsbProtocol, ComProtocol, EthProtocol, GpibProtocol>;

// Visitors for operations
struct ConnectVisitor {
	bool operator()(auto& protocol) { return protocol.connect(); }
};

struct DisconnectVisitor {
	bool operator()(auto& protocol) { return protocol.disconnect(); }
};

struct WriteVisitor {
	const std::string& command;
	bool operator()(auto& protocol) { return protocol.write(command); }
};

struct ReadVisitor {
	std::string& response;
	bool operator()(auto& protocol) { return protocol.read(response); }
};

struct SetTimeoutVisitor {
	uint32_t timeout;
	void operator()(auto& protocol) { protocol.setTimeout(timeout); }
};

struct SetAddressVisitor {
	const std::string& address;
	void operator()(auto& protocol) { protocol.setAddress(address); }
};

struct GetTimeoutVisitor {
	uint32_t operator()(const auto& protocol) { return protocol.getTimeout(); }
};

struct GetAddressVisitor {
	std::string operator()(const auto& protocol) { return protocol.getAddress(); }
};

// Factory function
ProtocolVariant createProtocol(PROTOCOL protocol, const std::string& address, uint32_t timeout)
{
	switch (protocol)
	{
	case PROTOCOL::USB:
		return UsbProtocol(address, timeout);
	case PROTOCOL::COM:
		return ComProtocol(address, timeout);
	case PROTOCOL::ETH:
		return EthProtocol(address, timeout);
	case PROTOCOL::GPIB:
		return GpibProtocol(address, timeout);
	default:
		return UsbProtocol(address, timeout);
	}
}

// Implementation class
class cVisa::cVisaImpl
{
public:
	cVisaImpl(configVISA config)
		: m_protocol(createProtocol(config.protocol, config.address, config.timeout))
		, m_protocolType(config.protocol)
	{
		std::cout << "[*] VISA instance created.\n";
	}

	bool connect() {
		return std::visit(ConnectVisitor{}, m_protocol);
	}

	bool disconnect() {
		return std::visit(DisconnectVisitor{}, m_protocol);
	}

	bool write(const std::string& command) {
		return std::visit(WriteVisitor{command}, m_protocol);
	}

	bool read(std::string& response) {
		return std::visit(ReadVisitor{response}, m_protocol);
	}

	void setTimeout(uint32_t timeout) {
		std::visit(SetTimeoutVisitor{timeout}, m_protocol);
	}

	void setAddress(const std::string& address) {
		std::visit(SetAddressVisitor{address}, m_protocol);
	}

	uint32_t getTimeout() const {
		return std::visit(GetTimeoutVisitor{}, m_protocol);
	}

	std::string getAddress() const {
		return std::visit(GetAddressVisitor{}, m_protocol);
	}

	PROTOCOL getProtocol() const {
		return m_protocolType;
	}

private:
	ProtocolVariant m_protocol;
	PROTOCOL m_protocolType;
};

// cVisa public interface
cVisa::cVisa(configVISA config) 
	: pvisa(std::make_unique<cVisaImpl>(config))
{
}

cVisa::~cVisa()
{
}

bool cVisa::connect()
{
	return pvisa->connect();
}

bool cVisa::disconnect()
{
	return pvisa->disconnect();
}

bool cVisa::write(const std::string& command)
{
	return pvisa->write(command);
}

bool cVisa::read(std::string& response)
{
	return pvisa->read(response);
}

void cVisa::setTimeout(uint32_t timeout)
{
	pvisa->setTimeout(timeout);
}

void cVisa::setAddress(const std::string& address)
{
	pvisa->setAddress(address);
}

uint32_t cVisa::getTimeout() const
{
	return pvisa->getTimeout();
}

std::string cVisa::getAddress() const
{
	return pvisa->getAddress();
}

PROTOCOL cVisa::getProtocol() const
{
	return pvisa->getProtocol();
}