#include "../typedefs.hpp"

void receive_and_send_packages_to_server(msg_t msg_recv, struct sockaddr_in sockAddr_Recv, int retard_to_server, struct sockaddr_in sockAddr,int retard_of_client){
    // bool flag = true;
    while(true){
        recvfrom(retard_to_server, &msg_recv, sizeof(msg_recv), 0, (struct sockaddr *)&sockAddr_Recv, (socklen_t *)&sockAddr_Recv);
        
        // float randomValue = ((float)rand() / RAND_MAX);
        // if (randomValue < PACKET_LOSS_RATE && flag == false)
        // {
        //     cout << "Dropping packet to server" << endl;
        //     continue; // Drop the packet
        // }
        // flag = false;
        sendto(retard_of_client, &msg_recv, sizeof(msg_recv), 0, (struct sockaddr *)&sockAddr, sizeof(sockAddr));
        
        // Simulate delay
        usleep(DELAY_MS_LOSSPACKAGE * 500);
    }
}

void receive_and_send_packages_to_client(msg_t msg_recv, struct sockaddr_in sockAddr_Recv, int retard_to_client, struct sockaddr_in sockAddr,int retard_of_server){
    // bool flag = true;
    while(true){
        recvfrom(retard_to_client, &msg_recv, sizeof(msg_recv), 0, (struct sockaddr *)&sockAddr_Recv, (socklen_t *)&sockAddr_Recv);

        // float randomValue = ((float)rand() / RAND_MAX);
        // if (randomValue < PACKET_LOSS_RATE && flag == false)
        // {
        //     cout << "Dropping packet to client" << endl;
        //     continue; // Drop the packet
        // }
        sendto(retard_of_server, &msg_recv, sizeof(msg_recv), 0, (struct sockaddr *)&sockAddr, sizeof(sockAddr));
        // flag = false;
        // Simulate delay
        usleep(DELAY_MS_LOSSPACKAGE * 500);
    }
}

void create_socket_send(struct sockaddr_in *sockAddr, int *retard_of_client,int PORT){
    *retard_of_client = socket(NET, SOCK, PROTOCOL);
    if (*retard_of_client < 0){
        cout <<"Cannot open client send port " << PORT << " socket" << endl;
        exit(1);
    }
    
    sockAddr->sin_family = NET;
    sockAddr->sin_port = htons(PORT);
    sockAddr->sin_addr.s_addr = inet_addr("127.0.0.1");
}

void create_socket_recv(struct sockaddr_in *sockAddr_Recv, int *retard_to_server,int PORT){

    *retard_to_server = socket(NET, SOCK, PROTOCOL);
    if (*retard_to_server < 0){
        cout << "Cannot open client receive port " << PORT <<" socket" << endl;
        exit(1);
    }

    memset(sockAddr_Recv, 0, sizeof(sockAddr_Recv));
    sockAddr_Recv->sin_family = NET;
    sockAddr_Recv->sin_port = htons(PORT);
    sockAddr_Recv->sin_addr.s_addr = htonl(INADDR_ANY);

    int err = bind(*retard_to_server, (struct sockaddr *)sockAddr_Recv, sizeof(*sockAddr_Recv));
    if (err == ERROR){
        cout << "Error to bind the client receive port " << PORT << "!!" << endl;
    }
}

int main()
{
    /*
        Definition of Variables
    */
    msg_t msg_recv;
    struct sockaddr_in sockAddr;
    int retard_of_client = 0;

    int retard_to_server =0;
    struct sockaddr_in sockAddr_Recv;
    
    // Below is the variables to receive the values of server
    msg_t msg_from_server;
    int retard_to_client =0;
    struct sockaddr_in sock_to_client1;

    struct sockaddr_in sock_of_server;
    int server_to_retard;

    /*
        Create of the socket to send and receive packages
    */
    create_socket_send(&sockAddr, &retard_of_client,SERVER_PORT);            // to send for the server
    create_socket_recv(&sockAddr_Recv, &retard_to_server,CLIENT_PORT);       // to receive of the client

    create_socket_send(&sock_to_client1,&retard_to_client,RETARTD_RC_PORT);  // to send for the client
    create_socket_recv(&sock_of_server, &server_to_retard,SERVER_TO_RETARD); // to receive of the server
    /*
        Creation of threads of functions
    */
    thread rcv_and_send(receive_and_send_packages_to_server, ref(msg_recv), ref(sockAddr_Recv),ref(retard_to_server), ref(sockAddr),ref(retard_of_client));
    thread send_to_client1(receive_and_send_packages_to_client, ref(msg_from_server), ref(sock_of_server),ref(server_to_retard), ref(sock_to_client1),ref(retard_to_client));

    /*
        Calling the threads to start the send and receive mensages
    */
    if (rcv_and_send.joinable() && send_to_client1.joinable())
    {
        rcv_and_send.join();
        send_to_client1.join();
    }
    else{
        cout<<"Some error with Threads, the program not will execute" << endl;
    }


    close(retard_to_server);
    close(retard_of_client);
    return 0;
}
