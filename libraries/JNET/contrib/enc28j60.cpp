// Microchip ENC28J60 Ethernet Interface Driver
// Author: Guido Socher
// Copyright: GPL V2
// 
// Based on the enc28j60.c file from the AVRlib library by Pascal Stang.
// For AVRlib See http://www.procyonengineering.com/
// Used with explicit permission of Pascal Stang.
//
// Mods bij jcw, 2010-05-20
// Mods by jrv, 2010-10-14

#include "enc28j60.h"
#include <WProgram.h>

// ENC28J60 Control Registers
// Control register definitions are a combination of address,
// bank number, and Ethernet/MAC/PHY indicator bits.
// - Register address        (bits 0-4)
// - Bank number        (bits 5-6)
// - MAC/PHY indicator        (bit 7)
#define ADDR_MASK        0x1F
#define BANK_MASK        0x60
#define SPRD_MASK        0x80
// All-bank registers
#define EIE              0x1B
#define EIR              0x1C
#define ESTAT            0x1D
#define ECON2            0x1E
#define ECON1            0x1F
// Bank 0 registers
#define ERDPTL           (0x00|0x00)
#define ERDPTH           (0x01|0x00)
#define EWRPTL           (0x02|0x00)
#define EWRPTH           (0x03|0x00)
#define ETXSTL           (0x04|0x00)
#define ETXSTH           (0x05|0x00)
#define ETXNDL           (0x06|0x00)
#define ETXNDH           (0x07|0x00)
#define ERXSTL           (0x08|0x00)
#define ERXSTH           (0x09|0x00)
#define ERXNDL           (0x0A|0x00)
#define ERXNDH           (0x0B|0x00)
#define ERXRDPTL         (0x0C|0x00)
#define ERXRDPTH         (0x0D|0x00)
#define ERXWRPTL         (0x0E|0x00)
#define ERXWRPTH         (0x0F|0x00)
#define EDMASTL          (0x10|0x00)
#define EDMASTH          (0x11|0x00)
#define EDMANDL          (0x12|0x00)
#define EDMANDH          (0x13|0x00)
#define EDMADSTL         (0x14|0x00)
#define EDMADSTH         (0x15|0x00)
#define EDMACSL          (0x16|0x00)
#define EDMACSH          (0x17|0x00)
// Bank 1 registers
#define EHT0             (0x00|0x20)
#define EHT1             (0x01|0x20)
#define EHT2             (0x02|0x20)
#define EHT3             (0x03|0x20)
#define EHT4             (0x04|0x20)
#define EHT5             (0x05|0x20)
#define EHT6             (0x06|0x20)
#define EHT7             (0x07|0x20)
#define EPMM0            (0x08|0x20)
#define EPMM1            (0x09|0x20)
#define EPMM2            (0x0A|0x20)
#define EPMM3            (0x0B|0x20)
#define EPMM4            (0x0C|0x20)
#define EPMM5            (0x0D|0x20)
#define EPMM6            (0x0E|0x20)
#define EPMM7            (0x0F|0x20)
#define EPMCSL           (0x10|0x20)
#define EPMCSH           (0x11|0x20)
#define EPMOL            (0x14|0x20)
#define EPMOH            (0x15|0x20)
#define EWOLIE           (0x16|0x20)
#define EWOLIR           (0x17|0x20)
#define ERXFCON          (0x18|0x20)
#define EPKTCNT          (0x19|0x20)
// Bank 2 registers
#define MACON1           (0x00|0x40|0x80)
#define MACON2           (0x01|0x40|0x80)
#define MACON3           (0x02|0x40|0x80)
#define MACON4           (0x03|0x40|0x80)
#define MABBIPG          (0x04|0x40|0x80)
#define MAIPGL           (0x06|0x40|0x80)
#define MAIPGH           (0x07|0x40|0x80)
#define MACLCON1         (0x08|0x40|0x80)
#define MACLCON2         (0x09|0x40|0x80)
#define MAMXFLL          (0x0A|0x40|0x80)
#define MAMXFLH          (0x0B|0x40|0x80)
#define MAPHSUP          (0x0D|0x40|0x80)
#define MICON            (0x11|0x40|0x80)
#define MICMD            (0x12|0x40|0x80)
#define MIREGADR         (0x14|0x40|0x80)
#define MIWRL            (0x16|0x40|0x80)
#define MIWRH            (0x17|0x40|0x80)
#define MIRDL            (0x18|0x40|0x80)
#define MIRDH            (0x19|0x40|0x80)
// Bank 3 registers
#define MAADR1           (0x00|0x60|0x80)
#define MAADR0           (0x01|0x60|0x80)
#define MAADR3           (0x02|0x60|0x80)
#define MAADR2           (0x03|0x60|0x80)
#define MAADR5           (0x04|0x60|0x80)
#define MAADR4           (0x05|0x60|0x80)
#define EBSTSD           (0x06|0x60)
#define EBSTCON          (0x07|0x60)
#define EBSTCSL          (0x08|0x60)
#define EBSTCSH          (0x09|0x60)
#define MISTAT           (0x0A|0x60|0x80)
#define EREVID           (0x12|0x60)
#define ECOCON           (0x15|0x60)
#define EFLOCON          (0x17|0x60)
#define EPAUSL           (0x18|0x60)
#define EPAUSH           (0x19|0x60)

// ENC28J60 ERXFCON Register Bit Definitions
#define ERXFCON_UCEN     0x80
#define ERXFCON_ANDOR    0x40
#define ERXFCON_CRCEN    0x20
#define ERXFCON_PMEN     0x10
#define ERXFCON_MPEN     0x08
#define ERXFCON_HTEN     0x04
#define ERXFCON_MCEN     0x02
#define ERXFCON_BCEN     0x01
// ENC28J60 EIE Register Bit Definitions
#define EIE_INTIE        0x80
#define EIE_PKTIE        0x40
#define EIE_DMAIE        0x20
#define EIE_LINKIE       0x10
#define EIE_TXIE         0x08
#define EIE_WOLIE        0x04
#define EIE_TXERIE       0x02
#define EIE_RXERIE       0x01
// ENC28J60 EIR Register Bit Definitions
#define EIR_PKTIF        0x40
#define EIR_DMAIF        0x20
#define EIR_LINKIF       0x10
#define EIR_TXIF         0x08
#define EIR_WOLIF        0x04
#define EIR_TXERIF       0x02
#define EIR_RXERIF       0x01
// ENC28J60 ESTAT Register Bit Definitions
#define ESTAT_INT        0x80
#define ESTAT_LATECOL    0x10
#define ESTAT_RXBUSY     0x04
#define ESTAT_TXABRT     0x02
#define ESTAT_CLKRDY     0x01
// ENC28J60 ECON2 Register Bit Definitions
#define ECON2_AUTOINC    0x80
#define ECON2_PKTDEC     0x40
#define ECON2_PWRSV      0x20
#define ECON2_VRPS       0x08
// ENC28J60 ECON1 Register Bit Definitions
#define ECON1_TXRST      0x80
#define ECON1_RXRST      0x40
#define ECON1_DMAST      0x20
#define ECON1_CSUMEN     0x10
#define ECON1_TXRTS      0x08
#define ECON1_RXEN       0x04
#define ECON1_BSEL1      0x02
#define ECON1_BSEL0      0x01
// ENC28J60 MACON1 Register Bit Definitions
#define MACON1_LOOPBK    0x10
#define MACON1_TXPAUS    0x08
#define MACON1_RXPAUS    0x04
#define MACON1_PASSALL   0x02
#define MACON1_MARXEN    0x01
// ENC28J60 MACON2 Register Bit Definitions
#define MACON2_MARST     0x80
#define MACON2_RNDRST    0x40
#define MACON2_MARXRST   0x08
#define MACON2_RFUNRST   0x04
#define MACON2_MATXRST   0x02
#define MACON2_TFUNRST   0x01
// ENC28J60 MACON3 Register Bit Definitions
#define MACON3_PADCFG2   0x80
#define MACON3_PADCFG1   0x40
#define MACON3_PADCFG0   0x20
#define MACON3_TXCRCEN   0x10
#define MACON3_PHDRLEN   0x08
#define MACON3_HFRMLEN   0x04
#define MACON3_FRMLNEN   0x02
#define MACON3_FULDPX    0x01
// ENC28J60 MICMD Register Bit Definitions
#define MICMD_MIISCAN    0x02
#define MICMD_MIIRD      0x01
// ENC28J60 MISTAT Register Bit Definitions
#define MISTAT_NVALID    0x04
#define MISTAT_SCAN      0x02
#define MISTAT_BUSY      0x01

// PHY registers
#define PHCON1           0x00
#define PHSTAT1          0x01
#define PHHID1           0x02
#define PHHID2           0x03
#define PHCON2           0x10
#define PHSTAT2          0x11
#define PHIE             0x12
#define PHIR             0x13
#define PHLCON           0x14

// ENC28J60 PHY PHCON1 Register Bit Definitions
#define PHCON1_PRST      0x8000
#define PHCON1_PLOOPBK   0x4000
#define PHCON1_PPWRSV    0x0800
#define PHCON1_PDPXMD    0x0100
// ENC28J60 PHY PHSTAT1 Register Bit Definitions
#define PHSTAT1_PFDPX    0x1000
#define PHSTAT1_PHDPX    0x0800
#define PHSTAT1_LLSTAT   0x0004
#define PHSTAT1_JBSTAT   0x0002
// ENC28J60 PHY PHCON2 Register Bit Definitions
#define PHCON2_FRCLINK   0x4000
#define PHCON2_TXDIS     0x2000
#define PHCON2_JABBER    0x0400
#define PHCON2_HDLDIS    0x0100

// ENC28J60 Packet Control Byte Bit Definitions
#define PKTCTRL_PHUGEEN  0x08
#define PKTCTRL_PPADEN   0x04
#define PKTCTRL_PCRCEN   0x02
#define PKTCTRL_POVERRIDE 0x01

// SPI operation codes
#define ENC28J60_READ_CTRL_REG       0x00
#define ENC28J60_READ_BUF_MEM        0x3A
#define ENC28J60_WRITE_CTRL_REG      0x40
#define ENC28J60_WRITE_BUF_MEM       0x7A
#define ENC28J60_BIT_FIELD_SET       0x80
#define ENC28J60_BIT_FIELD_CLR       0xA0
#define ENC28J60_SOFT_RESET          0xFF

// The RXSTART_INIT must be zero. See Rev. B4 Silicon Errata point 5.
// Buffer boundaries applied to internal 8K ram
// the entire available packet buffer space is allocated

// start with recbuf at 0 (must be zero! assumed in code)
#define RXSTART_INIT     0x0
// receive buffer end, must be odd number:
#define RXSTOP_INIT      (0x1FFF-0x0600)
// start TX buffer after RXSTOP_INIT with space for one full ethernet frame (~1500 bytes)
#define TXSTART_INIT     (0x1FFF-0x0600+1)
// stp TX buffer at end of mem
#define TXSTOP_INIT      0x1FFF

// max frame length which the conroller will accept:
// (note: maximum ethernet frame length would be 1518)
#define        MAX_FRAMELEN        1500        
static byte Enc28j60Bank;
static int16_t gNextPacketPtr;

//AT: Use pin 10 for nuelectronics.com compatible ethershield.
#define ENC28J60_CONTROL_CS 10

void ENC28J60::spiInit() {
    const byte SPI_SS   = 10;
    const byte SPI_MOSI	= 11;
    const byte SPI_MISO	= 12;
    const byte SPI_SCK	= 13;
    
    pinMode(SPI_SS, OUTPUT);
    pinMode(SPI_MOSI, OUTPUT);
	pinMode(SPI_SCK, OUTPUT);	
	pinMode(SPI_MISO, INPUT);
	
	digitalWrite(SPI_MOSI, HIGH);
	digitalWrite(SPI_MOSI, LOW);
	digitalWrite(SPI_SCK, LOW);

    SPCR = (1<<SPE)|(1<<MSTR);
    // SPCR = (1<<SPE)|(1<<MSTR) | (1<<SPR0);
	SPSR |= (1<<SPI2X);
}

static void enableChip() {
    cli();
#if ENC28J60_CONTROL_CS == 8
    bitClear(PORTB, 0); // much faster
#else
    digitalWrite(ENC28J60_CONTROL_CS, LOW);
#endif
}

static void disableChip() {
#if ENC28J60_CONTROL_CS == 8
    bitSet(PORTB, 0); // much faster
#else
    digitalWrite(ENC28J60_CONTROL_CS, HIGH);
#endif
    sei();
}

static void sendSPI(byte data) {
    SPDR = data;
    while (!(SPSR&(1<<SPIF)))
        ;
}

static byte ReadOp(byte op, byte address) {
    enableChip();
    sendSPI(op | (address & ADDR_MASK));
    sendSPI(0x00);
    if (address & 0x80)
        sendSPI(0x00);
    byte result = SPDR;
    disableChip();
    return result;
}

static void WriteOp(byte op, byte address, byte data) {
    enableChip();
    sendSPI(op | (address & ADDR_MASK));
    sendSPI(data);
    disableChip();
}

static void ReadBuffer(word len, byte* data) {
    enableChip();
    sendSPI(ENC28J60_READ_BUF_MEM);
    while (len--) {
        sendSPI(0x00);
        *data++ = SPDR;
    }
    disableChip();
    *data='\0';
}

static word ReadBufferWord() {
    word result;
    ReadBuffer(2, (byte*) &result);
    return result;
}

static void WriteBuffer(word len, byte* data) {
    enableChip();
    sendSPI(ENC28J60_WRITE_BUF_MEM);
    while (len--)
        sendSPI(*data++);
    disableChip();
}

static void SetBank(byte address) {
    if ((address & BANK_MASK) != Enc28j60Bank) {
        WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_BSEL1|ECON1_BSEL0);
        Enc28j60Bank = address & BANK_MASK;
        WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, Enc28j60Bank>>5);
    }
}

static byte ReadByte(byte address) {
    SetBank(address);
    return ReadOp(ENC28J60_READ_CTRL_REG, address);
}

static void WriteByte(byte address, byte data) {
    SetBank(address);
    WriteOp(ENC28J60_WRITE_CTRL_REG, address, data);
}

static void WriteWord(byte address, word data) {
    WriteByte(address, data);
    WriteByte(address + 1, data >> 8);
}

static word PhyReadHi(byte address) {
    WriteByte(MIREGADR, address);
    WriteByte(MICMD, MICMD_MIIRD);
    while (ReadByte(MISTAT) & MISTAT_BUSY)
        ;
    WriteByte(MICMD, 0x00);
    return ReadByte(MIRDH);
}

static void PhyWriteWord(byte address, word data) {
    WriteByte(MIREGADR, address);
    WriteWord(MIWRL, data);
    while (ReadByte(MISTAT) & MISTAT_BUSY)
        ;
}

byte enc28j60Init(byte* macaddr) {
    pinMode(ENC28J60_CONTROL_CS, OUTPUT);
    disableChip();
    
    WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
    while (!ReadOp(ENC28J60_READ_CTRL_REG, ESTAT) & ESTAT_CLKRDY)
        ;
        
    gNextPacketPtr = RXSTART_INIT;
    WriteWord(ERXSTL, RXSTART_INIT);
    WriteWord(ERXRDPTL, RXSTART_INIT);
    WriteWord(ERXNDL, RXSTOP_INIT);
    WriteWord(ETXSTL, TXSTART_INIT);
    WriteWord(ETXNDL, TXSTOP_INIT);
    WriteByte(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN);
    WriteWord(EPMM0, 0x303f);
    WriteWord(EPMCSL, 0xf7f9);
    WriteByte(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
    WriteByte(MACON2, 0x00);
    WriteOp(ENC28J60_BIT_FIELD_SET, MACON3,
                        MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);
    WriteWord(MAIPGL, 0x0C12);
    WriteByte(MABBIPG, 0x12);
    WriteWord(MAMXFLL, MAX_FRAMELEN);  
    WriteByte(MAADR5, macaddr[0]);
    WriteByte(MAADR4, macaddr[1]);
    WriteByte(MAADR3, macaddr[2]);
    WriteByte(MAADR2, macaddr[3]);
    WriteByte(MAADR1, macaddr[4]);
    WriteByte(MAADR0, macaddr[5]);
    PhyWriteWord(PHCON2, PHCON2_HDLDIS);
    SetBank(ECON1);
    WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);
    WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

    return ReadByte(EREVID);
}

byte enc28j60linkup(void) {
    return (PhyReadHi(PHSTAT2) >> 2) & 1;
}

void enc28j60PacketSend(word len, byte* packet) {
    while (ReadOp(ENC28J60_READ_CTRL_REG, ECON1) & ECON1_TXRTS)
        if ( (ReadByte(EIR) & EIR_TXERIF) ) {
            WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
            WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
        }
    WriteWord(EWRPTL, TXSTART_INIT);
    WriteWord(ETXNDL, TXSTART_INIT+len);
    WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
    WriteBuffer(len, packet);
    WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}

byte enc28j60hasRxPkt(void) {
    return ReadByte(EPKTCNT) > 0;
}

word enc28j60PacketReceive(word maxlen, byte* packet) {
    word len = 0;
    if (ReadByte(EPKTCNT) > 0) {
        WriteWord(ERDPTL, gNextPacketPtr);
        gNextPacketPtr  = ReadBufferWord();
        len = ReadBufferWord() - 4; //remove the CRC count
        if (len>maxlen-1)
            len=maxlen-1;
        word rxstat  = ReadBufferWord();
        if ((rxstat & 0x80)==0)
            len=0;
        else
            ReadBuffer(len, packet);
        if (gNextPacketPtr - 1 > RXSTOP_INIT)
            WriteWord(ERXRDPTL, RXSTOP_INIT);
        else
            WriteWord(ERXRDPTL, gNextPacketPtr - 1);
        WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
    }
    return len;
}
