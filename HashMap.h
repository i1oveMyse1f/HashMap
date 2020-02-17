#pragma once

#include <unordered_map>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cstddef>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>> 
class HashMap {
public:
	class const_iterator;
	class iterator {
	private:
		int pos;
		HashMap* hmp;
	public:
		iterator(HashMap* hmp = nullptr, int pos = 0): pos(pos), hmp(hmp) {}
		iterator operator++(int) {
			iterator tmp = *this;
			++pos;
			return tmp;
		}
		iterator& operator++() {
			++pos;
			return *this;
		}
		iterator operator--(int) {
			iterator tmp = *this;
			--pos;
			return tmp;
		}
		iterator& operator--() {
			--pos;
			return *this;
		}
		bool operator==(const iterator other) const {
			return pos == other.pos;
		}
		bool operator!=(const iterator other) const {
			return pos != other.pos;
		}
		std::pair<const KeyType, ValueType>& operator*() {
			return reinterpret_cast<std::pair<const KeyType, ValueType>&>(hmp->get_pair(pos));
		}
		std::pair<const KeyType, ValueType>* operator->() {
			return reinterpret_cast<std::pair<const KeyType, ValueType>*>(&hmp->get_pair(pos));
		}
	};
	class const_iterator {
	private:
		int pos;
		const HashMap* hmp;
	public:
		const_iterator(const HashMap* hmp = nullptr, int pos = 0): pos(pos), hmp(hmp) {}
		const_iterator operator++(int) {
			const_iterator tmp = *this;
			++pos;
			return tmp;
		}
		const_iterator& operator++() {
			++pos;
			return *this;
		}
		const_iterator operator--(int) {
			const_iterator tmp = *this;
			--pos;
			return tmp;
		}
		const_iterator& operator--() {
			--pos;
			return *this;
		}
		bool operator==(const_iterator other) const {
			return hmp == other.hmp;
		}
		bool operator!=(const_iterator other) const {
			return hmp != other.hmp;
		}
		const std::pair<const KeyType, ValueType>& operator*() const {
			return reinterpret_cast<const std::pair<const KeyType, ValueType>&>(hmp->get_pair(pos));
		}
		const std::pair<const KeyType, ValueType>* operator->() const {
			return reinterpret_cast<const std::pair<const KeyType, ValueType>*>(&hmp->get_pair(pos));
		}
	};
	
	inline HashMap(Hash hasher = Hash()): table(2), hasher(hasher), _size(0), _capacity(2) {}
	template<class Iter>
	inline HashMap(Iter beg, Iter end, Hash hasher = Hash()): HashMap(hasher) {
		while (beg != end)
			insert(*beg++);
	}
	inline HashMap(const std::initializer_list<std::pair<KeyType, ValueType>> &list, Hash hasher = Hash()): 
		HashMap(list.begin(), list.end(), hasher) {}

	inline int size() const {
		return _size;
	}
	inline bool empty() const {
		return _size == 0;
	}
	Hash hash_function() const {
		return hasher;
	}
	void insert(const std::pair<KeyType, ValueType> &element) {
		if (!contains(element.first))
			add(element);
	}
	void erase(const KeyType &key) {
		if (contains(key))
			delite(key);
	}
	iterator begin() {
		return iterator(this, 0);
	}
	iterator end() {
		return iterator(this, elements.size());
	}
	const_iterator begin() const {
		return const_iterator(this, 0);
	}
	const_iterator end() const {
		return const_iterator(this, elements.size());
	}
	iterator find(const KeyType &key) {
		if (!contains(key))
			return end();
		return iterator(this, find_elem(key).second);
	}
	const_iterator find(const KeyType &key) const {
		if (!contains(key))
			return end();
		return const_iterator(this, find_elem(key).second);
	}
	ValueType& operator[] (const KeyType &key) {
		if (!contains(key))
			add({ key, ValueType() });
		return find_elem(key).first.second;
	}
	const ValueType& at(const KeyType &key) const {
		if (!contains(key))
			throw std::out_of_range("");
		int pos = get_pos(key);
		int i = 0;
		while (!(table[pos][i].first.first == key))
			++i;
		return table[pos][i].first.second;
	}
	void clear() {
		while (_size) {
			table[elements.back().first].clear();
			elements.pop_back();
			--_size;
		}
	}
private:
	std::vector<std::vector<std::pair<std::pair<KeyType, ValueType>, size_t>>> table;
	std::vector<std::pair<size_t, size_t>> elements;
	Hash hasher;
	int _size;
	int _capacity; // always is a power of two

	int get_pos(const KeyType &key) const {
		return hasher(key) & (_capacity - 1);
	}

	std::pair<std::pair<KeyType, ValueType>, size_t> &find_elem(const KeyType& key) {
		int pos = get_pos(key);
		int i = 0;
		while (!(table[pos][i].first.first == key))
			++i;
		return table[pos][i];
	}

	std::pair<std::pair<KeyType, ValueType>, size_t> find_elem(const KeyType& key) const {
		int pos = get_pos(key);
		int i = 0;
		while (!(table[pos][i].first.first == key))
			++i;
		return table[pos][i];
	}

	void add(const std::pair<KeyType, ValueType> &el) {
		int pos = get_pos(el.first);
		table[pos].push_back({ el, elements.size() });
		elements.push_back({ pos, table[pos].size() - 1 });
		++_size;
		rehash();
	}

	bool contains(const KeyType& key) const {
		int pos = get_pos(key);
		for (auto& i : table[pos])
			if (i.first.first == key)
				return true;
		return false;
	}

	void delite(const KeyType &key) {
		int pos = get_pos(key);
		int i = 0;
		while (!(table[pos][i].first.first == key))
			++i;
		int ind_el = table[pos][i].second;
		std::swap(elements.back(), elements[ind_el]);
		table[elements[ind_el].first][elements[ind_el].second].second = ind_el;
		table[pos][i].second = elements.size() - 1;
		std::swap(table[pos][i], table[pos].back());
		elements[table[pos][i].second].second = i;
		table[pos].pop_back();
		elements.pop_back();
		--_size;
	}

	void rehash() {
		if ((_size << 1) < _capacity)
			return;
		std::vector<std::pair<KeyType, ValueType>> tmp(_size);
		for (const auto &i : elements)
			tmp[--_size] = table[i.first][i.second].first;
		for (const auto &i : elements)
			if (table[i.first].size())
				table[i.first].clear();
		elements.clear();
		_capacity <<= 1;
		table.resize(_capacity);
		for (auto& i : tmp)
			add(i);
	}

	const std::pair<KeyType, ValueType>& get_pair(int x) const {
		return table[elements[x].first][elements[x].second].first;
	}
	std::pair<KeyType, ValueType>& get_pair(int x) {
		return table[elements[x].first][elements[x].second].first;
	}
};
