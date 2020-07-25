// Gestión de memoria simple
// Ozzie Gómez
#include "mem_alloc.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

void mem_summary()
{
    std::cout << "===========================\n";
    std::cout << "Memoria total: " << mem.len << " bytes\n";
    std::cout << "Memoria disponible (*): " << mem.available() << " bytes\n";
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

void g(Ptr<auto> ptr){ del ptr; }

void f()
{
    Ptr<int> ptr = nullptr;
    try
    {
        // trata de asignar memoria lo que podría lanzar std::bad_alloc
        ptr = ins<int>();
        mem_summary();
        Ptr ptr2 = ptr; // code smell
        g(ptr); // bad!
        mem_summary();
        Ptr abc = ins<char[4]>('a','b','c','\0');
        mem_summary();
        std::cout << abc << '\n';
        *ptr2 = 1234; // ouch!
        std::cout << *reinterpret_cast<int*>(static_cast<char*>(abc)) << '\n';
    }
    catch(exhausted_memory) { throw; }
    // alguna excepción posiblemente generada por código luego de la
    // asignación
    catch(...){ del ptr; throw; }
    // ... posiblemente más código
    del ptr;
    mem_summary();
}


int main()
{
    

}
// Gestión de memoria simple
// Ozzie Gómez
#include "mem_alloc.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

void mem_summary()
{
    std::cout << "===========================\n";
    std::cout << "Memoria total: " << mem.len << " bytes\n";
    std::cout << "Memoria disponible (*): " << mem.available() << " bytes\n";
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

void g(Ptr<auto> ptr){ del ptr; }

void f()
{
    Ptr<int> ptr = nullptr;
    try
    {
        // trata de asignar memoria lo que podría lanzar std::bad_alloc
        ptr = ins<int>();
        mem_summary();
        Ptr ptr2 = ptr; // code smell
        g(ptr); // bad!
        mem_summary();
        Ptr abc = ins<char[4]>('a','b','c','\0');
        mem_summary();
        std::cout << abc << '\n';
        *ptr2 = 1234; // ouch!
        std::cout << *reinterpret_cast<int*>(static_cast<char*>(abc)) << '\n';
    }
    catch(exhausted_memory) { throw; }
    // alguna excepción posiblemente generada por código luego de la
    // asignación
    catch(...){ del ptr; throw; }
    // ... posiblemente más código
    del ptr;
    mem_summary();
}


int main()
{
    

}
