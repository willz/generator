
GTEST_DIR = ./tests/gtest
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
				$(GTEST_DIR)/include/gtest/internal/*.h
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

OUT_DIR = bin
TESTS_DIR = tests

CXXFLAGS += -std=c++11 -pthread
CXXFLAGS += -isystem $(GTEST_DIR)/include -I.

EXAMPLES = 
TESTS = generator_unittest

# Function to create output diretory
create_bin = mkdir -p $(dir $(OUT_DIR)/$@)
achieve_all = $(AR) $(ARFLAGS) $(OUT_DIR)/$@ $(addprefix $(OUT_DIR)/,$^)

default: all

all: $(EXAMPLES) $(TESTS)

generator_unittest: generator_unittest.o gtest_main.a
	$(create_bin)
	$(CXX) $(CXXFLAGS) $(addprefix $(OUT_DIR)/, $^) -o $(OUT_DIR)/$@

generator_unittest.o: $(TESTS_DIR)/generator_unittest.cpp $(GTEST_HEADERS) \
					  generator.h
	$(create_bin)
	$(CXX) $(CXXFLAGS) -c $(TESTS_DIR)/generator_unittest.cpp -o $(OUT_DIR)/$@

gtest-all.o : $(GTEST_SRCS_)
	$(create_bin)
	$(CXX) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc -o $(OUT_DIR)/$@

gtest_main.o : $(GTEST_SRCS_)
	$(create_bin)
	$(CXX) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc -o $(OUT_DIR)/$@

gtest.a : gtest-all.o
	$(create_bin)
	$(achieve_all)

gtest_main.a : gtest-all.o gtest_main.o
	$(create_bin)
	$(achieve_all)
