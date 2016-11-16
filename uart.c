#include <p18cxxx.h>
#include "uart.h"
#include "HardwareProfile.h"

#pragma udata UART_BUFFER = UARTBUFFERADDR
char uRxBuffer[URXBUFFERSIZE];         // vételi buffer
char uTxBuffer[UTXBUFFERSIZE];         // adás buffer

#pragma udata

// vétel
//char uRxBufferLost = 0;
volatile unsigned char  uRxStart = 0, uRxLength = 0, uRxBufFree = URXBUFFERSIZE;

// adás
volatile char uTxBufferLost = 0;
volatile unsigned char  uTxStart = 0, uTxLength = 0, uTxBufFree = UTXBUFFERSIZE;

#define TRIS_(p, m)            TRIS ## p ## bits.TRIS ## p ## m
#define LAT_(p, m)             LAT ## p ## bits.LAT ## p ## m
#define PORT_(p, m)            PORT ## p ## bits.R ## p ## m

#define IOIN(x)                x(TRIS_) = 1
#define IOOUT(x)               x(TRIS_) = 0
#define SET(x)                 x(LAT_) = 1
#define CLR(x)                 x(LAT_) = 0
#define GET(x)                 x(PORT_)

//=============================================================================
void UartInit(void)
{
  #define BAUDDIV (SystemClock/BAUDRATE)

  #if BAUDDIV < (65536*4)
  #define BRG_DIV          4
  #define CLKSEL           1
  #elif BAUDDIV < (65536*16)
  #define BRG_DIV         16
  #define CLKSEL           0
  #else
  #error "PIC18: ez a BAUDRATE nem elõállítható"
  #endif // BAUDDIV

  #define BAUDRATEREG ((SystemClock+((BRG_DIV/2)*BAUDRATE))/BRG_DIV/BAUDRATE-1)
  SPBRG = BAUDRATEREG;                  // alsó byte;
  SPBRGH = BAUDRATEREG >> 8;            // felsõ byte
  TXSTAbits.BRGH = CLKSEL;              // lo/hi speed
  BAUDCONbits.BRG16 = 1;                // 16 bites
  IOIN(UARTRX); IOOUT(UARTTX);          // RX bemenet, TX kimenet
  RCSTAbits.SPEN = 1;                   // soros port eng
  TXSTAbits.SYNC = 0;                   // aszinkron mód

  #if (RXPR18 == -1 && TXPR18 != -1) || (TXPR18 == -1 && RXPR18 != -1)
  #error "Hiba: prioritás nélküli használat esetén az RX és a TX-nek is annak kell lennie!"
  #endif

  #if RXPR18 == -1

  #elif RXPR18 == 0
  IPR1bits.RCIP = 0;                    // alacsony prioritás
  #elif RXPR18 == 1
  IPR1bits.RCIP = 1;                    // magas prioritás
  #endif

  #if TXPR18 == -1

  #elif TXPR18 == 0
  IPR1bits.TXIP = 0;                    // alacsony prioritás
  RCONbits.IPEN = 1;                    // kétszintû IRQ eng
  #elif TXPR18 == 1
  IPR1bits.TXIP = 1;                    // magas prioritás
  RCONbits.IPEN = 1;                    // kétszintû IRQ eng
  #endif
   
  TXSTAbits.TXEN = 1;                   // adás engedélyezés
  RCSTAbits.CREN = 1;                   // vétel engedélyezés
  PIE1bits.RCIE  = 1;                   // vétel IRQ eng 
  // adás megszakítás engedélyezést csak forgalmazáskor kell megtenni, ha már TXREG nem szabad
  
//  INTCONbits.GIEL = 1;                  // globális IRQ eng
//  INTCONbits.GIEH = 1;                  // globális IRQ eng
  INTCONbits.PEIE = 1;                  // periféria IRQ eng
}

//=============================================================================
// Rx pufferbõl egy byte-ot kiolvas, törlõdik is a pufferbõl
unsigned char UartRx(void)
{
  unsigned char ch;
  ch = 0;
  PIE1bits.RCIE = 0;                    // megszakítás tiltás (hogy ne változhasson meg közben a puffermutató)
  if(uRxLength)                         // van még adat a pufferben ?
  {
    ch = uRxBuffer[uRxStart++];
    uRxStart &= (URXBUFFERSIZE - 1);
    uRxLength--; uRxBufFree++;
  }
  PIE1bits.RCIE = 1;                    // megszakítás engedélyezés
  return ch;
}

//=============================================================================
// Rx pufferbõl úgy olvas ki egy byte-ot hog nem törlõdik a pufferbõl
unsigned char UartRxTop(void)
{
  return uRxBuffer[uRxStart];
}

//=============================================================================
void UartRxProcess(void)
{
  unsigned char c;
  c = RCREG;
  if(RCSTAbits.FERR | RCSTAbits.OERR)
  {
    RCSTAbits.CREN = 0; //clear error (if any)
    RCSTAbits.CREN = 1; //Enables Receiver
  }
  else
  {
    if (uRxLength != (URXBUFFERSIZE - 1)) // van még hely a pufferben ?
    {   
      uRxBuffer[(uRxStart + uRxLength) & (URXBUFFERSIZE - 1)] = c; // egy karakter megy a pufferbe
      uRxLength++; uRxBufFree--;
    }
  }
  PIR1bits.RCIF = 0;
}

#ifdef UARTDEBUG
//=============================================================================
void UartRxWrite(unsigned char ch)
{
  if (uRxLength != (URXBUFFERSIZE - 1)) // van még hely a pufferben ?
  {   
    uRxBuffer[(uRxStart + uRxLength) & (URXBUFFERSIZE - 1)] = ch; // egy karakter megy a pufferbe
    uRxLength++; uRxBufFree--;
  }
}
#endif

//=============================================================================
void UartTx(unsigned char ch)
{
  if (!uTxLength && PIR1bits.TXIF)      // buffer üres, és TXREG írhtó
  {
    TXREG = ch;
  }
  else
  {
    PIE1bits.TXIE = 0;                  // TX IRQ tiltás (hogy a megszakítás ne nyúljon hozzá a puffermutatókhoz)
    if(uTxBufFree)                      // van még hely a pufferben ?
    {   
      uTxBuffer[(uTxStart + uTxLength) & (UTXBUFFERSIZE - 1)] = ch; // egy karakter megy a pufferbe
      uTxLength++; uTxBufFree--;
    }
    else
      uTxBufferLost = 1;                // itt jelezzük hogy buffertúlcsordulás miatt adatvesztés történt
    PIE1bits.TXIE = 1;                  // TX IRQ engedélyezés
  }
}
//=============================================================================
void UartTxProcess(void)
{
  PIR1bits.TXIF = 0;                    // megszakítás forrás jelzés törlése
  if(!uTxLength)                        // ha már nincs mit adni
    PIE1bits.TXIE = 0;                  // TX IRQ tiltás
  else
  {
    TXREG = uTxBuffer[uTxStart++];      // TXREG = adat mehet
    uTxStart &= (UTXBUFFERSIZE - 1);
    uTxLength--; uTxBufFree++;
  }
}
