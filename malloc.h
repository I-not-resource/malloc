#pragma once

/*-----------------------------------------------------
/
/File    :malloc.h
/By      :I-not-resource
/Version :v3.0
/
-----------------------------------------------------*/
#ifndef __MALLOC_H__
#define __MALLOC_H__
#include "stdint.h"
#pragma pack(1)
///////////////////////////////////////////
//设置
//--------------移植修改处--------------
#define MEM_BLOCK_SIZE       32                                     //内存块大小
#define MEM1_MAX_SIZE        (100*1024)                             //开100K大小内部内存
#define MEM1_ALLOC_BLOCK_SIZE (MEM1_MAX_SIZE/MEM_BLOCK_SIZE)         //内存表1大小
#define MEM2_MAX_SIZE        0
#define MEM2_ALLOC_BLOCK_SIZE (MEM2_MAX_SIZE/MEM_BLOCK_SIZE)         //内存表1大小
#define MEM3_MAX_SIZE        0
#define MEM3_ALLOC_BLOCK_SIZE (MEM2_MAX_SIZE/MEM_BLOCK_SIZE)         //内存表1大小

#define MEMORY_TOTALY_BLOCK_SIZE   (MEM1_ALLOC_BLOCK_SIZE + MEM2_ALLOC_BLOCK_SIZE + MEM3_ALLOC_BLOCK_SIZE)

///////////////////////////////////////////
//内存使用符号


///////////////////////////////////////////
//内存状态器
typedef union
{
	struct
	{
		uint8_t Using : 1;
		uint8_t MemLeve : 4;
		uint8_t MemPartner : 3;        //父页级别偏移
	}Format;
	uint8_t Byte;
}MemoryState_typedef;

//页管理
typedef struct
{
	void *PreAddress;                 //上一页内存位置
	void *NextAddress;                //下一页内存位置
	MemoryState_typedef MemoryState;  //内存状态
}MemoryPageLevelOfNun_TypeDef;

//内存管理器
typedef struct
{
	uint8_t LevelNum;
	MemoryPageLevelOfNun_TypeDef *MemoryPageLevelOf[11];
	uint32_t PageSizeOfLevel[11];

}MemoryDev_typedef;


//内存阈值级别
typedef enum
{
	Memory_Level_1x = 0,
	Memory_Level_2x,
	Memory_Level_3x,
	Memory_Level_4x,
	Memory_Level_5x,
	Memory_Level_6x,
	Memory_Level_7x,
	Memory_Level_8x,
	Memory_Level_9x,
	Memory_Level_10x,
	Memory_Level_11x
}memory_level_tyepdef;



void Memory_Init(void);
uint8_t pageMemoryInit(MemoryPageLevelOfNun_TypeDef *_pagehead, uint32_t _num, uint8_t _level);
uint8_t pageMemoryInit(MemoryPageLevelOfNun_TypeDef *_pagehead, uint32_t _num, uint8_t _level);
void* memoryPop(MemoryPageLevelOfNun_TypeDef **_pagehead);
uint8_t memoryPush(MemoryPageLevelOfNun_TypeDef **_pagehead, void* _addr);
void* memoryGet(uint32_t _size);
uint8_t memoryPut(void* _addr);

void* malloc(uint32_t _size);
uint8_t free(void* _addr);
#endif









