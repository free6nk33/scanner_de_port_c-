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

// Fonction pour récupérer la version
string getVersion(const string& ip, int port) {
    int sock;
    struct sockaddr_in server;
    char buffer[1024];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        cerr << "Erreur de création du socket" << endl;
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

    // Réception de la version
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived < 0) {
        cerr << "Erreur de récupération de la version" << endl;
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
            case 80:
                cout << " (HTTP/TCP)    version " << matches[0];
                break;
            case 443:
                cout << " (HTTPS/TCP)   version " << matches[0];
                break;
            case 22:
                cout << " (SSH/TCP)     version " << matches[0];
                break;
            case 21:
                cout << " (FTP/TCP)     version " << matches[0];
                break;
            case 25:
                cout << " (SMTP/TCP)    version " << matches[0];
                break;
            case 110:
                cout << " (POP3/TCP)    version " << matches[0];
                break;
            case 3306:
                cout << " (MySQL/TCP)   version " << matches[0];
                break;
            case 23:
                cout << " (Telnet/TCP)  version " << matches[0];
                break;
            case 53:
                cout << " (DNS/TCP)     version " << matches[0];
                break;
            default:
                cout << " (Service inconnu/?) version " << matches[0];
                break;
        }
    } else {
        cout << ": Version non trouvée ou bannière vide";
    }
    cout << endl;
}

int main() {
    string ip;
    vector<int> ports = {22, 25, 53, 80, 110, 443, 3306, 23}; // Les ports à scanner
    int choix;

    cout << "1) Scanner tous les ports" << endl;
    cout << "2) Scanner uniquement HTTP/HTTPS et SSH" << endl;
    cout << "NSC> ";
    cin >> choix;

    vector<thread> threads;

    if (choix == 1) {
        cout << "IP : ";
        cin >> ip;
        for (int port : ports) {
            threads.emplace_back(scanPort, ip, port);
        }
    } else if (choix == 2) {
        cout << "IP : ";
        cin >> ip;
        vector<int> port_ssh_http = {22, 80, 443};
        for (int port : port_ssh_http) {
            threads.emplace_back(scanPort, ip, port);
        }
    } else {
        cout << "Choix invalide !" << endl;
        return 1;
    }

    // Attente de la fin des threads
    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
