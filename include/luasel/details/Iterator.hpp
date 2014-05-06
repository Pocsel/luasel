#pragma once

#include <luasel/details/Ref.hpp>

namespace Luasel {

    class Iterator
    {
        private:
            Ref _table;
            Ref _key;
            Ref _value;
            bool _end;

        public:
            Iterator(Ref const& table, bool end); // can throw std::runtime_error
            bool operator ==(Iterator const& iterator) const noexcept; // iterators are equal ONLY if they are end iterators
            bool operator !=(Iterator const& iterator) const noexcept;
            Iterator& operator ++(); // can throw std::runtime_error
            bool IsEnd() const noexcept { return this->_end; }
            Ref const& GetTable() const noexcept { return this->_table; }
            Ref const& GetKey() const noexcept { return this->_key; }
            Ref const& GetValue() const noexcept { return this->_value; }
    };

}
