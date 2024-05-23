#define WIN32_LEAN_AND_MEAN // Уменьшение размера заголовочных файлов Windows

#include <Windows.h>  // Включение заголовочного файла Windows
#include <iostream>   // Включение заголовочного файла для работы с вводом-выводом
#include <WinSock2.h> // Включение заголовочного файла для использования сокетов
#include <WS2tcpip.h> // Включение заголовочного файла для работы с IP-адресами

using namespace std; // Использование стандартного пространства имен

int main() {
    WSADATA wsaData; // Структура для хранения информации о версии Winsock
    ADDRINFO hints; // Структура для хранения критериев выбора IP-адресов
    ADDRINFO* addrResult; // Указатель для хранения результатов getaddrinfo
    SOCKET ListenSocket = INVALID_SOCKET; // Сокет для прослушивания входящих подключений
    SOCKET ConnectSocket = INVALID_SOCKET; // Сокет для взаимодействия с клиентом
    char recvBuffer[512]; // Буфер для приема данных

    const char* sendBuffer = "Hello from server"; // Сообщение, которое будет отправлено клиенту

    // Инициализация Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed with result: " << result << endl;
        return 1; // Завершение программы при ошибке
    }

    // Обнуление структуры hints и задание параметров
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET; // Использование IPv4
    hints.ai_socktype = SOCK_STREAM; // Использование TCP
    hints.ai_protocol = IPPROTO_TCP; // Протокол TCP
    hints.ai_flags = AI_PASSIVE; // Указание на использование локального IP-адреса

    // Получение адресной информации для создания сокета
    result = getaddrinfo(NULL, "666", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo failed with error: " << result << endl;
        freeaddrinfo(addrResult); // Освобождение памяти
        WSACleanup(); // Завершение работы Winsock
        return 1; // Завершение программы при ошибке
    }

    // Создание сокета для прослушивания
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult); // Освобождение памяти
        WSACleanup(); // Завершение работы Winsock
        return 1; // Завершение программы при ошибке
    }

    // Привязка сокета к адресу и порту
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Bind failed, error: " << result << endl;
        closesocket(ListenSocket); // Закрытие сокета
        freeaddrinfo(addrResult); // Освобождение памяти
        WSACleanup(); // Завершение работы Winsock
        return 1; // Завершение программы при ошибке
    }

    // Перевод сокета в режим прослушивания
    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        cout << "Listen failed, error: " << result << endl;
        closesocket(ListenSocket); // Закрытие сокета
        freeaddrinfo(addrResult); // Освобождение памяти
        WSACleanup(); // Завершение работы Winsock
        return 1; // Завершение программы при ошибке
    }

    // Принятие входящего подключения
    ConnectSocket = accept(ListenSocket, NULL, NULL);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Accept failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket); // Закрытие сокета
        freeaddrinfo(addrResult); // Освобождение памяти
        WSACleanup(); // Завершение работы Winsock
        return 1; // Завершение программы при ошибке
    }

    // Закрытие сокета для прослушивания, так как больше не нужен
    closesocket(ListenSocket);


    // Цикл для получения данных от клиента и отправки ответа
    do {
        ZeroMemory(recvBuffer, 512); // Очистка буфера приема
        result = recv(ConnectSocket, recvBuffer, 512, 0); // Получение данных от клиента
        if (result > 0) {
            cout << "Received " << result << " bytes" << endl;
            cout << "Received data: " << recvBuffer << endl;

            // Отправка ответа клиенту
            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) {
                cout << "Send failed, error: " << result << endl;
                closesocket(ConnectSocket); // Закрытие сокета
                freeaddrinfo(addrResult); // Освобождение памяти
                WSACleanup(); // Завершение работы Winsock
                return 1; // Завершение программы при ошибке
            }
        }
        else if (result == 0) {
            cout << "Connection closing" << endl; // Клиент закрыл соединение
        }
        else {
            cout << "Recv failed, error: " << WSAGetLastError() << endl;
            closesocket(ConnectSocket); // Закрытие сокета
            freeaddrinfo(addrResult); // Освобождение памяти
            WSACleanup(); // Завершение работы Winsock
            return 1; // Завершение программы при ошибке
        }
    } while (result > 0);

    // Завершение отправки данных
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Shutdown failed, error: " << result << endl;
        closesocket(ConnectSocket); // Закрытие сокета
        freeaddrinfo(addrResult); // Освобождение памяти
        WSACleanup(); // Завершение работы Winsock
        return 1; // Завершение программы при ошибке
    }

    // Закрытие сокета для взаимодействия с клиентом
    closesocket(ConnectSocket);
    freeaddrinfo(addrResult); // Освобождение памяти, выделенной для адресной информации
    WSACleanup(); // Завершение работы Winsock
    return 0; // Успешное завершение программы
}
