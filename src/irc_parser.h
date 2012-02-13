#ifndef irc_parser_h
#define irc_parser_h

typedef struct irc_parser_s irc_parser;

typedef int (*irc_parser_cb)(irc_parser*, const char *at, size_t len);

// scanning states
enum irc_parser_state {
  IRC_STATE_INIT = 0,
  IRC_STATE_NICK,
  IRC_STATE_NAME,
  IRC_STATE_HOST,
  IRC_STATE_COMMAND,
  IRC_STATE_PARAMS,
  IRC_STATE_TRAILING,
  IRC_STATE_END,
  IRC_STATE_ERROR
};


struct irc_parser_s {
  int len;
  int last;
  enum irc_parser_state state;
  char raw[513];
  irc_parser_cb on_nick;
  irc_parser_cb on_name;
  irc_parser_cb on_host;
  irc_parser_cb on_command;
  irc_parser_cb on_param;
  irc_parser_cb on_end;
};

void irc_parser_init(irc_parser *parser);
size_t irc_parser_execute(irc_parser *parser, const char *data, size_t len);
void irc_parser_on_nick(irc_parser *parser, irc_parser_cb cb);
void irc_parser_on_name(irc_parser *parser, irc_parser_cb cb);
void irc_parser_on_host(irc_parser *parser, irc_parser_cb cb);
void irc_parser_on_command(irc_parser *parser, irc_parser_cb cb);
void irc_parser_on_param(irc_parser *parser, irc_parser_cb cb);
void irc_parser_on_end(irc_parser *parser, irc_parser_cb cb);

#endif irc_parser_h
