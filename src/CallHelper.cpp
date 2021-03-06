#include <luasel/Luasel.hpp>

namespace Luasel {

    CallHelper::CallHelper(Interpreter& i) noexcept :
        _i(i)
    {
    }

    void CallHelper::PushArg(Ref const& arg) noexcept
    {
        this->_args.push_front(arg);
    }

    Ref CallHelper::PopArg(std::string const& error /* = "" */)
    {
        if (this->_args.empty())
        {
            if (!error.empty())
                throw std::runtime_error(error);
            else
                throw std::runtime_error("Luasel::CallHelper: Missing argument");
        }
        Ref arg = this->_args.front();
        this->_args.pop_front();
        return arg;
    }

    void CallHelper::ClearArgs() noexcept
    {
        this->_args.clear();
    }

    void CallHelper::PushRet(Ref const& ret) noexcept
    {
        this->_rets.push_front(ret);
    }

    Ref CallHelper::PopRet(std::string const& error /* = "" */)
    {
        if (this->_rets.empty())
        {
            if (!error.empty())
                throw std::runtime_error(error);
            else
                throw std::runtime_error("Luasel::CallHelper: Missing return value");
        }
        Ref ret = this->_rets.front();
        this->_rets.pop_front();
        return ret;
    }

    void CallHelper::ClearRets() noexcept
    {
        this->_rets.clear();
    }

}
