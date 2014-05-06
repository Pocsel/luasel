#ifndef __LUASEL_STATE_HPP__
#define __LUASEL_STATE_HPP__

#include <functional>
#include <stdexcept>
#include <unordered_map>
#include <boost/noncopyable.hpp>

extern "C" {
    struct lua_State;
}

namespace Luasel {

    class Interpreter;
    class Ref;
    class CallHelper;
    class MetaTable;

    class State :
        private boost::noncopyable
    {
        public:
            struct ClosureEnv
            {
                Interpreter* i;
                std::function<void(CallHelper&)>* f;
            };

        private:
            Interpreter& _interpreter;
            lua_State* _state;
            std::unordered_map<std::size_t, MetaTable*> _metaTables;
            Ref* _weakTable;
            unsigned int _weakId;
            bool _garbageCollectionEnabled;

        public:
            State(Interpreter& interpreter); // can throw std::runtime_error
            ~State();
            bool GetGarbageCollectionEnabled() const noexcept { return this->_garbageCollectionEnabled; }
            void StopGarbageCollector() noexcept;
            void RestartGarbageCollector() noexcept;
            MetaTable& RegisterMetaTable(MetaTable&& metaTable, std::size_t hash) noexcept;
            Ref MakeBoolean(bool val) noexcept;
            Ref MakeFunction(std::function<void(CallHelper&)> val) noexcept;
            Ref MakeNil() noexcept;
            Ref MakeInteger(int val) noexcept;
            Ref MakeNumber(double val) noexcept;
            Ref MakeString(std::string const& val) noexcept;
            Ref MakeTable() noexcept;
            Ref MakeUserData(void** data, size_t size); // can throw std::runtime_error
            template<typename T>
                Ref Make(T const& val) noexcept;
            operator lua_State*() const noexcept { return this->_state; }
            Interpreter& GetInterpreter() noexcept { return this->_interpreter; }
            MetaTable const& GetMetaTable(std::size_t hash); // can throw std::runtime_error
            Ref GetWeakReference(unsigned int id) const;
            unsigned int GetWeakReference(Ref const& ref);
    };

    template<> Ref State::Make<bool>(bool const& val) noexcept;
    template<> Ref State::Make<int>(int const& val) noexcept;
    template<> Ref State::Make<unsigned int>(unsigned int const& val) noexcept;
    template<> Ref State::Make<char>(char const& val) noexcept;
    template<> Ref State::Make<unsigned char>(unsigned char const& val) noexcept;
    template<> Ref State::Make<double>(double const& val) noexcept;
    template<> Ref State::Make<float>(float const& val) noexcept;
    template<> Ref State::Make<std::string>(std::string const& val) noexcept;
    template<> Ref State::Make<char const*>(char const* const& val) noexcept;
    template<> Ref State::Make<std::function<void(CallHelper&)>>(std::function<void(CallHelper&)> const& val) noexcept;

    template<> Ref State::Make<Ref>(Ref const& val) noexcept;

}

#endif
