#include "../typedefs.hpp"

using namespace Eigen;

static float stats_engine[6][2];
ArrayXd in(ARMS);
ArrayXd fb(ARMS);
ArrayXd err(ARMS);
ArrayXd out(ARMS);
ArrayXd K(ARMS);

void print_msg(msg_t &msg)
{
    printf("msg_t:\n");
    printf("\tid: %d\n", (int) msg.id);
    printf("\ttime: %d\n", msg.time);
    for(int i = 0; i < ARMS; i++) {
        printf("\t[%d]: %6.4lf\n", i, msg.joints[i]);
    }
    printf("\n");
}

void receive_packages(msg_t &msg, struct sockaddr_in &sockAddr_Recv, int *res2,int *res,struct sockaddr_in &sockAddr)
{
    struct timeval start;       
    struct timeval end;  
    unsigned long long delta;   

    while (true)
    {

        recvfrom(*res2, &msg, sizeof(msg), 0, (struct sockaddr *)&sockAddr_Recv, (socklen_t *)&sockAddr_Recv);
        msg.id++;
        print_msg(msg);

        /* Measure time delay. */

        start = msg.sys_time;
        gettimeofday(&end, NULL);
        delta = (end.tv_sec - start.tv_sec) * 1000 +
                (end.tv_usec - start.tv_usec) / 1000;

        if (delta <= 100)
        {
            K[1] = 2.00;
            printf("delta <= 100\n");
        }
        else if (delta <= 200)
        {
            K[1] = 1.00;
            printf("delta <= 200\n");
        }
        else if (delta <= 400)
        {
            K[1] = 0.50;
            printf("delta <= 400\n");
        }
        else if (delta <= 1000)
        {
            K[1] = 0.25;
            printf("delta <= 1000\n");
        }
        else
        {
            K[1] = 1.21;
            printf("non controlable \n");
        }

        /* Generate input singal and retrive feedback data. */

        for (int j = 0; j < ARMS; j++)
        {
            in[j] = 1 * (msg.time % 10000 > msg.time % 5000) - 0.5;
            fb[j] = msg.joints[j];
        }

        /* Calculate controller output (speed) for each channel. */

        err = in - fb; // error
        out = err * K;

        /* Put output in values in the message struct. */

        for (int j = 0; j < ARMS; j++)
        {
            msg.joints[j] = out[j];
        }

        /* Print values useful for debugging. */

        printf("time: %6.4lf\n", msg.time / 1000.0);
        printf("retard: %6.4lf\n", delta / 1000.0);
        printf("in[1]: %6.4lf\n", in[1]);
        printf("fb[1]: %6.4lf\n", fb[1]);
        printf("err[1]: %6.4lf\n", err[1]);
        printf("out[1]: %6.4lf\n", out[1]);

        /* Update message time and send message. */

        msg.time += PERIOD;
        sendto(*res, &msg, sizeof(msg), 0, (struct sockaddr *)&sockAddr, sizeof(sockAddr));
    }
}

void create_socket_send(struct sockaddr_in *sockAddr, int *retard, int PORT)
{
    *retard = socket(NET, SOCK, PROTOCOL);
    if (*retard < 0)
    {
        cout << "Cannot open client send port " << PORT << " socket " << endl;
        exit(1);
    }

    sockAddr->sin_family = NET;
    sockAddr->sin_port = htons(PORT);
    sockAddr->sin_addr.s_addr = inet_addr("127.0.0.1");
}

void create_socket_recv(struct sockaddr_in *sockAddr_Recv, int *res2, int PORT)
{
    *res2 = socket(NET, SOCK, PROTOCOL);
    if (*res2 < 0)
    {
        cout << "Cannot open client receive port " << PORT << " socket" << endl;
        exit(1);
    }

    memset(sockAddr_Recv, 0, sizeof(sockAddr_Recv));
    sockAddr_Recv->sin_family = NET;
    sockAddr_Recv->sin_port = htons(PORT);
    sockAddr_Recv->sin_addr.s_addr = htonl(INADDR_ANY);

    int err = bind(*res2, (struct sockaddr *)sockAddr_Recv, sizeof(*sockAddr_Recv));
    if (err == ERROR)
    {
        cout << "Error to bind the client receive port " << PORT << "!!" << endl;
    }
}

int main()
{
    /*
        Definition of Variables
    */
   
    msg_t msg;

    struct sockaddr_in sockAddr;
    int retard = 0;

    int res2 = 0;
    struct sockaddr_in sockAddr_Recv;

    /*
        Create of the socket to send and receive packages
    */
    create_socket_send(&sockAddr, &retard, RETARD_CR_PORT);
    create_socket_recv(&sockAddr_Recv, &res2, RETARTD_RC_PORT);

    for(int j = 0; j < ARMS; j++) {
        K[j] = 0; 
    }
    /*
        Creation of threads of functions
    */
    thread receive(receive_packages, ref(msg), ref(sockAddr_Recv), &res2, &retard, ref(sockAddr));

    /*
        Calling the threads to start the send and receive mensages
    */
    if (receive.joinable())
    {
        receive.join();
    }
    else
    {
        cout << "Some error with Thread, the program not will execute" << endl;
    }

    close(retard);
    return 0;
}
