/**
 * @file TcpServer.cpp
 * @author Niklas Gaudlitz (niklas.gaudlitz@ed-chemnitz.de)
 * @brief 
 * @version 0.1
 * @date 2023-06-12
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "TcpServer.hpp"

static char LOGTAG[]="TcpServer";

char TcpServer::m_rx_buffer[128];
int TcpServer::m_sock[CONFIG_LWIP_MAX_SOCKETS - 1];

TcpServer::TcpServer():
    Task("TcpServer", 8192, 5){

}


TcpServer::~TcpServer(){

}

void TcpServer::startServer(std::string tcpServerBindAddress, std::string tcpServerBindPort){

    //set the binAddress and bindPort for the new server that will be started
    m_tcpServerBindAddress = tcpServerBindAddress;
    m_tcpServerBindPort = tcpServerBindPort;
    
    if(m_tcpServerIsRunning == false){
        //create Semaphore to block until the Server is started
        m_serverStartupDone = xSemaphoreCreateBinary();
        assert(m_serverStartupDone);
        m_tcpServerShouldStop = false;

        //start the Task
        Task::start();

        //wait for the Server to be ready and hands over the Semaphore
        xSemaphoreTake(m_serverStartupDone, portMAX_DELAY);
        vSemaphoreDelete(m_serverStartupDone);

        ESP_LOGD(LOGTAG, "Server startup complete.");
    }else{
        stopServer();
        startServer(m_tcpServerBindAddress, tcpServerBindPort);
    }



}

void TcpServer::stopServer(){

    if(m_tcpServerIsRunning == true){
        //create Semaphore to block until the Server is shut down
        m_serverShutdownDone = xSemaphoreCreateBinary();
        assert(m_serverShutdownDone);

        //signal the task to stop
        m_tcpServerShouldStop = true;

        //wait for the Server to be shut down and hands over the Semaphore
        xSemaphoreTake(m_serverShutdownDone, portMAX_DELAY);
        vSemaphoreDelete(m_serverShutdownDone);

        ESP_LOGD(LOGTAG, "Server shutdown complete.");
    }else{
        ESP_LOGE(LOGTAG, "Server already shut down. Maybe we aborted a previous attempt?");
    }

}

void TcpServer::run(void * data){  

    // Prepare a list of file descriptors to hold client's sockets, mark all of them as invalid, i.e. available
    for (int i=0; i < m_max_socks; ++i) {
        m_sock[i] = INVALID_SOCK;
    }

    // Translating the hostname or a string representation of an IP to address_info
    int res = getaddrinfo(m_tcpServerBindAddress.c_str(), m_tcpServerBindPort.c_str(), &m_hints, &m_address_info);

    if (res != 0 || m_address_info == NULL) {
        ESP_LOGE(LOGTAG, "couldn't get hostname for `%s` "
                      "getaddrinfo() returns %d, addrinfo=%p", m_tcpServerBindAddress.c_str(), res, m_address_info);

        shutdownServer(true);
    }

    // Creating a listener socket
    m_listen_sock = socket(m_address_info->ai_family, m_address_info->ai_socktype, m_address_info->ai_protocol);

    if (m_listen_sock < 0) {
        log_socket_error(LOGTAG, m_listen_sock, errno, "Unable to create socket");

        shutdownServer(true);
    }

    ESP_LOGI(LOGTAG, "Listener socket created");

    // Marking the socket as non-blocking
    int flags = fcntl(m_listen_sock, F_GETFL);

    if (fcntl(m_listen_sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        log_socket_error(LOGTAG, m_listen_sock, errno, "Unable to set socket non blocking");

        shutdownServer(true);
    }

    ESP_LOGI(LOGTAG, "Socket marked as non blocking");

    // Binding socket to the given address
    //TODO: comment out next to lines; just to see if error handling is working
    //maybe build a function abort with context to give connection status back to Controller
	int opt = 1;
    setsockopt(m_listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    int err = bind(m_listen_sock, m_address_info->ai_addr, m_address_info->ai_addrlen);

    if (err != 0) {
        log_socket_error(LOGTAG, m_listen_sock, errno, "Socket unable to bind");

        shutdownServer(true);
    }

    ESP_LOGI(LOGTAG, "Socket bound on %s:%s", m_tcpServerBindAddress.c_str(), m_tcpServerBindPort.c_str());

    // Set queue (backlog) of pending connections to one (can be more)
    err = listen(m_listen_sock, 1);

    if (err != 0) {
        log_socket_error(LOGTAG, m_listen_sock, errno, "Error occurred during listen");

        shutdownServer(true);
    }

    ESP_LOGI(LOGTAG, "Socket listening");

    //release the startServer call from its hold. Server now started
    xSemaphoreGive(m_serverStartupDone);
    m_tcpServerIsRunning = true;

    // Main loop for accepting new connections and serving all connected clients
    while (m_tcpServerShouldStop == false) {
        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);

        // Find a free socket
        int new_sock_index = 0;
        for (new_sock_index=0; new_sock_index < m_max_socks; ++new_sock_index) {
            if (m_sock[new_sock_index] == INVALID_SOCK) {
                break;
            }
        }

        // We accept a new connection only if we have a free socket
        if (new_sock_index < m_max_socks) {
            // Try to accept a new connections
            m_sock[new_sock_index] = accept(m_listen_sock, (struct sockaddr *)&source_addr, &addr_len);

            if (m_sock[new_sock_index] < 0) {
                if (errno == EWOULDBLOCK) { // The listener socket did not accepts any connection
                                            // continue to serve open connections and try to accept again upon the next iteration
                    ESP_LOGV(LOGTAG, "No pending connections...");
                } else {
                    log_socket_error(LOGTAG, m_listen_sock, errno, "Error when accepting connection");
                    shutdownServer(false);
                }
            } else {
                // We have a new client connected -> print it's address
                ESP_LOGI(LOGTAG, "[sock=%d]: Connection accepted from IP:%s", m_sock[new_sock_index], get_clients_address(&source_addr));

                // ...and set the client's socket non-blocking
                flags = fcntl(m_sock[new_sock_index], F_GETFL);
                if (fcntl(m_sock[new_sock_index], F_SETFL, flags | O_NONBLOCK) == -1) {
                    log_socket_error(LOGTAG, m_sock[new_sock_index], errno, "Unable to set socket non blocking");
                    shutdownServer(false);
                }
                ESP_LOGI(LOGTAG, "[sock=%d]: Socket marked as non blocking", m_sock[new_sock_index]);
            }
        }

        // We serve all the connected clients in this loop
        for (int i=0; i < m_max_socks; ++i) {
            if (m_sock[i] != INVALID_SOCK) {

                handleAcceptedConnection(i);

            } // one client's socket
        } // for all sockets

        // Yield to other tasks
        FreeRTOS::sleep(YIELD_TO_ALL_MS);
    }

    //if we leave the while loop we shut down the server -> lets exit savely
    shutdownServer(false);
}

void TcpServer::shutdownServer(bool abbortBeforeStartupComplete){
    
    //check if the listen socket invalid, i.e. available -> if not close it 
    if (m_listen_sock != INVALID_SOCK) {
        ESP_LOGW(LOGTAG, "[sock=%d]: Closing listen Socket", m_listen_sock);
        close(m_listen_sock);
        m_listen_sock = INVALID_SOCK;
    }

    //check if the client sockets invalid, i.e. available -> if not close them
    for (int i=0; i < m_max_socks; ++i) {
        if (m_sock[i] != INVALID_SOCK) {
            ESP_LOGW(LOGTAG, "[sock=%d]: Closing client Socket", m_sock[i]);
            close(m_sock[i]);
            m_sock[i] = INVALID_SOCK;
        }
    }

	ESP_LOGW(LOGTAG, "Exiting Task!");

    //free the memory of addrinfo * m_address_info
    free(m_address_info);

    if(abbortBeforeStartupComplete == true){
        //we aborted the startup so we still have to release the Semaphore
        xSemaphoreGive(m_serverStartupDone);
    }


    if(m_tcpServerShouldStop == true){
        //we got the signal to stop the server and are done. Lets release the semaphore 
        xSemaphoreGive(m_serverShutdownDone);
    }

    m_tcpServerIsRunning = false;

    //stop the task
    Task::stop();
    
    ESP_LOGE(LOGTAG, "Trying to return, we should not have ended up here!");
    return;
}

/**
 * @brief Utility to log socket errors
 *
 * @param[in] tag Logging tag
 * @param[in] sock Socket number
 * @param[in] err Socket errno
 * @param[in] message Message to print
 */
void TcpServer::log_socket_error(const char *tag, const int sock, const int err, const char *message){

    ESP_LOGE(tag, "[sock=%d]: %s\n""error=%d: %s", sock, message, err, strerror(err));

}

/**
 * @brief Tries to receive data from specified sockets in a non-blocking way,
 *        i.e. returns immediately if no data.
 *
 * @param[in] tag Logging tag
 * @param[in] sock Socket for reception
 * @param[out] data Data pointer to write the received data
 * @param[in] max_len Maximum size of the allocated space for receiving data
 * @return
 *          >0 : Size of received data
 *          =0 : No data available
 *          -1 : Error occurred during socket read operation
 *          -2 : Socket is not connected, to distinguish between an actual socket error and active disconnection
 */
int TcpServer::try_receive(const char *tag, const int sock, char * data, size_t max_len){

    int len = recv(sock, data, max_len, 0);
    if (len < 0) {
        if (errno == EINPROGRESS || errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;   // Not an error
        }
        if (errno == ENOTCONN) {
            ESP_LOGW(LOGTAG, "[sock=%d]: Connection closed", sock);
            return -2;  // Socket has been disconnected
        }
        log_socket_error(LOGTAG, sock, errno, "Error occurred during receiving");
        return -1;
    }

    return len;

}

/**
 * @brief Sends the specified data to the socket. This function blocks until all bytes got sent.
 *
 * @param[in] tag Logging tag
 * @param[in] sock Socket to write data
 * @param[in] data Data to be written
 * @param[in] len Length of the data
 * @return
 *          >0 : Size the written data
 *          -1 : Error occurred during socket write operation
 */
int TcpServer::socket_send(const char *tag, const int sock, const char * data, const size_t len){

    int to_write = len;
    while (to_write > 0) {
        int written = send(sock, data + (len - to_write), to_write, 0);
        if (written < 0 && errno != EINPROGRESS && errno != EAGAIN && errno != EWOULDBLOCK) {
            log_socket_error(LOGTAG, sock, errno, "Error occurred during sending");
            return -1;
        }
        to_write -= written;
    }
    return len;

}

/**
 * @brief Returns the string representation of client's address (accepted on this server)
 */
inline char* TcpServer::get_clients_address(struct sockaddr_storage *source_addr){

    static char address_str[128];
    char *res = NULL;
    // Convert ip address to string
    if (source_addr->ss_family == PF_INET) {
        res = inet_ntoa_r(((struct sockaddr_in *)source_addr)->sin_addr, address_str, sizeof(address_str) - 1);
    }
#ifdef CONFIG_LWIP_IPV6
    else if (source_addr->ss_family == PF_INET6) {
        res = inet6_ntoa_r(((struct sockaddr_in6 *)source_addr)->sin6_addr, address_str, sizeof(address_str) - 1);
    }
#endif
    if (!res) {
        address_str[0] = '\0'; // Returns empty string if conversion didn't succeed
    }
    return address_str;

}

/**
 * @brief example handle function to server an accepted connection; can be overwritten
 * 
 */
void TcpServer::handleAcceptedConnection(int socketContext){
    // This is an open socket -> try to serve it
    int len = try_receive(LOGTAG, m_sock[socketContext], m_rx_buffer, sizeof(m_rx_buffer));

    if (len < 0) {

        // Error occurred within this client's socket -> close and mark invalid
        ESP_LOGI(LOGTAG, "[sock=%d]: try_receive() returned %d -> closing the socket", m_sock[socketContext], len);
        close(m_sock[socketContext]);
        m_sock[socketContext] = INVALID_SOCK;

    } else if (len > 0) {

        // Received some data -> echo back
        ESP_LOGI(LOGTAG, "[sock=%d]: Received %.*s", m_sock[socketContext], len, m_rx_buffer);

        len = socket_send(LOGTAG, m_sock[socketContext], m_rx_buffer, len);
        if (len < 0) {
            // Error occurred on write to this socket -> close it and mark invalid
            ESP_LOGI(LOGTAG, "[sock=%d]: socket_send() returned %d -> closing the socket", m_sock[socketContext], len);
            close(m_sock[socketContext]);
            m_sock[socketContext] = INVALID_SOCK;
        } else {
            // Successfully echoed to this socket
            ESP_LOGI(LOGTAG, "[sock=%d]: Written %.*s", m_sock[socketContext], len, m_rx_buffer);
        }

    }
}
