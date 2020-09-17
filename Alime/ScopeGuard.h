#pragma once

#include <functional>
#include <new>
#include <type_traits>
#include <utility>
#include <tuple>


/*
modified from folly/scopeGuard.h
*/

//TupleTy类型为std::Tuple<F, A...>
template <class TupleTy, size_t... indices>
auto _Invoke(void* _RawVals) noexcept
{
    TupleTy* ptr=static_cast<TupleTy*>(_RawVals);
    TupleTy& tupleData = *ptr;
    return std::invoke(std::move(std::get<indices>(tupleData))...);
}

//
template <class TupleTy, size_t... _Indices>
static constexpr auto _Get_invoke(std::index_sequence<_Indices...>) noexcept
{
    return &_Invoke<TupleTy, _Indices...>;
}

template <typename F, typename... A>
auto invoke_cold(F&& f, A&&... a)-> decltype(static_cast<F&&>(f)(static_cast<A&&>(a)...))
{
    //way-1, return directly:
    return static_cast<F&&>(f)(static_cast<A&&>(a)...);

    //way-2, 这里仅仅演示如何封装一个闭包函数 make tuple-> make make equence with F->invoke this func
    //using TupleTy = std::tuple<std::decay_t<F>, std::decay_t<A>...>;
    //auto arg_copy = std::make_unique<TupleTy>(std::forward<F>(f), std::forward<A>(a)...);
    //we just has a template function point here
    //constexpr auto _Invoker_proc = _Get_invoke<TupleTy>(std::make_index_sequence<1 + sizeof...(A)>{});
   // return _Invoker_proc(arg_copy.get());

}

//指定异常类型
//异常对象作为参数传给CatchFunc
template <typename E, typename TryFunc, typename CatchFunc, typename... CatchArg>
auto catch_exception(TryFunc&& t, CatchFunc&& c, CatchArg&&... a) -> typename std::common_type<
    decltype(static_cast<TryFunc&&>(t)()),
    decltype(static_cast<CatchFunc&&>(c)(std::declval<E>(), static_cast<CatchArg&&>(a)...))
    >::type
{
    try {
        return static_cast<TryFunc&&>(t)();
    }
    catch (E& e) {
        return invoke_cold(static_cast<CatchFunc&&>(c), e, static_cast<CatchArg&&>(a)...);
    }
}

//catch all
//t :functor
//c :functor
//困难在于返回值类型限制了t和c
template <typename Try, typename Catch, typename... CatchA>
auto catch_exception(Try&& t, Catch&& c, CatchA&&... a) -> typename std::common_type<
    decltype(static_cast<Try&&>(t)()),
    decltype(static_cast<Catch&&>(c)(static_cast<CatchA&&>(a)...))>::type
{
    try
    {
        return static_cast<Try&&>(t)();
    }
    catch (...) {
        return invoke_cold(static_cast<Catch&&>(c), static_cast<CatchA&&>(a)...);
    }
}



class ScopeGuardImplBase
{
public:
    void dismiss() noexcept
    {
        dismissed_ = true;
    }

protected:
    ScopeGuardImplBase() noexcept
        :dismissed_(false)
    {
    }

    static void warnAboutToCrash() noexcept
    {
        //fix me, log here
    }

    static ScopeGuardImplBase makeEmptyScopeGuard() noexcept
    {
        return ScopeGuardImplBase{};
    }

    template <typename T>
    static const T& asConst(const T& t) noexcept
    {
        return t;
    }

protected:
    bool dismissed_;
};


template <typename F, bool InvokeNoexcept>
class ScopeGuardImpl : public ScopeGuardImplBase
{
public:
    //作为作用域保护者, guard构造时自身不允许抛出异常，允许不起作用。
    explicit ScopeGuardImpl(F& fn) noexcept(std::is_nothrow_copy_constructible<F>::value)
        : ScopeGuardImpl(asConst(fn), makeFailsafe(std::is_nothrow_copy_constructible<F>{}, &fn))
    {
    }

    explicit ScopeGuardImpl(const F& fn) noexcept(std::is_nothrow_copy_constructible<F>::value)
        : ScopeGuardImpl(fn, makeFailsafe(std::is_nothrow_copy_constructible<F>{},&fn))
    {
    }

    explicit ScopeGuardImpl(F&& fn) noexcept(std::is_nothrow_move_constructible<F>::value)
        : ScopeGuardImpl(std::move_if_noexcept(fn), makeFailsafe(std::is_nothrow_move_constructible<F>{},& fn))
    {
    }

    ScopeGuardImpl(ScopeGuardImpl&& other) noexcept(std::is_nothrow_move_constructible<F>::value)
        : function_(std::move_if_noexcept(other.function_))
    {
        dismissed_ = std::exchange(other.dismissed_, true);
    }

    ~ScopeGuardImpl() noexcept(InvokeNoexcept)
    {
        if (!dismissed_)
        {
            execute();
        }
    }

private:
    static ScopeGuardImplBase makeFailsafe(std::true_type, const void*) noexcept
    {
        return makeEmptyScopeGuard();
    }

    template <typename Fn>
    static auto makeFailsafe(std::false_type, Fn* fn) noexcept
        -> ScopeGuardImpl<decltype(std::ref(*fn)), InvokeNoexcept>
    {
        return ScopeGuardImpl<decltype(std::ref(*fn)), InvokeNoexcept>{std::ref(*fn)};
    }

    template <typename Fn>
    explicit ScopeGuardImpl(Fn&& fn, ScopeGuardImplBase&& failsafe)
        : ScopeGuardImplBase{}, function_(std::forward<Fn>(fn))
    {
        failsafe.dismiss();
    }

    void* operator new(std::size_t) = delete;

    void execute() noexcept(InvokeNoexcept)
    {
        if (InvokeNoexcept)
        {
            using R = decltype(function_());
            auto catcher = []() -> R { warnAboutToCrash(), std::terminate(); };
            catch_exception(function_, catcher);
        }
        else
        {
            function_();
        }
    }

    F function_;
};

template <typename F, bool INE>
using ScopeGuardImplDecay = ScopeGuardImpl<typename std::decay<F>::type, INE>;


template <typename F>
ScopeGuardImplDecay<F, true> makeGuard(F&& f) noexcept(
    noexcept(ScopeGuardImplDecay<F, true>(static_cast<F&&>(f)))) {
    return ScopeGuardImplDecay<F, true>(static_cast<F&&>(f));
}


template <typename F, bool ExecuteOnException>
class ScopeGuardForNewException
{
public:
    explicit ScopeGuardForNewException(const F& fn)
        : guard_(fn)
    {
    }

    explicit ScopeGuardForNewException(F&& fn)
        : guard_(std::move(fn))
    {
    }

    ScopeGuardForNewException(ScopeGuardForNewException&& other) = default;

    ~ScopeGuardForNewException() noexcept(ExecuteOnException)
    {
        guard_.dismiss();
    }

private:
    void* operator new(std::size_t) = delete;
    void operator delete(void*) = delete;

    ScopeGuardImpl<F, ExecuteOnException> guard_;
    int exceptionCounter_{ uncaught_exceptions() };
};

enum class ScopeGuardOnExit {};

template <typename F>
ScopeGuardImpl<typename std::decay<F>::type, true>
    operator+(ScopeGuardOnExit, F&& fn)
{
    return ScopeGuardImpl<typename std::decay<F>::type, true>(std::forward<F>(fn));
}

#define CONCATENATE_IMPL(s1, s2) s1##s2
#define ALIME_CONCATENATE(s1, s2) CONCATENATE_IMPL(s1, s2)
#define ALIME_MAKE_VARIABLE(str) ALIME_CONCATENATE(str, __COUNTER__)

#define ALIME_SCOPE_EXIT \
  auto ALIME_MAKE_VARIABLE(SCOPE_EXIT_STATE) = \
      ScopeGuardOnExit() + [&]() noexcept


