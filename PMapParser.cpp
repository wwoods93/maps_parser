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

PMapParser::PMapParser(std::string proc)
{
    pid = proc;
    maptxt = "pidMaps/" + pid + "pmap.txt";
    mapCmd = "pmap -x " + pid + " > " + maptxt;
}

void PMapParser::execCommand()
{
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
    const std::size_t tbPos = entry.find_first_of(NUM);
    const std::string tb    = entry.substr(tbPos, entry.find(' ', tbPos) - tbPos);
    const std::size_t trPos = entry.find_first_of(NUM, (tbPos + tb.length()));
    const std::string tr    = entry.substr(trPos, entry.find(' ', trPos) - trPos);
    const std::size_t tdPos = entry.find_first_of(NUM, (trPos + tr.length()));
    const std::string td    = entry.substr(tdPos, entry.find(' ', tdPos) - tdPos);
    totalBytes = convertStr(tb);
    totalRSS = convertStr(tr);
    totalDirty = convertStr(td);
}

void PMapParser::parseAddress()
{
    hex   = entry.substr(0, entry.find(' '));
    bPos  = entry.find_first_of(NUM, hex.length() + 1);
    bytes = entry.substr(bPos, entry.find(' ', bPos) - bPos);
}

void PMapParser::parseRSS()
{
    rPos = entry.find_first_of(NUM, bPos + bytes.length());
    RSS  = entry.substr(rPos, entry.find(' ', rPos) - rPos);
}

void PMapParser::parseDirty()
{
    dPos  = entry.find_first_of(NUM, rPos + RSS.length());
    dirty = entry.substr(dPos, entry.find(' ', dPos) - dPos);
}

void PMapParser::parseMode()
{
    mode = entry.substr(entry.find_first_of("-r"), 5);
}

std::string PMapParser::parseMapping()
{
    std::size_t mEnd = entry.find_first_of("-r") + 5;
    std::size_t mPos = entry.find_first_not_of(' ', mEnd);
    return entry.substr(mPos, entry.size() - mPos);
}

void PMapParser::countMode(Permissions& perms)
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
        parsedPmap << std::setw(12) << mapEntries[i].size << std::setw(12) << mapEntries[i].rss 
                   << std::setw(12) << mapEntries[i].dirty << '\n';
    parsedPmap.close();
}

void PMapParser::computeAvgs(int& elem)
{
    avgBytes /= (elem - 1);
    avgRSS   /= (elem - 1);
    avgDirty /= (elem - 1);
}

void PMapParser::writeMacros(std::vector<MapEntries>& mapEntries, const Permissions& perms)
{
    std::ofstream pmapMacros;
    std::string macrostxt = "IndMacros.txt";
    std::string dir = "output/";
    std::string fileName = dir + pid + macrostxt;
    pmapMacros.open(fileName);
    pmapMacros << "Readable:        " << perms.r << '\n';
    pmapMacros << "Writable:        " << perms.w << '\n';
    pmapMacros << "Executable:      " << perms.x << '\n';
    pmapMacros << "Sharable:        " << perms.s << '\n';
    pmapMacros << "Private:         " << perms.p << '\n';
    pmapMacros << "No permissions:  " << perms.none << "\n\n";
    pmapMacros << std::setw(30) << "Total size:               " << std::setw(10) << totalBytes << '\n';
    pmapMacros << std::setw(30) << "Total RSS:                " << std::setw(10) << totalRSS << '\n';
    pmapMacros << std::setw(30) << "Total dirty pages:        " << std::setw(10) << totalDirty << "\n\n";
    pmapMacros << std::setw(30) << "Average total size:       " << std::setw(10) << avgBytes << '\n';
    pmapMacros << std::setw(30) << "Average RSS:              " << std::setw(10) << avgRSS << '\n';
    pmapMacros << std::setw(30) << "Average dirty pages size: " << std::setw(10) << avgDirty << '\n';
    pmapMacros << std::endl;
    pmapMacros.close();
}

void PMapParser::sumTotals(long& totalB, long& totalR, long& totalD)
{
    totalB += totalBytes;
    totalR += totalRSS;
    totalD += totalDirty;
}

// free functions

void getPIDs()
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
    std::system("mkdir pidMaps");
    std::system("mkdir output");
}

std::vector<std::string> vectorizePIDs()
{
    std::vector<std::string> pids;
    pids.push_back("");
    int pos = 0;
    std::ifstream pidFile("pids.txt");
    for (std::string line; std::getline(pidFile, line);)
    {
        for(int c = 0; c < line.length(); ++c)
            if(isdigit(line[c]))
                pids[pos] += line[c];
            else
            {
                pids.push_back("");
                ++pos;
            }
    }
    return pids;
}

void writeTotals(const long& totalB, const long& totalR, const long& totalD)
{
    std::ofstream appTotals;
    appTotals.open("appTotals.txt");
    appTotals << std::setw(12) << "Bytes Total" << std::setw(12) << "RSS Total" << std::setw(12) << "Dirty Total" << '\n';
    appTotals << std::setw(12) << totalB << std::setw(12) << totalR << std::setw(12) << totalD << '\n';
    appTotals.close();
}