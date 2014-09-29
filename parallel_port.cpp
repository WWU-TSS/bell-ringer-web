#include "parallel_port.h"
#include <linux/ioctl.h>
#include <linux/ppdev.h>
#include <linux/parport.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

using namespace std;

namespace
{
struct ParallelPortData
{
    int fd;
    ParallelPort::Pin currentPinWriteData;
    ParallelPortData(string portName)
    {
        fd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
        if(fd < 0)
        {
            int errnoValue = errno;
            throw ParallelPortError("can't open " + portName + " : " + strerror(errnoValue));
        }
        if(ioctl(fd, PPCLAIM) < 0)
        {
            int errnoValue = errno;
            close(fd);
            throw ParallelPortError("can't claim " + portName + " : " + strerror(errnoValue));
        }
        currentPinWriteData = readPins(ParallelPort::SignalingPins);
    }
    ~ParallelPortData()
    {
        ioctl(fd, PPRELEASE);
        close(fd);
    }
    ParallelPort::Pin readPins(ParallelPort::Pin pins)
    {
        unsigned char byte;
        ParallelPort::Pin retval = 0;
        if((pins & ParallelPort::ControlPins) != 0)
        {
            ioctl(fd, PPRCONTROL, &byte);
            if((byte & PARPORT_CONTROL_INIT) != 0)
                retval |= ParallelPort::InitializePin;
            if((byte & PARPORT_CONTROL_AUTOFD) == 0)
                retval |= ParallelPort::LineFeedPin;
            if((byte & PARPORT_CONTROL_SELECT) == 0)
                retval |= ParallelPort::SelectPin;
            if((byte & PARPORT_CONTROL_STROBE) == 0)
                retval |= ParallelPort::StrobePin;
        }
        if((pins & ParallelPort::StatusPins) != 0)
        {
            ioctl(fd, PPRSTATUS, &byte);
            if((byte & PARPORT_STATUS_ACK) != 0)
                retval |= ParallelPort::AckPin;
            if((byte & PARPORT_STATUS_BUSY) == 0)
                retval |= ParallelPort::BusyPin;
            if((byte & PARPORT_STATUS_PAPEROUT) != 0)
                retval |= ParallelPort::OutOfPaperPin;
            if((byte & PARPORT_STATUS_SELECT) != 0)
                retval |= ParallelPort::OnlinePin;
            if((byte & PARPORT_STATUS_ERROR) != 0)
                retval |= ParallelPort::ErrorPin;
        }
        if((pins & ParallelPort::DataPins) != 0)
        {
            ioctl(fd, PPRDATA, &byte);
            retval |= (ParallelPort::Pin)byte * ParallelPort::getDataPinConstant(0);
        }
        return retval & pins;
    }
    void writePins(ParallelPort::Pin pinmask, ParallelPort::Pin pinvalues)
    {
        unsigned char byte;
        ParallelPort::Pin lastdata = currentPinWriteData;
        currentPinWriteData = (currentPinWriteData & ~pinmask) | (pinvalues & pinmask);
        if((pinmask & ParallelPort::ControlPins) != 0)
        {
            if((pinmask & ParallelPort::ControlPins) == ParallelPort::ControlPins)
                byte = 0;
            else
                ioctl(fd, PPRCONTROL, &byte);
            if((pinmask & ParallelPort::InitializePin) != 0)
            {
                byte &= ~PARPORT_CONTROL_INIT;
                if((pinvalues & ParallelPort::InitializePin) != 0)
                    byte |= PARPORT_CONTROL_INIT;
            }
            if((pinmask & ParallelPort::SelectPin) != 0)
            {
                byte &= ~PARPORT_CONTROL_SELECT;
                if((pinvalues & ParallelPort::SelectPin) == 0)
                    byte |= PARPORT_CONTROL_SELECT;
            }
            if((pinmask & ParallelPort::LineFeedPin) != 0)
            {
                byte &= ~PARPORT_CONTROL_AUTOFD;
                if((pinvalues & ParallelPort::LineFeedPin) == 0)
                    byte |= PARPORT_CONTROL_AUTOFD;
            }
            if((pinmask & ParallelPort::StrobePin) != 0)
            {
                byte &= ~PARPORT_CONTROL_STROBE;
                if((pinvalues & ParallelPort::StrobePin) == 0)
                    byte |= PARPORT_CONTROL_STROBE;
            }
            ioctl(fd, PPWCONTROL, &byte);
        }
        if((pinmask & ParallelPort::DataPins) != 0)
        {
            lastdata &= ParallelPort::DataPins;
            pinmask &= ParallelPort::DataPins;
            pinvalues &= ParallelPort::DataPins;
            pinmask /= ParallelPort::getDataPinConstant(0);
            pinvalues /= ParallelPort::getDataPinConstant(0);
            lastdata /= ParallelPort::getDataPinConstant(0);
            byte = (lastdata & ~pinmask) | (pinmask & pinvalues);
            ioctl(fd, PPWDATA, &byte);
        }
    }
};
}

ParallelPort::Pin ParallelPort::readPins(Pin pins)
{
    return static_pointer_cast<ParallelPortData>(data)->readPins(pins);
}

void ParallelPort::writePins(Pin pins, Pin pinValues)
{
    static_pointer_cast<ParallelPortData>(data)->writePins(pins, pinValues);
}

ParallelPort::ParallelPort(string portName)
{
    if(portName == "LPT1")
    {
        portName = "/dev/parport0";
    }
    else if(portName == "LPT2")
    {
        portName = "/dev/parport1";
    }
    else if(portName == "LPT3")
    {
        portName = "/dev/parport2";
    }
    else if(portName == "LPT4")
    {
        portName = "/dev/parport3";
    }
    data = make_shared<ParallelPortData>(portName);
}
