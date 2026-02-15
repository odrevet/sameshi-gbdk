#include "sameshi.h"
#include <gb/gb.h>
#include <stdio.h>
extern int b[120], bs, bd;
static char pc(int q) {
  int a = j(q);
  if (!a)
    return '.';
  if (a == 1)
    return q > 0 ? 'P' : 'p';
  if (a == 2)
    return q > 0 ? 'N' : 'n';
  if (a == 3)
    return q > 0 ? 'B' : 'b';
  if (a == 4)
    return q > 0 ? 'R' : 'r';
  if (a == 5)
    return q > 0 ? 'Q' : 'q';
  return q > 0 ? 'K' : 'k';
}
static void board(void) {
  puts("");
  puts("  a b c d e f g h");
  for (int r = 9; r >= 2; r--) {
    printf("%d ", r - 1);
    for (int c = 1; c <= 8; c++)
      printf("%c ", pc(b[r * 10 + c]));
    printf("%d\n", r - 1);
  }
  puts("  a b c d e f g h");
}
static int sq(char f, char r) {
  if (f < 'a' || f > 'h' || r < '1' || r > '8')
    return -1;
  return (r - '0' + 1) * 10 + (f - 'a' + 1);
}

#define SIDE -1

#ifdef GAMEBOY
#define ALPHA -10000
#define BETA 10000
#define DEPTH 3
#else
#define ALPHA -30000
#define BETA 30000
#define DEPTH 5
#endif

uint8_t joypad_previous, joypad_current;
uint8_t input_index = 0;
uint8_t player_turn = 0;

int8_t s;
int8_t d;

char m[8] = {'a', '1', 'a', '1', 0, 0, 0, 0};

void print_input() {
  printf("%s\n", m);
  for (int i = 0; i < input_index; i++)
    printf(" ");
  printf("^\n");
}

void main(void) {
  I();

  DISPLAY_ON;
  joypad_previous, joypad_current = 0;
  
  board();
  print_input();

  while (1) {
    joypad_previous = joypad_current;
    joypad_current = joypad();
    if (joypad_current & J_START && !(joypad_previous & J_START)) {
      s = sq(m[0], m[1]);
      d = sq(m[2], m[3]);
      b[d] = b[s];
      b[s] = 0;

      printf("thinking...\n");
      S(SIDE, DEPTH, ALPHA, BETA);
      printf("ai: %c%c%c%c\n", 'a' + bs % 10 - 1, '0' + bs / 10 - 1,
             'a' + bd % 10 - 1, '0' + bd / 10 - 1);
      b[bd] = b[bs];
      b[bs] = 0;
      board();
      print_input();
    }

    if (joypad_current & J_UP && !(joypad_previous & J_UP)) {
      if (input_index % 2 == 0) {
        if (m[input_index] < 'a' || m[input_index] >= 'h')
          m[input_index] = 'a';
        else
          m[input_index]++;
      } else {
        if (m[input_index] < '1' || m[input_index] >= '8')
          m[input_index] = '1';
        else
          m[input_index]++;
      }
      print_input();
    }

    if (joypad_current & J_DOWN && !(joypad_previous & J_DOWN)) {
      if (input_index % 2 == 0) {
        if (m[input_index] <= 'a' || m[input_index] > 'h')
          m[input_index] = 'h';
        else
          m[input_index]--;
      } else {
        if (m[input_index] <= '1' || m[input_index] > '8')
          m[input_index] = '8';
        else
          m[input_index]--;
      }
      print_input();
    }

    if (joypad_current & J_LEFT && !(joypad_previous & J_LEFT) &&
        input_index > 0) {
      input_index--;
      printf("%s\n", m);
      for (int i = 0; i < input_index; i++)
        printf(" ");
      printf("^\n");
    }

    if (joypad_current & J_RIGHT && !(joypad_previous & J_RIGHT) &&
        input_index < 3) {
      input_index++;
      print_input();
    }

    if (joypad_current & J_B && !(joypad_previous & J_B)) {
      board();
      print_input();
    }

    wait_vbl_done();
  }
}