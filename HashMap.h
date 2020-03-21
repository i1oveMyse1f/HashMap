#pragma once

#include <unordered_map>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cstddef>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
public:
	static constexpr const size_t MIN_CAPACITY = 2;
	typedef std::pair<KeyType, ValueType> PKeyValue;
	typedef std::pair<const KeyType, ValueType> PConstKeyValue;

	class const_iterator;
	class iterator {
	private:
		size_t pos;
		HashMap* hmp;
	public:
		iterator(HashMap* hmp = nullptr, size_t pos = 0) : pos(pos), hmp(hmp) {}
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
		PConstKeyValue& operator*() {
			return reinterpret_cast<PConstKeyValue&>(hmp->get_pair(pos));
		}
		PConstKeyValue* operator->() {
			return reinterpret_cast<PConstKeyValue*>(&hmp->get_pair(pos));
		}
	};
	class const_iterator {
	private:
		size_t pos;
		const HashMap* hmp;
	public:
		const_iterator(const HashMap* hmp = nullptr, size_t pos = 0) : pos(pos), hmp(hmp) {}
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
		const PConstKeyValue& operator*() const {
			return reinterpret_cast<const PConstKeyValue&>(hmp->get_pair(pos));
		}
		const PConstKeyValue* operator->() const {
			return reinterpret_cast<const PConstKeyValue*>(&hmp->get_pair(pos));
		}
	};

	inline HashMap(Hash hasher = Hash()) : table(MIN_CAPACITY), hasher(hasher), _size(0), _capacity(MIN_CAPACITY) {}
	template<class Iter>
	inline HashMap(Iter beg, Iter end, Hash hasher = Hash()) : HashMap(hasher) {
		while (beg != end)
			insert(*beg++);
	}
	inline HashMap(const std::initializer_list<PKeyValue>& list, Hash hasher = Hash()) :
		HashMap(list.begin(), list.end(), hasher) {}

	inline size_t size() const {
		return _size;
	}
	inline bool empty() const {
		return _size == 0;
	}
	inline Hash hash_function() const {
		return hasher;
	}
	inline void insert(const PKeyValue& element) {
		if (!contains(element.first))
			add(element);
	}
	inline void erase(const KeyType& key) {
		if (contains(key))
			delite(key);
	}
	inline iterator begin() {
		return iterator(this, 0);
	}
	inline iterator end() {
		return iterator(this, elements.size());
	}
	inline const_iterator begin() const {
		return const_iterator(this, 0);
	}
	inline const_iterator end() const {
		return const_iterator(this, elements.size());
	}
	inline iterator find(const KeyType& key) {
		if (!contains(key))
			return end();
		return iterator(this, find_elem(key).second);
	}
	inline const_iterator find(const KeyType& key) const {
		if (!contains(key))
			return end();
		return const_iterator(this, find_elem(key).second);
	}
	inline ValueType& operator[] (const KeyType& key) {
		if (!contains(key))
			add({ key, ValueType() });
		return find_elem(key).first.second;
	}
	inline const ValueType& at(const KeyType& key) const {
		if (!contains(key))
			throw std::out_of_range("");
		size_t pos = get_pos(key);
		size_t i = 0;
		while (!(table[pos][i].first.first == key))
			++i;
		return table[pos][i].first.second;
	}
	inline void clear() {
		while (_size) {
			table[elements.back().first].clear();
			elements.pop_back();
			--_size;
		}
	}
private:
	std::vector<std::vector<std::pair<PKeyValue, size_t>>> table;
	std::vector<std::pair<size_t, size_t>> elements;
	Hash hasher;
	size_t _size;
	size_t _capacity; // always is a power of two

	size_t get_pos(const KeyType& key) const {
		return hasher(key) & (_capacity - 1);
	}

	std::pair<PKeyValue, size_t>& find_elem(const KeyType& key) {
		size_t pos = get_pos(key);
		size_t i = 0;
		while (!(table[pos][i].first.first == key))
			++i;
		return table[pos][i];
	}

	std::pair<PKeyValue, size_t> find_elem(const KeyType& key) const {
		size_t pos = get_pos(key);
		size_t i = 0;
		while (!(table[pos][i].first.first == key))
			++i;
		return table[pos][i];
	}

	void add(const PKeyValue& el) {
		size_t pos = get_pos(el.first);
		table[pos].push_back({ el, elements.size() });
		elements.push_back({ pos, table[pos].size() - 1 });
		++_size;
		rehash();
	}

	bool contains(const KeyType& key) const {
		size_t pos = get_pos(key);
		for (auto& i : table[pos])
			if (i.first.first == key)
				return true;
		return false;
	}

	void delite(const KeyType& key) {
		size_t pos = get_pos(key);
		size_t i = 0;
		while (!(table[pos][i].first.first == key))
			++i;
		size_t ind_el = table[pos][i].second;
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
		std::vector<PKeyValue> elements_container(_size);
		for (const auto& i : elements)
			elements_container[--_size] = table[i.first][i.second].first;
		for (const auto& i : elements)
			if (table[i.first].size())
				table[i.first].clear();
		elements.clear();
		_capacity <<= 1;
		table.resize(_capacity);
		for (auto& i : elements_container)
			add(i);
	}

	const PKeyValue& get_pair(size_t x) const {
		return table[elements[x].first][elements[x].second].first;
	}
	PKeyValue& get_pair(size_t x) {
		return table[elements[x].first][elements[x].second].first;
	}
};
