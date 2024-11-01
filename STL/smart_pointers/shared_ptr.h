#include <exception>



namespace my {

    struct empty_weak_ptr_exception : std::exception {
        const char* what() const noexcept override {
            return "shared_ptr cannot be constructed from an empty weak_ptr!";
        }        
    };

    template<typename T>
    class weak_ptr;

    template<typename T>
    class shared_ptr {
        

        template<typename U>
        friend class my::weak_ptr;

        struct Control_Block {
            
            std::size_t cnt_shared;
            std::size_t cnt_weak : 62;
            
            explicit Control_Block() noexcept
                : cnt_shared(1u),
                cnt_weak(0u)
            {}

            /*virtual bool has_alloc() const noexcept {
                return false;
            }*/

            virtual void delete_control_block() noexcept {
                delete this;
            }

            virtual void destroy_object() noexcept = 0;

            virtual ~Control_Block() = default;
        };


        template<typename U>
        struct Control_Block_Value : Control_Block {
            
            U* value;

            explicit Control_Block_Value(U* p) noexcept : value(p) {}
            
            void destroy_object() noexcept override {
                delete value;
            }

        };


        template<typename U, typename Deleter>
        struct Control_Block_Deleter : Control_Block_Value<U> {
            
            Deleter del;

            explicit Control_Block_Deleter(U* p, const Deleter& del)
                : Control_Block_Value<U>(p),
                del(del)
            {}

            void destroy_object() noexcept override {
                del(Control_Block_Value<U>::value);
            }

        };


        template<typename U, typename Deleter, typename Alloc>
        struct Control_Block_Alloc: Control_Block_Deleter<U, Deleter> {
            
            Alloc alloc;
        
            explicit Control_Block_Alloc(U* p, const Deleter& del, const Alloc& alloc)
               : Control_Block_Deleter<U, Deleter>(p, del),
              alloc(alloc)
            {}

           /*bool has_alloc() const noexcept override {
               return true;
           }*/

        private:
            
            static void delete_control_block_helper(Control_Block_Alloc* control_block) noexcept {
                using CB_Alloc_Type  = typename std::allocator_traits<Alloc>::template rebind_alloc<Control_Block_Alloc>;
                CB_Alloc_Type cb_alloc = control_block->alloc;
                std::allocator_traits<CB_Alloc_Type>::destroy(cb_alloc, control_block);
                std::allocator_traits<CB_Alloc_Type>::deallocate(cb_alloc, control_block, 1);
            }

        public:
           
            void delete_control_block() noexcept override {
                delete_control_block_helper(this);
            }

        };


        template<typename Alloc>
        struct Control_Block_Alloc_Shared : Control_Block {
            
            Alloc alloc;
            
            explicit Control_Block_Alloc_Shared(const Alloc& alloc) : alloc(alloc) {}
            
            /*bool has_alloc() const noexcept override {
                return true;
            }*/

        private:
        
            static void delete_control_block_helper(Control_Block_Alloc_Shared* control_block) noexcept {
                using CB_Alloc_Type = typename std::allocator_traits<Alloc>::template rebind_alloc<Alloc_Shared_Helper_Struct<Alloc>>;
                CB_Alloc_Type cb_alloc = control_block->alloc;
                std::allocator_traits<CB_Alloc_Type>::destroy(cb_alloc, control_block);
                std::allocator_traits<CB_Alloc_Type>::deallocate(cb_alloc, static_cast<Alloc_Shared_Helper_Struct<Alloc>*>(control_block), 1);
            }

        public:

            void delete_control_block() noexcept override {
                delete_control_block_helper(this);
            }    

            void destroy_object() noexcept override {
                std::allocator_traits<Alloc>::destroy(alloc, reinterpret_cast<T*>(reinterpret_cast<char*>(this) + sizeof(*this))); // fix this later
            }
        
        };





        template<typename Alloc>
        struct Alloc_Shared_Helper_Struct : Control_Block_Alloc_Shared<Alloc> {
            T value;
        };

        

        struct Make_Shared_CB : Control_Block {
            
            explicit Make_Shared_CB() noexcept = default;
            
            void destroy_object() noexcept override {
                reinterpret_cast<T*>(reinterpret_cast<char*>(this) + sizeof(*this))->~T(); // fix this later
            }

        };
  

        T* ptr;
        Control_Block* cb;

        
        template<typename... Args>
        shared_ptr(Args&&... args) : ptr(nullptr), cb(static_cast<Control_Block*>(::operator new(sizeof(Make_Shared_CB) +sizeof(T)))) {
            ptr = static_cast<T*>(static_cast<char*>(cb) + sizeof(Make_Shared_CB));
            try {
                new(ptr) T(std::forward<Args>(args)...);
            }
            catch(...) {
                ::operator delete(cb, sizeof(Make_Shared_CB) + sizeof(T));
                throw;
            }
            new(static_cast<Make_Shared_CB*>(cb)) Make_Shared_CB();  
        }

        template<typename Alloc, typename... Args>
        shared_ptr(const Alloc& alloc, Args&&... args) : ptr(nullptr), cb(nullptr) {            
            
            using CB_Alloc_Type = typename std::allocator_traits<Alloc>::template rebind_alloc<Alloc_Shared_Helper_Struct<Alloc>>;
            
            CB_Alloc_Type cb_alloc = alloc;
            
            Control_Block_Alloc_Shared<Alloc>* temp_cb = std::allocator_traits<CB_Alloc_Type>::allocate(cb_alloc, 1);
            
            try {
                std::allocator_traits<CB_Alloc_Type>::construct(cb_alloc, temp_cb, alloc);
                try {
                    std::allocator_traits<Alloc>::construct(temp_cb->alloc, &static_cast<Alloc_Shared_Helper_Struct<Alloc>*>(temp_cb)->value, std::forward<Args>(args)...);
                }
                catch(...) {
                    std::allocator_traits<CB_Alloc_Type>::destroy(cb_alloc, temp_cb);
                    throw;
                }
            }
            catch(...) {
                std::allocator_traits<CB_Alloc_Type>::deallocate(cb_alloc, static_cast<Alloc_Shared_Helper_Struct<Alloc>*>(temp_cb), 1);
                throw;
            }
            
            cb = temp_cb;
            ptr = &static_cast<Alloc_Shared_Helper_Struct<Alloc>*>(temp_cb)->value;
        }

    public:

        template<typename U, typename... Args>
        friend shared_ptr<U> make_shared(Args&&... args);

        template<typename U, typename Alloc, typename... Args>
        friend shared_ptr<U> allocate_shared(const Alloc& alloc, Args&&... args);
        
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

        template<typename U>
        shared_ptr(const weak_ptr<U>& wptr);
        
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
    template<typename U>
    shared_ptr<T>::shared_ptr(const weak_ptr<U>& wptr) : ptr(wptr.ptr), cb(wptr.cb) {
        if (wptr.use_count() == 0u) {
            throw my::empty_weak_ptr_exception{};
        }
    }



    template<typename T>
    shared_ptr<T>& shared_ptr<T>::operator=(const shared_ptr& other) noexcept {
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
            if (--cb->cnt_shared == 0u) {

                cb->destroy_object();
                
                if (cb->cnt_weak == 0u) {
                    cb->delete_control_block();
                }
            
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
    void shared_ptr<T>::reset(U* p, const Deleter& del) {
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
    void shared_ptr<T>::reset(U* p, const Deleter& del, const Alloc& alloc) {
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


    template<typename T, typename... Args>
    shared_ptr<T> make_shared(Args&&... args) {
        return shared_ptr<T>(std::forward<Args>(args)...);        
    }

    template<typename T, typename Alloc, typename... Args>
    shared_ptr<T> allocate_shared(const Alloc& alloc, Args&&... args) {
        return shared_ptr<T>(alloc, std::forward<Args>(args)...);        
    }

};




















