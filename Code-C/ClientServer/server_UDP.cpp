#include "../typedefs.hpp"

void rcv_l_send_r(msg_t msg_recv, struct sockaddr_in sockAddr_Recv, int retart_to_loss, struct sockaddr_in sockAddr, int server_to_robot)
{
  while (true)
  {
    recvfrom(retart_to_loss, &msg_recv, sizeof(msg_recv), 0, (struct sockaddr *)&sockAddr_Recv, (socklen_t *)&sockAddr_Recv);
    usleep(DELAY_MS_SERVER * 500);
    sendto(server_to_robot, &msg_recv, sizeof(msg_recv), 0, (struct sockaddr *)&sockAddr, sizeof(sockAddr));
  }
}

void rcv_r_send_l(msg_t msg_recv, struct sockaddr_in sockAddr_Recv, int robot_to_server, struct sockaddr_in sockAddr, int server_to_retard)
{
  while (true)
  {
    recvfrom(robot_to_server, &msg_recv, sizeof(msg_recv), 0, (struct sockaddr *)&sockAddr_Recv, (socklen_t *)&sockAddr_Recv);
    usleep(DELAY_MS_SERVER * 500);
    sendto(server_to_retard, &msg_recv, sizeof(msg_recv), 0, (struct sockaddr *)&sockAddr, sizeof(sockAddr));
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

void create_socket_recv(struct sockaddr_in *sock_received, int *loss_to_server, int PORT)
{
  *loss_to_server = socket(NET, SOCK, PROTOCOL);
  if (*loss_to_server < 0)
  {
    cout << "Error to bind the client receive port " << PORT << "!!" << endl;
    exit(1);
  }

  memset(sock_received, 0, sizeof(sock_received));
  sock_received->sin_family = NET;
  sock_received->sin_port = htons(PORT);
  sock_received->sin_addr.s_addr = htonl(INADDR_ANY);

  int err = bind(*loss_to_server, (struct sockaddr *)sock_received, sizeof(*sock_received));
  if (err == ERROR)
  {
    cout << "Error to bind the client receive port " << PORT << "!!" << endl;
  }
}

int main(int nba, char *arg[])
{
  /*
    Definition of Variables
  */
  msg_t msg;
  struct sockaddr_in sock_received, sock_send;
  int loss_to_server, server_to_client2;

  // Below are the variables to receive the values of client2
  msg_t msg_of_robot;
  int client2_to_server = 0;
  struct sockaddr_in sock_of_robot;

  // Below are the variables to send the values to retard
  struct sockaddr_in sock_to_retard;
  int server_to_retard;

  /*
    Create of the socket to send and receive packages
  */
  create_socket_recv(&sock_received, &loss_to_server, RETARD_RS_PORT); // to receive of the retard
  create_socket_send(&sock_send, &server_to_client2, ROBOT_SC2_PORT);  // to send to the robot

  create_socket_send(&sock_to_retard, &server_to_retard, SERVER_TO_RETARD); // to send for the retard
  create_socket_recv(&sock_of_robot, &client2_to_server, ROBOT_TO_SERVER);  // to receive of the robot
  /*
    Creation of threads of functions
  */

  thread rcv_of_loss_send_to_robot(rcv_l_send_r, ref(msg), ref(sock_received), ref(loss_to_server), ref(sock_send), ref(server_to_client2));
  thread rcv_of_robot_send_to_loss(rcv_r_send_l, ref(msg_of_robot), ref(sock_of_robot), ref(client2_to_server), ref(sock_to_retard), ref(server_to_retard));

  /*
    Calling the threads to start the send and receive mensages
  */
  if (rcv_of_loss_send_to_robot.joinable() && rcv_of_robot_send_to_loss.joinable())
  {
    rcv_of_loss_send_to_robot.join();
    rcv_of_robot_send_to_loss.join();
  }
  else
  {
    cout << "Some error with Threads, the program not will execute" << endl;
  }

  close(loss_to_server);
  close(server_to_client2);
  return 0;
}
