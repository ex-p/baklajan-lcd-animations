#include <SD.h>
#include <SPI.h>

#define ROWS 3
#define LEFT_POOL 300 * ROWS
#define RIGHT_POOL 192 * ROWS

#define PIXELS LEFT_POOL
#define PIXEL_PORT  PORTB  // Port of the pin the pixels are connected to
#define PIXEL_DDR   DDRB   // Port of the pin the pixels are connected to
#define PIXEL_BIT   4      // Bit of the pin the pixels are connected to
#define T1H  900    // Width of a 1 bit in ns
#define T1L  600    // Width of a 1 bit in ns
#define T0H  400    // Width of a 0 bit in ns
#define T0L  900    // Width of a 0 bit in ns
#define RES 6000    // Width of the low gap between bits to cause a frame to latch
#define NS_PER_SEC (1000000000L)          // Note that this has to be SIGNED since we want to be able to check for negative values of derivatives
#define CYCLES_PER_SEC (F_CPU)
#define NS_PER_CYCLE ( NS_PER_SEC / CYCLES_PER_SEC )
#define NS_TO_CYCLES(n) ( (n) / NS_PER_CYCLE )

inline void sendBit( bool bitVal ) {
  
    if (  bitVal ) {        // 0 bit
      
    asm volatile (
      "sbi %[port], %[bit] \n\t"        // Set the output bit
      ".rept %[onCycles] \n\t"                                // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      "cbi %[port], %[bit] \n\t"                              // Clear the output bit
      ".rept %[offCycles] \n\t"                               // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      ::
      [port]    "I" (_SFR_IO_ADDR(PIXEL_PORT)),
      [bit]   "I" (PIXEL_BIT),
      [onCycles]  "I" (NS_TO_CYCLES(T1H) - 2),    // 1-bit width less overhead  for the actual bit setting, note that this delay could be longer and everything would still work
      [offCycles]   "I" (NS_TO_CYCLES(T1L) - 2)     // Minimum interbit delay. Note that we probably don't need this at all since the loop overhead will be enough, but here for correctness

    );
                                  
    } else {          // 1 bit

    // **************************************************************************
    // This line is really the only tight goldilocks timing in the whole program!
    // **************************************************************************


    asm volatile (
      "sbi %[port], %[bit] \n\t"        // Set the output bit
      ".rept %[onCycles] \n\t"        // Now timing actually matters. The 0-bit must be long enough to be detected but not too long or it will be a 1-bit
      "nop \n\t"                                              // Execute NOPs to delay exactly the specified number of cycles
      ".endr \n\t"
      "cbi %[port], %[bit] \n\t"                              // Clear the output bit
      ".rept %[offCycles] \n\t"                               // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      ::
      [port]    "I" (_SFR_IO_ADDR(PIXEL_PORT)),
      [bit]   "I" (PIXEL_BIT),
      [onCycles]  "I" (NS_TO_CYCLES(T0H) - 2),
      [offCycles] "I" (NS_TO_CYCLES(T0L) - 2)

    );
      
    }
    
    // Note that the inter-bit gap can be as long as you want as long as it doesn't exceed the 5us reset timeout (which is A long time)
    // Here I have been generous and not tried to squeeze the gap tight but instead erred on the side of lots of extra time.
    // This has thenice side effect of avoid glitches on very long strings becuase 

    
}  



inline void sendByte(int byte) {
  for (int bit = 0; bit < 8;++bit) {
    // Neopixel wants bit in highest-to-lowest order
    // so send highest bit (bit #7 in an 8-bit byte
    // since they start at 0)
    sendBit(bitRead(byte, 7));
    // and then shift left so bit 6 moves into 7, 5 moves into 6, etc
    byte <<= 1;
  }
}

void showPixel(int r, int g, int b) {
  // Neopixel wants colors in green then red then blue order
  sendByte(g);
  sendByte(r);
  sendByte(b);
}

void showBoard() {
   _delay_us( (RES / 1000UL) + 1);
}

struct OuterAnimation {
  String path = "outer";
  int stages = 0;
  OuterAnimation() {
    cli();
    File info = SD.open(path + "/" + "info.txt");
    if (!info) {
      // TODO(SnakeAce): handle error
    }
    stages = info.readStringUntil('\n').toInt();
    info.close();
    sei();
  }

  void show(String side, int pool_size) {
    for (int stage = 0; stage < stages; ++stage) {
      cli();
      String file_path = path + "/l" + stage + ".txt";
      File data = SD.open(file_path);
      if (!data) {
        // TODO: handle error
      }

      for (int i = 0; i < pool_size; ++i) {
        int r = data.readStringUntil('\n').toInt();
        int g = data.readStringUntil('\n').toInt();
        int b = data.readStringUntil('\n').toInt();

        showPixel(r, g, b);
      }

      showBoard();
      data.close();
      sei();
    }
  }

  void showLeft() { show("l", LEFT_POOL); }

  void showRight() { show("r", RIGHT_POOL); }
};

OuterAnimation *outer;

void setup() {
  Serial.begin(9600);
  Serial.println("Hello World!");
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);

  if (!SD.begin(10)) {
    Serial.print("The SD card cannot be found");
    while (true) {
    }
  }
  outer = new OuterAnimation();
}

void loop() {
  outer->showLeft();
}
