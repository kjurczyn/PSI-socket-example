#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include "lab_msg.h"

/** Node of a linked list
 */
typedef struct msg_node { // size 160
    struct msg_node* next_node_;
    int64_t int_64_;
    int32_t int_32_;
    uint32_t index_;
    char txt_[TXT_SIZE];
} msg_node;
static const int msg_node_size = sizeof(msg_node);

/** Linked list descriptor
 */
struct msg_list {
    uint32_t size;
    msg_node* first;
    msg_node* last;
};
static const int msg_list_size = sizeof(msg_list);

/** Create a new list
 */
msg_list* msg_list_new() {
    msg_list* list = malloc(sizeof(msg_list));
    list->size = 0;
    list->first = NULL;
    list->last = NULL;

    return list;
}

/** Create and add element to the end of list from values
 */
void msg_list_add(msg_list* list, const int64_t int_64, const int32_t int_32, const char txt[]) {
    msg_node* node = malloc(msg_node_size);
    node->int_64_ = int_64;
    node->int_32_ = int_32;
    memcpy(node->txt_, txt, TXT_SIZE);
    node->next_node_ = NULL;

    if (list->last == NULL) {
        node->index_ = 1;
        list->first = node;
        list->last = node;
        list->size++;
        return;
    }

    node->index_ = list->last->index_ + 1;
    list->last->next_node_ = node;
    list->last = node;
    list->size ++;
    return;
}

/** Add existing node to the end of list
 */
void msg_list_append(msg_list *list, msg_node* node) {
    if (list->last == NULL) {
        list->first = node;
        node->index_ = 1;
    } else {
        list->last->next_node_ = node;
        node->index_ = list->last->index_ + 1;
    }
    list->last = node;
    node->next_node_ = NULL;
    list->size++;
    return;
}

/** Print linked list using specified function
 */
void print_list(msg_list* list, void (*print_func)(const char* restrict format, ...)) {
    msg_node* next = list->first;
    while (next) {
        print_func("-------------------------------------");
        print_func("Node no. %i,", next->index_);
        print_func("int_64: %lli,", next->int_64_);
        print_func("int_32: %i", next->int_32_);
        print_func("text: %s,", next->txt_);
        next = next->next_node_;
    }
    return;
}

/** Print a string and a timestamp in front and newline at the end
 */
void printlog(const char* restrict format, ...) {
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    printf("[%i:%i:%i] ", local->tm_hour, local->tm_min, local->tm_sec);
    va_list arg;
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
    printf("\n");
    return;
}

void serialize_size(const uint32_t hostsize, uint8_t buffer[sizeof(uint32_t)]) {
    uint32_t netsize = htonl(hostsize);
    memcpy(buffer, &netsize, sizeof(uint32_t));
    return;
}

/** Serialize node into buffer and return size
 */
size_t serialize_node(const msg_node* node, uint8_t* buffer) {
    uint32_t net_int_64_1 = htonl((uint32_t) (node->int_64_ >> 32));
    uint32_t net_int_64_2 = htonl((uint32_t) node->int_64_);
    uint32_t net_int_32 = htonl((uint32_t) node->int_32_);
    uint32_t net_index = htonl(node->index_);

    size_t txt_len = strlen(node->txt_) + 1;
    uint8_t* net_txt = malloc(txt_len);
    for (int i = 0; i < txt_len; i++) {
        net_txt[i] = htons(node->txt_[i] << 8);
    }

    size_t offset = 0;
    memcpy(buffer + offset, &net_int_64_1, sizeof(net_int_64_1));
    offset += sizeof(net_int_64_1);
    memcpy(buffer + offset, &net_int_64_2, sizeof(net_int_64_2));
    offset += sizeof(net_int_64_2);
    memcpy(buffer + offset, &net_int_32, sizeof(net_int_32));
    offset += sizeof(net_int_32);
    memcpy(buffer + offset, &net_index, sizeof(net_index));
    offset += sizeof(net_index);
    memcpy(buffer + offset, net_txt, txt_len);

    return offset + txt_len;
}

/** Serialize and send list size 
 */
int send_size(const msg_list* list, const int sock) {
    uint8_t buffer[sizeof(uint32_t)];
    serialize_size(list->size, buffer);
    
    size_t sent = send(sock, buffer, sizeof(uint32_t), 0);
    
    if (sent != sizeof(uint32_t)) return -1;

    return 0;
}

/** Serialize and send a linked list node
 */
size_t send_node(const msg_node* node, const int sock) {
    uint8_t buffer[msg_node_size];
    size_t size = serialize_node(node, buffer);
    
    size_t sent = send(sock, buffer, size, 0);
    
    if (sent != size) return 0;
    
    return size;
}

/** Serialize and send a linked list
 */
int send_list(const msg_list* list, const int sock) {
    size_t sent_sum = 0;
    if(send_size(list, sock) != 0) {
        printlog("Failed to send size");
        return -1;
    }
    msg_node* node = list->first;
    while(node) {
        size_t sent = send_node(node, sock);
        if(sent == 0) {
            printlog("Failed to send node %u", node->index_);
            return -1;
        }
        sent_sum += sent;
        printlog("Sent node no %lu", node->index_);
        node = node->next_node_;
        usleep(2000);
    }
    printlog("Sent %u bytes in total", sent_sum);
    
    return 0;
}

size_t deserialize_size(uint8_t buffer[sizeof(uint32_t)]) {
    size_t netsize;
    memcpy(&netsize, buffer, sizeof(uint32_t));
    size_t size = ntohl(netsize);
    return size;
}

/** Deserialize newly allocated node from buffer
 */
msg_node* deserialize_node(uint8_t buffer[msg_node_size]) {
    uint32_t net_int_64_1;
    uint32_t net_int_64_2;
    uint32_t net_int_32;
    uint32_t net_index;

    size_t offset = 0;
    memcpy(&net_int_64_1, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&net_int_64_2, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&net_int_32, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&net_index, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    size_t txt_len = strlen((char*) buffer + offset);
    msg_node* node = malloc(msg_node_size);
    node->index_ = ntohl(net_index);
    node->int_32_ = (int32_t) ntohl(net_int_32);
    node->int_64_ =  (int64_t) ((uint64_t) ntohl(net_int_64_1) << 32 ) + (uint64_t) ntohl(net_int_64_2); 

    for (int i = 0; i < txt_len; i++) {
        node->txt_[i] = ntohs((buffer + offset)[i]) >> 8; 
    }

    return node;
}

/** Receive and deserialize list size
 */
uint32_t* recv_size(const int sock) {
    uint8_t buffer[sizeof(uint32_t)];

    size_t received = recv(sock, buffer, sizeof(uint32_t), 0);
    if (received != sizeof(uint32_t)) return NULL;

    size_t offset = 0;
    uint32_t* size = malloc(sizeof(uint32_t)); 
    *size = deserialize_size(buffer);

    return size;
}

/** Receive and deserialize node
 */
msg_node* recv_node(const int sock) {
    uint8_t buffer[msg_node_size];
    
    size_t received = recv(sock, buffer, msg_node_size, 0);
    if (received < sizeof(uint32_t) * 4) return NULL;

    msg_node* node = deserialize_node(buffer);

    return node;
}

/** Receive and deserialize linked list
 */
msg_list* recv_list(const int sock) {
    uint32_t* size = recv_size(sock);
    if (!size) {
        printlog("Failed to receive list length");
        return NULL;
    }

    printlog("Received list length: %lu", *size);
    msg_list* list = msg_list_new();

    for (int i = 1; i <= *size; i++) {
        msg_node* node = recv_node(sock);
        if (!node) {
            printlog("Failed to receive node no: %i", i);
            return NULL;
        }
        msg_list_append(list, node);
        printlog("Received node no: %u", node->index_);
    }

    return list;
}