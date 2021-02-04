# arduino-xmodem

Goodle code has been closed for some time, I just decided to finally move 
my stuff to GitHib. I guess I am late by a few years, but still it is 
better to do it later, rather than never.

XMODEM is well known, a little bit old, but still working protocol for 
data transfer over serial connection.I started this project because 
I needed to transfer some binary files from a PC to Arduino compatible board. 
I could not manage to find any existing solution and decided to provide 
simple XMODEM implementation. This implementation is done from scratch and is coded in C++.

he code is organized around singe C++ class: XModem. The usage is quite simple - 
just make instance of XModem and call methods transmit() or receive(). XMomdem doesn't 
know which function to use in order to send or receive data. So they must be set, 
when construct XModem object: 

int recvChar(int msDelay) { ... } 
void sendChar(char sym) { ... }
XModem modem(recvChar, sendChar);

In order to utilize Serial library of Arduino the functions recvChar and sendChar can look 
like this: 
int recvChar(int msDelay) { 
  int cnt = 0; 
  while(cnt < msDelay) { 
    if(Serial.available() > 0) 
      return Serial.read();
    delay(1); cnt++; 
  } 
 return -1; 
} 
void sendChar(char sym) { 
  Serial.write(sym); 
}

The transmit function is quite straightforward, but in receive route there is 
little complication. XMODEM protocol uses predefined timeouts when reading data 
from serial connection. So we need routine that must read symbol if it is available 
or wait for it specified amount of time. If no symbol is received within this 
period, the function must return -1. Much better approach is to use serialEvent() 
handler to point when symbol is available. Data transmission is accomplished by 
using XModem's methods receive() or transmit()

modem.transmit(); ... modem.receive();

Of course someone should handle actual data. This is done by using special data 
handler which should be specified when constructing xmodem object:

bool dataHandler(unsigned long no, char* data, int size) { ... }
XModem modem(recvChar, sendChar, dataHandler); 

dataHandler is called in following cases: * when XModem is used as receiver i.e. 
receive() method is called, then dataHandler is called when there is available data. 
The first parameter holds the sequence number of data chunk received. The first 
block is with number 1, next is 2 and so on. The data is pointer to a buffer(char *) 
with length of 128 holding received data. The size is actual size of receive data. 
If data reception is successful, the dataHandler must return true, otherwise false.

when XModem is used as transmitter i.e. transmit method is called, then dataHandler 
is called before sending data. It is responsible for providing data to be send. The 
first parameter indicate the number of block which is going to be transmitted. Note 
that currently all block are with fixed size of 128. The data parameter is pointer to 
buffer (char *) which dataHandler must fill. Be aware that buffer is only 128 bytes 
long! The third parameter contains number of requested bytes. Currently it is fixed 
to 128. End of data transmission is indicated by returning false.
