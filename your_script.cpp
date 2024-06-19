#include <bits/stdc++.h>
using namespace std;

struct Process
{
    int id;
    int arrivalTime;
    int burstTime;
    int priority;
    int finishTime;
    int turnaroundTime;
    int waitingTime;
    int remtime;
};

struct SchedulingStats
{
    std::string name;
    double averageWaitingTime;
    double averageTurnaroundTime;
    double throughput;
};

vector<SchedulingStats> stats;

string findBestAlgorithm(const vector<SchedulingStats> &stats)
{
    auto bestAlgo = std::min_element(stats.begin(), stats.end(), [](const SchedulingStats &a, const SchedulingStats &b)
                                     {
        if (a.averageWaitingTime != b.averageWaitingTime) {
            return a.averageWaitingTime < b.averageWaitingTime;
        } else if (a.averageTurnaroundTime != b.averageTurnaroundTime) {
            return a.averageTurnaroundTime < b.averageTurnaroundTime;
        } else {
            return a.throughput > b.throughput;
        } });

    return bestAlgo->name;
}

void PrintResult(vector<Process> &processes, vector<int> &waitingTimes, vector<int> &turnaroundTimes, int numProcesses, string algoName)
{
    double total_WT = 0, total_TAT = 0;

    for (int i = 0; i < numProcesses; ++i)
    {
        total_WT += waitingTimes[i];
        total_TAT += turnaroundTimes[i];
    }

    double avg_WT = total_WT / numProcesses;
    double avg_TAT = total_TAT / numProcesses;

    int startTime = INT_MAX;
    int finishTime = 0;
    for (const auto &process : processes)
    {
        if (process.arrivalTime < startTime)
        {
            startTime = process.arrivalTime;
        }
        if (process.finishTime > finishTime)
        {
            finishTime = process.finishTime;
        }
    }
    double totalTime = finishTime - startTime;
    double throughput = numProcesses / totalTime;

    cout << left << setw(20) << algoName
         << right << setw(15) << avg_WT
         << right << setw(20) << avg_TAT
         << right << setw(20) << fixed << setprecision(2) << throughput << "\n";

    stats.push_back({algoName, avg_WT, avg_TAT, throughput});
}

void FCFS(vector<Process> processes)
{
    int numProcesses = processes.size();
    vector<int> waitingTimes(numProcesses, 0);
    vector<int> turnaroundTimes(numProcesses, 0);

    sort(processes.begin(), processes.end(), [](const Process &a, const Process &b)
         { return a.arrivalTime < b.arrivalTime; });

    int currentTime = 0;

    for (int i = 0; i < numProcesses; ++i)
    {
        if (currentTime < processes[i].arrivalTime)
        {
            currentTime = processes[i].arrivalTime;
        }

        processes[i].finishTime = currentTime + processes[i].burstTime;
        processes[i].turnaroundTime = processes[i].finishTime - processes[i].arrivalTime;
        processes[i].waitingTime = processes[i].turnaroundTime - processes[i].burstTime;

        currentTime = processes[i].finishTime;

        waitingTimes[i] = processes[i].waitingTime;
        turnaroundTimes[i] = processes[i].turnaroundTime;
    }

    PrintResult(processes, waitingTimes, turnaroundTimes, numProcesses, "FCFS");
}

void SJF(vector<Process> processes)
{
    int numProcesses = processes.size();
    vector<int> waitingTimes(numProcesses, 0);
    vector<int> turnaroundTimes(numProcesses, 0);

    sort(processes.begin(), processes.end(), [](const Process &a, const Process &b)
         {
             if (a.arrivalTime != b.arrivalTime) 
                 return a.arrivalTime < b.arrivalTime; 
             return a.burstTime < b.burstTime; });

    vector<bool> completed(numProcesses, false);
    int currentTime = 0;
    int completedCount = 0;

    while (completedCount < numProcesses)
    {
        int shortestIndex = -1;
        int shortestBurst = INT_MAX;

        for (int i = 0; i < numProcesses; ++i)
        {
            if (!completed[i] && processes[i].arrivalTime <= currentTime && processes[i].burstTime < shortestBurst)
            {
                shortestBurst = processes[i].burstTime;
                shortestIndex = i;
            }
        }

        if (shortestIndex == -1)
        {
            currentTime++;
            continue;
        }

        Process &process = processes[shortestIndex];
        currentTime += process.burstTime;
        process.finishTime = currentTime;
        process.turnaroundTime = process.finishTime - process.arrivalTime;
        process.waitingTime = process.turnaroundTime - process.burstTime;

        waitingTimes[shortestIndex] = process.waitingTime;
        turnaroundTimes[shortestIndex] = process.turnaroundTime;

        completed[shortestIndex] = true;
        completedCount++;
    }

    PrintResult(processes, waitingTimes, turnaroundTimes, numProcesses, "SJF");
}

void SJF_Preemptive(vector<Process> processes)
{
    int numProcesses = processes.size();
    vector<int> waitingTimes(numProcesses, 0);
    vector<int> turnaroundTimes(numProcesses, 0);

    // Sort processes by arrival time first, then by burst time
    sort(processes.begin(), processes.end(), [](const Process &a, const Process &b)
         { 
             if (a.arrivalTime != b.arrivalTime) 
                 return a.arrivalTime < b.arrivalTime; 
             return a.burstTime < b.burstTime; });

    int currentTime = processes[0].arrivalTime;
    vector<int> remainingBurstTimes(numProcesses);
    for (int i = 0; i < numProcesses; ++i)
    {
        remainingBurstTimes[i] = processes[i].burstTime;
    }

    vector<bool> completed(numProcesses, false);
    int completedCount = 0;

    while (completedCount < numProcesses)
    {
        int shortestIndex = -1;
        int shortestBurst = INT_MAX;

        // Find the process with the shortest remaining burst time that has arrived
        for (int i = 0; i < numProcesses; ++i)
        {
            if (!completed[i] && processes[i].arrivalTime <= currentTime && remainingBurstTimes[i] < shortestBurst)
            {
                shortestBurst = remainingBurstTimes[i];
                shortestIndex = i;
            }
        }

        if (shortestIndex == -1)
        {
            currentTime++;
            continue;
        }

        Process &process = processes[shortestIndex];
        int executeTime = 1; // Preemptive SJF executes in small units of time
        currentTime += executeTime;
        remainingBurstTimes[shortestIndex] -= executeTime;

        if (remainingBurstTimes[shortestIndex] == 0)
        {
            process.finishTime = currentTime;
            process.turnaroundTime = process.finishTime - process.arrivalTime;
            process.waitingTime = process.turnaroundTime - process.burstTime;

            waitingTimes[shortestIndex] = process.waitingTime;
            turnaroundTimes[shortestIndex] = process.turnaroundTime;

            completed[shortestIndex] = true;
            completedCount++;
        }
    }

    PrintResult(processes, waitingTimes, turnaroundTimes, numProcesses, "SJF(Pre-emp)");
}

void PriorityNonPreemptive(vector<Process> processes)
{
    int numProcesses = processes.size();
    vector<int> waitingTimes(numProcesses, 0);
    vector<int> turnaroundTimes(numProcesses, 0);

    // Sort by arrival time, then by priority
    sort(processes.begin(), processes.end(), [](const Process &a, const Process &b)
         {
        if (a.arrivalTime != b.arrivalTime) return a.arrivalTime < b.arrivalTime;
        return a.priority < b.priority; });

    vector<bool> completed(numProcesses, false);
    int currentTime = 0;
    int completedCount = 0;

    while (completedCount < numProcesses)
    {
        int indexToExecute = -1;

        // Find the process with the highest priority (lowest priority number) that has arrived
        for (int i = 0; i < numProcesses; ++i)
        {
            if (!completed[i] && processes[i].arrivalTime <= currentTime)
            {
                if (indexToExecute == -1 || processes[i].priority < processes[indexToExecute].priority)
                {
                    indexToExecute = i;
                }
            }
        }

        // If no process is found, increment the current time
        if (indexToExecute == -1)
        {
            currentTime++;
            continue;
        }

        Process &process = processes[indexToExecute];
        currentTime = max(currentTime, process.arrivalTime) + process.burstTime;
        process.finishTime = currentTime;
        process.turnaroundTime = process.finishTime - process.arrivalTime;
        process.waitingTime = process.turnaroundTime - process.burstTime;

        waitingTimes[indexToExecute] = process.waitingTime;
        turnaroundTimes[indexToExecute] = process.turnaroundTime;

        completed[indexToExecute] = true;
        completedCount++;
    }
    PrintResult(processes, waitingTimes, turnaroundTimes, numProcesses, "Priority(Non-Preemp)");
}

void PriorityPreemptive(vector<Process> processes)
{
    int numProcesses = processes.size();
    vector<int> waitingTimes(numProcesses, 0);
    vector<int> turnaroundTimes(numProcesses, 0);

    // Sort by arrival time, then by priority
    sort(processes.begin(), processes.end(), [](const Process &a, const Process &b)
         { 
             if (a.arrivalTime != b.arrivalTime) 
                 return a.arrivalTime < b.arrivalTime; 
             return a.priority < b.priority; });

    int currentTime = 0;
    vector<int> remainingBurstTimes(numProcesses);
    for (int i = 0; i < numProcesses; ++i)
    {
        remainingBurstTimes[i] = processes[i].burstTime;
    }

    vector<bool> completed(numProcesses, false);
    int completedCount = 0;

    while (completedCount < numProcesses)
    {
        int highestPriorityIndex = -1;
        int highestPriority = INT_MAX;

        // Find the process with the highest priority (lowest priority number) that has arrived
        for (int i = 0; i < numProcesses; ++i)
        {
            if (!completed[i] && processes[i].arrivalTime <= currentTime && processes[i].priority < highestPriority)
            {
                highestPriority = processes[i].priority;
                highestPriorityIndex = i;
            }
        }

        if (highestPriorityIndex == -1)
        {
            currentTime++;
            continue;
        }

        Process &process = processes[highestPriorityIndex];
        int executeTime = 1; // Preemptive Priority executes in small units of time
        currentTime += executeTime;
        remainingBurstTimes[highestPriorityIndex] -= executeTime;

        if (remainingBurstTimes[highestPriorityIndex] == 0)
        {
            process.finishTime = currentTime;
            process.turnaroundTime = process.finishTime - process.arrivalTime;
            process.waitingTime = process.turnaroundTime - process.burstTime;

            waitingTimes[highestPriorityIndex] = process.waitingTime;
            turnaroundTimes[highestPriorityIndex] = process.turnaroundTime;

            completed[highestPriorityIndex] = true;
            completedCount++;
        }
    }

    PrintResult(processes, waitingTimes, turnaroundTimes, numProcesses, "Priority (pre-emp)");
}

void RoundRobin(vector<Process> processes, int timeQuantum)
{
    int numProcesses = processes.size();
    vector<int> waitingTimes(numProcesses, 0);
    vector<int> turnaroundTimes(numProcesses, 0);
    vector<vector<int>> starttimes(numProcesses);

    int total_wt = 0, total_tat = 0, cnt = numProcesses;
    bool check = false;
    float time_passed = 0, mini = INT_MAX;
    vector<int> bt(numProcesses), at(numProcesses);
    vector<vector<int>> store(numProcesses, vector<int>(20, -1));

    for (int i = 0; i < numProcesses; i++)
    {
        bt[i] = processes[i].burstTime;
        at[i] = processes[i].arrivalTime;
        processes[i].remtime = processes[i].burstTime;
    }
    while (cnt != 0)
    {
        mini = INT_MAX;
        check = false;
        for (int i = 0; i < numProcesses; i++)
        {
            float temp = time_passed + 0.1;
            if (at[i] <= temp && mini > at[i] && processes[i].remtime > 0)
            {
                mini = at[i];
                check = true;
            }
        }
        if (!check)
        {
            time_passed++;
            continue;
        }
        int it = -1;
        for (int i = 0; i < numProcesses; i++)
        {
            float temp = time_passed + 0.1;
            if (at[i] <= temp && mini == at[i] && processes[i].remtime > 0)
            {
                it = i;
                break;
            }
        }
        int k = 0;
        while (store[it][k] != -1)
        {
            k++;
        }

        if (store[it][k] == -1)
        {
            store[it][k] = time_passed;
            // processes[it].start_time[k] = time_passed;
        }

        if (processes[it].remtime <= timeQuantum)
        {
            time_passed += processes[it].remtime;
            processes[it].remtime = 0;
        }
        else
        {
            time_passed += timeQuantum;
            processes[it].remtime -= timeQuantum;
        }

        if (processes[it].remtime > 0)
        {
            at[it] = time_passed + 0.1;
        }

        if (processes[it].remtime == 0)
        {
            cnt--;
            processes[it].finishTime = time_passed;
            processes[it].waitingTime = processes[it].finishTime - processes[it].arrivalTime - processes[it].burstTime;
            processes[it].turnaroundTime = processes[it].burstTime + processes[it].waitingTime;
            waitingTimes[it] = processes[it].waitingTime;
            turnaroundTimes[it] = processes[it].turnaroundTime;
        }
    }
    PrintResult(processes, waitingTimes, turnaroundTimes, numProcesses, "Round Robin");
}

int main()
{
    ifstream inputFile("input.txt");
    if (!inputFile)
    {
        cerr << "Error opening input file." << endl;
        return 1;
    }

    int numProcesses;
    inputFile >> numProcesses;

    vector<Process> processes(numProcesses);
    for (int i = 0; i < numProcesses; ++i)
    {
        processes[i].id = i;
        inputFile >> processes[i].arrivalTime;
    }

    for (int i = 0; i < numProcesses; ++i)
    {
        inputFile >> processes[i].burstTime;
    }

    for (int i = 0; i < numProcesses; ++i)
    {
        inputFile >> processes[i].priority;
    }

    int timeQuantum;
    inputFile >> timeQuantum;

    inputFile.close();

    cout << "                        Avg. Waiting Time" << "   Avg. Turnaround Time" << "   Throughput" << "\n";
    cout << "\n";
    FCFS(processes);
    cout << "\n";
    SJF(processes);
    cout << "\n";
    SJF_Preemptive(processes);
    cout << "\n";
    PriorityPreemptive(processes);
    cout << "\n";
    PriorityNonPreemptive(processes);
    cout << "\n";
    RoundRobin(processes, timeQuantum);
    cout << "\n";
    cout << "----------------------------------------------------------------------------" << "\n";
    string bestAlgorithm = findBestAlgorithm(stats);
    cout << "\n";
    cout << "CONCLUSION : " << "\n";
    cout << "\n";
    cout << bestAlgorithm << ", will be the best Algorithm to do the processing for given \n      Arrival times, burst times and Priority Order\n";
    cout << "\n";
    cout << "We are assuming the priority order in performance matrices as : \n";
    cout << "                    (Avg. WT > Avg. TAT > Throughput)" << "\n";
    return 0;
}
