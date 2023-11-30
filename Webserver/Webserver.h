#pragma once
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>

class Webserver{
private:
  int server_fd;
  struct sockaddr_in address;
  int address_len = sizeof(address);
  char buffer[1024];
  std::string dir;

  std::string getRequestPath(std::string buffer);
  std::string getHeader(std::string content_type, int body_len, int status_code);
  std::string getContentType(std::string path);
  std::string loadFile(std::string path);
  

public:
  Webserver(int port, std::string directory);
  void shutdownServer();
  void init();
  ~Webserver();
};


