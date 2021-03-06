/*
 ===============================================================================
 Name        : uri beacon
 Author      : uCXpresso
 Version     : v1.0.0
 Copyright   : www.ucxpresso.net
 License	 : Free
 Description : Implement Google URI Beacon
 ===============================================================================
 History
 ---------+---------+--------------------------------------------+-------------
 DATE     |	VERSION |	DESCRIPTIONS							 |	By
 ---------+---------+--------------------------------------------+-------------
 2015/2/8	v1.0.0	First Edition.									Jason
 2015/2/13	v1.0.1	Add onBleEvent function.						Jason
 ===============================================================================
 */

#include <uCXpresso.h>

#ifdef DEBUG
#include <debug.h>
#include <class/serial.h>
#define DBG 	dbg_printf
#define ASSERT	dbg_assert
#else
#define DBG(...)
#define ASSERT(...)
#endif

// TODO: insert other include files here
//#include <class/ble/ble_device_manager.h>
#include <class/ble/ble_device.h>
#include <class/ble/ble_service.h>
#include <class/ble/ble_conn_params.h>
#include <class/power.h>
#include <class/pin.h>

#include <ble_uri.h>
#include <string.h>

// TODO: insert other definitions and declarations here
#define DEVICE_NAME                        	"URI Beacon CFG"            /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                  	"uCXpresso.NRF"        /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_COMPANY_IDENTIFIER          	0x0059                 /**< Company identifier for Nordic Semi. as per www.bluetooth.org. */

#define APP_ADV_INTERVAL                  	1000                   /**< The advertising interval (in ms). */
#define NON_CONNECTABLE_ADV_INTERVAL		1000
#define APP_ADV_TIMEOUT_IN_SECONDS      	30                     /**< The advertising timeout (in units of seconds). */


#define MIN_CONN_INTERVAL					500
#define MAX_CONN_INTERVAL					1000

#define ADV_FLAGS_LEN                 		3
static uint8_t adv_flags[ADV_FLAGS_LEN] = {0x02, 0x01, 0x04};

// Normal: send configured ADV packet
// Config: enable GATT characteristic configuration
typedef enum {
	beacon_mode_config,
	beacon_mode_normal
} beacon_mode_t;

/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(beacon_mode_t mode) {
	if (mode == beacon_mode_normal) {
		uint8_t adv_data[ADV_FLAGS_LEN + APP_ADV_DATA_MAX_LEN];
		uint8_t adv_data_len;

		memcpy(adv_data, adv_flags, ADV_FLAGS_LEN);
		adv_data_len = gpUriBeacon->get_adv_data(adv_data+ADV_FLAGS_LEN);

		if ((adv_data_len > 0) && (adv_data_len <= APP_ADV_DATA_MAX_LEN)) {
			gpBLE->m_advertising.update(adv_data, adv_data_len + ADV_FLAGS_LEN, NULL, 0); // adv. raw data updated
			gpBLE->m_advertising.type(ADV_TYPE_ADV_NONCONN_IND);
			gpBLE->m_advertising.timeout(0);
			gpBLE->m_advertising.interval(NON_CONNECTABLE_ADV_INTERVAL);
		}

	} else 	if (mode == beacon_mode_config) {
		ble_uuid_t adv_uuids[] = {{URI_UUID_BEACON_SERVICE, gpUriBeacon->uuid_type()}};
		ble_advdata_t advdata;
		ble_advdata_t scanrsp;
		uint8_t       flags = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

	    memset(&advdata, 0, sizeof(advdata));
	    advdata.include_appearance      = true;
	    advdata.name_type				= BLE_ADVDATA_FULL_NAME;
	    advdata.flags.size              = sizeof(flags);
	    advdata.flags.p_data            = &flags;

	    memset(&scanrsp, 0, sizeof(scanrsp));
	    scanrsp.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
	    scanrsp.uuids_complete.p_uuids  = adv_uuids;
		gpBLE->m_advertising.update(&advdata, &scanrsp);

		gpBLE->m_advertising.type(ADV_TYPE_ADV_IND);
		gpBLE->m_advertising.interval(APP_ADV_INTERVAL);
		gpBLE->m_advertising.timeout(APP_ADV_TIMEOUT_IN_SECONDS);
	}
}

//
// parse BLE events
//
void onBleEvent(bleDevice * p_ble, BLE_EVENT_T evt) {
	switch(evt) {
	case BLE_ON_CONNECTED:
		p_ble->onConnected();
		break;
	case BLE_ON_DISCONNECTED:
		p_ble->onDisconnected();
		break;
	case BLE_ON_TIMEOUT:
		system_reset();		// In configure mode, reset when adv. timeout
		break;
	}
}

//
// Hardware
//
//#define BOARD_PCA10001
#define BOARD_LILYPAD
#include <config/board.h>

//
// Main Routine
//
int main(void) {
#ifdef DEBUG
	CSerial ser;		// declare a UART object
	ser.enable();
	CDebug dbg(ser);	// Debug stream use the UART object
	dbg.start();
#endif

	beacon_mode_t beacon_mode = beacon_mode_normal;

	//
	// SoftDevice
	//
	bleDevice ble;
	ble.attachEvent(onBleEvent);
	ble.enable();	// enable BLE SoftDevice task

	// GAP
	ble.m_gap.settings(DEVICE_NAME);	// set Device Name on GAP
	ble.m_gap.tx_power(BLE_TX_0dBm);

	bleServiceUriBeacon beacon(ble);	// add uri beacon service

	bleConnParams conn(ble);

	advertising_init(beacon_mode);

	// Start advertising
	ble.m_advertising.start();

	//
	// Optional: Enable tickless technology
	//
#ifndef DEBUG
	CPowerSave::tickless(true);
#endif

	//
	//
	//
	CPin led(LED_PIN_0);
	led.output();

	CPin btn(BUTTON_PIN_0);
	btn.input();

	//
	// Enter main loop.
	//
	while (1) {
		//
		// check button
		//
		if ( btn==LOW ) {

			// stop advertising
			ble.m_advertising.stop();

			// change beacon mode
			if ( beacon_mode==beacon_mode_config ) {
				beacon_mode = beacon_mode_normal;
			} else {
				beacon_mode = beacon_mode_config;
			}

			// update mode and re-start the advertising
			advertising_init(beacon_mode);
			ble.m_advertising.start();

			// waiting for btn released
			while(btn==LOW);
		}

		//
		// LED Status
		//
		if ( beacon_mode==beacon_mode_config  ) {
			//
			// Negotiate the "connection parameters update" in main-loop
			//
			conn.negotiate();

			led = LED_ON;
			sleep(50);
			led = LED_OFF;
			sleep(150);

		} else {
			led = LED_ON;
			sleep(5);
			led = LED_OFF;
			sleep(995);
		}
	}
}
