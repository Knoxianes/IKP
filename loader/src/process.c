#include "../include/consts.h"
#include "../include/sockets.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include <sys/socket.h>

int main() {
  int rc;
  char buffer[BUFFER_SIZE];
  int conn = connect_process();
  while (1) {
    rc = recv(conn, buffer, sizeof(buffer), 0);
    if (strncmp(buffer, "end", 3) == 0) {
      break;
    }
    sleep(rand() % 10 + 1);
    bzero(buffer, sizeof(buffer));
    strcpy(buffer, "finished");
    rc = send(conn, buffer, sizeof(buffer), 0);
  }
  close(conn);
  return 0;
}
