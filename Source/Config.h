#pragma once

#if !defined TO_STRING && !defined __TO_STRING
# define __TO_STRING(value)        # value
# define TO_STRING(value)          __TO_STRING(value)
#endif

#define VCC_VERSION_MAJOR          1
#define VCC_VERSION_MINOR          0
#define VCC_VERSION_BUILD          0

#define VCC_VERSION_STRING         TO_STRING(VCC_VERSION_MAJOR) "." TO_STRING(VCC_VERSION_MINOR) "." TO_STRING(VCC_VERSION_BUILD)
#define VCC_VERSION_RSRC_NUM       VCC_VERSION_MAJOR, VCC_VERSION_MINOR, VCC_VERSION_BUILD, 0
#define VCC_VERSION_RSRC_STR       VCC_VERSION_STRING ".0"

#define VCC_OWNER_REPO             "SpriteOvO/VsCacheCleaner"
#define VCC_URL_REPO               "https://github.com/" VCC_OWNER_REPO
#define VCC_URL_ISSUES             VCC_URL_REPO "/issues"
#define VCC_URL_RELEASES           VCC_URL_REPO "/releases"
#define VCC_URL_LICENSE            VCC_URL_REPO "/blob/main/LICENSE"
#define VCC_URL_CURRENT_RELEASE    VCC_URL_RELEASES "/tag/" VCC_VERSION_STRING
#define VCC_LICENSE                "MIT License"
#define VCC_RESOURCE_ICON          ":/VsCacheCleaner/Resource/Icon.svg"
