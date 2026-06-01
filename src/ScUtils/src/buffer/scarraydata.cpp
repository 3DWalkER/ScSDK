#include "scutils/buffer/scarraydata.h"

#include "scutils/utils/scnumeric.h"
#include "scutils/utils/scmath.h"

const ScArrayData ScArrayData::shared_null[2] = {
	{ SC_REFCOUNT_INITIALIZE_STATIC, 0, 0, 0, sizeof(ScArrayData) },
};

static const ScArrayData sc_array[3] = {
	{ SC_REFCOUNT_INITIALIZE_STATIC, 0, 0, 0, sizeof(ScArrayData) },
	{ ATOMIC_VAR_INIT(0), 0, 0, 0, sizeof(ScArrayData)},
};

static const ScArrayData& sc_array_empty = sc_array[0];
static const ScArrayData& sc_array_unsharable_empty = sc_array[1];

size_t calculateBlockSize(size_t elementCount, size_t elementSize, size_t headerSize) noexcept
{
	unsigned count = unsigned(elementCount);
	unsigned size = unsigned(elementSize);
	unsigned header = unsigned(headerSize);
	SC_ASSERT(elementSize);
	SC_ASSERT(size == elementSize);
	SC_ASSERT(header == headerSize);

	if (SC_UNLIKELY(count != elementCount))
		return std::numeric_limits<size_t>::max();

	unsigned bytes;
	if (SC_UNLIKELY(Sc::mul_overflow(size, count, &bytes)) ||
		SC_UNLIKELY(Sc::add_overflow(bytes, header, &bytes)))
		return std::numeric_limits<size_t>::max();

	if (SC_UNLIKELY(int(bytes) < 0))
		return std::numeric_limits<size_t>::max();
	return bytes;
}

size_t calculateGrowingBlockSize(size_t& elementCount, size_t elementSize, size_t headerSize) noexcept
{
	unsigned bytes = unsigned(calculateBlockSize(elementCount, elementSize, headerSize));
	if (int(bytes) < 0)
	{
		elementCount = std::numeric_limits<size_t>::max();
		return elementCount;
	}

	unsigned morebytes = Sc::nextPowerOfTwo(bytes);
	if (SC_UNLIKELY(int(morebytes) < 0))
		bytes += (morebytes - bytes) / 2;
	else
		bytes = morebytes;

	elementCount = (bytes - unsigned(headerSize)) / unsigned(elementSize);
	return bytes;
}

static inline size_t calculateBlockSize(size_t& capacity, size_t objectSize, size_t headerSize, scuint32 options)
{
	if (options & ScArrayData::Grow)
		return calculateGrowingBlockSize(capacity, objectSize, headerSize);
	return calculateBlockSize(capacity, objectSize, headerSize);
}

static inline ScArrayData* reallocateData(ScArrayData* header, size_t allocSize, scuint32 options)
{
	auto ret = ::realloc(header, allocSize);
	header = static_cast<ScArrayData*>(ret);
	if (header)
		header->capacityReserved = bool(options & ScArrayData::CapacityReserved);
	return header;
}

ScArrayData* ScArrayData::allocate(size_t objSize, size_t alignment, size_t capacity, AllocOptions options) noexcept
{
	SC_ASSERT((alignment >= SC_ALIGNOF(ScArrayData)) && !(alignment & (alignment - 1)));
	if (!(options & RawData) && !capacity)
	{
		if (options & Unsharable)
			return const_cast<ScArrayData*>(&sc_array_unsharable_empty);
		return const_cast<ScArrayData*>(&sc_array_empty);
	}

	size_t headerSize = sizeof(ScArrayData);
	if (!(options & RawData))
		headerSize += (alignment - SC_ALIGNOF(ScArrayData));

	if (headerSize > size_t(INT_MAX))
		return nullptr;

	size_t allocSize = calculateBlockSize(capacity, objSize, headerSize, options);
	ScArrayData* header = static_cast<ScArrayData*>(::malloc(allocSize));
	if (header)
	{
		scuintptr data = (scuintptr(header) + sizeof(ScArrayData) + alignment - 1) & ~(alignment - 1);
		header->ref.atomic.storeRelaxed(bool(!(options & Unsharable)));
		header->size = 0;
		header->alloc = capacity;
		header->capacityReserved = bool(options & CapacityReserved);
		header->offset = data - scuintptr(header);
	}
	return header;
}

SC_REQUIRED_RESULT ScArrayData* ScArrayData::reallocateUnaligned(ScArrayData* data, size_t objectSize, size_t capacity, AllocOptions options) noexcept
{
	SC_ASSERT(data);
	SC_ASSERT(data->isMutable());
	SC_ASSERT(!data->ref.isShared());

	size_t headerSize = sizeof(ScArrayData);
	size_t allocSize = calculateBlockSize(capacity, objectSize, headerSize, options);
	ScArrayData* header = static_cast<ScArrayData*>(reallocateData(data, allocSize, options));
	if (header)
		header->alloc = capacity;
	return header;
}

void ScArrayData::deallocate(ScArrayData* data, size_t objectSize, size_t alignment) noexcept
{
	SC_ASSERT(alignment >= SC_ALIGNOF(ScArrayData)
		&& !(alignment & (alignment - 1)));
	SC_UNUSED(objectSize) SC_UNUSED(alignment);

	if (data == &sc_array_unsharable_empty)
		return;

	SC_ASSERT_X(data == 0 || !data->ref.isStatic(), "QArrayData::deallocate", "Static data cannot be deleted");
	::free(data);
}
