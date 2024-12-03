# scanner_de_port_c-
Ce programme est un outil de scan de port pour un petit projet en c++

    g++ -o port_scanner port_scanner.cpp -lpthread
    
Pour build le Dockerfile:


    docker build -t ubuntu-server:1.0 .

Pour lancer le conteneur ubuntu-server-container:

    docker run -d --name ubuntu-server-container -p 80:80 -p 21:21 -p 22:22 -p 3306:3306 ubuntu-server:1.0

Pour vérifier si le conteneur est lancé

    docker ps