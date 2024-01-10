#ifndef __RPX_ARRAY_H_
#define __RPX_ARRAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gctypes.h>
#include "../common/common.h"
#include "memory_area_table.h"

/* Struct used to organize rpx/rpl data in memory */
typedef struct _s_rpx_rpl
{
    struct _s_rpx_rpl* next;
    s_mem_area*        area;
    unsigned int       offset;
    unsigned int       size;
    unsigned char      is_rpx;
    char               name[0];
} s_rpx_rpl;

void rpxRplTableInit(void);
s_rpx_rpl* rpxRplTableAddEntry(const char *name, int offset, int size, int is_rpx, int entry_index, s_mem_area* area);
s_rpx_rpl* rpxRplTableGet(void);
int rpxRplTableGetCount(void);

s_mem_area *rpxRplTableGetNextFreeMemArea(uint32_t * mem_area_addr_start, uint32_t * mem_area_addr_end, uint32_t * mem_area_offset);
int rpxRplCopyDataToMem(s_rpx_rpl *rpx_rpl_struct, uint32_t fileOffset, const uint8_t *data, uint32_t dataSize);
int rpxRplCopyDataFromMem(s_rpx_rpl *rpx_rpl_struct, uint32_t fileOffset, uint8_t *data, uint32_t dataSize);

#ifdef __cplusplus
}
#endif


#endif
