
namespace my {
    template<typename T>
    class shared_ptr {
        
        struct Control_Block {
            
            std::size_t cnt_shared;
            std::size_t cnt_weak : 62;
            std::size_t option : 2;
            
            explicit Control_Block(std::size_t option) noexcept
                : cnt_shared(1u),
                cnt_weak(1u),
                option(option)
            {}
            virtual ~Control_Block() = default;
        };


        template<typename U>
        struct Control_Block_Value : Control_Block {
            
            U* value;

            explicit Control_Block_Value(U* p, std::size_t option = 0) noexcept : Control_Block(option), value(p) {}
        };


        template<typename U, typename Deleter>
        struct Control_Block_Deleter : Control_Block_Value<U> {
            
            Deleter del;

            explicit Control_Block_Deleter(U* p, const Deleter& del, std::size_t option = 1u)
                : Control_Block_Value<U>(p, option),
                del(del)
            {}
        };


        template<typename U, typename Deleter, typename Alloc>
        struct Control_Block_Alloc: Control_Block_Deleter<U, Deleter> {
            
            Alloc alloc;
        
            explicit Control_Block_Alloc(U* p, const Deleter& del, const Alloc& alloc)
               : Control_Block_Deleter<U, Deleter>(p, del, 2u),
              alloc(alloc)
            {} 
        };

        T* ptr;
        Control_Block* cb;


    public:

        shared_ptr() noexcept;
        
        template<typename U>
        shared_ptr(U* p);
        
        template<typename U, typename Deleter>
        shared_ptr(U* p, const Deleter& del);

        template<typename U, typename Deleter, typename Alloc>
        shared_ptr(U* p, const Deleter& del, const Alloc& alloc);

        shared_ptr(const shared_ptr& other) noexcept;

        shared_ptr(shared_ptr&& other) noexcept;

        template<typename U>
        shared_ptr(const shared_ptr<U>& other) noexcept;

        template<typename U>
        shared_ptr(shared_ptr<U>&& other) noexcept;

        ~shared_ptr();


        shared_ptr& operator=(const shared_ptr& other) noexcept;

        shared_ptr& operator=(shared_ptr&& other) noexcept;

        template<typename U>
        shared_ptr& operator=(const shared_ptr<U>& other) noexcept;

        template<typename U>
        shared_ptr& operator=(shared_ptr<U>&& other) noexcept;


        //modifiers

        void reset() noexcept;

        template<typename U>
        void reset(U* p);

        template<typename U, typename Deleter>
        void reset(U* p, const Deleter& del);
    
        template<typename U, typename Deleter, typename Alloc>
        void reset(U* p, const Deleter& del, const Alloc& alloc);

        void swap(shared_ptr& other) noexcept;
        
    
        //observers
    
        T* get() noexcept;
        const T* get() const noexcept;

        T& operator*();
        const T& operator*() const;

        T* operator->();
        const T* operator->() const;
    
        std::size_t use_count() const noexcept;

        operator bool() const noexcept;
    
        bool unique() const noexcept;
    
    };


    template<typename T>
    shared_ptr<T>::shared_ptr() noexcept : ptr(nullptr), cb(nullptr) {}

    template<typename T>
    template<typename U>
    shared_ptr<T>::shared_ptr(U* p) : ptr(p), cb(new Control_Block_Value(p)) {}

    template<typename T>
    template<typename U, typename Deleter>
    shared_ptr<T>::shared_ptr(U* p, const Deleter& del) : ptr(p), cb(new Control_Block_Deleter(p,del)) {}

    template<typename T>
    template<typename U, typename Deleter, typename Alloc>
    shared_ptr<T>::shared_ptr(U* p, const Deleter& del, const Alloc& alloc) : ptr(p), cb(nullptr) {
        using CB_Alloc_Type = typename std::allocator_traits<Alloc>::template rebind_alloc<Control_Block_Alloc<U, Deleter, Alloc>>;
        CB_Alloc_Type cb_alloc = alloc;
        cb = std::allocator_traits<CB_Alloc_Type>::allocate(cb_alloc, 1u);
        try {
            std::allocator_traits<CB_Alloc_Type>::construct(cb_alloc, reinterpret_cast<Control_Block_Alloc<U,Deleter,Alloc>*>(cb), p, del, alloc);
        }
        catch(...) {
            std::allocator_traits<CB_Alloc_Type>::deallocate(cb_alloc, reinterpret_cast<Control_Block_Alloc<U, Deleter, Alloc>*>(cb), 1);
            throw;
        }
    }

    template<typename T>
    shared_ptr<T>::shared_ptr(const shared_ptr& other) noexcept : ptr(other.ptr), cb(other.cb) {
        if (other.cb) other.cb->cnt_shared += 1;
    } 

    template<typename T>
    shared_ptr<T>::shared_ptr(shared_ptr&& other) noexcept : ptr(other.ptr), cb(other.cb) {
        other.ptr = nullptr;
        other.cb = nullptr;
    }

    template<typename T>
    template<typename U>
    shared_ptr<T>::shared_ptr(const shared_ptr<U>& other) noexcept 
        : ptr(other.get()), 
        cb(reinterpret_cast<Control_Block*>(*(reinterpret_cast<char*>(&other) + sizeof(other.get())))) 
    {
        if (cb) ++cb->shared_cnt;
    }

    template<typename T>
    template<typename U>
    shared_ptr<T>::shared_ptr(shared_ptr<U>&& other) noexcept 
        : ptr(other.get()),
        cb(reinterpret_cast<Control_Block*>(*(reinterpret_cast<char*>(&other) + sizeof(other.get())))) 
    {
        if(cb) ++cb->cnt_shared;
        other.reset();
    }

    template<typename T>
    shared_ptr& shared_ptr<T>::operator=(const shared_ptr& other) noexcept {
        shared_ptr<T> new_ptr(other);
        swap(new_ptr);
        /*
        if (cb) {
            if(cb->cnt_shared == 1u) {
                delete cb;
            }
            else {
                --cb->cnt_shared;
            }                        
        }
        ptr = other.ptr;
        cb = other.cb;
        if (cb) ++cb->cnt_shared;*/
        return *this;
    }

    template<typename T>
    shared_ptr<T>& shared_ptr<T>::operator=(shared_ptr&& other) noexcept {
        shared_ptr<T> new_ptr(std::move(other));
        swap(new_ptr);
        /*
        if (cb) {
            if (cb->cnt_shared == 1u) {
                delete cb;
            }
            else {
                --cb->cnt_shared;
            }
        }
        
        ptr = other.ptr;
        cb = other.cb;
        
        other.ptr = nullptr;
        other.cb = nullptr;
        */
        return *this;
    }

    template<typename T>
    template<typename U>
    shared_ptr<T>& shared_ptr<T>::operator=(const shared_ptr<U>& other) noexcept {
        shared_ptr<T> new_ptr(other);
        swap(new_ptr);
        /*
        if (cb) {
            if (cb->cnt_shared == 1u) {
                delete cb;
            }
            else {
                --cb->cnt_shared;
            }
        }
        
        ptr = other.get();
        cb = reinterpret_cast<Control_Block*>(*(reinterpret_cast<char*>(&other) + sizeof(other.get())));
        if (cb) ++cb->cnt_shared;*/
        return *this;
    }

    template<typename T>
    template<typename U>
    shared_ptr<T>& shared_ptr<T>::operator=(shared_ptr<U>&& other) noexcept {
        shared_ptr<T> new_ptr(std::move(other));
        swap(new_ptr);
        /*if (cb) {
            if (cb->cnt_shared == 1u) {
                delete cb;
            }
            else {
                --cb->cnt_shared;
            }
        }
        
        ptr = other.get();
        cb = reinterpret_cast<Control_Block*>(*(reinterpret_cast<char*>(&other) + sizeof(other.get())));
        
        other.release();*/

        return *this;
    } 

    template<typename T>
    shared_ptr<T>::~shared_ptr() {
        if (cb) {
            if (cb->cnt_shared == 1u) {
                delete cb;
            }
            else {
                --cb->cnt_shared;
            }
        }
    }


    template<typename T>
    void shared_ptr<T>::reset() noexcept {
        shared_ptr<T> new_ptr;
        swap(new_ptr);
        /*
        if (cb) {
            if (cb->cnt_shared == 1u) {
                delete cb;
            }
            else {
                --cb->cnt_shared;
            }
        }
        ptr = nullptr;
        cb = nullptr;*/
    }

    template<typename T>
    template<typename U>
    void shared_ptr<T>::reset(U* p) {
        shared_ptr<T> new_ptr(p);
        swap(new_ptr);
        /*
        auto* new_cb = new Control_Block_Value(p);
        if (cb) {
            if (cb->cnt->shared == 1u) {
                delete cb;
            }
            else {
                --cb->cnt_shared;
            }
        }
        ptr = p;
        cb = new_cb;*/
    }

    template<typename T>
    template<typename U, typename Deleter>
    void reset(U* p, const Deleter& del) {
        shared_ptr<T> new_ptr(p,del);
        swap(new_ptr);
        /*auto* new_cb = new Control_Block_Deleter(p,del);
        if (cb) {
            if (cb->cnt_shared == 1u) {
                delete cb;
            }
            else {
                --cb->cnt_shared;
            }
        }

        ptr = p;
        cb = new_cb;*/
    }

    template<typename T>
    template<typename U, typename Deleter, typename Alloc>
    void reset(U* p, const Deleter& del, const Alloc& alloc) {
        shared_ptr<T> new_ptr(p,del,alloc);
        swap(new_ptr);
    }

    template<typename T>
    void shared_ptr<T>::swap(shared_ptr& other) noexcept {
        std::swap(ptr, other.ptr);
        std::swap(cb, other.cb);
    }

    template<typename T>
    T& shared_ptr<T>::operator*() {
        return *ptr;
    }

    template<typename T>
    const T& shared_ptr<T>::operator*() const {
        return *ptr;
    }

    template<typename T>
    T* shared_ptr<T>::operator->() {
        return ptr;
    }

    template<typename T>
    const T* shared_ptr<T>::operator->() const {
        return ptr;
    }

    template<typename T>
    std::size_t shared_ptr<T>::use_count() const noexcept {
        return cb->cnt_shared;
    }

    template<typename T>
    bool shared_ptr<T>::unique() const noexcept {
        return (cb && (cb->cnt_shared == 1u));
    }

    template<typename T>
    shared_ptr<T>::operator bool() const noexcept {
        return ptr != nullptr;
    }


};




















