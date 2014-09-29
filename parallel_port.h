#ifndef PARALLEL_PORT_H_INCLUDED
#define PARALLEL_PORT_H_INCLUDED

#include <cstdint>
#include <cassert>
#include <memory>
#include <string>
#include <stdexcept>

using namespace std;

struct ParallelPortError : public runtime_error
{
    explicit ParallelPortError(const string &msg)
        : runtime_error(msg)
    {
    }
};

class ParallelPort
{
    shared_ptr<void> data;
    ParallelPort(const ParallelPort &) = delete;
    const ParallelPort & operator =(const ParallelPort &) = delete;
public:
    typedef uint_fast32_t Pin;
    static constexpr Pin Pin1 = (Pin)0x1;
    static constexpr Pin Pin2 = (Pin)0x2;
    static constexpr Pin Pin3 = (Pin)0x4;
    static constexpr Pin Pin4 = (Pin)0x8;
    static constexpr Pin Pin5 = (Pin)0x10;
    static constexpr Pin Pin6 = (Pin)0x20;
    static constexpr Pin Pin7 = (Pin)0x40;
    static constexpr Pin Pin8 = (Pin)0x80;
    static constexpr Pin Pin9 = (Pin)0x100;
    static constexpr Pin Pin10 = (Pin)0x200;
    static constexpr Pin Pin11 = (Pin)0x400;
    static constexpr Pin Pin12 = (Pin)0x800;
    static constexpr Pin Pin13 = (Pin)0x1000;
    static constexpr Pin Pin14 = (Pin)0x2000;
    static constexpr Pin Pin15 = (Pin)0x4000;
    static constexpr Pin Pin16 = (Pin)0x8000;
    static constexpr Pin Pin17 = (Pin)0x10000;
    static constexpr Pin Pin18 = (Pin)0x20000;
    static constexpr Pin Pin19 = (Pin)0x40000;
    static constexpr Pin Pin20 = (Pin)0x80000;
    static constexpr Pin Pin21 = (Pin)0x100000;
    static constexpr Pin Pin22 = (Pin)0x200000;
    static constexpr Pin Pin23 = (Pin)0x400000;
    static constexpr Pin Pin24 = (Pin)0x800000;
    static constexpr Pin Pin25 = (Pin)0x1000000;
    static constexpr Pin GroundPins = (Pin)0x1FE0000;
    static constexpr Pin SignalingPins = (Pin)0x1FFFF;
    static constexpr Pin InputPins = (Pin)0x5FFE;
    static constexpr Pin OutputPins = (Pin)0x1A1FF;
    static constexpr Pin DataPins = (Pin)0x1FE;
    static constexpr Pin ControlPins = (Pin)0x1A001;
    static constexpr Pin StatusPins = (Pin)0x5E00;
    static constexpr Pin StrobePin = Pin1;
    static constexpr Pin AckPin = Pin10;
    static constexpr Pin BusyPin = Pin11;
    static constexpr Pin OutOfPaperPin = Pin12;
    static constexpr Pin OnlinePin = Pin13;
    static constexpr Pin LineFeedPin = Pin14;
    static constexpr Pin ErrorPin = Pin15;
    static constexpr Pin InitializePin = Pin16;
    static constexpr Pin SelectPin = Pin17;
    static constexpr Pin getPinConstant(int pinNumber)
    {
        return (Pin)1 << (pinNumber - 1);
    }
    static constexpr Pin getDataPinConstant(int bitNumber)
    {
        return getPinConstant(bitNumber + 2);
    }
    static constexpr bool isSinglePin(Pin pins)
    {
        return pins != 0 && (pins & (pins - 1)) == 0;
    }
    static int getPinNumber(Pin pin)
    {
        switch(pin)
        {
        case Pin1:
            return 1;
        case Pin2:
            return 2;
        case Pin3:
            return 3;
        case Pin4:
            return 4;
        case Pin5:
            return 5;
        case Pin6:
            return 6;
        case Pin7:
            return 7;
        case Pin8:
            return 8;
        case Pin9:
            return 9;
        case Pin10:
            return 10;
        case Pin11:
            return 11;
        case Pin12:
            return 12;
        case Pin13:
            return 13;
        case Pin14:
            return 14;
        case Pin15:
            return 15;
        case Pin16:
            return 16;
        case Pin17:
            return 17;
        case Pin18:
            return 18;
        case Pin19:
            return 19;
        case Pin20:
            return 20;
        case Pin21:
            return 21;
        case Pin22:
            return 22;
        case Pin23:
            return 23;
        case Pin24:
            return 24;
        case Pin25:
            return 25;
        default:
            assert(false);
            return -1;
        }
    }
    Pin readPins(Pin pins);
    bool readPin(Pin pin)
    {
        assert(isSinglePin(pin));
        return readPins(pin) != (Pin)0;
    }
    void writePins(Pin pins, Pin pinValues);
    void writePin(Pin pin, bool value)
    {
        assert(isSinglePin(pin));
        writePins(pin, value ? pin : 0);
    }
    class PinReference
    {
        friend class ParallelPort;
        ParallelPort * pport;
        Pin pin;
        PinReference(ParallelPort * pport, Pin pin)
            : pport(pport), pin(pin)
        {
            assert(isSinglePin(pin));
        }
        PinReference(const PinReference &) = delete;
    public:
        PinReference(PinReference && rt)
            : pport(rt.pport), pin(rt.pin)
        {
        }
        bool operator !()
        {
            return !pport->readPin(pin);
        }
        bool operator ~()
        {
            return !pport->readPin(pin);
        }
        operator bool()
        {
            return pport->readPin(pin);
        }
        void operator =(PinReference & rt)
        {
            pport->writePin(pin, rt.pport->readPin(rt.pin));
        }
        void operator =(bool value)
        {
            pport->writePin(pin, value);
        }
    };
    ParallelPort(string portName);
    PinReference operator [](Pin pin)
    {
        return PinReference(this, pin);
    }
};

#endif // PARALLEL_PORT_H_INCLUDED
