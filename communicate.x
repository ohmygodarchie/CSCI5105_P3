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
};

program COMMUNICATE_PROG {
version COMMUNICATE_VERSION
    {
        NodeList Find (char* filename) = 1;
        int Download (char* filename) = 2;
        int GetLoad() = 3;
        FileList UpdateList () = 4;
    } = 1;
} = 0x20000002;
