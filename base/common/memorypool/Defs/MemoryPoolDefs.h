/*!
 * MIT License
 *
 * Copyright (c) 2019 ericyonng<120453674@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file  : MemoryPoolDefs.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/8/6
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_MemoryPool_Defs_MemoryPoolDefs_H__
#define __Base_Common_MemoryPool_Defs_MemoryPoolDefs_H__
#pragma once

#undef __MEMORY_POOL_ALIGN_BYTES__
#define __MEMORY_POOL_ALIGN_BYTES__          (sizeof(void *)<<1)    // Ĭ��16�ֽڶ��� �漰����cache line����

#undef __FS_MEMORY_ALIGN__
#define __FS_MEMORY_ALIGN__(BYTES)  \
(BYTES) / __MEMORY_POOL_ALIGN_BYTES__ * __MEMORY_POOL_ALIGN_BYTES__ + ((BYTES)%__MEMORY_POOL_ALIGN_BYTES__ ? __MEMORY_POOL_ALIGN_BYTES__ : 0)

#undef __MEMORY_POOL_MINIMUM_BLOCK__
#define __MEMORY_POOL_MINIMUM_BLOCK__        64          // ��С�ڴ��64�ֽ�

#undef __MEMORY_POOL_MAXIMUM_BLOCK__
#define __MEMORY_POOL_MAXIMUM_BLOCK__        131072       // ����ڴ��128K ֻ֧��64�ı���

#undef __MEMORY_POOL_MAXBLOCK_LIMIT__
#define __MEMORY_POOL_MAXBLOCK_LIMIT__      __MEMORY_POOL_MAXIMUM_BLOCK__   // �ܹ�֧�ֵ�����ڴ�鷶Χ

#undef __MEMORY_POOL_MAX_EXPAND_BYTES__
#define __MEMORY_POOL_MAX_EXPAND_BYTES__    4294967296llu      // �ڴ�����ֻ֧�ֵ�4GB���������ʹ��ϵͳ����

#ifndef BLOCK_AMOUNT_DEF
#define BLOCK_AMOUNT_DEF    128    // Ĭ���ڴ������
#endif

#endif
