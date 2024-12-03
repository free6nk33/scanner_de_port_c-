FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive


RUN apt-get update && \
    apt-get install -y \
    apache2 \
    openssh-server \
    mysql-server \
    vsftpd \
    supervisor && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Préparer les services
RUN mkdir /var/run/sshd && \
    echo 'root:root' | chpasswd && \
    echo "ServerName localhost" >> /etc/apache2/apache2.conf

# Configurer le superviseur
COPY supervisord.conf /etc/supervisor/conf.d/supervisord.conf

# Répertoires de travail
WORKDIR /scanner


EXPOSE 80 21 22 3306

# Commande par défaut
CMD ["/usr/bin/supervisord"]
