#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"

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
  all_passing &= 1;
  run_next_case();
  return 0;
}

int on_error(irc_parser *parser, const char *at, size_t len) {
  print_test_result(0);
  all_passing = 0;
  run_next_case();
  return 0;
}

void print_test_result(int result) {
  printf("%c", (result) ? '.' : 'x');
  if (current_case + 1 % 20 == 0) {
    printf("%c", '\n');
  }
  fflush(stdout);
}

void run_current_case() {
  if (current_case < sizeof(cases) / sizeof(irc_parser_test_case)) {
    result.param_len = 0;
    irc_parser_reset(&parser);
    irc_parser_execute( &parser
                      , cases[current_case].test
                      , strlen(cases[current_case].test) + 1
                      );
  } else {
    printf("\nDone\n");
  }
}

void run_next_case() {
  ++current_case;
  run_current_case();
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
  run_current_case();
  return !all_passing;
}
