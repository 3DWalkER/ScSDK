#ifndef QCONFIGBUILDER_H
#define QCONFIGBUILDER_H

#include "qconfigmain.h"
#include "qconfigcategory.h"
#include "qconfigentry.h"

#define QU_CATEGORIES_WITH_METANAME_AND_TITLE(Type, Title, MetaName, Body) \
    class Type##Categories : public QConfigMain \
    { \
        Q_DECLARE_SINGLETON(Type##Categories); \
        public: \
            Body \
        private: \
            Type##Categories(bool isPersistable) : QConfigMain(#Type, Title, MetaName, isPersistable) { } \
    };

#define QU_UI_CATEGORIES(Type, Body) \
    QU_CATEGORIES_WITH_METANAME_AND_TITLE(Type, QString(), "", Body)

#define QU_CATEGORIES_WITH_METANAME(Type, Body, MetaName) \
    QU_CATEGORIES_WITH_METANAME_AND_TITLE(Type, QString(), MetaName, Body)

#define QU_DEFINE_CATEGORIES(Type, Persistant) \
    Q_DEFINE_SIGNLETON_HUNGRY(Type##Categories, Persistant)

#define QU_CATEGORIES_WITH_METANAME_AND_TITLE_LAZY(Type, Title, MetaName, Body) \
    struct Type##Categories : public QConfigMain \
    { \
        Type##Categories(bool isPersistable) : QConfigMain(#Type, Title, MetaName, isPersistable) { } \
        Body \
    }; \
    Type##Categories *Type##Categories##Instance();

#define QU_DEFINE_CATEGORIES_LAZY(Type) \
    Type##Categories *p##Type##CategoriesInstance = nullptr; \
    void init##Type##CategoriesInstance() \
    {\
        p##Type##CategoriesInstance = new Type##Categories(true); \
    }\
    \
    Type##Categories *Type##CategoriesInstance() \
    { \
        return p##Type##CategoriesInstance; \
    } \
    QConfigLazyInitializer *configInstance##Type##LazyInit = new QConfigLazyInitializer(init##Type##CategoriesInstance);

#define QU_CATEGORIES_WITH_METANAME_LAZY(Type, Body, MetaName) \
    QU_CATEGORIES_WITH_METANAME_AND_TITLE_LAZY(Type, QString(), MetaName, Body)

#define QU_CATEGORY_WITH_TITLE(Name, Title, Body) \
    struct Name##Category : public QConfigCategory \
    { \
        Name##Category() : QConfigCategory(#Name, Title) { } \
        Body \
    }; \
    Name##Category Name;

#define QU_CATEGORY(Name, Body) \
    QU_CATEGORY_WITH_TITLE(Name, QString(), Body)

#define QU_ENTRY(Type, Name, ...) QConfigTypedEntry<Type> Name = QConfigTypedEntry<Type>(#Name, ##__VA_ARGS__);

#define QU_SHORTCUTS_METANAME "Shortcuts"

#define QU_SHORTCUTS_CATEGORIES(Type, Title, Entries) \
    QU_CATEGORIES_WITH_METANAME_LAZY(Shortcuts##Type, \
        QU_CATEGORY_WITH_TITLE(Shortcuts##Type, Title, Entries), QU_SHORTCUTS_METANAME)

#define QU_DEFINE_SHORTCUTS(Type) QU_DEFINE_CATEGORIES_LAZY(Shortcuts##Type)

#define QU_SHORTCUT_ENTRY(Name, KeyStr, Title) QU_ENTRY(QString, Name, QKeySequence(KeyStr).toString(), Title)

#endif // QCONFIGBUILDER_H
