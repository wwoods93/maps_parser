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
    long totalB = 0;
    long totalR = 0;
    long totalD = 0;
    std::vector<long> setTotals(3, 0);
    getPIDs();
    std::vector<std::string> pids = vectorizePIDs();

    for (int count = 0; count < pids.size(); ++count)
    {
        std::vector <PMapParser::MapEntries> mapEntries;
        PMapParser::Permissions perms;
        PMapParser m(pids[count]);

        m.execCommand();
        std::ifstream pmapFile(m.maptxt.c_str());
        pmapFile.ignore(1024, '\n');
        pmapFile.ignore(1024, '\n');
        int elem = 0;

        for (m.entry; std::getline(pmapFile, m.entry);)
        {
            if (m.entry.substr(0, 4) == "----")
                continue;
            else if(m.entry.substr(0, 5) == "total")
                m.parseTotal();
            else
            {
                mapEntries.push_back(PMapParser::MapEntries());
                m.parseAddress();
                if(mapEntries[elem -1].addr == m.convertHex(m.hex))
                    continue;
                mapEntries[elem].addr = m.convertHex(m.hex);
                mapEntries[elem].size = m.convertStr(m.bytes);
                m.avgBytes += mapEntries[elem].size;

                m.parseRSS();
                mapEntries[elem].rss = m.convertStr(m.RSS);
                m.avgRSS += mapEntries[elem].rss;

                m.parseDirty();
                mapEntries[elem].dirty = m.convertStr(m.dirty);
                m.avgDirty += mapEntries[elem].dirty;

                m.parseMode();
                mapEntries[elem].modes = m.mode;
                m.countMode(perms);
                mapEntries[elem].map = m.parseMapping();
            }
            ++elem;
        }
        m.computeAvgs(elem);
        m.writeMacros(mapEntries, perms);
        m.sumTotals(totalB, totalR, totalD);
    }
    writeTotals(totalB, totalR, totalD);
    std::system("rm -rf pidMaps");

    return 0;
}
