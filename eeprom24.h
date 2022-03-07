/* eeprom24.h
 *
 * Created on: Mar 7, 2022
 * Author: Martin Danek, martin@embedblog.eu
 */

#ifndef EEPROM24_H_
#define EEPROM24_H_

#include "hal_inc.h"

#ifndef EEPROM24_I2C_TIMEOUT
#define EEPROM24_I2C_TIMEOUT		25
#endif

class Eeprom24
{
public:
	Eeprom24(I2C_HandleTypeDef* i2c, uint8_t address, uint32_t size, uint16_t page):
		m_i2c(i2c), m_i2c_address(address), m_sizeInBytes(size), m_pageSizeInBytes(page) {};

	bool init();

	bool isReady(void) const;
	bool waitForReady(uint32_t timeout = EEPROM24_I2C_TIMEOUT) const;

	uint32_t getSizeInBytes(void) const {return m_sizeInBytes;};
	uint16_t getPageSizeInBytes(void) const {return m_pageSizeInBytes;};

	static constexpr uint8_t DEFAULT_ADDRESS = 0b1010000;

protected:
	bool writeByte_internal(uint8_t devAddress, uint16_t byteAddress, uint8_t data);
	bool writeByte_internal(uint8_t devAddress, uint8_t byteAddress, uint8_t data);
	uint8_t readByte_internal(uint8_t devAddress, uint16_t byteAddress);
	uint8_t readByte_internal(uint8_t devAddress, uint8_t byteAddress);

	bool writePage_internal(uint8_t devAddress, uint16_t byteAddress, uint8_t* data, uint16_t length);
	bool writePage_internal(uint8_t devAddress, uint8_t byteAddress, uint8_t* data, uint16_t length);
	bool readPage_internal(uint8_t devAddress, uint16_t byteAddress, uint8_t* data, uint16_t length);
	bool readPage_internal(uint8_t devAddress, uint8_t byteAddress, uint8_t* data, uint16_t length);

	I2C_HandleTypeDef* const m_i2c;
	const uint8_t m_i2c_address;
	const uint32_t m_sizeInBytes;
	const uint16_t m_pageSizeInBytes;
};


/** 24x512 memories; size = 64 kB; page size = 128 B.
 *
 */
class Eeprom24_512: public Eeprom24
{
public:
	Eeprom24_512(I2C_HandleTypeDef* i2c, uint8_t address = DEFAULT_ADDRESS): Eeprom24(i2c, address, 65535, 128) {};
	Eeprom24_512(I2C_HandleTypeDef* i2c, bool A0, bool A1, bool A2): Eeprom24(i2c, DEFAULT_ADDRESS | (A0) | (A1 << 1) | (A2 << 2), 65535, 128) {};

	bool writeByte(uint16_t address, uint8_t data) {return writeByte_internal(m_i2c_address, address, data);};
	uint8_t readByte(uint16_t address) {return readByte_internal(m_i2c_address, address);};

	bool writePage(uint16_t address, uint8_t* data, uint16_t length) {return writePage_internal(m_i2c_address, address, data, length);};
	bool readPage(uint16_t address, uint8_t* data, uint16_t length) {return readPage_internal(m_i2c_address, address, data, length);};
};



#endif /* EEPROM24_H_ */
