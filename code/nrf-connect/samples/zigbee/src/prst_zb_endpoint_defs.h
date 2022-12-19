#ifndef _PRST_ZB_H_
#define _PRST_ZB_H_

#include "prst_zb_soil_moisture_defs.h"

#define PRST_ZIGBEE_ENDPOINT 10
#define PRST_BASIC_MANUF_NAME "b-parasite"

#define PRST_ZB_DEVICE_ID 0x0008
#define PRST_ZB_DEVICE_VERSION 0
#define PRST_ZB_IN_CLUSTER_NUM 7
#define PRST_ZB_OUT_CLUSTER_NUM 0
#define PRST_ZB_CLUSTER_NUM (PRST_ZB_IN_CLUSTER_NUM + PRST_ZB_OUT_CLUSTER_NUM)
#define PRST_ZB_ATTR_REPORTING_COUNT 5

#define PRST_ZB_DECLARE_CLUSTER_LIST(                                       \
    cluster_list_name,                                                      \
    basic_attr_list,                                                        \
    identify_attr_list,                                                     \
    temp_measurement_attr_list,                                             \
    rel_humidity_attr_list,                                                 \
    batt_att_list,                                                          \
    soil_moisture_attr_list,                                                \
    illuminance_attr_list)                                                  \
  zb_zcl_cluster_desc_t cluster_list_name[] =                               \
      {                                                                     \
          ZB_ZCL_CLUSTER_DESC(                                              \
              ZB_ZCL_CLUSTER_ID_IDENTIFY,                                   \
              ZB_ZCL_ARRAY_SIZE(identify_attr_list, zb_zcl_attr_t),         \
              (identify_attr_list),                                         \
              ZB_ZCL_CLUSTER_SERVER_ROLE,                                   \
              ZB_ZCL_MANUF_CODE_INVALID),                                   \
          ZB_ZCL_CLUSTER_DESC(                                              \
              ZB_ZCL_CLUSTER_ID_BASIC,                                      \
              ZB_ZCL_ARRAY_SIZE(basic_attr_list, zb_zcl_attr_t),            \
              (basic_attr_list),                                            \
              ZB_ZCL_CLUSTER_SERVER_ROLE,                                   \
              ZB_ZCL_MANUF_CODE_INVALID),                                   \
          ZB_ZCL_CLUSTER_DESC(                                              \
              ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,                           \
              ZB_ZCL_ARRAY_SIZE(temp_measurement_attr_list, zb_zcl_attr_t), \
              (temp_measurement_attr_list),                                 \
              ZB_ZCL_CLUSTER_SERVER_ROLE,                                   \
              ZB_ZCL_MANUF_CODE_INVALID),                                   \
          ZB_ZCL_CLUSTER_DESC(                                              \
              ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,                   \
              ZB_ZCL_ARRAY_SIZE(rel_humidity_attr_list, zb_zcl_attr_t),     \
              (rel_humi_attr_list),                                         \
              ZB_ZCL_CLUSTER_SERVER_ROLE,                                   \
              ZB_ZCL_MANUF_CODE_INVALID),                                   \
          ZB_ZCL_CLUSTER_DESC(                                              \
              PRST_ZB_ZCL_ATTR_SOIL_MOISTURE_CLUSTER_ID,                    \
              ZB_ZCL_ARRAY_SIZE(soil_moisture_attr_list, zb_zcl_attr_t),    \
              (soil_moisture_attr_list),                                    \
              ZB_ZCL_CLUSTER_SERVER_ROLE,                                   \
              ZB_ZCL_MANUF_CODE_INVALID),                                   \
          ZB_ZCL_CLUSTER_DESC(                                              \
              ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT,                    \
              ZB_ZCL_ARRAY_SIZE(illuminance_attr_list, zb_zcl_attr_t),      \
              (illuminance_attr_list),                                      \
              ZB_ZCL_CLUSTER_SERVER_ROLE,                                   \
              ZB_ZCL_MANUF_CODE_INVALID),                                   \
          ZB_ZCL_CLUSTER_DESC(                                              \
              ZB_ZCL_CLUSTER_ID_POWER_CONFIG,                               \
              ZB_ZCL_ARRAY_SIZE(batt_attr_list, zb_zcl_attr_t),             \
              (batt_attr_list),                                             \
              ZB_ZCL_CLUSTER_SERVER_ROLE,                                   \
              ZB_ZCL_MANUF_CODE_INVALID)}

#define PRST_ZB_DECLARE_SIMPLE_DESC(ep_name, ep_id, in_clust_num, out_clust_num) \
  ZB_DECLARE_SIMPLE_DESC(in_clust_num, out_clust_num);                           \
  ZB_AF_SIMPLE_DESC_TYPE(in_clust_num, out_clust_num)                            \
  simple_desc_##ep_name =                                                        \
      {                                                                          \
          ep_id,                                                                 \
          ZB_AF_HA_PROFILE_ID,                                                   \
          PRST_ZB_DEVICE_ID,                                                     \
          PRST_ZB_DEVICE_VERSION,                                                \
          0,                                                                     \
          in_clust_num,                                                          \
          out_clust_num,                                                         \
          {                                                                      \
              ZB_ZCL_CLUSTER_ID_BASIC,                                           \
              ZB_ZCL_CLUSTER_ID_IDENTIFY,                                        \
              ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,                                \
              ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,                        \
              PRST_ZB_ZCL_ATTR_SOIL_MOISTURE_CLUSTER_ID,                         \
              ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT,                         \
              ZB_ZCL_CLUSTER_ID_POWER_CONFIG,                                    \
          }}

#define PRST_ZB_DECLARE_ENDPOINT(ep_name, ep_id, cluster_list)                                      \
  ZBOSS_DEVICE_DECLARE_REPORTING_CTX(reporting_ctx_##ep_name, PRST_ZB_ATTR_REPORTING_COUNT);        \
  PRST_ZB_DECLARE_SIMPLE_DESC(ep_name, ep_id,                                                       \
                              PRST_ZB_IN_CLUSTER_NUM, PRST_ZB_OUT_CLUSTER_NUM);                     \
  ZB_AF_DECLARE_ENDPOINT_DESC(ep_name, ep_id, ZB_AF_HA_PROFILE_ID,                                  \
                              /*reserved_length=*/0, /*reserved_ptr=*/NULL,                         \
                              ZB_ZCL_ARRAY_SIZE(cluster_list, zb_zcl_cluster_desc_t), cluster_list, \
                              (zb_af_simple_desc_1_1_t *)&simple_desc_##ep_name,                    \
                              PRST_ZB_ATTR_REPORTING_COUNT, reporting_ctx_##ep_name,                \
                              /*lev_ctrl_count=*/0, /*lev_ctrl_ctx=*/NULL)

#endif  // _PRST_ZB_H_
