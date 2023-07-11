#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "http.h"

void parse_firstline(HTTPRequest *req, char *buffer, ssize_t segment_start, ssize_t segment_end) {
  if (segment_end - segment_start == 1) {
    return;
  }
  ssize_t action_start = segment_start;
  if (segment_start != 0) {
    action_start = segment_start + 1;

  }
  
  ssize_t action_end = 0;

  ssize_t path_start = 0;
  ssize_t path_end = 0;

  ssize_t version_start = 0;
  ssize_t version_end = 0;

  int space_count = 0;
  for (ssize_t i = segment_start; i <= segment_end - 1; i++) {
    char to_compare = buffer[i];
    char space = ' ';
    if (to_compare == space && space_count == 0) {
      action_end = i - 1;
      path_start = i + 1;
      space_count = space_count + 1;
    } else if (to_compare == space && space_count == 1) {
      path_end = i - 1;
      version_start = i + 1;
      version_end = segment_end - 1;
      break;
    } else {
      continue;
    }
  }
  ssize_t action_size = action_end - action_start + 1;
  ssize_t path_size = path_end - path_start + 1;
  ssize_t version_size = version_end - version_start + 1;

  char* action_string = (char*) malloc(sizeof(char) * (action_size + 1));
  strncpy(action_string, buffer + action_start, action_size);
  action_string[action_size] = '\0';
  req->action = (const char*) action_string;

  char* path_string = (char*) malloc(sizeof(char) * (path_size + 1));
  strncpy(path_string, buffer + path_start, path_size);
  path_string[path_size] = '\0';
  req->path = (const char*) path_string;

  char* version_string = (char*) malloc(sizeof(char) * (version_size + 1));
  strncpy(version_string, buffer + version_start, version_size);
  version_string[version_size] = '\0';
  req->version = (const char*) version_string;

  return;
}

void parse_key_value_pair(HTTPRequest *req, char *buffer, ssize_t segment_start, ssize_t segment_end) {
  if (segment_end - segment_start == 1) {
    return;
  }
  ssize_t key_start = segment_start + 1;
  ssize_t key_end = 0;

  ssize_t value_start = 0;
  ssize_t value_end = 0;

  for (ssize_t i = segment_start; i <= segment_end - 1; i++) {
    char to_compare1 = buffer[i];
    char to_compare2 = buffer[i + 1];
    char colon = ':';
    char space = ' ';
    if (to_compare1 == colon && to_compare2 == space) {
      key_end = i - 1;
      value_start = i + 2;
      value_end = segment_end - 1;
      break;
    } else {
      continue;
    }
  }
  ssize_t key_size = key_end - key_start + 1;
  ssize_t value_size = value_end - value_start + 1;

  key_value_pair * new_pair = (key_value_pair *)malloc(sizeof(key_value_pair));

  char* key_string = (char *) malloc(sizeof(char) * (key_size + 1));
  strncpy(key_string, buffer + key_start, key_size);
  key_string[key_size] = '\0';
  new_pair->key = key_string;

  char* value_string = (char *) malloc(sizeof(char) * (value_size + 1));
  strncpy(value_string, buffer + value_start, value_size);
  value_string[value_size] = '\0';
  new_pair->value = value_string;

  new_pair->next = req->head_pair;
  req->head_pair = new_pair;
 
  return;
}

/**
 * httprequest_parse_headers
 * 
 * Populate a `req` with the contents of `buffer`, returning the number of bytes used from `buf`.
 */
ssize_t httprequest_parse_headers(HTTPRequest *req, char *buffer, ssize_t buffer_len) {
  req->head_pair = NULL;
  ssize_t segment_start = 0;
  ssize_t segment_end = 0;
  ssize_t size_used = 0;

  while(1) {

    for (ssize_t i = segment_start; i < buffer_len; i++) {
      char to_compare1 = buffer[i];
      char to_compare2 = buffer[i + 1];
      char slash_r = '\r';
      char slash_n = '\n';
      if (to_compare1 == slash_r && to_compare2 == slash_n) {
        segment_end = i;
        break;
      }
    }
    
    if (segment_start == 0) {
      parse_firstline(req, buffer, segment_start, segment_end);
      size_used = size_used + (segment_end - segment_start - 1);
      segment_start = segment_end + 1;
    } else {
      parse_key_value_pair(req, buffer, segment_start, segment_end);
      size_used = size_used + (segment_end - segment_start - 1);

      char to_compare3 = buffer[segment_end + 2];
      char to_compare4 = buffer[segment_end + 3];
      char slash_r = '\r';
      char slash_n = '\n';

      if (!(to_compare3 == slash_r && to_compare4 == slash_n)) {
        segment_start = segment_end + 1;
      } else {
        if (segment_end + 3 == buffer_len - 1) {
          req->payload = NULL;
          break;
        } else {
          ssize_t payload_start = segment_end + 3;
          ssize_t payload_size = atol(httprequest_get_header(req, "Content-Length"));
          if (payload_size < 0) {
            req->payload = NULL;
            break;
          }
          char* payload_buffer = malloc(sizeof(char*) * (payload_size + 1));
          memcpy(payload_buffer, buffer + payload_start + 1, payload_size);
          payload_buffer[payload_size] = '\0';
          req->payload = payload_buffer;

          size_used = size_used + payload_size;
          break;
        }
      }
    }
  }
  return size_used;
}


/**
 * httprequest_read
 * 
 * Populate a `req` from the socket `sockfd`, returning the number of bytes read to populate `req`.
 */
ssize_t httprequest_read(HTTPRequest *req, int sockfd) {
  ssize_t size = 4096;
  char* buffer = (char*)malloc(size);
  ssize_t current_length = 0;
  while(1) {
    ssize_t len = read(sockfd, buffer + current_length, 4096);
    if (len == -1) {
      return -1;
    }
    if (len == 4096){
      size = size + 4096;
      current_length = current_length + 4096;
      buffer = realloc(buffer, size);
    } else {
      current_length = current_length + len;
      break;
    }
  }

  ssize_t len_read = httprequest_parse_headers(req, buffer, current_length);
  free(buffer);

  return len_read;
}


/**
 * httprequest_get_action
 * 
 * Returns the HTTP action verb for a given `req`.
 */
const char *httprequest_get_action(HTTPRequest *req) {
  return req->action;
}


/**
 * httprequest_get_header
 * 
 * Returns the value of the HTTP header `key` for a given `req`.
 */
const char *httprequest_get_header(HTTPRequest *req, const char *key) {
  key_value_pair *current = req->head_pair;
  if (current == NULL) {
    return NULL;
  } else {
    while(current != NULL) {
      if (strcmp(current->key, key) == 0) {
        return current->value;
      }
      current = current->next;
    }
    return NULL;
  }
}


/**
 * httprequest_get_path
 * 
 * Returns the requested path for a given `req`.
 */
const char *httprequest_get_path(HTTPRequest *req) {
  return req->path;
}


/**
 * httprequest_destroy
 * 
 * Destroys a `req`, freeing all associated memory.
 */
void httprequest_destroy(HTTPRequest *req) {
  key_value_pair *current = req->head_pair;
  while(current != NULL) {
    key_value_pair * temp = current;
    current = temp->next;
    free((char*)temp->key);
    free((char*)temp->value);
    free(temp);
  }
  if(req->action != NULL) {
    free((char*)req->action);
  }
  if (req->path != NULL) {
    free((char*)req->path);
  }
  if (req->payload != NULL) {
    free((void*)req->payload);
  }
  if (req->version != NULL) {
    free((char*)req->version);
  }
  return;
}