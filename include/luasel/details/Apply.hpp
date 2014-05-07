#pragma once

#include <functional>
#include <tuple>
#include <type_traits>

namespace Luasel {

    // Functions
    template<class>
    struct FunctionTraits;
    template<class T>
    struct FunctionTraits<T*> : public FunctionTraits<T>{};
    template<class T>
    struct FunctionTraits<T&> : public FunctionTraits<T>{};

    template<class TResult, class... TArgs>
    struct FunctionTraits<TResult(TArgs...)>
    {
        static const std::size_t nbArgs = sizeof...(TArgs);
        using result = TResult;
        using args = std::tuple<TArgs...>;

        template<class TFunc>
        static result call(TFunc f, TArgs... args)
        {
            return f(std::forward<TArgs>(args)...);
        }
    };
    template<class TResult, class... TArgs>
    struct FunctionTraits<TResult(*const)(TArgs...)> : public FunctionTraits<TResult(TArgs...)>{};

    template<class>
    struct FunctorTraits;
    template<class TResult, class TClass, class... TArgs>
    struct FunctorTraits<TResult(TClass, TArgs...)>
    {
        static const std::size_t nbArgs = sizeof...(TArgs);
        using result = TResult;
        using args = std::tuple<TClass, TArgs...>;
        using argsWithoutObj = std::tuple<TArgs...>;

        template<class TFunc>
        static result call(TFunc f, TClass obj, TArgs... args)
        {
            return (obj.*f)(std::forward<TArgs>(args)...);
        }
    };

    template<class TResult, class TClass, class... TArgs>
    struct FunctionTraits<TResult(TClass::*)(TArgs...)> : public FunctorTraits<TResult(TClass&, TArgs...)>{};
    template<class TResult, class TClass, class... TArgs>
    struct FunctionTraits<TResult(TClass::*)(TArgs...) const> : public FunctorTraits<TResult(TClass&, TArgs...)>{};
    template<class TResult, class TClass, class... TArgs>
    struct FunctionTraits<TResult(TClass::*)(TArgs...) volatile> : public FunctorTraits<TResult(TClass&, TArgs...)>{};
    template<class TResult, class TClass, class... TArgs>
    struct FunctionTraits<TResult(TClass::* const)(TArgs...)> : public FunctorTraits<TResult(TClass&, TArgs...)>{};
    template<class TResult, class TClass, class... TArgs>
    struct FunctionTraits<TResult(TClass::* const)(TArgs...) const> : public FunctorTraits<TResult(TClass&, TArgs...)>{};
    template<class TResult, class TClass, class... TArgs>
    struct FunctionTraits<TResult(TClass::* const)(TArgs...) volatile> : public FunctorTraits<TResult(TClass&, TArgs...)>{};
    template<class TResult, class... TArgs>
    struct FunctionTraits<std::function<TResult(TArgs...)>>
    {
        static const std::size_t nbArgs = sizeof...(TArgs);
        using result = TResult;
        using args = std::tuple<TArgs...>;

        static result call(std::function<TResult(TArgs...)>& func, TArgs... args)
        {
            return func(std::forward<TArgs>(args)...);
        }
    };

    template<class T>
    struct FunctionTraits
    {
        static_assert(!std::is_bind_expression<T>::value, "Can't be used with \"std::bind\"");
        using traits = FunctionTraits<decltype(&T::operator())>;
        static const std::size_t nbArgs = traits::nbArgs;
        using result = typename traits::result;
        using args = typename traits::argsWithoutObj;

        template<class TFunc, class... TArgs>
        static result call(TFunc f, TArgs... args)
        {
            return f(std::forward<TArgs>(args)...);
        }
    };

    // Sequences
    template<std::size_t...>
    struct Indices;

    template<std::size_t N, class TIndices, class... TTypes>
    struct MakeIndices_Impl;

    template<std::size_t N, std::size_t... TIndices, class TType, class... TTypes>
    struct MakeIndices_Impl<N, Indices<TIndices...>, TType, TTypes...>
    {
        using type = typename MakeIndices_Impl<N + 1, Indices<TIndices..., N>, TTypes...>::type;
    };

    template<std::size_t N, std::size_t... TIndices>
    struct MakeIndices_Impl<N, Indices<TIndices...>>
    {
        using type = Indices<TIndices...>;
    };

    template<std::size_t N, class... TTypes>
    struct MakeIndices
    {
        using type = typename MakeIndices_Impl<0, Indices<>, TTypes...>::type;
    };

    // Apply
    template<class Indices>
    struct ApplyTuple_Impl;

    template<class TRet>
    struct Get
    {
        using Ret = typename std::remove_reference<typename std::remove_cv<TRet>::type>::type;
        using RetNP = typename std::remove_pointer<Ret>::type;
        using RetP = typename std::add_pointer<Ret>::type;

        //template<class TArg>
        //static TRet get(TArg&& x);// { return std::forward<TArg>(x); }

        template<class TArg>
        static TRet get(TArg x, typename std::enable_if<std::is_convertible<typename std::remove_reference<TArg>::type, Ret>::value>::type* = nullptr)
        { return std::forward<TArg>(x); }

        template<class TArg>
        static TRet get(TArg x, typename std::enable_if<std::is_convertible<typename std::remove_reference<TArg>::type, RetP>::value>::type* = nullptr)
        { return *x; }

        template<class TArg>
        static TRet get(TArg x, typename std::enable_if<std::is_convertible<typename std::remove_reference<TArg>::type, RetNP>::value && !std::is_same<Ret, RetNP>::value>::type* = nullptr)
        { return &x; }
    };

    template<template<std::size_t...> class TIndice, std::size_t... Indices>
    struct ApplyTuple_Impl<TIndice<Indices...>>
    {
        template<class TFunc, class... TArgs>
        static typename FunctionTraits<TFunc>::result apply(TFunc f, std::tuple<TArgs...>&& args)
        {
            using traits = FunctionTraits<TFunc>;
            return traits::call(std::forward<TFunc>(f), Get<std::tuple_element<Indices, traits::args>::type>::get(std::get<Indices>(args))...);
        }
    };

    template<class TFunc, class... TArgs, class Indices = typename MakeIndices<0, TArgs...>::type>
    inline typename FunctionTraits<TFunc>::result apply(TFunc f, std::tuple<TArgs...>&& args)
    {
        return ApplyTuple_Impl<Indices>::apply(std::forward<TFunc>(f), std::forward<std::tuple<TArgs...>&&>(args));
    }
}
