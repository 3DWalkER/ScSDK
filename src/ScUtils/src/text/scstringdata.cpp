#include "scutils/text/scstringdata.h"

#include "scutils/utils/scendian.h"
#include "scutils/utils/scnumeric.h"
#include "scutils/memory/scmalloc.h"
#include "scutils/thread/scatomic.h"

#include <stdexcept>

SC_BEGIN_NAMESPACE

SC_BEGIN_DETAIL_NAMESPACE

template <typename Char>
struct RefCounted
{
	ScAtomicInteger<size_t> _ref;
	Char data[1];

	SC_CONSTEXPR static size_t offset() {
		return offsetof(RefCounted, data);
	}

	static RefCounted* fromData(Char* p) {
		return static_cast<RefCounted*>(static_cast<void*>(
			static_cast<unsigned char*>(static_cast<void*>(p)) - offset()));
	}

	static size_t refs(Char* p) {
		return fromData(p)->_ref.loadAcquire();
	}

	static void ref(Char* p) {
		fromData(p)->_ref.fetchAndAddOrdered(1);
	}

	static void deref(Char* p)
	{
		auto const dis = fromData(p);
		size_t oldCnt = dis->_ref.fetchAndSubOrdered(1);
		SC_ASSERT(oldCnt > 0);
		if (1 == oldCnt)
			::free(dis);
	}

	static RefCounted* create(size_t* size)
	{
		size_t bytes;
		if (Sc::add_overflow(*size, size_t(1), &bytes))
			throw std::length_error("");

		if (Sc::muladd_overflow(bytes, sizeof(Char), offset(), &bytes))
			throw std::length_error("");

		const size_t allocSize = Sc::goodMallocSize(bytes);
		RefCounted* result = static_cast<RefCounted*>(Sc::checkedMalloc(allocSize));
		result->_ref.storeRelease(1);
		*size = (allocSize - offset()) / sizeof(Char) - 1;
		return result;
	}

	static RefCounted* create(const Char* data, size_t* size)
	{
		const size_t effectSize = *size;
		auto result = create(size);
		if (SC_LIKELY(effectSize > 0))
			SC_DETAIL::podCopy(data, data + effectSize, result->data);
		return result;
	}

	static RefCounted* reallocate(Char * const data, const size_t currSize, const size_t currCapacity, size_t *newCapacity)
	{
		SC_ASSERT(*newCapacity > 0 && *newCapacity > currSize);
		size_t bytes;
		if (Sc::add_overflow(*newCapacity, size_t(1), &bytes))
			throw std::length_error("");

		if (Sc::muladd_overflow(bytes, sizeof(Char), offset(), &bytes))
			throw std::length_error("");

		const size_t allocSize = Sc::goodMallocSize(bytes);
		auto const dis = fromData(data);
		SC_ASSERT(1 == dis->_ref.loadAcquire());
		auto result = static_cast<RefCounted*>(Sc::smartRealloc(
			dis,
			offset() + (currSize + 1) * sizeof(Char),
			offset() + (currCapacity + 1) * sizeof(Char),
			allocSize
		));
		SC_ASSERT(1 == dis->_ref.loadAcquire());
		*newCapacity = (allocSize - offset()) / sizeof(Char) - 1;
		return result;
	}
};

SC_END_DETAIL_NAMESPACE

SC_END_NAMESPACE

using ScRefCounted = SC_DETAIL::RefCounted<char>;

ScStringData::ScStringData(const char* const data, size_t size)
	: _bytes{}, _small{}
{
	if (!data) return;

	if (ScStringData::npos == size)
		size = strlen(data);

	if (size <= kMaxSmallSize)
	{
		if (size > 0)
			std::memcpy(_small, data, size);
		setSmallSize(size);
	}
	else if (size <= kMaxMediumSize)
	{
		auto const allocSize = Sc::goodMallocSize((1 + size) * sizeof(char));
		ml.data = static_cast<char*>(Sc::checkedMalloc(allocSize));
		if (SC_LIKELY(size > 0))
			SC_DETAIL::podCopy(data, data + size, ml.data);
		ml.size = size;
		ml.setCapacity(allocSize / sizeof(char) - 1, Category::Medium);
		ml.data[size] = '\0';
	}
	else
	{
		size_t effectCapacity = size;
		auto const newRc = ScRefCounted::create(data, &effectCapacity);
		ml.data = newRc->data;
		ml.size = size;
		ml.setCapacity(effectCapacity, Category::Large);
		ml.data[size] = '\0';
	}
	auto ca = category();
	SC_ASSERT(this->size() == size);
	SC_ASSERT(0 == size || memcmp(this->data(), data, size * sizeof(char)) == 0);
}

ScStringData::ScStringData(const ScStringData& other)
{
	SC_ASSERT(&other != this);
	switch (other.category())
	{
	case Category::Small:
	{
		SC_STATIC_ASSERT_X(offsetof(ScMediumLargeString, data) == 0, "ScString layout failure.");
		SC_STATIC_ASSERT_X(offsetof(ScMediumLargeString, size) == sizeof(ml.data), "ScString layout failure.");
		SC_STATIC_ASSERT_X(offsetof(ScMediumLargeString, _capacity) == 2 * sizeof(ml.data), "ScString layout failure.");
		ml = other.ml;
		SC_ASSERT(Category::Small == category());
		break;
	}
	case Category::Medium:
	{
		auto const allocSize = Sc::goodMallocSize((1 + other.ml.size) * sizeof(char));
		ml.data = static_cast<char*>(Sc::checkedMalloc(allocSize));
		SC_DETAIL::podCopy(other.ml.data, other.ml.data + 1 + other.ml.size, ml.data);
		ml.size = other.ml.size;
		ml.setCapacity(allocSize / sizeof(char) - 1, Category::Medium);
		break;
	}
	case Category::Large:
		ml = other.ml;
		ScRefCounted::ref(ml.data);
		SC_ASSERT(Category::Large == category() && size() == other.size());
		break;
	default:
		break;
	}
}

ScStringData::~ScStringData()
{
	auto const cat = category();
	if (Category::Small == cat)
		return;

	if (Category::Medium == cat)
		free(ml.data);
	else
		ScRefCounted::deref(ml.data);
}

size_t ScStringData::capacity() const
{
	switch (category())
	{
	case Category::Small:
		return kMaxSmallSize;
	case Category::Large:
		if (ScRefCounted::refs(ml.data) > 1)
			return ml.size;
	default:
		return ml.capacity();
	}
}

char* ScStringData::mutableData()
{
	switch (category())
	{
	case Category::Small:
		return _small;
	case Category::Medium:
		return ml.data;
	case Category::Large:
		if (ScRefCounted::refs(ml.data) > 1)
			unshare();
		return ml.data;
	default:
		SC_UNREACHABLE();
	}
	return nullptr;
}

void ScStringData::shrink(size_t delta)
{
	const Category cat = category();
	if (Category::Small == cat)
	{
		SC_ASSERT(delta <= smallSize());
		setSmallSize(smallSize() - delta);
	}
	else if (Category::Medium == cat || 1 == ScRefCounted::refs(ml.data))
	{
		SC_ASSERT(ml.size >= delta);
		ml.size -= delta;
		ml.data[ml.size] = '\0';
	}
	else
	{
		SC_ASSERT(ml.size >= delta);
		if (delta)
			ScStringData(ml.data, ml.size - delta).swap(*this);
	}
}

void ScStringData::reserve(size_t minCapacity)
{
	switch (category())
	{
	case Category::Small:
		reserveSmall(minCapacity);
		break;
	case Category::Medium:
		reserveMedium(minCapacity);
		break;
	case Category::Large:
		reserveLarge(minCapacity);
		break;
	default:
		SC_UNREACHABLE();
		break;
	}
	SC_ASSERT(capacity() >= minCapacity);
}

void ScStringData::reserveSmall(size_t minCapacity)
{
	SC_ASSERT(Category::Small == category());
	if (minCapacity <= kMaxSmallSize)
		return;

	if (minCapacity <= kMaxMediumSize)
	{
		auto const allocSize = Sc::goodMallocSize((1 + minCapacity) * sizeof(char));
		const auto pData = static_cast<char*>(Sc::checkedMalloc(allocSize));
		const auto size = smallSize();
		
		SC_DETAIL::podCopy(_small, _small + size + 1, pData);
		ml.data = pData;
		ml.size = size;
		ml.setCapacity(allocSize / sizeof(char) - 1, Category::Medium);
	}
	else
	{
		auto const newRc = ScRefCounted::create(&minCapacity);
		const auto size = smallSize();

		SC_DETAIL::podCopy(_small, _small + size + 1, newRc->data);
		ml.data = newRc->data;
		ml.size = size;
		ml.setCapacity(minCapacity, Category::Large);
		SC_ASSERT(capacity() >= minCapacity);
	}
}

void ScStringData::reserveMedium(size_t minCapacity)
{
	SC_ASSERT(Category::Medium == category());
	if (minCapacity <= ml.capacity())
		return;

	if (minCapacity <= kMaxMediumSize)
	{
		auto const allocSize = Sc::goodMallocSize((1 + minCapacity) * sizeof(char));
		ml.data = reinterpret_cast<char*>(Sc::smartRealloc(ml.data, (ml.size + 1) * sizeof(char)
			, (ml.capacity() + 1) * sizeof(char), allocSize));
		ml.setCapacity(allocSize / sizeof(char) - 1, Category::Medium);
	}
	else
	{
		ScStringData nascent;
		nascent.reserve(minCapacity);
		nascent.ml.size = ml.size;
		SC_DETAIL::podCopy(ml.data, ml.data + ml.size + 1, nascent.ml.data);
		nascent.swap(*this);
		SC_ASSERT(capacity() >= minCapacity);
	}
}

void ScStringData::reserveLarge(size_t minCapacity)
{
	SC_ASSERT(Category::Large == category());
	if (ScRefCounted::refs(ml.data) <= 1)
	{
		if (minCapacity > ml.capacity())
		{
			const auto newRc = ScRefCounted::reallocate(ml.data, ml.size, ml.capacity(), &minCapacity);
			ml.data = newRc->data;
			ml.setCapacity(minCapacity, Category::Large);
		}
		SC_ASSERT(capacity() >= minCapacity);
	}
	else
		unshare(minCapacity);
}

void ScStringData::unshare(size_t minCapacity)
{
	SC_ASSERT(Category::Large == category());
	size_t effectCapacity = std::max(minCapacity, ml.capacity());
	const auto newRc = ScRefCounted::create(&effectCapacity);

	SC_ASSERT(effectCapacity >= ml.capacity());
	SC_DETAIL::podCopy(ml.data, ml.data + ml.size + 1, newRc->data);
	ScRefCounted::deref(ml.data);
	ml.data = newRc->data;
	ml.setCapacity(effectCapacity, Category::Large);
}

char *ScStringData::expand(size_t delta, bool isExpGrowth)
{
	SC_ASSERT(capacity() >= size());

	size_t size, newSize;
	if (category() == Category::Small)
	{
		size = smallSize();
		newSize = size + delta;
		if (SC_LIKELY(newSize <= kMaxSmallSize))
		{
			setSmallSize(newSize);
			return _small + size;
		}
		reserveSmall(isExpGrowth ? std::max(newSize, 2 * kMaxSmallSize) : newSize);
	}
	else
	{
		size = ml.size;
		newSize = size + delta;
		if (SC_UNLIKELY(newSize > capacity()))
			reserve(isExpGrowth ? std::max(newSize, 1 + capacity() * 3 / 2) : newSize);
	}

	SC_ASSERT(capacity() >= newSize);
	SC_ASSERT(Category::Medium == category() || Category::Large == category());
	ml.size = newSize;
	ml.data[ml.size] = '\0';
	SC_ASSERT(this->size() == newSize);
	return ml.data + size;
}

bool ScStringData::isShared() const
{
	return Category::Large == category() && ScRefCounted::refs(ml.data) > 1;
}
