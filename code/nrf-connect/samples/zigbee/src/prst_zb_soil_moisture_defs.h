#ifndef _PRST_ZB_SOIL_MOISTURE_DEFS_
#define _PRST_ZB_SOIL_MOISTURE_DEFS_

#include <zboss_api.h>
#include <zcl/zb_zcl_common.h>

// Most defines in this file are updated from the ZB_ZCL_DECLARE_TEMP_MEASUREMENT_ATTRIB_LIST,
// adapting attributes and IDs to match the mSoilMoisture cluster spec.
// Values from https://github.com/Koenkk/zigbee-herdsman/blob/master/src/zcl/definition/cluster.ts#L2570
// (msSoilMoisture).
// Cluster attributes definitions in https://www.st.com/resource/en/user_manual/um2977-stm32wb-series-zigbee-cluster-library-api-stmicroelectronics.pdf.

#define PRST_ZB_ZCL_ATTR_SOIL_MOISTURE_CLUSTER_ID 1032

// Soil moisture value represented as an uint16.
#define PRST_ZB_ZCL_ATTR_SOIL_MOISTURE_VALUE_ID 0x00

// Required callbacks. ZBOSS will call these.
void prst_zcl_soil_moisture_init_server(void);
void prst_zcl_soil_moisture_init_client(void);

#define PRST_ZB_ZCL_ATTR_SOIL_MOISTURE_CLUSTER_ID_SERVER_ROLE_INIT prst_zcl_soil_moisture_init_server
#define PRST_ZB_ZCL_ATTR_SOIL_MOISTURE_CLUSTER_ID_CLIENT_ROLE_INIT prst_zcl_soil_moisture_init_client

#define PRST_ZB_ZCL_SOIL_MOISTURE_CLUSTER_REVISION_DEFAULT ((zb_uint16_t)0x42)

#define ZB_SET_ATTR_DESCR_WITH_PRST_ZB_ZCL_ATTR_SOIL_MOISTURE_VALUE_ID(data_ptr) \
  {                                                                              \
    PRST_ZB_ZCL_ATTR_SOIL_MOISTURE_VALUE_ID,                                     \
        ZB_ZCL_ATTR_TYPE_U16,                                                    \
        ZB_ZCL_ATTR_ACCESS_READ_ONLY | ZB_ZCL_ATTR_ACCESS_REPORTING,             \
        (void*)data_ptr                                                          \
  }

#define PRST_ZB_ZCL_DECLARE_SOIL_MOISTURE_ATTRIB_LIST(attr_list, value)                   \
  ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(attr_list, PRST_ZB_ZCL_SOIL_MOISTURE) \
  ZB_ZCL_SET_ATTR_DESC(PRST_ZB_ZCL_ATTR_SOIL_MOISTURE_VALUE_ID, (value))                  \
  ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST

#endif  // _PRST_ZB_SOIL_MOISTURE_DEFS_