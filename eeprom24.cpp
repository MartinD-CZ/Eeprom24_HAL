/* eeprom24.cpp
 *
 * Created on: Mar 7, 2022
 * Author: Martin Danek, martin@embedblog.eu
 */

#include "eeprom24.h"
#include "custom_assert.h"


/** Initialization function, doesn't have to be called, only checks connectivity with the EEPROM
 *
 * @return				True if connection with the EEPROM was established.
 */
bool Eeprom24::init()
{
	auto retval = HAL_I2C_IsDeviceReady(m_i2c, m_i2c_address << 1, 2, 100);
	return (retval == HAL_OK);
}


/** After a write operation, the memory enters an internal lock-up state, during which it doesn't respond on the I2C bus.
 *  This function is used to check whether the memory is ready for new commands. Read operations don't start lock-up state.
 *
 * @return				True is memory is ready to accept new commands.
 */
bool Eeprom24::isReady(void) const
{
	return (HAL_I2C_IsDeviceReady(m_i2c, m_i2c_address << 1, 1, 100) == HAL_OK);
}


/** Polling function with timeout, used to wait until EEPROM is ready to accept new commands after write
 *
 * @param timeout		Timeout in ms.
 * @return				True if device became ready before timeout.
 */
bool Eeprom24::waitForReady(uint32_t timeout) const
{
	uint32_t start = HAL_GetTick();
	while (!isReady())
	{
		HAL_Delay(1);

		if (HAL_GetTick() - start > timeout)
			return false;
	}

	return true;
}


/** Writes a byte to the EEPROM. Version for larger memories with 2 byte addresses.
 *
 * @param devAddress	EEPROM's I2C address, managed internally.
 * @param byteAddress	The address of the byte to write.
 * @param data			Byte to write.
 * @return 				True if write operation was successful.
 *
 * @note After writing, it takes the memory some time to save the data; poll using waitForReady.
 */
bool Eeprom24::writeByte_internal16(uint8_t devAddress, uint16_t byteAddress, uint8_t data)
{
	uint8_t tmp[3] = {(uint8_t)(byteAddress >> 8), (uint8_t)(byteAddress & 0xFF), data};
	auto retval = HAL_I2C_Master_Transmit(m_i2c, devAddress << 1, tmp, sizeof(tmp), EEPROM24_I2C_TIMEOUT);
	return (retval == HAL_OK);
}


/** Writes a byte to the EEPROM. Version for smaller memories with single byte addresses.
 *
 * @param devAddress	EEPROM's I2C address, managed internally.
 * @param byteAddress	The address of the byte to write.
 * @param data			Byte to write.
 * @return 				True if write operation was successful.
 *
 * @note After writing, it takes the memory some time to save the data; poll using waitForReady.
 */
bool Eeprom24::writeByte_internal8(uint8_t devAddress, uint8_t byteAddress, uint8_t data)
{
	uint8_t tmp[3] = {(uint8_t)(byteAddress >> 8), (uint8_t)(byteAddress & 0xFF), data};
	auto retval = HAL_I2C_Master_Transmit(m_i2c, devAddress << 1, tmp, sizeof(tmp), EEPROM24_I2C_TIMEOUT);
	return (retval == HAL_OK);
}


/** Reads a byte from the EEPROM. Version for larger memories with 2 byte addresses.
 *
 * @param devAddress	EEPROM's I2C address, managed internally.
 * @param byteAddress	The address of the byte to read.
 * @return				Value of the read byte.
 */
uint8_t Eeprom24::readByte_internal16(uint8_t devAddress, uint16_t byteAddress)
{
	uint8_t tmp[2] = {(uint8_t)(byteAddress >> 8), (uint8_t)(byteAddress & 0xFF)};
	HAL_I2C_Master_Transmit(m_i2c, devAddress << 1, tmp, sizeof(tmp), 25);

	uint8_t retval = 0;
	HAL_I2C_Master_Receive(m_i2c, devAddress << 1, &retval, 1, EEPROM24_I2C_TIMEOUT);
	return retval;
}


/** Reads a byte from the EEPROM. Version for smaller memories with single byte addresses.
 *
 * @param devAddress	EEPROM's I2C address, managed internally.
 * @param byteAddress	The address of the byte to read.
 * @return				Value of the read byte.
 */
uint8_t Eeprom24::readByte_internal8(uint8_t devAddress, uint8_t byteAddress)
{
	HAL_I2C_Master_Transmit(m_i2c, devAddress << 1, &byteAddress, 1, 25);

	uint8_t retval = 0;
	HAL_I2C_Master_Receive(m_i2c, devAddress << 1, &retval, 1, EEPROM24_I2C_TIMEOUT);
	return retval;
}


/** Writes up to a page-size of bytes to the memory. Version for larger memories with 2 byte addresses.
 *
 * @param devAddress	EEPROM's I2C address, managed internally.
 * @param byteAddress	The address of the byte the write should start at.
 * @param data			Pointer to an array with data to be written.
 * @param length		How many bytes to write. If page size is exceeded, a roll-over happens and the write starts from
 * 						the page beginning.
 * @return 				True if write operation was successful.
 *
 * @note After writing, it takes the memory some time to save the data; poll using waitForReady.
 */
bool Eeprom24::writePage_internal16(uint8_t devAddress, uint16_t byteAddress, uint8_t* data, uint16_t length)
{
	uint8_t tmp[m_pageSizeInBytes + 2];
	tmp[0] = byteAddress >> 8;
	tmp[1] = byteAddress;

	for (uint16_t i = 0; i < length; i++)
		tmp[i + 2] = data[i];

	auto retval = HAL_I2C_Master_Transmit(m_i2c, devAddress << 1, tmp, length + 2, EEPROM24_I2C_TIMEOUT);
	return (retval == HAL_OK);
}


/** Writes up to a page-size of bytes to the memory. Version for smaller memories with single byte addresses.
 *
 * @param devAddress	EEPROM's I2C address, managed internally.
 * @param byteAddress	The address of the byte the write should start at.
 * @param data			Pointer to an array with data to be written.
 * @param length		How many bytes to write. If page size is exceeded, a roll-over happens and the write starts from
 * 						the page beginning.
 * @return 				True if write operation was successful.
 *
 * @note After writing, it takes the memory some time to save the data; poll using waitForReady.
 */
bool Eeprom24::writePage_internal8(uint8_t devAddress, uint8_t byteAddress, uint8_t* data, uint16_t length)
{
	uint8_t tmp[m_pageSizeInBytes + 1];
	tmp[0] = byteAddress;

	for (uint16_t i = 0; i < length; i++)
		tmp[i + 1] = data[i];

	auto retval = HAL_I2C_Master_Transmit(m_i2c, devAddress << 1, tmp, length + 1, EEPROM24_I2C_TIMEOUT);
	return (retval == HAL_OK);
}


/** Reads a number of bytes from the EEPROM. Version for larger memories with 2 byte addresses.
 *
 * @param devAddress	EEPROM's I2C address, managed internally.
 * @param byteAddress	The address of the byte the read should start at.
 * @param data			Pointer to an array in which data will be stored.
 * @param length		How many bytes should be read, not limited by page boundaries.
 * @return 				True if write operation was successful.
 */
bool Eeprom24::readPage_internal16(uint8_t devAddress, uint16_t byteAddress, uint8_t* data, uint16_t length)
{
	uint8_t tmp[2] = {(uint8_t)(byteAddress >> 8), (uint8_t)(byteAddress & 0xFF)};
	HAL_I2C_Master_Transmit(m_i2c, devAddress << 1, tmp, sizeof(tmp), EEPROM24_I2C_TIMEOUT);

	auto retval = HAL_I2C_Master_Receive(m_i2c, devAddress << 1, data, length, EEPROM24_I2C_TIMEOUT);
	return (retval == HAL_OK);
}


/** Reads a number of bytes from the EEPROM. Version for smaller memories with single byte addresses.
 *
 * @param devAddress	EEPROM's I2C address, managed internally.
 * @param byteAddress	The address of the byte the read should start at.
 * @param data			Pointer to an array in which data will be stored.
 * @param length		How many bytes should be read, not limited by page boundaries.
 * @return 				True if write operation was successful.
 */
bool Eeprom24::readPage_internal8(uint8_t devAddress, uint8_t byteAddress, uint8_t* data, uint16_t length)
{
	HAL_I2C_Master_Transmit(m_i2c, devAddress << 1, &byteAddress, sizeof(byteAddress), EEPROM24_I2C_TIMEOUT);

	auto retval = HAL_I2C_Master_Receive(m_i2c, devAddress << 1, data, length, EEPROM24_I2C_TIMEOUT);
	return (retval == HAL_OK);
}



