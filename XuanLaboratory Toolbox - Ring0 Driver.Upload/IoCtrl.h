#include <ntddk.h>

#define INIT_DRIVER CTL_CODE(FILE_DEVICE_UNKNOWN,0x19491001,METHOD_BUFFERED,FILE_ALL_ACCESS)
#define GET_DRIVER_LAST_MSG CTL_CODE(FILE_DEVICE_UNKNOWN,0x19491002,METHOD_BUFFERED,FILE_ALL_ACCESS)
#define PROCESS_OPERATION CTL_CODE(FILE_DEVICE_UNKNOWN,0x19491003,METHOD_BUFFERED,FILE_ALL_ACCESS)
#define SET_CLIENT_PROC_PROTECTION_METHOD CTL_CODE(FILE_DEVICE_UNKNOWN,0x19491004,METHOD_BUFFERED,FILE_ALL_ACCESS)
#define KERNEL_INFO_QUERY CTL_CODE(FILE_DEVICE_UNKNOWN,0x19491005,METHOD_BUFFERED,FILE_ALL_ACCESS)