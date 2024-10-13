# HTTP Server

Creating a simple HTTP Server in C, as a way to familiarize myself with low-level programming. 

# TODO:
- [ ] Implement HTTP 1.1
  - [ ] 'Keep-alive' - Make it so a single client-server connection can be supported by 1 client_socket, rather than every request requiring a new socket to be opened and subsequently closed
- [ ] Write tests
  - [ ] Research some tools for writing unit tests