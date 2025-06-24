#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <chrono>
#include <cmath>
#include "simInput.h"
#include "simConstants.h"

using namespace std;
using namespace chrono;

// Forward declaration
void useFrame(int frameIndex, int processId, int pageNumber);

// Global variables
int pointer;
int pageSize;
int pageFault;
int pagingType;
int numProcesses;
int replacementType;
int printDetails;
int processTraceListSize;
int counter = 0;
auto start = chrono::high_resolution_clock::now();

vector<Process> processes;
vector<MemoryEntry> memory;
vector<int> pageFaultTracker;
queue<ProcessTraceEntry> processTraceList;

int fifoReplace()
{
    cout << "[DEBUG] FIFO Replacement called\n";

    int firstComeFrame = 0;
    long long firstComeTime = processes[memory[0].processId].pages[memory[0].pageNumber].addedTime;

    for (int i = pageSize; i < MEMORY_SIZE; i += pageSize)
    {
        if (processes[memory[i].processId].pages[memory[i].pageNumber].addedTime < firstComeTime)
        {
            firstComeTime = processes[memory[i].processId].pages[memory[i].pageNumber].addedTime;
            firstComeFrame = i / pageSize;
        }
    }

    int processId = memory[firstComeFrame * pageSize].processId;
    int pageNumber = memory[firstComeFrame * pageSize].pageNumber;

    cout << "[DEBUG] Replacing frame: " << firstComeFrame << " | PID: " << processId << " | Page: " << pageNumber << endl;

    for (int i = 0; i < pageSize; i++)
    {
        memory[firstComeFrame * pageSize + i].occupied = 0;
        memory[firstComeFrame * pageSize + i].processId = -1;
        memory[firstComeFrame * pageSize + i].pageNumber = -1;
    }

    processes[processId].pageTable[pageNumber].valid = 0;
    processes[processId].pageTable[pageNumber].frame = -1;
    processes[processId].pages[pageNumber].addedTime = -1;
    processes[processId].pages[pageNumber].lastAccessedTime = -1;

    cout << "[DEBUG] FIFO Replacement called" << endl;

    return firstComeFrame;
}

int lruReplace()
{

    int firstAccessedFrame = 0;
    long long firstAccessedTime = processes[memory[0].processId].pages[memory[0].pageNumber].lastAccessedTime;

    for (int i = pageSize; i < MEMORY_SIZE; i += pageSize)
    {
        if (processes[memory[i].processId].pages[memory[i].pageNumber].lastAccessedTime < firstAccessedTime)
        {
            firstAccessedTime = processes[memory[i].processId].pages[memory[i].pageNumber].lastAccessedTime;
            firstAccessedFrame = i / pageSize;
        }
    }

    int processId = memory[firstAccessedFrame * pageSize].processId;
    int pageNumber = memory[firstAccessedFrame * pageSize].pageNumber;

    // ✅ Now safe to log
    cout << "[DEBUG] LRU Replacement called - Replacing frame " << firstAccessedFrame
         << ", Process " << processId << ", Page " << pageNumber << endl;

    for (int i = 0; i < pageSize; i++)
    {
        memory[firstAccessedFrame * pageSize + i].occupied = 0;
        memory[firstAccessedFrame * pageSize + i].processId = -1;
        memory[firstAccessedFrame * pageSize + i].pageNumber = -1;
    }

    processes[processId].pageTable[pageNumber].valid = 0;
    processes[processId].pageTable[pageNumber].frame = -1;
    processes[processId].pages[pageNumber].addedTime = -1;
    processes[processId].pages[pageNumber].lastAccessedTime = -1;

    cout << "[DEBUG] LRU Replacement called" << endl;

    return firstAccessedFrame;
}

int clockReplace()
{
    cout << "[DEBUG] CLOCK Replacement called\n";

    int frame_count = MEMORY_SIZE / pageSize;

    while (true)
    {
        int frame = pointer;

        if (memory[frame * pageSize].usedBit == 0)
        {
            int processId = memory[frame * pageSize].processId;
            int pageNumber = memory[frame * pageSize].pageNumber;

            cout << "[DEBUG] Replacing frame: " << frame << " | PID: " << processId << " | Page: " << pageNumber << endl;

            for (int i = 0; i < pageSize; i++)
            {
                memory[frame * pageSize + i].occupied = 0;
                memory[frame * pageSize + i].processId = -1;
                memory[frame * pageSize + i].pageNumber = -1;
                memory[frame * pageSize + i].usedBit = 0;
            }

            processes[processId].pageTable[pageNumber].valid = 0;
            processes[processId].pageTable[pageNumber].frame = -1;
            processes[processId].pages[pageNumber].addedTime = -1;
            processes[processId].pages[pageNumber].lastAccessedTime = -1;

            pointer = (pointer + 1) % frame_count;
            return frame;
        }
        else
        {
            for (int i = 0; i < pageSize; i++)
            {
                memory[pointer * pageSize + i].usedBit = 0;
            }
            pointer = (pointer + 1) % frame_count;
        }
    }

    cout << "[DEBUG] CLOCK Replacement called" << endl;
}

int replacement()
{
    cout << "[DEBUG] Entering replacement() with type = " << replacementType << endl;

    switch (replacementType)
    {
    case FIFO:
        cout << "[DEBUG] Calling FIFO replace\n";
        return fifoReplace();
    case LRU:
        cout << "[DEBUG] Calling LRU replace\n";
        return lruReplace();
    case CLOCK:
        cout << "[DEBUG] Calling CLOCK replace\n";
        return clockReplace();
    default:
        cout << "[DEBUG] Unknown replacement type\n";
        return -1;
    }
}

void useFrame(int frameIndex, int processId, int pageNumber)
{
    auto elapsed = high_resolution_clock::now() - start;
    long long now = duration_cast<nanoseconds>(elapsed).count();

    processes[processId].pageTable[pageNumber].valid = 1;
    processes[processId].pageTable[pageNumber].frame = frameIndex;
    processes[processId].pages[pageNumber].addedTime = now;
    processes[processId].pages[pageNumber].lastAccessedTime = now;

    for (int i = 0; i < pageSize; i++)
    {
        int index = frameIndex * pageSize + i;
        memory[index].occupied = 1;
        memory[index].processId = processId;
        memory[index].pageNumber = pageNumber;
        memory[index].usedBit = 1;
    }
}

void load(int processId, int pageNumber)
{
    for (int i = 0; i < MEMORY_SIZE; i += pageSize)
    {
        if (!memory[i].occupied)
        {
            useFrame(i / pageSize, processId, pageNumber);
            return;
        }
    }
    int frameToUse = replacement();
    useFrame(frameToUse, processId, pageNumber);
}

int findNext(int processId, int pageNumber)
{
    int pageCount = ceil(float(processes[processId].totalMemory) / pageSize);
    for (int i = pageNumber + 1; i < pageCount; i++)
    {
        if (!processes[processId].pageTable[i].valid)
            return i;
    }
    for (int i = 0; i < pageNumber; i++)
    {
        if (!processes[processId].pageTable[i].valid)
            return i;
    }
    return -1;
}

void prePaging(int processId, int pageNumber)
{
    load(processId, pageNumber);
    int nextPageNumber = findNext(processId, pageNumber);
    if (nextPageNumber != -1)
        load(processId, nextPageNumber);
}

void demandPaging(int processId, int pageNumber)
{
    load(processId, pageNumber);
}

void paging(int processId, int pageNumber)
{
    if (pagingType == PRE)
        prePaging(processId, pageNumber);
    else
        demandPaging(processId, pageNumber);
}

void check(int processNumber, int pageNumber)
{
    if (!processes[processNumber].pageTable[pageNumber].valid)
    {
        pageFault++;
        paging(processNumber, pageNumber);
    }
    else
    {
        auto elapsed = high_resolution_clock::now() - start;
        processes[processNumber].pages[pageNumber].lastAccessedTime = duration_cast<nanoseconds>(elapsed).count();
        int frame = processes[processNumber].pageTable[pageNumber].frame;
        for (int i = 0; i < pageSize; i++)
        {
            memory[frame * pageSize + i].usedBit = 1;
        }
    }
}

void printData()
{
    cout << "Processes: " << numProcesses << endl;
    cout << "Trace Size: " << processTraceListSize << endl;
    cout << "Total Page Faults: " << pageFault << endl;
    if (printDetails)
    {
        cout << "\nFaults over time (per 1000 accesses):" << endl;
        for (int i = 0; i < pageFaultTracker.size(); i++)
        {
            cout << "  " << (i * 1000) << ": " << pageFaultTracker[i] << endl;
        }
    }
    cout << "\n✅ Simulation Complete" << endl;
}

int main(int argc, char *argv[])
{

    cout << "[DEBUG] Replacement Type = " << replacementType << endl; // debug

    cout << " Simulator Started" << endl;
    if (argc != 7)
    {
        cerr << " Invalid arguments. Expected 6 arguments." << endl;
        return 1;
    }

    for (int i = 0; i < argc; i++)
    {
        cout << "  argv[" << i << "] = " << argv[i] << endl;
    }

    string processListFilename = argv[1];
    string processTraceFilename = argv[2];
    string replacementParam = argv[3];
    string pagingParam = argv[4];
    string pageSizeParam = argv[5];
    string printDetailsParam = argv[6];

    if (replacementParam == "FIFO")
        replacementType = FIFO;
    else if (replacementParam == "LRU")
        replacementType = LRU;
    else if (replacementParam == "CLOCK")
        replacementType = CLOCK;
    else
        replacementType = FIFO;

    cout << "[DEBUG] Replacement type enum value: " << replacementType << endl;

    pagingType = (pagingParam == "PRE") ? PRE : DEMAND;
    printDetails = (printDetailsParam == "1") ? 1 : 0;

    pointer = 0;
    pageFault = 0;
    pageSize = stoi(pageSizeParam);
    processes = readProcessList(processListFilename);
    processTraceList = readProcessTrace(processTraceFilename);

    cout << " Reading process list from: " << processListFilename << endl;
    cout << " Reading trace list from: " << processTraceFilename << endl;

    numProcesses = processes.size();
    processTraceListSize = processTraceList.size();
    memory = vector<MemoryEntry>(MEMORY_SIZE);

    for (int i = 0; i < numProcesses; i++)
    {
        int pageCount = (processes[i].totalMemory + pageSize - 1) / pageSize;
        processes[i].pages = vector<Page>(pageCount);
        processes[i].pageTable = vector<PageTableEntry>(pageCount);
        for (int j = 0; j < pageCount; j++)
        {
            processes[i].pages[j].pageNumber = j;
            processes[i].pages[j].processId = i;
            processes[i].pageTable[j].pageNumber = j;
        }
    }

    pageFaultTracker.push_back(0);

    while (!processTraceList.empty())
    {
        ProcessTraceEntry ptrace = processTraceList.front();
        processTraceList.pop();

        int processNumber = ptrace.processId;
        int pageNumber = ptrace.memoryLocation / pageSize;

        check(processNumber, pageNumber);
        counter++;

        if (counter % 1000 == 0)
        {
            pageFaultTracker.push_back(pageFault);
        }
    }

    if (counter % 1000 != 0)
    {
        pageFaultTracker.push_back(pageFault);
    }

    printData();
    return 0;
}
// g++ full_simulator.cpp simInput.cpp -o full_simulator
// .\full_simulator plist.txt ptrace.txt FIFO DEMAND 2 1
// cd ../frontend
// streamlit run vmms_app.py
