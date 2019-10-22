#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "thanks/mt19937ar.c"
#include "./stack.c"

typedef bool (char_provider)(void);

char *string;

char *word = NULL;

size_t word_cap;

char c;

stack_t context;

int bg_color;

int fg_color;

bool bold;

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
  else if(strcmp(color, "magenta") == 0) {
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
  else if(strcmp(color, "random") == 0) {
    return genrand_int32() % 10;
  }
  else {
    return -1;
  }
}

void start_context(void) {
  stack_push(&context, -1);
  stack_push(&context, fg_color);
  stack_push(&context, bg_color);
  if(bold) {
    stack_push(&context, bold);
  }
}

void set_context_bold(bool b) {
  bold = b;
  (void)(b && printf("\e[1m"));
}

void set_context_bg_color(int color) {
  fg_color = 40 + color;
  printf("\e[%dm", fg_color);
}

void set_context_fg_color(int color) {
  bg_color = 30 + color;
  printf("\e[%dm", bg_color);
}

void set_context_style(int style) {
  if(style == 1) {
    set_context_bold(true);
  }
  else if(style >= 30 && style < 40) {
    set_context_fg_color(style % 30);
  }
  else if(style >= 40 && style < 50) {
    set_context_bg_color(style % 40);
  }
}

void clear_context_style(void) {
  printf("\e[0m");
  fg_color = 9;
  bg_color = 9;
  bold = false;
}

void end_context(void) {
  clear_context_style();
  char style;
  bool empty;
  while((style = stack_pop(&context, &empty), true) && !empty && style != -1) {
    set_context_style(style);
  }
}

void parse_cmd(char_provider next_char) {
  parse_word(next_char);
  if(strcmp(word, "bold") == 0) {
    set_context_bold(true);
  }
  else if(strcmp(word, "bg") == 0) {
    parse_word(next_char);
    set_context_bg_color(colorid(word));
  }
  else if(colorid(word) != -1) {
    set_context_fg_color(colorid(word));
  }
  if(c == ';') {
    next_char();
  }
  else {
    parse_cmd(next_char);
  }
}

void parse_script(char_provider *next_char) {
  if(c == '\0') {
    return;
  }
  parse_text(next_char);
  switch (c) {
    case '<':
      start_context();
      next_char();
      parse_cmd(next_char);
      break;
    case '>':
      next_char();
      end_context();
      break;
  }
  parse_script(next_char);
}

int main(int argc, char *args[]) {
  struct timespec t;
  timespec_get(&t, TIME_UTC);
  init_genrand(t.tv_nsec ^ t.tv_sec);
  start_context();
  set_context_fg_color(9);
  set_context_bg_color(9);
  set_context_bold(false);
  realloc_word(10);
  if(argc > 1) {
    size_t repeat = 1;
    for(size_t I = 0; I < repeat; ++I) {
      for(size_t i = 1; i < argc; ++i) {
        string = args[i];
	if(strcmp(string, "-r") * strcmp(string, "--repeat") == 0) {
	  if(argc <= i + 1) {
            printf("Parameter for \"%s\" is missing.", string);
	    exit(1);
	  }
	  char *remaining;
	  long r = strtol(args[i + 1], &remaining, 10);
	  if(*remaining != '\0' || r < 0) {
            printf("\"%s\" is not a positive integer.", args[i + 1]);
	    exit(1);
	  }
	  repeat = r;
          ++i;
	  continue;
	}
        string_stream();
        parse_script(string_stream);
        printf("\n");
      }
    }
  }
  free(word);
  return 0;
}
