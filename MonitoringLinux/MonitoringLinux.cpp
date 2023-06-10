#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

using namespace std;

class Computer {
private:
    ifstream statFile;

public:
    float getCpuLoad() {
        statFile.open("/proc/stat");
        string line;
        getline(statFile, line);
        statFile.close();

        size_t pos = line.find("cpu");
        if (pos == string::npos)
            return -1;

        line = line.substr(pos + 4);  // Удаляем "cpu " префикс

        float user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
        sscanf(line.c_str(), "%f %f %f %f %f %f %f %f %f %f",
            &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);

        float idleTicks = idle + iowait;
        float systemTicks = system + irq + softirq;
        float totalTicks = idleTicks + systemTicks + user + nice + steal;

        float cpuLoad = (totalTicks - idleTicks) / totalTicks * 100.0;

        return cpuLoad;
    }

    void monitorCpuLoad() {
        while (true) {
            float cpuLoad = getCpuLoad();
            if (cpuLoad == -1) {
                cout << "Failed to read CPU load." << endl;
            }
            else {
                cout << "CPU Load: " << cpuLoad << "%" << endl;
            }
            usleep(1000000);  // Пауза на 1 секунду 
        }
    }
};

int main() {
    Computer computer;
    computer.monitorCpuLoad();

    return 0;
}
