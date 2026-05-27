#ifndef QSTRINGHASH_H
#define QSTRINGHASH_H

#include <QHash>

template <class T>
class QStringHash
{
public:
    QStringHash() { }
    QStringHash(std::initializer_list<std::pair<QString, T>> list);
    QStringHash(const QHash<QString, T> &other);

    void insert(const QString &key, const T &value);

    QStringList keys() const { return hash.keys(); }

    T value(const QString &key, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    QList<T> values() const { return hash.values(); }

    T take(const QString &key, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    int	remove(const QString &key, Qt::CaseSensitivity cs = Qt::CaseSensitive);

    int count() const { return hash.count(); }
    int count(const QString &key, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool isEmpty() const { return hash.isEmpty(); }
    bool contains(const QString &key, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

    T &operator[](const QString &key);
    const T operator[](const QString& key) const { return hash[lowerCaseHash[key.toLower()]]; }

    QHashIterator<QString, T> iterator() const { return QHashIterator<QString, T>(hash); }

    void clear() { hash.clear(); lowerCaseHash.clear(); }

private:
    void toLowerCastHash();

    QHash<QString, T> hash;
    QHash<QString, QString> lowerCaseHash;
};

template<class T>
QStringHash<T>::QStringHash(std::initializer_list<std::pair<QString, T> > list)
    : hash(QHash<QString, T>(list))
{
    toLowerCastHash();
}

template<class T>
QStringHash<T>::QStringHash(const QHash<QString, T> &other)
    : hash(QHash<QString, T>(other))
{
    toLowerCastHash();
}

template<class T>
void QStringHash<T>::insert(const QString &key, const T &value)
{
    if (lowerCaseHash.contains(key.toLower()))
        remove(key, Qt::CaseInsensitive);

    hash.insert(key, value);
    lowerCaseHash.insert(key.toLower(), key);
}

template<class T>
T QStringHash<T>::value(const QString &key, Qt::CaseSensitivity cs) const
{
    if (Qt::CaseInsensitive == cs)
        return hash.value(key);
    return hash.value(lowerCaseHash.value(key.toLower()));
}

template<class T>
T QStringHash<T>::take(const QString &key, Qt::CaseSensitivity cs)
{
    if (Qt::CaseSensitive == cs)
    {
        lowerCaseHash.remove(key.toLower());
        return hash.take(key);
    }

    QString lowerKey = key.toLower();
    if (lowerCaseHash.contains(lowerKey))
    {
        QString theKey = lowerCaseHash.value(lowerKey);
        lowerCaseHash.remove(lowerKey);
        return hash.take(theKey);
    }

    return QString();
}

template<class T>
int QStringHash<T>::remove(const QString &key, Qt::CaseSensitivity cs)
{
    if (Qt::CaseSensitive == cs)
    {
        int res = hash.remove(key);
        if (res > 0)
            lowerCaseHash.remove(key.toLower());

        return res;
    }

    QString lowerKey = key.toLower();
    if (lowerCaseHash.contains(lowerKey))
    {
        int res = hash.remove(lowerCaseHash.value(lowerKey));
        lowerCaseHash.remove(lowerKey);
        return res;
    }

    return 0;
}

template<class T>
int QStringHash<T>::count(const QString &key, Qt::CaseSensitivity cs) const
{
    if (cs == Qt::CaseSensitive)
        return hash.count(key);
    return lowerCaseHash.count(key.toLower());
}

template<class T>
bool QStringHash<T>::contains(const QString &key, Qt::CaseSensitivity cs) const
{
    if (Qt::CaseSensitive == cs)
        return hash.contains(key);

    return lowerCaseHash.contains(key.toLower());
}

template<class T>
T &QStringHash<T>::operator[](const QString &key)
{
    if (lowerCaseHash.contains(key.toLower()) && !hash.contains(key))
    {
        T value = hash[lowerCaseHash[key.toLower()]];
        remove(key, Qt::CaseInsensitive);
        hash[key] = value;
    }

    lowerCaseHash[key.toLower()] = key;
    return hash[key];
}

template<class T>
void QStringHash<T>::toLowerCastHash()
{
    QHashIterator<QString, T> it(hash);
    while (it.hasNext())
    {
        it.next();
        lowerCaseHash[it.key().toLower()] = it.key();
    }
}

#endif // QSTRINGHASH_H
