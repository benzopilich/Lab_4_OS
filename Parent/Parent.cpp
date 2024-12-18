#include <windows.h>
#include <iostream>
#include <string>
#include <thread>

void ReceiveFromChild() {
    HANDLE hEventC = OpenEvent(SYNCHRONIZE, FALSE, TEXT("EventC"));
    HANDLE hEventD = OpenEvent(SYNCHRONIZE, FALSE, TEXT("EventD"));
    HANDLE events[] = { hEventC, hEventD };

    if (!hEventC || !hEventD) {
        std::cerr << "Parent: Error opening events for messages C and D." << std::endl;
        return;
    }

    while (true) {
        DWORD waitResult = WaitForMultipleObjects(2, events, FALSE, 100); // ����-��� ��� ����������
        if (waitResult == WAIT_OBJECT_0) {
            std::cout << "Parent: Received message C from Child." << std::endl;
        }
        else if (waitResult == WAIT_OBJECT_0 + 1) {
            std::cout << "Parent: Received message D from Child." << std::endl;
        }
    }

    CloseHandle(hEventC);
    CloseHandle(hEventD);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Parent: Delay parameter missing." << std::endl;
        return 1;
    }

    int delayMs = std::stoi(argv[1]);

    HANDLE hSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, TEXT("ParentSyncSemaphore"));
    HANDLE hEventA = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("EventA"));
    HANDLE hEventB = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("EventB"));
    HANDLE hEventParentDone = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("ParentDoneEvent"));

    if (!hSemaphore || !hEventA || !hEventB || !hEventParentDone) {
        std::cerr << "Parent: Error opening synchronization objects." << std::endl;
        return 1;
    }

    // �������� ���������� ���������
    int messageCount;
    std::cout << "Enter the number of messages to send: ";
    std::cin >> messageCount;

    std::thread receiverThread(ReceiveFromChild);

    for (int i = 0; i < messageCount; ++i) {
        WaitForSingleObject(hSemaphore, INFINITE);
        std::cout << "Activ: Parent acquired semaphore." << std::endl;

        std::string message;
        std::cout << "Enter message A or B: ";
        std::cin >> message;

        if (message == "A") {
            SetEvent(hEventA);
        }
        else if (message == "B") {
            SetEvent(hEventB);
        }

        ReleaseSemaphore(hSemaphore, 1, NULL);

        // �������� ����� ��������� ���������� �������
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
    }

    SetEvent(hEventParentDone);
    receiverThread.join();

    CloseHandle(hSemaphore);
    CloseHandle(hEventA);
    CloseHandle(hEventB);
    return 0;
}
