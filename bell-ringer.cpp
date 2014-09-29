/***************************************************************************
 *            main.cc
 *
 *  Friday June 5, 2009
 *  Rob Frohne
 *  Rob.Frohne@wallawalla.edu
 *
 *  Tuesday August 5, 2014
 *  ported to use device file to no longer need root
 *  Jacob Lifshay
 *  programmerjake@gmail.com
 ****************************************************************************/
/* This program is to make the Administration building bell chime the hour
 * like a clock.  It can also ring the bell a given number of times, and them go
 * on to do its thing as a clock afterwards. */
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*  This program is to control the bell on the Administration Building.*/
#include <time.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#ifdef USE_PARALLEL_PORT
#undef USE_GPIO
#include "parallel_port.h"
#elif defined(USE_GPIO)
#include <sstream>
#include <fstream>
#else
#error invalid port : define USE_PARALLEL_PORT or USE_GPIO
#endif
#include <thread> // for this_thread::sleep_*
#include <chrono>

using namespace std;

#ifdef USE_PARALLEL_PORT
/*  The bell parallel port interface uses the following pin:
        Pin 16
*/
static constexpr ParallelPort::Pin BellPin = ParallelPort::Pin16;
typedef ParallelPort IOPort;
#elif defined(USE_GPIO)
static constexpr int BellPin = 18;
struct IOPort
{
    const int pinNumber;
    bool isOutput;
    string getGPIOPinDir()
    {
        ostringstream os;
        os << "/sys/class/gpio/gpio" << pinNumber << "/";
        return os.str();
    }
    void setIsOutput(bool isOutput)
    {
        this->isOutput = isOutput;
        ofstream os((getGPIOPinDir() + "direction").c_str());
        os << (isOutput ? "out" : "in") << endl;
        os.close();
    }
    IOPort(int pinNumber, bool isOutput)
        : pinNumber(pinNumber)
    {
        ofstream os("/sys/class/gpio/export");
        os << pinNumber << endl;
        os.close();
        setIsOutput(isOutput);
    }
    ~IOPort()
    {
        ofstream os("/sys/class/gpio/unexport");
        os << pinNumber << endl;
    }
    void write(bool value)
    {
        if(!isOutput)
            setIsOutput(true);
        ofstream os((getGPIOPinDir() + "value").c_str());
        os << (value ? "1" : "0") << endl;
        os.close();
    }
    bool read()
    {
        ifstream is((getGPIOPinDir() + "value").c_str());
        int v = 0;
        is >> v;
        if(!is)
            return false;
        return v != 0;
    }
    static int getPinNumber(int v)
    {
        return v;
    }
};
#endif

/* To use this command, you need to > ./bell-ringer -b8 -e21
 * assuming those are the options you want.
 * Options are:
 * -? for help
 * -v or --verbose for verbose response.
 * -d or --delay for the amount of delay between swings in seconds
 * -b or --beginning-hour for the beginning hour in the morning in 24 hour format
 * -e or --ending-hour for the end hour in the evening in 24 hour format
 * -o or --valve-time-ms for the hammer down time in milliseconds
 * -r for immediately ringing the bell argument times
 */
/* Flag set by '--verbose'. */
static int verbose_flag;
// Specify options and their syntax
static struct option long_options[] =
{
    /* These options set a flag. */
    {"verbose",         no_argument,       &verbose_flag, 1},
    {"brief",           no_argument,       &verbose_flag, 0},
    /* These options don't set a flag.
    We distinguish them by their indices. */
    {"help",            no_argument,       0, '?'},
    {"delay",           required_argument, 0, 'd'},
    {"beginning-hour",  required_argument, 0, 'b'},
    {"ending-hour",     required_argument, 0, 'e'},
    {"valve-time-ms",   required_argument, 0, 'o'},
    {"ring-now",        optional_argument, 0, 'r'},
    {0, 0, 0, 0}
};

int ring(IOPort & ioport, int chime_number, int swing_delay_ms, int hammer_on_ms)
{
    for(int i = 0; i < chime_number; i++)
    {
#ifdef USE_PARALLEL_PORT
        ioport[BellPin] = true;
#elif defined(USE_GPIO)
        ioport.write(true);
#endif
        this_thread::sleep_for(chrono::milliseconds(hammer_on_ms));
#ifdef USE_PARALLEL_PORT
        ioport[BellPin] = false;
#elif defined(USE_GPIO)
        ioport.write(false);
#endif

        if(verbose_flag)
        {
            std::cout << "Dong!\n" << std::endl;
        }

        this_thread::sleep_for(chrono::milliseconds(swing_delay_ms - hammer_on_ms));
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int sleep_seconds, swing_delay_ms = 4000, begin_hour = 8, end_hour = 22;
    int hammer_on_ms = 350, chime_number, ring_now_number = 0;
    time_t curtime;
    struct tm *loctime;
    // Iterate over options and handle each one as appropriate
    int c;
    // getopt_long stores the option index here.
    int option_index = 0;

    while((c = getopt_long(argc, argv, "v?d:b:e:o:r:",
                           long_options, &option_index)) != -1)
    {
        switch(c)
        {
        case 0:

            // If this option set a flag, do nothing else now.
            if(long_options[option_index].flag != 0)
            {
                break;
            }

        case 'v':
            verbose_flag = 1;
            break;

        case 'd':
            swing_delay_ms = atoi(optarg);
            break;

        case 'b':
            begin_hour = atoi(optarg);
            break;

        case 'e':
            end_hour = atoi(optarg);
            break;

        case 'o':
            hammer_on_ms = atoi(optarg);
            break;

        case 'r':
            ring_now_number = atoi(optarg);
            break;

        case '?':
        default :
            std::cout << "Options are: \n"
                      << "-v or --verbose for verbose output\n"
                      << "-d or --delay (delay in seconds between swings)\n"
                      << "-b or --beginning-hour (first hour to chime in the morning)\n"
                      << "-e or --ending-hour (last hour to chime in the evening in 24 hour format) \n"
                      << "-o or valve-time-ms (hammer on valve time in milliseconds) \n"
                      << "-r or ring-now (how many times to ring immediately.) \n"
                      << std::endl;
            exit(0);
        }
    }

    try
    {
#ifdef USE_PARALLEL_PORT
        IOPort ioport("LPT1");
        ioport[BellPin] = false;
#elif defined(USE_GPIO)
        IOPort ioport(BellPin, true);
        ioport.write(false);
#endif

        if(verbose_flag)
            std::cout << "You selected verbose output. We are using pin " << IOPort::getPinNumber(BellPin) << " as the output to "
                      << "sound the bell.\n" << std::endl;

        if(ring_now_number != 0)
        {
            ring(ioport, ring_now_number, swing_delay_ms, hammer_on_ms);
        }

        while(true)    // Do we really want an infinite loop?
        {
            /* Get the current time. */
            curtime = time(NULL);
            /* Convert it to local time representation. */
            loctime = localtime(&curtime);
            /* Sleep until the next hour comes around. */
            sleep_seconds = (59 - loctime->tm_min) * 60 + 60 - loctime->tm_sec;

            if(verbose_flag)
                std::cout << "We will sleep for " << sleep_seconds << " seconds, and then wake up. \n"
                          << std::endl;

            this_thread::sleep_for(chrono::seconds(sleep_seconds));

            curtime = time(NULL);
            loctime = localtime(&curtime);

            if(loctime->tm_min == 0)
            {
                chime_number = loctime->tm_hour % 12;

                if(chime_number == 0)
                {
                    chime_number = 12;    // Chime 12 times at noon.
                }

                if((loctime->tm_hour >= begin_hour) && (loctime->tm_hour <= end_hour))
                {
                    if(verbose_flag)    /* Print out the date and time in the standard format. */
                    {
                        fputs(asctime(loctime), stdout);
                    }

                    ring(ioport, chime_number, swing_delay_ms, hammer_on_ms);
                }
            }
            else if(loctime->tm_min == 59)  //We have had a problem with it waking up prematurely so this is to fix that
            {
                chime_number = (loctime->tm_hour + 1) % 12;

                if(chime_number == 0)
                {
                    chime_number = 12;    // Chime 12 times at noon.
                }

                if((loctime->tm_hour + 1 >= begin_hour) && (loctime->tm_hour + 1 <= end_hour))
                {
                    if(verbose_flag)    /* Print out the date and time in the standard format. */
                    {
                        fputs(asctime(loctime), stdout);
                    }

                    ring(ioport, chime_number, swing_delay_ms, hammer_on_ms);
                }
            }
        }
    }
    catch(exception &e)
    {
        cerr << "Error : " << e.what() << endl;
        exit(1);
    }
}

