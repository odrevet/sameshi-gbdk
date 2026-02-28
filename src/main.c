#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#include <stdio.h>

#include "sameshi.h"

#include <Background.h>
#include <Font.h>
#include <Hand.h>
#include <Pieces.h>

extern int b[120], bs, bd;
uint8_t input_index = 0;
uint8_t player_turn = 0;

int8_t s;
int8_t d;

char m[8] = {'a', '1', 'a', '1', 0, 0, 0, 0};

#define SIDE -1
#define DEPTH 3
#define ALPHA -10000
#define BETA 10000

#define PIECE_SIZE_IN_TILES 2
#define PX_PER_SQUARE 16

#define BOARD_DRAW_X 1
#define BOARD_DRAW_Y 1

#ifdef GAMEBOY
#define HAND_OFFSET_X 24
#define HAND_OFFSET_Y 32
#elif defined(NINTENDO_NES)
#define HAND_OFFSET_X 16
#define HAND_OFFSET_Y 14
#else
#define HAND_OFFSET_X 0
#define HAND_OFFSET_Y 0
#endif

uint8_t cursor_x = 4;
uint8_t cursor_y = 7;

uint8_t joypad_previous = 0;
uint8_t joypad_current = 0;

uint8_t grabbing = 0;
int grabbed_piece = 0;
uint8_t grabbed_sq = 0;

const uint8_t king_black_on_white_tiles[4] = {0, 1, 12, 13};
const uint8_t queen_black_on_white_tiles[4] = {2, 3, 14, 15};
const uint8_t bishop_black_on_white_tiles[4] = {4, 5, 16, 17};
const uint8_t knight_black_on_white_tiles[4] = {6, 7, 18, 19};
const uint8_t rook_black_on_white_tiles[4] = {8, 9, 20, 21};
const uint8_t pawn_black_on_white_tiles[4] = {10, 11, 22, 23};

const uint8_t king_white_on_white_tiles[4] = {24, 25, 36, 37};
const uint8_t queen_white_on_white_tiles[4] = {26, 27, 38, 39};
const uint8_t bishop_white_on_white_tiles[4] = {28, 29, 40, 41};
const uint8_t knight_white_on_white_tiles[4] = {30, 31, 42, 43};
const uint8_t rook_white_on_white_tiles[4] = {32, 33, 44, 45};
const uint8_t pawn_white_on_white_tiles[4] = {34, 35, 46, 47};

const uint8_t king_black_on_black_tiles[4] = {48, 49, 60, 61};
const uint8_t queen_black_on_black_tiles[4] = {50, 51, 62, 63};
const uint8_t bishop_black_on_black_tiles[4] = {52, 53, 64, 65};
const uint8_t knight_black_on_black_tiles[4] = {54, 55, 66, 67};
const uint8_t rook_black_on_black_tiles[4] = {56, 57, 68, 69};
const uint8_t pawn_black_on_black_tiles[4] = {58, 59, 70, 71};

const uint8_t king_white_on_black_tiles[4] = {72, 73, 84, 85};
const uint8_t queen_white_on_black_tiles[4] = {74, 75, 86, 87};
const uint8_t bishop_white_on_black_tiles[4] = {76, 77, 88, 89};
const uint8_t knight_white_on_black_tiles[4] = {78, 79, 90, 91};
const uint8_t rook_white_on_black_tiles[4] = {80, 81, 92, 93};
const uint8_t pawn_white_on_black_tiles[4] = {82, 83, 94, 95};

const uint8_t empty_white_tiles[4] = {
    Background_TILE_ORIGIN + 0, Background_TILE_ORIGIN + 0,
    Background_TILE_ORIGIN + 0, Background_TILE_ORIGIN + 0};
const uint8_t empty_black_tiles[4] = {
    Background_TILE_ORIGIN + 1, Background_TILE_ORIGIN + 1,
    Background_TILE_ORIGIN + 1, Background_TILE_ORIGIN + 1};

void clear_screen(void) {
  for (uint8_t ty = 0; ty < DEVICE_SCREEN_HEIGHT; ty++) {
    for (uint8_t tx = 0; tx < DEVICE_SCREEN_WIDTH; tx++) {
      set_bkg_tiles(tx, ty, 1, 1, &empty_white_tiles[0]);
    }
  }
}

static char cursor_file(void) {
  return 'a' + cursor_x;
}

static char cursor_rank(void) {
  return '8' - cursor_y;
}

static int alg_to_sq(char file, char rank) {
  return (rank - '0' + 1) * 10 + (file - 'a' + 1);
}

// Draw the board. If grabbing, the source square is shown empty and the
// grabbed piece is shown at the current cursor square as a preview.
void draw_board(void) {
  uint8_t preview_sq = grabbing ? alg_to_sq(cursor_file(), cursor_rank()) : 0;

  for (uint8_t row = 0; row < 8; row++) {
    for (uint8_t col = 0; col < 8; col++) {
      uint8_t sq = (9 - row) * 10 + (col + 1);
      int sq_val = b[sq];

      // Hide piece from its origin square while dragging
      if (grabbing && sq == grabbed_sq)
        sq_val = 0;

      // Show grabbed piece at the cursor square
      if (grabbing && sq == preview_sq)
        sq_val = grabbed_piece;

      int piece_type = j(sq_val);
      uint8_t tx = BOARD_DRAW_X + col * PIECE_SIZE_IN_TILES;
      uint8_t ty = BOARD_DRAW_Y + row * PIECE_SIZE_IN_TILES;
      uint8_t is_white_piece = (sq_val > 0);
      uint8_t is_white_square = !((row + col) & 1);

      const uint8_t *tiles = NULL;

      if (piece_type == 0) {
        tiles = is_white_square ? empty_white_tiles : empty_black_tiles;
      } else if (is_white_piece) {
        if (is_white_square) {
          switch (piece_type) {
          case 6: tiles = king_white_on_white_tiles;   break;
          case 5: tiles = queen_white_on_white_tiles;  break;
          case 3: tiles = bishop_white_on_white_tiles; break;
          case 2: tiles = knight_white_on_white_tiles; break;
          case 4: tiles = rook_white_on_white_tiles;   break;
          case 1: tiles = pawn_white_on_white_tiles;   break;
          }
        } else {
          switch (piece_type) {
          case 6: tiles = king_white_on_black_tiles;   break;
          case 5: tiles = queen_white_on_black_tiles;  break;
          case 3: tiles = bishop_white_on_black_tiles; break;
          case 2: tiles = knight_white_on_black_tiles; break;
          case 4: tiles = rook_white_on_black_tiles;   break;
          case 1: tiles = pawn_white_on_black_tiles;   break;
          }
        }
      } else {
        if (is_white_square) {
          switch (piece_type) {
          case 6: tiles = king_black_on_white_tiles;   break;
          case 5: tiles = queen_black_on_white_tiles;  break;
          case 3: tiles = bishop_black_on_white_tiles; break;
          case 2: tiles = knight_black_on_white_tiles; break;
          case 4: tiles = rook_black_on_white_tiles;   break;
          case 1: tiles = pawn_black_on_white_tiles;   break;
          }
        } else {
          switch (piece_type) {
          case 6: tiles = king_black_on_black_tiles;   break;
          case 5: tiles = queen_black_on_black_tiles;  break;
          case 3: tiles = bishop_black_on_black_tiles; break;
          case 2: tiles = knight_black_on_black_tiles; break;
          case 4: tiles = rook_black_on_black_tiles;   break;
          case 1: tiles = pawn_black_on_black_tiles;   break;
          }
        }
      }

      if (tiles != NULL) {
        set_bkg_tiles(tx, ty, PIECE_SIZE_IN_TILES, PIECE_SIZE_IN_TILES, tiles);
      }
    }
  }
}

void main(void) {
  I();
  SPRITES_8x16;
  DISPLAY_ON;
  SHOW_SPRITES;
  SHOW_BKG;

  set_sprite_data(Hand_TILE_ORIGIN, Hand_TILE_COUNT, Hand_tiles);
  set_bkg_data(Pieces_TILE_ORIGIN, Pieces_TILE_COUNT, Pieces_tiles);
  set_bkg_data(Background_TILE_ORIGIN, Background_TILE_COUNT, Background_tiles);

  clear_screen();
  draw_board();

  while (1) {
    move_metasprite_ex(Hand_metasprites[grabbing], Hand_TILE_ORIGIN, 0, 0,
                       BOARD_DRAW_X * 8 + cursor_x * PX_PER_SQUARE + HAND_OFFSET_X,
                       BOARD_DRAW_Y * 8 + cursor_y * PX_PER_SQUARE + HAND_OFFSET_Y);
    joypad_previous = joypad_current;
    joypad_current = joypad();

    uint8_t moved = 0;

    if (joypad_current & J_UP && !(joypad_previous & J_UP)) {
      if (cursor_y > 0) { cursor_y--; moved = 1; }
    }
    if (joypad_current & J_DOWN && !(joypad_previous & J_DOWN)) {
      if (cursor_y < 7) { cursor_y++; moved = 1; }
    }
    if (joypad_current & J_LEFT && !(joypad_previous & J_LEFT)) {
      if (cursor_x > 0) { cursor_x--; moved = 1; }
    }
    if (joypad_current & J_RIGHT && !(joypad_previous & J_RIGHT)) {
      if (cursor_x < 7) { cursor_x++; moved = 1; }
    }

    // Redraw whenever the cursor moves while grabbing so the preview updates
    if (grabbing && moved) {
      draw_board();
    }

    if (joypad_current & J_B && !(joypad_previous & J_B)) {
      if (!grabbing) {
        grabbed_sq = alg_to_sq(cursor_file(), cursor_rank());
        if (b[grabbed_sq] > 0) {
          m[0] = cursor_file();
          m[1] = cursor_rank();
          grabbed_piece = b[grabbed_sq];
          grabbing = 1;
          draw_board();
        }
      } else {
        m[2] = cursor_file();
        m[3] = cursor_rank();
        grabbing = 0;
        grabbed_piece = 0;

        if (m[2] == m[0] && m[3] == m[1]) {
          draw_board(); // restore board with piece back in place
        } else {
          s = alg_to_sq(m[0], m[1]);
          d = alg_to_sq(m[2], m[3]);

          b[d] = b[s];
          b[s] = 0;

          draw_board();

          S(SIDE, DEPTH, ALPHA, BETA);

          b[bd] = b[bs];
          b[bs] = 0;

          draw_board();
        }
      }
    }

    // Cancel grab with A button
    if (joypad_current & J_A && !(joypad_previous & J_A)) {
      if (grabbing) {
        grabbing = 0;
        grabbed_piece = 0;
        draw_board(); // restore piece to origin square
      }
    }

    wait_vbl_done();
  }
}