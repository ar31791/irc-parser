#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"

#define NUM_TESTS sizeof(cases) / sizeof(irc_parser_test_case)

const irc_parser_test_case cases[] = {
  { "PRIVMSG #test :hello world!\r\n"
  , NULL
  , NULL
  , NULL
  , "PRIVMSG"
  , "#test hello world!"
  },
  { ":lohkey!name@host PRIVMSG #test :hello test script!\r\n"
  , "lohkey"
  , "name"
  , "host"
  , "PRIVMSG"
  , "#test hello test script!"
  },
  { "PRIVMSG lohkey :boo!\r\n"
  , NULL
  , NULL
  , NULL
  , "PRIVMSG"
  , "lohkey boo!"
  },
  { ":lohkey!name@host NOTICE test :PM me again and ban hammer4u\r\n"
  , "lohkey"
  , "name"
  , "host"
  , "NOTICE"
  , "test PM me again and ban hammer4u"
  },
  { "NOTICE lohkey :what about notices?\r\n"
  , NULL
  , NULL
  , NULL
  , "NOTICE"
  , "lohkey what about notices?"
  },
  { "KICK #test test :for testing purposes\r\n"
  , NULL
  , NULL
  , NULL
  , "KICK"
  , "#test test for testing purposes"
  },
  { ":lohkey!name@host KICK #test test :for testing purposes\r\n"
  , "lohkey"
  , "name"
  , "host"
  , "KICK"
  , "#test test for testing purposes"
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
    for (int i = 0; i < NUM_TESTS; i++) {
      acc += strlen(cases[i].test);
    }
    return acc;
  }
}

void _build_input_buffer() {
  input = calloc(_get_total_input_size(), sizeof(char));
  for (int i = 0; i < NUM_TESTS; i++) {
    strcat(input, cases[i].test);
  }
}

int on_nick(irc_parser *parser, const char *at, size_t len) {
  result.nick = at;
  result.nick_len = len;
  return 0;
}

int on_name(irc_parser *parser, const char *at, size_t len) {
  result.name = at;
  result.name_len = len;
  return 0;
}

int on_host(irc_parser *parser, const char *at, size_t len) {
  result.host = at;
  result.host_len = len;
  return 0;
}

int on_command(irc_parser *parser, const char *at, size_t len) {
  result.command = at;
  result.command_len = len;
  return 0;
}

int on_param(irc_parser *parser, const char *at, size_t len) {
  memcpy(&param_buffer[result.param_len], at, len);
  result.param_len += len;
  return 0;
}

int on_end(irc_parser *parser, const char *at, size_t len) {
  print_test_result(1);
  next_case();
  all_passing &= 1;
  return 0;
}

int on_error(irc_parser *parser, const char *at, size_t len) {
  print_test_result(0);
  next_case();
  all_passing = 0;
  return 0;
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
  current_case = current_case + 1 % NUM_TESTS;
  irc_parser_reset(&parser);
}

void run_tests() {
  _build_input_buffer();
  for (int i = 1; i < _get_total_input_size(); i++) {
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

int main (int argc, char **argv) {
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
  printf("Running test suite\n");
  printf("==================\n");
  run_tests();
  printf("Finished with %s errors\n", (all_passing) ? "no" : "some");
  return !all_passing;
}
