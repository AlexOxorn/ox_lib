#ifndef __STD_GENERATOR_INCLUDED
#define __STD_GENERATOR_INCLUDED
///////////////////////////////////////////////////////////////////////////////
// Reference implementation of std::generator proposal P2168.
//
// See https://wg21.link/P2168 for details.
//
///////////////////////////////////////////////////////////////////////////////
// Copyright Lewis Baker, Corentin Jabot
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0.
// (See accompanying file LICENSE or http://www.boost.org/LICENSE_1_0.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

#if __has_include(<coroutine>)
  #include <coroutine>
#else
  // Fallback for older experimental implementations of coroutines.
  #include <experimental/coroutine>
namespace std {
    using std::experimental::coroutine_handle;
    using std::experimental::coroutine_traits;
    using std::experimental::noop_coroutine;
    using std::experimental::suspend_always;
    using std::experimental::suspend_never;
} // namespace std
#endif

#include <exception>
#include <iterator>
#include <new>
#include <type_traits>
#include <utility>
#include <concepts>
#include <cassert>

#if __has_include(<ranges>)
  #include <ranges>
#else

// Placeholder implementation of the bits we need from <ranges> header
// when we don't have the <ranges> header (e.g. Clang 12 and earlier).
namespace stdfuture {

    // Don't create naming conflicts with recent libc++ which defines std::iter_reference_t
    // in <iterator> but doesn't yet provide a <ranges> header.
    template <typename _T>
    using iter_reference_t = decltype(*std::declval<_T&>());

    template <typename _T>
    using iter_value_t = typename std::iterator_traits<std::remove_cvref_t<_T>>::value_type;

    namespace ranges {

        namespace begin {
            void begin();

            struct _fn {
                template <typename _Range>
                requires requires(_Range& r) { r.begin(); }
                auto operator()(_Range&& r) const noexcept(noexcept(r.begin())) -> decltype(r.begin()) {
                    return r.begin();
                }

                template <typename _Range>
                requires(!requires(_Range& r) { r.begin(); }) && requires(_Range& r) { begin(r); }
                auto operator()(_Range&& r) const noexcept(noexcept(begin(r))) -> decltype(begin(r)) {
                    return begin(r);
                }
            };

        } // namespace begin

        inline namespace begin_cpo {
            constexpr inline begin::_fn begin = {};
        }

        namespace end {
            void end();

            struct _fn {
                template <typename _Range>
                requires requires(_Range& r) { r.end(); }
                auto operator()(_Range&& r) const noexcept(noexcept(r.end())) -> decltype(r.end()) {
                    return r.end();
                }

                template <typename _Range>
                requires(!requires(_Range& r) { r.end(); }) && requires(_Range& r) { end(r); }
                auto operator()(_Range&& r) const noexcept(noexcept(end(r))) -> decltype(end(r)) {
                    return end(r);
                }
            };
        } // namespace end

        inline namespace _end_cpo {
            constexpr inline end::_fn end = {};
        }

        template <typename _Range>
        using iterator_t = decltype(begin(std::declval<_Range>()));

        template <typename _Range>
        using sentinel_t = decltype(end(std::declval<_Range>()));

        template <typename _Range>
        using range_reference_t = iter_reference_t<iterator_t<_Range>>;

        template <typename _Range>
        using range_value_t = iter_value_t<iterator_t<_Range>>;

        template <typename _T>
        concept range = requires(_T& t) {
            ranges::begin(t);
            ranges::end(t);
        };

    } // namespace ranges
} // namespace stdfuture

#endif // !__has_include(<ranges>)

namespace stdfuture {

    template <typename _T>
    class manual_lifetime {
    public:
        manual_lifetime() noexcept {}
        ~manual_lifetime() {}

        template <typename... _Args>
        _T& construct(_Args&&... args) noexcept(std::is_nothrow_constructible_v<_T, _Args...>) {
            return *::new (static_cast<void*>(std::addressof(value_))) _T((_Args&&) args...);
        }

        void destruct() noexcept(std::is_nothrow_destructible_v<_T>) { value_.~_T(); }

        _T& get() & noexcept { return value_; }
        _T&& get() && noexcept { return static_cast<_T&&>(value_); }
        const _T& get() const & noexcept { return value_; }
        const _T&& get() const && noexcept { return static_cast<const _T&&>(value_); }
    private:
        union {
            std::remove_const_t<_T> value_;
        };
    };

    template <typename _T>
    class manual_lifetime<_T&> {
    public:
        manual_lifetime() noexcept : value_(nullptr) {}
        ~manual_lifetime() {}

        _T& construct(_T& value) noexcept {
            value_ = std::addressof(value);
            return value;
        }

        void destruct() noexcept {}

        _T& get() const noexcept { return *value_; }
    private:
        _T* value_;
    };

    template <typename _T>
    class manual_lifetime<_T&&> {
    public:
        manual_lifetime() noexcept : value_(nullptr) {}
        ~manual_lifetime() {}

        _T&& construct(_T&& value) noexcept {
            value_ = std::addressof(value);
            return static_cast<_T&&>(value);
        }

        void destruct() noexcept {}

        _T&& get() const noexcept { return static_cast<_T&&>(*value_); }
    private:
        _T* value_;
    };

    struct use_allocator_arg {};

    namespace ranges {

        template <typename _Rng, typename _Allocator = use_allocator_arg>
        struct elements_of {
            explicit constexpr elements_of(_Rng&& rng) noexcept
            requires std::is_default_constructible_v<_Allocator>
                    : range(static_cast<_Rng&&>(rng)) {}

            constexpr elements_of(_Rng&& rng, _Allocator&& alloc) noexcept :
                    range((_Rng&&) rng), alloc((_Allocator&&) alloc) {}

            constexpr elements_of(elements_of&&) noexcept = default;

            constexpr elements_of(const elements_of&) = delete;
            constexpr elements_of& operator=(const elements_of&) = delete;
            constexpr elements_of& operator=(elements_of&&) = delete;

            constexpr _Rng&& get() noexcept { return static_cast<_Rng&&>(range); }

            constexpr _Allocator get_allocator() const noexcept { return alloc; }
        private:
            [[no_unique_address]] _Allocator alloc; // \expos
            _Rng&& range;                           // \expos
        };

        template <typename _Rng>
        elements_of(_Rng&&) -> elements_of<_Rng>;

        template <typename _Rng, typename Allocator>
        elements_of(_Rng&&, Allocator&&) -> elements_of<_Rng, Allocator>;

    } // namespace ranges

    template <typename _Alloc>
    constexpr static bool allocator_needs_to_be_stored =
            !std::allocator_traits<_Alloc>::is_always_equal::value || !std::is_default_constructible_v<_Alloc>;

    // Round s up to next multiple of a.
    constexpr size_t aligned_allocation_size(size_t s, size_t a) {
        return (s + a - 1) & ~(a - 1);
    }

    template <typename _Ref, typename _Value = std::remove_cvref_t<_Ref>, typename _Allocator = use_allocator_arg>
    class generator;

    template <typename _Alloc>
    class promise_base_alloc {
        constexpr static std::size_t offset_of_allocator(std::size_t frameSize) noexcept {
            return aligned_allocation_size(frameSize, alignof(_Alloc));
        }

        constexpr static std::size_t padded_frame_size(std::size_t frameSize) noexcept {
            return offset_of_allocator(frameSize) + sizeof(_Alloc);
        }

        static _Alloc& get_allocator(void* frame, std::size_t frameSize) noexcept {
            return *reinterpret_cast<_Alloc*>(static_cast<char*>(frame) + offset_of_allocator(frameSize));
        }
    public:
        template <typename... _Args>
        static void* operator new(std::size_t frameSize, std::allocator_arg_t, _Alloc alloc, _Args&...) {
            void* frame = alloc.allocate(padded_frame_size(frameSize));

            // Store allocator at end of the coroutine frame.
            // Assuming the allocator's move constructor is non-throwing (a requirement for allocators)
            ::new (static_cast<void*>(std::addressof(get_allocator(frame, frameSize)))) _Alloc(std::move(alloc));

            return frame;
        }

        template <typename _This, typename... _Args>
        static void* operator new(std::size_t frameSize, _This&, std::allocator_arg_t, _Alloc alloc, _Args&...) {
            return promise_base_alloc::operator new(frameSize, std::allocator_arg, std::move(alloc));
        }

        static void operator delete(void* ptr, std::size_t frameSize) noexcept {
            _Alloc& alloc = get_allocator(ptr, frameSize);
            _Alloc localAlloc(std::move(alloc));
            alloc.~Alloc();
            localAlloc.deallocate(static_cast<std::byte*>(ptr), padded_frame_size(frameSize));
        }
    };

    template <typename _Alloc>
    requires(!allocator_needs_to_be_stored<_Alloc>)
    class promise_base_alloc<_Alloc> {
    public:
        static void* operator new(std::size_t size) {
            _Alloc alloc;
            return alloc.allocate(size);
        }

        static void operator delete(void* ptr, std::size_t size) noexcept {
            _Alloc alloc;
            alloc.deallocate(static_cast<std::byte*>(ptr), size);
        }
    };

    template <typename _Ref>
    struct generator_promise_base {
        template <typename _Ref2, typename _Value, typename _Alloc>
        friend class generator;

        generator_promise_base* root_;
        std::coroutine_handle<> parentOrLeaf_;
        // Note: Using manual_lifetime here to avoid extra calls to exception_ptr
        // constructor/destructor in cases where it is not needed (i.e. where this
        // generator coroutine is not used as a nested coroutine).
        // This member is lazily constructed by the yield_sequence_awaiter::await_suspend()
        // method if this generator is used as a nested generator.
        manual_lifetime<std::exception_ptr> exception_;
        manual_lifetime<_Ref> value_;

        explicit generator_promise_base(std::coroutine_handle<> thisCoro) noexcept :
                root_(this), parentOrLeaf_(thisCoro) {}

        ~generator_promise_base() {
            if (root_ != this) {
                // This coroutine was used as a nested generator and so will
                // have constructed its exception_ member which needs to be
                // destroyed here.
                exception_.destruct();
            }
        }

        std::suspend_always initial_suspend() noexcept { return {}; }

        void return_void() noexcept {}

        void unhandled_exception() {
            if (root_ != this) {
                exception_.get() = std::current_exception();
            } else {
                throw;
            }
        }

        // Transfers control back to the parent of a nested coroutine
        struct final_awaiter {
            bool await_ready() noexcept { return false; }

            template <typename _Promise>
            std::coroutine_handle<> await_suspend(std::coroutine_handle<_Promise> h) noexcept {
                _Promise& promise = h.promise();
                generator_promise_base& root = *promise.root_;
                if (&root != &promise) {
                    auto parent = promise.parentOrLeaf_;
                    root.parentOrLeaf_ = parent;
                    return parent;
                }
                return std::noop_coroutine();
            }

            void await_resume() noexcept {}
        };

        final_awaiter final_suspend() noexcept { return {}; }

        std::suspend_always yield_value(_Ref&& x) noexcept(std::is_nothrow_move_constructible_v<_Ref>) {
            root_->value_.construct((_Ref&&) x);
            return {};
        }

        template <typename _T>
        requires(!std::is_reference_v<_Ref>) && std::is_convertible_v<_T, _Ref>
        std::suspend_always yield_value(_T&& x) noexcept(std::is_nothrow_constructible_v<_Ref, _T>) {
            root_->value_.construct((_T&&) x);
            return {};
        }

        template <typename _Gen>
        struct yield_sequence_awaiter {
            _Gen gen_;

            yield_sequence_awaiter(_Gen&& g) noexcept
                    // Taking ownership of the generator ensures frame are destroyed
                    // in the reverse order of their execution.
                    :
                    gen_((_Gen&&) g) {}

            bool await_ready() noexcept { return false; }

            // set the parent, root and exceptions pointer and
            // resume the nested
            template <typename _Promise>
            std::coroutine_handle<> await_suspend(std::coroutine_handle<_Promise> h) noexcept {
                generator_promise_base& current = h.promise();
                generator_promise_base& nested = *gen_.get_promise();
                generator_promise_base& root = *current.root_;

                nested.root_ = current.root_;
                nested.parentOrLeaf_ = h;

                // Lazily construct the exception_ member here now that we
                // know it will be used as a nested generator. This will be
                // destroyed by the promise destructor.
                nested.exception_.construct();
                root.parentOrLeaf_ = gen_.get_coro();

                // Immediately resume the nested coroutine (nested generator)
                return gen_.get_coro();
            }

            void await_resume() {
                generator_promise_base& nestedPromise = *gen_.get_promise();
                if (nestedPromise.exception_.get()) {
                    std::rethrow_exception(std::move(nestedPromise.exception_.get()));
                }
            }
        };

        template <typename _OValue, typename _OAlloc>
        yield_sequence_awaiter<generator<_Ref, _OValue, _OAlloc>>
        yield_value(ranges::elements_of<generator<_Ref, _OValue, _OAlloc>> g) noexcept {
            return std::move(g).get();
        }

        template <std::ranges::range _Rng, typename _Allocator>
        yield_sequence_awaiter<generator<_Ref, std::remove_cvref_t<_Ref>, _Allocator>>
        yield_value(ranges::elements_of<_Rng, _Allocator>&& x) {
            return [](std::allocator_arg_t,
                      _Allocator alloc,
                      auto&& rng) -> generator<_Ref, std::remove_cvref_t<_Ref>, _Allocator> {
                for (auto&& e : rng)
                    co_yield static_cast<decltype(e)>(e);
            }(std::allocator_arg, x.get_allocator(), std::forward<_Rng>(x.get()));
        }

        void resume() { parentOrLeaf_.resume(); }

        // Disable use of co_await within this coroutine.
        void await_transform() = delete;
    };

    template <typename _Generator, typename _ByteAllocator, bool _ExplicitAllocator = false>
    struct generator_promise;

    template <typename _Ref, typename _Value, typename _Alloc, typename _ByteAllocator, bool _ExplicitAllocator>
    struct generator_promise<generator<_Ref, _Value, _Alloc>, _ByteAllocator, _ExplicitAllocator> final
            : public generator_promise_base<_Ref>,
              public promise_base_alloc<_ByteAllocator> {
        generator_promise() noexcept :
                generator_promise_base<_Ref>(std::coroutine_handle<generator_promise>::from_promise(*this)) {}

        generator<_Ref, _Value, _Alloc> get_return_object() noexcept {
            return generator<_Ref, _Value, _Alloc>{std::coroutine_handle<generator_promise>::from_promise(*this)};
        }

        using generator_promise_base<_Ref>::yield_value;

        template <std::ranges::range _Rng>
        typename generator_promise_base<_Ref>::template yield_sequence_awaiter<generator<_Ref, _Value, _Alloc>>
        yield_value(ranges::elements_of<_Rng>&& x) {
            static_assert(!_ExplicitAllocator,
                          "This coroutine has an explicit allocator specified with std::allocator_arg so an allocator "
                          "needs to be passed "
                          "explicitely to std::elements_of");
            return [](auto&& rng) -> generator<_Ref, _Value, _Alloc> {
                for (auto&& e : rng)
                    co_yield static_cast<decltype(e)>(e);
            }(std::forward<_Rng>(x.get()));
        }
    };

    template <typename _Alloc>
    using byte_allocator_t =
            typename std::allocator_traits<std::remove_cvref_t<_Alloc>>::template rebind_alloc<std::byte>;
}
namespace std {
    // Type-erased allocator with default allocator behaviour.
    template <typename _Ref, typename _Value, typename... _Args>
    struct coroutine_traits<stdfuture::generator<_Ref, _Value>, _Args...> {
        using promise_type = stdfuture::generator_promise<stdfuture::generator<_Ref, _Value>, std::allocator<std::byte>>;
    };

    // Type-erased allocator with std::allocator_arg parameter
    template <typename _Ref, typename _Value, typename _Alloc, typename... _Args>
    struct coroutine_traits<stdfuture::generator<_Ref, _Value>, allocator_arg_t, _Alloc, _Args...> {
    private:
        using byte_allocator = stdfuture::byte_allocator_t<_Alloc>;
    public:
        using promise_type = stdfuture::generator_promise<stdfuture::generator<_Ref, _Value>, byte_allocator, true /*explicit Allocator*/>;
    };

    // Type-erased allocator with std::allocator_arg parameter (non-static member functions)
    template <typename _Ref, typename _Value, typename _This, typename _Alloc, typename... _Args>
    struct coroutine_traits<stdfuture::generator<_Ref, _Value>, _This, allocator_arg_t, _Alloc, _Args...> {
    private:
        using byte_allocator = stdfuture::byte_allocator_t<_Alloc>;
    public:
        using promise_type = stdfuture::generator_promise<stdfuture::generator<_Ref, _Value>, byte_allocator, true /*explicit Allocator*/>;
    };

    // Generator with specified allocator type
    template <typename _Ref, typename _Value, typename _Alloc, typename... _Args>
    struct coroutine_traits<stdfuture::generator<_Ref, _Value, _Alloc>, _Args...> {
        using byte_allocator = stdfuture::byte_allocator_t<_Alloc>;
    public:
        using promise_type = stdfuture::generator_promise<stdfuture::generator<_Ref, _Value, _Alloc>, byte_allocator>;
    };
}

namespace stdfuture {

    // TODO :  make layout compatible promise casts possible
    template <typename _Ref, typename _Value, typename _Alloc>
    class generator {
        using byte_allocator = byte_allocator_t<_Alloc>;
    public:
        using promise_type = generator_promise<generator<_Ref, _Value, _Alloc>, byte_allocator>;
        friend promise_type;
    private:
        using coroutine_handle = std::coroutine_handle<promise_type>;
    public:
        generator() noexcept = default;

        generator(generator&& other) noexcept :
                coro_(std::exchange(other.coro_, {})), started_(std::exchange(other.started_, false)) {}

        ~generator() noexcept {
            if (coro_) {
                if (started_ && !coro_.done()) {
                    coro_.promise().value_.destruct();
                }
                coro_.destroy();
            }
        }

        generator& operator=(generator&& g) noexcept {
            swap(g);
            return *this;
        }

        void swap(generator& other) noexcept {
            std::swap(coro_, other.coro_);
            std::swap(started_, other.started_);
        }

        struct sentinel {};

        class iterator {
        public:
            using iterator_category = std::input_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = _Value;
            using reference = _Ref;
            using pointer = std::add_pointer_t<_Ref>;

            iterator() noexcept = default;
            iterator(const iterator&) = delete;

            iterator(iterator&& other) noexcept : coro_(std::exchange(other.coro_, {})) {}

            iterator& operator=(iterator&& other) {
                std::swap(coro_, other.coro_);
                return *this;
            }

            ~iterator() {}

            friend bool operator==(const iterator& it, sentinel) noexcept { return it.coro_.done(); }

            iterator& operator++() {
                coro_.promise().value_.destruct();
                coro_.promise().resume();
                return *this;
            }
            void operator++(int) { (void) operator++(); }

            reference operator*() const noexcept { return static_cast<reference>(coro_.promise().value_.get()); }
        private:
            friend generator;

            explicit iterator(coroutine_handle coro) noexcept : coro_(coro) {}

            coroutine_handle coro_;
        };

        iterator begin() {
            assert(coro_);
            assert(!started_);
            started_ = true;
            coro_.resume();
            return iterator{coro_};
        }

        sentinel end() noexcept { return {}; }
    private:
        explicit generator(coroutine_handle coro) noexcept : coro_(coro) {}
    public: // to get around access restrictions for yield_sequence_awaitable
        std::coroutine_handle<> get_coro() noexcept { return coro_; }
        promise_type* get_promise() noexcept { return std::addressof(coro_.promise()); }
    private:
        coroutine_handle coro_;
        bool started_ = false;
    };

    // Specialisation for type-erased allocator implementation.
    template <typename _Ref, typename _Value>
    class generator<_Ref, _Value, use_allocator_arg> {
        using promise_base = generator_promise_base<_Ref>;
    public:
        generator() noexcept : promise_(nullptr), coro_(), started_(false) {}

        generator(generator&& other) noexcept :
                promise_(std::exchange(other.promise_, nullptr)),
                coro_(std::exchange(other.coro_, {})),
                started_(std::exchange(other.started_, false)) {}

        ~generator() noexcept {
            if (coro_) {
                if (started_ && !coro_.done()) {
                    promise_->value_.destruct();
                }
                coro_.destroy();
            }
        }

        generator& operator=(generator g) noexcept {
            swap(g);
            return *this;
        }

        void swap(generator& other) noexcept {
            std::swap(promise_, other.promise_);
            std::swap(coro_, other.coro_);
            std::swap(started_, other.started_);
        }

        struct sentinel {};

        class iterator {
        public:
            using iterator_category = std::input_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = _Value;
            using reference = _Ref;
            using pointer = std::add_pointer_t<_Ref>;

            iterator() noexcept = default;
            iterator(const iterator&) = delete;

            iterator(iterator&& other) noexcept :
                    promise_(std::exchange(other.promise_, nullptr)), coro_(std::exchange(other.coro_, {})) {}

            iterator& operator=(iterator&& other) {
                promise_ = std::exchange(other.promise_, nullptr);
                coro_ = std::exchange(other.coro_, {});
                return *this;
            }

            ~iterator() = default;

            friend bool operator==(const iterator& it, sentinel) noexcept { return it.coro_.done(); }

            iterator& operator++() {
                promise_->value_.destruct();
                promise_->resume();
                return *this;
            }

            void operator++(int) { (void) operator++(); }

            reference operator*() const noexcept { return static_cast<reference>(promise_->value_.get()); }
        private:
            friend generator;

            explicit iterator(promise_base* promise, std::coroutine_handle<> coro) noexcept :
                    promise_(promise), coro_(coro) {}

            promise_base* promise_;
            std::coroutine_handle<> coro_;
        };

        iterator begin() {
            assert(coro_);
            assert(!started_);
            started_ = true;
            coro_.resume();
            return iterator{promise_, coro_};
        }

        sentinel end() noexcept { return {}; }
    private:
        template <typename _Generator, typename _ByteAllocator, bool _ExplicitAllocator>
        friend struct generator_promise;

        template <typename _Promise>
        explicit generator(std::coroutine_handle<_Promise> coro) noexcept :
                promise_(std::addressof(coro.promise())), coro_(coro) {}
    public: // to get around access restrictions for yield_sequence_awaitable
        std::coroutine_handle<> get_coro() noexcept { return coro_; }
        promise_base* get_promise() noexcept { return promise_; }
    private:
        promise_base* promise_;
        std::coroutine_handle<> coro_;
        bool started_ = false;
    };
}

namespace std {

#if __has_include(<ranges>)
    namespace ranges {

        template <typename _T, typename _U, typename _Alloc>
        constexpr inline bool enable_view<stdfuture::generator<_T, _U, _Alloc>> = true;

    } // namespace ranges
#endif

} // namespace stdfuture

#endif // STD_GENERATOR_INCLUDED