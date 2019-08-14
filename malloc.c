/*-----------------------------------------------------
/
/File    :malloc.c
/By      :I-not-resource
/Version :v3.0
/        1、改变管理结构，将内存池进行分页，
/           借鉴linux的方式
/        2、借鉴Windows的方式，将部分内存信息写到
/           将要分配的内存的第一个字节
/        3、页格式(单位：块)
/           1:1 - 2:2 - 3:4 - 4:8 - 5:16 - 6:32 - 
/           7:64 - 8:128 - 9:256 - 10:512 - 11:1024
-----------------------------------------------------*/
#include "malloc.h"

uint8_t MemOneBase[MEM1_MAX_SIZE] = {0};

MemoryDev_typedef *MemoryDev;


/*--------------------------------------------------------------------------
函数名称： Memory_Init
函数功能： 内存管理初始化函数
函数备注：
--------------------------------------------------------------------------*/
void Memory_Init(void)
{
	uint32_t size, block_residue = 0, block_num = 0, offset = 0;
	void *blockbaseaddres;
	uint8_t level = 11, level_i = 0, multiple = 2, level_n, level_num, i;

	offset = (uint32_t)MemOneBase;
	MemoryDev = (MemoryDev_typedef*)offset;                  //内存池起始地址为内存管理地址

	offset += 3 * MEM_BLOCK_SIZE;
	while (level > 0)
	{
		size = (1 << level) * level;                                 //计算每一级内存最大页数量，倒数进行  最大页块数 * 2 * 级数
		if ((multiple = (MEMORY_TOTALY_BLOCK_SIZE - 3) / size) > 1)  //若现有内存大小大于等于该级别，则进行该级别内存规格初始化
			break;
		else
			level--;                                                 //若不足，则进行下一级比较
	}


	MemoryDev->LevelNum = level_n = level;                                      //level_n 用于递减运算
	blockbaseaddres = (void*)((uint32_t)MemOneBase + 3 * MEM_BLOCK_SIZE);       //内存块起始地址偏移96个字节，偏移大小用于内存管理器存储信息
	block_num = multiple * (1<< level);                                         //平均每级内存块数
	block_residue = MEMORY_TOTALY_BLOCK_SIZE - 3 - block_num * level;           //剩余内存块数

	offset = block_num * MEM_BLOCK_SIZE;                                        //偏移地址，块数 * 块大小
	for (level_i = 11; level_i > 1; level_i--)
	{
		if (level_i > level)
		{
			MemoryDev->PageSizeOfLevel[level_i - 1] = 0;
			MemoryDev->MemoryPageLevelOf[level_i - 1] = 0;
		}
		else
		{
			//当前页级别 页数
			MemoryDev->PageSizeOfLevel[level_i - 1] = multiple * (2 << (level - level_i));
			//当前页级别 页内存起始地址
			MemoryDev->MemoryPageLevelOf[level_i - 1] = (MemoryPageLevelOfNun_TypeDef*)((uint32_t)MemOneBase + MEM1_MAX_SIZE - offset * (level - level_i + 1));
			//页内存初始化
			pageMemoryInit(MemoryDev->MemoryPageLevelOf[level_i - 1], MemoryDev->PageSizeOfLevel[level_i - 1], level_i);
		}
	}
	MemoryDev->PageSizeOfLevel[0] = multiple * (1 << level) + block_residue;
	MemoryDev->MemoryPageLevelOf[0] = (MemoryPageLevelOfNun_TypeDef*)blockbaseaddres;
	pageMemoryInit(MemoryDev->MemoryPageLevelOf[0], MemoryDev->PageSizeOfLevel[0], 1);
}

/*--------------------------------------------------------------------------
函数名称： pageMemoryInit
函数功能： 页内存初始化
函数备注：
--------------------------------------------------------------------------*/
uint8_t pageMemoryInit(MemoryPageLevelOfNun_TypeDef *_pagehead, uint32_t _num, uint8_t _level)
{
	uint32_t i;
	MemoryPageLevelOfNun_TypeDef *pagebuf = 0, *pagetem = 0;
	pagetem = _pagehead;
	for (i = 0; i < _num; i++)
	{
		pagetem->MemoryState.Format.Using = 0;          //内存未被使用
		pagetem->MemoryState.Format.MemLeve = _level;   //内存所属页级别
		pagetem->MemoryState.Format.MemPartner = 0;     //所属父页级别
		pagetem->PreAddress = pagebuf;                  //内存上一级节点地址

		pagebuf = (MemoryPageLevelOfNun_TypeDef *)(((uint32_t)pagetem) + ((1 << (_level-1)) * MEM_BLOCK_SIZE));//计算下一节点的地址
		pagetem->NextAddress = pagebuf;                 //存放下一级节点地址

		pagetem = pagebuf;                              //将当前节点地址替换为缓存下一级节点地址
		pagebuf = (MemoryPageLevelOfNun_TypeDef*)pagetem->PreAddress;                  //将缓存节点替换为当前节点
	}
	return i;
}

/*--------------------------------------------------------------------------
函数名称： memoryPop
函数功能： 弹出空闲内存用于分配
函数备注：
--------------------------------------------------------------------------*/
void* memoryPop(MemoryPageLevelOfNun_TypeDef **_pagehead)
{

	MemoryPageLevelOfNun_TypeDef *pagebuf = 0;
	pagebuf = (*_pagehead);
	(*_pagehead) = (MemoryPageLevelOfNun_TypeDef *)pagebuf->NextAddress;
	(*_pagehead)->PreAddress = 0;

	return (void*)pagebuf;
}

/*--------------------------------------------------------------------------
函数名称： memoryPush
函数功能： 将内存压入页级内存池
函数备注：
--------------------------------------------------------------------------*/
uint8_t memoryPush(MemoryPageLevelOfNun_TypeDef **_pagehead, void* _addr)
{
	MemoryState_typedef buf;
	buf.Byte = ((MemoryState_typedef*)_addr)->Byte;                        //获取内存页信息
	((MemoryPageLevelOfNun_TypeDef*)_addr)->NextAddress = (*_pagehead);    //把将要压入的内存的下一个地址指向当前栈顶
	(*_pagehead)->PreAddress = _addr;                                      //把当前栈顶的上一个地址指向将要压入的内存
	((MemoryPageLevelOfNun_TypeDef*)_addr)->PreAddress = 0;                //把将要压入的内存的上一个地址指向为0
	(*_pagehead) = (MemoryPageLevelOfNun_TypeDef*)_addr;                   //将当前栈顶地址更换为压入的内存
	(*_pagehead)->MemoryState.Byte = buf.Byte;                            //将内存页信息写回
	(*_pagehead)->MemoryState.Format.Using = 0;                            //标记该内存页空闲
	return 0;
}

/*--------------------------------------------------------------------------
函数名称： memoryGet
函数功能： 获取一块内存页
函数备注：
--------------------------------------------------------------------------*/
void* memoryGet(uint32_t _size)
{
	uint8_t level, count, i;   //level 为级数
	void *buf;
	uint32_t addr = 0, block;
	//判断想要获取的内存大小属于哪个页级别的
	for (level = MemoryDev->LevelNum; level > 0; level--)
	{
		if (((_size + 32) / MEM_BLOCK_SIZE) >> (level-1) == 1)
		{
			if ((((_size + 32) / MEM_BLOCK_SIZE) - 1) >> (level - 1) == 1)
			{
				level += 1;
			}
			break;
		}
	}
	count = 0;
	while (0 == MemoryDev->PageSizeOfLevel[level - 1 + count])                      //如果为空，使用更大一级内存页
	{
		count++;
	}
	if (MemoryDev->LevelNum < (level + count) || level < 1)
	{
		goto error;
	}
	if (0 != count)
	{
		buf = memoryPop(&MemoryDev->MemoryPageLevelOf[level - 1 + count]);         //弹出内存页
		MemoryDev->PageSizeOfLevel[level - 1 + count]--;                           //该级页数量减一
		block = (1 << level);                                                      //原页级内存块数量
		for(i = 0; i < (2 << (count - 1)) - 1; i++)
		{
			addr = ((uint32_t)buf) + block * i * MEM_BLOCK_SIZE;                 //根据原内存页数量级对大内存页进行偏移分割
			((MemoryState_typedef*)addr)->Format.MemLeve = level;
			((MemoryState_typedef*)addr)->Format.MemPartner = count;
			MemoryDev->PageSizeOfLevel[level-1]++;
			memoryPush(&MemoryDev->MemoryPageLevelOf[level + count], (void*)addr);  //将偏移分割的内存页压入
		}
		addr = ((uint32_t)buf) + block * i * MEM_BLOCK_SIZE;                 //根据原内存页数量级对大内存页进行偏移分割
		((MemoryState_typedef*)addr)->Format.MemLeve = level;
		((MemoryState_typedef*)addr)->Format.MemPartner = count;
		((MemoryState_typedef*)addr)->Format.Using = 1;                            //标记该内存为原始内存
		addr -= 1;
	}
	else
	{
		buf = memoryPop(&MemoryDev->MemoryPageLevelOf[level-1]);        //弹出内存页
		MemoryDev->PageSizeOfLevel[level-1]--;                          //该级页数量减一
		((MemoryState_typedef*)buf)->Format.Using = 1;                  //标记该内存在使用
		((MemoryState_typedef*)buf)->Format.MemLeve = level;            //标记该内存所属级别
		((MemoryState_typedef*)buf)->Format.MemPartner = 0;             //标记该内存为原始内存
		addr = ((uint32_t)buf) + 1;
	}

error:
	return (void *)addr;
}


/*--------------------------------------------------------------------------
函数名称： memoryPut
函数功能： 放回一块内存
函数备注：
--------------------------------------------------------------------------*/
uint8_t memoryPut(void* _addr)
{
	uint8_t level;
	uint32_t buf;
	buf = (uint32_t)_addr - 1;

	level = ((MemoryState_typedef*)buf)->Format.MemLeve;
	MemoryDev->PageSizeOfLevel[level - 1]++;
	memoryPush(&MemoryDev->MemoryPageLevelOf[level - 1], (void*)buf);

	return 0;
}


/*--------------------------------------------------------------------------
函数名称：
函数功能：
函数备注：
--------------------------------------------------------------------------*/
void* malloc(uint32_t _size)
{
	return memoryGet(_size);
}

/*--------------------------------------------------------------------------
函数名称：
函数功能：
函数备注：
--------------------------------------------------------------------------*/
uint8_t free(void* _addr)
{
	uint8_t err;
	if (0 == _addr)
	{
		err = 1;
		goto error;
	}
	else
	{
		memoryPut(_addr);
		err = 0;
	}


error:
	return err;
}




















