#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // remove this eventually
#include <string.h>
#include "irc_parser.h"

#define IRC_PARSER_CALL_AND_PROGRESS_ON(_parser, _a, _b) do { \
  if (_a == _b) { _irc_parser_call_and_progress(parser); }    \
} while(0)

//// private
void _irc_parser_append_raw(irc_parser *parser, char c) {
  parser->raw[parser->len++] = c;
}

void _irc_parser_reinit(irc_parser *parser) {
  parser->len    = 0;
  parser->last   = 0;
  parser->state  = IRC_STATE_INIT;
  parser->raw[0] = '\0';  
}

irc_parser_cb _irc_parser_get_cb(irc_parser *parser) {
  switch(parser->state) {
  case IRC_STATE_INIT:
  case IRC_STATE_NICK:     return parser->on_nick;
  case IRC_STATE_NAME:     return parser->on_name;
  case IRC_STATE_HOST:     return parser->on_host;
  case IRC_STATE_COMMAND:  return parser->on_command;
  case IRC_STATE_PARAMS:   return parser->on_param;
  case IRC_STATE_TRAILING: return parser->on_param;
  case IRC_STATE_END:      return parser->on_param;
  case IRC_STATE_ERROR:
  default:                 return NULL;
  }
}

enum irc_parser_state _irc_get_next_state(irc_parser *parser) {
  switch(parser->state) {
  case IRC_STATE_INIT:     return IRC_STATE_NICK;
  case IRC_STATE_NICK:     return IRC_STATE_NAME;
  case IRC_STATE_NAME:     return IRC_STATE_HOST;
  case IRC_STATE_HOST:     return IRC_STATE_COMMAND;
  case IRC_STATE_COMMAND:  return IRC_STATE_PARAMS;
  case IRC_STATE_PARAMS:   return IRC_STATE_TRAILING;
  case IRC_STATE_TRAILING: return IRC_STATE_END;
  case IRC_STATE_END:      return IRC_STATE_INIT;
  case IRC_STATE_ERROR:    return IRC_STATE_ERROR;
  default:                 return IRC_STATE_ERROR;;
  }
}

void _irc_parser_call(irc_parser *parser) {
  irc_parser_cb f = _irc_parser_get_cb(parser);

  if (f == NULL) { return; }

  int result = f( parser
                , &parser->raw[parser->last]
                , parser->len - parser->last - 1
                );

  // TODO: error checking
  if (result) {
    //
  }
}

void _irc_parser_progress_state(irc_parser *parser) {
  parser->state = _irc_get_next_state(parser);
  parser->last = parser->len;
}

void _irc_parser_call_and_progress(irc_parser *parser) {
  _irc_parser_call(parser);
  _irc_parser_progress_state(parser);
}

//// public
void irc_parser_init(irc_parser *parser) {
  parser->on_nick    = NULL;
  parser->on_name    = NULL;
  parser->on_host    = NULL;
  parser->on_command = NULL;
  parser->on_param   = NULL;
  parser->on_end     = NULL;
  _irc_parser_reinit(parser);
}

size_t irc_parser_execute(irc_parser *parser, const char *data, size_t len) {
  for (int i = 0; i < len; i++) {
    switch(data[i]){
    case '\r':
      parser->state = IRC_STATE_END;
      break;
    case '\n':
      if (parser->state == IRC_STATE_END) {
        _irc_parser_call(parser);
        _irc_parser_reinit(parser);
      } else {
        return -1;
      }
      break;
    default:
      _irc_parser_append_raw(parser, data[i]);
      switch(parser->state) {
      case IRC_STATE_INIT:
        if (data[i] == ':') {
          parser->last = 1;
          parser->state = IRC_STATE_NICK;
        } else {
          i--;
          parser->state = IRC_STATE_COMMAND;
        }
        break;
      case IRC_STATE_NICK:
        IRC_PARSER_CALL_AND_PROGRESS_ON(parser, data[i], '!');
        break;
      case IRC_STATE_NAME:
        IRC_PARSER_CALL_AND_PROGRESS_ON(parser, data[i], '@');
        break;
      case IRC_STATE_HOST:
        IRC_PARSER_CALL_AND_PROGRESS_ON(parser, data[i], ' ');
        break;
      case IRC_STATE_COMMAND:
        IRC_PARSER_CALL_AND_PROGRESS_ON(parser, data[i], ' ');
        break;
      case IRC_STATE_PARAMS:
        if (data[i] == ' ') {
          _irc_parser_call(parser);
          parser->last = parser->len;
        } else if (data[i] == ':'  && parser->len == (parser->last + 1)) {
          _irc_parser_progress_state(parser);
        }
        break;
      default: // do nothing
        break;
      }
    }
  }
  return len;
}

void irc_parser_on_nick(irc_parser *parser, irc_parser_cb cb) {
  parser->on_nick = cb;
}

void irc_parser_on_name(irc_parser *parser, irc_parser_cb cb) {
  parser->on_name = cb;
}

void irc_parser_on_host(irc_parser *parser, irc_parser_cb cb) {
  parser->on_host = cb;
}

void irc_parser_on_command(irc_parser *parser, irc_parser_cb cb) {
  parser->on_command = cb;
}

void irc_parser_on_param(irc_parser *parser, irc_parser_cb cb) {
  parser->on_param = cb;
}

void irc_parser_on_end(irc_parser *parser, irc_parser_cb cb) {
  parser->on_end = cb;
}

