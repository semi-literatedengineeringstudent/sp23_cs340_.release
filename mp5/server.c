#include "http.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>

void *client_thread(void *vptr) {
  int fd = *((int *)vptr);
  HTTPRequest *req = (HTTPRequest *) malloc(sizeof(HTTPRequest));
  httprequest_read(req, fd);

  char* action = (char*) calloc(sizeof(char),  (strlen(req->action) + 1));
  strcpy(action, req->action);

  char* path = (char*) calloc(sizeof(char),  (strlen(req->path) + 1));
  strcpy(path, req->path);

  char* version = (char*) calloc(sizeof(char),  (strlen(req->version) + 1));
  strcpy(version, req->version);

  if (strlen(path) == 1 && path[0] == '/') {
    free(path);
    path = (char*) calloc(sizeof(char), 12);
    char* file_name = "/index.html";
    strcpy(path, file_name);
  }

  size_t dot_index = -1;
  for (size_t i = 0; i < strlen(path); i++) {
    char dot = '.';
    if (path[i] == dot) {
      dot_index = i;
      break;
    }
  }

  ssize_t extension_length = strlen(path) - dot_index;
  char* file_extension = (char*) calloc(sizeof(char), extension_length + 1);
  strncpy(file_extension, path + dot_index, extension_length);
  char* png = ".png";
  char* html = ".html";
  char* content_type = "wtf";
  if (strcmp(file_extension, png) == 0) {
    content_type = "image/png";
  } else if (strcmp(file_extension, html) == 0) {
    content_type = "text/html";
  } else {
    content_type = "wtf";
  }
  free(file_extension);

  FILE *file;
  char* relative_dir = "./static";
  char* full_file_path = calloc(strlen(path) + strlen(relative_dir) + 1,sizeof(char));
  sprintf(full_file_path, "./static%s", path);

  file = fopen(full_file_path, "r");
  free(full_file_path);
  if (file == NULL) {
    char* response = "404 Not Found";
    char* buffer = (char*) calloc(sizeof(char), strlen(response) + 1);
    send(fd, buffer, strlen(buffer), 0);
    free(buffer);
    close(fd);
    return NULL;
  }
  fseek(file, 0, SEEK_END);//move file pointer to end of file.
  size_t fileLength = ftell(file);//get length by seeing index of pointer at end of file.
  fseek(file, 0, SEEK_SET);
  char* payload = (char*) calloc(fileLength + 1, sizeof(char));
  
  fread(payload, 1, fileLength, file);
  fclose(file);

  char* full_request_begin = malloc(1024);
  sprintf(full_request_begin, "%s 200 OK\r\nContent-Length: %ld\r\nContent_Type: %s\r\n\r\n", version, fileLength, content_type);
  
  char* full_request = malloc(strlen(full_request_begin) + fileLength + 1);
  memcpy(full_request, full_request_begin, strlen(full_request_begin));
  memcpy(full_request + strlen(full_request_begin), payload, fileLength);
  full_request[(int)strlen(full_request_begin) + fileLength] = '\0';

  send(fd, full_request, (int)strlen(full_request_begin) + fileLength, 0);
  
  free(action);
  free(version);
  free(path);
  free(full_request_begin);
  free(full_request);
  free(payload);
  httprequest_destroy(req);
  close(fd);
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return 1;
  }
  int port = atoi(argv[1]);
  printf("Binding to port %d. Visit http://localhost:%d/ to interact with your server!\n", port, port);

  // socket:
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // bind:
  struct sockaddr_in server_addr, client_address;
  memset(&server_addr, 0x00, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);  
  bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr));

  // listen:
  listen(sockfd, 10);

  // accept:
  socklen_t client_addr_len;
  while (1) {
    int *fd = malloc(sizeof(int));
    client_addr_len = sizeof(struct sockaddr_in);
    *fd = accept(sockfd, (struct sockaddr *)&client_address, &client_addr_len);
    printf("Client connected (fd=%d)\n", *fd);

    pthread_t tid;
    pthread_create(&tid, NULL, client_thread, fd);
    pthread_detach(tid);
  }

  return 0;
}