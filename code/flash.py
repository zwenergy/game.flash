#!/usr/bin/env python3

# A script to flash the game.flash cartridge.
# Author: zwenergy

import serial
import time
import math
import sys

# Some definitions up first.
BYTES = 32
BAUD = 250000
CMDERASE = 123
CMDFLASH = 82
CMDUNLOCKBYPASSRESET = 14
INCRMOD = 1024

# Get the command line.
if ( not( len( sys.argv ) == 3 or len( sys.argv ) == 4 ) ):
  print( "Wrong usage. Use of script:" )
  print( "flash.py ROMFILE COMPORT [MIRROR]" )
  print( "\nWhere [MIRROR] is optional and can be either TRUE or FALSE. By default it is turned off." )
  exit( 1 )


romfile = sys.argv[ 1 ]
comport = sys.argv[ 2 ]
mirror = False

if ( len( sys.argv ) == 4 ) :
  tmp = sys.argv[ 3 ]
  if ( tmp.lower() == "true" ):
    mirror = True
  elif ( tmp.lower() == "false" ):
    mirror = False
  else:
    print( "Wrong usage. Optional command MIRROR can be either TRUE or FALSE." )
    print( "Given argument: " + tmp )
    exit( 1 )

ser = serial.Serial( comport, BAUD,
  bytesize=serial.EIGHTBITS,
  parity=serial.PARITY_NONE,
  stopbits=serial.STOPBITS_ONE,
  timeout=0,
  xonxoff=0,
  rtscts=0 )

time.sleep( 1 )

print( "Opened COM port." )

# Reset the arduino by toggling the DTR line.
print( "Resetting Arduino." )
ser.dtr = False;
time.sleep( 1 );
ser.flushInput()
ser.dtr = True;
time.sleep( 1 );

# Send unlock bypass reset signal (in case flashing was interrupted before)
vals = [ CMDUNLOCKBYPASSRESET ]
ser.write( bytearray( vals ) )
time.sleep( 0.5 )

print( "Start erasing chip contents." )

# Send erase command.
vals = [ CMDERASE ]
ser.write( bytearray( vals ) )
time.sleep( 1 )

# Wait until done.
while( not ser.in_waiting ):
    # Dummy.
    a = 0

# Will send exactly one byte.
r = ser.read()

print( "Chip erased." )

f = open( romfile, "rb" )
rom = f.read()

i = 0
romLen = len( rom )
maxAddr = len( rom )

if ( mirror ):
  maxAddr = 2097152
  if ( ( romLen & ( romLen - 1 ) == 0) and romLen != 0 ):
    print( "Mirror on and ROM is power 2." )
  else:
    print( "Mirror on and ROM is NOT power 2, verify if this is correct." )

print( "Starting to flash." )

# Send flash mode command.
vals = [ CMDFLASH ]
ser.write( bytearray( vals ) )
time.sleep( 1 )

while ( i < maxAddr ):
  tmp = []
  for jj in range( BYTES ):
    if ( i < maxAddr ):
      tmp.append( rom[ i % romLen ] )
      i += 1
    else:
      break
      
  ser.write( bytearray( tmp ) )
  
  if ( i % INCRMOD == 0 ):
    print( "Wrote " + str( i ) + " of " + str( maxAddr ) )
  
  while( not ser.in_waiting ):
    # Dummy
    a = 0

  r = ser.read()

print( "Done flashing." )

f.close()
ser.close()
