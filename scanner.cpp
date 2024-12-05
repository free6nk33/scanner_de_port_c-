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
#include <krb5.h>

using namespace std;

// Fonction pour récupérer la version du service
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

    memset(buffer, 0, sizeof(buffer));
    ssize_t bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived < 0) {
        cerr << "Erreur de réception de la bannière" << endl;
        close(sock);
        return "Erreur";
    }

    close(sock);
    return string(buffer);
}

// Fonction pour vérifier la vulnérabilité EternalBlue
bool checkEternalBlue(const string& ip) {
    int sock;
    sockaddr_in serverAddr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "Erreur lors de la création du socket." << endl;
        return false;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(445);
    if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0) {
        cerr << "Adresse IP invalide : " << ip << endl;
        close(sock);
        return false;
    }

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Impossible de se connecter à " << ip << endl;
        close(sock);
        return false;
    }

    unsigned char smbNegotiate[] = {
        0x00, 0x00, 0x00, 0x90, 0xFF, 0x53, 0x4D, 0x42, 0x72,
        0x00, 0x00, 0x00, 0x00, 0x18, 0x01, 0x28, 0x00, 0x00
    };

    ssize_t sentBytes = send(sock, smbNegotiate, sizeof(smbNegotiate), 0);
    if (sentBytes < 0) {
        cerr << "Erreur lors de l'envoi de la requête SMB" << endl;
        close(sock);
        return false;
    }

    unsigned char response[1024] = {0};
    ssize_t receivedBytes = recv(sock, response, sizeof(response), 0);
    close(sock);

    if (receivedBytes > 0 && response[4] == 0x72) {
        cout << "[ALERTE] Le serveur " << ip << " est vulnérable à EternalBlue" << endl;
        return true;
    }

    cout << "[INFO] Le serveur " << ip << " n'est pas vulnérable à EternalBlue" << endl;
    return false;
}

// Fonction pour scanner les ports
void scanPort(const string& ip, int port) {
    string banner = getVersion(ip, port);
    regex versionRegex(R"(([0-9]+\.[0-9]+(\.[0-9]+)?))");
    smatch matches;

    cout << "Port " << port;
    if (regex_search(banner, matches, versionRegex)) {
        switch (port){
            case 80:
                cout << " : " << matches[0].str() << " (HTTP)" << endl
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
            case 445:
                cout << " (SMB/TCP)     version " << matches[0];
                checkEternalBlue(ip,445);
                break;
            case 88:
                cout << " (Kerberos/TCP)    version " << matches[0];
                if(!kerberosContext(context)){
                    return 1;
                }
                string realm;
                cout << "Entrez le domaine : ";
                cin >> realm;

                authKrb5Ano(context, realm);
                krb5_free_context(context);
                break;
            
            
            default:
                cout << " (Service inconnu/?) version " << matches[0];
                break;
        }
    } else {
        cout <<": Version non trouvée ou bannière vide";
    }
    cout << endl;
}

// Fonction pour scanner uniquement SMB
void scanSmb(const string& ip) {
    scanPort(ip, 445);
}

// Fonction pour initialiser le contexte Kerberos
bool kerberosContext(krb5_context &context) {
    krb5_error_code ret = krb5_init_context(&context);
    if (ret != 0) {
        cerr << "Impossible d'initialiser le contexte Kerberos. Erreur : " << ret << endl;
        return false;
    }
    return true;
}

// Fonction pour tester l'authentification anonyme
void authKrb5Ano(krb5_context &context, const std::string &realm) {
    krb5_error_code ret;
    krb5_creds creds;
    krb5_principal principal = nullptr;
    krb5_ccache cache = nullptr;

    memset(&creds, 0, sizeof(creds));

    // Construire le principal anonyme
    ret = krb5_parse_name(context, "anonymous", &principal);
    if (ret) {
        cerr << "Echec lors de la création du principal anonyme pour le domaine " << realm << ". Erreur : " << ret << endl;
        return;
    }

    // Ouvrir le cache par défaut
    ret = krb5_cc_default(context, &cache);
    if (ret) {
        cerr << "Impossible d'accéder au cache des tickets Kerberos. Erreur : " << ret << endl;
        krb5_free_principal(context, principal);
        return;
    }

    // Tenter d'obtenir des credentials anonymes
    ret = krb5_get_init_creds_password(context, &creds, principal, nullptr, nullptr, nullptr, 0, nullptr, nullptr);
    if (ret == 0) {
        cout << "[INFO] Le domaine " << realm << " accepte l'authentification anonyme (potentiellement vulnérable)." << endl;
    } else {
        cout << "[INFO] Le domaine " << realm << " ne permet pas l'authentification anonyme." << endl;
    }

    // Libérer les ressources
    krb5_free_cred_contents(context, &creds);
    krb5_free_principal(context, principal);
    krb5_cc_close(context, cache);
}

// Fonction principale
int main() {
    krb5_context context;
    string ip;
    int choix;
    vector<thread> threads;

    cout << "1) Scanner tous les ports courants" << endl;
    cout << "2) Scanner uniquement HTTP/HTTPS et SSH" << endl;
    cout << "3) Scanner SMB et vérifier la vulnérabilité EternalBlue" << endl;
    cout << "4) Tester l'authentification Kerberos anonyme" << endl;
    cout << "NSC> ";
    cin >> choix;

    if (choix == 1) {
        cout << "IP : ";
        cin >> ip;
        vector<int> ports = {22, 25, 53, 80, 110, 443, 3306, 23, 445};
        for (int port : ports) {
            threads.emplace_back(scanPort, ip, port);
        }
    } else if (choix == 2) {
        cout << "IP : ";
        cin >> ip;
        vector<int> ports = {22, 80, 443};
        for (int port : ports) {
            threads.emplace_back(scanPort, ip, port);
        }
    } else if (choix == 3) {
        cout << "IP : ";
        cin >> ip;
        threads.emplace_back(scanSmb, ip);
    } else if (choix == 4) {
        // Initialiser le contexte Kerberos
        if (!kerberosContext(context)) {
            return 1;
        }

        string realm;
        cout << "Entrez le domaine : ";
        cin >> realm;

        // Tester l'authentification anonyme
        authKrb5Ano(context, realm);

        // Libérer le contexte Kerberos
        krb5_free_context(context);
    } else {
        cout << "Choix invalide !" << endl;
        return 1;
    }

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
