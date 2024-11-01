#include "shared_ptr.h"

namespace my {

    template<typename T>
    class weak_ptr {

        T* ptr;
        typename my::shared_ptr<T>::Control_Block* cb;
    
    public:
        weak_ptr() noexcept;
        
        template<typename U>
        weak_ptr(const shared_ptr<U>& sptr) noexcept;
        
        weak_ptr(const weak_ptr& other) noexcept;
        
        weak_ptr(weak_ptr&& other) noexcept;
        
        template<typename U>
        weak_ptr(const weak_ptr<U>& other) noexcept;

        template<typename U>
        weak_ptr(weak_ptr<U>&& other) noexcept;

        ~weak_ptr();

        //assignment operators

        template<typename U>
        weak_ptr& operator=(const shared_ptr<U>& sptr) noexcept;
        
        weak_ptr& operator=(const weak_ptr& other) noexcept;
        
        weak_ptr& operator=(weak_ptr&& other) noexcept;
        
        template<typename U>
        weak_ptr& operator=(const weak_ptr<U>& other) noexcept;

        template<typename U>
        weak_ptr& operator=(weak_ptr<U>&& other) noexcept;

         //modifiers
        
        void reset() noexcept;

        void swap(weak_ptr& other) noexcept;

        //observers

        std::size_t use_count() const noexcept;

        bool expired() const noexcept;

        my::shared_ptr<T> lock() const noexcept;
    
    };


    template<typename T>
    weak_ptr<T>::weak_ptr() noexcept : ptr(nullptr), cb(nullptr) {}

    template<typename T>
    template<typename U>
    weak_ptr<T>::weak_ptr(const shared_ptr<U>& sptr) noexcept : ptr(sptr.ptr), cb(ptr.cb) {
        if (cb) ++cb->cnt_weak;
    }

    template<typename T>
    weak_ptr<T>::weak_ptr(const weak_ptr& other) noexcept : ptr(other.ptr), cb(other.cb) {
        if (cb) ++cb->cnt_weak;
    }

    template<typename T>
    weak_ptr<T>::weak_ptr(weak_ptr&& other) noexcept : ptr(other.ptr), cb(other.cb) {
        other.ptr = nullptr;
        other.cb = nullptr;
    }

    template<typename T>
    template<typename U>
    weak_ptr<T>::weak_ptr(const weak_ptr<U>& other) noexcept : weak_ptr(other.lock()) {}

    template<typename T>
    template<typename U>
    weak_ptr<T>::weak_ptr(weak_ptr<U>&& other) noexcept : weak_ptr(other.lock()) {
        other.reset();
    }

    template<typename T>
    weak_ptr<T>::~weak_ptr() {
        if (cb) {
            if ((cb->cnt_shared == 0u) && (cb->cnt_weak == 1u)) {
                cb->delete_control_block();
            }
            else {
                --cb->cnt_weak;
            }
        }
    }

    
    template<typename T>
    template<typename U>
    weak_ptr<T>& weak_ptr<T>::operator=(const shared_ptr<U>& sptr) noexcept {
        weak_ptr new_weak_ptr(sptr);
        swap(new_weak_ptr);
        return *this;
    }

    template<typename T>
    weak_ptr<T>& weak_ptr<T>::operator=(const weak_ptr& other) noexcept {
        weak_ptr new_weak_ptr(other);
        swap(new_weak_ptr);
        return *this;
    }

    template<typename T>
    weak_ptr<T>& weak_ptr<T>::operator=(weak_ptr&& other) noexcept {
        weak_ptr new_weak_ptr(std::move(other));
        swap(new_weak_ptr);
        return *this;
    }

    template<typename T>
    template<typename U>
    weak_ptr<T>& weak_ptr<T>::operator=(const weak_ptr<U>& other) noexcept {
        weak_ptr new_weak_ptr(other);
        swap(new_weak_ptr);
        return *this;
    }

    template<typename T>
    template<typename U>
    weak_ptr<T>& weak_ptr<T>::operator=(weak_ptr<U>&& other) noexcept {
        weak_ptr new_weak_ptr(std::move(other));
        swap(new_weak_ptr);
        return *this;
    }


     //modifiers
        
    template<typename T>
    void weak_ptr<T>::reset() noexcept {
        weak_ptr new_weak_ptr;
        swap(new_weak_ptr);
    }

    template<typename T>
    void weak_ptr<T>::swap(weak_ptr& other) noexcept {
        std::swap(ptr, other.ptr);
        std::swap(cb, other.cb);
    }

    //observers

    template<typename T>
    std::size_t weak_ptr<T>::use_count() const noexcept {
        return cb ? cb->cnt_shared : 0;
    }

    template<typename T>
    bool weak_ptr<T>::expired() const noexcept {
        return use_count() == 0;
    }

    template<typename T>
    my::shared_ptr<T> weak_ptr<T>::lock() const noexcept {
        return expired()? shared_ptr<T>() : shared_ptr<T>(*this);
    }

};

















