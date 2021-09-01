#ifndef __PZEM_H__
#define __PZEM_H__
#include <interval.h>

#define PZEM_RECEIVE_TIMEOUT 1000 // timeout pro prijem odpovedi z modulu [ms]
#define PZEM_PAUSE_TIMEOUT 200 // mezera mezi jednotlivymi dotazy na modul [ms]

enum
{
  RS_CONFIG = 0,
  RS_PAUSE1 = 1,
  RS_READVOLTAGE = 2,
  RS_PAUSE2 = 3,
  RS_READCURRENT = 4,
  RS_PAUSE3 = 5,
  RS_READPOWER = 6,
  RS_PAUSE4 = 7,
  RS_READENERGY = 8,
  RS_PAUSE5 = 9,
};

class Pzem
{
  public:
    Pzem(Stream &comms): _comms(comms), _ReceiveState(RS_CONFIG), _periode(3000), _rptr(0), _voltage(-1), _current(-1), _power(-1), _energy(0) {}
    void poll(void);
    void setperiode(uint32_t periode);
    float getVoltage(void);
    float getCurrent(void);
    float getPower(void);
    int32_t getEnergy(void);

  private:
    uint8_t _receive[7];
    uint8_t _rptr;
    Stream &_comms;
    float _voltage;
    float _current;
    float _power;
    int32_t _energy;
    uint32_t _periode;
    Interval _timeout;
    int _ReceiveState;
};
#endif
