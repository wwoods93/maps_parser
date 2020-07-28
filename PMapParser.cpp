/*
 *  PMapParser.hpp
 *
 *  ***Desgined and tested on an Ubuntu 18.04 Linux Distribution.  For use ONLY on Linux Systems.***
 *
 *  The PMapParser class, along with its accompanying free functions, is designed to execute the pmap -x [PID]
 *  command on a currently running process in a Linux System.
 *
 *  The class performs the following operations:
 *
 *      - Acquires the PIDs of all running processes for a user-selected software application
 *      - Executes the pmap -x [PID] command on all processes found
 *      - Parses and stores the output from each of these commands through a number of sub-tasks:
 *
 *          - Converts hexidecial mapping addresses into decimal for analysis and graphing
 *          - Parses and stores mapping size, resident set size (RSS), and dirty bytes size for each mapping in the process
 *          - Parses the [Mode] field of the output and counts the permissions (rwxsp) associated with each mapping
 *          - Parses and stores the file path associated with each mapping
 *          - NOTE: The parsed fields are stored in a vector of structs such that they may still be grouped by mapping, but
 *            also that all entries for a particular field may be iterated through sequentially
 *
 *      - While parsing the output for each individual PID, totals across all PIDs are also calculated giving the entire
 *        mapping of the application software.  Among these are total mapping size, total RSS, and total dirty bytes
 *
 *  The class provides the following output:
 *
 *      - The mapping size, RSS size, and dirty bytes size of each individual process
 *      - The total mapping size, RSS size, and dirty bytes size across all processes
 *      - The average mapping size, RSS size, and dirty bytes size of individual mappings within an individual process
 *      - A count of the permissions associated with all mappings in a process
 *      - A parsed, "clean" version of the command output with converted addresses, removal of headers, footers, and empty lines
 */

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "PMapParser.hpp"

PMapParser::PMapParser(std::string proc, int& run)
{
    std::string c = std::to_string(run);
    pid = proc;
    maptxt = "pidMaps" + c + "/" + pid + "pmap.txt";
    mapCmd = "pmap -x " + pid + " > " + maptxt;
}

void PMapParser::execCommand()
{
    std::system(mapCmd.c_str());
}

bool PMapParser::foundVoid()
{
    return entry.substr(0, 5) == "-----";
}

bool PMapParser::foundTotal()
{
    return entry.substr(0, 5) == "total";
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

bool PMapParser::foundHeader()
{
    return mapEntries[elem -1].addr == convertHex(hex);
}

void PMapParser::parseAddress()
{
    mapEntries.push_back(PMapParser::MapEntries());
    hex   = entry.substr(0, entry.find(' '));
    bPos  = entry.find_first_of(NUM, hex.length() + 1);
    bytes = entry.substr(bPos, entry.find(' ', bPos) - bPos);
    mapEntries[elem].addr = convertHex(hex);
    mapEntries[elem].size = convertStr(bytes);
    avgBytes += mapEntries[elem].size;
}

void PMapParser::parseRSS()
{
    rPos = entry.find_first_of(NUM, bPos + bytes.length());
    RSS  = entry.substr(rPos, entry.find(' ', rPos) - rPos);
    mapEntries[elem].rss = convertStr(RSS);
    avgRSS += mapEntries[elem].rss;
}

void PMapParser::parseDirty()
{
    dPos  = entry.find_first_of(NUM, rPos + RSS.length());
    dirty = entry.substr(dPos, entry.find(' ', dPos) - dPos);
    mapEntries[elem].dirty = convertStr(dirty);
    avgDirty += mapEntries[elem].dirty;
}

void PMapParser::parseMode()
{
    mode = entry.substr(entry.find_first_of("-r"), 5);
    mapEntries[elem].modes = mode;
    countMode();
}

void PMapParser::parseMapping()
{
    std::size_t mEnd = entry.find_first_of("-r") + 5;
    std::size_t mPos = entry.find_first_not_of(' ', mEnd);
    mapping = entry.substr(mPos, entry.size() - mPos);
    mapEntries[elem].map = mapping;
}

void PMapParser::countMode()
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

void PMapParser::writeOutput()
{
    std::ofstream parsedPmap;
    std::string txt = ".txt";
    std::string fileName = pid + txt;
    parsedPmap.open(fileName);
    for (int i = 0; i < elem -1; ++i)
        parsedPmap << std::setw(12) << mapEntries[i].size << std::setw(12) << mapEntries[i].rss
                   << std::setw(12) << mapEntries[i].dirty << '\n';
    parsedPmap.close();
}

void PMapParser::computeAvgs()
{
    avgBytes /= (elem - 1);
    avgRSS   /= (elem - 1);
    avgDirty /= (elem - 1);
}

void PMapParser::writeMacros(int& run)
{
    std::string c = std::to_string(run);
    std::ofstream pmapMacros;
    std::string fileName = "output" + c + "/" + pid + "IndMacros.txt";
    pmapMacros.open(fileName);
    pmapMacros << "Readable:        " << perms.r << '\n';
    pmapMacros << "Writable:        " << perms.w << '\n';
    pmapMacros << "Executable:      " << perms.x << '\n';
    pmapMacros << "Sharable:        " << perms.s << '\n';
    pmapMacros << "Private:         " << perms.p << '\n';
    pmapMacros << "No permissions:  " << perms.none << "\n\n";
    pmapMacros << "Total size:               " << std::setw(10) << totalBytes << '\n';
    pmapMacros << "Total RSS:                " << std::setw(10) << totalRSS << '\n';
    pmapMacros << "Total dirty pages:        " << std::setw(10) << totalDirty << "\n\n";
    pmapMacros << "Average size:             " << std::setw(10) << avgBytes << '\n';
    pmapMacros << "Average RSS:              " << std::setw(10) << avgRSS << '\n';
    pmapMacros << "Average dirty pages:      " << std::setw(10) << avgDirty << "\n\n";
    pmapMacros.close();
}

void PMapParser::sumTotals(long& totalB, long& totalR, long& totalD)
{
    totalB += totalBytes;
    totalR += totalRSS;
    totalD += totalDirty;
}

// free functions

void getPIDs(int& run, std::string& program)
{
    std::string c = std::to_string(run);
    std::string file = "pids" + c + ".txt";
    std::string catCmd = "cat " + file;
    std::string pidCmd = "pidof "+ program + " > " + file;
    std::string cmdOut = "mkdir pidMaps" + c;
    std::string prgOut = "mkdir output" + c;
    std::system(pidCmd.c_str());
    std::system(catCmd.c_str());
    std::system(cmdOut.c_str());
    std::system(prgOut.c_str());
}

std::vector<std::string> vectorizePIDs(int& run)
{
    std::string c = std::to_string(run);
    std::vector<std::string> pids;
    pids.push_back("");
    int pos = 0;
    std::string file = "pids" + c + ".txt";
    std::ifstream pidFile(file.c_str());
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

void writeTotals(const long& totalB, const long& totalR, const long& totalD, int& run)
{
    std::string c = std::to_string(run);
    std::ofstream appTotals;
    std::string totalFile = "appTotals" + c + ".txt";
    appTotals.open(totalFile.c_str());
    // appTotals << std::setw(12) << "Bytes Total" << std::setw(12) << "RSS Total" << std::setw(12) << "Dirty Total" << '\n';
    appTotals << std::setw(12) << totalB << std::setw(12) << totalR << std::setw(12) << totalD << '\n';
    appTotals.close();
}

void removeDir(int& run)
{
    std::string c = std::to_string(run);
    std::string rmPidMaps = "rm -rf pidMaps" + c;
    std::system(rmPidMaps.c_str());
}
