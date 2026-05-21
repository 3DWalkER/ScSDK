#ifndef QORDEREDHASH_H
#define QORDEREDHASH_H

#include <QList>
#include <QHash>

template <class K, class V>
class QOrderedHash
{
public:
    /**
     * @brief value 根据键获取值
     */
    inline V value(const K &key) const { return hash.value(key); }

    /**
     * @brief keys 按照出入顺序获取所有键
     */
    inline QList<K> keys() const { return keyList; }

    /**
     * @brief values 按照出入顺序获取所有值
     */
    QList<V> values() const
    {
        QList<V> ret;
        ret.reserve(keyList.size());
        for (const K &key : keyList)
            ret.push_back(hash.value(key));
        return ret;
    }

    /**
     * @brief insert 插入新的键值对，如果值已存在，则按照最先插入的顺序输出
     */
    void insert(const K &key, const V &value)
    {
        if (!keyList.contains(key))
            keyList.append(key);
        hash[key] = value;
    }

    /**
     * @brief iterate 遍历所有元素
     * @param func 函数表达式
     */
    void iterate(const std::function<void(const K &, const V &)> &func) const {
        for (const K &key : keyList)
            func(key, hash.value(key));
    }

private:
    QList<K> keyList;
    QHash<K, V> hash;
};

#endif // QORDEREDHASH_H
