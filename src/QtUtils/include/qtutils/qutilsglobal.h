#ifndef QUTILSGLOBAL_H
#define QUTILSGLOBAL_H

#include <QVariant>
#include <QDataStream>

#if defined(QU_API_EXPORTS)
#   define QU_API_EXPORT Q_DECL_EXPORT
#else
#   define QU_API_EXPORT Q_DECL_IMPORT
#endif

#define QU_BEGIN_NAMESPACE namespace Qu {
#define QU_END_NAMESPACE }

#define _QU(x) QString::fromLocal8Bit(x)

#define QU_SAVE_DELETE(ptr) \
    if (nullptr != ptr) { delete ptr; ptr = nullptr; }

#define QU_SAVE_DELETE_ARRAY(ptr) \
    if (nullptr != ptr) { delete [] ptr; ptr = nullptr; }

#define QU_SAVE_FREEE(ptr) \
    if (nullptr != ptr) { free(ptr); ptr = nullptr; }

#define Q_DECLARE_SINGLETON(Cls) \
    public: \
    static Cls *instance(); \
    static void destroy(); \
    private: \
    static Cls *m_pInstance;

#define Q_DEFINE_SIGNLETON_LAZY(Cls) \
    Cls *Cls::m_pInstance = new Cls(); \
    \
    Cls *Cls::instance() \
{ \
    return m_pInstance; \
    } \
    \
    void Cls::destroy() \
{\
    QU_SAVE_DELETE(m_pInstance); \
    }

#define Q_DEFINE_SIGNLETON_HUNGRY(Cls, ...) \
    Cls *Cls::m_pInstance = nullptr; \
    \
    Cls *Cls::instance() \
{ \
    if (nullptr == m_pInstance) \
    m_pInstance = new Cls(##__VA_ARGS__); \
    \
    return m_pInstance; \
    } \
    \
    void Cls::destroy() \
{\
    QU_SAVE_DELETE(m_pInstance); \
    }

#define Q_DEFINE_SIGNLETON_SAVE(Cls) \
    std::mutex Cls##_instance_mutex; \
    \
    Cls *Cls::m_pInstance = nullptr; \
    \
    Cls *Cls::instance() \
{ \
    std::lock_guard<std::mutex> lock(Cls##_instance_mutex); \
    if (nullptr == m_pInstance) \
    m_pInstance = new Cls(); \
    \
    return m_pInstance; \
    } \
    \
    void Cls::destroy() \
{\
    SAVE_DELETE(m_pInstance); \
    }

#define Q_DEFINE_SIGNLETON_SAVE_FAST(Cls) \
    std::mutex Cls##_instance_mutex; \
    \
    Cls *Cls::m_pInstance = nullptr; \
    \
    Cls *Cls::instance() \
{ \
    if (nullptr == m_pInstance) \
{\
    std::lock_guard<std::mutex> lock(Cls##_instance_mutex); \
    if (nullptr == m_pInstance) \
    m_pInstance = new Cls(); \
    }\
    \
    return m_pInstance; \
    } \
    \
    void Cls::destroy() \
{ \
    QU_SAVE_DELETE(m_pInstance); \
    }

#define QU_DEFINE_ICONS(TypeName, ObjName, Body) \
    struct TypeName \
{ \
    Body \
    }; \
    TypeName ObjName;

#define QU_ICON_N(E, N)         QIconContainer E = QIconContainer(#E, N);
#define QU_ICON_A(E, Src)       QIconContainer E = QIconContainer::aliasOf(#E, &Src);
#define QU_ICON_C(E, Src, Attr) QIconContainer E = QIconContainer::createFrom(#E, Src, QIconContainer::Attr);

QU_BEGIN_NAMESPACE

/**
 * @brief serialize 序列化
 */
template <typename T>
QVariant serialize(const T& list)
{
	QByteArray bytes;
	QDataStream ds(&bytes, QIODevice::WriteOnly);
	ds << list;
	return QVariant::fromValue(bytes);
}

/**
 * @brief serialize 反序列化
 */
template <typename T>
inline T deserialize(const QVariant& var)
{
	if (var.isNull() || !var.isValid())
		return T();

	T ret;
	QDataStream ds(var.toByteArray());
	ds >> ret;
	return ret;
}

QU_END_NAMESPACE

#endif // QUTILSGLOBAL_H
