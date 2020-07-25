// simple memory handler
// author: Ozzie Gómez
#ifndef MEM_ALLOC_HPP_INCLUDED
#define MEM_ALLOC_HPP_INCLUDED
#include <algorithm>
#include <map>
#include <memory>
#include <new>
#include <ranges>
#include <string>
// exceptions
struct exhausted_memory
{
    const std::string msg;
    exhausted_memory()=default;
    explicit exhausted_memory(std::string msg): msg{std::move(msg)}{}
    const std::string& what() const noexcept { return msg; }
}; // sin memoria disponible
class double_detele{};       // operador 'del' sobre un puntero 'invalido'
class null_deref{};    // dereferia de un puntero null
struct ins_error
{
    const std::string msg;
    explicit ins_error(std::string msg): msg{std::move(msg)} {}
    const std::string& what() const noexcept { return msg; }
};


// memory detail
struct mdetail
{
    void* p;
    bool free;
    int sz;
    void* next() const { return static_cast<char*>(p) + sz;}
};


// memory handler
class mem_handler
{
public:
    mem_handler() { reset(); }
    [[nodiscard]] void* alloc(int);
    void dealloc(void*) noexcept;
    int in_use();
    int available();
    bool allocated(void *p)
    {
        return minfo.find(p) != minfo.end();
    }
    bool is_free(void* p) const noexcept { return minfo[p].free; }
    void defrag() noexcept;
    int segments() noexcept { return minfo.size(); }
    char* bbase() noexcept { return &arena[0]; }
    char* ebase() noexcept { return bbase() + len; }
    const auto& mem_info() const noexcept { return minfo; }

    static constexpr int len = 64; // 64 bytes
private:
    void reset() noexcept
    {
        minfo.clear();
        minfo[arena] = mdetail(arena,true,len);
    }
    static inline std::map<const void*, mdetail> minfo{};
    static inline char arena[len]{};
};

//-----------------------------------------------------
int mem_handler::in_use()
{
    int r{};
    for(const auto& [_, s]: minfo)
        r += (!s.free? s.sz: 0);
    return r;
}

int mem_handler::available()
{
    return len - in_use();
}

void mem_handler::defrag() noexcept
{
    auto lo = minfo.begin();
    while(lo != minfo.end() && !lo->second.free) ++lo;
    auto hi = lo;
    if(hi == minfo.end())
        return;

    while(hi != minfo.end() && hi->second.free)
        ++hi;

    if(auto n = distance(lo, hi); 1<n)
    {
        if(2 == n)
        {
            --hi;
            const auto sz = lo->second.sz + hi->second.sz;
            auto md = mdetail(lo->second.p, true, sz);
            minfo.erase(hi->second.p);
            minfo[md.p] = md;
            return;

        }
        // more than two

        auto cpy = minfo;
        auto md = mdetail(lo->second.p, true, lo->second.sz);

        for(auto&& [a, d]: std::ranges::subrange(std::ranges::next(lo, 1), hi))
        {
            md.sz += d.sz;
            cpy.erase(a);
        }
        cpy[lo->second.p] = md;
        minfo = move(cpy);
    }

}

[[nodiscard]] void* mem_handler::alloc(int sz)
{
     for(auto&& [a,i]: minfo)
    {
        // marcar como 'in use'
        if(i.free && sz==i.sz)
        {
            minfo[a].free = false;
            return minfo[a].p;

        }
        // fragmentar
        else if(i.free && sz<i.sz)
        {
            auto d = mdetail(i.p, false, sz);
            minfo[d.next()] = mdetail{d.next(), true, i.sz-sz};
            minfo[a] = d;
            return minfo[a].p;
        }
    }
    return nullptr;
}

void mem_handler::dealloc(void* p) noexcept
{
    if(nullptr==p) return;
    minfo[p].free = true;
}

// instancia del gestionador de memoria
inline mem_handler mem{};

//---------------------------------------------------
// ptr
template<class T>
struct try_deref_ptr
{
    T* data;
    try_deref_ptr(T* d): data{d} {}
    operator T*() const noexcept { return data; }
    T& operator*()
    {
        if(data && !mem.is_free(data))
            return *data;
        throw null_deref{};
    }
};

// Alias
#define Ptr try_deref_ptr

//------------------------ MEMORY ASSIGNMENT OPERATORS --------------------------
template<class T>
concept nonref = (not std::is_reference_v<T>);
template<nonref T>
using ptr_to = std::conditional_t<std::is_array_v<T>, std::decay_t<T>, T*>;
// operator «ins»
template<class T, class... A>
[[nodiscard]] auto ins(A&&... a) -> ptr_to<T>
{
    if constexpr(std::is_array_v<T>)
    {
        using U = std::decay_t<T>;
        if(auto optr = static_cast<U>(mem.alloc(sizeof (T)))){
            ::new(static_cast<void*>(optr)) T{std::forward<A>(a)...};
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


// operator «del»
struct del_oper
{
    del_oper()=default;

    template<class T>
    auto operator,(T ptr)
    {
        if constexpr(std::is_pointer_v<T>)
        {
            if(!mem.allocated(ptr))
                return;
            else if(mem.is_free(ptr))
                throw double_detele{};

            std::destroy_at(ptr);
            mem.dealloc(ptr);
        }
    }

    template<class T>
    auto operator,(try_deref_ptr<T>& p)
    {
        if(auto ptr=static_cast<T*>(p); !mem.allocated(ptr))
            return;
        else if(mem.is_free(ptr))
                throw double_detele{};
        else
        {
            std::destroy_at(ptr);
            mem.dealloc(ptr);
        }
    }
};

// operator 'del'
#define del del_oper{},

template<class T>
struct my_allocator
{
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;

    my_allocator() noexcept = default;
    value_type* allocate(size_t n){ return static_cast<value_type*>(mem.alloc(n)); }
    void deallocate(void* p, [[maybe_unused]] size_t n)
    { mem.dealloc(p); }

    friend operator==(const my_allocator&, const my_allocator&){ return true; }
};

//--------------------------------------------------------------------------------
#endif // MEM_ALLOC_HPP_INCLUDED
