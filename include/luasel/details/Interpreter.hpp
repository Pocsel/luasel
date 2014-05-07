#pragma once

#include <luasel/details/State.hpp>
#include <luasel/details/Ref.hpp>
#include <luasel/details/Serializer.hpp>

namespace Luasel {

    // Lua types
    template<class T> struct _LuaNative { using type = T; };
    template<class T> struct _CppNative { using type = typename std::conditional<std::is_pointer<T>::value, T, typename std::add_pointer<T>::type>::type; };
    template<class T>
    struct GetLuaType;
    template<> struct GetLuaType<bool> : public _LuaNative<bool>{};
    template<> struct GetLuaType<int> : public _LuaNative<int>{};
    template<> struct GetLuaType<double> : public _LuaNative<double>{};
    template<> struct GetLuaType<std::string> : public _LuaNative<std::string>{};
    template<> struct GetLuaType<Ref> : public _LuaNative<Ref>{};
    template<class T> struct GetLuaType : public _CppNative<T>{};
    template<class T> struct GetLuaType<T const> : public GetLuaType<T>{};
    template<class T> struct GetLuaType<T&> : public GetLuaType<T>{};

    template<class TTuple>
    struct GetTuple;
    template<class... Ts>
    struct GetTuple<std::tuple<Ts...>>
    {
        using tuple = std::tuple<Ts...>;
        static const std::size_t N = std::tuple_size<tuple>::value;

        using type = std::tuple<typename GetLuaType<Ts>::type...>;
    };

    enum class LibId : int
    {
        Base, // print(), error(), _G, dofile(), select(), tostring(), unpack()...
        Math, // math.abs(), math.sqrt()...
        Table, // table.insert(), table.sort()...
        String, // string.len(), string.format(), string.find()...
        Io, // io.open(), io.read()...
        Os, // os.execute(), os.exit(), os.date()...
        Debug, // debug.sethook(), debug.debug()...
        Package, // module(), package.loadlib(), package.seeall()...
    };

    class Interpreter :
        private boost::noncopyable
    {
    private:
        State* _state;
        Ref* _globals;
        Serializer _serializer;

    public:
        Interpreter(); // can throw std::runtime_error
        ~Interpreter() noexcept;
        Serializer const& GetSerializer() const { return this->_serializer; }
        // script loading
        Ref LoadString(std::string const& code) const; // can throw std::runtime_error
        void RegisterLib(LibId lib); // can throw std::runtime_error
        void DoString(std::string const& code) const; // can throw std::runtime_error
        void DoFile(std::string const& path) const; // can throw std::runtime_error
        // global table
        Ref const& Globals() const noexcept;
        // reference creators
        Ref MakeBoolean(bool val) noexcept{ return this->_state->MakeBoolean(val); }
        Ref MakeFunction(std::function<void(CallHelper&)> val) noexcept{ return this->_state->MakeFunction(val); }
        Ref MakeNil() noexcept{ return this->_state->MakeNil(); }
        Ref MakeInteger(int val) noexcept{ return this->_state->MakeInteger(val); }
        Ref MakeNumber(double val) noexcept{ return this->_state->MakeNumber(val); }
        Ref MakeString(std::string const& val) noexcept{ return this->_state->MakeString(val); }
        Ref MakeTable() noexcept{ return this->_state->MakeTable(); }
        Ref MakeUserData(void** data, size_t size) { return this->_state->MakeUserData(data, size); } // can throw std::runtime_error
        template <typename T>
        Ref Make(T val); // can throw std::runtime_error
        template <typename T>
        Ref MakeMove(T&& val); // can throw std::runtime_error
        // other stuff
        State& GetState() noexcept{ return *this->_state; }
        void DumpStack() const noexcept;

        template<class TFunc>
        typename std::enable_if<!std::is_bind_expression<TFunc>::value && std::is_void<typename FunctionTraits<TFunc>::result>::value, Ref>::type Bind(TFunc function)
        {
            return this->MakeFunction([function](CallHelper& helper) {
                apply(function, helper.popArgsTuple<typename GetTuple<typename FunctionTraits<TFunc>::args>::type>());
            });
        }

        template<class TFunc>
        typename std::enable_if<!std::is_bind_expression<TFunc>::value && !std::is_void<typename FunctionTraits<TFunc>::result>::value, Ref>::type Bind(TFunc function)
        {
            return this->MakeFunction([function](CallHelper& helper) {
                helper.PushRetMove(apply(function, helper.popArgsTuple<typename GetTuple<typename FunctionTraits<TFunc>::args>::type>()));
            });
        }

        template<class TFunc, class... TArgs>
        Ref BindTyped(TFunc function, TArgs...);
    };

}

#include <luasel/details/Function.hpp>
