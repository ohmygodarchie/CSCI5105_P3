/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _COMMUNICATE_H_RPCGEN
#define _COMMUNICATE_H_RPCGEN

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif


struct file {
	char name[120];
	char checksum[200];
	int size;
};
typedef struct file file;

struct FileList {
	file files[50];
	int fileAmount;
};
typedef struct FileList FileList;

struct Node {
	char ip[120];
	int port;
	int load;
	FileList files;
};
typedef struct Node Node;

struct NodeList {
	Node nodes[50];
	int numNodes;
};
typedef struct NodeList NodeList;

struct getload_1_argument {
	char *ip;
	int port;
};
typedef struct getload_1_argument getload_1_argument;

struct updatelist_1_argument {
	char *ip;
	int port;
};
typedef struct updatelist_1_argument updatelist_1_argument;

#define COMMUNICATE_PROG 0x20000002
#define COMMUNICATE_VERSION 1

#if defined(__STDC__) || defined(__cplusplus)
#define Find 1
extern  NodeList * find_1(char *, CLIENT *);
extern  NodeList * find_1_svc(char *, struct svc_req *);
#define Download 2
extern  int * download_1(char *, CLIENT *);
extern  int * download_1_svc(char *, struct svc_req *);
#define GetLoad 3
extern  int * getload_1(char *, int , CLIENT *);
extern  int * getload_1_svc(char *, int , struct svc_req *);
#define UpdateList 4
extern  FileList * updatelist_1(char *, int , CLIENT *);
extern  FileList * updatelist_1_svc(char *, int , struct svc_req *);
extern int communicate_prog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define Find 1
extern  NodeList * find_1();
extern  NodeList * find_1_svc();
#define Download 2
extern  int * download_1();
extern  int * download_1_svc();
#define GetLoad 3
extern  int * getload_1();
extern  int * getload_1_svc();
#define UpdateList 4
extern  FileList * updatelist_1();
extern  FileList * updatelist_1_svc();
extern int communicate_prog_1_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_file (XDR *, file*);
extern  bool_t xdr_FileList (XDR *, FileList*);
extern  bool_t xdr_Node (XDR *, Node*);
extern  bool_t xdr_NodeList (XDR *, NodeList*);
extern  bool_t xdr_getload_1_argument (XDR *, getload_1_argument*);
extern  bool_t xdr_updatelist_1_argument (XDR *, updatelist_1_argument*);

#else /* K&R C */
extern bool_t xdr_file ();
extern bool_t xdr_FileList ();
extern bool_t xdr_Node ();
extern bool_t xdr_NodeList ();
extern bool_t xdr_getload_1_argument ();
extern bool_t xdr_updatelist_1_argument ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_COMMUNICATE_H_RPCGEN */
