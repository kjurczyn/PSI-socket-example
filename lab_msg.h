#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdint.h>

#ifndef LAB_MSG
#define LAB_MSG
#define TXT_SIZE 128

typedef struct msg_list msg_list ;
struct msg_list;

/** Create a new list
 */
msg_list* msg_list_new();

/** Create and add element to the end of list from values
 */
void msg_list_add(msg_list* list, const int64_t int_64, const int32_t int_32, const char txt[]);

/** Print linked list using specified function
 */
void print_list(msg_list* list, void (*print_func)(const char* restrict format, ...));

/** Print a string and a timestamp in front and newline at the end
 */
void printlog(const char* restrict format, ...);

/** Serialize and send a linked list
 */
int send_list(const msg_list* list, const int sock);

/** Receive and deserialize linked list
 */
msg_list* recv_list(const int sock);

#endif