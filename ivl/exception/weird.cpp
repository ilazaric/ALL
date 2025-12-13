#include <cstdint>
#include <print>
#include <vector>
#include <exception>
#include <cstring>

// dunno how to include unwind-cxx.h for __cxa_refcounted_exception definition :'( 
constexpr size_t sizeof_cxa_refcounted_exception = 0x80;
constexpr size_t alignof_cxa_refcounted_exception = 0x8;

extern "C" {
void* __cxxabiv1::__cxa_allocate_exception(std::size_t thrown_size) noexcept {
    size_t alignment = std::max(alignof_cxa_refcounted_exception, 1uz << std::countr_zero(thrown_size));
    size_t front_padding = alignment - sizeof_cxa_refcounted_exception % alignment;
    thrown_size += sizeof_cxa_refcounted_exception;
    thrown_size += front_padding;
    // void* ret = malloc(thrown_size);
    void* ret = aligned_alloc(alignment, thrown_size);
    if (!ret) std::terminate();
    memset(ret, 0, thrown_size);
    *(void**)((char*)ret + front_padding - 8) = ret;
    return (void*)((char*)ret + sizeof_cxa_refcounted_exception + front_padding);
}

void __cxxabiv1::__cxa_free_exception(void* vptr) noexcept {
    char *ptr = (char *) vptr - sizeof_cxa_refcounted_exception;
    free(*(void**)(ptr - 8));
}
}

template<size_t SIZE, size_t ALIGNMENT>
requires (SIZE > 0 && std::popcount(ALIGNMENT) == 1 && SIZE % ALIGNMENT == 0)
struct alignas(ALIGNMENT) GenericException {
    char data[SIZE];
};

bool check_alignment(const auto& x){
    return reinterpret_cast<uintptr_t>(&x) % alignof(x) == 0;
}

template<typename E>
void run_experiment(){
    size_t success_count = 0;
    size_t trial_count = 0;
    // Prevent freeing exceptions early to get more varied allocation behaviour.
    std::vector<std::exception_ptr> exceptions;
    for (int i = 0; i < 10000; ++i){
        try {
            throw E{};
        } catch (const E& e) {
            ++trial_count;
            success_count += check_alignment(e);
            exceptions.emplace_back(std::current_exception());
        }
    }

    std::println("{} {} -> {:.2f}% {} {}", sizeof(E), alignof(E), (double)success_count / trial_count * 100, success_count, trial_count);
}

int main() {
    std::println("max alignment: {}", alignof(std::max_align_t));
    run_experiment<GenericException<16, 16>>();
    run_experiment<GenericException<32, 32>>();
    run_experiment<GenericException<64, 64>>();
}
