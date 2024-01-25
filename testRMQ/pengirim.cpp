#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <amqp_tcp_socket.h>
#include <amqp.h>

int main()
{
    const char *hostname = "localhost";
    int port = 5672;
    const char *username = "user";
    const char *password = "user";
    const char *exchange_name = "test_exchange";
    const char *routing_key = "test_routing_key";

    amqp_connection_state_t conn = amqp_new_connection();
    amqp_socket_t *socket = amqp_tcp_socket_new(conn);

    int status = amqp_socket_open(socket, hostname, port);
    if (status != AMQP_STATUS_OK)
    {
        fprintf(stderr, "Gagal membuka socket ke RabbitMQ\n");
        return 1;
    }

    amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, username, password);
    amqp_channel_open(conn, 1);
    amqp_get_rpc_reply(conn);

    amqp_exchange_declare(conn, 1, amqp_cstring_bytes(exchange_name), amqp_cstring_bytes("fanout"),
                          0, 0, 0, 0, amqp_empty_table);
    amqp_queue_declare(conn, 1, amqp_cstring_bytes("test"), 0, 0, 0, 1, amqp_empty_table);
    amqp_queue_bind(conn, 1, amqp_cstring_bytes("test"), amqp_cstring_bytes(exchange_name),
                    amqp_cstring_bytes(routing_key), amqp_empty_table);

    const char *message_body = "Hello, RabbitMQ!";
    amqp_basic_properties_t props;
    props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
    props.content_type = amqp_cstring_bytes("text/plain");
    props.delivery_mode = 2; // Pesan persisten

    amqp_basic_publish(conn, 1, amqp_cstring_bytes(exchange_name), amqp_cstring_bytes(routing_key),
                       0, 0, &props, amqp_cstring_bytes(message_body));

    printf("Pesan berhasil dikirim\n");

    amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
    amqp_connection_close(conn, AMQP_REPLY_SUCCESS);

    amqp_destroy_connection(conn);
    return 0;
}
