// Types (assuming you have these typedef'd elsewhere, standardizing for safety)
typedef uint32_t u32;
typedef uint8_t  u8;

// Pin definitions kept for reference, but physical manipulation 
// is now handled directly via hardware PORT registers.
#define A0 22
#define A1 23
#define A2 24
#define A3 25
#define A4 26
#define A5 27
#define A6 28
#define A7 29
#define A8 30
#define A9 31
#define A10 32
#define A11 33
#define A12 34
#define A13 35
#define A14 36
#define A15 37
#define A16 38
#define A17 39
#define A18 40
#define A19 41
#define A20 42

#define D0 43
#define D1 44
#define D2 45
#define D3 46
#define D4 47
#define D5 48
#define D6 49
#define D7 50

#define nWE 2  // PE4
#define nRP 3  // PE5
#define nRB 4  // PG5
#define CE 51  // PB2
#define nOE 52 // PB1

// Fast Control Pin Macros
#define nWE_HIGH (PORTE |= (1 << 4))
#define nWE_LOW  (PORTE &= ~(1 << 4))
#define nRP_HIGH (PORTE |= (1 << 5))
#define nRP_LOW  (PORTE &= ~(1 << 5))
#define CE_HIGH  (PORTB |= (1 << 2))
#define CE_LOW   (PORTB &= ~(1 << 2))
#define nOE_HIGH (PORTB |= (1 << 1))
#define nOE_LOW  (PORTB &= ~(1 << 1))

// Debug offset for dumping
#define OFFSET 262144
//#define OFFSET 0
#define MAXADDR 2097152

// Baud rate.
#define BAUD (250000)

// Wait time for chip erase. This is typically 25 seconds according to the data sheet, yet 
// at max. 120 seconds. If flashing is not done correctly, trying increasing this time value.
#define ERASE_WAIT_MS 30000

// NOP definitions for timing predictability
#define NOP1 asm volatile( "nop\n" )
#define NOP2 NOP1; NOP1
#define WAITSEQ NOP2

#define RXBYTES 32

void setDatPinsInput() {
  // D0-D6 map to PL6-PL0. D7 maps to PB3.
  DDRL &= 0b10000000; // Set PL0-PL6 to INPUT (leaves PL7 / A20 alone)
  DDRB &= ~(1 << 3);  // Set PB3 to INPUT
}

void setDatPinsOutput() {
  // D0-D6 map to PL6-PL0. D7 maps to PB3.
  DDRL |= 0b01111111; // Set PL0-PL6 to OUTPUT (leaves PL7 / A20 alone)
  DDRB |= (1 << 3);   // Set PB3 to OUTPUT
}

void setup() {
  Serial.begin( BAUD );
  
  // Set Address Pins to OUTPUT directly via Data Direction Registers (DDR)
  DDRA = 0xFF;         // A0-A7 (Pins 22-29) -> PA0-PA7
  DDRC = 0xFF;         // A8-A15 (Pins 30-37) -> PC7-PC0
  DDRD |= (1 << 7);    // A16 (Pin 38) -> PD7
  DDRG |= 0b00000111;  // A17-A19 (Pins 39-41) -> PG2, PG1, PG0
  DDRL |= (1 << 7);    // A20 (Pin 42) -> PL7

  // Data pins default to input
  setDatPinsInput();

  // Control pins direction setup
  DDRE |= (1 << 4);    // nWE (Pin 2, PE4) OUTPUT
  DDRE |= (1 << 5);    // nRP (Pin 3, PE5) OUTPUT
  DDRG &= ~(1 << 5);   // nRB (Pin 4, PG5) INPUT
  DDRB |= (1 << 2);    // CE (Pin 51, PB2) OUTPUT
  DDRB |= (1 << 1);    // nOE (Pin 52, PB1) OUTPUT

  // Initial Control states
  nWE_HIGH;
  nRP_HIGH;
  CE_LOW;
  nOE_HIGH;
}

void setAddrPins( u32 addr ) {  
  // A0 to A7 (Direct mapping to PORTA)
  PORTA = (u8)(addr & 0xFF);
  
  // A8 to A15 map to PC7 to PC0. We bit-reverse this using branchless math 
  // rather than a slow for-loop.
  u8 a8_15 = (u8)((addr >> 8) & 0xFF);
  a8_15 = (a8_15 & 0xF0) >> 4 | (a8_15 & 0x0F) << 4;
  a8_15 = (a8_15 & 0xCC) >> 2 | (a8_15 & 0x33) << 2;
  a8_15 = (a8_15 & 0xAA) >> 1 | (a8_15 & 0x55) << 1;
  PORTC = a8_15;
  
  // A16 (PD7)
  if ( addr & (1ul << 16) ) {
    PORTD |= (1 << 7); 
  } else {
    PORTD &= ~(1 << 7);
  }
  
  // A17 (PG2), A18 (PG1), A19 (PG0). Mask and shift in one go.
  u8 pg_mask = (((addr >> 17) & 1) << 2) | 
               (((addr >> 18) & 1) << 1) | 
               (((addr >> 19) & 1));
  PORTG = (PORTG & 0xF8) | pg_mask;
  
  // A20 (PL7)
  if ( addr & (1ul << 20) ) { 
    PORTL |= (1 << 7);
  } else {
    PORTL &= ~(1 << 7);
  }
}

void setDataOutput( unsigned dat ) {
  // D0-D6 go to PL6-PL0 reversed. Branchless shift mapping.
  u8 l_out = ((dat & 0x01) << 6) | ((dat & 0x02) << 4) | ((dat & 0x04) << 2) |
             ((dat & 0x08)     ) | ((dat & 0x10) >> 2) | ((dat & 0x20) >> 4) |
             ((dat & 0x40) >> 6);
             
  PORTL = (PORTL & 0x80) | l_out; // Apply leaving PL7 (A20) intact
  
  // D7 goes to PB3
  if (dat & 0x80) PORTB |= (1 << 3);
  else PORTB &= ~(1 << 3);
}

// Dump game.
void doDump() {
  Serial.println( "DoDump" );
  setDatPinsInput();
  
  // Activate chip.
  CE_HIGH;
  nOE_LOW;
  nWE_HIGH;
  nRP_HIGH;
  
  // Wait.
  delayMicroseconds( 10 );
  Serial.println( "waited" );

  // Start dump.
  for ( u32 i = OFFSET; i < MAXADDR + OFFSET; ++i ) {
    setAddrPins( i );
    WAITSEQ;
    
    // Read data pins directly from PIN hardware registers.
    // Reversing PL6-PL0 back into bits 0-6, and placing PB3 into bit 7.
    u8 l = PINL;
    char dat = ((l & 0x40) >> 6) | ((l & 0x20) >> 4) | ((l & 0x10) >> 2) |
               ((l & 0x08)     ) | ((l & 0x04) << 2) | ((l & 0x02) << 4) |
               ((l & 0x01) << 6) | ((PINB & 0x08) << 4);

    // Send it.
    Serial.write( dat );
  }

  // Turn off again.
  CE_LOW;
  nOE_HIGH;
}

void writeToAddress( u32 addr, u8 data ) {
  // Set the address.
  setAddrPins( addr );
  // Set the data.
  setDataOutput( data );

  // Activate chip.
  CE_HIGH;
  nWE_LOW;

  WAITSEQ;

  // Deactivate.
  nWE_HIGH;
  CE_LOW;
}

void unlockBypass() {
  CE_HIGH;
  nOE_HIGH;
  nWE_HIGH;

  setDatPinsOutput();
  
  writeToAddress( 0xAAA, 0xAA );
  WAITSEQ;

  writeToAddress( 0x555, 0x55 );
  WAITSEQ;

  writeToAddress( 0xAAA, 0x20 );
  WAITSEQ;

  setDatPinsInput();
  nOE_HIGH;
  nWE_HIGH;
}

void unlockBypassReset() {
  CE_HIGH;
  nOE_HIGH;
  nWE_HIGH;

  setDatPinsOutput();
  
  writeToAddress( 0x111, 0x90 );
  WAITSEQ;

  writeToAddress( 0x111, 0x00 );
  WAITSEQ;

  setDatPinsInput();
  nOE_HIGH;
  nWE_HIGH;
}

void doBypassProgram( u32 addr, u8 data ) {
  CE_HIGH;
  nOE_HIGH;
  nWE_HIGH;

  setDatPinsOutput();
  
  writeToAddress( 0x111, 0xA0 );
  WAITSEQ;

  writeToAddress( addr, data );
  WAITSEQ;

  setDatPinsInput();
  nOE_HIGH;
  nWE_HIGH;
}

void doChipErase() {
  CE_HIGH;
  nOE_HIGH;
  nWE_HIGH;
  
  setDatPinsOutput();
  
  writeToAddress( 0xAAA, 0xAA );
  WAITSEQ;
  writeToAddress( 0x555, 0x55 );
  WAITSEQ;
  writeToAddress( 0xAAA, 0x80 );
  WAITSEQ;
  writeToAddress( 0xAAA, 0xAA );
  WAITSEQ;
  writeToAddress( 0x555, 0x55 );
  WAITSEQ;
  writeToAddress( 0xAAA, 0x10 );

  delay( ERASE_WAIT_MS );

  setDatPinsInput();
  nOE_HIGH;
  nWE_HIGH;
}

void doProgram( u32 addr, u8 data ) {
  writeToAddress( 0xAAA, 0xAA );
  WAITSEQ;
  writeToAddress( 0x555, 0x55 );
  WAITSEQ;
  writeToAddress( 0xAAA, 0xA0 );
  WAITSEQ;
  
  writeToAddress( addr, data );
  WAITSEQ;
}

void doFlash() {
  nRP_HIGH;

  CE_HIGH;
  nOE_HIGH;
  nWE_HIGH;

  setDatPinsOutput();

  u32 nrBytes = 0;

  while ( nrBytes < MAXADDR ) {
    if ( Serial.available() >= RXBYTES ) {
      for ( int i = 0; i < RXBYTES; ++i ) {
        char b = Serial.read();
        doProgram( nrBytes, b );
        ++nrBytes;
      }
      char conf = 1;
      Serial.write( conf );
      Serial.flush();
    }
  }

  CE_LOW;
  nOE_HIGH;
  nWE_HIGH;
  
  setDatPinsInput();
}

void doFlashBypass() {
  CE_HIGH;
  nOE_HIGH;
  nWE_HIGH;

  setDatPinsOutput();

  u32 nrBytes = 0;
  unlockBypass();

  while ( nrBytes < MAXADDR ) {
    if ( Serial.available() >= RXBYTES ) {
      for ( int i = 0; i < RXBYTES; ++i ) {
        char b = Serial.read();
        doBypassProgram( nrBytes, b );
        ++nrBytes;
      }
      char conf = 1;
      Serial.write( conf );
      Serial.flush();
    }
  }

  CE_LOW;
  nOE_HIGH;
  nWE_HIGH;
  
  setDatPinsInput();
  unlockBypassReset();
}

void loop() {
  if ( Serial.available() ) {
    char rx = Serial.read();
    if ( rx == 42 ) {
      doDump();
    } else if ( rx == 82 ) {
      doFlashBypass();
    } else if ( rx == 14 ) {
      unlockBypassReset();
    } else if ( rx == 123 ) {
      doChipErase();
      Serial.write( 42 );
    }
  }
}