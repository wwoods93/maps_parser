/*
 *  PMapParser.cpp
 */

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "PMapParser.hpp"


void PMapParser::getPIDs()
{
    std::string program;
    std::string pidof = "pidof ";
    std::string pidFile = " > pids.txt";
    std::string printPIDs = "cat pids.txt";
    std::cout << "Executing pidof command.\n";
    std::cout << "Enter name of running program:\n";
    std::cin >> program;
    std::string pidCmd = pidof + program + pidFile;
    std::system(pidCmd.c_str());
    std::system(printPIDs.c_str());
}

void PMapParser::execCommand()
{
    std::string pmapx = "pmap -x ";
    std::string pipeFile = " > pmap.txt";
    std::cout << "Executing " << pmapx << "command.\n";
    std::cout << "Enter PID of a running process:\n";
    std::cin >> pid;
    std::string mapCmd = pmapx + pid + pipeFile;
    std::system(mapCmd.c_str());
}

long PMapParser::convertStr(std::string input)
{
    std::stringstream inputSS(input);
    long outputLong = 0;
    inputSS >> outputLong;
    return outputLong;
}

long PMapParser::convertHex(std::string hexString)
{
    std::stringstream hex(hexString);
    long decAddr = 0;
    hex >> std::hex >> decAddr;
    return decAddr;
}

void PMapParser::parseTotal()
{
    std::size_t tbPos = entry.find_first_of(NUM);
    std::string tb = entry.substr(tbPos, entry.find(' ', tbPos) - tbPos);
    totalBytes = convertStr(tb);
    std::size_t trPos = entry.find_first_of(NUM, (tbPos + tb.length()));
    std::string tr = entry.substr(trPos, entry.find(' ', trPos) - trPos);
    totalRSS = convertStr(tr);
    std::size_t tdPos = entry.find_first_of(NUM, (trPos + tr.length()));
    std::string td = entry.substr(tdPos, entry.find(' ', tdPos) - tdPos);
    totalDirty = convertStr(td);
}

void PMapParser::parseAddress()
{
    hex = entry.substr(0, entry.find(' '));
    bPos = entry.find_first_of(NUM, hex.length() + 1);
    bytes = entry.substr(bPos, entry.find(' ', bPos) - bPos);
}

void PMapParser::parseRSS()
{
    rPos = entry.find_first_of(NUM, bPos + bytes.length());
    RSS = entry.substr(rPos, entry.find(' ', rPos) - rPos);
}

void PMapParser::parseDirty()
{
    dPos = entry.find_first_of(NUM, rPos + RSS.length());
    dirty = entry.substr(dPos, entry.find(' ', dPos) - dPos);
}

std::string PMapParser::parseMode()
{
    return entry.substr(entry.find_first_of("-r"), 5);
}

std::string PMapParser::parseMapping()
{
    std::size_t mEnd = entry.find_first_of("-r") + 5;
    std::size_t mPos = entry.find_first_not_of(' ', mEnd);
    return entry.substr(mPos, entry.size() - mPos);
}

void PMapParser::countMode(std::string mode, Permissions& perms)
{
    if (mode[0] == 'r')
        ++perms.r;
    if (mode[1] == 'w')
        ++perms.w;
    if (mode[2] == 'x')
        ++perms.x;
    if (mode[3] == 's')
        ++perms.s;
    if (mode[4] == 'p')
        ++perms.p;
    if (mode == "-----")
        ++perms.none;
}

void PMapParser::writeOutput(const int& pos, std::vector<MapEntries>& mapEntries)
{
    std::ofstream parsedPmap;
    std::string txt = ".txt";
    std::string fileName = pid + txt;
    parsedPmap.open(fileName);
    for (int i = 0; i < pos -1; ++i)
    {
        parsedPmap << std::setw(12) << mapEntries[i].size << std::setw(12) << mapEntries[i].rss 
                   << std::setw(12) << mapEntries[i].dirty << '\n';
    }
    parsedPmap.close();
}

void PMapParser::writeMacros(std::vector<MapEntries>& mapEntries)
{
    std::ofstream pmapTotals;
    std::string totalstxt = "totals.txt";
    std::string fileName = pid + totalstxt;
    pmapTotals.open(fileName);
    pmapTotals << std::setw(12) << "Stat:" << std::setw(12) << "Bytes" << std::setw(12) << "RSS" << std::setw(12) << "Dirty" << '\n';
    pmapTotals << std::setw(12) << "Total:" << std::setw(12) << totalBytes << std::setw(12) << totalRSS << std::setw(12) << totalDirty << '\n';
    pmapTotals << std::setw(12) << "Average:" << std::setw(12) << avgBytes << std::setw(12) << avgRSS << std::setw(12) << avgDirty << '\n';
    pmapTotals.close();
}

void PMapParser::printOutput(const Permissions& perms)
{
    std::cout << "____________________________" << '\n';
    std::cout << "Readable:        " << perms.r << '\n';
    std::cout << "Writable:        " << perms.w << '\n';
    std::cout << "Executable:      " << perms.x << '\n';
    std::cout << "Sharable:        " << perms.s << '\n';
    std::cout << "Private:         " << perms.p << '\n';
    std::cout << "No permissions:  " << perms.none << "\n\n";
    std::cout << std::setw(30) << "Total size:               " << std::setw(10) << totalBytes << '\n';
    std::cout << std::setw(30) << "Total RSS:                " << std::setw(10) << totalRSS << '\n';
    std::cout << std::setw(30) << "Total dirty pages:        " << std::setw(10) << totalDirty << '\n';
    std::cout << std::setw(30) << "Average total size:       " << std::setw(10) << avgBytes << '\n';
    std::cout << std::setw(30) << "Average RSS:              " << std::setw(10) << avgRSS << '\n';
    std::cout << std::setw(30) << "Average dirty pages size: " << std::setw(10) << avgDirty << '\n';
}