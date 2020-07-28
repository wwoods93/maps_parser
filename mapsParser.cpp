/*
 *  pmapParser.cpp
 *
 *
 *  ***Desgined and tested on an Ubuntu 18.04 Linux Distribution.  For use ONLY on Linux Systems.***
 *
 *  mapsParser.cpp employs the PMapParser describes in the accompanying header and implementation files.
 *
 *  This program is designed to execute the PMapParser functions a set number of times.
 *  This can be adjusted by adjusting NUM_RUNS.  Default NUM_RUNS is 10.
 *
 *
 *  Output can be managed by adjusting the removeDir(), writeOutput(), writeMacros(), and writeTotals() functions.
 *
 *  Build the program (g++):    g++ -o run PMapParser.hpp PMapParser.cpp mapsParser.cpp
 *
 *  Execute from command line:  ./run
 *
 *  Delete all output files:    rm -rf app* out* pid*
 */

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "PMapParser.hpp"

int main()
{
    const int NUM_RUNS = 1;
    std::cout << "Enter name of running program:\n";
    std::string program;
    std::cin >> program;
    for (int run = 0; run < NUM_RUNS; ++run)
    {
        long tB = 0;
        long tR = 0;
        long tD = 0;
        getPIDs(run, program);
        std::vector<std::string> pids = vectorizePIDs(run);
        std::vector<std::string> pids0;
        pids0.reserve(pids.size());

        for (int n = 0; n < pids.size(); ++n)
        {
            PMapParser m(pids[n], run);
            m.execCommand();
            std::ifstream ifs(m.maptxt.c_str());
            ifs.ignore(1024, '\n');
            ifs.ignore(1024, '\n');

            for (m.entry; std::getline(ifs, m.entry);)
            {
                if (m.foundVoid())
                    continue;
                else if(m.foundTotal())
                    m.parseTotal();
                else
                {
                    m.parseAddress();
                    if(m.foundHeader())
                        continue;
                    m.parseRSS();
                    m.parseDirty();
                    m.parseMode();
                    m.parseMapping();
                }
                ++m.elem;
            }
            m.computeAvgs();
            m.writeMacros(run);
            m.sumTotals(tB, tR, tD);
        }
        writeTotals(tB, tR, tD, run);
        removeDir(run);
        pids0 = pids;
    }
    return 0;
}
