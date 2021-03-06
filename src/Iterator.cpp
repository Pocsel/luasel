#include <luasel/Luasel.hpp>
#include "Lua.hpp"

namespace Luasel {

    Iterator::Iterator(Ref const& table, bool end) :
        _table(table), _key(table.GetState()), _value(table.GetState()), _end(end)
    {
        if (!this->_end)
            ++(*this);
    }

    bool Iterator::operator ==(Iterator const& iterator) const noexcept
    {
        return this->_end && iterator.IsEnd();
        //return iterator.GetTable() == this->_table && iterator.GetKey() == this->_key;
    }

    bool Iterator::operator !=(Iterator const& iterator) const noexcept
    {
        return !(*this == iterator);
    }

    Iterator& Iterator::operator ++()
    {
        if (this->_end)
            throw std::runtime_error("Luasel::Iterator: Incrementing end iterator");
        this->_table.ToStack();
        if (!lua_istable(this->_table.GetState(), -1))
        {
            lua_pop(this->_table.GetState(), 1);
            throw std::runtime_error("Luasel::Iterator: Iterating a value that is not a table");
        }
        this->_key.ToStack();
        if (lua_next(this->_table.GetState(), -2))
        {
            this->_value.FromStack();
            this->_key.FromStack();
        }
        else
        {
            this->_end = true;
            this->_key.Unref();
            this->_value.Unref();
        }
        lua_pop(this->_table.GetState(), 1);
        return *this;
    }

}
