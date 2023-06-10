#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <cstdlib>

using namespace std;

class Computer {
private:
    ifstream statFile;

public:
    float getCpuLoad() {
        // Получение нагрузки ЦП
        statFile.open("/proc/stat");  // Открываем файл /proc/stat, содержащий информацию о статистике ЦП
        string line;
        getline(statFile, line);
        statFile.close();

        size_t pos = line.find("cpu");  // Находим строку, начинающуюся с "cpu"
        if (pos == string::npos)
            return -1;  // Если строка не найдена, возвращаем -1

        line = line.substr(pos + 4);  // Убираем префикс "cpu "

        // Парсинг данных о нагрузке ЦП
        float user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
        sscanf(line.c_str(), "%f %f %f %f %f %f %f %f %f %f",
            &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);

        float idleTicks = idle + iowait;
        float systemTicks = system + irq + softirq;
        float totalTicks = idleTicks + systemTicks + user + nice + steal;

        float cpuLoad = (totalTicks - idleTicks) / totalTicks * 100.0;

        return cpuLoad;
    }

    float getGpuLoad() {
        // Реализация для получения загрузки графического процессора Radeon Sapphire 4670
        string command = "radeontop -d 1 -b --no-summary | awk '/GPU load/ {print $4}'";
        string result = executeCommand(command);

        if (result.empty())
            return -1;

        float gpuLoad = stof(result);

        return gpuLoad;
    }

    float getMemoryLoad() {
        // Получаем загрузку ОЗУ
        string command = "free -m | awk '/Mem:/ {print $3}'";
        string result = executeCommand(command);

        if (result.empty())
            return -1;

        float memoryLoad = stof(result);

        return memoryLoad;
    }

    float getTotalMemory() {
        // Общий объем ОЗУ
        string command = "free -m | awk '/Mem:/ {print $2}'";
        string result = executeCommand(command);

        if (result.empty())
            return -1;

        float totalMemory = stof(result);

        return totalMemory;
    }

    string executeCommand(const string& command) {
        // Выполняет команду в командной оболочке и возвращает ее вывод в виде строки
        string result = "";
        FILE* pipe = popen(command.c_str(), "r");
        if (pipe) {
            char buffer[128];
            while (!feof(pipe)) {
                if (fgets(buffer, 128, pipe) != NULL)
                    result += buffer;
            }
            pclose(pipe);
        }
        return result;
    }

    void monitorSystemLoad() {
        // Главный метод мониторинга загрузки системы
        while (true) {
            // Получаем и выводим загрузку ЦП
            float cpuLoad = getCpuLoad();
            if (cpuLoad == -1) {
                cout << "Не удалось получить загрузку ЦП." << endl;
            }
            else {
                cout << "Загрузка ЦП: " << cpuLoad << "%" << endl;
            }

            // Получаем и выводим загрузку графического процессора
            float gpuLoad = getGpuLoad();
            if (gpuLoad == -1) {
                cout << "Не удалось получить загрузку графического процессора." << endl;
            }
            else {
                cout << "Загрузка графического процессора: " << gpuLoad << "%" << endl;
            }

            // Получаем и выводим загрузку памяти
            float memoryLoad = getMemoryLoad();
            if (memoryLoad == -1) {
                cout << "Не удалось получить загрузку памяти." << endl;
            }
            else {
                // Получаем общий объем памяти
                float totalMemory = getTotalMemory();
                if (totalMemory == -1) {
                    cout << "Не удалось получить общий объем памяти." << endl;
                }
                else {
                    cout << "Загрузка памяти: " << memoryLoad << "MB / " << totalMemory << "MB (" << (memoryLoad / totalMemory) * 100 << "%)" << endl;
                }
            }

            cout << endl;

            usleep(1000000);  // Пауза на 1 секунду
        }
    }
};

int main() {
    Computer computer;
    computer.monitorSystemLoad();

    return 0;
}
