
GTEST_DIR = ./tests/gtest
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
				$(GTEST_DIR)/include/gtest/internal/*.h
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

OUT_DIR = bin
EXAMPLES_DIR = examples
TESTS_DIR = tests

CXXFLAGS += -std=c++11 -lboost_context -pthread
CXXFLAGS += -isystem $(GTEST_DIR)/include -I.

EXAMPLES = simple_example
TESTS = generator_unittest

# Function to create output diretory
create_bin = mkdir -p $(dir $(OUT_DIR)/$@)
achieve_all = $(AR) $(ARFLAGS) $(OUT_DIR)/$@ $(addprefix $(OUT_DIR)/,$^)

default: all

all: $(EXAMPLES) $(TESTS)

simple_example: simple_example.o
	$(create_bin)
	$(CXX) $(addprefix $(OUT_DIR)/, $^) -o $(OUT_DIR)/$@ $(CXXFLAGS)

simple_example.o: $(EXAMPLES_DIR)/simple_example.cpp $(GTEST_HEADERS) \
				  generator.h
	$(create_bin)
	$(CXX) -c $(EXAMPLES_DIR)/simple_example.cpp -o $(OUT_DIR)/$@ $(CXXFLAGS)

generator_unittest: generator_unittest.o gtest_main.a
	$(create_bin)
	$(CXX) $(addprefix $(OUT_DIR)/, $^) -o $(OUT_DIR)/$@ $(CXXFLAGS)

generator_unittest.o: $(TESTS_DIR)/generator_unittest.cpp $(GTEST_HEADERS) \
					  generator.h
	$(create_bin)
	$(CXX) -c $(TESTS_DIR)/generator_unittest.cpp -o $(OUT_DIR)/$@ $(CXXFLAGS)

gtest-all.o : $(GTEST_SRCS_)
	$(create_bin)
	$(CXX) -I$(GTEST_DIR) -c \
            $(GTEST_DIR)/src/gtest-all.cc -o $(OUT_DIR)/$@ $(CXXFLAGS)

gtest_main.o : $(GTEST_SRCS_)
	$(create_bin)
	$(CXX) -I$(GTEST_DIR) -c \
            $(GTEST_DIR)/src/gtest_main.cc -o $(OUT_DIR)/$@ $(CXXFLAGS)

gtest.a : gtest-all.o
	$(create_bin)
	$(achieve_all)

gtest_main.a : gtest-all.o gtest_main.o
	$(create_bin)
	$(achieve_all)
