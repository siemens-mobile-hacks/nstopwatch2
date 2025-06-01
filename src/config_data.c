#include <cfg_items.h>

__CFG_BEGIN(0)

__root CFG_HDR cfghdr_0 = {CFG_CHECKBOX, "Enable icon", 0, 2};
__root int CFG_ENABLE_ICONBAR_ICON = 1;

__root CFG_HDR cfghdr_1={CFG_UINT, "Icon ID", 1, 20000};
__root int CFG_ICON_ID = 633;

__CFG_END(0)
