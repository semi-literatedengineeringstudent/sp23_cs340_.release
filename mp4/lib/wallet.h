#pragma once
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wallet_node_ {
  // Add anything here! :)
  int resource_count;
  char *resource_type;
  struct wallet_node_ *next;
} wallet_node;

typedef struct walslet_t_ {
  // Add anything here! :)
  struct wallet_node_ *head;
  pthread_cond_t *cond_withdraw;
  pthread_mutex_t *lock;
} wallet_t;

void wallet_init(wallet_t *wallet);
wallet_node * find_node(wallet_t *wallet, const char* resource);
int wallet_get(wallet_t *wallet, const char *resource);
int wallet_change_resource(wallet_t *wallet, const char *resource, const int delta);
void wallet_destroy(wallet_t *wallet);

#ifdef __cplusplus
}
#endif