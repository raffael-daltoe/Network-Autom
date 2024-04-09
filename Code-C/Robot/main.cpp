#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "../typedefs.hpp"
using namespace std;

extern "C"
{
    #include "extApi.h"
}

int handles[6], all_ok = 1;
simxInt handle, error;


double vrep_get(int joint,int clientID)
{
    simxFloat value;

    simxGetJointPosition(clientID, handles[joint], &value,
        simx_opmode_oneshot);

    return (double) value;
}

void GetHandles(int clientID)
{
    simxChar objectName[100];
    char str[10];
    for (int i = 0; i < 6; i++)
    {
        strcpy(objectName, "joint");
        sprintf(str, "%d", i + 1);
        strcat(objectName, str);
        error = simxGetObjectHandle(clientID, objectName, &handle, simx_opmode_oneshot_wait);
        if (error == simx_return_ok)
            handles[i] = handle;
        else
        {
            printf("Error in Object Handle - joint number %d\n", i);
            all_ok = 0;
        }
    }
}

void create_socket_recv(struct sockaddr_in *sock_received, int *server_to_robot, int PORT)
{
    *server_to_robot = socket(NET, SOCK, PROTOCOL);
    if (*server_to_robot < 0)
    {
        cout << "Error to bind the ROBOT receive port " << PORT << "!!" << endl;
        exit(1);
    }

    memset(sock_received, 0, sizeof(sock_received));
    sock_received->sin_family = NET;
    sock_received->sin_port = htons(PORT);
    sock_received->sin_addr.s_addr = htonl(INADDR_ANY);

    int err = bind(*server_to_robot, (struct sockaddr *)sock_received, sizeof(*sock_received));
    if (err == ERROR)
    {
        cout << "Error to bind the ROBOT receive port " << PORT << "!!" << endl;
    }
}

void create_socket_send(struct sockaddr_in *sock_send, int *server_to_client2, int PORT)
{
    *server_to_client2 = socket(NET, SOCK, PROTOCOL);
    if (*server_to_client2 < 0)
    {
        cout << "Cannot open client send port " << PORT << " socket " << endl;
        exit(1);
    }

    sock_send->sin_family = NET;
    sock_send->sin_port = htons(PORT);
    sock_send->sin_addr.s_addr = inet_addr("127.0.0.1");
}

void sending_packages(msg_t &msg, int server, struct sockaddr_in sockAddr)
{
    sendto(server, &msg, sizeof(msg), 0, (struct sockaddr *)&sockAddr, sizeof(sockAddr));
}

void receive_packages(msg_t &msg_recv, struct sockaddr_in &sockAddr_Recv, int *res2)
{
    recvfrom(*res2, &msg_recv, sizeof(msg_recv), 0, (struct sockaddr *)&sockAddr_Recv, (socklen_t *)&sockAddr_Recv);
}

double vrep_time(int clientID)
{
    return (int) simxGetLastCmdTime(clientID);
}

void takeValuesOfCoppelia(msg_t *msg, int ClientID)
{
    for(int j = 0; j < ARMS; j++) { 
        msg->joints[j] = vrep_get(j,ClientID); 
    }
    msg->time = vrep_time(ClientID);
    gettimeofday(&msg->sys_time, NULL);
}

void vrep_set(int joint, double value,int clientID)
{
    simxSetJointTargetVelocity(clientID, handles[joint], (simxFloat) value,
        simx_opmode_oneshot);
}

void SetValuesArm(msg_t msg,int clientID)
{
    for(int i = 0; i < ARMS; i++) { 
        vrep_set(i, msg.joints[i],clientID);
    }
}

void receiveSetVelocity(int clientID, struct sockaddr_in sock_received, int server_to_robot,
                      struct sockaddr_in sock_send, int robot_to_server)
{
    msg_t *msg = new msg_t();
    msg->id=0;
    takeValuesOfCoppelia(msg,clientID);
    while(true){
        sending_packages(*msg, robot_to_server, ref(sock_send));
        receive_packages(*msg, sock_received, &server_to_robot);
        GetHandles(clientID);
        SetValuesArm(*msg,clientID);
        takeValuesOfCoppelia(msg,clientID);
        for(int i = 0; i < ARMS; i++) {
            simxSetObjectIntParameter(clientID, handles[i], 2001, 0,
            simx_opmode_oneshot);
        }
        usleep(TIME_WAITING);
    }
}

int main()
{
    struct sockaddr_in sock_received, sock_send;
    int server_to_robot = 0, robot_to_server = 0;
    
    /*
        Connection to the robot
    */
    int clientID = simxStart((simxChar *)"127.0.0.1", MAIN_TO_ROBOT, true, true, TIMEOUT, ATTEMPTS);

    /*
        Conection with the server
    */
    create_socket_recv(&sock_received, &server_to_robot, ROBOT_SC2_PORT);
    create_socket_send(&sock_send, &robot_to_server, ROBOT_TO_SERVER);
    /*
        Giving commands to the robot
    */
    if (clientID != -1)
    {
        simxSynchronous(clientID, true); // Enable the synchronous mode (Blocking function call)
        simxStartSimulation(clientID, simx_opmode_oneshot);

        thread setVel(receiveSetVelocity, clientID, sock_received, server_to_robot, sock_send, 
                         robot_to_server);
        if (setVel.joinable())
        {
            setVel.join();
        }
        else
        {
            cout << "Some error with Threads, the program not will execute" << endl;
        }
        simxStopSimulation(clientID, simx_opmode_oneshot);
        // Close the connection to the server
        simxFinish(clientID);
    }
    else
        printf("Connection to the server not possible\n");

    return (0);
}
