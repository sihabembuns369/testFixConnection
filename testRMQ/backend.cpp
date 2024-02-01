#include <zmq.h>
#include <iostream>

int main()
{
    void *context = zmq_ctx_new();
    void *backend = zmq_socket(context, ZMQ_ROUTER);

    // Bind to a specific address, for example, tcp://localhost:5555
    zmq_bind(backend, "tcp://localhost:5555");

    std::cout << "Backend (Server) started..." << std::endl;

    zmq_proxy(backend, backend, nullptr); // Proxy loop

    zmq_close(backend);
    zmq_ctx_destroy(context);

    return 0;
}
