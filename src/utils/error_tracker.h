

class errorTracker {

    enum fail_type {
    FAIL_NONE = 0, //No error. Exists mostly for if explicit checks are wanted.
    FAIL_SOCKET_INIT = 1, //Error during socket setup
    FAIL_SOCKET_CONNECT, //Error during socket connecting
    FAIL_SOCKET_HANDSHAKE, //Error during socket handshake
    FAIL_SOCKET_CORE, //Error during normal socket communications
    FAIL_SOCKET_OTHER, //Other error with the socket
    FAIL_FUNCTION_SUCCESS, //A function was not successfully called
    FAIL_ADDRESS_EFFECTIVE, 
    FAIL_MEM_ALLOCATION, //Memory failed to allocate
    FAIL_OTHER, //Other misc error
    };

    errorTracker();
    ~errorTracker();

    fail_type error = FAIL_NONE;
    char details[30];

    bool check_error(bool cond, fail_type err, char details[30]);


};