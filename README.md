generator
=========

Provide python-style generator(yield) for C++ 11

Examples
--------
Read examples/simple_example.cpp first, which implements a int generator and a token generator

How to Use
----------
First, Define a class inherits GeneratorCore, and implement a public generate() method. e.x.

	class SomeInt : public GeneratorCore<int> {
	public:
		void generate() {
			yield(1);
			yield(3);
			...
		}
	}
	
Second, Create the generator via Generator template. e.x.

	Geneorator<SomeInt>()


How to Compile
--------------
Only the generator.h file is needed for the generator library. It is implemented using c++ 11 features and thread. So to compile, using options
	-std=c++11 -pthread
