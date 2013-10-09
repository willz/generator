#include "generator.h"
#include <gtest/gtest.h>
#include <vector>
#include <unistd.h>

// classes for tests
class EmptyTestGen : public GeneratorCore<int> {
public:
    void generate() {
        // do nothing
    }
};

class SimpleTestGen : public GeneratorCore<int> {
public:
    void generate(int start, int end, int interval = 1) {
        for (int i = start; i < end; ++i) {
            yield(i);
        }
    }
};

class OrderTestGen : public GeneratorCore<int> {
public:
    void generate(int start, int end, int interval = 1) {
        for (int i = start; i < end; ++i) {
            usleep(std::rand() % 2000);
            yield(i);
        }
    }
};

class GeneratorTest : public testing::Test {
protected:
    virtual void SetUp() {
        std::srand(std::time(0));
    }
};

TEST_F(GeneratorTest, EmptyTest) {
    for (auto i : Generator<EmptyTestGen>()) {
        FAIL();
    }
}

TEST_F(GeneratorTest, SimpleTest) {
    int start = 1;
    int end = 100;
    int j = 1;
    for (auto i : Generator<SimpleTestGen>(start, end)) {
        EXPECT_EQ(i, j);
        ++j;
    }
    EXPECT_EQ(j, 100);
}

TEST_F(GeneratorTest, OrderTest) {
    int start = 1;
    int end = 100;
    int j = 1;
    for (auto i : Generator<OrderTestGen>(start, end)) {
        usleep(std::rand() % 2000);
        EXPECT_EQ(i, j);
        ++j;
    }
    EXPECT_EQ(j, 100);
}
