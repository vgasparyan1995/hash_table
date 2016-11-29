#pragma once

#include <functional>
#include <list>
#include <memory>
#include <vector>

namespace std {

template <typename Key,
          typename Value,
          typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>,
          typename Allocator = std::allocator< std::pair<Key, Value> > >
class hash_table {
public:
    typedef Key key_type;
    typedef Value mapped_type;
    typedef std::pair<Key, Value> value_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef Hash hasher;
    typedef KeyEqual key_equal;
    typedef Allocator allocator;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef typename std::allocator_traits<Allocator>::pointer pointer;
    typedef typename std::allocator_traits<Allocator>::const_pointer const_pointer;
private:
    typedef std::list<value_type, Allocator> Bucket;
    typedef std::vector< Bucket > Buckets;

private:
    static constexpr float DEFAULT_MAX_LOAD_FACTOR = 3.0;

private:
    template <typename PointerType,
              typename ReferenceType>
    struct iterator_helper
    {
        friend hash_table;
    public:
        typedef hash_table::difference_type difference_type;
        typedef hash_table::value_type value_type;
        typedef PointerType pointer;
        typedef ReferenceType& reference;
        typedef std::forward_iterator_tag iterator_category;

    public:
        iterator_helper() = default;
        iterator_helper(const iterator_helper& that) = default;
        iterator_helper& operator= (const iterator_helper& that) = default;
        ~iterator_helper() = default;

        template <typename FwdIter>
        iterator_helper(FwdIter that)
            : m_bucket_iterator(that.m_bucket_iterator)
            , m_buckets_end(that.m_buckets_end)
            , m_pair_iterator(that.m_pair_iterator)
        {}

        reference operator* ()
        {
            return *m_pair_iterator;
        }

        const reference operator* () const
        {
            return *m_pair_iterator;
        }

        pointer operator-> ()
        {
            return &(*m_pair_iterator);
        }

        const pointer operator-> () const
        {
            return &(*m_pair_iterator);
        }

        iterator_helper& operator++ ()
        {
            ++m_pair_iterator;
            if (m_pair_iterator == m_bucket_iterator->end()) {
                do {
                    ++m_bucket_iterator;
                } while (m_bucket_iterator != m_buckets_end && m_bucket_iterator->empty());
                if (m_bucket_iterator != m_buckets_end) {
                    m_pair_iterator = m_bucket_iterator->begin();
                }
            }
            return *this;
        }

        iterator_helper operator++ (int)
        {
            iterator_helper tmp(*this);
            ++(*this);
            return tmp;
        }

        bool operator== (const iterator_helper& that)
        {
            if ((m_bucket_iterator == m_buckets_end ||
                 m_pair_iterator == m_bucket_iterator->end()) &&
                (that.m_bucket_iterator == that.m_buckets_end ||
                 that.m_pair_iterator == that.m_bucket_iterator->end())) {
                return true;
            }
            if (m_pair_iterator == that.m_pair_iterator) {
                return true;
            }
            return false;
        }

        bool operator!= (const iterator_helper& that)
        {
            return !(*this == that);
        }

    private:
        typename Buckets::iterator m_bucket_iterator;
        typename Buckets::iterator m_buckets_end;
        typename Bucket::iterator m_pair_iterator;
    };

public:
    typedef iterator_helper<pointer, reference> iterator;
    typedef iterator_helper<const_pointer, const_reference> const_iterator;

public:
    /// Constructors
    explicit hash_table(const size_type buckets = 1,
                        const Hash hash = Hash(),
                        const KeyEqual key_eq = KeyEqual(),
                        const Allocator alloc = Allocator())
        : m_buckets(buckets)
        , m_size(0)
        , m_max_load_factor(DEFAULT_MAX_LOAD_FACTOR)
        , m_hasher(hash)
        , m_key_equal(key_eq)
    {}

    template <typename Iter>
    hash_table(Iter first,
               Iter last,
               const size_type buckets = 1,
               const Hash hash = Hash(),
               const KeyEqual key_eq = KeyEqual(),
               const Allocator alloc = Allocator())
        : m_buckets(buckets)
        , m_size(0)
        , m_max_load_factor(DEFAULT_MAX_LOAD_FACTOR)
        , m_hasher(hash)
        , m_key_equal(key_eq)
    {
        for (auto it = first; it != last; ++it) {
            insert(*it);
        }
    }

    hash_table(const hash_table& that) = default;
    
    hash_table(hash_table&& that)
        : m_buckets(std::move(that.m_buckets))
        , m_size(that.m_size)
        , m_max_load_factor(that.m_max_load_factor)
        , m_hasher(that.m_hasher)
        , m_key_equal(that.m_key_equal)
    {
        that.m_size = 0;
    }

    hash_table(const std::initializer_list<value_type>& il,
               const size_type buckets = 1,
               const Hash hash = Hash(),
               const KeyEqual key_eq = KeyEqual(),
               const Allocator alloc = Allocator())
        : m_buckets(buckets)
        , m_size(0)
        , m_max_load_factor(DEFAULT_MAX_LOAD_FACTOR)
        , m_hasher(hash)
        , m_key_equal(key_eq)
    {
        rehash(il.size() / m_max_load_factor);
        for (const auto& val : il) {
            insert(val);
        }
    }

    /// Destructor
    ~hash_table() = default;

    /// Assignment operator
    hash_table& operator= (const hash_table& that)
    {
        if (this != &that) {
            m_buckets = that.m_buckets;
            m_size = that.m_size;
            m_max_load_factor = that.m_max_load_factor;
            m_hasher = that.m_hasher;
            m_key_equal = that.m_key_equal;
        }
        return *this;
    }

    hash_table& operator= (hash_table&& that)
    {
        if (this != &that) {
            m_buckets.assign(that.m_buckets.begin(), that.m_buckets.end());
            m_size = that.m_size;
            that.m_size = 0;
            m_max_load_factor = that.m_max_load_factor;
            m_hasher = that.m_hasher;
            m_key_equal = that.m_key_equal;
        }
        return *this;
    }

    hash_table& operator= (const std::initializer_list<value_type>& il)
    {
        for (const auto& val : il) {
            insert(val);
        }
    }

    /// Mutators
    iterator insert(const key_type& key, const mapped_type& value);
    iterator insert(const value_type& value);
    void erase(const key_type& key);
    iterator erase(const iterator position);
    iterator find(const key_type& key);
    mapped_type& operator[] (const key_type& key);
    void clear();

    /// Selectors
    const_iterator find(const key_type& key) const;
    size_type size() const;
    bool empty() const;
    size_type buckets() const;

    /// Iterators
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    /// Hasher
    void rehash(const size_type buckets);
    hasher& get_hasher();
    const hasher& get_hasher() const;
    const float get_max_load_factor() const;
    void set_max_load_factor(const float max_load_factor);

private:
    bool rehash_needed() const;

private:
    Buckets m_buckets;
    size_type m_size;
    float m_max_load_factor;
    hasher m_hasher;
    key_equal m_key_equal;
};

#define TEMPLATE_DECL template <typename Key, typename Value, typename Hash, typename KeyEqual, typename Allocator>
#define CLASS_NAME hash_table<Key, Value, Hash, KeyEqual, Allocator>

TEMPLATE_DECL
typename CLASS_NAME::iterator CLASS_NAME::insert(const key_type& key, const mapped_type& value)
{
    return insert(std::make_pair(key, value));
}

TEMPLATE_DECL
typename CLASS_NAME::iterator CLASS_NAME::insert(const value_type& value)
{
    auto existing_value = find(value.first);
    if (existing_value != end()) {
        existing_value->second = value.second;
    } else {
        ++m_size;
        if (rehash_needed()) {
            rehash(m_buckets.size() + 1);
        }
        existing_value.m_bucket_iterator = m_buckets.begin() + m_hasher(value.first) % m_buckets.size();
        existing_value.m_buckets_end = m_buckets.end();
        existing_value.m_pair_iterator = existing_value.m_bucket_iterator->insert(
                existing_value.m_bucket_iterator->begin(), value);
    }
    return existing_value;
}

TEMPLATE_DECL
void CLASS_NAME::erase(const key_type& key)
{
    auto position = find(key);
    if (position != end()) {
        erase(position);
    }
}

TEMPLATE_DECL
typename CLASS_NAME::iterator CLASS_NAME::erase(const iterator position)
{
    auto tmp = position;
    ++tmp;
    position.m_bucket_iterator->erase(position.m_pair_iterator);
    return tmp;
}

TEMPLATE_DECL
typename CLASS_NAME::iterator CLASS_NAME::find(const key_type& key)
{
    iterator result;
    result.m_bucket_iterator = m_buckets.begin() + m_hasher(key) % m_buckets.size();
    result.m_buckets_end = m_buckets.end();
    for (auto pair_iter = result.m_bucket_iterator->begin();
            pair_iter != result.m_bucket_iterator->end();
            ++pair_iter) {
        if (m_key_equal(key, pair_iter->first)) {
            result.m_pair_iterator = pair_iter;
            return result;
        }
    }
    result.m_pair_iterator = result.m_bucket_iterator->end();
    return result;
}

TEMPLATE_DECL
typename CLASS_NAME::mapped_type& CLASS_NAME::operator[] (const key_type& key)
{
    auto position = find(key);
    if (position != end()) {
        return position.m_pair_iterator->second;
    }
    position = insert(std::make_pair(key, mapped_type()));
    return position.m_pair_iterator->second;
}

TEMPLATE_DECL
void CLASS_NAME::clear()
{
    m_buckets.resize(1);
    m_buckets.front().clear();
    m_size = 0;
}

TEMPLATE_DECL
typename CLASS_NAME::const_iterator CLASS_NAME::find(const key_type& key) const
{
    const_iterator result;
    result.m_bucket_iterator = m_buckets.begin() + m_hasher(key) % m_buckets.size();
    result.m_buckets_end = m_buckets.end();
    for (auto pair_iter = result.m_bucket_iterator->begin();
            pair_iter != result.m_bucket_iterator->end();
            ++pair_iter) {
        if (m_key_equal(key, pair_iter->first)) {
            result.m_pair_iterator = pair_iter;
            return result;
        }
    }
    result.m_pair_iterator = result.m_bucket_iterator->end();
    return result;
}

TEMPLATE_DECL
typename CLASS_NAME::size_type CLASS_NAME::size() const
{
    return m_size;
}

TEMPLATE_DECL
bool CLASS_NAME::empty() const
{
    return m_size == 0;
}

TEMPLATE_DECL
typename CLASS_NAME::size_type CLASS_NAME::buckets() const
{
    return m_buckets.size();
}

TEMPLATE_DECL
typename CLASS_NAME::iterator CLASS_NAME::begin()
{
    iterator result;
    result.m_bucket_iterator = m_buckets.begin();
    result.m_buckets_end = m_buckets.end();
    while (result.m_bucket_iterator != result.m_buckets_end &&
            result.m_bucket_iterator->empty()) {
        ++result.m_bucket_iterator;
    }
    if (result.m_bucket_iterator != result.m_buckets_end) {
        result.m_pair_iterator = result.m_bucket_iterator->begin();
    }
    return result;
}

TEMPLATE_DECL
typename CLASS_NAME::const_iterator CLASS_NAME::begin() const
{
    const_iterator result;
    result.m_bucket_iterator = m_buckets.begin();
    result.m_buckets_end = m_buckets.end();
    while (result.m_bucket_iterator != result.m_buckets_end &&
            result.m_bucket_iterator->empty()) {
        ++result.m_bucket_iterator;
    }
    if (result.m_bucket_iterator != result.m_buckets_end) {
        result.m_pair_iterator = result.m_bucket_iterator->begin();
    }
    return result;
}

TEMPLATE_DECL
typename CLASS_NAME::iterator CLASS_NAME::end()
{
    iterator result;
    result.m_buckets_end = result.m_bucket_iterator = m_buckets.end();
    return result;
}

TEMPLATE_DECL
typename CLASS_NAME::const_iterator CLASS_NAME::end() const
{
    const_iterator result;
    result.m_buckets_end = result.m_bucket_iterator = m_buckets.end();
    return result;
}

TEMPLATE_DECL
void CLASS_NAME::rehash(const size_type buckets)
{
    Buckets new_buckets(buckets);
    for (const auto& value : *this) {
        auto bucket_iter = new_buckets.begin() + m_hasher(value.first) % new_buckets.size();
        bucket_iter->insert(bucket_iter->begin(), value);
    }
    m_buckets = std::move(new_buckets);
}

TEMPLATE_DECL
typename CLASS_NAME::hasher& CLASS_NAME::get_hasher()
{
    return m_hasher;
}

TEMPLATE_DECL
const typename CLASS_NAME::hasher& CLASS_NAME::get_hasher() const
{
    return m_hasher;
}

TEMPLATE_DECL
const float CLASS_NAME::get_max_load_factor() const
{
    return m_max_load_factor;
}

TEMPLATE_DECL
void CLASS_NAME::set_max_load_factor(const float max_load_factor)
{
    m_max_load_factor = max_load_factor;
}

TEMPLATE_DECL
bool CLASS_NAME::rehash_needed() const
{
    return float(m_size + 1) / float(m_buckets.size()) > m_max_load_factor;
}

#undef TEMPLATE_DECL
#undef CLASS_NAME

} // namespace std
