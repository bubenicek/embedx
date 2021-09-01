// Komunikace
#include <Arduino.h>
#include "pzem.h"

#define DEBUG_OUT(a) {}
//#define DEBUG_OUT(a) Serial.print(a)

const uint8_t PZEM_SETADDRESS[] = {0xB4, 0xC0, 0xA8, 0x01, 0x02, 0x00, 0x1F};
const uint8_t PZEM_GETVOLTAGE[] = {0xB0, 0xC0, 0xA8, 0x01, 0x02, 0x00, 0x1B};
const uint8_t PZEM_GETCURRENT[] = {0xB1, 0xC0, 0xA8, 0x01, 0x02, 0x00, 0x1C};
const uint8_t PZEM_GETPOWER[] = {0xB2, 0xC0, 0xA8, 0x01, 0x02, 0x00, 0x1D};
const uint8_t PZEM_GETENERGY[] = {0xB3, 0xC0, 0xA8, 0x01, 0x02, 0x00, 0x1E};

void Pzem::poll(void)
{

  while ((_comms.available() > 0) && (_rptr < 7))
  {
    uint8_t b = _comms.read();
    _receive[_rptr] = b;
    ++_rptr;
  }
  if (7 == _rptr)
  {
    _rptr = 0;
    switch (_ReceiveState)
    {
      case RS_CONFIG:
        _timeout.set(PZEM_PAUSE_TIMEOUT);
        ++_ReceiveState;
        DEBUG_OUT("Address set.\r\n");
        break;

      case RS_READVOLTAGE:
        _voltage = (_receive[1] << 8) + _receive[2] + (_receive[3] / 10.0);
        _timeout.set(PZEM_PAUSE_TIMEOUT);
        ++_ReceiveState;
        DEBUG_OUT("Voltage read.\r\n");
        break;

      case RS_READCURRENT:
        _current = (_receive[1] << 8) + _receive[2] + (_receive[3] / 100.0);
        _timeout.set(PZEM_PAUSE_TIMEOUT);
        ++_ReceiveState;
        DEBUG_OUT("Current read.\r\n");
        break;

      case RS_READPOWER:
        _power = (_receive[1] << 8) + _receive[2];
        _timeout.set(PZEM_PAUSE_TIMEOUT);
        ++_ReceiveState;
        DEBUG_OUT("Power read.\r\n");
        break;

      case RS_READENERGY:
        _energy = (_receive[1] << 16) + (_receive[2] << 8) + _receive[3];
        _timeout.set(2000);
        _ReceiveState = RS_PAUSE1;
        DEBUG_OUT("Energy read.\r\n");
        break;
    }
  }
  switch (_ReceiveState)
  {
    case RS_PAUSE1:
      if (_timeout.expired())
      {
        _timeout.set(PZEM_RECEIVE_TIMEOUT);
        _ReceiveState = RS_READVOLTAGE;
        _comms.write(PZEM_GETVOLTAGE, sizeof(PZEM_GETVOLTAGE));
      }
      break;

    case RS_PAUSE2:
      if (_timeout.expired())
      {
        _timeout.set(PZEM_RECEIVE_TIMEOUT);
        _ReceiveState = RS_READCURRENT;
        _comms.write(PZEM_GETCURRENT, sizeof(PZEM_GETCURRENT));
      }
      break;

    case RS_PAUSE3:
      if (_timeout.expired())
      {
        _timeout.set(PZEM_RECEIVE_TIMEOUT);
        _ReceiveState = RS_READPOWER;
        _comms.write(PZEM_GETPOWER, sizeof(PZEM_GETPOWER));
      }
      break;

    case RS_PAUSE4:
      if (_timeout.expired())
      {
        _timeout.set(PZEM_RECEIVE_TIMEOUT);
        _ReceiveState = RS_READENERGY;
        _comms.write(PZEM_GETENERGY, sizeof(PZEM_GETENERGY));
      }
      break;

    default:
      if (_timeout.expired())
      {
        DEBUG_OUT("Protocol timeout in state ");
        DEBUG_OUT(_ReceiveState);
        DEBUG_OUT("\r\n");
        _ReceiveState = RS_CONFIG;
        _timeout.set(PZEM_RECEIVE_TIMEOUT);
        _comms.write(PZEM_SETADDRESS, sizeof(PZEM_SETADDRESS));
        _rptr = 0;
      }
      break;
  }
}

void Pzem::setperiode(uint32_t periode)
{

  _periode = periode;
}

float Pzem::getVoltage(void)
{

  return _voltage;
}

float Pzem::getCurrent(void)
{
  return _current;
}

float Pzem::getPower(void)
{
  return _power;
}

int32_t Pzem::getEnergy(void)
{
  return _energy;
}
