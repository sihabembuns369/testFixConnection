#include <zmq.h>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h> // Tambahkan untuk sleep
#define ZMQ_AOPROXY_IDENTITY "ORDER_ENGINE"

int main()
{
    int hwm = 0;

    // Inisialisasi context dan socket
    void *context = zmq_ctx_new();
    void *zsock = zmq_socket(context, ZMQ_DEALER);

    if (NULL == zsock)
    {
        std::cout << " - Can not create ZMQ Socket, ZMQAOProxyFrontendThread Ended. " << std::endl;
        return 1;
    }

    zmq_setsockopt(zsock, ZMQ_IDENTITY, ZMQ_AOPROXY_IDENTITY, strlen(ZMQ_AOPROXY_IDENTITY));
    zmq_setsockopt(zsock, ZMQ_RCVHWM, &hwm, sizeof(hwm));
    zmq_setsockopt(zsock, ZMQ_SNDHWM, &hwm, sizeof(hwm));
    zmq_setsockopt(zsock, ZMQ_LINGER, &(hwm = 100), sizeof(hwm)); // Set Linger 100 mseconds
    zmq_setsockopt(zsock, ZMQ_RCVTIMEO, &hwm, sizeof(hwm));       // Receive time out 100

    // Connect socket ke alamat tcp://localhost:5555
    if (-1 == zmq_connect(zsock, "tcp://localhost:5555"))
    {
        std::cout << "koneksi gagal: " << zmq_strerror(errno) << std::endl;
        zmq_close(zsock);
        zmq_ctx_destroy(context);
        return 1;
    }
    else
    {
        std::cout << "koneksi berhasil" << std::endl;
    }

    while (true)
    {
        // Kirim pesan ke server
        const char *message = "Hello from Client!";
        zmq_send(zsock, message, strlen(message), 0);
        std::cout << "Sent: " << message << std::endl;

        // Menerima respons dari server
        // char buffer[256];
        // zmq_msg_t msg;

        // zmq_msg_init(&msg);
        // int s = zmq_msg_recv(&msg, zsock, 0);

        // if (s != -1)
        // {
        //     // Menggunakan zmq_msg_data untuk mendapatkan pointer ke data
        //     std::memcpy(buffer, zmq_msg_data(&msg), zmq_msg_size(&msg));
        //     buffer[zmq_msg_size(&msg)] = '\0'; // Menambahkan null-terminator
        //     std::cout << "Received from Server: " << buffer << std::endl;
        // }
        // else
        // {
        //     std::cout << "Error receiving from server: " << zmq_strerror(errno) << std::endl;
        // }

        // Membersihkan pesan
        // zmq_msg_close(&msg);

        // Menunggu sebentar sebelum mengirim pesan berikutnya
        sleep(1);
    }

    // Menutup socket dan hancurkan context
    zmq_close(zsock);
    zmq_ctx_destroy(context);

    return 0;
}
