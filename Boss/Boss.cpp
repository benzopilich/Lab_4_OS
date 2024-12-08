#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

int main() {
    // Создание синхронизационных объектов
    HANDLE hMutex = CreateMutex(NULL, FALSE, TEXT("ChildSyncMutex"));
    HANDLE hSemaphore = CreateSemaphore(NULL, 2, 2, TEXT("ParentSyncSemaphore"));
    HANDLE hEventParentDone = CreateEvent(NULL, TRUE, FALSE, TEXT("ParentDoneEvent"));
    HANDLE hEventChildDone = CreateEvent(NULL, TRUE, FALSE, TEXT("ChildDoneEvent"));
    HANDLE hEventA = CreateEvent(NULL, FALSE, FALSE, TEXT("EventA")); // auto-reset
    HANDLE hEventB = CreateEvent(NULL, FALSE, FALSE, TEXT("EventB")); // auto-reset
    HANDLE hEventC = CreateEvent(NULL, FALSE, FALSE, TEXT("EventC")); // auto-reset
    HANDLE hEventD = CreateEvent(NULL, FALSE, FALSE, TEXT("EventD")); // auto-reset

    if (!hMutex || !hSemaphore || !hEventParentDone || !hEventChildDone ||
        !hEventA || !hEventB || !hEventC || !hEventD) {
        std::cerr << "Boss: Error creating synchronization objects." << std::endl;
        return 1;
    }

    int parentCount, childCount;
    std::cout << "Enter number of Parent processes: ";
    std::cin >> parentCount;
    std::cout << "Enter number of Child processes: ";
    std::cin >> childCount;

    // Запуск процессов Parent
    std::vector<HANDLE> parentProcesses;
    for (int i = 0; i < parentCount; ++i) {
        STARTUPINFO si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        std::wstring command = L"Parent.exe";
        if (CreateProcess(NULL, &command[0], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
            parentProcesses.push_back(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else {
            std::cerr << "Boss: Failed to create Parent process #" << i + 1 << std::endl;
        }
    }

    // Запуск процессов Child
    std::vector<HANDLE> childProcesses;
    for (int i = 0; i < childCount; ++i) {
        STARTUPINFO si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        std::wstring command = L"Child.exe";
        if (CreateProcess(NULL, &command[0], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
            childProcesses.push_back(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else {
            std::cerr << "Boss: Failed to create Child process #" << i + 1 << std::endl;
        }
    }

    // Ожидание завершения всех процессов
    WaitForSingleObject(hEventParentDone, INFINITE);
    WaitForSingleObject(hEventChildDone, INFINITE);

    std::cout << "All processes completed their work." << std::endl;

    for (HANDLE hProcess : parentProcesses) {
        CloseHandle(hProcess);
    }
    for (HANDLE hProcess : childProcesses) {
        CloseHandle(hProcess);
    }

    CloseHandle(hMutex);
    CloseHandle(hSemaphore);
    CloseHandle(hEventParentDone);
    CloseHandle(hEventChildDone);
    CloseHandle(hEventA);
    CloseHandle(hEventB);
    CloseHandle(hEventC);
    CloseHandle(hEventD);

    return 0;
}
