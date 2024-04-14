#include "../typedefs.hpp"

//using namespace Eigen;

static float stats_engine[6][2];
double in[ARMS];
double fb[ARMS];
double err[ARMS];
double out[ARMS];
double K[ARMS];


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
// Defines a vector for each, containing the values provided for G and tau.
std::vector<double> G_values = {57.3, 57.3, 57.3, 57.3, 57.3, 57.3};
std::vector<double> tau_values = {0.521, 0.772, 0.505, 0.834, 0.639, 0.742};

double f(double Kc, double Trc, double G, double tau) {
    if (Kc * G <= 1) {
        return std::numeric_limits<double>::max(); // Return a large value if out of domain
    }
    double acos_value = std::acos(-1 / (Kc * G));
    return ((tau * acos_value) / std::sqrt(((Kc * G) * (Kc * G)) - 1)) - Trc;
}

// Bisection method to find the root of f(Kc) = 0
double bissection_method(double Trc, double G, double tau, double lower_bound, double upper_bound, double tolerance) {
    double lower = lower_bound;
    double upper = upper_bound;
    double midpoint;
    double f_midpoint;
    double f_lower = f(lower, Trc, G, tau);

    while ((upper - lower) > tolerance) {
        midpoint = (upper + lower) / 2;
        f_midpoint = f(midpoint, Trc, G, tau);

        if (f_midpoint * f_lower > 0) { // f_midpoint and f_lower have the same sign
            lower = midpoint;
            f_lower = f_midpoint;
        } else {
            upper = midpoint;
        }
    }

    return (upper + lower) / 2; // The root lies between upper and lower
}


void receive_packages(msg_t &msg, struct sockaddr_in &sockAddr_Recv, int *res2,int *res,struct sockaddr_in &sockAddr)
{
    struct timeval start;       
    struct timeval end;  
    unsigned long long delta;
    double Kc;
    
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

        double Trc = delta / 1000.0; // Converting to seconds
        if (delta > 1000)
        {
            //for (size_t i = 0; i < G_values.size(); ++i) {
                double Kc = bissection_method(Trc, G_values[ENGINE], tau_values[ENGINE], K1 / G_values[ENGINE], K2, 1e-6);
                //double Kc = bissection_method(Trc, G_values[1], tau_values[1], 1.01 / G_values[1], 100, 1e-6);
                //double Kc_values = Kc;
                cout << "No Controlable, delta = " << delta << endl;
            //}
        }
        else
        {
            //for (size_t i = 0; i < G_values.size(); ++i) {
                double Kc = bissection_method(Trc, G_values[ENGINE], tau_values[ENGINE], K1 / G_values[ENGINE], K2, 1e-6);
                //double Kc = bissection_method(Trc, G_values[1], tau_values[1], 1.01 / G_values[1], 100, 1e-6);
                //double Kc_values = Kc;
                cout << "Controlable, delta = " << delta << endl;
            //}
        }

        // Kc_values now contains Kc values for each arm
        //for (int j=0; j<ARMS;j++) {
            K[ENGINE] = Kc;
        //}
        
        // Update Kc gain for each arm.
        //std::vector<double> Kc_values;
        //for (size_t i = 0; i < G_values.size(); ++i) {
            //double Kc = bissection_method(Trc, G_values[ENGINE], tau_values[ENGINE], 0 / G_values[ENGINE], K1, 1e-6);
            //double Kc_values = Kc;
        //}

        // Kc_values now contains Kc values for each arm
        //for (int j=0; j<ARMS;j++) {
            //K[ENGINE] = Kc;
        //}


        /* Generate input signal and retrieve feedback data. */

        //for (int j = 0; j < ARMS; j++)
        //{
            in[ENGINE] = 1 * (msg.time % 10000 > msg.time % 5000) - 0.5;
            fb[ENGINE] = msg.joints[ENGINE];
        //}

        /* Calculate controller output (speed) for each channel. */
        //for(int j = 0 ; j < ARMS ;j++ ){
            err[ENGINE] = in[ENGINE] - fb[ENGINE];
            out[ENGINE] = err[ENGINE] * K[ENGINE];
        //}
        /* Put output in values in the message struct. */

        //for (int j = 0; j < ARMS; j++)
        //{
            msg.joints[ENGINE] = out[ENGINE];
        //}

        
        /* Print values useful for debugging. */

        //cout << "time" << 
        printf("time: %6.4lf\n", msg.time / 1000.0);
        printf("delay: %6.4lf\n", delta / 1000.0);
        printf("in[%d]: %6.4lf\n", ENGINE, in[ENGINE]);
        printf("fb[%d]: %6.4lf\n", ENGINE, fb[ENGINE]);
        printf("err[%d]: %6.4lf\n",ENGINE, err[ENGINE]);
        printf("out[%d]: %6.4lf\n",ENGINE, out[ENGINE]);

        /* Update message time and send message. */
        msg.pos = M_PI/DIVISION_POS;

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

    //msg.pos = 1.2;
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
