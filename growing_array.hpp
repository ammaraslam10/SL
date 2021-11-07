#pragma once
#ifndef GROWING_ARRAY_H
#define GROWING_ARRAY_H
#include <iostream>
#define GROWING_MINIMUM 32
template <class T>
class growing_array
{
private:
	T *arr;
	unsigned int _size;
	//growing_array(growing_array& ref) {}
public:
	growing_array(const growing_array &ref) = default;
	growing_array(unsigned int initial_size = GROWING_MINIMUM)
	{
		_size = initial_size;
		arr = new T[_size]();
	}
	// Avoid size check
	inline T &direct_access(int index)
	{
		return arr[index];
	}
	// Access with size check
	inline T &operator[](unsigned int index)
	{
		while (index >= _size)
		{
			grow();
		}
		return arr[index];
	}
	inline void grow()
	{
		T *tmp = new T[_size * 2]();
		for (unsigned int i = 0; i < _size; i++)
		{
			tmp[i] = arr[i];
		}
		delete[] arr;
		arr = tmp;
		_size *= 2;
	}
	void shrink()
	{
		if (_size / 2 <= GROWING_MINIMUM)
			return;
		_size /= 2;
		T *tmp = new T[_size];
		for (unsigned int i = 0; i < _size; i++)
		{
			tmp[i] = arr[i];
		}
		delete[] arr;
		arr = tmp;
	}
	inline unsigned int size()
	{
		return _size;
	}
	~growing_array()
	{
		if (arr)
			delete[] arr;
		arr = 0;
		_size = 0;
	}
};
#undef GROWING_MINIMUM
#endif // !GROWING_ARRAY_H
