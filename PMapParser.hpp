/**
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
 *          - note: The parsed fields are stored in a vector of structs such that they may still be grouped by mapping, but
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

#ifndef PMAP_PARSER_HPP
#define PMAP_PARSER_HPP

#include <iostream>
#include <string>
#include <vector>

class PMapParser
{
    public:
        struct MapEntries
        {
            long addr = 0;
            long size = 0;
            int rss = 0;
            int dirty = 0;
            std::string modes;
            std::string map;
        };

        struct Permissions
        {
            int r = 0;
            int w = 0;
            int x = 0;
            int s = 0;
            int p = 0;
            int none = 0;
        } perms;

        std::vector <PMapParser::MapEntries> mapEntries;
        int elem = 0;
        long totalBytes = 0;
        long totalRSS = 0;
        long totalDirty = 0;
        double avgBytes = 0;
        double avgRSS = 0;
        double avgDirty = 0;
        std::size_t bPos = 0;
        std::size_t rPos = 0;
        std::size_t dPos = 0;
        std::string entry;
        std::string pid;
        std::string mode;
        std::string mapping;
        std::string maptxt;
        std::string mapCmd;
        std::string bytes;
        std::string RSS;
        std::string dirty;
        std::string hex;
        const std::string NUM = "0123456789";

        PMapParser(std::string proc, int& run);
        void execCommand();
        bool foundVoid();
        bool foundTotal();
        long convertStr(std::string input);
        long convertHex(std::string hexString);
        void parseTotal();
        bool foundHeader();
        void parseAddress();
        void parseRSS();
        void parseDirty();
        void parseMode();
        void parseMapping();
        void countMode();
        void writeOutput();
        void computeAvgs();
        void writeMacros(int& run);
        void sumTotals(long& totalB, long& totalR, long& totalD);
};

void getPIDs(int& run, std::string& program);
std::vector<std::string> vectorizePIDs(int& run);
void writeTotals(const long& totalB, const long& totalR, const long& totalD, int& run);
void removeDir(int& run);

#endif
