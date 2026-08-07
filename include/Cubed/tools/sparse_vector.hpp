#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <utility>
#include <vector>
namespace Cubed {
template <typename T> class SparseVector {
private:
    std::vector<std::optional<T>> m_data;
    std::vector<uint32_t> m_free_list;
    std::vector<uint32_t> m_generation;

    std::vector<uint32_t> m_dense;
    std::vector<uint32_t> m_dense_index;

public:
    struct Handle {
        uint32_t index;
        uint32_t generation;

        uint64_t value() const noexcept {
            return (uint64_t(index) << 32) | generation;
        }

        bool operator==(const Handle&) const = default;

        struct Hash {
            size_t operator()(const Handle& h) const noexcept {
                return h.value();
            }
        };
    };

    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;

    using size_type = size_t;

    class iterator { // NOLINT
    private:
        SparseVector* m_owner;
        size_t m_index;

    public:
        using iterator_category = std::random_access_iterator_tag;
        using iterator_concept = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator(SparseVector* owner, size_t index)
            : m_owner(owner), m_index(index) {}

        reference operator*() {
            return m_owner->m_data[m_owner->m_dense[m_index]].value();
        }

        pointer operator->() { return &(**this); }

        iterator& operator++() {
            ++m_index;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        iterator& operator--() {
            --m_index;
            return *this;
        }

        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const {
            return m_owner == other.m_owner && m_index == other.m_index;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

        iterator operator+(difference_type n) const {
            return iterator(m_owner, m_index + n);
        }

        iterator operator-(difference_type n) const {
            return iterator(m_owner, m_index - n);
        }

        difference_type operator-(const iterator& other) const {
            return static_cast<difference_type>(m_index) -
                   static_cast<difference_type>(other.m_index);
        }

        iterator& operator+=(difference_type n) {
            m_index += n;
            return *this;
        }

        iterator& operator-=(difference_type n) {
            m_index -= n;
            return *this;
        }

        reference operator[](difference_type n) const { return *(*this + n); }

        bool operator<(const iterator& other) const {
            return m_index < other.m_index;
        }

        bool operator>(const iterator& other) const {
            return m_index > other.m_index;
        }

        bool operator<=(const iterator& other) const {
            return m_index <= other.m_index;
        }

        bool operator>=(const iterator& other) const {
            return m_index >= other.m_index;
        }
        friend iterator operator+(difference_type n, const iterator& it) {
            return it + n;
        }
    };

    class const_iterator { // NOLINT
    private:
        const SparseVector* m_owner;
        size_t m_index;

    public:
        using iterator_category = std::random_access_iterator_tag;
        using iterator_concept = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        const_iterator(const SparseVector* owner, size_t index)
            : m_owner(owner), m_index(index) {}

        reference operator*() const {
            return m_owner->m_data[m_owner->m_dense[m_index]].value();
        }

        pointer operator->() const { return &(**this); }

        const_iterator& operator++() {
            ++m_index;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        const_iterator& operator--() {
            --m_index;
            return *this;
        }

        const_iterator operator--(int) {
            const_iterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const const_iterator& other) const {
            return m_owner == other.m_owner && m_index == other.m_index;
        }

        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }

        const_iterator operator+(difference_type n) const {
            return const_iterator(m_owner, m_index + n);
        }

        const_iterator operator-(difference_type n) const {
            return const_iterator(m_owner, m_index - n);
        }

        difference_type operator-(const const_iterator& other) const {
            return static_cast<difference_type>(m_index) -
                   static_cast<difference_type>(other.m_index);
        }

        const_iterator& operator+=(difference_type n) {
            m_index += n;
            return *this;
        }

        const_iterator& operator-=(difference_type n) {
            m_index -= n;
            return *this;
        }

        reference operator[](difference_type n) const { return *(*this + n); }

        bool operator<(const const_iterator& other) const {
            return m_index < other.m_index;
        }

        bool operator>(const const_iterator& other) const {
            return m_index > other.m_index;
        }

        bool operator<=(const const_iterator& other) const {
            return m_index <= other.m_index;
        }

        bool operator>=(const const_iterator& other) const {
            return m_index >= other.m_index;
        }
        friend const_iterator operator+(difference_type n,
                                        const const_iterator& it) {
            return it + n;
        }
    };

    template <class U> [[nodiscard]] Handle insert(U&& value) {
        uint32_t id;
        if (!m_free_list.empty()) {
            id = m_free_list.back();
            m_free_list.pop_back();
            m_data[id].emplace(std::forward<U>(value));

            m_dense_index[id] = m_dense.size();
        } else {
            id = m_data.size();
            m_data.push_back(std::forward<U>(value));
            m_generation.push_back(1);
            m_dense_index.emplace_back(m_dense.size());
        }

        m_dense.push_back(id);
        return {id, m_generation[id]};
    }

    template <typename... Args> Handle emplace(Args&&... args) {
        return insert(T(std::forward<Args>(args)...));
    }

    void erase(Handle h) {

        if (!exists(h)) {
            return;
        }
        uint32_t id = h.index;
        m_free_list.push_back(id);
        uint32_t index = m_dense_index[id];
        uint32_t last_id = m_dense.back();
        m_dense[index] = last_id;
        m_dense_index[last_id] = index;

        m_dense.pop_back();
        m_data[id].reset();
        ++m_generation[id];
    }

    T& operator[](Handle h) {
        assert(exists(h));
        return m_data[h.index].value();
    }

    const T& operator[](Handle h) const {
        assert(exists(h));
        return m_data[h.index].value();
    }

    bool exists(Handle h) const {
        return h.index < m_generation.size() &&
               m_generation[h.index] == h.generation;
    }

    void reserve(size_t n) {
        m_data.reserve(n);
        m_generation.reserve(n);
        m_dense.reserve(n);
        m_dense_index.reserve(n);
    }

    size_t size() const { return m_dense.size(); }

    bool empty() const { return m_dense.empty(); }

    iterator begin() { return iterator(this, 0); }

    iterator end() { return iterator(this, m_dense.size()); }

    const_iterator begin() const { return const_iterator(this, 0); }

    const_iterator end() const { return const_iterator(this, m_dense.size()); }

    const_iterator cbegin() const { return begin(); }

    const_iterator cend() const { return end(); }
};

} // namespace Cubed