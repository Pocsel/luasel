#pragma once

#include <luasel/details/Ref.hpp>
#include <luasel/details/State.hpp>

namespace Luasel {

    class WeakRef
    {
        private:
            State& _state;
            unsigned int _id;

        public:
            explicit WeakRef(Ref const& ref) noexcept;
            ~WeakRef();

            WeakRef& operator =(Ref const& ref) noexcept;
            WeakRef& operator =(WeakRef const& ref) noexcept;

            Ref Lock() const noexcept;
    };

}
