#pragma once
#ifndef HASH_TABLE_H
#define HASH_TABLE_H
#include <iostream>
#include <string>
#define HASH_TABLE_MINIMUM 8

#ifndef EMPTYOBJ
#define EMPTYOBJ
struct Empty
{
	operator int() { return 0; }
};
#endif

#ifndef VSTRING
#define VSTRING
struct vstring
{
	char *string_data;
	unsigned int length;
	vstring(const char *data, int len) { copy(data, len); }
	inline void copy(const char *data, int len)
	{
		string_data = DBG_NEW char[len + 1];
#ifdef LINUX
		strcpy(string_data, data);
#endif
#ifndef LINUX
		strcpy_s(string_data, len + 1, data);
#endif
		length = len;
	}
	~vstring()
	{
		if (string_data != nullptr)
		{
			delete[] string_data;
			string_data = nullptr;
		}
	}
};
#endif
// From Zend Engine
inline unsigned long hash_f(const char *key, unsigned long len)
{
	register unsigned long hash = 5381;
	for (; len >= 8; len -= 8)
	{
		hash = ((hash << 5) + hash) + *key++;
		hash = ((hash << 5) + hash) + *key++;
		hash = ((hash << 5) + hash) + *key++;
		hash = ((hash << 5) + hash) + *key++;
		hash = ((hash << 5) + hash) + *key++;
		hash = ((hash << 5) + hash) + *key++;
		hash = ((hash << 5) + hash) + *key++;
		hash = ((hash << 5) + hash) + *key++;
	}
	switch (len)
	{
	case 7:
		hash = ((hash << 5) + hash) + *key++;
	case 6:
		hash = ((hash << 5) + hash) + *key++;
	case 5:
		hash = ((hash << 5) + hash) + *key++;
	case 4:
		hash = ((hash << 5) + hash) + *key++;
	case 3:
		hash = ((hash << 5) + hash) + *key++;
	case 2:
		hash = ((hash << 5) + hash) + *key++;
	case 1:
		hash = ((hash << 5) + hash) + *key++;
		break;
	case 0:
		break;
	}
	return hash;
}
template <class T>
struct hash_bucket
{
	hash_bucket<T> *_next;
	union
	{
		unsigned long hash;
		vstring *key;
	} hval;
	T data;
	inline hash_bucket<T> *next() { return (hash_bucket<T> *)((unsigned long long)_next & ~1ULL); }
	inline void next(hash_bucket<T> *addr) { _next = (hash_bucket<T> *)((unsigned long long)addr | type()); }
	inline bool type() { return ((unsigned long long)_next & 1ULL); }
	inline hash_bucket() : _next(0), data() { hval.key = 0; }
	inline hash_bucket(unsigned long h, vstring *k, hash_bucket<T> *n, T d) : data(d)
	{
		_next = n;
		if (k)
		{
			hval.key = k;
			_next = (hash_bucket<T> *)((unsigned long long)_next | 1ULL);
		}
		else
		{
			hval.hash = h;
			_next = (hash_bucket<T> *)((unsigned long long)_next & ~1ULL);
		}
	}
};
template <class T>
class hash_table
{
	hash_bucket<T> *values;
	unsigned long int *hashes;
	unsigned int table_size;
	unsigned int count;

public:
	hash_table(unsigned int initial_size = HASH_TABLE_MINIMUM - 1)
	{
		table_size = size_calculation(initial_size + 1);
		count = 0;
		values = new hash_bucket<T>[table_size];
		hashes = new unsigned long int[table_size]();
		table_size--;
	}
	inline unsigned int length() { return count; }
	void clear()
	{
		for (unsigned int i = 1; i <= count; i++)
		{
			if (!values[i].data == false)
			{
				if (values[i].type())
					delete values[i].hval.key;
				values[i].data = Empty();
			}
		}
		count = 0;
	}
	~hash_table()
	{
		for (unsigned int i = 1; i <= count; i++)
		{
			if (!values[i].data == false)
			{
				if (values[i].type())
					delete values[i].hval.key;
				values[i].data = Empty();
			}
		}
		if (values && hashes)
		{
			delete[] values;
			delete[] hashes;
		}
		hashes = 0;
		values = 0;
		count = table_size = 0;
	}
	inline T *get(const char *key, unsigned long keylen)
	{
		unsigned int i = hashes[hash_f(key, keylen) & table_size];
		hash_bucket<T> *bkt = &values[i];
		while (bkt && !(!bkt->data == false && bkt->type() && strcmp(bkt->hval.key->string_data, key) == 0))
		{
			bkt = bkt->next();
		}
		if (bkt)
			return &(bkt->data);
		else
			return 0;
	}
	// Get OR Add at key
	inline T *get_add(const char *key, unsigned long keylen)
	{
		T *tmp = get(key, keylen);
		if (tmp)
			return tmp;
		T val = 0;
		return add(key, keylen, val);
	}
	inline T *get(unsigned long key)
	{
		unsigned int i = hashes[key & table_size];
		hash_bucket<T> *bkt = &values[i];
		while (bkt && !(!bkt->data == false && !bkt->type() && bkt->hval.hash == key))
		{
			bkt = bkt->next();
		}
		if (bkt)
			return &(bkt->data);
		else
			return 0;
	}
	// Get OR Add at key
	inline T *get_add(unsigned long key)
	{
		T *tmp = get(key);
		if (tmp)
			return tmp;
		T val = 0;
		return add(key, val);
	}
	inline T *access(unsigned int n)
	{
		return &(values[n + 1].data);
	}
	inline T *add(const char *key, unsigned long keylen, const T &value)
	{
		if (count >= table_size)
		{
			remake((table_size + 1) * 2);
		}
		count++;
		unsigned int loc = hash_f(key, keylen) & table_size;
		if (!values[hashes[loc]].data)
		{
			hashes[loc] = count;
		}
		else
		{
			hash_bucket<T> *bkt = &values[hashes[loc]];
			while (bkt->next() != 0)
				bkt = bkt->next();
			bkt->next(&values[count]);
		}
		values[count] = hash_bucket<T>(0, new vstring(key, keylen), 0, value);
		return &(values[count].data);
	}
	inline T *add(unsigned long key, const T &value)
	{
		if (count >= table_size)
		{
			remake((table_size + 1) * 2);
		}
		count++;
		unsigned int loc = key & table_size;
		if (!values[hashes[loc]].data)
		{
			hashes[loc] = count;
		}
		else
		{
			hash_bucket<T> *bkt = &values[hashes[loc]];
			while (bkt->next() != 0)
				bkt = bkt->next();
			bkt->next(&values[count]);
		}
		values[count] = hash_bucket<T>(key, 0, 0, value);
		return &(values[count].data);
	}
	inline void add(const T &value)
	{
		add(count, value);
	}
	inline void remove(const char *key, unsigned long keylen)
	{
		if (count + HASH_TABLE_MINIMUM + table_size / 128 <= table_size / 2)
		{
			remake((table_size + 1) / 2);
		}
		unsigned int i = hashes[hash_f(key, keylen) & table_size];
		hash_bucket<T> *bkt = &values[i];
		while (bkt && !(!bkt->data == false && bkt->type() && strcmp(bkt->hval.key->string_data, key) == 0))
		{
			bkt = bkt->next();
		}
		if (bkt && !bkt->data == false)
		{
			bkt->data = Empty();
			count--;
			delete (bkt->hval.key);
			bkt->hval.key = 0;
		}
	}
	inline void remove(unsigned long key)
	{
		if (count + HASH_TABLE_MINIMUM + table_size / 128 <= table_size / 2)
		{
			remake((table_size + 1) / 2);
		}
		unsigned int i = hashes[key & table_size];
		hash_bucket<T> *bkt = &values[i];
		while (bkt && !(!bkt->data == false && !bkt->type() && bkt->hval.hash == key))
		{
			bkt = bkt->next();
		}
		if (bkt && !bkt->data == false)
		{
			bkt->data = Empty();
			count--;
			bkt->hval.hash = 0;
		}
	}
	inline void remake(unsigned int new_s)
	{
		hash_bucket<T> *tmp = new hash_bucket<T>[new_s];
		unsigned long int *tmp_h = new unsigned long int[new_s]();
		unsigned int ind = 1, andv = new_s - 1;
		for (unsigned int i = 1; i <= table_size; i++)
		{
			if (!values[i].data == false)
			{
				unsigned int loc = 0;
				if (values[i].type())
				{
					loc = hash_f(values[i].hval.key->string_data, values[i].hval.key->length) & andv;
					tmp[ind] = hash_bucket<T>(0, values[i].hval.key, 0, std::move(values[i].data));
				}
				else
				{
					loc = values[i].hval.hash & andv;
					tmp[ind] = hash_bucket<T>(values[i].hval.hash, 0, 0, std::move(values[i].data));
				}
				values[i].data = 0;
				if (!tmp[tmp_h[loc]].data)
					tmp_h[loc] = ind;
				else
				{
					hash_bucket<T> *bkt = &tmp[tmp_h[loc]];
					while (bkt->next() != 0)
					{
						bkt = bkt->next();
					}
					bkt->next(&tmp[ind]);
				}
				ind++;
			}
		}
		count = ind - 1;
		table_size = andv;
		delete[] hashes;
		delete[] values;
		hashes = tmp_h;
		values = tmp;
	}
	hash_table(hash_table &&ref)
	{
		table_size = ref.table_size;
		count = ref.count;
		values = ref.values;
		hashes = ref.hashes;
		ref.values = 0;
		ref.hashes = 0;
		ref.count = 0;
		ref.table_size = 0;
	}
	hash_table(hash_table &ref)
	{
		table_size = 0;
		copy(&ref);
	}
	void copy(hash_table *ref)
	{
		if (table_size > ref->table_size)
		{
			count = ref->count;
			hashes[0] = ref->hashes[0];
			for (unsigned int i = 1; i <= count; i++)
			{
				if (ref->values[i].type())
					values[i].hval.key = new vstring(values[i].hval.key->string_data, values[i].hval.key->length);
				else
					values[i].hval.hash = ref->values[i].hval.hash;
				values[i].data = ref->values[i].data;
				hashes[i] = ref->hashes[i];
			}
			return;
		}
		this->~hash_table();
		table_size = ref->table_size;
		count = ref->count;
		values = new hash_bucket<T>[table_size + 1];
		hashes = new unsigned long int[table_size + 1]();
		hashes[0] = ref->hashes[0];
		for (unsigned int i = 1; i <= count; i++)
		{
			if (ref->values[i].type())
				values[i].hval.key = new vstring(values[i].hval.key->string_data, values[i].hval.key->length);
			else
				values[i].hval.hash = ref->values[i].hval.hash;
			values[i].data = ref->values[i].data;
			hashes[i] = ref->hashes[i];
		}
	}
	unsigned int size_calculation(unsigned int n)
	{
		unsigned count = 0;
		if (n && !(n & (n - 1)))
			return n;
		while (n != 0)
		{
			n >>= 1;
			count += 1;
		}
		return 1 << count;
	}
};
#endif