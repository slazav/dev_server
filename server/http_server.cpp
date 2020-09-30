#include <iostream>
#include <fstream>
#include <string>
#include <arpa/inet.h>

#include "err/err.h"
#include "http_server.h"

// callback for getting GET arguments (appending them to Opt object)
int
AppendToOpt (void *cls,
      enum MHD_ValueKind kind,
      const char *key, const char *value){
  ((Opt*)cls)->put(key, value);
  return MHD_YES;
}


// callback for processing requests
int
ProcessRequest(void * cls,
      struct MHD_Connection * connection,
      const char * url,
      const char * method,
                    const char * version,
      const char * upload_data,
      size_t * upload_data_size,
                    void ** ptr) {


  //get connection number
  auto info = MHD_get_connection_info(
    connection, MHD_CONNECTION_INFO_SOCKET_CONTEXT);
  uint64_t cnum = *(uint64_t*)info->socket_context;

  static int dummy;
  if (0 != strcmp(method, "GET"))
    return MHD_NO; /* unexpected method */
  if (&dummy != *ptr)
    {
      /* The first time only the headers are valid,
         do not respond in the first round... */
      *ptr = &dummy;
      return MHD_YES;
    }
  if (0 != *upload_data_size)
    return MHD_NO; /* upload data in a GET!? */
  *ptr = NULL; /* clear context pointer */

  DevManager * dm = (DevManager*)cls;
  struct MHD_Response * response;
  int ret;
  try {
    Opt opts;
    MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, AppendToOpt, &opts);
    Log(3) << "conn:" << cnum << " process request: " << url;
    std::string msg = dm->run(url, opts, cnum);
    Log(3) << "conn:" << cnum << " answer: " << msg;
    response = MHD_create_response_from_buffer(
        msg.length(), (void*)msg.data(), MHD_RESPMEM_MUST_COPY);
    ret = MHD_queue_response(connection, 200, response);
    MHD_destroy_response(response);
  }
  catch (Err e) {
    Log(3) << "conn:" << cnum << " error: " << e.str();
    response = MHD_create_response_from_buffer(
        e.str().length(), (void*)e.str().data(), MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Error", e.str().c_str());
    ret = MHD_queue_response(connection, 400, response);
    MHD_destroy_response(response);
  }

  return ret;
}

// global connection counter
uint64_t cmax = 0;

// Callback for opening/closing a connection
void ConnFunc (void *cls,
               struct MHD_Connection *connection,
               void **socket_context,
               enum MHD_ConnectionNotificationCode toe){

  DevManager * dm = (DevManager*)cls;

  switch (toe){
  case MHD_CONNECTION_NOTIFY_STARTED:
    MHD_set_connection_option(connection, MHD_CONNECTION_OPTION_TIMEOUT, 10);
    *socket_context = new uint64_t;
    *(uint64_t*)*socket_context = cmax++; // set connection number
    dm->conn_open(*(uint64_t*)*socket_context);
    break;
  case MHD_CONNECTION_NOTIFY_CLOSED:
    dm->conn_close(*(uint64_t*)*socket_context);
    delete (uint64_t*)*socket_context;
    break;
  }
}

HTTP_Server::HTTP_Server(
      const std::string & addr,
      const int port,
      const bool test,
      DevManager * dm) {

  // create option structure
  std::vector<struct MHD_OptionItem> ops;

  // server flags
  int flags = MHD_USE_THREAD_PER_CONNECTION;

  // notifications about opening/closing connections
  ops.push_back((MHD_OptionItem)
    {MHD_OPTION_NOTIFY_CONNECTION, (intptr_t)&ConnFunc, dm});

  // listen only one address
  if (addr!="*"){
    // fill sockaddr_in structure
    struct sockaddr_in sock;
    memset (&sock, 0, sizeof (struct sockaddr_in));
    sock.sin_family = AF_INET;
    sock.sin_port = htons(port);
    sock.sin_addr.s_addr = htonl(str_to_type_ip4(addr));

    ops.push_back((MHD_OptionItem)
      { MHD_OPTION_SOCK_ADDR, (intptr_t)&sock, NULL });
  }

  // test mode - only one connection at a time, to
  // have reproducible logs
  if (test){
    ops.push_back((MHD_OptionItem)
      { MHD_OPTION_CONNECTION_LIMIT, 1, NULL });
    flags |= MHD_USE_ITC;
  }

  // terminating option
  ops.push_back((MHD_OptionItem)
      { MHD_OPTION_END, 0, NULL });

  d = MHD_start_daemon(
      flags, port, NULL, NULL, &ProcessRequest, dm,
      MHD_OPTION_ARRAY, ops.data(),
      MHD_OPTION_END);

  if (d == NULL)
    throw Err() << "Can't start http server at " << addr << ":" << port;
}

HTTP_Server::~HTTP_Server(){
  MHD_stop_daemon((MHD_Daemon*)d);
}


