#ifndef ENUMS_H
#define ENUMS_H
enum struct ResponseType{
    NOT_FOUND,
    BAD_REQUEST,
    OK
};

enum struct NetworkEvent{
    POLLING_ERROR,
    NO_ERROR
};

enum struct NetworkError{
    ERROR_NO_ERROR,
    PORT_NO_INTEGER,
    BIND_FAILED,
    LISTEN_FAILED,
    ACCEPT_FAILED,
    SOCKET_FAILED,
    POLL_FAILED,
    SET_SOCK_OPT_FAILED
};
#endif // ENUMS_H
