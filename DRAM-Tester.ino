/* Simple DRAM tester
 *            2022 Dominic Thibodeau
 *            https://github.com/hotkeysoft/arduino-dram-tester
 * 
 * Based on:  http://insentricity.com/a.cl/252  Chris Osborn <fozztexx@fozztexx.com>
 *            https://github.com/FozzTexx/DRAM-Tester
 *
 *            http://www.andremiller.net/ Andre Miller 
 *            https://github.com/andremiller/arduino-dram-tester
 */

// Tested on Arduino UNO

#define DIN             2
#define DOUT            3
#define CAS             5
#define RAS             6
#define WE              7

// 10 is max address bits
//  8 for 64K (4164 et al.)
// etc.
#define ADDR_BITS       8

const int ADDR_MAX (1 << ADDR_BITS);

void fill(int val, bool readBack = false);
void fillAlternating(int start);
void marchUp(int val);
void marchDown(int val);

void setup()
{
  int mask;
  Serial.begin(115200); 
  Serial.println("SETUP");
  Serial.print("Number of address bits: ");
  Serial.println(ADDR_BITS);
  Serial.print("Row / Columns: ");
  Serial.print(ADDR_MAX);
  Serial.print(" x ");
  Serial.println(ADDR_MAX);
  
  pinMode(DIN, OUTPUT);
  pinMode(DOUT, INPUT);

  pinMode(CAS, OUTPUT);
  pinMode(RAS, OUTPUT);
  pinMode(WE, OUTPUT);

  /* 10 is max address bits, even if chip is smaller */
  mask = (1 << 10) - 1; 
  DDRB = mask & 0x3f;
  mask >>= 6;
  DDRC = mask & 0x0f;
  
  digitalWrite(CAS, HIGH);
  digitalWrite(RAS, HIGH);
  digitalWrite(WE, HIGH);
}

void loop()
{
  static int i=1;
  Serial.print("START ITERATION: ");
  Serial.println(i);

  fillAlternating(1);
  fillAlternating(0);

  // Marching 1/0
  fill(0);      // Write all 0
  marchUp(1);   // Read 0, write 1, read back - up addressing
  marchDown(0); // Read 1, write 0, read back - down addressing
  fill(1);      // Write all 1
  marchUp(0);   // Read 1, write 0, read back - up addressing
  marchDown(1); // Read 0, write 1, read back - down addressing

  fillRandom(10);
  fillRandom(200);

  Serial.print("END ITERATION: ");
  Serial.println(i);
  ++i;
}

int setAddress(int row, int col, bool write = false)
{
  int val = 0;

  PORTB = row & 0x3f;
  PORTC = (PORTC & 0xf0) | (row >> 6) & 0x0f;
  digitalWrite(RAS, LOW);

  if (write)
  {
    digitalWrite(WE, LOW);
  }

  PORTB = col & 0x3f;
  PORTC = (PORTC & 0xf0) | (col >> 6) & 0x0f;
  digitalWrite(CAS, LOW);

  if (write)
  {
    digitalWrite(WE, HIGH);
  }
  else
  {
    val = digitalRead(DOUT);
  }
 
  digitalWrite(CAS, HIGH);
  digitalWrite(RAS, HIGH);

  return val;
}

void fail(int row, int col, int val)
{
  Serial.print("*** FAIL row ");
  Serial.print(row);
  Serial.print(" col ");
  Serial.print(col);
  Serial.print(" was expecting ");
  Serial.print(val);
  Serial.print(" got ");
  Serial.println(!val);

  while (1)
  {
    ;
  }
}

void fill(int val, bool readBack)
{
  int row, col;

  Serial.print("  Setting all bits set to: ");
  Serial.println(val);
  digitalWrite(DIN, val);

  Serial.println("    Write");
  for (col = 0; col < ADDR_MAX; col++)
  {
    for (row = 0; row < ADDR_MAX; row++)
    {
      setAddress(row, col, true);
    }
  }

  /* Reverse DIN in case DOUT is floating */
  digitalWrite(DIN, !val);

  if (readBack)
  {
    Serial.println("    Read");
    for (col = 0; col < ADDR_MAX; col++)
    {
      for (row = 0; row < ADDR_MAX; row++)
      {
        if (setAddress(row, col) != val)
        {
          fail(row, col, val);
        }
      }
    }

    Serial.println("    PASS");    
  }
}

void fillAlternating(int start)
{
  int row, col, i;

  Serial.print("  Alternating bits starting with: ");
  Serial.println(start);

  Serial.println("    Write");
  i = start;
  for (col = 0; col < ADDR_MAX; col++)
  {
    for (row = 0; row < ADDR_MAX; row++)
    {
      digitalWrite(DIN, i);
      i = !i;
      setAddress(row, col, true);
    }
  }

  Serial.println("    Read");
  for (col = 0; col < ADDR_MAX; col++)
  {
    for (row = 0; row < ADDR_MAX; row++)
    {
      if (setAddress(row, col) != i)
      {
        fail(row, col, i);
      }

      i = !i;
    }
  }
  
  Serial.println("    PASS");
}

void marchUp(int val)
{
  int row, col;

  Serial.print("  Marching UP, read ");
  Serial.print(!val);
  Serial.print(" , write ");
  Serial.print(val);
  Serial.print(" , read ");
  Serial.println(val);

  for (col = 0; col < ADDR_MAX; col++)
  {
    for (row = 0; row < ADDR_MAX; row++)
    {
      // Read previous value
      if (setAddress(row, col) != !val)
      {
        fail(row, col, val);
      }

      // Write new value
      digitalWrite(DIN, val);
      setAddress(row, col, true);

      // Read back value
      if (setAddress(row, col) != val)
      {
        fail(row, col, val);
      }
    }
  }

  Serial.println("    PASS");
}

void marchDown(int val)
{
  int row, col;

  Serial.print("  Marching DOWN, read ");
  Serial.print(!val);
  Serial.print(" , write ");
  Serial.print(val);
  Serial.print(" , read ");
  Serial.println(val);

  for (col = ADDR_MAX - 1; col >= 0; col--)
  {
    for (row = ADDR_MAX - 1; row >=0 ; row--)
    {
      // Read previous value
      if (setAddress(row, col) != !val)
      {
        fail(row, col, val);
      }

      // Write new value
      digitalWrite(DIN, val);
      setAddress(row, col, true);

      // Read back value
      if (setAddress(row, col) != val)
      {
        fail(row, col, val);
      }
    }
  }

  Serial.println("    PASS");
}

void fillRandom(int seed)
{
  int row, col, i;

  randomSeed(seed);

  Serial.print("  Random bit values with seed: ");
  Serial.println(seed);

  Serial.println("    Write");
  for (col = 0; col < ADDR_MAX; col++)
  {
    for (row = 0; row < ADDR_MAX; row++)
    {
      i = random(0,2);
      digitalWrite(DIN, i);
      setAddress(row, col, 1);
    }
  }

  randomSeed(seed);

  Serial.println("    Read");
  for (col = 0; col < ADDR_MAX; col++)
  {
    for (row = 0; row < ADDR_MAX; row++)
    {
      i = random(0,2);
      if (setAddress(row, col, 0) != i)
      {
        fail(row, col, i);
      }
    }
  }
  
  Serial.println("    PASS");
}
