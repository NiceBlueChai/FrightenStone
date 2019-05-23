#ifndef __Base_Common_BaseDefs_Structure_Defs_BinaryArray_H__
#define __Base_Common_BaseDefs_Structure_Defs_BinaryArray_H__
/**
 * @file    BinaryArray.h
 * @author  Huiya Song
 * @date    2019/03/24
 * @brief   ��������Ͷ�������
            ����ѯЧ��o(1),ֵ��ѯЧ��o(logn)
            ֵ����Ч�ʱƽ�o(logn)������vector�����ݿ��������Ӱ�����ܣ��������˼������п����Ƿ�ʹ�ã�
            ��������С���ݹ�ģ
 */

#pragma once

#include <base/exportbase.h>
#include "base/common/basedefs/Macro/MacroDefs.h"
#include <vector>
#include<map>

FS_NAMESPACE_BEGIN

// �ȽϷ���
#pragma region BALess
template<typename Value>
class BALess
{
public:
    bool operator()(const Value l, const Value r) const;
};
#pragma endregion

class BASE_EXPORT BinaryArrayDefs
{
public:
    enum :int
    {
        End = -1,           // ��Чpos
    };
};

// ��������Ͷ�������
template<typename Value, typename less = BALess<Value>>
class BinaryArray
{
#pragma region ������
public:
    typedef typename std::vector<Value>::iterator ArrayIterator;
    typedef typename std::vector<Value>::const_iterator ConstArrayIterator;

#pragma endregion 
    #pragma region ��ɾ������ͨ�ýӿ�
public:
    BinaryArray();

    int Add(Value v);                                                     // ��ͬ��value���뽫ʧ��
    void Erase(Value v);                                                        // v�ڵ㣨����v��֮��ڵ�keyȫ��ʧЧ
    void EraseByKey(int k);
    void Clear();
    int Find(Value v);
    void GetRange(int l, int r, std::map<int, Value> &kvs);

    typename BinaryArray<Value, less>::ArrayIterator begin();
    typename BinaryArray<Value, less>::ConstArrayIterator begin() const;
    typename BinaryArray<Value, less>::ArrayIterator end();
    typename BinaryArray<Value, less>::ConstArrayIterator end() const;

    void Sort();                                                                // �������ܲ������ã�������ɾ��Ԫ�غ�Add
    size_t Size();
    const Value *Data();
    #pragma endregion

    #pragma region �±��������
public:
    const Value &operator[](int k) const;
    Value &operator[](int k);
    bool operator <(const BinaryArray<Value, less> &other) const;
    #pragma endregion

    #pragma region �Ƚϴ�С
private:
    bool _IsLess(const Value &l, const Value &r) const;
    bool _IsEqual(const Value &l, const Value &r) const;
    bool _IsBigger(const Value &l, const Value &r) const;
    #pragma endregion

private:
    std::vector<Value> _elements;    // value Ĭ�ϰ���pos����
    less _less;
    int _left;
    int _mid;
    int _right;
    int _pos;
};

FS_NAMESPACE_END
#include "base/common/basedefs/Structure/Defs/BinaryArrayImpl.h"

#endif //!__Base_Common_BaseDefs_Structure_Defs_BinaryArray_H__
