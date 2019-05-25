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
 * @file  : FS_Random.h
 * @author: ericyonng<120453674@qq.com>
 * @date  : 2019/5/25
 * @brief :
 * 
 *
 * 
 */
#ifndef __Base_Common_Component_Impl_FS_Random_H__
#define __Base_Common_Component_Impl_FS_Random_H__
#pragma once

#include "base/exportbase.h"
#include "base/common/basedefs/DataType/DataType.h"
#include "base/common/basedefs/Macro/MacroDefs.h"
#include <chrono>
#include <random>

class BASE_EXPORT FS_RandomDefs
{
public:

    //����ֲ�����
    enum RAND_DIS_TYPE
    {
        RAND_DIS_TYPE_SMALLINT = 0, // һ���������ϵ���ɢ���ȷֲ�
        RAND_DIS_TYPE_INT,          // һ���������ϵ���ɢ���ȷֲ�
        RAND_DIS_TYPE_01,           // [0, 1)�ϵ��������ȷֲ�
        RAND_DIS_TYPE_REAL,         // [min, max)�ϵ��������ȷֲ�
        RAND_DIS_TYPE_BERNOULLI,    // ��Ŭ���ֲ�
        RAND_DIS_TYPE_GEOMETRIC,    // ���ηֲ�
        RAND_DIS_TYPE_TRIANGLE,     // ���Ƿֲ�
        RAND_DIS_TYPE_EXPONENTIAL,  // ָ���ֲ�
        RAND_DIS_TYPE_NORMAL,       // ��̬�ֲ�
        RAND_DIS_TYPE_LOGNORMAL,    // �����ֲ�
        RAND_DIS_TYPE_ON_SPHERE,    // �����ϵľ��ȷֲ�
        RAND_DIS_TYPE_BETA,         // �����ֲ�
        RAND_DIS_TYPE_BINOMIAL,     // ����ֲ�
        RAND_DIS_TYPE_CAUCHY,       // �����ֲ�
        RAND_DIS_TYPE_DISCRETE,     // ��ɢ�ֲ�
    };

    //�����Դ�����㷨 �㷨�ٶ��иߵ��ͣ��㷨�����ɵ͵���
    enum  RAND_GEN_ALGORITHM_TYPE
    {
        RAND_GEN_ALGORITHM_TYPE_RAND48 = 0,             // rand48�㷨�����������
        RAND_GEN_ALGORITHM_TYPE_MT19937,                // mt19937�㷨�����������
        RAND_GEN_ALGORITHM_TYPE_MT19937_64,             // mt19937-64�㷨�����������
        RAND_GEN_ALGORITHM_TYPE_LAGGED_FIBONACCI19937,  // lagged_fibonacci19937�㷨�����������
    };

#undef RAND_DIS_NUM_SCOPE_MIN
#undef RAND_DIS_NUM_SCOPE_I64MAX
#undef RAND_DIS_NUM_SCOPE_I32MAX
#define RAND_DIS_NUM_SCOPE_MIN          0LL
#define RAND_DIS_NUM_SCOPE_INT64MAX     1152921504606846976LL
#define RAND_DIS_NUM_SCOPE_INT32MAX     1073741824

};

template<typename RandValType = Int64, FS_RandomDefs::RAND_GEN_ALGORITHM_TYPE RanmdomGenType = FS_RandomDefs::RAND_GEN_ALGORITHM_TYPE_MT19937_64, FS_RandomDefs::RAND_DIS_TYPE DisType = FS_RandomDefs::RAND_DIS_TYPE_NORMAL>
class BASE_EXPORT FS_Random
{
public:
    FS_Random();
    FS_Random(RandValType minVal, RandValType maxVal, RandValType srandVal);
    virtual ~FS_Random();
    // ���������
    operator typename RandValType()() const;

private:
    // ������ֲ���
    template<typename RandValType, FS_RandomDefs::RAND_DIS_TYPE>
    struct Distributor
    {

    };

    // �����Դ
    template<typename RandValType, FS_RandomDefs::RAND_GEN_ALGORITHM_TYPE>
    struct RandomSource
    {

    };

private:
    Distributor<RandValType, DisType>           _distributor;
    RandomSource<RandValType, RanmdomGenType>   _randomSource;
};

#include "base/common/component/Impl/FS_RandomImpl.h"

#endif
