// #include <iostream>
// #include <fstream>
// #include <unordered_map>
// #include <set>
// #include <queue>
// #include <list>
// #include <tuple>
// #include <algorithm>

// using namespace std;

// int PAGE_SIZE = 1;
// int MEMORY_FRAMES = 3;
// string ALGORITHM = "FIFO"; // default

// struct Page
// {
//     int pid;
//     int page_no;

//     bool operator<(const Page &other) const
//     {
//         return tie(pid, page_no) < tie(other.pid, other.page_no);
//     }

//     bool operator==(const Page &other) const
//     {
//         return pid == other.pid && page_no == other.page_no;
//     }
// };

// int main(int argc, char *argv[])
// {
//     cout << "Argument count: " << argc << endl;

//     if (argc != 4)
//     {
//         cout << "Usage: vmsimulator.exe <PAGE_SIZE> <MEMORY_FRAMES> <ALGORITHM>" << endl;
//         return 1;
//     }

//     PAGE_SIZE = stoi(argv[1]);
//     MEMORY_FRAMES = stoi(argv[2]);
//     ALGORITHM = argv[3]; // "FIFO", "LRU", or "Clock"

//     ifstream plist("../backend/plist.txt");
//     ifstream ptrace("../backend/ptrace.txt");

//     if (!plist.is_open() || !ptrace.is_open())
//     {
//         cerr << "Error: Could not open input files." << endl;
//         return 1;
//     }

//     unordered_map<int, int> process_sizes;
//     int pid, size;
//     while (plist >> pid >> size)
//     {
//         process_sizes[pid] = size;
//     }

//     set<Page> in_memory;
//     int page_faults = 0;

//     if (ALGORITHM == "FIFO")
//     {
//         queue<Page> memory_queue;
//         int ref_pid, addr;
//         while (ptrace >> ref_pid >> addr)
//         {
//             int page_num = addr / PAGE_SIZE;
//             Page p = {ref_pid, page_num};

//             if (in_memory.find(p) == in_memory.end())
//             {
//                 page_faults++;
//                 if ((int)memory_queue.size() >= MEMORY_FRAMES)
//                 {
//                     Page evict = memory_queue.front();
//                     memory_queue.pop();
//                     in_memory.erase(evict);
//                 }
//                 memory_queue.push(p);
//                 in_memory.insert(p);
//             }
//         }
//     }
//     else if (ALGORITHM == "LRU")
//     {
//         list<Page> lru_list;
//         int ref_pid, addr;
//         while (ptrace >> ref_pid >> addr)
//         {
//             int page_num = addr / PAGE_SIZE;
//             Page p = {ref_pid, page_num};

//             auto it = find(lru_list.begin(), lru_list.end(), p);
//             if (it == lru_list.end())
//             {
//                 page_faults++;
//                 if ((int)lru_list.size() >= MEMORY_FRAMES)
//                 {
//                     in_memory.erase(lru_list.back());
//                     lru_list.pop_back();
//                 }
//             }
//             else
//             {
//                 lru_list.erase(it); // Move to front
//             }
//             lru_list.push_front(p);
//             in_memory.insert(p);
//         }
//     }
//     else if (ALGORITHM == "Clock")
//     {
//         vector<Page> clock;
//         vector<bool> ref_bit;
//         int pointer = 0;
//         int ref_pid, addr;

//         while (ptrace >> ref_pid >> addr)
//         {
//             int page_num = addr / PAGE_SIZE;
//             Page p = {ref_pid, page_num};

//             auto it = find(clock.begin(), clock.end(), p);
//             if (it != clock.end())
//             {
//                 int index = distance(clock.begin(), it);
//                 ref_bit[index] = true;
//             }
//             else
//             {
//                 page_faults++;
//                 if ((int)clock.size() < MEMORY_FRAMES)
//                 {
//                     clock.push_back(p);
//                     ref_bit.push_back(true);
//                 }
//                 else
//                 {
//                     while (ref_bit[pointer])
//                     {
//                         ref_bit[pointer] = false;
//                         pointer = (pointer + 1) % MEMORY_FRAMES;
//                     }
//                     in_memory.erase(clock[pointer]);
//                     clock[pointer] = p;
//                     ref_bit[pointer] = true;
//                     pointer = (pointer + 1) % MEMORY_FRAMES;
//                 }
//                 in_memory.insert(p);
//             }
//         }
//     }
//     else
//     {
//         cout << "Invalid algorithm selected." << endl;
//         return 1;
//     }

//     cout << "Page Replacement: " << ALGORITHM << endl;
//     cout << "Total Page Faults: " << page_faults << endl;

//     return 0;
// }
