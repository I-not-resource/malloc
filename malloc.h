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
//����
//--------------��ֲ�޸Ĵ�--------------
#define MEM_BLOCK_SIZE       32                                     //�ڴ���С
#define MEM1_MAX_SIZE        (100*1024)                             //��100K��С�ڲ��ڴ�
#define MEM1_ALLOC_BLOCK_SIZE (MEM1_MAX_SIZE/MEM_BLOCK_SIZE)         //�ڴ��1��С
#define MEM2_MAX_SIZE        0
#define MEM2_ALLOC_BLOCK_SIZE (MEM2_MAX_SIZE/MEM_BLOCK_SIZE)         //�ڴ��1��С
#define MEM3_MAX_SIZE        0
#define MEM3_ALLOC_BLOCK_SIZE (MEM2_MAX_SIZE/MEM_BLOCK_SIZE)         //�ڴ��1��С

#define MEMORY_TOTALY_BLOCK_SIZE   (MEM1_ALLOC_BLOCK_SIZE + MEM2_ALLOC_BLOCK_SIZE + MEM3_ALLOC_BLOCK_SIZE)

///////////////////////////////////////////
//�ڴ�ʹ�÷���


///////////////////////////////////////////
//�ڴ�״̬��
typedef union
{
	struct
	{
		uint8_t Using : 1;
		uint8_t MemLeve : 4;
		uint8_t MemPartner : 3;        //��ҳ����ƫ��
	}Format;
	uint8_t Byte;
}MemoryState_typedef;

//ҳ����
typedef struct
{
	void *PreAddress;                 //��һҳ�ڴ�λ��
	void *NextAddress;                //��һҳ�ڴ�λ��
	MemoryState_typedef MemoryState;  //�ڴ�״̬
}MemoryPageLevelOfNun_TypeDef;

//�ڴ������
typedef struct
{
	uint8_t LevelNum;
	MemoryPageLevelOfNun_TypeDef *MemoryPageLevelOf[11];
	uint32_t PageSizeOfLevel[11];

}MemoryDev_typedef;


//�ڴ���ֵ����
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









