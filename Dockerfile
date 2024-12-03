FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y \
    apache2 \            # Apache web server
    openssh-server \     # SSH server
    mysql-server \       # MySQL server
    vsftpd \             # FTP server
    && apt-get clean     # Clean up unnecessary package files


    RUN mkdir /var/run/sshd
RUN echo 'root:root' | chpasswd   

RUN echo "ServerName localhost" >> /etc/apache2/apache2.conf

WORKDIR /scanner

EXPOSE 80
EXPOSE 21
EXPOSE 22
EXPOSE 3306

CMD service mysql start && \
    service vsftpd start && \
    service apache2 start && \
    /usr/sbin/sshd -D  # Keep SSH running in the foreground
