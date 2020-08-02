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

/**
 *  Default Constructor
 *
 *  @param proc     PID in string form
 *  @param run      Current iteration of test
 */
PMapParser::PMapParser(std::string proc, int& run)
{
    std::string c = std::to_string(run);
    pid = proc;
    maptxt = "pidMaps" + c + "/" + pid + "pmap.txt";
    mapCmd = "pmap -x " + pid + " > " + maptxt;
}

/**
 *  execCommand()
 *
 *  Execute command formulated in ctor
 *  Create file of raw output from command
 *  
 *  @return         Void
 */
void PMapParser::execCommand()
{
    std::system(mapCmd.c_str());
}


/**
 *  foundVoid()
 *
 *  Check for line missing memory address
 *
 *  @return         True if line is empty
 */
bool PMapParser::foundVoid()
{
    return entry.substr(0, 5) == "-----";
}

/**
 *  foundTotal()
 *
 *  Find line containing byte totals
 *    
 *  @return         True if line begings with "total"
 */
bool PMapParser::foundTotal()
{
    return entry.substr(0, 5) == "total";
}

/**
 *  convertStr()
 *
 *  Convert string to long
 *  
 *  @param  input      String from file to be converted
 *  @return            String in long int form
 */
long PMapParser::convertStr(std::string input)
{
    std::stringstream inputSS(input);
    long outputLong = 0;
    inputSS >> outputLong;
    return outputLong;
}

/**
 *  convertHex()
 *
 *  Convert hex address to long
 *
 *  @param  hexString   Hex address to convert
 *  @return             String in long int form
 */
long PMapParser::convertHex(std::string hexString)
{
    std::stringstream hex(hexString);
    long decAddr = 0;
    hex >> std::hex >> decAddr;
    return decAddr;
}

/**
 *  parseTotal()
 *
 *  Parse all elements of the "total" line
 *  Collect and store totalBytes, totalRSS, totalDirty
 *
 *  @return         Void
 */
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

/**
 *  foundHeader()
 *
 *  Find and ignore output header
 *
 *  @return         True if header is located
 */
bool PMapParser::foundHeader()
{
    return mapEntries[elem -1].addr == convertHex(hex);
}

/**
 *  parseAddress()
 *
 *  Parse and store hex address of each mapping (line)
 *  Parse and store size of mapping
 *  
 *  @return         Void
 */
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

/**
 *  parseRSS()
 *
 *  Parse and store resident set size (RSS) for each mapping
 *  
 *  @return         Void
 */
void PMapParser::parseRSS()
{
    rPos = entry.find_first_of(NUM, bPos + bytes.length());
    RSS  = entry.substr(rPos, entry.find(' ', rPos) - rPos);
    mapEntries[elem].rss = convertStr(RSS);
    avgRSS += mapEntries[elem].rss;
}

/**
 *  parseDirty()
 *
 *  Parse and store number of dirty pages (in bytes) for each mapping
 *  
 *  @return         Void
 */
void PMapParser::parseDirty()
{
    dPos  = entry.find_first_of(NUM, rPos + RSS.length());
    dirty = entry.substr(dPos, entry.find(' ', dPos) - dPos);
    mapEntries[elem].dirty = convertStr(dirty);
    avgDirty += mapEntries[elem].dirty;
}

/**
 *  parseMode()
 *
 *  Parse and store string of permissions for an individual mapping
 *  Call countMode() method to count individual permissions
 *
 *  @return         Void
 */
void PMapParser::parseMode()
{
    mode = entry.substr(entry.find_first_of("-r"), 5);
    mapEntries[elem].modes = mode;
    countMode();
}

/**
 *  countMode()
 *
 *  Keep a running count of permissions for mappings within the current PID
 *
 *  @return         Void
 */
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

/**
 *  parseMapping()
 *
 *  Parse and store string containing file path for each mapping
 *  
 *  @return         Void
 */
void PMapParser::parseMapping()
{
    std::size_t mEnd = entry.find_first_of("-r") + 5;
    std::size_t mPos = entry.find_first_not_of(' ', mEnd);
    mapping = entry.substr(mPos, entry.size() - mPos);
    mapEntries[elem].map = mapping;
}

/**
 *  writeOutput()
 *
 *  Initialize and populate a file with cleaned output
 *  File will contain mapping size, RSS size, and dirty bytes size
 *  Whitespace delineated, each mapping on a new line, etraneous lines and data removed
 *  File name of the form [PID].txt
 *  
 *  @return         Void
 */
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

/**
 *  computeAvgs()
 *
 *  Compute average mapping size, RSS size, and dirty bytes size for an individual mapping in a PID
 *
 *  @return         Void
 */
void PMapParser::computeAvgs()
{
    avgBytes /= (elem - 1);
    avgRSS   /= (elem - 1);
    avgDirty /= (elem - 1);
}

/**
 *  writeMacros()
 *
 *  Initialize and populate a file with statistics for an individual PID
 *  File lists total permissions counts across all mappings in the PID
 *  Also listed are total and average values for mapping size, RSS size, and dirty bytes size
 *  Files will be found in the newly located output/ directory
 *  File name of the form [PID]IndMacros.txt
 *
 *  @param run      Current iteration of test
 *  @return         Void
 */
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

/**
 *  sumTotals()
 *
 *  Sum total bytes, total RSS, and total dirty bytes across all PIDs
 *  for the running program selected for testing
 * 
 *  @param totalB   Long sum total of mapping sizes  
 *  @param totalR   Long sum total of RSS sizes
 *  @param totalD   Long sum total of dirty bytes sizes
 *  @return         Void   
 */
void PMapParser::sumTotals(long& totalB, long& totalR, long& totalD)
{
    totalB += totalBytes;
    totalR += totalRSS;
    totalD += totalDirty;
}

/************************************ Free Functions ***************************************/

/**
 *  getPIDs()
 *
 *  Get PIDs of running program with pidof command
 *  Create pidMaps and output directories
 *  
 *  @param run      Current iteration of test
 *  @param program  Name of running program specified by user 
 *  @return         Void   
 */
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

/**
 *  vectorizePIDs()
 *
 *  Create vector of PIDs from output file generated by getPIDs() (pidof command)
 *  
 *  @param run      Current iteration of test
 *  @return         Vector of strings containing PIDs of program chosen for testing
 */
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

/**
 *  writeTotals()
 *
 *  Initialize and populate a file containing program-wide totals
 *  File will contain sum of mapping size, RSS size, and dirty bytes size for all PIDs of a running program
 *  File name of the form appTotals[run].txt where c is the run is the current iteration of the test
 * 
 *  @param totalB   Long sum total of mapping sizes  
 *  @param totalR   Long sum total of RSS sizes
 *  @param totalD   Long sum total of dirty bytes sizes
 *  @param run      Current iteration of test
 *  @return         Void   
 */
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

/**
 *  removeDir()
 * 
 *  Remove extraneous directories generated by terminal commands
 * 
 *  @param run      Current iteration of test
 *  @return         Void
 */
void removeDir(int& run)
{
    std::string c = std::to_string(run);
    std::string rmPidMaps = "rm -rf pidMaps" + c;
    std::system(rmPidMaps.c_str());
}
