/*
 ===============================================================================
 Name        : aes_ecb
 Author      : Jason
 Version     : v1.0.0
 Date		 : 2014/11/28
 Copyright   : Copyright (C) www.embeda.com.tw
 Description : AES ECB encryption
 ===============================================================================
 History
 ---------+---------+--------------------------------------------+-------------
 DATE     |	VERSION |	DESCRIPTIONS							 |	By
 ---------+---------+--------------------------------------------+-------------
 2014/11/28	v1.0.0	First Edition									Jason
 ===============================================================================
 */

#ifndef CLASS_AES_H_
#define CLASS_AES_H_

#include <string.h>
#include <class/object.h>

#define AES_BLOCK_SIZE	16

/**
 * @brief AES 128 bits CTR mode encryption/decryption service class.
 * @class aesCTR aes.h	"class/aes.h"
 * @ingroup Peripherals
 */
class aesCTR: public CObject {
public:
	/**
	 * @brief aesECB constructor
	 * @param key A 16 bytes key.
	 */
	aesCTR(uint8_t const *key);

	/**
	 * @brief Encrypt/Decrypt data by AES CTR mode.
	 *
	 * @param output Result of encryption/decryption.
	 * @param input  Source data to be encrypted/decrypted.
	 * @param length The size of the data.
	 * @param p_offset Indicate the start byte position of the data to crypt.
	 *
	 * @retval true  If the crypt operation completed.
	 * @retval false If the crypt operation did not complete.
	 */
	virtual bool crypt(uint8_t *output, uint8_t const *input, uint16_t length, uint8_t *p_offset=NULL);

	/**
	 * @brief Generate a new nonce block.
	 * @return Pointer to the m_nonce.
	 */
	uint8_t* new_nonce();

	/**
	 * @brief Pointer to the m_nonce.
	 */
	inline uint8_t* get_nonce() {
		return m_nonce;
	}

	/**
	 * @brief Assign a nonce block
	 */
	inline void set_nonce(uint8_t *p_nonce) {
		memcpy(m_nonce, p_nonce, AES_BLOCK_SIZE);
	}

	/**
	 * @brief Shorthand operator to assign a nonce block.
	 */
	inline void operator = (uint8_t *p_nonce) {
		set_nonce(p_nonce);
	}

	//
	///@cond
	virtual ~aesCTR();
protected:
	xHandle	 	m_handle;
	uint8_t		m_nonce[AES_BLOCK_SIZE];
	///@endcond
};

/**\example /peripherals/aes_test/src/main.cpp
 * This is an example of how to use the aesCTR class.
 * More details about this example.
 */

#endif /* CLASS_AES_H_ */
