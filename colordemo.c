#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

typedef bool (char_provider)(void);

char *string;

char *word = NULL;

size_t word_cap;

char c;

bool string_stream(void) {
  c = *(string++);
  return c != '\0';
}

bool is_whitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void parse_whitespace(char_provider *next_char) {
  while(is_whitespace(c) && next_char());
}

void parse_text(char_provider *next_char) {
  while(c != '<' && c != '>' && (putchar(c), next_char()));
}

void realloc_word(size_t new_cap) {
  word_cap = new_cap;
  word = (char *)realloc(word, new_cap);
}

void set_word(size_t idx, char c) {
  while(idx + 1 > word_cap) {
    realloc_word(word_cap * 2);
  }
  word[idx] = c;
}

void parse_word(char_provider next_char) {
  parse_whitespace(next_char);
  bool nc = true;
  size_t i;
  for(i = 0; nc && c != ';' && !is_whitespace(c); ++i, nc = next_char()) {
    set_word(i, c);
  }
  set_word(i, '\0');
}

int colorid(char *color) {
  if(strcmp(color, "black") == 0) {
    return 0;
  }
  else if(strcmp(color, "red") == 0) {
    return 1;
  }
  else if(strcmp(color, "green") == 0) {
    return 2;
  }
  else if(strcmp(color, "yellow") == 0) {
    return 3;
  }
  else if(strcmp(color, "blue") == 0) {
    return 4;
  }
  else if(strcmp(color, "megenta") == 0) {
    return 5;
  }
  else if(strcmp(color, "cyan") == 0) {
    return 6;
  }
  else if(strcmp(color, "white") == 0) {
    return 7;
  }
  else if(strcmp(color, "normal") == 0) {
    return 9;
  }
  else {
    return -1;
  }
}

void start_context(int id) {
  printf("\e[%dm", id);
}

void start_context_bg_color(char *color) {
  start_context(40 + colorid(color));
}

void start_context_fg_color(char *color) {
  start_context(30 + colorid(color));
}

void parse_cmd(char_provider next_char) {
  parse_word(next_char);
  if(strcmp(word, "bold") == 0) {
    start_context(1);
  }
  else if(strcmp(word, "bg") == 0) {
    parse_word(next_char);
    start_context_bg_color(word);
  }
  else if(colorid(word) != -1) {
    start_context_fg_color(word);
  }
  if(c == ';') {
    next_char();
  }
  else {
    parse_cmd(next_char);
  }
}

void pop_context() {}

void parse_script(char_provider *next_char) {
  if(c == '\0') {
    return;
  }
  parse_text(next_char);
  switch (c) {
    case '<':
      next_char();
      parse_cmd(next_char);
      break;
    case '>':
      next_char();
      pop_context();
      break;
  }
  parse_script(next_char);
}

int main(int argc, char *args[]) {
  realloc_word(10);
  if(argc > 1) {
    string = args[1];
    string_stream();
    parse_script(string_stream);
  }
  free(word);
  return 0;
}
