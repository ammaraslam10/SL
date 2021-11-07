#ifndef HEAP_MANAGER
#define HEAP_MANAGER
#define HEAP_MANAGER_MINIMUM 8
#include "growing_array.hpp"

#ifndef EMPTYOBJ
#define EMPTYOBJ
struct Empty
{
	operator int() { return 0; }
};
#endif

template <class T>
class heap_manager
{
	growing_array<T *> arr;
	growing_array<bool> available;
	int count;
	int values;

public:
	heap_manager(int values, int size) : arr(size > HEAP_MANAGER_MINIMUM ? size : HEAP_MANAGER_MINIMUM),
										 available(size > HEAP_MANAGER_MINIMUM ? size : HEAP_MANAGER_MINIMUM)
	{

		count = 0;
		this->values = values;
		for (unsigned int i = 0; i < arr.size(); i++)
		{
			arr.direct_access(i) = new T[values];
			available.direct_access(i) = true;
		}
	}
	T *get()
	{
		for (int i = 0; i < count; i++)
		{
			if (available.direct_access(i))
			{
				available.direct_access(i) = false;
				return arr.direct_access(i);
			}
		}
		if (count == arr.size())
		{
			arr.grow();
			available.grow();
			for (unsigned int i = count; i < arr.size(); i++)
			{
				arr.direct_access(i) = new T[values];
				available.direct_access(i) = true;
			}
		}
		available.direct_access(count) = false;
		return arr.direct_access(count++);
	}
	void ret(T *val)
	{
		if (arr.direct_access(count - 1) == val)
		{
			available.direct_access(count - 1) = true;
			return;
		}
		for (int i = 0; i < count; i++)
		{
			if (arr.direct_access(i) == val)
			{
				available.direct_access(i) = true;
				return;
			}
		}
	}
	~heap_manager()
	{
		for (unsigned int i = 0; i < arr.size(); i++)
		{
			delete[] arr.direct_access(i);
		}
		count = values = 0;
	}
};
#endif