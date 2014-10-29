//
//  MooshimeterProfileTypes.h
//  Mooshimeter
//
//  Created by James Whong on 11/26/13.
//  Copyright (c) 2013 mooshim. All rights reserved.
//


#ifndef Mooshimeter_MooshimeterProfileTypes_h
#define Mooshimeter_MooshimeterProfileTypes_h

#ifndef __IAR_SYSTEMS_ICC__
#define LO_UINT16(i) ((i   ) & 0xFF)
#define HI_UINT16(i) ((i>>8) & 0xFF)
#endif

#define MOOSHIM_BASE_UUID_128( uuid )  0xd4, 0xdb, 0x05, 0xe0, 0x54, 0xf2, 0x11, 0xe4, \
                                  0xab, 0x62, 0x00, 0x02, LO_UINT16( uuid ), HI_UINT16( uuid ), 0xc5, 0x1b                                   

#define METER_SERVICE_UUID  0xFFA0
#define METER_INFO          0xFFA1
#define METER_NAME          0xFFA2
#define METER_SETTINGS      0xFFA3
#define METER_LOG_SETTINGS  0xFFA4
#define METER_SAMPLE        0xFFA5
#define METER_CH1BUF        0xFFA6
#define METER_CH2BUF        0xFFA7
#define METER_CAL           0xFFA8
#define METER_LOG_DATA      0xFFA9
#define METER_TEMP          0xFFAA
#define METER_BAT           0xFFAB

#define METER_DEBUG_MSG     0xFFAF
                                    
#define METER_NAME_LEN 16

#define METER_INFO_DEFAULT {\
METER_PCB_VERSION,\
0,\
0,\
BUILD_TIME,\
{0,0,0,0,0,0,0,0,0,0,0,0}}

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

#ifdef __IAR_SYSTEMS_ICC__
#include "int24.h"
#else
typedef unsigned char uint8;
typedef unsigned short uint16;

typedef signed char int8;
typedef signed short int16;

typedef union {
    char bytes[3];
} int24_test;
#endif

// XCode 64 bit transition makes longs 8 bytes wide
// But IAR for 8051 calls int 2 bytes wide, so we need some compiler
// specific switches
#ifndef __IAR_SYSTEMS_ICC__
typedef unsigned int uint32;
typedef signed int int32;
#endif

typedef enum
#ifndef __IAR_SYSTEMS_ICC__
: uint8
#endif
{
    METER_SHUTDOWN=0,   // Booting from power down.
    METER_STANDBY,      // uC sleeping, ADC powered down.  Wakes to advertise occasionally.
    METER_PAUSED,       // uC active, ADC active, not sampling
    METER_RUNNING,      // uC active, ADC active, sampling until buffer is full, then performing computations and repeating
} meter_state_t;

typedef enum
#ifndef __IAR_SYSTEMS_ICC__
: uint8
#endif
{
  LOGGING_OFF=0,
  LOG_CALC,
  LOG_BUF,
} logging_state_t;

typedef enum 
#ifndef __IAR_SYSTEMS_ICC__
: uint8
#endif
{
  LOGGING_NO_MEDIA=0,
  LOGGING_READY,
  LOGGING_INSUFFICIENT_SPACE,
  LOGGING_WRITE_ERROR,
} logging_error_t;

#define TRIGGER_SETTING_SRC_OFF      (0x00)
#define TRIGGER_SETTING_SRC_CH1      (0x01)
#define TRIGGER_SETTING_SRC_CH2      (0x02)
#define TRIGGER_SETTING_EDGE_RISING  (0x00 <<2)
#define TRIGGER_SETTING_EDGE_FALLING (0x01 <<2)
#define TRIGGER_SETTING_EDGE_EITHER  (0x02 <<2)

typedef struct {
    uint8      setting;        
    uint16     x_offset;       // TODO: x offset for the trigger point to rest at
    int24_test crossing;       // Value at which to trigger
}
#ifndef __IAR_SYSTEMS_ICC__
__attribute__((packed))
#endif
trigger_settings_t;

typedef struct {
    uint8 pcb_version;
    uint8 assembly_variant;
    uint16 lot_number;
    uint32 build_time;
    uint8 reserved[12];
}
#ifndef __IAR_SYSTEMS_ICC__
__attribute__((packed))
#endif
MeterInfo_t;

/*
* Signal chains include: 
* WORLD -> 10M DIVIDER -> PGA -> ADC
* WORLD -> 1mOhm SENSE RES -> CURRENT SENSE AMP -> PGA -> ADC
* WORLD -> PGA -> ADC
*
* We can get PGA and ADC gain by putting known voltage on CH3 and mapping
* Ch1 and CH2 to it.  Then we can get the divider ratios by applying known test
* current and voltages.
*
* Stage 1: Determine CH1 and CH2 offsets
*   Short ACTIVE and VOLTAGE to COMMON
*   For each PGA gain, determine the offset and populate ch1_offsets and ch2_offsets
* Stage 2:  Determine CH3 offsets
*   Short CH3 to COMMON
*   Map CH1 and CH2 to CH3
*   For each PGA gain, determine the offset and populate ch1_3_offsets and ch2_3_offsets
* Stage 3:  Determine internal gains
*   Apply 100mV from CH3 to COMMON
*   Map CH1 and CH2 to CH3
*   For each PGA gain, determine the gain error and populate ch1_gain
* Stage 4:  Determine external gains
*   Apply 100mA test current and 5V test voltage
*   Sample CH1, calculate current gain, populate ch1_isns_gain
*   Sample CH2 at 60V setting, calculate voltage divider gain, populate ch2_60v_gain
*   Sample CH2 at 600V setting, calculate voltage divider gain, populate ch2_600v_gain
* Stage 5:  Record the temperature
*/

#define METER_FAKE_CAL { \
0,\
{{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}},\
{0,0,0,0,0,0,0},\
{0,0,0,0,0,0,0},\
{{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}},\
{{0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000},{0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,}},\
0x8000,\
0x8000,\
0x8000,\
0x8000,\
0x8000\
}

// The 7 indices refer to the 7 possible PGA settings
// The gains below are 16 bit fixed point values representing a number
// between 0 and 2
typedef struct {
  uint16 cal_temp;
  int16 ch_offsets[2][7];
  int16 ch2_lv_offsets[7];
  int16 ch2_hv_offsets[7];
  int16 ch_3_offsets[2][7];
  uint16 ch_gain[2][7];     // Gains are fixed point between 0 and 2
  uint16 ch1_isns_gain;
  uint16 ch2_60v_gain;
  uint16 ch2_600v_gain;
  uint16 ch2_100na_gain;
  uint16 ch2_100ua_gain;
}
#ifndef __IAR_SYSTEMS_ICC__
__attribute__((packed))
#endif
MeterFactoryCal_t;

#define METER_DEFAULT_SETTINGS {\
  METER_SHUTDOWN,\
  METER_SHUTDOWN,\
  0x00,\
  0x07,\
}

#define METER_MEASURE_SETTINGS_ISRC_ON         0x01
#define METER_MEASURE_SETTINGS_ISRC_LVL        0x02
#define METER_MEASURE_SETTINGS_ACTIVE_PULLDOWN 0x04
#define METER_SETTINGS_CH1_AUTORANGE           0x08
#define METER_SETTINGS_CH2_AUTORANGE           0x10

#define METER_CALC_SETTINGS_DEPTH_LOG2 0x0F
#define METER_CALC_SETTINGS_MEAN       0x10
#define METER_CALC_SETTINGS_ONESHOT    0x20

#define ADC_SETTINGS_SAMPLERATE_MASK 0x07
#define ADC_SETTINGS_GPIO_MASK 0x30

typedef struct {
  struct {
    meter_state_t present_meter_state;   // The state of the meter right now.
  } ro;
  struct {
    meter_state_t target_meter_state;    // The target state of the meter
    trigger_settings_t trigger_settings; // Trigger control
    uint8 measure_settings;              // Specifies features to turn on and off.  Note that voltage gain is controlled through ADC settings
    uint8 calc_settings;                 // Specifies what analysis to run on captured data
    uint8 ch1set;
    uint8 ch2set;
    uint8 adc_settings;
  } rw;
}
#ifndef __IAR_SYSTEMS_ICC__
__attribute__((packed)) 
#endif
MeterSettings_t;

#define DEFAULT_LOG_SETTINGS {\
{ 0,\
  LOGGING_OFF,\
  LOGGING_NO_MEDIA},\
{ LOGGING_OFF,\
  0,\
  0} }
  

typedef struct {
  struct {
    uint8 sd_present;
    logging_state_t present_logging_state;
    logging_error_t logging_error;
  } ro;
  struct {
    logging_state_t  target_logging_state;        
    uint16 logging_period_ms;              // How long to wait between taking log samples
    uint32 logging_n_cycles;               // How many samples to take before sleeping forever
  } rw;
}
#ifndef __IAR_SYSTEMS_ICC__
__attribute__((packed)) 
#endif
MeterLogSettings_t;

typedef struct {
    int24_test ch1_reading_lsb; // Mean of the sample buffer
    int24_test ch2_reading_lsb; // Mean of the sample buffer
}
#ifndef __IAR_SYSTEMS_ICC__
__attribute__((packed)) 
#endif
MeterMeasurement_t;

#endif
