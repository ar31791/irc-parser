#ifndef irc_parser_test_h
#define irc_parser_test_h

#include "irc_parser.h"

typedef struct irc_parser_test_case_s irc_parser_test_case;
typedef struct irc_parser_test_result_s irc_parser_test_result;

struct irc_parser_test_case_s {
  const char *test;
  const char *nick;
  const char *name;
  const char *host;
  const char *command;
  const char *param;
};

struct irc_parser_test_result_s {
  const char *nick;
  size_t nick_len;
  const char *name;
  size_t name_len;
  const char *host;
  size_t host_len;
  const char *command;
  size_t command_len;
  const char *param;
  size_t param_len;
};

int on_nick(irc_parser *parser, const char *at, size_t len);
int on_name(irc_parser *parser, const char *at, size_t len);
int on_host(irc_parser *parser, const char *at, size_t len);
int on_command(irc_parser *parser, const char *at, size_t len);
int on_param(irc_parser *parser, const char *at, size_t len);
int on_end(irc_parser *parser, const char *at, size_t len);
int on_error(irc_parser *parser, const char *at, size_t len);

void print_expected_results();
void print_test_result(int result);
void next_case();

#endif
