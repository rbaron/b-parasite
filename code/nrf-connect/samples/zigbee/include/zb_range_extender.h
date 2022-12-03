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
#define ZB_RANGE_EXTENDER_IN_CLUSTER_NUM 2

/** Range extender OUT (client) clusters number */
#define ZB_RANGE_EXTENDER_OUT_CLUSTER_NUM 0

#define ZB_RANGE_EXTENDER_CLUSTER_NUM \
	(ZB_RANGE_EXTENDER_IN_CLUSTER_NUM + ZB_RANGE_EXTENDER_OUT_CLUSTER_NUM)

/** Number of attribute for reporting on Range extender device */
#define ZB_RANGE_EXTENDER_REPORT_ATTR_COUNT 0

/** @endcond */ /* internals_doc */

/**
 * @brief Declare cluster list for Range extender device
 * @param cluster_list_name - cluster list variable name
 * @param basic_attr_list - attribute list for Basic cluster
 * @param identify_attr_list - attribute list for Identify cluster
 */
#define ZB_DECLARE_RANGE_EXTENDER_CLUSTER_LIST(			      \
		cluster_list_name,				      \
		basic_attr_list,				      \
		identify_attr_list)				      \
zb_zcl_cluster_desc_t cluster_list_name[] =			      \
{								      \
	ZB_ZCL_CLUSTER_DESC(					      \
		ZB_ZCL_CLUSTER_ID_IDENTIFY,			      \
		ZB_ZCL_ARRAY_SIZE(identify_attr_list, zb_zcl_attr_t), \
		(identify_attr_list),				      \
		ZB_ZCL_CLUSTER_SERVER_ROLE,			      \
		ZB_ZCL_MANUF_CODE_INVALID			      \
	),							      \
	ZB_ZCL_CLUSTER_DESC(					      \
		ZB_ZCL_CLUSTER_ID_BASIC,			      \
		ZB_ZCL_ARRAY_SIZE(basic_attr_list, zb_zcl_attr_t),    \
		(basic_attr_list),				      \
		ZB_ZCL_CLUSTER_SERVER_ROLE,			      \
		ZB_ZCL_MANUF_CODE_INVALID			      \
	)							      \
}

/** @cond internals_doc */

/**
 * @brief Declare simple descriptor for Range extender device
 * @param ep_name - endpoint variable name
 * @param ep_id - endpoint ID
 * @param in_clust_num - number of supported input clusters
 * @param out_clust_num - number of supported output clusters
 */
#define ZB_ZCL_DECLARE_RANGE_EXTENDER_SIMPLE_DESC(ep_name, ep_id, in_clust_num, out_clust_num) \
	ZB_DECLARE_SIMPLE_DESC(in_clust_num, out_clust_num);				       \
	ZB_AF_SIMPLE_DESC_TYPE(in_clust_num, out_clust_num) simple_desc_##ep_name =	       \
	{										       \
		ep_id,									       \
		ZB_AF_HA_PROFILE_ID,							       \
		ZB_RANGE_EXTENDER_DEVICE_ID,						       \
		ZB_DEVICE_VER_RANGE_EXTENDER,						       \
		0,									       \
		in_clust_num,								       \
		out_clust_num,								       \
		{									       \
			ZB_ZCL_CLUSTER_ID_BASIC,					       \
			ZB_ZCL_CLUSTER_ID_IDENTIFY					       \
		}									       \
	}

/** @endcond */ /* internals_doc */

/**
 * @brief Declare endpoint for Range extender device
 * @param ep_name - endpoint variable name
 * @param ep_id - endpoint ID
 * @param cluster_list - endpoint cluster list
 */
#define ZB_DECLARE_RANGE_EXTENDER_EP(ep_name, ep_id, cluster_list)		      \
	ZB_ZCL_DECLARE_RANGE_EXTENDER_SIMPLE_DESC(ep_name, ep_id,		      \
		ZB_RANGE_EXTENDER_IN_CLUSTER_NUM, ZB_RANGE_EXTENDER_OUT_CLUSTER_NUM); \
	ZB_AF_DECLARE_ENDPOINT_DESC(ep_name, ep_id, ZB_AF_HA_PROFILE_ID, 0, NULL,     \
		ZB_ZCL_ARRAY_SIZE(cluster_list, zb_zcl_cluster_desc_t), cluster_list, \
			(zb_af_simple_desc_1_1_t *)&simple_desc_##ep_name,	      \
			0, NULL, /* No reporting ctx */				      \
			0, NULL) /* No CVC ctx */

/*! @} */

#endif /* ZB_RANGE_EXTENDER_H */
