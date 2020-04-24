// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* \li File:               eeprom.c
* \li Compiler:           IAR EWAAVR 3.10c
* \li Support mail:       avr@atmel.com
*
* \li Supported devices:  All devices with split EEPROM erase/write
*                         capabilities can be used.
*                         The example is written for ATmega48.
*
* \li AppNote:            AVR103 - Using the EEPROM Programming Modes.
*
* \li Description:        Example on how to use the split EEPROM erase/write
*                         capabilities in e.g. ATmega48. All EEPROM
*                         programming modes are tested, i.e. Erase+Write,
*                         Erase-only and Write-only.
*
*                         $Revision: 1.6 $
*                         $Date: Friday, February 11, 2005 07:16:44 UTC $
****************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>

/* These EEPROM bits have different names on different devices. */
#ifndef EEPE
#define EEPE EEWE	//!< EEPROM program/write enable.
#define EEMPE EEMWE //!< EEPROM master program/write enable.
#endif

/* These two are unfortunately not defined in the device include files. */
#define EEPM1 5 //!< EEPROM Programming Mode Bit 1.
#define EEPM0 4 //!< EEPROM Programming Mode Bit 0.

/* Define to reduce code size. */
#define EEPROM_IGNORE_SELFPROG //!< Remove SPM flag polling.

/*! \brief  Read byte from EEPROM.
 *
 *  This function reads one byte from a given EEPROM address.
 *
 *  \note  The CPU is halted for 4 clock cycles during EEPROM read.
 *
 *  \param  addr  EEPROM address to read from.
 *  \return  The byte read from the EEPROM address.
 */
unsigned char eeprom_get_char(unsigned int addr)
{
	EEARL = addr;
	EEARH = addr >> 8;

	EECR |= (1 << EERE);
	__asm__ __volatile__("nop" ::);
	__asm__ __volatile__("nop" ::);
	return EEDR;
}

/*! \brief  Write byte to EEPROM.
 *
 *  This function writes one byte to a given EEPROM address.
 *  The differences between the existing byte and the new value is used
 *  to select the most efficient EEPROM programming mode.
 *
 *  \note  The CPU is halted for 2 clock cycles during EEPROM programming.
 *
 *  \note  When this function returns, the new EEPROM value is not available
 *         until the EEPROM programming time has passed. The EEPE bit in EECR
 *         should be polled to check whether the programming is finished.
 *
 *  \note  The EEPROM_GetChar() function checks the EEPE bit automatically.
 *
 *  \param  addr  EEPROM address to write to.
 *  \param  new_value  New EEPROM value.
 */
void eeprom_put_char(unsigned int addr, unsigned char new_value)
{
	// set address & data
	EEARL = addr;
	EEARH = addr >> 8;
	EEDR = new_value;
	uint8_t __bk_sreg = SREG;
	cli();
	EECR = 0x04;
	EECR = 0x02;
	SREG = __bk_sreg;
}

// Extensions added as part of Grbl

void memcpy_to_eeprom_with_checksum(unsigned int destination, char *source, unsigned int size) {
  unsigned char checksum = 0;
  for(; size > 0; size--) { 
    checksum = (checksum << 1) || (checksum >> 7);
    checksum += *source;
    eeprom_put_char(destination++, *(source++)); 
  }
  eeprom_put_char(destination, checksum);
}

int memcpy_from_eeprom_with_checksum(char *destination, unsigned int source, unsigned int size) {
  unsigned char data, checksum = 0;
  for(; size > 0; size--) { 
    data = eeprom_get_char(source++);
    checksum = (checksum << 1) || (checksum >> 7);
    checksum += data;    
    *(destination++) = data; 
  }
  return(checksum == eeprom_get_char(source));
}

// #define	e2pReset()	do { ECCR |= 0x20; } while(0)
// #define	e2pSWMON()	do { ECCR = 0x80; ECCR |= 0x10; } while(0);
// #define	e2pSWMOFF()	do { ECCR = 0x80; ECCR &= 0xEF; } while(0);

// #define E2PD0	(*((volatile unsigned char *)0x40))	
// #define E2PD1	(*((volatile unsigned char *)0x5A))	
// #define E2PD2	(*((volatile unsigned char *)0x57))	
// #define E2PD3	(*((volatile unsigned char *)0x5C))	
// #define	ECCR	(*((volatile unsigned char *)0x56))

// void memcpy_to_eeprom_with_checksum(unsigned int destination, char *source, unsigned int size)
// {
// 	uint8_t i;
//     uint8_t __bk_sreg;
//     uint8_t __checksum;
//     e2pReset();
//     e2pSWMON();
//     EEARH = destination >> 8;
//     EEARL = destination;

//     for (i = 0; i < size; i += 4)
//     {
//         if (size - i <= 4)
//         {
//             e2pSWMOFF();
//             EEARL = 0;
//             EEDR = source[i];
//             __checksum = ((__checksum << 1) | (__checksum >> 7)) + EEDR;

//             if (size - i > 1)
//             {
//                 EEARL = 1;
//                 EEDR = source[i + 1];
//                 __checksum += EEDR;
//             }
//             if (size - i > 2)
//             {
//                 EEARL = 2;
//                 EEDR = source[i + 2];
//                 __checksum += EEDR;
//             }
//             if (size - i > 3)
//             {
//                 EEARL = 3;
//                 EEDR = source[i + 3];
//                 __checksum += EEDR;
//             }
//         }
//         else
//         {
//             E2PD0 = source[i];
//             E2PD1 = source[i + 1];
//             E2PD2 = source[i + 2];
//             E2PD3 = source[i + 3];
//             __checksum = ((__checksum << 1) | (__checksum >> 7)) + E2PD0 + E2PD1 + E2PD2 + E2PD3;
//         }
//         __bk_sreg = SREG;
//         cli();
//         EECR = 0x44;
//         EECR = 0x42;
//         SREG = __bk_sreg;
// 		sei();
//     }
// 	eeprom_put_char(destination+size, __checksum);
// }

// int memcpy_from_eeprom_with_checksum(char *destination, unsigned int source, unsigned int size)
// {
// 	uint8_t i;
//     uint8_t __checksum;
//     e2pReset();
//     e2pSWMON();

//     EEARH = source >> 8;
//     EEARL = source;

//     for (i = 0; i < size; i += 4)
//     {
//         if (size - i <= 4)
//             e2pSWMOFF();

//         EECR |= (1 << EERE);

//         __asm__ __volatile__("nop" ::);
//         __asm__ __volatile__("nop" ::);

//         destination[i] = E2PD0;
//         __checksum = ((__checksum << 1) | (__checksum >> 7)) + E2PD0;
//         if (size - i > 1)
//         {
//             destination[i + 1] = E2PD1;
//             __checksum += E2PD1;
//         }
//         if (size - i > 2)
//         {
//             destination[i + 2] = E2PD2;
//             __checksum += E2PD2;
//         }
//         if (size - i > 3)
//         {
//             destination[i + 3] = E2PD3;
//             __checksum += E2PD3;
//         }
//     }
// 	fake_serial_write(__checksum/100%10+'0');
// 	fake_serial_write(__checksum/10%10+'0');
// 	fake_serial_write(__checksum%10+'0');
// 	fake_serial_write('\n');
// 	fake_serial_write(eeprom_get_char(source+size)/100%10+'0');
// 	fake_serial_write(eeprom_get_char(source+size)/10%10+'0');
// 	fake_serial_write(eeprom_get_char(source+size)%10+'0');
// 	fake_serial_write('\n');
//     return (__checksum == eeprom_get_char(source+size));
// }

// end of file
