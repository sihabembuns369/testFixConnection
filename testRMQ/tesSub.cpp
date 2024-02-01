#include <zmq.h>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h> // Tambahkan untuk sleep

int main()
{
    void *context = zmq_ctx_new();
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    zmq_connect(subscriber, "tcp://localhost:5555");
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0);
    std::cout << "Subscriber (Pelanggan) started..." << std::endl;

    while (true)
    {
        zmq_msg_t msg;
        zmq_msg_init(&msg);
        int received_bytes = zmq_msg_recv(&msg, subscriber, 0);
        size_t msg_size = zmq_msg_size(&msg);
        char buffer[256];
        memcpy(buffer, zmq_msg_data(&msg), std::min(msg_size, sizeof(buffer) - 1));

        buffer[std::min(msg_size, sizeof(buffer) - 1)] = '\0';

        std::cout << "Received: " << buffer << std::endl;

        // Clean up the message
        zmq_msg_close(&msg);
    }

    zmq_close(subscriber);
    zmq_ctx_destroy(context);

    return 0;
}
