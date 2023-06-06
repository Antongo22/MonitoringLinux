#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

using namespace std;

float getCpuLoad() {
    ifstream statFile("/proc/stat");
    string line;
    getline(statFile, line);
    statFile.close();

    size_t pos = line.find("cpu");
    if (pos == string::npos)
        return -1;

    line = line.substr(pos + 3);  // Skip "cpu" prefix

    int user, nice, system, idle;
    sscanf(line.c_str(), "%d %d %d %d", &user, &nice, &system, &idle);

    float total = user + nice + system + idle;
    float cpuLoad = (total - idle) / total * 100.0;

    return cpuLoad;
}

int main() {
    while (true) {
        float cpuLoad = getCpuLoad();
        if (cpuLoad == -1) {
            cout << "Failed to read CPU load." << endl;
        }
        else {
            cout << "CPU Load: " << cpuLoad << "%" << endl;
        }
        usleep(1000000);  // Pause for 1 second
    }

    return 0;
}
