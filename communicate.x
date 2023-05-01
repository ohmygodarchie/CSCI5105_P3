struct file {
    char name[120];
    char checksum[200];
    int size;
};

struct FileList {
    file files[50];
    int fileAmount;
};

struct Node {
    char ip[120];
    int port;
    int load;
    FileList files;
};

struct NodeList {
    Node nodes[50];
    int numNodes;
};

program COMMUNICATE_PROG {
version COMMUNICATE_VERSION
    {
        NodeList Find (string filename) = 1;
        int Download (string filename) = 2;
        int GetLoad(string ip, int port) = 3;
        FileList UpdateList (string ip, int port) = 4;
    } = 1;
} = 0x20000002;
