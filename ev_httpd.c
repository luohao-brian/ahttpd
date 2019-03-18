#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>


static void any_request_cb(struct evhttp_request *req, void *arg);

int main(int argc, char **argv) {
    struct event_base *base;
    struct evhttp *http;
    struct evhttp_bound_socket *handle;
 
    /* firstly, you should build a event_base looper */
    base = event_base_new();
    if (!base) {
        fprintf(stderr, "Couldn't create an event_base: exiting\n");
        return 1;
    }
 
    /* sencondly, Create a new evhttp object to handle requests. */
    http = evhttp_new(base);
    if (!http) {
        fprintf(stderr, "couldn't create evhttp. Exiting.\n");
        return 1;
    }
 
	evhttp_set_gencb(http, any_request_cb, argv[1]);
	handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", 7777);
    if (!handle) {
        fprintf(stderr, "couldn't bind to port %d. Exiting.\n",
                (int)8090);
        return 1;
    }
 
    event_base_dispatch(base);
 
    return 0;
}

static void any_request_cb (struct evhttp_request *req, void *arg)
{
    //fprintf (stderr, "what is my version \n");
    evhttp_send_reply(req, 200, "OK", NULL);
}
