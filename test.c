#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"

#define CMP_RESULTS(_val) do {                                                \
  if (strncmp(result._val, cases[current_case]._val, result._val_len) != 0) { \
    return 0;                                                                 \
  }                                                                           \
} while(0)

#define NUM_TESTS (sizeof(cases) / sizeof(irc_parser_test_case))

const irc_parser_test_case cases[] = {
  { "PRIVMSG #test :hello world!\r\n"
  , NULL
  , NULL
  , NULL
  , "PRIVMSG"
  , "#testhello world!"
  },
  { ":lohkey!name@host PRIVMSG #test :hello test script!\r\n"
  , "lohkey"
  , "name"
  , "host"
  , "PRIVMSG"
  , "#testhello test script!"
  },
  { "PRIVMSG lohkey :boo!\r\n"
  , NULL
  , NULL
  , NULL
  , "PRIVMSG"
  , "lohkeyboo!"
  },
  { ":lohkey!name@host NOTICE test :PM me again and ban hammer4u\r\n"
  , "lohkey"
  , "name"
  , "host"
  , "NOTICE"
  , "testPM me again and ban hammer4u"
  },
  { "NOTICE lohkey :what about notices?\r\n"
  , NULL
  , NULL
  , NULL
  , "NOTICE"
  , "lohkeywhat about notices?"
  },
  { "KICK #test test :for testing purposes\r\n"
  , NULL
  , NULL
  , NULL
  , "KICK"
  , "#testtestfor testing purposes"
  },
  { ":lohkey!name@host KICK #test test :for testing purposes\r\n"
  , "lohkey"
  , "name"
  , "host"
  , "KICK"
  , "#testtestfor testing purposes"
  }
};

// global state .... yup don't care right now
irc_parser_test_result result;
irc_parser parser;
irc_parser_settings settings;
char param_buffer[1024];
int current_case = 0;
int all_passing = 1;
int tests_ran = 0;
int input_size = 0;
char *input;

size_t _get_total_input_size() {
  if (input_size) {
    return input_size;
  } else {
    size_t acc = 0;
    for (size_t i = 0; i < NUM_TESTS; i++) {
      acc += strlen(cases[i].test);
    }
    return acc;
  }
}

void _build_input_buffer() {
  input = calloc(_get_total_input_size(), sizeof(char));
  for (size_t i = 0; i < NUM_TESTS; i++) {
    strcat(input, cases[i].test);
  }
}

int on_nick(irc_parser *parser, const char *at, size_t len) {
  irc_parser_test_result *res = parser->data;
  res->nick = at;
  res->nick_len = len;
  return 0;
}

int on_name(irc_parser *parser, const char *at, size_t len) {
  irc_parser_test_result *res = parser->data;
  res->name = at;
  res->name_len = len;
  return 0;
}

int on_host(irc_parser *parser, const char *at, size_t len) {
  irc_parser_test_result *res = parser->data;
  res->host = at;
  res->host_len = len;
  return 0;
}

int on_command(irc_parser *parser, const char *at, size_t len) {
  irc_parser_test_result *res = parser->data;
  res->command = at;
  res->command_len = len;
  return 0;
}

int on_param(irc_parser *parser, const char *at, size_t len) {
  irc_parser_test_result *res = parser->data;
  memcpy(&param_buffer[res->param_len], at, len);
  result.param = param_buffer;
  return 0;
}

int _passes_current_case() {
  const irc_parser_test_case *c = &cases[current_case];
  if ((result.nick == NULL || c->nick == NULL) && result.nick != c->nick) {
    return 0;
  }
  if (strncmp(result.nick, c->nick, result.nick_len) != 0) {
    return 0;
  }
  if ((result.name == NULL || c->name == NULL) && result.name != c->name) {
    return 0;
  }
  if (strncmp(result.name, c->name, result.name_len) != 0) {
    return 0;
  }
  if ((result.host == NULL || c->host == NULL) && result.host != c->host) {
    return 0;
  }
  if (strncmp(result.host, c->host, result.host_len) != 0) {
    return 0;
  }
  if ((result.command == NULL || c->command == NULL) && 
      result.command != c->command) {
    return 0;
  }
  if (strncmp(result.command, c->command, result.command_len) != 0) {
    return 0;
  }
  if ((result.param == NULL || c->param == NULL) && result.param != c->param) {
    return 0;
  }
  if (strncmp(result.param, c->param, result.param_len) != 0) {
    return 0;
  }
  return 1;
}

void _reset_results() {
  memset(&result, 0, sizeof(irc_parser_test_result));
}

int on_end(irc_parser *parser, const char *at, size_t len) {
  int passing = _passes_current_case();
  if (passing) {
    print_test_result(passing);
    _reset_results();
    next_case();
    all_passing &= 1;
  } else {
    on_error(parser, at, len);
  }
  return 0;
}

int on_error(irc_parser *parser, const char *at, size_t len) {
  irc_parser_test_result *res = parser->data;
  print_test_result(0);
  print_expected_results(res, at, len);
  _reset_results();
  next_case();
  all_passing = 0;
  return 0;
}

void print_expected_results(irc_parser_test_result *res, const char *at, 
                            size_t len) {
  const irc_parser_test_case *c_case = &cases[current_case];
  char nick[513], name[513], host[513], command[513], param[513];
  printf("At 0x%p(%zu)\n", at, len);
  printf("Expected: { raw: %s"
         "          , nick: %s\n"
         "          , name: %s\n"
         "          , host: %s\n"
         "          , command: %s\n"
         "          , param: %s\n"
         "          }\n"
         , c_case->test
         , c_case->nick
         , c_case->name
         , c_case->host
         , c_case->command
         , c_case->param
         );
  strncpy(nick, res->nick, res->nick_len);
  nick[res->nick_len] = '\0';
  strncpy(name, res->name, res->name_len);
  name[res->name_len] = '\0';
  strncpy(host, res->host, res->host_len);
  host[res->host_len] = '\0';
  strncpy(command, res->command, res->command_len);
  command[res->command_len] = '\0';
  strncpy(param, res->param, res->param_len);
  param[res->param_len] = '\0';
  printf("Got:      { raw: %s\n"
         "          , nick: %s\n"
         "          , name: %s\n"
         "          , host: %s\n"
         "          , command: %s\n"
         "          , param: %s\n"
         "          }\n"
         , parser.raw
         , (nick[0])    ? nick    : NULL
         , (name[0])    ? name    : NULL
         , (host[0])    ? host    : NULL
         , (command[0]) ? command : NULL
         , (param[0])   ? param   : NULL
         );
}

void print_test_result(int result) {
  printf("%c", (result) ? '.' : 'x');
  if (++tests_ran % 60 == 0) {
    printf("%c", '\n');
  }
  fflush(stdout);
}

void next_case() {
  result.param_len = 0;
  current_case = (current_case + 1) % NUM_TESTS;
  irc_parser_reset(&parser);
}

void run_tests() {
  _build_input_buffer();
  for (size_t i = 1; i < _get_total_input_size(); i++) {
    int full_peices = _get_total_input_size() / i;
    int remainding  = _get_total_input_size() % i;
    for (int j = 0; j < full_peices; j++) {
      irc_parser_execute(&parser, &input[i * j], i);   
    }
    if (remainding) {
      irc_parser_execute( &parser
                        , &input[_get_total_input_size() - remainding]
                        , remainding
                        );
    }
  }
  printf("\nDone\n");
}

int main () {
  irc_parser_settings_init( &settings
                          , on_nick
                          , on_name
                          , on_host
                          , on_command
                          , on_param
                          , on_end
                          , on_error
                          );
  irc_parser_init(&parser, &settings);
  parser.data = &result;
  printf("Running test suite\n");
  printf("==================\n");
  run_tests();
  printf("Finished with %s errors\n", (all_passing) ? "no" : "some");
  return !all_passing;
}
