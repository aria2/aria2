/*
 * Wslay - The WebSocket Library
 *
 * Copyright (c) 2011, 2012 Tatsuhiro Tsujikawa
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "wslay_event_test.h"

#include <assert.h>

#include <CUnit/CUnit.h>

#include "wslay_event.h"

struct scripted_data_feed {
  uint8_t data[8192];
  uint8_t* datamark;
  uint8_t* datalimit;
  size_t feedseq[8192];
  size_t seqidx;
};

struct accumulator {
  uint8_t buf[4096];
  size_t length;
};

struct my_user_data {
  struct scripted_data_feed *df;
  struct accumulator *acc;
};

static void scripted_data_feed_init(struct scripted_data_feed *df,
                                    const uint8_t *data, size_t data_length)
{
  memset(df, 0, sizeof(struct scripted_data_feed));
  memcpy(df->data, data, data_length);
  df->datamark = df->data;
  df->datalimit = df->data+data_length;
  df->feedseq[0] = data_length;
}

static ssize_t scripted_read_callback
(wslay_event_context_ptr ctx,
 uint8_t *data, size_t len, const union wslay_event_msg_source *source,
 int *eof, void *user_data)
{
  struct scripted_data_feed *df = (struct scripted_data_feed*)source->data;
  size_t wlen = df->feedseq[df->seqidx] > len ? len : df->feedseq[df->seqidx];
  memcpy(data, df->datamark, wlen);
  df->datamark += wlen;
  if(wlen <= len) {
    ++df->seqidx;
  } else {
    df->feedseq[df->seqidx] -= wlen;
  }
  if(df->datamark == df->datalimit) {
    *eof = 1;
  }
  return wlen;
}

static ssize_t scripted_recv_callback(wslay_event_context_ptr ctx,
                                      uint8_t* data, size_t len, int flags,
                                      void *user_data)
{
  struct scripted_data_feed *df = ((struct my_user_data*)user_data)->df;
  size_t wlen = df->feedseq[df->seqidx] > len ? len : df->feedseq[df->seqidx];
  memcpy(data, df->datamark, wlen);
  df->datamark += wlen;
  if(wlen <= len) {
    ++df->seqidx;
  } else {
    df->feedseq[df->seqidx] -= wlen;
  }
  return wlen;
}

static ssize_t accumulator_send_callback(wslay_event_context_ptr ctx,
                                         const uint8_t *buf, size_t len,
                                         int flags, void* user_data)
{
  struct accumulator *acc = ((struct my_user_data*)user_data)->acc;
  assert(acc->length+len < sizeof(acc->buf));
  memcpy(acc->buf+acc->length, buf, len);
  acc->length += len;
  return len;
}

static ssize_t one_accumulator_send_callback(wslay_event_context_ptr ctx,
                                             const uint8_t *buf, size_t len,
                                             int flags, void* user_data)
{
  struct accumulator *acc = ((struct my_user_data*)user_data)->acc;
  assert(len > 0);
  memcpy(acc->buf+acc->length, buf, 1);
  acc->length += 1;
  return 1;
}

static ssize_t fail_recv_callback(wslay_event_context_ptr ctx,
                                  uint8_t* data, size_t len, int flags,
                                  void *user_data)
{
  wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
  return -1;
}

static ssize_t fail_send_callback(wslay_event_context_ptr ctx,
                                  const uint8_t *buf, size_t len, int flags,
                                  void* user_data)
{
  wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
  return -1;
}


void test_wslay_event_send_fragmented_msg(void)
{
  wslay_event_context_ptr ctx;
  struct wslay_event_callbacks callbacks;
  struct my_user_data ud;
  struct accumulator acc;
  const char msg[] = "Hello";
  struct scripted_data_feed df;
  struct wslay_event_fragmented_msg arg;
  const uint8_t ans[] = {
    0x01, 0x03, 0x48, 0x65, 0x6c,
    0x80, 0x02, 0x6c, 0x6f
  };
  scripted_data_feed_init(&df, (const uint8_t*)msg, sizeof(msg)-1);
  df.feedseq[0] = 3;
  df.feedseq[1] = 2;
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.send_callback = accumulator_send_callback;
  memset(&acc, 0, sizeof(acc));
  ud.acc = &acc;
  wslay_event_context_server_init(&ctx, &callbacks, &ud);

  memset(&arg, 0, sizeof(arg));
  arg.opcode = WSLAY_TEXT_FRAME;
  arg.source.data = &df;
  arg.read_callback = scripted_read_callback;
  CU_ASSERT(0 == wslay_event_queue_fragmented_msg(ctx, &arg));
  CU_ASSERT(0 == wslay_event_send(ctx));
  CU_ASSERT_EQUAL(9, acc.length);
  CU_ASSERT(0 == memcmp(ans, acc.buf, acc.length));
  wslay_event_context_free(ctx);
}

void test_wslay_event_send_fragmented_msg_with_ctrl(void)
{
  int i;
  wslay_event_context_ptr ctx;
  struct wslay_event_callbacks callbacks;
  struct my_user_data ud;
  struct accumulator acc;
  const char msg[] = "Hello";
  struct scripted_data_feed df;
  struct wslay_event_fragmented_msg arg;
  struct wslay_event_msg ctrl_arg;
  const uint8_t ans[] = {
    0x01, 0x03, 0x48, 0x65, 0x6c, /* "Hel" */
    0x89, 0x00, /* unmasked ping */
    0x80, 0x02, 0x6c, 0x6f /* "lo" */
  };
  scripted_data_feed_init(&df, (const uint8_t*)msg, sizeof(msg)-1);
  df.feedseq[0] = 3;
  df.feedseq[1] = 2;
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.send_callback = one_accumulator_send_callback;
  memset(&acc, 0, sizeof(acc));
  ud.acc = &acc;
  wslay_event_context_server_init(&ctx, &callbacks, &ud);
  
  memset(&arg, 0, sizeof(arg));
  arg.opcode = WSLAY_TEXT_FRAME;
  arg.source.data = &df;
  arg.read_callback = scripted_read_callback;
  CU_ASSERT(0 == wslay_event_queue_fragmented_msg(ctx, &arg));
  CU_ASSERT(1 == wslay_event_get_queued_msg_count(ctx));
  CU_ASSERT(0 == wslay_event_get_queued_msg_length(ctx));
  CU_ASSERT(0 == wslay_event_send(ctx));

  memset(&ctrl_arg, 0, sizeof(ctrl_arg));
  ctrl_arg.opcode = WSLAY_PING;
  ctrl_arg.msg_length = 0;
  CU_ASSERT(0 == wslay_event_queue_msg(ctx, &ctrl_arg));
  CU_ASSERT(2 == wslay_event_get_queued_msg_count(ctx));
  for(i = 0; i < 10; ++i) {
    CU_ASSERT(0 == wslay_event_send(ctx));
  }
  CU_ASSERT(0 == wslay_event_get_queued_msg_count(ctx));
  CU_ASSERT(11 == acc.length);
  CU_ASSERT(0 == memcmp(ans, acc.buf, acc.length));
  wslay_event_context_free(ctx);
}

void test_wslay_event_send_ctrl_msg_first(void)
{
  wslay_event_context_ptr ctx;
  struct wslay_event_callbacks callbacks;
  struct my_user_data ud;
  struct accumulator acc;
  const char msg[] = "Hello";
  struct wslay_event_msg arg;
  const uint8_t ans[] = {
    0x89, 0x00, /* unmasked ping */
    0x81, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f /* "Hello" */
  };
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.send_callback = accumulator_send_callback;
  memset(&acc, 0, sizeof(acc));
  ud.acc = &acc;
  wslay_event_context_server_init(&ctx, &callbacks, &ud);
  
  memset(&arg, 0, sizeof(arg));
  arg.opcode = WSLAY_PING;
  arg.msg_length = 0;
  CU_ASSERT(0 == wslay_event_queue_msg(ctx, &arg));
  arg.opcode = WSLAY_TEXT_FRAME;
  arg.msg = (const uint8_t*)msg;
  arg.msg_length = 5;
  CU_ASSERT(0 == wslay_event_queue_msg(ctx, &arg));
  CU_ASSERT(0 == wslay_event_send(ctx));
  CU_ASSERT(9 == acc.length);
  CU_ASSERT(0 == memcmp(ans, acc.buf, acc.length));
  wslay_event_context_free(ctx);
}

void test_wslay_event_queue_close(void)
{
  wslay_event_context_ptr ctx;
  struct wslay_event_callbacks callbacks;
  struct my_user_data ud;
  struct accumulator acc;
  const char msg[] = "H";
  const uint8_t ans[] = {
    0x88, 0x03, 0x03, 0xf1, 0x48 /* "H" */
  };
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.send_callback = accumulator_send_callback;
  memset(&acc, 0, sizeof(acc));
  ud.acc = &acc;
  wslay_event_context_server_init(&ctx, &callbacks, &ud);
  CU_ASSERT(0 == wslay_event_queue_close(ctx, WSLAY_CODE_MESSAGE_TOO_BIG,
                                         (const uint8_t*)msg, 1));
  CU_ASSERT(0 == wslay_event_send(ctx));
  CU_ASSERT(5 == acc.length);
  CU_ASSERT(0 == memcmp(ans, acc.buf, acc.length));
  CU_ASSERT(1 == wslay_event_get_close_sent(ctx));
  wslay_event_context_free(ctx);
}

void test_wslay_event_queue_close_without_code(void)
{
  wslay_event_context_ptr ctx;
  struct wslay_event_callbacks callbacks;
  struct my_user_data ud;
  struct accumulator acc;
  const uint8_t ans[] = { 0x88, 0x00 };
  struct wslay_event_msg ping = { WSLAY_PING, NULL, 0 };
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.send_callback = accumulator_send_callback;
  memset(&acc, 0, sizeof(acc));
  ud.acc = &acc;
  wslay_event_context_server_init(&ctx, &callbacks, &ud);
  CU_ASSERT(0 == wslay_event_queue_msg(ctx, &ping));
  /* See that ping is not sent because close frame is queued */
  CU_ASSERT(0 == wslay_event_queue_close(ctx, 0, NULL, 0));
  CU_ASSERT(0 == wslay_event_send(ctx));
  CU_ASSERT(2 == acc.length);
  CU_ASSERT(0 == memcmp(ans, acc.buf, acc.length));
  CU_ASSERT(1 == wslay_event_get_close_sent(ctx));
  CU_ASSERT(WSLAY_CODE_NO_STATUS_RCVD ==
            wslay_event_get_status_code_sent(ctx));
  wslay_event_context_free(ctx);
}

void test_wslay_event_recv_close_without_code(void)
{
  wslay_event_context_ptr ctx;
  struct wslay_event_callbacks callbacks;
  struct my_user_data ud;
  const uint8_t msg[] = { 0x88u, 0x00 };
  struct scripted_data_feed df;
  scripted_data_feed_init(&df, (const uint8_t*)msg, sizeof(msg));
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.recv_callback = scripted_recv_callback;
  ud.df = &df;
  wslay_event_context_client_init(&ctx, &callbacks, &ud);
  CU_ASSERT(0 == wslay_event_recv(ctx));
  CU_ASSERT(1 == wslay_event_get_close_received(ctx));
  CU_ASSERT(WSLAY_CODE_NO_STATUS_RCVD ==
            wslay_event_get_status_code_received(ctx));
  wslay_event_context_free(ctx);
}

void test_wslay_event_reply_close(void)
{
  wslay_event_context_ptr ctx;
  struct wslay_event_callbacks callbacks;
  struct my_user_data ud;
  struct accumulator acc;
  /* Masked close frame with code = 1009, reason = "Hello" */
  const uint8_t msg[] = { 0x88u, 0x87u, 0x00u, 0x00u, 0x00u, 0x00u,
                          0x03, 0xf1, /* 1009 */
                          0x48, 0x65, 0x6c, 0x6c, 0x6f /* "Hello" */
  };
  const uint8_t ans[] = { 0x88u, 0x07u,
                          0x03, 0xf1, /* 1009 */
                          0x48, 0x65, 0x6c, 0x6c, 0x6f /* "Hello" */
  };
  struct scripted_data_feed df;
  scripted_data_feed_init(&df, (const uint8_t*)msg, sizeof(msg));
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.send_callback = accumulator_send_callback;
  callbacks.recv_callback = scripted_recv_callback;
  memset(&acc, 0, sizeof(acc));
  ud.df = &df;
  ud.acc = &acc;
  wslay_event_context_server_init(&ctx, &callbacks, &ud);
  CU_ASSERT(0 == wslay_event_recv(ctx));
  CU_ASSERT(1 == wslay_event_get_queued_msg_count(ctx));
  /* 7 bytes = 2 bytes status code + "Hello" */
  CU_ASSERT(7 == wslay_event_get_queued_msg_length(ctx));
  CU_ASSERT(1 == wslay_event_get_close_received(ctx));
  CU_ASSERT(WSLAY_CODE_MESSAGE_TOO_BIG ==
            wslay_event_get_status_code_received(ctx));
  CU_ASSERT(WSLAY_CODE_ABNORMAL_CLOSURE ==
            wslay_event_get_status_code_sent(ctx));
  CU_ASSERT(0 == wslay_event_send(ctx));
  CU_ASSERT(0 == wslay_event_get_queued_msg_count(ctx));
  CU_ASSERT(0 == wslay_event_get_queued_msg_length(ctx));
  CU_ASSERT(9 == acc.length);
  CU_ASSERT(0 == memcmp(ans, acc.buf, acc.length));
  CU_ASSERT(1 == wslay_event_get_close_sent(ctx));
  CU_ASSERT(WSLAY_CODE_MESSAGE_TOO_BIG ==
            wslay_event_get_status_code_received(ctx));
  CU_ASSERT(WSLAY_CODE_MESSAGE_TOO_BIG ==
            wslay_event_get_status_code_sent(ctx));
  wslay_event_context_free(ctx);
}

void test_wslay_event_no_more_msg(void)
{
  wslay_event_context_ptr ctx;
  struct wslay_event_callbacks callbacks;
  memset(&callbacks, 0, sizeof(callbacks));
  wslay_event_context_server_init(&ctx, &callbacks, NULL);
  CU_ASSERT(0 == wslay_event_queue_close(ctx, 0, NULL, 0));
  CU_ASSERT(WSLAY_ERR_NO_MORE_MSG == wslay_event_queue_close(ctx, 0, NULL, 0));
  wslay_event_context_free(ctx);
}

void test_wslay_event_callback_failure(void)
{
  wslay_event_context_ptr ctx;
  struct wslay_event_callbacks callbacks;
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.recv_callback = fail_recv_callback;
  callbacks.send_callback = fail_send_callback;
  wslay_event_context_server_init(&ctx, &callbacks, NULL);
  CU_ASSERT(WSLAY_ERR_CALLBACK_FAILURE == wslay_event_recv(ctx));
  /* close control frame is in queue */
  CU_ASSERT(WSLAY_ERR_CALLBACK_FAILURE == wslay_event_send(ctx));
  wslay_event_context_free(ctx);
}

static void no_buffering_callback(wslay_event_context_ptr ctx,
                                  const struct wslay_event_on_msg_recv_arg *arg,
                                  void *user_data)
{
  if(arg->opcode == WSLAY_PING) {
    CU_ASSERT(3 == arg->msg_length);
    CU_ASSERT(0 == memcmp("Foo", arg->msg, arg->msg_length));
  } else {
    CU_ASSERT(WSLAY_TEXT_FRAME == arg->opcode);
    CU_ASSERT(0 == arg->msg_length);
  }
}

void test_wslay_event_no_buffering(void)
{
  wslay_event_context_ptr ctx;
  struct wslay_event_callbacks callbacks;
  struct my_user_data ud;
  const uint8_t msg[] = {
    0x01, 0x03, 0x48, 0x65, 0x6c, /* "Hel" */
    0x89, 0x03, 0x46, 0x6f, 0x6f, /* ping with "Foo" */
    0x80, 0x02, 0x6c, 0x6f, /* "lo" */
  };
  struct scripted_data_feed df;
  scripted_data_feed_init(&df, (const uint8_t*)msg, sizeof(msg));
  memset(&callbacks, 0, sizeof(callbacks));
  ud.df = &df;
  callbacks.recv_callback = scripted_recv_callback;
  callbacks.on_msg_recv_callback = no_buffering_callback;
  wslay_event_context_client_init(&ctx, &callbacks, &ud);
  wslay_event_config_set_no_buffering(ctx, 1);
  CU_ASSERT(0 == wslay_event_recv(ctx));
  /* pong must be queued */
  CU_ASSERT(wslay_event_want_write(ctx));
  wslay_event_context_free(ctx);
}

void test_wslay_event_frame_too_big(void)
{
  wslay_event_context_ptr ctx;
  struct wslay_event_callbacks callbacks;
  struct my_user_data ud;
  struct accumulator acc;
  /* Masked text frame */
  const uint8_t msg[] = { 0x81, 0x85, 0x00, 0x00, 0x00, 0x00,
                          0x48, 0x65, 0x6c, 0x6c, 0x6f /* "Hello" */
  };
  const uint8_t ans[] = { 0x88, 0x02,
                          0x03, 0xf1 /* 1009 */
  };
  struct scripted_data_feed df;
  scripted_data_feed_init(&df, (const uint8_t*)msg, sizeof(msg));
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.send_callback = accumulator_send_callback;
  callbacks.recv_callback = scripted_recv_callback;
  memset(&acc, 0, sizeof(acc));
  ud.df = &df;
  ud.acc = &acc;
  wslay_event_context_server_init(&ctx, &callbacks, &ud);
  wslay_event_config_set_max_recv_msg_length(ctx, 4);
  CU_ASSERT(0 == wslay_event_recv(ctx));
  CU_ASSERT(0 == wslay_event_send(ctx));
  CU_ASSERT(4 == acc.length);
  CU_ASSERT(0 == memcmp(ans, acc.buf, acc.length));
  CU_ASSERT(1 == wslay_event_get_close_sent(ctx));
  CU_ASSERT(WSLAY_CODE_MESSAGE_TOO_BIG ==
            wslay_event_get_status_code_sent(ctx));
  wslay_event_context_free(ctx);
}

void test_wslay_event_message_too_big(void)
{
  wslay_event_context_ptr ctx;
  struct wslay_event_callbacks callbacks;
  struct my_user_data ud;
  struct accumulator acc;
  /* Masked text 2 frames */
  const uint8_t msg[] = { 0x01, 0x85, 0x00, 0x00, 0x00, 0x00,
                          0x48, 0x65, 0x6c, 0x6c, 0x6f, /* "Hello" */
                          0x80, 0x85, 0x00, 0x00, 0x00, 0x00,
                          0x48, 0x65, 0x6c, 0x6c, 0x6f /* "Hello" */
  };
  const uint8_t ans[] = { 0x88, 0x02,
                          0x03, 0xf1 /* 1009 */
  };
  struct scripted_data_feed df;
  scripted_data_feed_init(&df, (const uint8_t*)msg, sizeof(msg));
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.send_callback = accumulator_send_callback;
  callbacks.recv_callback = scripted_recv_callback;
  memset(&acc, 0, sizeof(acc));
  ud.df = &df;
  ud.acc = &acc;
  wslay_event_context_server_init(&ctx, &callbacks, &ud);
  wslay_event_config_set_max_recv_msg_length(ctx, 9);
  CU_ASSERT(0 == wslay_event_recv(ctx));
  CU_ASSERT(0 == wslay_event_send(ctx));
  CU_ASSERT(4 == acc.length);
  CU_ASSERT(0 == memcmp(ans, acc.buf, acc.length));
  CU_ASSERT(1 == wslay_event_get_close_sent(ctx));
  CU_ASSERT(WSLAY_CODE_MESSAGE_TOO_BIG ==
            wslay_event_get_status_code_sent(ctx));
  wslay_event_context_free(ctx);
}
