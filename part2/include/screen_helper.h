#define NUM_COLUMNS 80
#define NUM_ROWS    25

enum Colors {
  BLACK = 0b0000,
  BLUE = 0b0001,
  GREEN = 0b0010,
  CYAN = 0b0011,
  RED = 0b0100,
  MAGENTA = 0b0101,
  BROWN = 0b0110,
  LIGHT_GRAY = 0b0111,
  // All color below only work for Foreground [used for Background will raise an error]
  DARK_GRAY = 0b1000,
  LIGHT_BLUE = 0b1001,
  LIGHT_GREEN = 0b1010,
  LIGHT_CYAN = 0b1011,
  LIGHT_RED = 0b1100,
  LIGHT_MAGENTA = 0b1101,
  YELLOW = 0b1110,
  WHITE = 0b1111
};