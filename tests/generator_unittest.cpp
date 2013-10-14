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

// Test exception
struct GeneratorException : public std::exception {
    const char* what() const noexcept { return "generate exception\n"; }
};

class BasicExceptionTestGen : public GeneratorCore<int> {
public:
    void generate() {
        throw GeneratorException{};
    }
};

class MoreExceptionTestGen : public GeneratorCore<int> {
public:
    void generate() {
        yield(1);
        yield(2);
        throw GeneratorException{};
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

TEST_F(GeneratorTest, BasicExceptionTest) {
    EXPECT_THROW(Generator<BasicExceptionTestGen>{}, GeneratorException);
}
    
TEST_F(GeneratorTest, MoreExceptionTest) {
    Generator<MoreExceptionTestGen> gen{};
    auto it = gen.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    EXPECT_THROW(++it, GeneratorException);
}
