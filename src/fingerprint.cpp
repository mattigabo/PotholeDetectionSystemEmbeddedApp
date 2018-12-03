//
// Created by Xander on 03/12/2018.
//

#include "fingerprint.h"

#include <iostream>
#include <algorithm>
#include <functional>

#if defined(_WIN32)
#define WINDOWS
#include <windows.h>
#include <intrin.h>
#include <cstdlib>
#include <cstdio>
#include <array>
#include <vector>
#include <regex>
#include <stdint.h>
//#else
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
//#include <sys/socket.h>
//#include <netdb.h>
//#include <netinet/in.h>
//#include <netinet/in_systm.h>
//#include <netinet/ip.h>
//#include <netinet/ip_icmp.h>
//
//#include <sys/ioctl.h>
//
//#if defned(__APPLE__)
//#defined OSX
//#include <net/if_dl.h>
//#include <ifaddrs.h>
//#include <net/if_types.h>
//#elseif defined(__linux__)
//#defin LINUX
//#include <linux/if.h>
//#include <linux/sockios.h>
//#endif
//
//#include <sys/resource.h>
//#include <sys/utsname.h>
#endif

#if !defined(u8)
typedef uint8_t u8;
#endif

#if !defined(u16)
typedef uint16_t u16;
#endif

#if !defined(u32)
typedef uint32_t u32;
#endif

namespace fingerprint {

    const int FINGERPRINT_LENGTH = 6;

    u16 hashString(const std::string str) {
        u16 hash = 0;

        for ( u32 i = 0; i < str.length(); i++ ) {
            hash += (str[i] << (( i & 1 ) * 8 ));
        }

        return hash;
    }

    u16 hashMachineName(const std::string name) {
        return hashString(name);
    }

    u16 hashMacAddress(std::string mac) {
        return hashString(mac);
    }

    u16 hashCPUInfo(u32 *cpu_info, size_t length) {

        u16 hash = 0;
        u32* ptr = &cpu_info[0];
        for (u32 i = 0; i < length; ++i) {
            hash += (ptr[i] & 0xFFFF) + (ptr[i] >> 16);
        }

        return hash;
    }

    u16 hashVolumeSerial(const DWORD serialNumber) {
        return (u16)(( serialNumber + ( serialNumber >> 16 )) & 0xFFFF );
    }

    std::vector<u16> hashMACs(const std::vector<std::string> macs) {
        std::vector<u16> hmacs;

        for (const auto mac : macs) {
            u32 l = static_cast<u32>(std::count_if(mac.begin(), mac.end(), [](char c){ return c == '-' || c == ':';}));
            hmacs.emplace_back(hashMacAddress(mac));
        }

        // sort the mac addresses. We don't want to invalidate
        // both macs if they just change order.
        std::sort(hmacs.begin(), hmacs.end(), [](u16 m1, u16 m2) { return m1 > m2;});

        return hmacs;
    }

#ifdef WINDOWS

    const std::regex macRegex("(Physical Address[.| ]+: )(([\\d\\w]{2,}[-:]?){6,})");
    const std::string command("ipconfig /all 2>&1");

    std::vector<std::string> getMACs() {

        std::array<char, 256> buffer;
        std::vector<std::string> mac_addrs;

        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe)
        {
            return std::vector<std::string>();
        }

        while (fgets(buffer.data(), 256, pipe) != NULL) {
            const std::string tmp = std::string(buffer.data());

            std::smatch match;

            if (std::regex_search(tmp.begin(), tmp.end(), match, macRegex)) {
                mac_addrs.emplace_back(std::string(match[match.size() - 2l]));
            }
        }
        pclose(pipe);

        return mac_addrs;
    }

    DWORD getVolumeSerialNumber() {
        DWORD serialNumber = 0;

        // Determine if this volume uses an NTFS file system.
        GetVolumeInformation( "c:\\", NULL, 0, &serialNumber, NULL, NULL, NULL, 0 );
        return serialNumber;
    }

    std::vector<u32> getCPUInfo() {
        int cpu_info[4] = { 0, 0, 0, 0 };
        __cpuid(cpu_info, 0 );

        std::vector<u32> info;

        for (int i = 0; i < 4; ++i) {
            info.emplace_back((u32) cpu_info[i]);
        }

        return info;
    }

    char* getMachineName()
    {
        static char computerName[1024];
        DWORD size = 1024;
        GetComputerName( computerName, &size );
        return &(computerName[0]);
    }

#else

//---------------------------------get MAC addresses ---------------------------------
//void getMacHash( u16& mac1, u16& mac2 )
//{
//   mac1 = 0;
//   mac2 = 0;
//
//#ifdef DARWIN
//
//   struct ifaddrs* ifaphead;
//   if ( getifaddrs( &ifaphead ) != 0 )
//      return;
//
//   // iterate over the net interfaces
//   bool foundMac1 = false;
//   struct ifaddrs* ifap;
//   for ( ifap = ifaphead; ifap; ifap = ifap->ifa_next )
//   {
//      struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifap->ifa_addr;
//      if ( sdl && ( sdl->sdl_family == AF_LINK ) && ( sdl->sdl_type == IFT_ETHER ))
//      {
//          if ( !foundMac1 )
//          {
//             foundMac1 = true;
//             mac1 = hashMacAddress( (u8*)(LLADDR(sdl))); //sdl->sdl_data) +
//                                    sdl->sdl_nlen) );
//          } else {
//             mac2 = hashMacAddress( (u8*)(LLADDR(sdl))); //sdl->sdl_data) +
//                                    sdl->sdl_nlen) );
//             break;
//          }
//      }
//   }
//
//   freeifaddrs( ifaphead );
//
//#else // !DARWIN
//
//   int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP );
//   if ( sock < 0 ) return;
//
//   // enumerate all IP addresses of the system
//   struct ifconf conf;
//   char ifconfbuf[ 128 * sizeof(struct ifreq)  ];
//   memset( ifconfbuf, 0, sizeof( ifconfbuf ));
//   conf.ifc_buf = ifconfbuf;
//   conf.ifc_len = sizeof( ifconfbuf );
//   if ( ioctl( sock, SIOCGIFCONF, &conf ))
//   {
//      assert(0);
//      return;
//   }
//
//   // get MAC address
//   bool foundMac1 = false;
//   struct ifreq* ifr;
//   for ( ifr = conf.ifc_req; (s8*)ifr < (s8*)conf.ifc_req + conf.ifc_len; ifr++ )
//   {
//      if ( ifr->ifr_addr.sa_data == (ifr+1)->ifr_addr.sa_data )
//         continue;  // duplicate, skip it
//
//      if ( ioctl( sock, SIOCGIFFLAGS, ifr ))
//         continue;  // failed to get flags, skip it
//      if ( ioctl( sock, SIOCGIFHWADDR, ifr ) == 0 )
//      {
//         if ( !foundMac1 )
//         {
//            foundMac1 = true;
//            mac1 = hashMacAddress( (u8*)&(ifr->ifr_addr.sa_data));
//         } else {
//            mac2 = hashMacAddress( (u8*)&(ifr->ifr_addr.sa_data));
//            break;
//         }
//      }
//   }
//
//   close( sock );
//
//#endif // !DARWIN
//
//   // sort the mac addresses. We don't want to invalidate
//   // both macs if they just change order.
//   if ( mac1 > mac2 )
//   {
//      u16 tmp = mac2;
//      mac2 = mac1;
//      mac1 = tmp;
//   }
//}
//
//u16 getVolumeHash()
//{
//   // we don't have a 'volume serial number' like on windows.
//   // Lets hash the system name instead.
//   u8* sysname = (u8*)getMachineName();
//   u16 hash = 0;
//
//   for ( u32 i = 0; sysname[i]; i++ )
//      hash += ( sysname[i] << (( i & 1 ) * 8 ));
//
//   return hash;
//}
//
//#ifdef DARWIN
// #include <mach-o/arch.h>
// u16 getCpuHash()
// {
//     const NXArchInfo* info = NXGetLocalArchInfo();
//     u16 val = 0;
//     val += (u16)info->cputype;
//     val += (u16)info->cpusubtype;
//     return val;
// }
//
//#else // !DARWIN
//
// static void getCpuid( u32* p, u32 ax )
// {
//    __asm __volatile
//    (   "movl %%ebx, %%esi\n\t"
//        "cpuid\n\t"
//        "xchgl %%ebx, %%esi"
//        : "=a" (p[0]), "=S" (p[1]),
//          "=c" (p[2]), "=d" (p[3])
//        : "0" (ax)
//    );
// }
//
// u16 getCpuHash()
// {
//    u32 cpuinfo[4] = { 0, 0, 0, 0 };
//    getCpuid( cpuinfo, 0 );
//    u16 hash = 0;
//    u32* ptr = (&cpuinfo[0]);
//    for ( u32 i = 0; i < 4; i++ )
//       hash += (ptr[i] & 0xFFFF) + ( ptr[i] >> 16 );
//
//    return hash;
// }
//#endif // !DARWIN
//
//const char* getMachineName()
//{
//   static struct utsname u;
//
//   if ( uname( &u ) < 0 )
//   {
//      assert(0);
//      return "unknown";
//   }
//
//   return u.nodename;
//}
#endif

    const u16 mask[5] = { 0x4e25, 0xf4a1, 0x5437, 0xab41, 0x0000 };

    static void smear( u16* id )
    {
        for ( u32 i = 0; i < 5; i++ )
            for ( u32 j = i; j < 5; j++ )
                if ( i != j )
                    id[i] ^= id[j];

        for ( u32 i = 0; i < 5; i++ )
            id[i] ^= mask[i];
    }

    static void unsmear( u16* id )
    {
        for ( u32 i = 0; i < 5; i++ )
            id[i] ^= mask[i];

        for ( u32 i = 0; i < 5; i++ )
            for ( u32 j = 0; j < i; j++ )
                if ( i != j )
                    id[4-i] ^= id[4-j];
    }

    static u16* computeSystemUniqueId()
    {
        static u16 id[FINGERPRINT_LENGTH];
        static bool computed = false;

        if ( computed ) return id;

        // produce a number that uniquely identifies this system.
        id[0] = hashMachineName(std::string(getMachineName()));

        auto info = getCPUInfo();

        id[1] = hashCPUInfo(info.data(), info.size());
        id[2] = hashVolumeSerial(getVolumeSerialNumber());

        auto hmacs = hashMACs(getMACs());
        id[3] = hmacs[0];
        id[4] = hmacs[1];

        // fifth block is some checkdigits
        id[5] = 0;
        for (u32 i = 0; i < FINGERPRINT_LENGTH - 1; i++ )
            id[5] += id[i];


        smear( id );

        computed = true;
        return id;
    }

    std::string getUID(){
        // get the name of the computer
#ifdef WINDOWS
        std::string buf;

        u16* id = computeSystemUniqueId();
        for ( u32 i = 0; i < FINGERPRINT_LENGTH; i++ )
        {
            char num[16];
            snprintf( num, 16, "%x", id[i] );

            switch( strlen( num ))
            {
                case 1: buf.append("000"); break;
                case 2: buf.append("00");  break;
                case 3: buf.append("0");   break;
                default:break;
            }
            buf.append(num);
            if (i < FINGERPRINT_LENGTH - 1) {
                buf.append("-");
            }
        }

        std::transform(buf.begin(), buf.end(), buf.begin(), ::toupper);

        return buf;
#else
        return std::string("6293-F04C-23B9-AB47-9BDB-4AFE")
#endif
    }

    bool validateUID(const std::string uid)
    {
        // unpack the given string. parse failures return false.
        char * testString = new char[uid.length() + 1];
        strcpy(testString, uid.c_str());

        u16 testId[FINGERPRINT_LENGTH];

        int unit_len = static_cast<int>((uid.length() - FINGERPRINT_LENGTH + 1) / FINGERPRINT_LENGTH);

        for ( u32 i = 0; i < FINGERPRINT_LENGTH; i++ )
        {
            char* testNum = strtok(&testString[i*(unit_len+1)], "-" );

            if ( !testNum ) return false;
            testId[i] = (u16)(strtol( testNum, nullptr, 16 ));

        }

        unsmear(testId);

        // make sure this id is valid - by looking at the checkdigits
        u16 check = 0;
        for ( u32 i = 0; i < FINGERPRINT_LENGTH - 1; i++ )
            check += testId[i];
        if ( check != testId[5] ) return false;

        // get the current system information
        u16 systemId[FINGERPRINT_LENGTH];
        memcpy(systemId, computeSystemUniqueId(), sizeof( systemId ));
        unsmear(systemId);

        // now start scoring the match
        u32 score = 0;

        for ( u32 i = 0; i < FINGERPRINT_LENGTH - 1; i++ )
            if (testId[i] == systemId[i])
                score++;

        delete [] testString;

        // if we score 4 points or more then the id matches.
        return score >= 4;
    }

}
