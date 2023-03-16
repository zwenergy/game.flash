// Pin defintions. Note that when these are changed,
// pin-related functions need to updated manually.
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

#define nWE 2
#define nRP 3
#define nRB 4
#define CE 51
#define nOE 52

// Debug offset for dumping
#define OFFSET 262144
//#define OFFSET 0
#define MAXADDR 2097152

// Some NOP definitions.
// Yes, this is very pretty. But we do not have code size problems,
// hence using stupid sequences of NOPs makes it nicely timing 
// predictable.
#define NOP1 asm volatile( "nop\n" )
#define NOP2 NOP1; NOP1

#define WAITSEQ NOP2

#define RXBYTES 32

void setDatPinsInput() {
  pinMode( D0, INPUT );
  pinMode( D1, INPUT );
  pinMode( D2, INPUT );
  pinMode( D3, INPUT );
  pinMode( D4, INPUT );
  pinMode( D5, INPUT );
  pinMode( D6, INPUT );
  pinMode( D7, INPUT );
}

void setDatPinsOutput() {
  pinMode( D0, OUTPUT );
  pinMode( D1, OUTPUT );
  pinMode( D2, OUTPUT );
  pinMode( D3, OUTPUT );
  pinMode( D4, OUTPUT );
  pinMode( D5, OUTPUT );
  pinMode( D6, OUTPUT );
  pinMode( D7, OUTPUT );
}

void setup() {
  Serial.begin( 250000 );
  
  // Pin directions
  pinMode( A0, OUTPUT );
  pinMode( A1, OUTPUT );
  pinMode( A2, OUTPUT );
  pinMode( A3, OUTPUT );
  pinMode( A4, OUTPUT );
  pinMode( A5, OUTPUT );
  pinMode( A6, OUTPUT );
  pinMode( A7, OUTPUT );
  pinMode( A8, OUTPUT );
  pinMode( A9, OUTPUT );
  pinMode( A10, OUTPUT );
  pinMode( A11, OUTPUT );
  pinMode( A12, OUTPUT );
  pinMode( A13, OUTPUT );
  pinMode( A14, OUTPUT );
  pinMode( A15, OUTPUT );
  pinMode( A16, OUTPUT );
  pinMode( A17, OUTPUT );
  pinMode( A18, OUTPUT );
  pinMode( A19, OUTPUT );
  pinMode( A20, OUTPUT );

  // First, set them to input
  setDatPinsInput();

  pinMode( nWE, OUTPUT );
  pinMode( nRP, OUTPUT );
  pinMode( nRB, INPUT );
  pinMode( CE, OUTPUT );
  pinMode( nOE, OUTPUT );

  digitalWrite( nWE, 1 );
  digitalWrite( nRP, 1 );
  digitalWrite( CE, 0 );
  digitalWrite( nOE, 1 );
}

void setAddrPins( u32 addr ) {  
  // A0 to A7
  PORTA = (u8) ( (addr >> 0) & 0b11111111 );
  
  // A8 to A15
  u8 tmp = 0;
  for ( int i = 0; i < 8; i ++ ) {
    if ( addr & ( 1l << ( i + 8 ) ) ) {
      tmp |= ( 1 << ( 7 - i ) );
    }
  }
  PORTC = tmp;
  
  // A16 to A20
  if ( addr & ( 1l << 16 ) ) {
    PORTD |= ( 1 << 7 );
  } else {
    PORTD &= ~( 1l << 7 );
  }
  
  if ( addr & ( 1l << 17 ) ) {
    PORTG |= ( 1 << 2 );
  } else {
    PORTG &= ~( 1 << 2 );
  }
  
  if ( addr & ( 1l << 18 ) ) {
    PORTG |= ( 1 << 1 );
  } else {
    PORTG &= ~( 1 << 1 );
  }
  
  if ( addr & ( 1l << 19 ) ) {
    PORTG |= ( 1 << 0 );
  } else {
    PORTG &= ~( 1 << 0 );
  }
  
  if ( addr & ( 1l << 20 ) ) {
    PORTL |= ( 1 << 7 );
  } else {
    PORTL &= ~( 1 << 7 );
  }
  
}

void setDataOutput( unsigned dat ) {
  // Pin 43
  if ( dat & ( 1 << 0 ) ) {
    PORTL |= ( 1 << 6 );
  } else {
    PORTL &= ~( 1 << 6 );
  }
  
  // Pin 44
  if ( dat & ( 1 << 1 ) ) {
    PORTL |= ( 1 << 5 );
  } else {
    PORTL &= ~( 1 << 5 );
  }
  
  // Pin 45
  if ( dat & ( 1 << 2 ) ) {
    PORTL |= ( 1 << 4 );
  } else {
    PORTL &= ~( 1 << 4 );
  }
  
  // Pin 46
  if ( dat & ( 1 << 3 ) ) {
    PORTL |= ( 1 << 3 );
  } else {
    PORTL &= ~( 1 << 3 );
  }
  
  // Pin 47
  if ( dat & ( 1 << 4 ) ) {
    PORTL |= ( 1 << 2 );
  } else {
    PORTL &= ~( 1 << 2 );
  }
  
  // Pin 48
  if ( dat & ( 1 << 5 ) ) {
    PORTL |= ( 1 <<  1);
  } else {
    PORTL &= ~( 1 << 1 );
  }
  
  // Pin 49
  if ( dat & ( 1 << 6 ) ) {
    PORTL |= ( 1 << 0 );
  } else {
    PORTL &= ~( 1 << 0 );
  }
  
  // Pin 50
  if ( dat & ( 1 << 7 ) ) {
    PORTB |= ( 1 << 3 );
  } else {
    PORTB &= ~( 1 << 3 );
  }
  
}

// Dump game.
void doDump() {
  Serial.println( "DoDump" );
  setDatPinsInput();
  
  // Activate chip.
  digitalWrite( CE, 1 );
  digitalWrite( nOE, 0 );
  digitalWrite( nWE, 1 );
  digitalWrite( nRP, 1 );
  
  // Wait.
  delayMicroseconds( 10 );
  Serial.println( "waited" );

  // Start dump.
  for ( u32 i = OFFSET; i < MAXADDR + OFFSET; ++i ) {
    //Serial.println( i );
    setAddrPins( i );
    // Wait.
    WAITSEQ;
    // Read.
    char dat = digitalRead( D0 );
    dat |= digitalRead( D1 ) << 1;
    dat |= digitalRead( D2 ) << 2;
    dat |= digitalRead( D3 ) << 3;
    dat |= digitalRead( D4 ) << 4;
    dat |= digitalRead( D5 ) << 5;
    dat |= digitalRead( D6 ) << 6;
    dat |= digitalRead( D7 ) << 7;

    // Send it.
    Serial.write( dat );
  }

  // Turn off again.
  digitalWrite( CE, 0 );
  digitalWrite( nOE, 1 );
}

void writeToAddress( u32 addr, u8 data ) {
  // Assuming nOE is HIGH
  
  // Set the address.
  setAddrPins( addr );
  // Set the data.
  setDataOutput( data );

  // Activate chip.
  digitalWrite( CE, 1 );
  digitalWrite( nWE, 0 );

  WAITSEQ;

  // Deactivate.
  digitalWrite( nWE, 1 );
  digitalWrite( CE, 0 );
}

void unlockBypass() {
  digitalWrite( CE, 1 );
  digitalWrite( nOE, 1 );
  digitalWrite( nWE, 1 );

  setDatPinsOutput();
  
  // First command.
  writeToAddress( 0xAAA, 0xAA );
  WAITSEQ;

  // 2nd command.
  writeToAddress( 0x555, 0x55 );
  WAITSEQ;

  // 3rd command.
  writeToAddress( 0xAAA, 0x20 );
  WAITSEQ;

  setDatPinsInput();
  digitalWrite( nOE, 1 );
  digitalWrite( nWE, 1 );
}

void unlockBypassReset() {
  digitalWrite( CE, 1 );
  digitalWrite( nOE, 1 );
  digitalWrite( nWE, 1 );

  setDatPinsOutput();
  
  // First command (addr don't care)
  writeToAddress( 0x111, 0x90 );
  WAITSEQ;

  // 2nd command. (addr don't care)
  writeToAddress( 0x111, 0x00 );
  WAITSEQ;

  setDatPinsInput();
  digitalWrite( nOE, 1 );
  digitalWrite( nWE, 1 );
}

void doBypassProgram( u32 addr, u8 data ) {
  digitalWrite( CE, 1 );
  digitalWrite( nOE, 1 );
  digitalWrite( nWE, 1 );

  setDatPinsOutput();
  
  // First command (addr don't care)
  writeToAddress( 0x111, 0xA0 );
  WAITSEQ;

  // 2nd command. (addr don't care)
  writeToAddress( addr, data );
  WAITSEQ;

  setDatPinsInput();
  digitalWrite( nOE, 1 );
  digitalWrite( nWE, 1 );
}

void doChipErase() {
  digitalWrite( CE, 1 );
  digitalWrite( nOE, 1 );
  digitalWrite( nWE, 1 );
  
  setDatPinsOutput();
  
  // First command.
  writeToAddress( 0xAAA, 0xAA );
  WAITSEQ;
  
  // Second command.
  writeToAddress( 0x555, 0x55 );
  WAITSEQ;

  // 3rd command.
  writeToAddress( 0xAAA, 0x80 );
  WAITSEQ;

  // 4th command.
  writeToAddress( 0xAAA, 0xAA );
  WAITSEQ;

  // 5th command.
  writeToAddress( 0x555, 0x55 );
  WAITSEQ;

  // 6th command.
  writeToAddress( 0xAAA, 0x10 );

  // Wait up to 120 s
  delay( 120000 );

  setDatPinsInput();
  digitalWrite( nOE, 1 );
  digitalWrite( nWE, 1 );
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
  // First set data pins to output.
  digitalWrite( nRP, 1 );

  digitalWrite( CE, 1 );
  digitalWrite( nOE, 1 );
  digitalWrite( nWE, 1 );

  setDatPinsOutput();

  // Wait until we receive the first byte.
  u32 nrBytes = 0;

  while ( nrBytes < MAXADDR ) {
    if ( Serial.available() >= RXBYTES ) {
      for ( int i = 0; i < RXBYTES; ++i ) {
        char b = Serial.read();
  
        doProgram( nrBytes, b );
        ++nrBytes;
      }

      // Send conf.
      char conf = 1;
      Serial.write( conf );
      Serial.flush();
    }
  }

    // Turn off again.
  digitalWrite( CE, 0 );
  digitalWrite( nOE, 1 );
  digitalWrite( nWE, 1 );
  
  setDatPinsInput();
}

void doFlashBypass() {
  // First set data pins to output.
  digitalWrite( CE, 1 );
  digitalWrite( nOE, 1 );
  digitalWrite( nWE, 1 );

  setDatPinsOutput();

  // Wait until we receive the first byte.
  u32 nrBytes = 0;

  // Do unlock bypass
  unlockBypass();

  while ( nrBytes < MAXADDR ) {
    if ( Serial.available() >= RXBYTES ) {
      for ( int i = 0; i < RXBYTES; ++i ) {
        char b = Serial.read();
  
        doBypassProgram( nrBytes, b );
        ++nrBytes;
      }

      // Send conf.
      char conf = 1;
      Serial.write( conf );
      Serial.flush();
    }
  }

    // Turn off again.
  digitalWrite( CE, 0 );
  digitalWrite( nOE, 1 );
  digitalWrite( nWE, 1 );
  
  setDatPinsInput();

  // Reset unlock bypass
  unlockBypassReset();
}

void loop() {
  
  if ( Serial.available() ) {
    char rx = Serial.read();
    if ( rx == 42 ) {
      doDump();
      
    } else if ( rx == 82 ) {
      // Write.
      doFlashBypass();

    } else if ( rx == 14 ) {
      unlockBypassReset();
      
    } else if ( rx == 123 ) {
      doChipErase();
      Serial.write( 42 );
    }
  }
}
