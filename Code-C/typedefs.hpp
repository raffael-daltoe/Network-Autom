#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <chrono>
#include <semaphore>
#include <cmath>
#include <vector>
#include <Eigen/Dense>      
#include <Eigen/Core>
#include <inttypes.h>

#include <list>
#include <semaphore>
#include <thread>
#include <atomic>

#define ERROR (-1)
// --------------------------------------------------------
// server.cpp
#define ARMS 6
#define RETARD_RS_PORT 2000  // Retard to Server
#define ROBOT_TO_SERVER 2005 // Port of robot to server
#define DELAY_MS_SERVER 400

// --------------------------------------------------------
#define RETARD_CR_PORT 2002 // Client to Retard
//#define Te 200000
#define RETARTD_RC_PORT 2003
#define PERIOD (10)
// --------------------------------------------------------
// losspackage.cpp

#define NET PF_INET
#define SOCK SOCK_DGRAM
#define PROTOCOL IPPROTO_UDP

#define CLIENT_PORT 2002      // Port the client sends to
#define SERVER_PORT 2000      // Port the server listens on
#define SERVER_TO_RETARD 2004 // Port of server to retard sending back the package
//#define PACKET_LOSS_RATE 20  // 10% packet loss
#define DELAY_MS_LOSSPACKAGE       300    // 100ms delay

// --------------------------------------------------------

// main.cpp robot
#define ROBOT_SC2_PORT 2001 // port where the robot will receive the packages
#define MAIN_TO_ROBOT 5555  // the port number where to connect with the robot
#define TIMEOUT 5000        // connection time-out in milliseconds (for the first connection)
#define ATTEMPTS 5          // how much attempts to do the connection
#define TIMESIMULATION 10
#define TIME_WAITING 0.99
// --------------------------------------------------------

#define LABEL 100
using namespace std;

typedef struct {
    uint_fast32_t id;
    int time;
    struct timeval sys_time;
    double joints[ARMS];
} msg_t;
