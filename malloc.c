/*-----------------------------------------------------
/
/File    :malloc.c
/By      :I-not-resource
/Version :v3.0
/        1���ı����ṹ�����ڴ�ؽ��з�ҳ��
/           ���linux�ķ�ʽ
/        2�����Windows�ķ�ʽ���������ڴ���Ϣд��
/           ��Ҫ������ڴ�ĵ�һ���ֽ�
/        3��ҳ��ʽ(��λ����)
/           1:1 - 2:2 - 3:4 - 4:8 - 5:16 - 6:32 - 
/           7:64 - 8:128 - 9:256 - 10:512 - 11:1024
-----------------------------------------------------*/
#include "malloc.h"

uint8_t MemOneBase[MEM1_MAX_SIZE] = {0};

MemoryDev_typedef *MemoryDev;


/*--------------------------------------------------------------------------
�������ƣ� Memory_Init
�������ܣ� �ڴ�����ʼ������
������ע��
--------------------------------------------------------------------------*/
void Memory_Init(void)
{
	uint32_t size, block_residue = 0, block_num = 0, offset = 0;
	void *blockbaseaddres;
	uint8_t level = 11, level_i = 0, multiple = 2, level_n, level_num, i;

	offset = (uint32_t)MemOneBase;
	MemoryDev = (MemoryDev_typedef*)offset;                  //�ڴ����ʼ��ַΪ�ڴ�����ַ

	offset += 3 * MEM_BLOCK_SIZE;
	while (level > 0)
	{
		size = (1 << level) * level;                                 //����ÿһ���ڴ����ҳ��������������  ���ҳ���� * 2 * ����
		if ((multiple = (MEMORY_TOTALY_BLOCK_SIZE - 3) / size) > 1)  //�������ڴ��С���ڵ��ڸü�������иü����ڴ����ʼ��
			break;
		else
			level--;                                                 //�����㣬�������һ���Ƚ�
	}


	MemoryDev->LevelNum = level_n = level;                                      //level_n ���ڵݼ�����
	blockbaseaddres = (void*)((uint32_t)MemOneBase + 3 * MEM_BLOCK_SIZE);       //�ڴ����ʼ��ַƫ��96���ֽڣ�ƫ�ƴ�С�����ڴ�������洢��Ϣ
	block_num = multiple * (1<< level);                                         //ƽ��ÿ���ڴ����
	block_residue = MEMORY_TOTALY_BLOCK_SIZE - 3 - block_num * level;           //ʣ���ڴ����

	offset = block_num * MEM_BLOCK_SIZE;                                        //ƫ�Ƶ�ַ������ * ���С
	for (level_i = 11; level_i > 1; level_i--)
	{
		if (level_i > level)
		{
			MemoryDev->PageSizeOfLevel[level_i - 1] = 0;
			MemoryDev->MemoryPageLevelOf[level_i - 1] = 0;
		}
		else
		{
			//��ǰҳ���� ҳ��
			MemoryDev->PageSizeOfLevel[level_i - 1] = multiple * (2 << (level - level_i));
			//��ǰҳ���� ҳ�ڴ���ʼ��ַ
			MemoryDev->MemoryPageLevelOf[level_i - 1] = (MemoryPageLevelOfNun_TypeDef*)((uint32_t)MemOneBase + MEM1_MAX_SIZE - offset * (level - level_i + 1));
			//ҳ�ڴ��ʼ��
			pageMemoryInit(MemoryDev->MemoryPageLevelOf[level_i - 1], MemoryDev->PageSizeOfLevel[level_i - 1], level_i);
		}
	}
	MemoryDev->PageSizeOfLevel[0] = multiple * (1 << level) + block_residue;
	MemoryDev->MemoryPageLevelOf[0] = (MemoryPageLevelOfNun_TypeDef*)blockbaseaddres;
	pageMemoryInit(MemoryDev->MemoryPageLevelOf[0], MemoryDev->PageSizeOfLevel[0], 1);
}

/*--------------------------------------------------------------------------
�������ƣ� pageMemoryInit
�������ܣ� ҳ�ڴ��ʼ��
������ע��
--------------------------------------------------------------------------*/
uint8_t pageMemoryInit(MemoryPageLevelOfNun_TypeDef *_pagehead, uint32_t _num, uint8_t _level)
{
	uint32_t i;
	MemoryPageLevelOfNun_TypeDef *pagebuf = 0, *pagetem = 0;
	pagetem = _pagehead;
	for (i = 0; i < _num; i++)
	{
		pagetem->MemoryState.Format.Using = 0;          //�ڴ�δ��ʹ��
		pagetem->MemoryState.Format.MemLeve = _level;   //�ڴ�����ҳ����
		pagetem->MemoryState.Format.MemPartner = 0;     //������ҳ����
		pagetem->PreAddress = pagebuf;                  //�ڴ���һ���ڵ��ַ

		pagebuf = (MemoryPageLevelOfNun_TypeDef *)(((uint32_t)pagetem) + ((1 << (_level-1)) * MEM_BLOCK_SIZE));//������һ�ڵ�ĵ�ַ
		pagetem->NextAddress = pagebuf;                 //�����һ���ڵ��ַ

		pagetem = pagebuf;                              //����ǰ�ڵ��ַ�滻Ϊ������һ���ڵ��ַ
		pagebuf = (MemoryPageLevelOfNun_TypeDef*)pagetem->PreAddress;                  //������ڵ��滻Ϊ��ǰ�ڵ�
	}
	return i;
}

/*--------------------------------------------------------------------------
�������ƣ� memoryPop
�������ܣ� ���������ڴ����ڷ���
������ע��
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
�������ƣ� memoryPush
�������ܣ� ���ڴ�ѹ��ҳ���ڴ��
������ע��
--------------------------------------------------------------------------*/
uint8_t memoryPush(MemoryPageLevelOfNun_TypeDef **_pagehead, void* _addr)
{
	MemoryState_typedef buf;
	buf.Byte = ((MemoryState_typedef*)_addr)->Byte;                        //��ȡ�ڴ�ҳ��Ϣ
	((MemoryPageLevelOfNun_TypeDef*)_addr)->NextAddress = (*_pagehead);    //�ѽ�Ҫѹ����ڴ����һ����ַָ��ǰջ��
	(*_pagehead)->PreAddress = _addr;                                      //�ѵ�ǰջ������һ����ַָ��Ҫѹ����ڴ�
	((MemoryPageLevelOfNun_TypeDef*)_addr)->PreAddress = 0;                //�ѽ�Ҫѹ����ڴ����һ����ַָ��Ϊ0
	(*_pagehead) = (MemoryPageLevelOfNun_TypeDef*)_addr;                   //����ǰջ����ַ����Ϊѹ����ڴ�
	(*_pagehead)->MemoryState.Byte = buf.Byte;                            //���ڴ�ҳ��Ϣд��
	(*_pagehead)->MemoryState.Format.Using = 0;                            //��Ǹ��ڴ�ҳ����
	return 0;
}

/*--------------------------------------------------------------------------
�������ƣ� memoryGet
�������ܣ� ��ȡһ���ڴ�ҳ
������ע��
--------------------------------------------------------------------------*/
void* memoryGet(uint32_t _size)
{
	uint8_t level, count, i;   //level Ϊ����
	void *buf;
	uint32_t addr = 0, block;
	//�ж���Ҫ��ȡ���ڴ��С�����ĸ�ҳ�����
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
	while (0 == MemoryDev->PageSizeOfLevel[level - 1 + count])                      //���Ϊ�գ�ʹ�ø���һ���ڴ�ҳ
	{
		count++;
	}
	if (MemoryDev->LevelNum < (level + count) || level < 1)
	{
		goto error;
	}
	if (0 != count)
	{
		buf = memoryPop(&MemoryDev->MemoryPageLevelOf[level - 1 + count]);         //�����ڴ�ҳ
		MemoryDev->PageSizeOfLevel[level - 1 + count]--;                           //�ü�ҳ������һ
		block = (1 << level);                                                      //ԭҳ���ڴ������
		for(i = 0; i < (2 << (count - 1)) - 1; i++)
		{
			addr = ((uint32_t)buf) + block * i * MEM_BLOCK_SIZE;                 //����ԭ�ڴ�ҳ�������Դ��ڴ�ҳ����ƫ�Ʒָ�
			((MemoryState_typedef*)addr)->Format.MemLeve = level;
			((MemoryState_typedef*)addr)->Format.MemPartner = count;
			MemoryDev->PageSizeOfLevel[level-1]++;
			memoryPush(&MemoryDev->MemoryPageLevelOf[level + count], (void*)addr);  //��ƫ�Ʒָ���ڴ�ҳѹ��
		}
		addr = ((uint32_t)buf) + block * i * MEM_BLOCK_SIZE;                 //����ԭ�ڴ�ҳ�������Դ��ڴ�ҳ����ƫ�Ʒָ�
		((MemoryState_typedef*)addr)->Format.MemLeve = level;
		((MemoryState_typedef*)addr)->Format.MemPartner = count;
		((MemoryState_typedef*)addr)->Format.Using = 1;                            //��Ǹ��ڴ�Ϊԭʼ�ڴ�
		addr -= 1;
	}
	else
	{
		buf = memoryPop(&MemoryDev->MemoryPageLevelOf[level-1]);        //�����ڴ�ҳ
		MemoryDev->PageSizeOfLevel[level-1]--;                          //�ü�ҳ������һ
		((MemoryState_typedef*)buf)->Format.Using = 1;                  //��Ǹ��ڴ���ʹ��
		((MemoryState_typedef*)buf)->Format.MemLeve = level;            //��Ǹ��ڴ���������
		((MemoryState_typedef*)buf)->Format.MemPartner = 0;             //��Ǹ��ڴ�Ϊԭʼ�ڴ�
		addr = ((uint32_t)buf) + 1;
	}

error:
	return (void *)addr;
}


/*--------------------------------------------------------------------------
�������ƣ� memoryPut
�������ܣ� �Ż�һ���ڴ�
������ע��
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
�������ƣ�
�������ܣ�
������ע��
--------------------------------------------------------------------------*/
void* malloc(uint32_t _size)
{
	return memoryGet(_size);
}

/*--------------------------------------------------------------------------
�������ƣ�
�������ܣ�
������ע��
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




















