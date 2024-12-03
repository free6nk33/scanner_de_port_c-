#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <thread>
#include <cstring> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;


string getVersion(const string& ip, int port) {
    int sock;
    struct sockaddr_in server;
    char buffer[1024];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        cerr << "Erreur de creation du socket" << endl;
        return "Erreur";
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &server.sin_addr);


    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        cerr << "Impossible de se connecter à " << ip << ":" << port << endl;
        close(sock);
        return "Erreur";
    }

    //Reception de la version
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived < 0) {
        cerr << "Erreur de recuperation de la version" << endl;
        close(sock);
        return "Erreur";
    }

    close(sock);
    return string(buffer);
}

// Fonction pour scanner un port
void scanPort(const string& ip, int port) {
    string banner = getVersion(ip, port);
    regex versionRegex(R"(([0-9]+\.[0-9]+(\.[0-9]+)?))");
    smatch matches;

    cout << "Port " << port;

    if (regex_search(banner, matches, versionRegex)) {
        switch (port) {
            case 25:
                cout << " (SMTP)   version " << matches[0];
                break;
            case 110:
                cout << " (POP3)  version " << matches[0];
                break;
            case 3306:
                cout << " (MySQL)  version " << matches[0];
                break;
            case 23:
                cout << " (Telnet) version " << matches[0];
                break;
            case 53:
                cout << " (DNS)    " << matches[0];
                break;
            default:
                cout << " (Service inconnu) version " << matches[0];
        }
        cout << endl;
    } else {
        cout << ": Version non trouvée" << endl;
    }
}

int main() {
    string ip;
    vector<int> ports = {22, 25, 53, 80, 110, 443, 3306, 23}; //Les ports a scan

    cout << "IP : ";
    cin >> ip;

    vector<thread> threads;

    for (int port : ports) {
        threads.emplace_back(scanPort, ip, port);
    }

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
