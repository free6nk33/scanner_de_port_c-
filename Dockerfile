FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive


RUN apt-get update && \
    apt-get install -y \
    apache2 \
    openssh-server \
    mysql-server \
    vsftpd \
    samba \
    supervisor && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Préparer les services
RUN mkdir /var/run/sshd && \
    echo 'root:root' | chpasswd && \
    echo "ServerName localhost" >> /etc/apache2/apache2.conf && \
    mv /etc/samba/smb.conf /etc/samba/smb.conf.bak


RUN echo '[global]' > /etc/samba/smb.conf && \
    echo 'workgroup = WORKGROUP' >> /etc/samba/smb.conf && \
    echo 'server string = Samba Server' >> /etc/samba/smb.conf && \
    echo 'netbios name = vulnerable-smb' >> /etc/samba/smb.conf && \
    echo 'security = user' >> /etc/samba/smb.conf && \
    echo 'map to guest = Bad User' >> /etc/samba/smb.conf && \
    echo 'smb ports = 445' >> /etc/samba/smb.conf && \
    echo 'ntlm auth = yes' >> /etc/samba/smb.conf && \
    echo 'max protocol = NT1' >> /etc/samba/smb.conf && \
    echo 'min protocol = CORE' >> /etc/samba/smb.conf && \
    echo '[share]' >> /etc/samba/smb.conf && \
    echo 'path = /srv/samba/share' >> /etc/samba/smb.conf && \
    echo 'read only = no' >> /etc/samba/smb.conf && \
    echo 'guest ok = yes' >> /etc/samba/smb.conf

# Crée le répertoire partagé
RUN mkdir -p /srv/samba/share && \
    chmod -R 777 /srv/samba/share && \
    echo 'Ceci est un serveur SMBv1 vulnérable.' > /srv/samba/share/README.txt

# Ajout d'un utilisateur pour le serveur Samba
RUN useradd -M -s /sbin/nologin smbuser && \
    (echo "smbuser:smbpassword" | chpasswd) && \
    smbpasswd -a smbuser -n && \
    smbpasswd -e smbuser
# Configurer le superviseur
COPY supervisord.conf /etc/supervisor/conf.d/supervisord.conf

# Répertoires de travail
WORKDIR /scanner


EXPOSE 80 21 22 3306 445

# Commande par défaut
CMD ["/usr/bin/supervisord"]
