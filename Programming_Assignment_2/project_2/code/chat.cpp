
#include <stdio.h>
#include <ws2tcpip.h>
#include <winsock.h>

#define MAXDATASIZE 255

char Border[] = "#################################################";

//Gets the Address from sockaddr in network form
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) //IPv4 address
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr); //IPv6 address
}

//Gets the Address from sockaddr in network order
u_short get_in_port(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) //IPv4 port
    {
        return (((struct sockaddr_in*)sa)->sin_port);
    }

    return (((struct sockaddr_in6*)sa)->sin6_port); // IPv6 address
}

bool MainMenu(struct addrinfo *ServerInfo, fd_set *Master)
{
    printf("\nP.A._Chat>> ");

#if 1 //switch for debug check

    // get user input
    char Message[MAXDATASIZE];
    gets_s(Message);
    
#else /*[DEBUG]*/
    char Message[] = "connect 192.168.1.41 5000";
#endif

    
    char *word = "NULL";
    char *temp;
    if (strcmp(Message, "\0")) // check if the user enter an input
    {
        word = strtok_s(Message, " ", &temp);
    }

    char Border[] = "########################################";
    printf("%s\n\n", Border);

    // Goes throught the list of possible commands
    if(strcmp(word, "help") == 0) // list commands to user
    {
        printf("\tmyip - Show the IP of your machine.\n");
        printf("\tmyport - Show the port number that the process runs on.\n");
        
        printf("\tconnect <destination IP> <destination port> \n\tConnect this machine to another machine.\n");
        printf("\tlist - List all the connected peers.\n");

        printf("\tsend <connection id> <message> - Sends message to a peer.\n");
        printf("\tterminate - Terminate the connection.\n");

        printf("\texit - Terminate all the connections and exit the program.\n");
    }
    else if (strcmp(word, "myip") == 0) //show user's ip address
    {
     
        char ServerIPAddrStringBuffer[INET6_ADDRSTRLEN];

        //convert IP address into human readable data
        inet_ntop(((sockaddr_storage *)ServerInfo->ai_addr)->ss_family,
                  get_in_addr(ServerInfo->ai_addr),
                  ServerIPAddrStringBuffer,
                  sizeof(ServerIPAddrStringBuffer));
        
        printf("The IP address is %s\n", ServerIPAddrStringBuffer);

    }
    else if(strcmp(word, "myport") == 0) //show user's port number
    {
        // converts port number network byte order to host byte order
        printf("The Program runs on port number  %d\n",
               ntohs(get_in_port(ServerInfo->ai_addr)));

    }
    else if (strcmp(word, "connect") == 0) // connect to other server on the same network
    {
        char *ConnectingIpAddr = strtok_s(temp, " ", &temp);
        char *ConnectingPort = strtok_s(temp, " ", &temp);

        // error checking user input for ip address and port number
        if(ConnectingIpAddr != NULL && ConnectingPort != NULL)
        {
            char ServerIPAddrStringBuffer[INET6_ADDRSTRLEN];
            char ServerPort[5];
            
            //convert IP address into human readable data
            inet_ntop(((sockaddr_storage *)ServerInfo->ai_addr)->ss_family,
                      get_in_addr(ServerInfo->ai_addr),
                      ServerIPAddrStringBuffer,
                      sizeof(ServerIPAddrStringBuffer));

            
            _itoa_s(ntohs(get_in_port(ServerInfo->ai_addr)), ServerPort, 10);
                
            if(strcmp(ConnectingPort, ServerPort) == 0 && strcmp(ConnectingIpAddr, ServerIPAddrStringBuffer) == 0)
            {
                printf("You cannot add the IP Address and Port Number of the Server!!!\n");
                printf("\n%s\n", Border);
                return true;
            }
            
            struct addrinfo ClientHints = {};
            ZeroMemory(&ClientHints, sizeof(ClientHints));
            ClientHints.ai_family = AF_INET;
            ClientHints.ai_socktype = SOCK_STREAM;

            // Resolve the client address and port number information
            struct addrinfo *ClientInfo = NULL;
            int ClientInfoResult = getaddrinfo(ConnectingIpAddr,
                                               ConnectingPort,
                                               &ClientHints,
                                               &ClientInfo);

            if (ClientInfoResult != 0)
            {
                printf("Incorrect Information.\n");
            }
            else
            {


                SOCKET ClientSocket = INVALID_SOCKET;
                struct addrinfo *CurrentClientAI = NULL;
                {
                    for(CurrentClientAI = ClientInfo;
                        CurrentClientAI != NULL;
                        CurrentClientAI->ai_next)
                    {
                        ClientSocket = socket(CurrentClientAI->ai_family,
                                              CurrentClientAI->ai_socktype,
                                              CurrentClientAI->ai_protocol);

                        if(ClientSocket == INVALID_SOCKET)
                        {
                            printf("Unable to create socket:\n");
                            break;
                        }
                        break;
                    }
                }

                printf("...Connecting...\n"); //trying to find a connection
                if(connect(ClientSocket, CurrentClientAI->ai_addr,
                           (int)CurrentClientAI->ai_addrlen) == SOCKET_ERROR)
                {//failed to connect
                    closesocket(ClientSocket);

                    printf("Unable to connect to %s.\n", ConnectingIpAddr);
                    
                }            
                else
                {//able to connect to the server
                    //add the other server socket number to the list of socket on this server
                    FD_SET(ClientSocket, Master);
                    printf("The connection to peer %s is successfully established.\n",
                           ConnectingIpAddr);
                }            
            }
                                    
            freeaddrinfo(ClientInfo);
        }
        else
        {// user did not provide enough information
            printf("Did not input Ip Address or Port\n");
        }
    }
    else if(strcmp(word, "list") == 0) // checks this server connected
    {
        //setting up peers address information
        struct sockaddr_storage PeerSockAddr;
        socklen_t PeerSockAddrLen = sizeof(PeerSockAddr);
        char PeerIPAddrStringBuffer[INET6_ADDRSTRLEN];
        u_short PeerPort;

        printf("<ID>:\t <Ip Address>\t <Port>\n");

        //going throught the list of connection
        for(u_int i = 1; i < Master->fd_count; i++)
        {
            //getting peers address information
            getpeername(Master->fd_array[i],
                        (struct sockaddr *)&PeerSockAddr,
                        &PeerSockAddrLen);

            //decoding ip address
            inet_ntop(PeerSockAddr.ss_family,
                      get_in_addr((struct sockaddr*)&PeerSockAddr),
                      PeerIPAddrStringBuffer,
                      sizeof(PeerIPAddrStringBuffer));

            //decoding port number
            PeerPort = ntohs(get_in_port((sockaddr *)&PeerSockAddr));

            
            printf("%d:\t %s\t %d \n", i, PeerIPAddrStringBuffer, PeerPort);
        }
    }
    else if (strcmp(word, "send") == 0) // send message to peer
    {
        //getting user token
        char *ClientIDToken = strtok_s(temp, " ", &temp);
        char *Message = temp;

        
        if(ClientIDToken != '\0' && Message != '\0') //checking for user input
        {
            int length = (int)(strlen(ClientIDToken));
            bool isNumber = true;
            for(int i = 0; i < length; i++) //checking for user id input is a number
            {
                if(!isdigit(ClientIDToken[i]))
                {// user input is not a number
                    isNumber = false;
                    break;
                }
            }
                
            if(isNumber)
            {// user input is a number 
                unsigned int ClientID = atoi(ClientIDToken);

                //check if user input id is a valid socket index number 
                if(ClientID > 0 && ClientID < Master->fd_count)
                {
                    send(Master->fd_array[ClientID], Message, (int)strlen(Message), 0);
                    printf("Message Has Been Sent.\n");

                }
                else
                {
                    printf("Client ID is not on the list.\n");
                }
                    
            }
            else
            {// user input is not a number
                printf("Invalid Input.\n");
            }

        }
        else
        {//user did not provide enough information
            printf("Did not input Client ID or Message\n");
        }
            
    }
    else if (strcmp(word, "terminate") == 0) // close connection with server
    {
        char *ClientIDToken = strtok_s(temp, " ", &temp);

        if(ClientIDToken != '\0') //check if the user token is valid
        {
            int length = (int)(strlen(ClientIDToken));
            bool isNumber = true;

            //check if user input is a number
            for(int i = 0; i < length; i++)
            {
                if(!isdigit(ClientIDToken[i]))
                {
                    isNumber = false;
                    break;
                }
            }

            
            if(isNumber)
            {
                unsigned int ClientID = atoi(ClientIDToken);

                //check if user id is a valid socket index
                if(ClientID > 0 && ClientID < Master->fd_count)
                {
                    //setting up peers address information
                    struct sockaddr_storage PeerSockAddr;
                    socklen_t PeerSockAddrLen = sizeof(PeerSockAddr);
                    char PeerIPAddrStringBuffer[INET6_ADDRSTRLEN];
                    
                    //getting peers address information
                    getpeername(Master->fd_array[ClientID],
                                (struct sockaddr *)&PeerSockAddr,
                                &PeerSockAddrLen);

                    //decoding ip address
                    inet_ntop(PeerSockAddr.ss_family,
                              get_in_addr((struct sockaddr*)&PeerSockAddr),
                              PeerIPAddrStringBuffer,
                              sizeof(PeerIPAddrStringBuffer));

                    //remove a peer from the list a socket on this server
                    closesocket(Master->fd_array[ClientID]);
                    FD_CLR(Master->fd_array[ClientID], Master);
                    
                    printf("You have terminated %s\n", PeerIPAddrStringBuffer);
                }
                else
                {//invalid user id
                    printf("Client ID is not on the list.\n");
                }

            }
            else
            {// invalid input
                printf("Invalid Input.\n");
            }

        }
        else
        {//not enough user information
            printf("Did not input Client ID\n");
        }            
    }
    else if (strcmp(word, "exit") == 0) //exit this program
    {
        // disconnect every peer in the list of socket for this server
        for(u_int i = 1; i < Master->fd_count; i++)
        {
            closesocket(Master->fd_array[i]);
            FD_CLR(Master->fd_array[i], Master);
        }
        printf("Terminated  all connection.\n");
        printf("\n%s\n", Border);
        return false;
    }
    else
    {//no commands were inputed
        printf("Did not enter program command. \nInput [help] for list of commands\n");
    }
    printf("\n%s\n", Border);
    return true;
    
}

int main(int argc, char **argv)
{

    //Windows startup network
    WSADATA wsaData;   // if this doesn't work
    //WSAData wsaData; // then try this instead

    // MAKEWORD(1,1) for Winsock 1.1, MAKEWORD(2,0) for Winsock 2.0:

    if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        exit(1);
    }

    // getting host name for getaddrinfo() later
    char HostName[MAXDATASIZE];
    hostent *Host = NULL;
    gethostname(HostName, sizeof(HostName));

    //setting up socket list for this server 
    fd_set Master; //master file descriptor list
    fd_set Read_FDS;  // temp file descriptor list for select()
    int MaxFD;        // maximum file descriptor number

    //clearing socket list
    FD_ZERO(&Master);
    FD_ZERO(&Read_FDS);

    //setting up server adddress information 
    struct addrinfo ServerHints = {};
    ZeroMemory(&ServerHints, sizeof(ServerHints));
    //ServerHints.ai_family = AF_UNSPEC;
    ServerHints.ai_family = AF_INET;
    ServerHints.ai_socktype = SOCK_STREAM;
    ServerHints.ai_flags = AI_PASSIVE;

// Resolve the server address and port
    struct addrinfo *ServerInfo = NULL; 
    int ServerInfoResult = getaddrinfo(HostName, argv[1], &ServerHints, &ServerInfo);
    if (ServerInfoResult != 0) {
        printf("getaddrinfo failed: %d\n", ServerInfoResult);
        WSACleanup();
        return 1;
    }

    //creating this computer's server socket
    SOCKET ServerSocket = INVALID_SOCKET;
    struct addrinfo *CurrentServerAI = NULL;
    char optVal = 1;
    {
        for(CurrentServerAI = ServerInfo; CurrentServerAI != NULL; CurrentServerAI->ai_next)
        {
            // Create a SOCKET for connecting to server(Connection Endpoint)
            ServerSocket = socket(CurrentServerAI->ai_family, CurrentServerAI->ai_socktype, CurrentServerAI->ai_protocol);
            if (ServerSocket == INVALID_SOCKET) {
                printf("Error at socket(): %ld\n", WSAGetLastError());
                continue;
            }

            if(setsockopt(ServerSocket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(char)) == SOCKET_ERROR)
            {
                printf("setsocket encounter an error: %d\n", WSAGetLastError());
                return 1;
            }
        
            // binds (Attaching to an IP and Port) "Assign a unique telephone number"
            int BindReturn = bind(ServerSocket, CurrentServerAI->ai_addr, (int)CurrentServerAI->ai_addrlen);
            if(BindReturn == SOCKET_ERROR)
            {
                printf("bind failed with error %ld\n", WSAGetLastError());
                closesocket(ServerSocket);
                continue;
            }
            break;
        }        
    }
    
    if(CurrentServerAI == NULL)
    {
        printf("[ERROR] Server: Failed to bind\n");
        return 1;
    }

    // set the Server socket to listern to for any incoming connection
    if(listen(ServerSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("[ERROR] Failed at listen: %d\n", WSAGetLastError());
        closesocket(ServerSocket);
        WSACleanup();
        return 1;
    }
        
    // add the listener to the master set
    FD_SET(ServerSocket, &Master);

    // keep track of the biggest file descriptor
    MaxFD = (int)ServerSocket;

    // timeval is for setting up timer for select()
    struct timeval TimeVal;
    TimeVal.tv_sec = 2;
    TimeVal.tv_usec = 500000;


    //setting up other peer information when they connect to this server 
    SOCKET TempClientSocket = INVALID_SOCKET;
    struct sockaddr_storage TempClientSocketInfo; // connector's address information
    socklen_t TempClientSocketInfoLength;
    char TempClientIPAddrStringBuffer[INET6_ADDRSTRLEN];
    char TempClientDataBuffer[256];    // buffer for client data
    int NumOfBytes;

    // main server loop
    bool IsRunning = true;
    for(;IsRunning;)
    {
        // call Main menu for user input
        IsRunning = MainMenu(ServerInfo, &Master);
        if(!IsRunning) //check if the user want to exit the problem  
        {
            closesocket(ServerSocket);
            freeaddrinfo(ServerInfo);
            WSACleanup();
            printf("Closing Program");
            return 0;
        }

        Read_FDS = Master;

        //checking for any new signal coming in.
        printf(".....\n");
        if(select(MaxFD + 1, &Read_FDS, NULL, NULL, &TimeVal) == SOCKET_ERROR)
        {
            printf("[ERROR] Failed at select: %d\n", WSAGetLastError());
            closesocket(ServerSocket);
            WSACleanup();
            return 1;
        }
        
        // run through the existing connections looking for data to read        
        for(int i = 0; i <= MaxFD; i++)
        {
            //check if there is any signal activites within socket list 
            if(FD_ISSET(i, &Read_FDS)) // we got an connection
            {//a signal was detected from a socket 
                printf("\n%s\n\n", Border);

                // check if the current socket number is the server socket
                if(i == ServerSocket)
                {
                    //printf("Server running\n");
        
                    // handle new connections
                    TempClientSocketInfoLength = sizeof(TempClientSocketInfo);
                    TempClientSocket =
                        accept(ServerSocket,
                               (struct sockaddr *)&TempClientSocketInfo,
                               &TempClientSocketInfoLength);

                    if(TempClientSocket == SOCKET_ERROR)
                    {
                        printf("[ERROR] Accpet: %d\n", WSAGetLastError());
                    }
                    else
                    {
                        // getting Ip address
                        inet_ntop(TempClientSocketInfo.ss_family,
                                  get_in_addr((struct sockaddr *)&TempClientSocketInfo),
                                  TempClientIPAddrStringBuffer,
                                  sizeof(TempClientIPAddrStringBuffer));
                        
                        if(TempClientIPAddrStringBuffer == NULL)
                        {
                            printf("[ERROR] InetNtop failed: %d\n", WSAGetLastError());
                            continue;
                        }

                        // check if the new peer socket number is great than our highest record socket number 
                        if (TempClientSocket > MaxFD)
                        {
                            // keep track of the max
                            MaxFD = (int)TempClientSocket;
                        }

                        FD_SET(TempClientSocket, &Master); // add to master set
                        
                        printf("The connection to peer %s is successfully established.\n", TempClientIPAddrStringBuffer);
                    }
                }
                else
                {
                    // handle data from a client
                    struct sockaddr_storage ClientSockAddr;
                    socklen_t ClientSockAddrLen = sizeof(ClientSockAddr);
                    char ClientIPAddrStringBuffer[INET6_ADDRSTRLEN];
                    getpeername(i,(struct sockaddr *)&ClientSockAddr,
                                &ClientSockAddrLen);

                    inet_ntop(ClientSockAddr.ss_family,
                              get_in_addr((struct sockaddr*)&ClientSockAddr),
                              ClientIPAddrStringBuffer,
                              sizeof(ClientIPAddrStringBuffer));

                    NumOfBytes = recv(i, TempClientDataBuffer, sizeof(TempClientDataBuffer), 0);
                    //check if data received from client is valid
                    if(NumOfBytes <= 0)
                    {
                        // got error or connection closed by client
                        if(NumOfBytes == 0)
                        {
                            // connection closed
                            printf("Peer %s terminates the connection.\n",
                                   ClientIPAddrStringBuffer);
                        }
                        else
                        {
                            printf("Peer %s has been disconnected.\n",
                                   ClientIPAddrStringBuffer);
                        }
                        
                        closesocket(i);
                        FD_CLR(i, &Master); //remove from master set
                    }
                    else
                    {
                        // valid data received
                        TempClientDataBuffer[NumOfBytes] = '\0';
                        printf("Message received from %s:\n\t>> %s\n",
                               ClientIPAddrStringBuffer,
                               TempClientDataBuffer);
                    }
                }// END handle data from client
                printf("\n%s\n", Border);
            }// END got new incoming connection
        }// END looping through file descriptors
    }// END for(;;) -- and you thought it would never end     
}
