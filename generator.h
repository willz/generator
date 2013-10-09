#ifndef INCLUDED_GENERATOR_H
#define INCLUDED_GENERATOR_H

#include <mutex>
#include <thread>
#include <condition_variable>

// Support iterator opoeration
template<typename T>
class ForIterate {
public:
    ForIterate(T* gen) : _gen(gen) { }

    ForIterate<T>& operator ++ () {
        ++(*_gen);
        return *this;
    }

    typename T::Output operator * () {
        return **_gen;
    }

    bool operator () () const {
        return _gen && *_gen;
    }

    operator bool() const {
        return (_gen != nullptr)  && *_gen;
    }

    // we only need compare to ::end(), so we make end() false, and others
    // true
    bool operator != (const ForIterate<T>& other) {
        return static_cast<bool>(*this) != static_cast<bool>(other);
    }

private:
    T* _gen;
};

template<typename T>
class GeneratorCore {
public:
    typedef T Output;
    template<typename Core>
    friend class Generator;

    GeneratorCore() : _has_product(true) { }
    virtual ~GeneratorCore() { }

    // Item will be produced only when there is no item, if there exists
    // one item, yield() will wait until that item is consumed
    void yield(T product) {
        std::unique_lock<std::mutex> lck(_mtx);
        _cv.wait(lck, [&] () { return !_has_product; });
        _product = product;
        _has_product = true;
        _cv.notify_one();
    }

private:
    T _product;
    std::mutex _mtx;
    std::condition_variable _cv;
    bool _has_product;
};

template<typename Core>
class Generator {
public:
    typedef typename Core::Output Output;
    template<typename... ARGS>
    Generator(ARGS... args) : _mtx(_producer._mtx), _cv(_producer._cv),
                              _done(false) {
        auto callback = std::bind([&] (Generator<Core>* gen, ARGS... args) {
                                  gen->_thread_wrapper(args...);},
                                  this, args...);
        _gen_thread = new std::thread(callback);
        ++(*this);
    }

    ~Generator() {
        _gen_thread->join();
        delete _gen_thread;
    }

    template<typename... ARGS>
    void _thread_wrapper(ARGS... args) {
        _producer.generate(args...);
        std::unique_lock<std::mutex> lck(_mtx);
        // wait is needed here. If the final item hasn't been consumed and
        // end the producer thread. The comsumer thread(++ operator) will wait
        // and never be notified.
        _cv.wait(lck, [&] () { return !_producer._has_product; });
        _done = true;
        _cv.notify_one();
    }

    // By using condition variable, we make the two thread run like:
    // ++, yield, ++, yield, ...
    // In order to make the ++ run before producing the first item,
    // _has_output was init to true, so that even though yield thread
    // goes faster, it will wait until _has_ouput set to false
    Generator<Core>& operator ++ () {
        std::unique_lock<std::mutex> lck(_mtx);
        _producer._has_product = false;
        _cv.notify_one();
        _cv.wait(lck, [&] () { return _producer._has_product || _done; });
        return *this;
    }

    Output operator * () {
        std::unique_lock<std::mutex> lck(_mtx);
        return _producer._product;
    }

    operator bool() {
        return !_done;
    }

    typedef ForIterate<Generator<Core>> ForIterating;
    ForIterating begin() {
        return ForIterating(this);
    }

    ForIterating end() {
        return ForIterating(nullptr);
    }

private:
    Core _producer;
    std::mutex& _mtx;
    std::condition_variable& _cv;
    std::thread* _gen_thread;
    bool _done;
};

#endif
