/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef ZB_RANGE_EXTENDER_H
#define ZB_RANGE_EXTENDER_H 1

/**
 *  @defgroup ZB_DEFINE_DEVICE_RANGE_EXTENDER Range Extender
 *  @{
 *  @details
 *      - @ref ZB_ZCL_IDENTIFY \n
 *      - @ref ZB_ZCL_BASIC
 */

/** Range Extender Device ID*/
#define ZB_RANGE_EXTENDER_DEVICE_ID 0x0008

/** Range extender device version */
#define ZB_DEVICE_VER_RANGE_EXTENDER 0

/** @cond internals_doc */

/** Range extender IN (server) clusters number */
#define ZB_RANGE_EXTENDER_IN_CLUSTER_NUM 4

/** Range extender OUT (client) clusters number */
#define ZB_RANGE_EXTENDER_OUT_CLUSTER_NUM 0

#define ZB_RANGE_EXTENDER_CLUSTER_NUM \
  (ZB_RANGE_EXTENDER_IN_CLUSTER_NUM + ZB_RANGE_EXTENDER_OUT_CLUSTER_NUM)

/** Number of attribute for reporting on Range extender device */
#define ZB_RANGE_EXTENDER_REPORT_ATTR_COUNT 0

typedef struct {
  zb_uint16_t rel_humidity;
  zb_uint16_t min_val;
  zb_uint16_t max_val;
} prst_rel_humidity_attrs_t;

#define ZB_DECLARE_RANGE_EXTENDER_CLUSTER_LIST(                             \
    cluster_list_name,                                                      \
    basic_attr_list,                                                        \
    identify_attr_list,                                                     \
    temp_measurement_attr_list,                                             \
    rel_humidity_attr_list)                                                 \
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
              ZB_ZCL_MANUF_CODE_INVALID)}

/** @cond internals_doc */

/**
 * @brief Declare simple descriptor for Range extender device
 * @param ep_name - endpoint variable name
 * @param ep_id - endpoint ID
 * @param in_clust_num - number of supported input clusters
 * @param out_clust_num - number of supported output clusters
 */
#define ZB_ZCL_DECLARE_RANGE_EXTENDER_SIMPLE_DESC(ep_name, ep_id, in_clust_num, out_clust_num) \
  ZB_DECLARE_SIMPLE_DESC(in_clust_num, out_clust_num);                                         \
  ZB_AF_SIMPLE_DESC_TYPE(in_clust_num, out_clust_num)                                          \
  simple_desc_##ep_name =                                                                      \
      {                                                                                        \
          ep_id,                                                                               \
          ZB_AF_HA_PROFILE_ID,                                                                 \
          ZB_RANGE_EXTENDER_DEVICE_ID,                                                         \
          ZB_DEVICE_VER_RANGE_EXTENDER,                                                        \
          0,                                                                                   \
          in_clust_num,                                                                        \
          out_clust_num,                                                                       \
          {ZB_ZCL_CLUSTER_ID_BASIC,                                                            \
           ZB_ZCL_CLUSTER_ID_IDENTIFY,                                                         \
           ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,                                                 \
           ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT}}

/** @endcond */ /* internals_doc */

/**
 * @brief Declare endpoint for Range extender device
 * @param ep_name - endpoint variable name
 * @param ep_id - endpoint ID
 * @param cluster_list - endpoint cluster list
 */
#define ZB_DECLARE_RANGE_EXTENDER_EP(ep_name, ep_id, cluster_list)                                                \
  ZB_ZCL_DECLARE_RANGE_EXTENDER_SIMPLE_DESC(ep_name, ep_id,                                                       \
                                            ZB_RANGE_EXTENDER_IN_CLUSTER_NUM, ZB_RANGE_EXTENDER_OUT_CLUSTER_NUM); \
  ZB_AF_DECLARE_ENDPOINT_DESC(ep_name, ep_id, ZB_AF_HA_PROFILE_ID, 0, NULL,                                       \
                              ZB_ZCL_ARRAY_SIZE(cluster_list, zb_zcl_cluster_desc_t), cluster_list,               \
                              (zb_af_simple_desc_1_1_t *)&simple_desc_##ep_name,                                  \
                              0, NULL, /* No reporting ctx */                                                     \
                              0, NULL) /* No CVC ctx */

/*! @} */

#endif /* ZB_RANGE_EXTENDER_H */
