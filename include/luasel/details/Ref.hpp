#pragma once

#include <luasel/details/CallHelper.hpp>
#include <luasel/details/State.hpp>

namespace Luasel {

    class Iterator;

    class Ref
    {
    private:
        State& _state;
        int _ref;

    public:
        Ref(State& state) noexcept;
        Ref(Ref const& ref) noexcept;
        Ref(Ref&& ref) noexcept;
        ~Ref() noexcept; // call Unref() to allow destruction after interpreter destruction without crashing
        Ref& operator =(Ref const& ref) noexcept;
        Ref& operator =(Ref&& ref) noexcept;
        bool operator ==(Ref const& ref) const noexcept; // this is a value comparison, not a reference comparison (uses lua_rawequal)
        bool operator !=(Ref const& ref) const noexcept; // same
        template <typename T>
        bool Equals(T value) const noexcept; // same
        void Unref() noexcept;
        bool IsValid() const noexcept; // returns false if Unref() was called, or if the reference was created without copy (with a State)
        size_t GetLength() const noexcept; // returns the length for a string, the size for a table (#), the number of bytes for a user data and 0 for everything else
        // function call
        void Call(CallHelper& callHelper) const; // can throw std::runtime_error
        template <class... Targs>
        Ref operator ()(Targs... args) const; // can throw std::runtime_error
        // array access
        Ref GetTable(std::string const& name) const; // can throw std::runtime_error // returns the table "name" (it is created if necessary)
        Iterator Begin() const; // can throw std::runtime_error
        Iterator End() const; // can throw std::runtime_error // iterators are equal ONLY if they are end iterators (ref.Begin() != ref.Begin())
        Ref operator [](Ref const& index) const; // can throw std::runtime_error
        template <typename T>
        Ref operator [](T index) const; // can throw std::runtime_error
        // array setters
        Ref Set(Ref const& key, Ref const& value) const; // can throw std::runtime_error // returns value
        template <typename T, typename U>
        Ref Set(T key, U value) const; // can throw std::runtime_error // returns a new reference to value
        // metatable
        Ref SetMetaTable(Ref const& table) const; // can throw std::runtime_error // returns table
        bool HasMetaTable() const noexcept; // might return true even if the metatable is not a table (but something else like a number)
        Ref GetMetaTable() const noexcept; // returns a reference to nil if no metatable was found
        // safe type conversions
        bool ToBoolean() const noexcept; // everything is true except nil and false (0 and "0" are true)
        int ToInteger() const noexcept; // type conversion done by lua, returns 0 in case of error
        double ToNumber() const noexcept; // type conversion done by lua, returns 0 in case of error
        std::string ToString() const noexcept; // type conversion done by lua, returns an empty string in case of error
        void* ToUserData() const noexcept; // returns a null pointer if it's not a user data
        template <typename T>
        T To() const noexcept;
        template <typename T>
        T To(T const& defaultValue) const noexcept;
        // unsafe type conversions
        bool CheckBoolean(std::string const& e = "") const; // can throw std::runtime_error
        int CheckInteger(std::string const& e = "") const; // can throw std::runtime_error // actually checks for a number, because an integer is a number for lua
        double CheckNumber(std::string const& e = "") const; // can throw std::runtime_error
        std::string CheckString(std::string const& e = "") const; // can throw std::runtime_error
        void* CheckUserData(std::string const& e = "") const; // can throw std::runtime_error
        template <typename T>
        T Check(std::string const& e) const; // can throw std::runtime_error
        template <typename T>
        T Check() const { return this->Check<T>(""); } // can throw std::runtime_error
        // type tests
        std::string GetTypeName() const noexcept;
        int GetType() const noexcept; // possible values : LUA_TNIL, LUA_TBOOLEAN, LUA_TLIGHTUSERDATA, LUA_TNUMBER, LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, LUA_TTHREAD, LUA_TNONE
        bool Exists() const noexcept; // same as IsNoneOrNil(), use this instead of IsNil()/IsNoneOrNil()/IsNone()
        bool IsBoolean() const noexcept;
        bool IsCFunction() const noexcept;
        bool IsFunction() const noexcept;
        bool IsLightUserData() const noexcept;
        bool IsNil() const noexcept;
        bool IsNone() const noexcept;
        bool IsNoneOrNil() const noexcept;
        bool IsNumber() const noexcept;
        bool IsString() const noexcept;
        bool IsTable() const noexcept;
        bool IsThread() const noexcept;
        bool IsUserData() const noexcept;
        template <typename T>
        bool Is() const noexcept;

        // other stuff (don't call)
        void FromStack() noexcept;
        void ToStack() const noexcept;
        State& GetState() const noexcept{ return this->_state; }
    };

}
