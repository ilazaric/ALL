#include <experimental/meta>
#include <map>
#include <iostream>
#include <cassert>
#include <memory>

#include "https://raw.githubusercontent.com/ilazaric/ALL/master/src/edg-reflection/incrementer.hpp"

template<typename = decltype([]{})>
struct TypeList {
private:
    using Inc = ivl::refl::Incrementer<>;
    template<std::size_t> struct Map;

public:

    template<typename T>
    static consteval bool push(){
        auto idx = Inc::get();
        Inc::advance();
        define_class(substitute(^Map, {std::meta::reflect_value(idx)}), {std::meta::nsdm_description(^T)});
        return true;
    }

    static consteval std::size_t length(){return Inc::get();}

    static consteval std::meta::info get(std::size_t idx){
        assert(idx < length());
        auto slot = substitute(^Map, {std::meta::reflect_value(idx)});
        return type_of(nonstatic_data_members_of(slot)[0]);
    }

    static consteval void foreach(auto&& callable){
        auto cnt = Inc::get();
        for (std::size_t idx = 0; idx < cnt; ++idx){
            auto slot = substitute(^Map, {std::meta::reflect_value(idx)});
            callable(type_of(nonstatic_data_members_of(slot)[0]));
        }
    }

    static consteval std::size_t find(std::meta::info i){
        bool found = false;
        std::size_t res = 0;
        std::size_t idx = 0;
        foreach([&](std::meta::info ci){
            if (i == ci){
                found = true;
                res = idx;
            }
            ++idx;
            // assert(idx <= length());
        });
        if (found) return res;
        return length();  
    }
};

struct InvokeState;
void invoke_it(InvokeState&, std::size_t, void*, void*);
struct Callable {
    void* underlying;
    InvokeState* invoke_state;
    using TL = TypeList<>;

    template<typename T>
    Callable(T&&);

    void operator()(auto&& arg) requires(TL::push<decltype(arg)>()){
        auto idx = TL::find(^decltype(arg));
        invoke_it(*invoke_state, idx, underlying, &arg);
    }
};

int main(){
    Callable f = [](auto const& e) { std::cout << __PRETTY_FUNCTION__ << std::endl; };
    f(12);
    f(3.0);
}

template<typename>
InvokeState* create_invoke_state();

template<typename T>
Callable::Callable(T&& underlying) 
: underlying(new std::remove_cvref_t<T> (std::forward<T>(underlying)))
, invoke_state(create_invoke_state<std::remove_cvref_t<T>>()) {}

template<std::size_t Idx, typename Fn>
void fill_vec(std::vector<void(*)(void*, void*)>& vec){
    if constexpr (Idx != 0){
        vec.push_back([](void* a, void* b){
            (*reinterpret_cast<Fn*>(a))(*reinterpret_cast<std::add_pointer_t<typename [:Callable::TL::get(Idx-1):]>>(b));
        });
        fill_vec<Idx-1, Fn>(vec);
    }
}

struct InvokeState {
    std::vector<void(*)(void*, void*)> invokers;
};

void invoke_it(InvokeState& is, std::size_t idx, void* a, void* b){
    assert(idx < is.invokers.size());
    is.invokers[idx](a, b);
}

static_assert(Callable::TL::length() == 2);

template<typename Fn>
InvokeState* create_invoke_state(){
static_assert(Callable::TL::length() == 2);
    auto is = new InvokeState;
    fill_vec<Callable::TL::length(), Fn>(is->invokers);
    return is;
}

