#ifndef INCLUDED_GENERATOR_H
#define INCLUDED_GENERATOR_H

#include <functional>
#include <exception>
#include <array>
#include <boost/context/all.hpp>

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

    GeneratorCore() { }
    virtual ~GeneratorCore() { }

protected:
    void yield(T product) {
        _product = product;
        // switch back to the main context
        boost::context::jump_fcontext(_child_ctx, _main_ctx, 0);
    }

private:
    T _product;
    boost::context::fcontext_t* _child_ctx;
    boost::context::fcontext_t* _main_ctx;
};

template<typename Core>
class Generator {
public:
    typedef typename Core::Output Output;
    template<typename... ARGS>
    Generator(ARGS... args) :  _done(false) {
        _callback = std::bind([&] (Generator<Core>* gen, ARGS... args) {
                                  gen->_generate_wrapper(args...);},
                                  this, args...);
        // the _child_ctx returned is created on the new stack, so no need
        // to delete it.
        _child_ctx = boost::context::make_fcontext(
                        _stack.data() + _stack.size(),
                        _stack.size(),
                        &Generator<Core>::_c_function_wrapper);
        _producer._main_ctx = &_main_ctx;
        _producer._child_ctx = _child_ctx;
        ++(*this);
    }

    // forbid copy constructor
    Generator(const Generator<Core>&) = delete;

    // NOTE: throw exception when there is exception in generate function
    Generator<Core>& operator ++ () {
        // switch to child context
        boost::context::jump_fcontext(&_main_ctx, _child_ctx, (intptr_t)this);
        if (_exception) {
            std::rethrow_exception(_exception);
        }
        return *this;
    }

    Output operator * () {
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
    template<typename... ARGS>
    void _generate_wrapper(ARGS... args) {
        try {
            _producer.generate(args...);
            _done = true;
            boost::context::jump_fcontext(_child_ctx, &_main_ctx, 0);
        } catch (...) {
            _exception = std::current_exception();
            boost::context::jump_fcontext(_child_ctx, &_main_ctx, 0);
        }
    }

    // Need this function wrapper because make_fcontext only accept c style
    // function pointer void(intptr_t)
    static void _c_function_wrapper(intptr_t param) {
        Generator<Core>* _this = reinterpret_cast<Generator<Core>*>(param);
        (_this->_callback)();
    }

    Core _producer;
    std::array<intptr_t, 64 * 1024> _stack;
    std::function<void(void)> _callback;
    boost::context::fcontext_t* _child_ctx;
    boost::context::fcontext_t _main_ctx;
    bool _done;
    std::exception_ptr _exception;
};

#endif
