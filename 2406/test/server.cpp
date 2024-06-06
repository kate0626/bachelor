#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <random>
#include <networkit/graph/Graph.hpp>
#include <networkit/generators/StochasticBlockmodelGenerator.hpp>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int getNextNode(NetworKit::Graph& graph, int currentNode, double alpha) {
    std::vector<int> neighbors;
    graph.forNeighborsOf(currentNode, [&](int neighbor) {
        neighbors.push_back(neighbor);
    });
    if (neighbors.empty()) return -1;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    if (dis(gen) < alpha) {
        std::uniform_int_distribution<> neighborDis(0, neighbors.size() - 1);
        return neighbors[neighborDis(gen)];
    } else {
        return -1;  // 終了
    }
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    if (argc < 3) {
        std::cerr << "ERROR, no port provided" << std::endl;
        exit(1);
    }

    double alpha = 0.85; // 移動確率
    if (argc >= 4) {
        alpha = atof(argv[3]);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        error("ERROR on accept");
    }

    // SBMのグラフ生成
    std::vector<count> clusterSizes = {50, 50};
    NetworKit::StochasticBlockmodelGenerator generator(clusterSizes, 0.1, 0.01);
    NetworKit::Graph graph = generator.generate();

    int currentNode;
    n = read(newsockfd, &currentNode, sizeof(currentNode));
    if (n < 0) {
        error("ERROR reading from socket");
    }

    while (true) {
        currentNode = getNextNode(graph, currentNode, alpha);
        if (currentNode == -1) break;
        n = write(newsockfd, &currentNode, sizeof(currentNode));
        if (n < 0) {
            error("ERROR writing to socket");
        }
        n = read(newsockfd, &currentNode, sizeof(currentNode));
        if (n < 0) {
            error("ERROR reading from socket");
        }
    }

    close(newsockfd);
    close(sockfd);
    return 0;
}
