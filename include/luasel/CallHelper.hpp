#ifndef __LUASEL_CALLHELPER_HPP__
#define __LUASEL_CALLHELPER_HPP__

#include <list>
#include <string>
#include <stdexcept>

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
            Interpreter& GetInterpreter() noexcept { return this->_i; }
            // arguments
            void PushArg(Ref const& arg) noexcept;
            Ref PopArg(std::string const& error = "") throw(std::runtime_error);
            std::list<Ref>& GetArgList() noexcept { return this->_args; }
            std::size_t GetNbArgs() const noexcept { return this->_args.size(); }
            void ClearArgs() noexcept;
            // return values
            void PushRet(Ref const& ret) noexcept;
            Ref PopRet(std::string const& error = "") throw(std::runtime_error);
            std::list<Ref>& GetRetList() noexcept { return this->_rets; }
            std::size_t GetNbRets() const noexcept { return this->_rets.size(); }
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
    };

}

#endif
