#pragma once

#include <luasel/Luasel.hpp>

#include <cassert>
#include <functional>
#include <tuple>

#include <luasel/details/Apply.hpp>
#include <luasel/details/Interpreter.hpp>
#include <luasel/details/Iterator.hpp>
#include <luasel/details/MetaTable.hpp>
#include <luasel/details/Ref.hpp>
#include <luasel/details/Signature.hpp>
#include <luasel/details/State.hpp>
#include <luasel/details/WeakRef.hpp>

namespace Luasel {

    /********* Interpreter *********/
    template <typename T>
    inline Ref Interpreter::Make(T val)
    {
        auto const& m = this->_state->GetMetaTable(typeid(T).hash_code());
        return m.MakeReference(std::move(val));
    }
    template<> inline Ref Interpreter::Make<bool>(bool val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::Make<int>(int val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::Make<unsigned int>(unsigned int val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::Make<char>(char val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::Make<unsigned char>(unsigned char val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::Make<double>(double val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::Make<float>(float val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::Make<std::string>(std::string val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::Make<char const*>(char const* val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::Make<std::function<void(CallHelper&)>>(std::function<void(CallHelper&)> val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::Make<Ref>(Ref val) { return this->_state->Make(val); }
    template<typename T>
    inline Ref Interpreter::MakeMove(T&& val)
    {
        auto const& m = this->_state->GetMetaTable(typeid(T).hash_code());
        return m.MakeReference(std::forward<T>(val));
    }
    template<> inline Ref Interpreter::MakeMove<bool>(bool&& val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::MakeMove<int>(int&& val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::MakeMove<unsigned int>(unsigned int&& val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::MakeMove<char>(char&& val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::MakeMove<unsigned char>(unsigned char&& val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::MakeMove<double>(double&& val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::MakeMove<float>(float&& val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::MakeMove<std::string>(std::string&& val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::MakeMove<char const*>(char const*&& val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::MakeMove<std::function<void(CallHelper&)>>(std::function<void(CallHelper&)>&& val) { return this->_state->Make(val); }
    template<> inline Ref Interpreter::MakeMove<Ref>(Ref&& val) { return this->_state->Make(val); }

    template<class TFunc, class... TArgs>
    Ref Interpreter::BindTyped(TFunc function, TArgs...)
    {
        return this->MakeFunction([function](CallHelper& helper) {
            apply(function, helper.popArgsTuple<std::tuple<TArgs...>>());
        });
    }

    /********* Ref *********/
    template<class T>
    inline T Ref::To() const noexcept
    {
        auto& cppMt = this->_state.GetMetaTable(typeid(typename std::remove_pointer<T>::type).hash_code());
        return cppMt.MakeNative<T>(*this);
    }
    template<> bool Ref::To<bool>() const noexcept;
    template<> int Ref::To<int>() const noexcept;
    template<> unsigned int Ref::To<unsigned int>() const noexcept;
    template<> char Ref::To<char>() const noexcept;
    template<> unsigned char Ref::To<unsigned char>() const noexcept;
    template<> double Ref::To<double>() const noexcept;
    template<> float Ref::To<float>() const noexcept;
    template<> std::string Ref::To<std::string>() const noexcept;
    template<> inline Ref Ref::To<Ref>() const noexcept{ return *this; }

    template<class T>
    inline bool Ref::Is() const noexcept
    {
        if (this->GetMetaTable().IsNoneOrNil() ||
        this->GetMetaTable() != this->_state.GetMetaTable(typeid(typename std::remove_pointer<T>::type).hash_code()).GetMetaTable())
        return false;
        return true;
    }

    template<> bool Ref::Check<bool>(std::string const& e /* = "" */) const;
    template<> int Ref::Check<int>(std::string const& e /* = "" */) const;
    template<> unsigned int Ref::Check<unsigned int>(std::string const& e /* = "" */) const;
    template<> char Ref::Check<char>(std::string const& e /* = "" */) const;
    template<> unsigned char Ref::Check<unsigned char>(std::string const& e /* = "" */) const;
    template<> double Ref::Check<double>(std::string const& e /* = "" */) const;
    template<> float Ref::Check<float>(std::string const& e /* = "" */) const;
    template<> std::string Ref::Check<std::string>(std::string const& e /* = "" */) const;
    template<> inline Ref Ref::Check<Ref>(std::string const&) const { return *this; }

    template<class T>
    inline T Ref::Check(std::string const& err /* = "" */) const
    {
        try
        {
            auto mt = this->GetMetaTable();
            if (mt.IsNoneOrNil())
                throw 1;
            auto& cppMt = this->_state.GetMetaTable(typeid(typename std::remove_pointer<T>::type).hash_code());
            if (mt != cppMt.GetMetaTable())
                throw 1;

            return cppMt.MakeNative<T>(*this);
        }
        catch (int)
        {
            if (!err.empty())
                throw std::runtime_error(err);
            else
                throw std::runtime_error(std::string("Luasel::Ref: Value is not of \"") + typeid(typename std::remove_pointer<T>::type).name() + "\" type");
        }
        catch (std::exception& e)
        {
            if (!err.empty())
                throw std::runtime_error(err);
            else
                throw std::runtime_error(std::string("Luasel::Ref::Check<") + typeid(typename std::remove_pointer<T>::type).name() + ">: " + e.what());
        }
    }

    template <typename T>
    inline T Ref::To(T const& defaultValue) const noexcept
    {
        try
        {
            return this->Check<T>();
        }
        catch (std::exception& e)
        {
            return defaultValue;
        }
    }

    template <class... Targs>
    inline Ref Ref::operator ()(Targs... args) const // can throw std::runtime_error
    {
        CallHelper helper(this->_state.GetInterpreter());
        helper.pushArgs(std::forward<Targs>(args)...);
        this->Call(helper);
        if (helper.GetNbRets())
            return helper.PopRet();
        return Ref(*this);
    }

    template <typename T>
    inline Ref Ref::operator [](T index) const
    {
        return (*this)[this->_state.Make(index)];
    }

    template <typename T, typename U>
    inline Ref Ref::Set(T key, U value) const
    {
        return this->Set(this->_state.Make(key), this->_state.Make(value));
    }

    template <typename T>
    inline bool Ref::Equals(T value) const noexcept
    {
        return *this == this->_state.Make(value);
    }

    /********* CallHelper *********/
    // arguments helpers
    template<class T>
    inline void CallHelper::PushArg(T const& arg) noexcept
    {
        this->PushArg(this->_i.Make(arg));
    }
    template<class T>
    inline void CallHelper::PushArgMove(T&& arg) noexcept
    {
        this->PushArg(this->_i.MakeMove(std::forward<T>(arg)));
    }

    // return helpers
    template<class T>
    inline void CallHelper::PushRet(T const& ret) noexcept
    {
        this->PushRet(this->_i.Make(ret));
    }

    template<class T>
    inline void CallHelper::PushRetMove(T&& ret) noexcept
    {
        this->PushRet(this->_i.MakeMove(std::forward<T>(ret)));
    }

    template<class Targ, class... Targs>
    inline void CallHelper::pushArgs(Targ&& arg, Targs&&... args) noexcept
    {
        this->pushArgs(std::forward<Targs>(args)...);
        this->PushArgMove(std::forward<Targ>(arg));
    }

    template<class Tret, class... Trets>
    inline std::tuple<Tret, Trets...> CallHelper::popArgs(Tret, Trets... args)
    {
        auto tuple = std::make_tuple(this->PopArg().Check<Tret>());
        return std::tuple_cat(std::move(tuple), this->popArgs(args...));
    }

    /********* MetaTable *********/
    template<class T>
    inline MetaTable& MetaTable::Create(
        Interpreter& interpreter,
        std::function<Ref(void const*)> makeRef,
        std::function<void(Ref const&, void*)> makeNative)
    {
        auto& tmp = interpreter.GetState().RegisterMetaTable(MetaTable(interpreter), typeid(T).hash_code());;
        tmp._makeRef = makeRef;
        tmp._makeNative = makeNative;
        tmp._metaTable.Set("__index", tmp._prototype);
        tmp.SetMetaMethod(MetaTable::Collect, [](CallHelper& helper) { _Destructor(helper.PopArg().To<T*>()); });
        return tmp;
    }
    template<class T>
    inline MetaTable& MetaTable::Create(
        Interpreter& interpreter, T&&,
        std::function<Ref(void const*)> makeRef,
        std::function<void(Ref const&, void*)> makeNative)
    {
        auto& tmp = interpreter.GetState().RegisterMetaTable(MetaTable(interpreter), typeid(T).hash_code());;
        tmp._makeRef = makeRef;
        tmp._makeNative = makeNative;
        tmp.SetMetaMethod(MetaTable::Collect, [](CallHelper& helper) { _Destructor(helper.PopArg().To<T*>()); });
        return tmp;
    }
    template<class T>
    inline MetaTable& MetaTable::Create(
        Ref const& table, T&&,
        std::function<Ref(void const*)> makeRef,
        std::function<void(Ref const&, void*)> makeNative)
    {
        auto& tmp = table.GetState().RegisterMetaTable(MetaTable(table), typeid(T).hash_code());;
        tmp._makeRef = makeRef;
        tmp._makeNative = makeNative;
        tmp.SetMetaMethod(MetaTable::Collect, [](CallHelper& helper) { _Destructor(helper.PopArg().To<T*>()); });
        return tmp;
    }

    inline MetaTable::MetaTable(MetaTable&& mt) :
        _interpreter(mt._interpreter),
        _prototype(std::move(mt._prototype)),
        _metaTable(std::move(mt._metaTable)),
        _makeRef(std::move(mt._makeRef))
    {
    }

    template<class T>
    inline Ref MetaTable::MakeReference(T&& data) const
    {
        if (this->_makeRef)
            return this->_makeRef(&data);
        T* luaValue = nullptr;
        auto r = this->_interpreter.MakeUserData(reinterpret_cast<void**>(&luaValue), sizeof(T));
        new (luaValue)T(std::forward<T>(data));
        r.SetMetaTable(this->_metaTable);
        return r;
    }

    template<class T>
    inline typename std::enable_if<!std::is_pointer<T>::value, T>::type MetaTable::MakeNative(Ref const& ref) const
    {
        if (this->_makeNative)
        {
            T value;
            this->_makeNative(ref, &value);
            return value;
        }
        assert("Do not use Check<Type> but Check<Type*>, or specify a \"MakeNative\" function");
        throw std::runtime_error("Do not use Check<Type> but Check<Type*>, or specify a \"MakeNative\" function");
    }
    template<class T>
    inline typename std::enable_if<std::is_pointer<T>::value, T>::type MetaTable::MakeNative(Ref const& ref) const
    {
        return reinterpret_cast<T>(ref.CheckUserData());
    }

}
