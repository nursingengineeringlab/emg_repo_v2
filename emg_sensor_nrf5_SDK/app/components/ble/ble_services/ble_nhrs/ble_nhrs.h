//home/ebenezer/nRF5_SDK/components/ble/ble_services/ble_nhrs/ble_nhrs.h
//home/ebenezer/nRF5_SDK/components/ble/ble_services/ble_nhrs/ble_nhrs.c

#ifndef BLE_NHRS_H__
#define BLE_NHRS_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"

#ifdef __cplusplus
extern "C" {
#endif

//home/ebenezer/nRF5_SDK/components/ble/ble_services/ble_nhrs/ble_nhrs.h
//home/ebenezer/nRF5_SDK/examples/myProjects/new_dyn_adv/main.c
/**@brief   Macro for defining a ble_hrs instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_NHRS_DEF(_name)                                                                          \
static ble_nhrs_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_HRS_BLE_OBSERVER_PRIO,                                                     \
                     ble_nhrs_on_ble_evt, &_name)


/**@brief Heart Rate Service event type. */
typedef enum
{
    BLE_HRS_EVT_NOTIFICATION_ENABLED,   /**< Heart Rate value notification enabled event. */
    BLE_HRS_EVT_NOTIFICATION_DISABLED   /**< Heart Rate value notification disabled event. */
} ble_nhrs_evt_type_t;

/**@brief Heart Rate Service event. */
typedef struct
{
    ble_nhrs_evt_type_t evt_type;    /**< Type of event. */
} ble_nhrs_evt_t;

// Forward declaration of the ble_hrs_t type.
typedef struct ble_nhrs_s ble_nhrs_t;

/**@brief Heart Rate Service event handler type. */
typedef void (*ble_nhrs_evt_handler_t) (ble_nhrs_t * p_hrs, ble_nhrs_evt_t * p_evt);

/**@brief Heart Rate Service init structure. This contains all options and data needed for
 *        initialization of the service. */
typedef struct
{
    ble_nhrs_evt_handler_t        evt_handler;                                          /**< Event handler to be called for handling events in the Heart Rate Service. */
    bool                         is_sensor_contact_supported;                          /**< Determines if sensor contact detection is to be supported. */
    uint8_t *                    p_body_sensor_location;                               /**< If not NULL, initial value of the Body Sensor Location characteristic. */
    security_req_t               hrm_cccd_wr_sec;                                      /**< Security requirement for writing the HRM characteristic CCCD. */
    security_req_t               bsl_rd_sec;                                           /**< Security requirement for reading the BSL characteristic value. */
} ble_nhrs_init_t;

/**@brief Heart Rate Service structure. This contains various status information for the service. */
struct ble_nhrs_s
{
    ble_nhrs_evt_handler_t        evt_handler;                                          /**< Event handler to be called for handling events in the Heart Rate Service. */
    bool                         is_expended_energy_supported;                         /**< TRUE if Expended Energy measurement is supported. */
    bool                         is_sensor_contact_supported;                          /**< TRUE if sensor contact detection is supported. */
    uint16_t                     service_handle;                                       /**< Handle of Heart Rate Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t     hrm_handles;                                          /**< Handles related to the Heart Rate Measurement characteristic. */
    uint16_t                     conn_handle;                                          /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    bool                         is_sensor_contact_detected;                           /**< TRUE if sensor contact has been detected. */
                                     /**< Current maximum HR measurement length, adjusted according to the current ATT MTU. */
};


/**@brief Function for initializing the Heart Rate Service.
 *
 * @param[out]  p_hrs       Heart Rate Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_hrs_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_nhrs_init(ble_nhrs_t * p_nhrs, ble_nhrs_init_t const * p_hrs_init);


/**@brief Function for handling the GATT module's events.
 *
 * @details Handles all events from the GATT module of interest to the Heart Rate Service.
 *
 * @param[in]   p_hrs      Heart Rate Service structure.
 * @param[in]   p_gatt_evt  Event received from the GATT module.
 */
void ble_nhrs_on_gatt_evt(ble_nhrs_t * p_nhrs, nrf_ble_gatt_evt_t const * p_gatt_evt);


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Heart Rate Service.
 *
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 * @param[in]   p_context   Heart Rate Service structure.
 */
void ble_nhrs_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);



uint32_t ble_nhrs_send(ble_nhrs_t * p_nhrs, uint8_t * p_data, uint16_t * p_length);


/**@brief Function for setting the state of the Sensor Contact Supported bit.
 *
 * @param[in]   p_nhrs                        Heart Rate Service structure.
 * @param[in]   is_sensor_contact_supported  New state of the Sensor Contact Supported bit.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_nhrs_sensor_contact_supported_set(ble_nhrs_t * p_nhrs, bool is_sensor_contact_supported);




#ifdef __cplusplus
}
#endif

#endif // BLE_NHRS_H__

/** @} */
