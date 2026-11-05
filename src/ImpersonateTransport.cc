// Copyright (C) 2026 aria2 contributors
// SPDX-License-Identifier: GPL-2.0-or-later
#include "ImpersonateTransport.h"

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <curl/curl.h>

#include "DlAbortEx.h"
#include "DlRetryEx.h"
#include "Option.h"
#include "Request.h"
#include "SocketCore.h"
#include "prefs.h"

// Also permits building against standard libcurl headers. configure checks
// that the linked library exports this additional API.
extern "C" CURLcode curl_easy_impersonate(CURL*, const char*, int);

namespace aria2 {

struct ImpersonateTransport::State {
  std::atomic<bool> cancelled{false};
  mutable std::mutex mutex;
  std::string error;
  CURLcode errorCode = CURLE_OK;

  void fail(const std::string& message, CURLcode code = CURLE_RECV_ERROR)
  {
    std::lock_guard<std::mutex> lock(mutex);
    if (error.empty()) {
      error = message;
      errorCode = code;
    }
  }
};

namespace {

struct Fd {
  int value;
  explicit Fd(int value) : value(value) {}
  ~Fd()
  {
    if (value >= 0) {
      shutdown(value, SHUT_RDWR);
      close(value);
    }
  }
  int release()
  {
    int result = value;
    value = -1;
    return result;
  }
  Fd(const Fd&) = delete;
  Fd& operator=(const Fd&) = delete;
  Fd(Fd&& other) : value(other.release()) {}
};

std::string lower(std::string s)
{
  for (auto& c : s) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return s;
}

std::string trim(const std::string& s)
{
  auto first = s.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) {
    return {};
  }
  return s.substr(first, s.find_last_not_of(" \t\r\n") - first + 1);
}

std::string headerName(const std::string& line)
{
  return lower(trim(line.substr(0, line.find(':'))));
}

bool hopByHop(const std::string& name)
{
  return name == "connection" || name == "proxy-connection" ||
         name == "keep-alive" || name == "transfer-encoding" ||
         name == "te" || name == "trailer" || name == "upgrade" ||
         name == "proxy-authorization" || name == "proxy-authenticate";
}

void check(CURLcode code)
{
  if (code != CURLE_OK) {
    throw std::runtime_error(curl_easy_strerror(code));
  }
}

class Worker {
  Fd fd_;
  std::shared_ptr<ImpersonateTransport::State> state_;
  std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl_;
  curl_slist* requestHeaders_ = nullptr;
  std::vector<std::string> responseHeaders_;
  size_t responseHeaderSize_ = 0;
  int status_ = 0;
  bool head_ = false;
  bool finalHeaders_ = false;
  bool chunked_ = false;

  bool sendAll(const char* data, size_t size)
  {
    while (size && !state_->cancelled.load()) {
#ifdef MSG_NOSIGNAL
      auto n = send(fd_.value, data, size, MSG_NOSIGNAL);
#else
      auto n = send(fd_.value, data, size, 0);
#endif
      if (n < 0 && errno == EINTR) {
        continue;
      }
      if (n <= 0) {
        return false;
      }
      data += n;
      size -= static_cast<size_t>(n);
    }
    return size == 0;
  }

  bool sendAll(const std::string& s) { return sendAll(s.data(), s.size()); }

  void appendHeader(const std::string& line)
  {
    auto list = curl_slist_append(requestHeaders_, line.c_str());
    if (!list) {
      throw std::bad_alloc();
    }
    requestHeaders_ = list;
  }

  void readRequest()
  {
    std::string request;
    char buffer[4096];
    while (request.find("\r\n\r\n") == std::string::npos) {
      auto n = recv(fd_.value, buffer, sizeof(buffer), 0);
      if (n < 0 && errno == EINTR) {
        continue;
      }
      if (n <= 0) {
        throw std::runtime_error("Request cancelled before headers arrived");
      }
      request.append(buffer, static_cast<size_t>(n));
      if (request.size() > 1024 * 1024) {
        throw std::runtime_error("HTTP request headers exceed 1 MiB");
      }
    }
    auto pos = request.find("\r\n");
    auto method = request.substr(0, request.find(' '));
    if (method != "GET" && method != "HEAD") {
      throw std::runtime_error("Impersonation supports GET and HEAD only");
    }
    head_ = method == "HEAD";
    check(curl_easy_setopt(curl_.get(), CURLOPT_NOBODY, head_ ? 1L : 0L));

    bool encoding = false;
    while (pos != std::string::npos) {
      auto next = request.find("\r\n", pos + 2);
      if (next == std::string::npos || next == pos + 2) {
        break;
      }
      auto line = request.substr(pos + 2, next - pos - 2);
      pos = next;
      auto name = headerName(line);
      if (hopByHop(name)) {
        continue;
      }
      // Use the profile's UA unless the caller explicitly customized it.
      if (name == "user-agent" &&
          trim(line.substr(line.find(':') + 1)) == "aria2/" PACKAGE_VERSION) {
        continue;
      }
      encoding = encoding || name == "accept-encoding";
      appendHeader(line);
    }
    // Browser profiles advertise br/zstd, which aria2 cannot decode. Keep
    // representation bytes stable for ranges, unless aria2 requested gzip.
    if (!encoding) {
      appendHeader("Accept-Encoding: identity");
    }
    check(curl_easy_setopt(curl_.get(), CURLOPT_HTTPHEADER, requestHeaders_));
  }

  bool receiveHeader(const char* data, size_t size)
  {
    std::string line(data, size);
    // Trailers belong to a completed header block, not a second response.
    if (finalHeaders_) {
      return true;
    }
    responseHeaderSize_ += size;
    if (responseHeaderSize_ > 1024 * 1024) {
      throw std::runtime_error("HTTP response headers exceed 1 MiB");
    }
    if (line.compare(0, 5, "HTTP/") == 0) {
      std::istringstream in(line);
      std::string version;
      in >> version >> status_;
      responseHeaders_.clear();
      return true;
    }
    if (line != "\r\n" && line != "\n") {
      responseHeaders_.push_back(trim(line));
      return true;
    }
    if (status_ >= 100 && status_ < 200) {
      if (status_ == 101) {
        throw std::runtime_error("Protocol upgrades are not supported");
      }
      return true; // libcurl handles informational responses.
    }
    if (status_ < 200 || status_ > 599) {
      throw std::runtime_error("Invalid HTTP response status");
    }

    bool hasLength = false;
    std::string headers =
        "HTTP/1.1 " + std::to_string(status_) + " Response\r\n";
    for (const auto& h : responseHeaders_) {
      auto name = headerName(h);
      if (!hopByHop(name)) {
        hasLength = hasLength || name == "content-length";
        headers += h + "\r\n";
      }
    }
    chunked_ = !hasLength && !head_ && status_ != 204 && status_ != 304;
    if (chunked_) {
      headers += "Transfer-Encoding: chunked\r\n";
    }
    // Never pool the local socket as an origin connection. The next range
    // gets a new adapter, while aria2 retains its own scheduling and state.
    headers += "Connection: close\r\n\r\n";
    finalHeaders_ = true;
    return sendAll(headers);
  }

  static size_t headerCallback(char* data, size_t size, size_t count, void* p)
  {
    auto self = static_cast<Worker*>(p);
    try {
      auto n = size * count;
      return self->receiveHeader(data, n) ? n : 0;
    }
    catch (const std::exception& e) {
      self->state_->fail(e.what());
      return 0;
    }
  }

  static size_t bodyCallback(char* data, size_t size, size_t count, void* p)
  {
    auto self = static_cast<Worker*>(p);
    auto n = size * count;
    if (n == 0) {
      return 0;
    }
    if (!self->finalHeaders_) {
      return 0;
    }
    if (self->chunked_) {
      char prefix[32];
      int len = std::snprintf(prefix, sizeof(prefix), "%zx\r\n", n);
      if (!self->sendAll(prefix, static_cast<size_t>(len)) ||
          !self->sendAll(data, n) || !self->sendAll("\r\n", 2)) {
        return 0;
      }
    }
    else if (!self->sendAll(data, n)) {
      return 0;
    }
    return n;
  }

  static int progressCallback(void* p, curl_off_t, curl_off_t, curl_off_t,
                              curl_off_t)
  {
    return static_cast<Worker*>(p)->state_->cancelled.load() ? 1 : 0;
  }

public:
  Worker(Fd fd, std::shared_ptr<ImpersonateTransport::State> state,
         const Request& request, const Option& option, const Request* proxy,
         bool proxyTunnel)
      : fd_(std::move(fd)), state_(std::move(state)),
        curl_(curl_easy_init(), curl_easy_cleanup)
  {
    if (!curl_) {
      throw std::runtime_error("Could not allocate a libcurl handle");
    }
    auto profile = option.get(PREF_IMPERSONATE);
    if (profile == "chrome") {
      profile = "chrome136";
    }
    if (profile.compare(0, 6, "chrome") != 0 ||
        curl_easy_impersonate(curl_.get(), profile.c_str(), 1) != CURLE_OK) {
      throw std::runtime_error("Unsupported Chrome profile: " + profile);
    }

    check(curl_easy_setopt(curl_.get(), CURLOPT_URL,
                          request.getCurrentUri().c_str()));
    check(curl_easy_setopt(curl_.get(), CURLOPT_NOSIGNAL, 1L));
    check(curl_easy_setopt(curl_.get(), CURLOPT_FOLLOWLOCATION, 0L));
    check(curl_easy_setopt(curl_.get(), CURLOPT_FAILONERROR, 0L));
    check(curl_easy_setopt(curl_.get(), CURLOPT_HTTP_CONTENT_DECODING, 0L));
    check(curl_easy_setopt(curl_.get(), CURLOPT_SUPPRESS_CONNECT_HEADERS, 1L));
    check(curl_easy_setopt(curl_.get(), CURLOPT_CONNECTTIMEOUT,
                          static_cast<long>(option.getAsInt(PREF_CONNECT_TIMEOUT))));
    auto verify = option.getAsBool(PREF_CHECK_CERTIFICATE);
    check(curl_easy_setopt(curl_.get(), CURLOPT_SSL_VERIFYPEER, verify ? 1L : 0L));
    check(curl_easy_setopt(curl_.get(), CURLOPT_SSL_VERIFYHOST, verify ? 2L : 0L));
    const auto& minTLS = option.get(PREF_MIN_TLS_VERSION);
    if (!minTLS.empty()) {
      long version = CURL_SSLVERSION_TLSv1_2;
      if (minTLS == A2_V_TLS13) {
        version = CURL_SSLVERSION_TLSv1_3;
      }
      else if (minTLS == A2_V_TLS11) {
        version = CURL_SSLVERSION_TLSv1_1;
      }
      check(curl_easy_setopt(curl_.get(), CURLOPT_SSLVERSION, version));
    }
    auto ca = option.get(PREF_CA_CERTIFICATE);
#ifdef CA_BUNDLE
    if (ca.empty()) {
      ca = CA_BUNDLE;
    }
#endif
    if (!ca.empty()) {
      check(curl_easy_setopt(curl_.get(), CURLOPT_CAINFO, ca.c_str()));
    }
    if (!option.get(PREF_CERTIFICATE).empty()) {
      check(curl_easy_setopt(curl_.get(), CURLOPT_SSLCERT,
                            option.get(PREF_CERTIFICATE).c_str()));
    }
    if (!option.get(PREF_PRIVATE_KEY).empty()) {
      check(curl_easy_setopt(curl_.get(), CURLOPT_SSLKEY,
                            option.get(PREF_PRIVATE_KEY).c_str()));
    }
    if (option.getAsBool(PREF_DISABLE_IPV6)) {
      check(curl_easy_setopt(curl_.get(), CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4));
    }
    if (!option.get(PREF_INTERFACE).empty()) {
      check(curl_easy_setopt(curl_.get(), CURLOPT_INTERFACE,
                            option.get(PREF_INTERFACE).c_str()));
    }
    if (!option.get(PREF_ASYNC_DNS_SERVER).empty()) {
      check(curl_easy_setopt(curl_.get(), CURLOPT_DNS_SERVERS,
                            option.get(PREF_ASYNC_DNS_SERVER).c_str()));
    }
    // Proxy/no-proxy selection has already been performed by aria2.
    // Do not allow libcurl's environment processing to override it.
    check(curl_easy_setopt(curl_.get(), CURLOPT_PROXY,
                          proxy ? proxy->getCurrentUri().c_str() : ""));
    check(curl_easy_setopt(curl_.get(), CURLOPT_NOPROXY, ""));
    if (proxy) {
      check(curl_easy_setopt(curl_.get(), CURLOPT_PROXYUSERNAME,
                            proxy->getUsername().c_str()));
      check(curl_easy_setopt(curl_.get(), CURLOPT_PROXYPASSWORD,
                            proxy->getPassword().c_str()));
      check(curl_easy_setopt(curl_.get(), CURLOPT_HTTPPROXYTUNNEL,
                            proxyTunnel ? 1L : 0L));
    }
    check(curl_easy_setopt(curl_.get(), CURLOPT_HEADERFUNCTION, headerCallback));
    check(curl_easy_setopt(curl_.get(), CURLOPT_HEADERDATA, this));
    check(curl_easy_setopt(curl_.get(), CURLOPT_WRITEFUNCTION, bodyCallback));
    check(curl_easy_setopt(curl_.get(), CURLOPT_WRITEDATA, this));
    check(curl_easy_setopt(curl_.get(), CURLOPT_XFERINFOFUNCTION, progressCallback));
    check(curl_easy_setopt(curl_.get(), CURLOPT_XFERINFODATA, this));
    check(curl_easy_setopt(curl_.get(), CURLOPT_NOPROGRESS, 0L));
  }

  ~Worker() { curl_slist_free_all(requestHeaders_); }

  void run() noexcept
  {
    try {
      readRequest();
      auto code = curl_easy_perform(curl_.get());
      if (code != CURLE_OK) {
        if (!state_->cancelled.load()) {
          state_->fail(curl_easy_strerror(code), code);
        }
      }
      else if (chunked_) {
        sendAll("0\r\n\r\n", 5);
      }
    }
    catch (const std::exception& e) {
      state_->fail(e.what());
    }
    catch (...) {
      state_->fail("Unexpected impersonation transport failure");
    }
    // EOF is delivered only after any error has been published.
    shutdown(fd_.value, SHUT_WR);
  }
};

} // namespace

ImpersonateTransport::ImpersonateTransport(std::shared_ptr<State> state)
    : state_(std::move(state))
{
}

ImpersonateTransport::~ImpersonateTransport() { cancel(); }

void ImpersonateTransport::cancel() { state_->cancelled.store(true); }

void ImpersonateTransport::checkError() const
{
  std::lock_guard<std::mutex> lock(state_->mutex);
  if (!state_->error.empty()) {
    auto message = "Impersonation transport: " + state_->error;
    if (state_->errorCode == CURLE_PEER_FAILED_VERIFICATION ||
        state_->errorCode == CURLE_SSL_CACERT_BADFILE) {
      throw DL_ABORT_EX(message);
    }
    throw DL_RETRY_EX(message);
  }
}

std::shared_ptr<SocketCore> ImpersonateTransport::createSocket(
    const Request& request, const Option& option, const Request* proxy,
    bool proxyTunnel)
{
  try {
    // Initialization is process-wide. Do not call global_cleanup while
    // cancelled workers (which hold no engine state) may still be exiting.
    static std::once_flag initialized;
    std::call_once(initialized, [] { check(curl_global_init(CURL_GLOBAL_DEFAULT)); });
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) {
      throw std::runtime_error(std::strerror(errno));
    }
    Fd client(fds[0]);
    Fd server(fds[1]);
    if (fcntl(client.value, F_SETFD, FD_CLOEXEC) == -1 ||
        fcntl(server.value, F_SETFD, FD_CLOEXEC) == -1) {
      throw std::runtime_error(std::strerror(errno));
    }
#ifdef SO_NOSIGPIPE
    int one = 1;
    setsockopt(server.value, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#endif
    auto state = std::make_shared<State>();
    auto socket = std::make_shared<SocketCore>(client.value, SOCK_STREAM);
    client.release();
    socket->setNonBlockingMode();
    socket->setImpersonateTransport(std::shared_ptr<ImpersonateTransport>(
        new ImpersonateTransport(state)));
    // Ownership remains exception-safe even if allocation/initialization fails.
    auto worker = std::make_shared<Worker>(std::move(server), state, request,
                                           option, proxy, proxyTunnel);
    // Bounded by aria2's connection limits. Socket backpressure provides
    // bounded streaming; cancellation never blocks the event loop on join.
    std::thread([worker] { worker->run(); }).detach();
    return socket;
  }
  catch (const std::exception& e) {
    throw DL_ABORT_EX(std::string("Cannot initialize impersonation: ") + e.what());
  }
}

} // namespace aria2
