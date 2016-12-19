#ifndef SMART_SCREEN_DAQ_INTERFACE_H_H
#define SMART_SCREEN_DAQ_INTERFACE_H_H

#ifdef __cplusplus
#define DAQ_INTERFACE_EXTERN_C extern "C"
#else
#define DAQ_INTERFACE_EXTERN_C

#endif

DAQ_INTERFACE_EXTERN_C void init_daq_interface();
DAQ_INTERFACE_EXTERN_C void addMEDALDataPoint(float current0,float current1,float current2,float current3,float current4,float current5,float voltage);


DAQ_INTERFACE_EXTERN_C void free_daq_interface();
#endif //SMART_SCREEN_DAQ_INTERFACE_H_H
