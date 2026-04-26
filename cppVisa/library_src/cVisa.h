/////////////////////////////////////////////////////////////////////////////
// Author:      Alexandre CARPENTIER
// Modified by:
// Created:     26/03/2026
// Copyright:   (c) Alexandre CARPENTIER
// Licence:     LGPL-2.1-or-later
/////////////////////////////////////////////////////////////////////////////
#pragma once
#define CVISA_EXPORTS
#ifdef CVISA_EXPORTS
#define CVISA_API __declspec(dllexport)
#else
#define CVISA_API __declspec(dllimport)
#endif

#include <iostream>
#include <string>
#include <memory>
#include <variant>
#include <optional>

#include "visa.h"

enum class PROTOCOL 
{
	USB=0,
	COM, 
	ETH,
	GPIB
};

// Instanciate like this:
//cVisa visaEth({
//	.address = "TCPIP0::192.168.1.100::inst0::INSTR",
//	.timeout = 3000,
//	.protocol = PROTOCOL::ETH
//	});

struct configVISA
{
	std::string address;
	uint32_t timeout;
	PROTOCOL protocol;
};

class CVISA_API cVisa
{
public:
	cVisa(configVISA config);
	~cVisa();

	bool connect();
	bool disconnect();
	bool write(const std::string& command);
	bool read(std::string& response);

	void setTimeout(uint32_t timeout);
	void setAddress(const std::string& address);
	uint32_t getTimeout() const;
	std::string getAddress() const;
	PROTOCOL getProtocol() const;

private:
	class cVisaImpl;// Opaque ptr
	std::unique_ptr<cVisaImpl> pvisa;
};
