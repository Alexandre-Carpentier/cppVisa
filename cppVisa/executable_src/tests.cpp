/////////////////////////////////////////////////////////////////////////////
// Author:      Alexandre CARPENTIER
// Modified by:
// Created:     26/03/2026
// Copyright:   (c) Alexandre CARPENTIER
// Licence:     LGPL-2.1-or-later
/////////////////////////////////////////////////////////////////////////////

#include "..\library_src\cVisa.h"
#include <iostream>

int main()
{
	std::cout << "=== Test USB Protocol ===\n";
	cVisa visaUsb({
		.address = "USB0::0x1AB1::0x0588::DS1ZA170000000::INSTR",
		.timeout = 5000,
		.protocol = PROTOCOL::USB
	});

	visaUsb.connect();
	visaUsb.write("*IDN?");
	std::string response;
	visaUsb.read(response);
	visaUsb.disconnect();

	std::cout << "\n=== Test ETH Protocol ===\n";
	cVisa visaEth({
		.address = "TCPIP0::192.168.1.100::inst0::INSTR",
		.timeout = 3000,
		.protocol = PROTOCOL::ETH
	});

	visaEth.connect();
	visaEth.write("MEAS:VOLT?");
	visaEth.read(response);
	
	// Test des setters
	std::cout << "\n=== Test Setters ===\n";
	std::cout << "Original timeout: " << visaEth.getTimeout() << "ms\n";
	visaEth.setTimeout(10000);
	std::cout << "New timeout: " << visaEth.getTimeout() << "ms\n";

	std::cout << "Original address: " << visaEth.getAddress() << "\n";
	visaEth.setAddress("TCPIP0::192.168.1.101::inst0::INSTR");
	std::cout << "New address: " << visaEth.getAddress() << "\n";

	visaEth.disconnect();

	std::cout << "\n=== Test COM Protocol ===\n";
	cVisa visaCom({
		.address = "ASRL1::INSTR",
		.timeout = 2000,
		.protocol = PROTOCOL::COM
	});

	visaCom.connect();
	visaCom.write("READ?");
	visaCom.read(response);
	visaCom.disconnect();

	std::cout << "\n=== Test GPIB Protocol ===\n";
	cVisa visaGpib({
		.address = "GPIB0::10::INSTR",
		.timeout = 4000,
		.protocol = PROTOCOL::GPIB
	});

	visaGpib.connect();
	visaGpib.write("*RST");
	visaGpib.disconnect();

	return 0;
}
