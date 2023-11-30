#include "Webserver.h"
#include <cstdlib>
#include <exception>
#include <iostream> 
#include <string>
#include <sys/socket.h>
#include <fstream>
#include <csignal>

Webserver* webServerInstance = nullptr;

// Signal handler for CTRL+C
void signalHandler(int signum) {
    if (webServerInstance != nullptr) {
        std::cout << "[-] Shutdown Webserver" << std::endl << std::endl;;
        webServerInstance->shutdownServer();
    }
    exit(signum);
}

// Implement the shutdownServer method
void Webserver::shutdownServer() {
    shutdown(server_fd, SHUT_RDWR);
}

Webserver::Webserver(int port, std::string directory){
  dir = directory;

  

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  server_fd = socket(AF_INET, SOCK_STREAM, 0);

  webServerInstance = this;

  signal(SIGINT, signalHandler);

  if(server_fd < 0){
    std::cerr << "[-] Socket failed" << std::endl;
    exit(EXIT_FAILURE);
  }

  int socket_binding = bind(server_fd, (struct sockaddr*)&address, sizeof(address));
  if(socket_binding < 0){
    std::cerr << "[-] Bind connection failed" << std::endl;
    exit(EXIT_FAILURE);
  }
  
}

void Webserver::init(){
  int new_socket, socket_listener, buffer_len;
  while(true){
    socket_listener = listen(server_fd, 3);
    if(socket_listener < 0){
      std::cerr << "[-] Listen to port failed" << std::endl;
      exit(EXIT_FAILURE);
    }
    
    new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&address_len);
    if(new_socket <0){
      std::cerr << "[-] Could not accept the connection" << std::endl;
      exit(EXIT_FAILURE);
    }

    buffer_len = read(new_socket, buffer, 1024);
    if(buffer_len == -1){
      std::cerr << "[-] Could not read the request buffer" << std::endl;
      exit(EXIT_FAILURE);
    }
    
    std::string request_path = getRequestPath(buffer);
    std::string path = dir + request_path;
    std::string body, content_type;
    int response_code;
    try{
      body = loadFile(path);
      content_type = getContentType(path);
      response_code = 200;
    }catch(...){
      body = "404 - Not found.";
      content_type = "text/plain";
      response_code = 404;
    }
    std::string payload = getHeader(content_type, body.length(), response_code) + body;
    send(new_socket, payload.c_str(), payload.length(), 0);
    printf("Debug: [Path: %s] [Content-Type: %s]\n", path.c_str(), content_type.c_str());
    printf("[%d] %s\n", response_code, request_path.c_str());
    close(new_socket);
  }
}

Webserver::~Webserver(){
  std::cout << "[-] Shutdown Webserver" << std::endl << std::endl;;
  webServerInstance->shutdownServer();
}


std::string Webserver::getHeader(std::string content_type, int body_len, int status_code){
  // TODO - Add Content-Length Header #5
  // TODO - Add Content-Encoding Header #6
  std::string header = 
    "HTTP/1.1 " + std::to_string(status_code) + "\r\n"
    "Content-Type: "+ content_type + "\r\n"
    "X-Frame-Options: DENY\r\n"
    "X-XSS-Protection: 0\r\n"
    "\r\n";
  return header;
}

std::string Webserver::getContentType(std::string path){
  std::string extension = "";
  size_t pos = path.find_last_of('.');
  if (pos != std::string::npos) {
    extension = path.substr(pos + 1);
  }  

  if(extension == "html")
    return "text/html";
  else if(extension == "css")
    return "text/css";
  else if(extension == "js")
    return "text/javascript";
  else if(extension == "png")
    return "image/png";
  else if(extension == "ico")
    return "image/x-icon";
  return "text/plain";
}

std::string Webserver::loadFile(std::string path){
  std::string content = "";
  std::ifstream file;
  file.open (path, std::ifstream::in);
  if(file.good()){
    content = std::string((std::istreambuf_iterator<char>(file) ), (std::istreambuf_iterator<char>()));
  }else{
    throw std::exception();
  }
  file.close();
  return content;
}

std::string Webserver::getRequestPath(std::string buffer){
  std::string path;
  int cnt = 0;
  for(size_t i = 0; i < buffer.length(); i++)
    if(buffer[i] == '\n')
      break;
    else
      cnt++;
  
  path = buffer.substr(4, cnt-14);

  if(path == "/")
    return "/index.html";
  return path;
}
