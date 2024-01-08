#include <experimental/meta>
#include <cstdint>
#include <bit>

template<typename = decltype([]{})>
struct Incrementer {
private:
    template<std::size_t, std::size_t> struct Data;

    static consteval std::meta::info get(std::size_t a, std::size_t b){
        auto datai = ^Data;
        auto ai = std::meta::reflect_value(a);
        auto bi = std::meta::reflect_value(b);
        auto classi = std::meta::substitute(datai, {ai, bi});
        return classi;
    }

    static consteval bool test(std::size_t a, std::size_t b){
        return not std::meta::is_incomplete_type(get(a, b));
    }

    static consteval std::size_t find_first_unset(){
        std::size_t exp = 0;
        while (test(0, exp)) ++exp;
        std::size_t res = 0;
        for (; exp != (std::size_t)-1; --exp)
            if (test(res, exp))
                res += (1ull << exp);
        return res;
    }

public:
    static consteval std::size_t get(){
        return find_first_unset();
    }

    static consteval void advance(){
        std::size_t idx = find_first_unset() + 1;
        std::size_t exp = std::countr_zero(idx);
        std::size_t prev = idx ^ (1ull << exp);
        auto classi = get(prev, exp);
        std::meta::define_class(classi, {});
    }
};

    template<typename = decltype([]{})>
    struct MemoryCell {
    private:
        using Inc = Incrementer<>;
        template<std::size_t> struct Map;
        template<std::meta::info> struct Data {};

        static consteval std::meta::info map(){
            std::size_t idx = Inc::get();
            auto idxi = std::meta::reflect_value(idx);
            return std::meta::substitute(^Map, {idxi});
        }

    public:
        static consteval std::meta::info load(){
            auto mapi = map();
            auto wrapper = std::meta::type_of(std::meta::nonstatic_data_members_of(mapi)[0]);
            auto tmp = std::meta::template_arguments_of(wrapper)[0];
            return std::meta::value_of<std::meta::info>(tmp);
        }

        static consteval void store(std::meta::info i){
            Inc::advance();
            auto mapi = map();
            auto wrapper = std::meta::substitute(^Data, {std::meta::reflect_value(i)});
            std::meta::define_class(mapi, {std::meta::nsdm_description(wrapper)});
        }
    };

consteval bool test1(){
    using Cell = MemoryCell<>;
#define STORE_LOAD_TEST(expr)\
    Cell::store(expr);\
    if (Cell::load() != expr)\
        return false;
    STORE_LOAD_TEST(^int);
    STORE_LOAD_TEST(^char);
    STORE_LOAD_TEST(^short);
    return true;
}

static_assert(test1());

template<typename = decltype([]{})>
struct Primes {
private:
    using Cell = MemoryCell<>;

    static consteval bool initialize(){
        Cell::store(std::meta::reflect_value((std::size_t)2));
        return true;
    }

    static_assert(initialize());

    static consteval bool is_prime(std::size_t n){
        for (std::size_t d = 2; d*d <= n; ++d)
            if (n % d == 0)
                return false;
        return true;
    }

public:
    static consteval std::size_t next(){
        std::size_t curr = std::meta::value_of<std::size_t>(Cell::load());
        while (not is_prime(curr))
            ++curr;
        Cell::store(std::meta::reflect_value(curr+1));
        return curr;
    }
};

using P = Primes<>;
static_assert(P::next() == 2);
static_assert(P::next() == 3);
static_assert(P::next() == 5);
static_assert(P::next() == 7);
static_assert(P::next() == 11);
static_assert(P::next() == 13);
static_assert(P::next() == 17);
static_assert(P::next() == 19);
