/*
 *  pmapParser.cpp
 */

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "PMapParser.hpp"

int main()
{
    PMapParser pmap;
    PMapParser::Permissions perms;
    std::vector <PMapParser::MapEntries> mapEntries;
    pmap.getPIDs();
    pmap.execCommand();
    std::ifstream pmapFile("pmap.txt");
    std::ifstream pidFile("pids.txt");
    pmapFile.ignore(1024, '\n');
    pmapFile.ignore(1024, '\n');
    int elem = 0;

    for (pmap.entry; std::getline(pmapFile, pmap.entry);)
    {
        if (pmap.entry.substr(0, 4) == "----")
            continue;

        else if(pmap.entry.find("total kB") != std::string::npos)
        {
            pmap.parseTotal();
        }
        else
        {
            mapEntries.push_back(PMapParser::MapEntries());
            pmap.parseAddress();
            if(mapEntries[elem -1].addr == pmap.convertHex(pmap.hex))
                continue;
            mapEntries[elem].addr = pmap.convertHex(pmap.hex);
            mapEntries[elem].size = pmap.convertStr(pmap.bytes);
            pmap.avgBytes += mapEntries[elem].size;
            pmap.parseRSS();
            mapEntries[elem].rss = pmap.convertStr(pmap.RSS);
            pmap.avgRSS += mapEntries[elem].rss;
            pmap.parseDirty();
            mapEntries[elem].dirty = pmap.convertStr(pmap.dirty);
            pmap.avgDirty += mapEntries[elem].dirty;

            std::string mode = pmap.parseMode();
            mapEntries[elem].modes = mode;
            pmap.countMode(mode, perms);
            mapEntries[elem].map = pmap.parseMapping();
        }
        ++elem;
    }
    pmap.avgBytes /= (elem - 1);
    pmap.avgRSS /= (elem - 1);
    pmap.avgDirty /= (elem - 1);
    pmap.writeOutput(elem, mapEntries);
    pmap.writeMacros(mapEntries);
    pmap.printOutput(perms);

    return 0;
}
