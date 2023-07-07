#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include "wallet.h"

/**
 * Initializes an empty wallet.
 */
void wallet_init(wallet_t *wallet) {
  // Implement `wallet_init`
  //wallet = malloc(sizeof(wallet_t));
  wallet->lock = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(wallet->lock, NULL);

  wallet->cond_withdraw = malloc(sizeof(pthread_cond_t));
  pthread_cond_init(wallet->cond_withdraw, NULL);
  wallet->head = NULL;
}

wallet_node * find_node(wallet_t *wallet, const char* resource) {
  wallet_node *current = wallet->head;
  while(current != NULL) {
    if (strcmp(current->resource_type, resource) == 0) {
      return current;
    }
    current = current->next;
  } 
  return NULL;
}

/**
 * Returns the amount of a given `resource` in the given `wallet`.
 */
int wallet_get(wallet_t *wallet, const char *resource) {
  // Implement `wallet_get`
  wallet_node *to_query = find_node(wallet, resource);
  if (to_query != NULL) {
    return to_query->resource_count;
  } else {
    return 0;
  }
}

/**
 * Modifies the amount of a given `resource` in a given `wallet by `delta`.
 * - If `delta` is negative, this function MUST NOT RETURN until the resource can be satisfied.
 *   (Ths function MUST BLOCK until the wallet has enough resources to satisfy the request.)
 * - Returns the amount of resources in the wallet AFTER the change has been applied.
 */
int wallet_change_resource(wallet_t *wallet, const char *resource, const int delta) {
  // Implement `wallet_change_resource`
  pthread_mutex_lock(wallet->lock);

  wallet_node *to_modify = find_node(wallet, resource);
  if (to_modify == NULL) {
    wallet_node *new_node = malloc(sizeof(wallet_node));

    new_node->resource_count = 0;

    size_t resource_len = strlen(resource);
    new_node->resource_type = (char*)malloc(sizeof(char) * (resource_len+1));
    strcpy(new_node->resource_type, (char*)resource);
    
    new_node->next = wallet->head;

    wallet->head = new_node;
    to_modify = new_node;
  }

  while(wallet_get(wallet, resource) + delta < 0) {
     pthread_cond_wait(wallet->cond_withdraw, wallet->lock);
  }

  to_modify->resource_count = to_modify->resource_count + delta;

  pthread_cond_broadcast(wallet->cond_withdraw);
  pthread_mutex_unlock(wallet->lock);
  return to_modify->resource_count;
}

/**
 * Destroys a wallet, freeing all associated memory.
 */
void wallet_destroy(wallet_t *wallet) {
  // Implement `wallet_destroy`
  pthread_cond_destroy(wallet->cond_withdraw);
  pthread_mutex_destroy(wallet->lock);
  free(wallet->cond_withdraw);
  free(wallet->lock);
  wallet_node *current = wallet->head;
  while(current != NULL) {
    wallet_node *temp = current;
    current = current->next;
    free(temp->resource_type);
    free(temp);
  }
  
}