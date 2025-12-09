/**
 * @file TcpServer.hpp
 * @author Niklas Gaudlitz (niklas.gaudlitz@ed-chemnitz.de)
 * @brief non blocking TCP server based on https://github.com/espressif/esp-idf/tree/213504238f77e01073f668e5e8f87e3b3cc02a8f/examples/protocols/sockets/non_blocking
 * @version 0.1
 * @date 2023-06-12
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef TCP_SERVER_HPP_
#define TCP_SERVER_HPP_

#include "FreeRTOS.hpp"
#include "Task.hpp"

#include <string>

#include "netdb.h"
#include "lwip/sockets.h"
#include "esp_log.h"

/**
 * @brief Indicates that the file descriptor represents an invalid (uninitialized or closed) socket
 *
 * Used in the TCP server structure `sock[]` which holds list of active clients we serve.
 */
#define INVALID_SOCK (-1)

/**
 * @brief Time in ms to yield to all tasks when a non-blocking socket would block
 *
 * Non-blocking socket operations are typically executed in a separate task validating
 * the socket status. Whenever the socket returns `EAGAIN` (idle status, i.e. would block)
 * we have to yield to all tasks to prevent lower priority tasks from starving.
 */
#define YIELD_TO_ALL_MS 50


class TcpServer: public Task{
    public:
        TcpServer();
        ~TcpServer();

        void startServer(std::string tcpServerBindAddress, std::string tcpServerBindPort);
        void stopServer();

        bool isRunning(){return m_tcpServerIsRunning;}

    protected:
        //function which will be run as a task
        void run(void * data);

        //flag which stops the task; indicate true for stop
        bool m_tcpServerShouldStop{true};
        bool m_tcpServerIsRunning{false};

        //Semaphores to sync with start and stop of task
        SemaphoreHandle_t m_serverStartupDone;
        SemaphoreHandle_t m_serverShutdownDone;

        //function to abbort the server if something goes wrong or server is closed
        void shutdownServer(bool abbortBeforeStartupComplete);

        //helper functions for non-blocking TCP socket server
        static void log_socket_error(const char *tag, const int sock, const int err, const char *message);
        static int try_receive(const char *tag, const int sock, char * data, size_t max_len);
        static int socket_send(const char *tag, const int sock, const char * data, const size_t len);

        static inline char* get_clients_address(struct sockaddr_storage *source_addr);

        //overrideable function for what to do if a connection is accepted
        virtual void handleAcceptedConnection(int socketContext);

        //variables for TCP server functionality
        static char m_rx_buffer[128];
        struct addrinfo m_hints = { 0,      		/* Input flags. */
                                    0,     			/* Address family of socket. */
                                    SOCK_STREAM,   	/* Socket type. */
                                    0,  			/* Protocol of socket. */
                                    0,    			/* Length of socket address. */
                                    nullptr,       	/* Socket address of socket. */
                                    nullptr,  		/* Canonical name of service location. */
                                    nullptr       	/* Pointer to next in list. */
                                };

        struct addrinfo*        m_address_info;
        const size_t            m_max_socks{CONFIG_LWIP_MAX_SOCKETS - 1};
        static int              m_sock[CONFIG_LWIP_MAX_SOCKETS - 1];
        int                     m_listen_sock{INVALID_SOCK};

        //internal variables for the bindAddress and binPort
        std::string m_tcpServerBindAddress{"0.0.0.0"};
        std::string m_tcpServerBindPort{"502"};
};

#endif //TCP_SERVER_HPP_