#pragma once

#include <list>
#include <stdexcept>
#include <string>
#include <tuple>

namespace Luasel {

    class Interpreter;
    class Ref;

    class CallHelper
    {
    private:
        Interpreter& _i;
        std::list<Ref> _args;
        std::list<Ref> _rets;

    public:
        CallHelper(Interpreter& i) noexcept;
        Interpreter& GetInterpreter() noexcept{ return this->_i; }
            // arguments
        void PushArg(Ref const& arg) noexcept;
        Ref PopArg(std::string const& error = ""); // can throw std::runtime_error
        std::list<Ref>& GetArgList() noexcept{ return this->_args; }
        std::size_t GetNbArgs() const noexcept{ return this->_args.size(); }
        void ClearArgs() noexcept;
        // return values
        void PushRet(Ref const& ret) noexcept;
        Ref PopRet(std::string const& error = ""); // can throw std::runtime_error
        std::list<Ref>& GetRetList() noexcept{ return this->_rets; }
        std::size_t GetNbRets() const noexcept{ return this->_rets.size(); }
        void ClearRets() noexcept;

        // arguments helpers
        template<class T>
        void PushArg(T const& arg) noexcept;
        template<class T>
        void PushArgMove(T&& arg) noexcept;

        // return helpers
        template<class T>
        void PushRet(T const& ret) noexcept;
        template<class T>
        void PushRetMove(T&& ret) noexcept;

        void pushArgs() noexcept {}
        template<class Targ, class... Targs>
        void pushArgs(Targ&& arg, Targs&&... args) noexcept;

        std::tuple<> popArgs() { return std::tuple<>(); }
        template<class Tret, class... Trets>
        std::tuple<Tret, Trets...> popArgs(Tret arg, Trets... args);
        
        template<class TTuple>
        TTuple popArgsTuple()
        {
            return _popArgsTuple(TTuple());
            //return _popArgs<TTuple>()(*this);
        }
        template<>
        inline std::tuple<> popArgsTuple<std::tuple<>>() { return std::tuple<>(); }

    private:
        template<class TR, class... TArgs>
        std::function<TR(TArgs...)> _objectBind(TR(CallHelper::* func)(TArgs...)) {
            return [=](TArgs... args){ return (this->*func)(args...); };
        }

        template<class... TArgs>
        std::tuple<TArgs...> _popArgsTuple(std::tuple<TArgs...>&& args)
        {
            //return helper.popArgs(TArgs()...);
            std::function<std::tuple<TArgs...>(TArgs...)> func = _objectBind(&CallHelper::popArgs<TArgs...>);
            return apply(std::move(func), std::forward<std::tuple<TArgs...>>(args));
        }
    };

}
