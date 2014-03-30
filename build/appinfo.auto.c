#include "pebble_app_info.h"
#include "src/resource_ids.auto.h"

const PebbleAppInfo __pbl_app_info __attribute__ ((section (".pbl_header"))) = {
  .header = "PBLAPP",
  .struct_version = { APP_INFO_CURRENT_STRUCT_VERSION_MAJOR, APP_INFO_CURRENT_STRUCT_VERSION_MINOR },
  .sdk_version = { APP_INFO_CURRENT_SDK_VERSION_MAJOR, APP_INFO_CURRENT_SDK_VERSION_MINOR },
  .app_version = { 2, 1 },
  .load_size = 0xb6b6,
  .offset = 0xb6b6b6b6,
  .crc = 0xb6b6b6b6,
  .name = "Morpheuz",
  .company = "James Fowler",
  .icon_resource_id = RESOURCE_ID_IMAGE_ICON,
  .sym_table_addr = 0xA7A7A7A7,
  .flags = 0,
  .num_reloc_entries = 0xdeadcafe,
  .uuid = { 0x5B, 0xE4, 0x4F, 0x1D, 0xD2, 0x62, 0x4E, 0xA6, 0xAA, 0x30, 0xDD, 0xBE, 0xC1, 0xE3, 0xCA, 0xB2 },
  .virtual_size = 0xb6b6
};
