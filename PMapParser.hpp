/*
 *  PMapParser.hpp
 */
#ifndef PMAP_PARSER_HPP
#define PMAP_PARSER_HPP

#include <iostream>
#include <string>
#include <vector>

class PMapParser
{
    public:
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
        std::string maptxt;
        std::string mapCmd;
        std::string bytes;
        std::string RSS;
        std::string dirty;
        std::string hex;
        const std::string NUM = "0123456789";

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
        };

        PMapParser(std::string proc);
        void execCommand();
        long convertStr(std::string input);
        long convertHex(std::string hexString);
        void parseTotal();
        void parseAddress();
        void parseRSS();
        void parseDirty();
        void parseMode();
        std::string parseMapping();
        void countMode(Permissions& perms);
        void writeOutput(const int& pos, std::vector<MapEntries>& mapEntries);
        void writeMacros(std::vector<MapEntries>& mapEntries, const Permissions& perms);
        void printOutput(const Permissions& perms);
};

void getPIDs();
std::vector<std::string> vectorizePIDs();
void writeTotals(const long& totalB, const long& totalR, const long& totalD);


#endif