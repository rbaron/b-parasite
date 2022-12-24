#include "prst_zb_soil_moisture_defs.h"

#include <zboss_api.h>

void prst_zcl_soil_moisture_init_server(void) {
  zb_zcl_add_cluster_handlers(PRST_ZB_ZCL_ATTR_SOIL_MOISTURE_CLUSTER_ID,
                              ZB_ZCL_CLUSTER_SERVER_ROLE,
                              /*cluster_check_value=*/NULL,
                              /*cluster_write_attr_hook=*/NULL,
                              /*cluster_handler=*/NULL);
}

void prst_zcl_soil_moisture_init_client(void) {
  // Nothing.
}
