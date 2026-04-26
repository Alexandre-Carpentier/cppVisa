// ****************************************************************************
//    NI-ModInst
//
//    National Instruments utility library for Modular Instruments Drivers
//
//    Copyright (c) National Instruments 2004-2019.  All Rights Reserved.
// ****************************************************************************

#ifndef ___niModInst_h___
#define ___niModInst_h___


#if defined(__cplusplus) || defined(__cplusplus__)
extern "C" {
#endif

#define IVI_DO_NOT_INCLUDE_VISA_HEADERS
#include <IviVisaType.h>
#undef IVI_DO_NOT_INCLUDE_VISA_HEADERS


#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)) && !defined(_NI_mswin16_)
   #ifndef DllExport
      #define DllExport    __declspec( dllexport )
   #endif
#endif

// ****************************************************************************
// --------------------------- Attribute Definitions --------------------------
// ****************************************************************************

// ViString Attributes
#define NIMODINST_ATTR_DEVICE_NAME                                       0                                             /* 0 (0x0), ViString, read-only   */
#define NIMODINST_ATTR_DEVICE_MODEL                                      1                                             /* 1 (0x1), ViString, read-only   */
#define NIMODINST_ATTR_SERIAL_NUMBER                                     2                                             /* 2 (0x2), ViString, read-only   */

// ViInt32 Attributes
#define NIMODINST_ATTR_SLOT_NUMBER                                       10                                            /* 10 (0xa), ViInt32, read-only   */
#define NIMODINST_ATTR_CHASSIS_NUMBER                                    11                                            /* 11 (0xb), ViInt32, read-only   */
#define NIMODINST_ATTR_BUS_NUMBER                                        12                                            /* 12 (0xc), ViInt32, read-only   */
#define NIMODINST_ATTR_SOCKET_NUMBER                                     13                                            /* 13 (0xd), ViInt32, read-only   */
#define NIMODINST_ATTR_PCIEXPRESS_LINK_WIDTH                             17                                            /* 17 (0x11), ViInt32, read-only  */
#define NIMODINST_ATTR_MAX_PCIEXPRESS_LINK_WIDTH                         18                                            /* 18 (0x12), ViInt32, read-only  */


// ****************************************************************************
// ------------------------- Library Function Declarations --------------------
// ****************************************************************************

// --- Session Based Functions
ViStatus DllExport _VI_FUNC niModInst_OpenInstalledDevicesSession(
        ViConstString driver,
        ViSession* handle,
        ViInt32* deviceCount
    );

ViStatus DllExport _VI_FUNC niModInst_GetInstalledDeviceAttributeViString(
        ViSession handle,
        ViInt32 index,
        ViInt32 attributeId,
        ViInt32 attributeValueBufferSize,
        ViChar attributeValue[]
    );

ViStatus DllExport _VI_FUNC niModInst_GetInstalledDeviceAttributeViInt32(
        ViSession handle,
        ViInt32 index,
        ViInt32 attributeId,
        ViInt32* attributeValue
    );

ViStatus DllExport _VI_FUNC niModInst_CloseInstalledDevicesSession(
        ViSession handle
    );


// --- Utility Functions
ViStatus DllExport _VI_FUNC niModInst_GetExtendedErrorInfo(
        ViInt32 errorInfoBufferSize,
        ViChar errorInfo[]
    );




// ****************************************************************************
// ----------------------- Error And Completion Codes -------------------------
// ****************************************************************************
#define NIMODINST_SUCCESS                                                0                                             /* 0 (0x0)                        */
#define NIMODINST_ERROR_INVALID_SESSION_HANDLE                           -250200                                       /* -250200 (-0x3d158)             */
#define NIMODINST_ERROR_NULL_SESSION_HANDLE                              -250201                                       /* -250201 (-0x3d159)             */
#define NIMODINST_ERROR_INVALID_DEVICE_INDEX                             -250202                                       /* -250202 (-0x3d15a)             */
#define NIMODINST_ERROR_INVALID_ATTRIBUTE_ID                             -250203                                       /* -250203 (-0x3d15b)             */
#define NIMODINST_ERROR_ATTRIBUTE_NOT_SUPPORTED_BY_DEVICE                -250204                                       /* -250204 (-0x3d15c)             */
#define NIMODINST_ERROR_NULL_OUTPUT_PARAMETER                            -250205                                       /* -250205 (-0x3d15d)             */
#define NIMODINST_ERROR_NULL_DRIVER                                      -250206                                       /* -250206 (-0x3d15e)             */
#define NIMODINST_ERROR_INCORRECT_ATTRIBUTE_TYPE                         -250207                                       /* -250207 (-0x3d15f)             */


#if defined(__cplusplus) || defined(__cplusplus__)
}
#endif

#endif /* ___niModInst_h___ */



