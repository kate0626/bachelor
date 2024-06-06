#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (argc < 4)
    {
        std::cerr << "usage " << argv[0] << " hostname port alpha" << std::endl;
        exit(0);
    }

    portno = atoi(argv[2]);
    double alpha = atof(argv[3]); // 移動確率

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }

    server = gethostbyname(argv[1]);
    if (server == nullptr)
    {
        std::cerr << "ERROR, no such host" << std::endl;
        exit(0);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR connecting");
    }

    int currentNode = 0; // スタートノードを指定
    n = write(sockfd, &currentNode, sizeof(currentNode));
    if (n < 0)
    {
        error("ERROR writing to socket");
    }

    while (true)
    {
        n = read(sockfd, &currentNode, sizeof(currentNode));
        if (n < 0)
        {
            error("ERROR reading from socket");
        }
        if (currentNode == -1)
            break; // 終了
        std::cout << "Current node: " << currentNode << std::endl;
        n = write(sockfd, &currentNode, sizeof(currentNode));
        if (n < 0)
        {
            error("ERROR writing to socket");
        }
    }

    close(sockfd);
    return 0;
}
