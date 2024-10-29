#include <memory>
#include <type_traits>

namespace my {
    template<typename T, typename Deleter = std::default_delete<T>>
    class unique_ptr {
        
        T* ptr;
        Deleter del;
        
    public:
        //constructors
        unique_ptr() noexcept(std::is_nothrow_default_constructible_v<Deleter>);
        
        unique_ptr(T* p) noexcept(std::is_nothrow_default_constructible_v<Deleter>);
        
        unique_ptr(T* p, const Deleter& d) noexcept(std::is_nothrow_copy_constructible_v<Deleter>); // we accept a deleter by const lvalue reference here to show that we're not gonna change it 
        
        unique_ptr(T* p, Deleter&& d) noexcept(std::is_nothrow_move_constructible_v<Deleter>);

        unique_ptr(const unique_ptr& other) = delete;
        
        template<typename U, typename D>
        unique_ptr(unique_ptr<U,D>&& other) noexcept(std::is_nothrow_move_constructible_v<Deleter>); // type U should have a virtual destructor if U != T
        
        unique_ptr(unique_ptr&& other) noexcept(std::is_nothrow_move_constructible_v<Deleter>);
       
        //assignment operators
        unique_ptr& operator=(const unique_ptr& other) = delete;
        
        template<typename U, typename D>
        unique_ptr& operator=(unique_ptr<U, D>&& other) noexcept(std::is_nothrow_move_assignable_v<Deleter>);
        
        unique_ptr& operator=(unique_ptr&& other) noexcept(std::is_nothrow_move_assignable_v<Deleter>);
        
        
        //modifiers
        T* release() noexcept;
        void reset(T* p = nullptr) noexcept;
        void swap(unique_ptr& other) noexcept(std::is_nothrow_swappable_v<Deleter>);

        //observers
        T* get() noexcept;
        const T* get() const noexcept;
        
        Deleter& get_deleter() noexcept;
        const Deleter& get_deleter() const noexcept;
        
        operator bool() const noexcept;
        
        T& operator*();
        const T& operator*() const;
        
        T* operator->();
        const T* operator->() const;

        //comparison operators        
        bool operator==(const unique_ptr& other) const noexcept;
        bool operator!=(const unique_ptr& other) const noexcept;
        bool operator>(const unique_ptr& other) const noexcept;
        bool operator<(const unique_ptr& other) const noexcept;
        bool operator>=(const unique_ptr& other) const noexcept;
        bool operator<=(const unique_ptr& other) const noexcept;


        //destructor
        ~unique_ptr();
    };


    //constructors

    template<typename T, typename Deleter>
    unique_ptr<T, Deleter>::unique_ptr() 
        noexcept(std::is_nothrow_default_constructible_v<Deleter>) 
        : ptr(nullptr) 
    {}

    template<typename T, typename Deleter>
    unique_ptr<T, Deleter>::unique_ptr(T* p) 
        noexcept(std::is_nothrow_default_constructible_v<Deleter>) 
        : ptr(p) 
    {}

    template<typename T, typename Deleter>
    unique_ptr<T, Deleter>::unique_ptr(T* p, const Deleter& d) 
        noexcept(std::is_nothrow_copy_constructible_v<Deleter>) 
        : ptr(p), 
        del(d) 
    {}

    template<typename T, typename Deleter>
    unique_ptr<T, Deleter>::unique_ptr(T* p, Deleter&& d) 
        noexcept(std::is_nothrow_move_constructible_v<Deleter>) 
        : ptr(p), 
        del(std::move(d)) 
    {}

    template<typename T, typename Deleter>
    unique_ptr<T, Deleter>::unique_ptr(unique_ptr&& other) 
        noexcept(std::is_nothrow_move_constructible_v<Deleter>) 
        : ptr(other.release()), 
        del(std::move(other.del)) 
    {}

    template<typename T, typename Deleter> 
    template<typename U, typename D>
    unique_ptr<T, Deleter>::unique_ptr(unique_ptr<U, D>&& other) 
        noexcept(std::is_nothrow_move_constructible_v<Deleter>) 
        : ptr(other.release()), 
        del(std::move(other.get_deleter()))
    {}


    //assignment operators

    template<typename T, typename Deleter>
    unique_ptr<T,Deleter>& unique_ptr<T,Deleter>::operator=(unique_ptr&& other) noexcept(std::is_nothrow_move_assignable_v<Deleter>) {
        //static_assert(std::is_nothrow_move_assignable_v<Deleter>); may be added to provide exception safety
        if (other != *this) {
            reset(other.release());
            if (del != other.del) del = std::move(other.del);
        }
        return *this;
    }

    template<typename T, typename Deleter>
    template<typename U, typename D>
    unique_ptr<T, Deleter>& unique_ptr<T,Deleter>::operator=(unique_ptr<U, D>&& other) noexcept(std::is_nothrow_move_assignable_v<Deleter>) {
            reset(other.release());
            if (del != other.get_deleter()) del = std::move(other.del);
            return *this;
    }

    //destructor

    template<typename T, typename Deleter>
    unique_ptr<T, Deleter>::~unique_ptr() {
        if (ptr) del(ptr);
    }
    

    //modifiers

    template<typename T, typename Deleter>
    T* unique_ptr<T, Deleter>::release() noexcept {
        T* value = ptr;
        ptr = nullptr;
        return value;
    }

    template<typename T, typename Deleter>
    void unique_ptr<T, Deleter>::reset(T* p) noexcept {
        if (ptr) {
            del(ptr);
        }
        ptr = p;
    }

    template<typename T, typename Deleter>
    void unique_ptr<T, Deleter>::swap(unique_ptr& other) noexcept(std::is_nothrow_swappable_v<Deleter>) {
        std::swap(ptr, other.ptr);
        if (del != other.del) {
            std::swap(del, other.del);
        }
    } 

    //observers

    template<typename T, typename Deleter>
    T* unique_ptr<T, Deleter>::get() noexcept {
        return ptr;
    }

    template<typename T, typename Deleter>
    const T* unique_ptr<T, Deleter>::get() const noexcept {
        return ptr;
    }

    template<typename T, typename Deleter>
    Deleter& unique_ptr<T, Deleter>::get_deleter() noexcept {
        return del;
    }
    
    template<typename T, typename Deleter>
    const Deleter& unique_ptr<T, Deleter>::get_deleter() const noexcept {
        return del;
    }

    template<typename T, typename Deleter>
    unique_ptr<T, Deleter>::operator bool() const noexcept {
        return (ptr != nullptr);
    }

    template<typename T, typename Deleter>
    T& unique_ptr<T, Deleter>::operator*() {
        return *ptr;
    }

    template<typename T, typename Deleter>
    const T& unique_ptr<T, Deleter>::operator*() const {
        return *ptr;
    }
    
    template<typename T, typename Deleter>
    T* unique_ptr<T, Deleter>::operator->() {
        return ptr;
    }

    template<typename T, typename Deleter>
    const T* unique_ptr<T, Deleter>::operator->() const {
        return ptr;
    }


    //comparison operators

    template<typename T, typename Deleter>
    bool unique_ptr<T, Deleter>::operator==(const unique_ptr& other) const noexcept {
        return (ptr == other.ptr);
    }
    
    template<typename T, typename Deleter>
    bool unique_ptr<T, Deleter>::operator!=(const unique_ptr& other) const noexcept {
        return (ptr != other.ptr);
    }

    template<typename T, typename Deleter>
    bool unique_ptr<T, Deleter>::operator>(const unique_ptr& other) const noexcept {
        return (ptr > other.ptr);
    }

    template<typename T, typename Deleter>
    bool unique_ptr<T, Deleter>::operator<(const unique_ptr& other) const noexcept {
        return (ptr < other.ptr);
    }

    template<typename T, typename Deleter>
    bool unique_ptr<T, Deleter>::operator>=(const unique_ptr& other) const noexcept {
        return (ptr >= other.ptr);
    }

    template<typename T, typename Deleter>
    bool unique_ptr<T, Deleter>::operator<=(const unique_ptr& other) const noexcept {
        return (ptr <= other.ptr);
    }

    //we don't need a custom deleter, because an object is constructed within this function using standard methods(calls 'new' operator)
    template<typename T, typename... Args>
    unique_ptr<T> make_unique(Args&&... args) {
        T* ptr = new T(std::forward<Args>(args)...);
        return unique_ptr<T>(ptr);
    }

};












