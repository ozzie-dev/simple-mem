// Gestión de memoria simple
// Ozzie Gómez
#include "mem_alloc.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

void mem_summary()
{
    std::cout << "Memoria total: " << mem.len << " bytes\n";
    std::cout << "Memoria disponible: " << mem.available() << " bytes\n";
    std::cout << "Memoria en uso: " << mem.in_use() << " bytes\n";
    std::cout << "Segmentos de memoria: " << mem.segments() << '\n';

    if(1 < mem.segments())
        for(int i=0; const auto& [_, d]: mem.mem_info())
            std::cout << "  segment #" << ++i << ": " << std::setw(3)
                      <<  std::right << d.sz << " bytes"
                      << (d.free? " (*)\n": "\n");
    std::cout << "===========================" << std::endl;

}

class odata
{
public:
    odata()=default;
    odata(const char* n, int i):
        name{ins<std::string>(n)},
        id{i}
        {
            std::cout << __PRETTY_FUNCTION__ << '\n';
        }
    ~odata() noexcept
    {
        std::cout << __PRETTY_FUNCTION__ << '\n';
        std::cout << "Deleting: " + to_s() + "@: " << static_cast<void*>(name) << '\n';
        del name; // operator 'del'
    }
    std::string to_s() const noexcept{ return *name; }
    std::string* name = nullptr;
    int id;


};



template<class T, class... A>
requires (not std::is_reference_v<T>)
[[nodiscard]] auto ins2(A&&... a)
    -> std::conditional_t<std::is_array_v<T>, std::decay_t<T>,
                                        T*>
{
    if constexpr(std::is_array_v<T>)
    {
        using U = std::decay_t<T>;
        if(auto optr = static_cast<U>(mem.alloc(sizeof (T)))){
            ::new(optr) T{std::forward<A>(a)...};
            return optr;
        }
    }
    else
    {
        if(auto optr = static_cast<T*>(mem.alloc(sizeof (T)))){
            std::construct_at(optr, std::forward<A>(a)...);
            return optr;
        }

    }

    throw exhausted_memory{
        "memory_exhausted: \n  disponible: " + std::to_string(mem.available()) +
        "\n  requerida: " + std::to_string(sizeof (T))
    };
};

int main()
{
    auto abc = ins2<int>(42);
    std::cout << *abc << '\n';


}
