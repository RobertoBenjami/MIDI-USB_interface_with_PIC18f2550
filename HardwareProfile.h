#ifndef __HARDWAREPROFILE_H
#define __HARDWAREPROFILE_H

#define SystemClock 48000000
#define CpuClock    48000000
#define BAUDRATE       31250

#ifdef   __18F14K50_H

#define  USBBUFFERADDR      0x260
#define  UARTBUFFERADDR     0x180

// A/D (minden láb digitális I/O)
#define ADINIT      ANSEL = 0; ANSELH = 0

// USB
#define self_power         1
#define USB_BUS_SENSE      1

// LEDEK
#define LEDUSB(f)    f(C, 0)
#define LEDIN(f)     f(C, 3)
#define LEDOUT(f)    f(C, 4)

// UART I/O (MIDI IN, MIDI OUT)
#define UARTRX(f)    f(B, 5)
#define UARTTX(f)    f(B, 7)
#define UTXBUFFERSIZE     64
#define URXBUFFERSIZE     64

// SPI debugger (opcionális, csak a fejlesztéshez kell)
#define SPIDEBUGCLK(f) f(B, 4)
#define SPIDEBUGDT(f)  f(B, 6)

// Boot loader gomb
#define BOOTLOADER(f)  f(B, 5)

#endif // #ifdef   __18F14K50_H

#ifdef   __18F2550_H

#define  USBBUFFERADDR      0x500
#define  UARTBUFFERADDR     0x700

// A/D (minden láb digitális I/O)
#define ADINIT      ADCON1 = 0x0F

// USB
#define self_power         1
#define USB_BUS_SENSE      1

// LEDEK
#define LEDUSB(f)    f(B, 2)
#define LEDIN(f)     f(B, 3)
#define LEDOUT(f)    f(B, 4)

// UART I/O (MIDI IN, MIDI OUT)
#define UARTRX(f)    f(C, 7)
#define UARTTX(f)    f(C, 6)
#define UTXBUFFERSIZE    128
#define URXBUFFERSIZE    128

// SPI debugger (opcionális, csak a fejlesztéshez kell)
#define SPIDEBUGCLK(f) f(B, 6)
#define SPIDEBUGDT(f)  f(B, 7)

// Boot loader gomb
#define BOOTLOADER(f)  f(C, 1)

#endif // __18F2550_H

#endif  // __HARDWAREPROFILE_H
