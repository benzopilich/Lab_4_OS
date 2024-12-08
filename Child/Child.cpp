#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>

std::atomic<bool> running = true;

void ReceiveFromParent() {
    HANDLE hEventA = OpenEvent(SYNCHRONIZE, FALSE, TEXT("EventA"));
    HANDLE hEventB = OpenEvent(SYNCHRONIZE, FALSE, TEXT("EventB"));
    HANDLE events[] = { hEventA, hEventB };

    if (!hEventA || !hEventB) {
        std::cerr << "Child: Error opening events for messages A and B." << std::endl;
        return;
    }

    while (running) {
        DWORD waitResult = WaitForMultipleObjects(2, events, FALSE, 100); // Тайм-аут для завершения
        if (waitResult == WAIT_OBJECT_0) {
            std::cout << "Child: Received message A from Parent." << std::endl;
        }
        else if (waitResult == WAIT_OBJECT_0 + 1) {
            std::cout << "Child: Received message B from Parent." << std::endl;
        }
    }

    CloseHandle(hEventA);
    CloseHandle(hEventB);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Child: Delay parameter missing." << std::endl;
        return 1;
    }

    int delayMs = std::stoi(argv[1]);

    HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, TEXT("ChildSyncMutex"));
    HANDLE hEventC = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("EventC"));
    HANDLE hEventD = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("EventD"));
    HANDLE hEventChildDone = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("ChildDoneEvent"));

    if (!hMutex || !hEventC || !hEventD || !hEventChildDone) {
        std::cerr << "Child: Error opening synchronization objects." << std::endl;
        return 1;
    }

    // Получаем количество сообщений
    int messageCount;
    std::cout << "Enter the number of messages to send: ";
    std::cin >> messageCount;

    std::thread receiverThread(ReceiveFromParent);

    for (int i = 0; i < messageCount; ++i) {
        WaitForSingleObject(hMutex, INFINITE);
        std::cout << "Activ: Child acquired mutex." << std::endl;

        std::string message;
        std::cout << "Enter message C or D: ";
        std::cin >> message;

        if (message == "C") {
            SetEvent(hEventC);
        }
        else if (message == "D") {
            SetEvent(hEventD);
        }

        ReleaseMutex(hMutex);

        // Задержка перед отправкой следующего события
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
    }

    running = false; // Завершение потока
    SetEvent(hEventChildDone);
    receiverThread.join();

    CloseHandle(hMutex);
    CloseHandle(hEventC);
    CloseHandle(hEventD);
    return 0;
}
