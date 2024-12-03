#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <mutex>

using namespace std;

mutex outputMutex; // Pour synchroniser les sorties des threads
string getBanner(const string &ip,int port) {
    int sock;
    struct sockaddr_in server;
    char buffer[1024];

    sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock < 0){
        cerr << "Erreur creation du socket";
        return "";
    }
    memset(&server,0,sizeof(server))
    server.sin_family = AF_INET; 
    server.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &server.sin_addr);

    if(connect(sock,(struct sockaddr *)&server, sizeof(server)) == 0){
        recv(sock,buffer,sizeof(buffer) - 1.0);
    }else{
        close(sock);
        return "";
    }
    close(sock);
    return string(buffer);
}

void scanPort(const string &ip, int port, const string &banner) {

    regex versionRegex(R"(([0-9]+\.[0-9]+(\.[0-9]+)?))");
    smatch matches;
    if(regex_search(banner,matches,versionRegex)) {
        return matches[0];
    }else{
        return "Version non trouvée";
    }


    int sock;
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "Erreur lors de la création du socket.\n";
        return;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &server.sin_addr);

    // Tentative de connexion
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == 0) {
        lock_guard<mutex> lock(outputMutex); // Empêche les sorties concurrentes
        cout << "Port " << port << " est ouvert";

        // Détection des services courants
        switch (port) {
            case 22: cout << " (SSH)    version" << matches[0]; break;
            case 21: cout << " (FTP)    version" << matches[0]; break;
            case 80: cout << " (HTTP)   version" << matches[0]; break;
            case 443: cout << " (HTTPS) version" << matches[0]; break;
            case 25: cout << " (SMTP)   version" << matches[0]; break;
            case 110: cout << " (POP3)  version" << matches[0]; break;
            case 3306: cout << " (MySQL)    version" << matches[0]; break;
            case 23: cout << " (Telnet) version" << matches[0]; break;
            case 53: cout << " (DNS)    " << matches[0]; break;
            default: cout << " (Service inconnu)";
        }
        cout << endl;
    }

    close(sock);
}

void scanRange(const string &ip, int startPort, int endPort, int numThreads) {
    vector<thread> threads;

    for (int port = startPort; port <= endPort; ++port) {
        threads.emplace_back(scanPort, ip, port);

        if (threads.size() >= numThreads) {
            for (auto &t : threads) {
                if (t.joinable()) t.join();
            }
            threads.clear();
        }
    }

    for (auto &t : threads) {
        if (t.joinable()) t.join();
    }
}

int main() {
    string targetIP;
    int startPort = 1, endPort = 65535;
    int numThreads = 10; 
    int choix;

    cout << "1) Scanner tous les ports (1-65535)" << endl;
    cout << "2) Scanner une plage de ports spécifique" << endl;
    cout << "Entrez votre choix : ";
    cin >> choix;

    if (choix == 1) {
        cout << "Entrez l'adresse IP cible : ";
        cin >> targetIP;
        cout << "Scan de l'adresse " << targetIP << " sur les ports de " << startPort << " a " << endPort << "...\n";
        scanRange(targetIP, startPort, endPort, numThreads);
    } else if (choix == 2) {
        cout << "Entrez l'IP cible : ";
        cin >> targetIP;
        cout << "Entrez le port de depart : ";
        cin >> startPort;
        cout << "Entrez le port de fin : ";
        cin >> endPort;
        cout << "Scan de l'adresse " << targetIP << " sur les ports de " << startPort << " a " << endPort << "...\n";
        scanRange(targetIP, startPort, endPort, numThreads);
    } else {
        cout << "Choix invalide. Fin du programme.\n";
    }

    cout << "Scan terminé.\n";
    return 0;
}
