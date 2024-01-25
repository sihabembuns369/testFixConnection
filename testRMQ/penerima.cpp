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
    const char *queue_name = "test";
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

    amqp_queue_declare(conn, 1, amqp_cstring_bytes(queue_name), 0, 0, 0, 1, amqp_empty_table);
    amqp_queue_bind(conn, 1, amqp_cstring_bytes(queue_name), amqp_cstring_bytes(exchange_name),
                    amqp_cstring_bytes(routing_key), amqp_empty_table);

    amqp_basic_consume(conn, 1, amqp_cstring_bytes(queue_name), amqp_empty_bytes, 0, 1, 0, amqp_empty_table);

    printf("Menunggu pesan...\n");

    while (1)
    {
        amqp_rpc_reply_t res;
        amqp_envelope_t envelope;

        amqp_maybe_release_buffers(conn);

        res = amqp_consume_message(conn, &envelope, NULL, 0);

        if (AMQP_RESPONSE_NORMAL != res.reply_type)
        {
            break;
        }

        printf("Menerima pesan:\n");
        printf("  Pesan ID: %u\n", (unsigned)envelope.delivery_tag);
        printf("  Pertukaran: %.*s\n", (int)envelope.exchange.len, (char *)envelope.exchange.bytes);
        printf("  Routing Key: %.*s\n", (int)envelope.routing_key.len, (char *)envelope.routing_key.bytes);
        printf("  Pesan: %.*s\n", (int)envelope.message.body.len, (char *)envelope.message.body.bytes);

        amqp_destroy_envelope(&envelope);
    }

    amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
    amqp_connection_close(conn, AMQP_REPLY_SUCCESS);

    amqp_destroy_connection(conn);
    return 0;
}
