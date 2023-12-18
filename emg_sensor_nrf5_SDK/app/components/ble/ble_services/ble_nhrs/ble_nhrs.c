


#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_HRS)
#include "ble_nhrs.h"
#include <string.h>
#include "ble_srv_common.h"
#include "nrf_delay.h"
#include "nrf_log.h"

static void on_connect(ble_nhrs_t * p_nhrs, ble_evt_t const * p_ble_evt)
{
    p_nhrs->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_hrs       Heart Rate Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_nhrs_t * p_nhrs, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_nhrs->conn_handle = BLE_CONN_HANDLE_INVALID;
}



/**@brief Function for handling write events to the Heart Rate Measurement characteristic.
 *
 * @param[in]   p_hrs         Heart Rate Service structure.
 * @param[in]   p_evt_write   Write event received from the BLE stack.
 */
static void on_hrm_cccd_write(ble_nhrs_t * p_nhrs, ble_gatts_evt_write_t const * p_evt_write)
{
    if (p_evt_write->len == 2)
    {
        // CCCD written, update notification state
        if (p_nhrs->evt_handler != NULL)
        {
            ble_nhrs_evt_t evt;

            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                evt.evt_type = BLE_HRS_EVT_NOTIFICATION_ENABLED;
            }
            else
            {
                evt.evt_type = BLE_HRS_EVT_NOTIFICATION_DISABLED;
            }

            p_nhrs->evt_handler(p_nhrs, &evt);
        }
    }
}



/**@brief Function for handling the Write event.
 *
 * @param[in]   p_hrs       Heart Rate Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_nhrs_t * p_nhrs, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (p_evt_write->handle == p_nhrs->hrm_handles.cccd_handle)
    {
        on_hrm_cccd_write(p_nhrs, p_evt_write);
    }
}



void ble_nhrs_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_nhrs_t * p_nhrs = (ble_nhrs_t *) p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_nhrs, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_nhrs, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_nhrs, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}



uint32_t ble_nhrs_init(ble_nhrs_t * p_nhrs, const ble_nhrs_init_t * p_nhrs_init)
{
    uint32_t              err_code;
    ble_uuid_t            ble_uuid;
    ble_add_char_params_t add_char_params;


    // Initialize service structure
    p_nhrs->evt_handler                 = p_nhrs_init->evt_handler;
    p_nhrs->is_sensor_contact_supported = p_nhrs_init->is_sensor_contact_supported;
    p_nhrs->conn_handle                 = BLE_CONN_HANDLE_INVALID;


    // Add service
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_HEART_RATE_SERVICE);

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_nhrs->service_handle);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add heart rate measurement characteristic
    memset(&add_char_params, 0, sizeof(add_char_params));

    add_char_params.uuid              = BLE_UUID_HEART_RATE_MEASUREMENT_CHAR;
    add_char_params.max_len           = 128;
    add_char_params.init_len          = 0;
    add_char_params.p_init_value      = NULL;
    add_char_params.is_var_len        = true;
    add_char_params.char_props.notify = 1;
    add_char_params.cccd_write_access = p_nhrs_init->hrm_cccd_wr_sec;

    err_code = characteristic_add(p_nhrs->service_handle, &add_char_params, &(p_nhrs->hrm_handles));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }


    return NRF_SUCCESS;
}
//home/ebenezer/nRF5_SDK/components/ble/ble_services/ble_nhrs/ble_nhrs.h
// /home/ebenezer/nRF5_SDK/components/ble/ble_services/ble_nhrs/ble_nhrs.c

uint32_t ble_nhrs_send(ble_nhrs_t * p_nhrs, uint8_t * p_data, uint16_t * p_length)
{
    uint32_t err_code;

    // Send value if connected and notifying
    if (p_nhrs->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
  
        ble_gatts_hvx_params_t hvx_params;

        VERIFY_PARAM_NOT_NULL(p_nhrs);

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_nhrs->hrm_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = p_length;
        hvx_params.p_data = p_data;

        err_code = sd_ble_gatts_hvx(p_nhrs->conn_handle, &hvx_params);

        while((err_code == NRF_ERROR_BUSY) || (err_code == NRF_ERROR_RESOURCES))
        {
            nrf_delay_ms(100);
            err_code = sd_ble_gatts_hvx(p_nhrs->conn_handle, &hvx_params);
        }

        if (err_code != NRF_SUCCESS)
        {
            NRF_LOG_INFO("******> BLE Send Error: %d\n");
        }
           if (err_code == NRF_SUCCESS)
        {
            NRF_LOG_INFO("******> BLE Send Success: %d\n");
        }
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

uint32_t ble_nhrs_sensor_contact_supported_set(ble_nhrs_t * p_nhrs, bool is_sensor_contact_supported)
{
    // Check if we are connected to peer
    if (p_nhrs->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        p_nhrs->is_sensor_contact_supported = is_sensor_contact_supported;
        return NRF_SUCCESS;
    }
    else
    {
        return NRF_ERROR_INVALID_STATE;
    }
}

void ble_nhrs_on_gatt_evt(ble_nhrs_t * p_nhrs, nrf_ble_gatt_evt_t const * p_gatt_evt)
{
    if (    (p_nhrs->conn_handle == p_gatt_evt->conn_handle)
        &&  (p_gatt_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        //p_nhrs->max_hrm_len = p_gatt_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
    }
}
#endif // NRF_MODULE_ENABLED(BLE_HRS)

