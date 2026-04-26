# cppVisa - Modern C++ Wrapper for National Instrument VISA

## Description

A C++23 library that wraps the NI VISA API.
The idea is to use NI VISA library easily without all the C boilerplate.
Supports USBTMC, Ethernet (VXI-11/HiSLIP), Serial (RS-232), and GPIB.

## Features

### ✅ Architecture
- A pointer to implementation hide details (cVisaImpl)for a clean header. 
Refer to pimpl for documentation.

### ✅ Supported Protocols
1. **USB** - Communication via USB-TMC
2. **Ethernet (ETH)** - TCP/IP communication with VXI-11 or HiSLIP
3. **Serial (COM)** - RS-232 serial communication
4. **GPIB** - IEEE-488 communication

### NI VISA Specifics

#### USB Protocol:
- Timeout configuration (`VI_ATTR_TMO_VALUE`)

#### COM Protocol (Serial):
- Baud rate configuration (`VI_ATTR_ASRL_BAUD`)
- Data bits configuration (`VI_ATTR_ASRL_DATA_BITS`)
- Stop bits configuration (`VI_ATTR_ASRL_STOP_BITS`)
- Parity configuration (`VI_ATTR_ASRL_PARITY`)

#### Ethernet Protocol:
- Timeout configuration
- Termination character configuration (`VI_ATTR_TERMCHAR`)
- Enable termination character detection (`VI_ATTR_TERMCHAR_EN`)

#### GPIB Protocol:
- Timeout configuration
- `viClear()` - Bus clearing

## Usage

### Basic Example

```cpp
#include "cVisa.h"

int main()
{
    // USB Connection
    cVisa visa({
        .address = "USB0::0x1AB1::0x0588::DS1ZA170000000::INSTR",
        .timeout = 5000,
        .protocol = PROTOCOL::USB
    });

    if (visa.connect()) {
        visa.write("*IDN?");
        
        std::string response;
        visa.read(response);
        std::cout << "Instrument ID: " << response << "\n";
        
        visa.disconnect();
    }

    return 0;
}
```

### Protocol-specific Examples

#### USB
```cpp
cVisa visaUsb({
    .address = "USB0::0x1AB1::0x0588::DS1ZA170000000::INSTR",
    .timeout = 5000,
    .protocol = PROTOCOL::USB
});
```

#### Ethernet (TCPIP)
```cpp
cVisa visaEth({
    .address = "TCPIP0::192.168.1.100::inst0::INSTR",
    .timeout = 3000,
    .protocol = PROTOCOL::ETH
});
```

#### Serial (COM)
```cpp
cVisa visaCom({
    .address = "ASRL1::INSTR",
    .timeout = 2000,
    .protocol = PROTOCOL::COM
});
```

#### GPIB
```cpp
cVisa visaGpib({
    .address = "GPIB0::10::INSTR",
    .timeout = 4000,
    .protocol = PROTOCOL::GPIB
});
```

### Post-construction Customization

```cpp
cVisa visa({...});

// Change timeout after creation
visa.setTimeout(10000);
uint32_t currentTimeout = visa.getTimeout();

// Change address
visa.setAddress("TCPIP0::192.168.1.101::inst0::INSTR");
std::string currentAddress = visa.getAddress();

// Get protocol type
PROTOCOL proto = visa.getProtocol();
```

## Public API

### Constructor
```cpp
cVisa(configVISA config);
```

### Communication Methods
```cpp
bool connect();                          // Open connection
bool disconnect();                       // Close connection
bool write(const std::string& command);  // Send a command
bool read(std::string& response);        // Read a response
```

### Configuration Methods
```cpp
void setTimeout(uint32_t timeout);       // Change timeout (ms)
void setAddress(const std::string& address); // Change address
uint32_t getTimeout() const;             // Get timeout
std::string getAddress() const;          // Get address
PROTOCOL getProtocol() const;            // Get protocol
```

## Error Handling


Errors are automatically displayed with:
- The name of the failed operation
- A descriptive VISA error message
- The error code in hexadecimal

## Dependencies

- **NIVISA** : NI-VISA library (visa64.lib)
- **C++ Standard** : C++23
- **CMake** : Minimum version 3.8

## Build

```bash
cmake -B build -G Ninja
cmake --build build
```

## Technical Notes

## License

LGPL-2.1-or-later

## Author

Alexandre CARPENTIER
