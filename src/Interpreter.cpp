#include <iostream>

#include <luasel/Luasel.hpp>

#include "Lua.hpp"

namespace Luasel {

    Interpreter::Interpreter() :
        _serializer(*this)
    {
        this->_state = new State(*this);
        this->_globals = new Ref(*this->_state);
        lua_pushvalue(*this->_state, LUA_GLOBALSINDEX);
        this->_globals->FromStack();
    }

    Interpreter::~Interpreter() noexcept
    {
        delete this->_globals;
        delete this->_state;
    }

    Ref Interpreter::LoadString(std::string const& code) const
    {
        if (luaL_loadstring(*this->_state, code.c_str()))
        {
            std::string e = "Luasel::Interpreter: Cannot load string: ";
            e += lua_tostring(*this->_state, -1);
            lua_pop(*this->_state, 1);
            throw std::runtime_error(e);
        }
        Ref r(*this->_state);
        r.FromStack();
        return r;
    }

    void Interpreter::RegisterLib(LibId lib)
    {
        char const* name;
        lua_CFunction func;
        switch (lib)
        {
        case LibId::Base:
            name = "";
            func = &luaopen_base;
            break;
        case LibId::Math:
            name = LUA_MATHLIBNAME;
            func = &luaopen_math;
            break;
        case LibId::Table:
            name = LUA_TABLIBNAME;
            func = &luaopen_table;
            break;
        case LibId::String:
            name = LUA_STRLIBNAME;
            func = &luaopen_string;
            break;
        case LibId::Io:
            name = LUA_IOLIBNAME;
            func = &luaopen_io;
            break;
        case LibId::Os:
            name = LUA_OSLIBNAME;
            func = &luaopen_os;
            break;
        case LibId::Debug:
            name = LUA_DBLIBNAME;
            func = &luaopen_debug;
            break;
        case LibId::Package:
            name = LUA_LOADLIBNAME;
            func = &luaopen_package;
            break;
        default:
            throw std::runtime_error("Luasel::Interpreter: Lib not found");
        }
        lua_pushcfunction(*this->_state, func);
        lua_pushstring(*this->_state, name);
        if (lua_pcall(*this->_state, 1, 0, 0))
        {
            std::string e = "Luasel::Interpreter: Cannot load lib: ";
            e += lua_tostring(*this->_state, -1);
            lua_pop(*this->_state, 1);
            throw std::runtime_error(e);
        }
        if (lib == LibId::Base)
        {
            auto oldGetmetatable = this->Globals()["getmetatable"];
            this->Globals().Set("getmetatable", this->MakeFunction(
                [oldGetmetatable](CallHelper& helper)
                {
                    auto obj = helper.GetArgList().front();
                    if (obj.IsUserData())
                        throw std::runtime_error("Luasel::Interpreter: getmetatable: expected table");
                    oldGetmetatable.Call(helper);
                }));
        }
    }

    void Interpreter::DoString(std::string const& code) const
    {
        if (luaL_dostring(*this->_state, code.c_str()))
        {
            std::string e = "Luasel::Interpreter: Cannot execute string: ";
            e += lua_tostring(*this->_state, -1);
            lua_pop(*this->_state, 1);
            throw std::runtime_error(e);
        }
    }

    void Interpreter::DoFile(std::string const& path) const
    {
        if (luaL_dofile(*this->_state, path.c_str()))
        {
            std::string e = "Luasel::Interpreter: Cannot execute file: ";
            e += lua_tostring(*this->_state, -1);
            lua_pop(*this->_state, 1);
            throw std::runtime_error(e);
        }
    }

    Ref const& Interpreter::Globals() const noexcept
    {
        return *this->_globals;
    }

    void Interpreter::DumpStack() const noexcept
    {
        std::cout << "------- Lua Stack Dump -------" << std::endl;
        std::cout << "(size: " << lua_gettop(*this->_state) << ")" << std::endl;
        for (int i = 1; i <= lua_gettop(*this->_state); ++i)
        {
            int type = lua_type(*this->_state, i);
            std::cout << i << "\t | " << lua_typename(*this->_state, type);
            switch (type)
            {
                case LUA_TSTRING:
                    std::cout << "\t | \"" << lua_tostring(*this->_state, i) << "\"";
                    break;
                case LUA_TBOOLEAN:
                    std::cout << "\t | " << lua_toboolean(*this->_state, i);
                    break;
                case LUA_TNUMBER:
                    std::cout << "\t | " << lua_tonumber(*this->_state, i);
                    break;
                default:
                    ;
            }
            std::cout << "" << std::endl;
        }
        std::cout << "------------------------------" << std::endl;
    }

}
