#ifndef _D_REQUEST_TO_STRING_H_
#define _D_REQUEST_TO_STRING_H_
#include "Util.h"
#include "Request.h"
#include <string>

using namespace std;

class RequestToString {
private:
  Request* req;
public:
  
  RequestToString(Request* req) {
    this->req = req;
  }

  string toString() {
    return "url = "+req->url+"\n"+
      "protocol = "+req->protocol+"\n"+
      "host = "+req->host+"\n"+
      "port = "+Util::ulitos(req->port)+"\n"+
      "dir = "+req->dir+"\n"+
      "file = "+req->file;
  }

#endif // _D_REQUEST_TO_STRING_H_
