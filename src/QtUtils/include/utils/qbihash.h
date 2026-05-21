#ifndef QBIHASH_H
#define QBIHASH_H

#include <QHash>

template <class K, class V>
class QBiHash
{
public:
    /**
     * @brief QBiHash 默认构造函数
     */
    QBiHash() { }

    /**
     * @brief QBiHash
     * @param list
     */
    QBiHash(std::initializer_list<std::pair<K, V>> list)
    {
        hash = QHash<K,V>(list);
        initInverted();
    }

    /**
     * @brief key 根据值获取键
     */
    inline K key(const V &val) const { return inverted.value(val); }

    /**
     * @brief value 根据键获取值
     */
    inline V value(const K &key) const { return hash.value(key); }

    /**
     * @brief containsKey 判断哈希表中是否包含指定的键
     */
    inline bool containsKey(const K &key) const { return hash.end() != hash.find(key); }

    /**
     * @brief containsKey 判断哈希表中是否包含指定的值
     */
    inline bool containsValue(const V &val) const { return inverted.end() != inverted.find(val); }

    /**
     * @brief count 统计哈希表中条目数
     */
    inline int count() const { return hash.size(); }

    /**
     * @brief isEmpty 判断哈希表是否为空
     */
    inline bool isEmpty() const { return hash.isEmpty(); }

    /**
     * @brief insert 在哈希表中插入新的键值对, 如果存在涉替换之前的
     */
    inline void insert(const K &key, const V &val)
    {
        hash[key] = val;
        inverted[val] = key;
    }

    /**
     * @brief removeKey 移除哈希表中指定的键
     */
    inline int removeKey(const K &key)
    {
        if (!containsKey(key))
            return 0;

        hash.remove(key);
        inverted.remove(hash.value(key));
        return 1;
    }

    /**
     * @brief removeKey 移除哈希表中指定的值
     */
    inline int removeValue(const K &key)
    {
        if (!containsValue(key))
            return 0;

        inverted.remove(key);
        hash.remove(inverted.value(key));
        return 1;
    }

    /**
     * @brief clear 清除哈希表中所有条目
     */
    inline void clear() { hash.clear(); inverted.clear(); }

    /**
     * @brief iterator 迭代器
     */
    inline QHashIterator<K, V> iterator() const { return QHashIterator<K, V>(hash); }

private:
    void initInverted()
    {
        QHashIterator<K,V> it(hash);
        while (it.hasNext())
        {
            it.next();
            inverted[it.value()] = it.key();
        }
    }

    /**
     * @brief hash 从键到值映射的哈希表
     */
    QHash<K, V> hash;

    /**
     * @brief inverted 从值到键映射的哈希表
     */
    QHash<V, K> inverted;
};

#endif // QBIHASH_H
