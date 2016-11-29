#pragma once

namespace test {

const int SIZE = 1000;

#define TEST(x) \
if ((x)) {\
    std::cout << "PASS    " << __FUNCTION__ << std::endl;\
} else {\
    std::cout << "FAILED  " << __FUNCTION__ << std::endl;\
}

template <typename T, typename E>
void initialize(std::hash_table<T, E>& tree)
{
    for (T i = 0; i < SIZE; ++i) {
        tree.insert(i, E(i % 26 + 65));
    }
}

} //namespace test

void default_constructor()
{
    std::hash_table<int, char> empty;

    TEST(empty.size() == 0 && empty.empty());
}

void initialize_list_constructor()
{
    std::hash_table<int, char> i_hash_table = {{1, 'A'}, {2, 'B'}, {3, 'A'}, {4, 'G'}};

    assert(i_hash_table.size() == 4);
    TEST(i_hash_table[1] == 'A');
    TEST(i_hash_table[2] == 'B');
    TEST(i_hash_table[3] == 'A');
    TEST(i_hash_table[4] == 'G');
}

void copy_constructor()
{
    std::hash_table<int, char> i_hash_table = {{1, 'A'}, {2, 'B'}, {3, 'A'}, {4, 'G'}};
    std::hash_table<int, char> copy_table(i_hash_table);

    assert(i_hash_table.size() == copy_table.size());
    TEST(std::equal(copy_table.begin(), copy_table.end(), i_hash_table.begin()));
}

void copy_assignment()
{
    std::hash_table<int, char> initial;
    test::initialize(initial);

    assert(initial.size() == test::SIZE);
    std::hash_table<int, char> copy;

    copy = initial;
    assert(initial.size() == test::SIZE);

    TEST(std::equal(copy.begin(), copy.end(), initial.begin()));
}

void move_constructor()
{
    std::hash_table<int, char> initial;
    test::initialize(initial);

    std::hash_table<int, char> moved(std::move(initial));
    assert(initial.empty());

    TEST(moved.size() == test::SIZE);
}

void move_assignement()
{
    std::hash_table<int, char> tree;
    test::initialize(tree);

    std::hash_table<int, char> moved;
    moved = std::move(tree);
    assert(tree.empty());

    TEST(moved.size() == test::SIZE);
}

void clear()
{
    std::hash_table<int, char> table;
    test::initialize(table);

    table.clear();
    TEST(table.size() == 0 && table.empty());
}

