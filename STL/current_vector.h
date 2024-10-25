#include <cassert>
#include <exception>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace my {
    template<typename T, typename Alloc = std::allocator<T>>
    class vector {
        Alloc alloc;
        T* arr;
        std::size_t cap;
        std::size_t sz;

        template<bool isConst>
        class common_iterator {
            std::conditional_t<isConst, const T*, T*> p;
            friend class vector<T, Alloc>;
        public:

            using difference_type = std::ptrdiff_t;
            using value_type = std::conditional_t<isConst, const T, T>;
            using pointer = std::conditional_t<isConst, const T*, T*>;
            using reference = std::conditional_t<isConst, const T&, T&>;
            using iterator_category = std::random_access_iterator_tag;

        private:
            common_iterator(std::conditional_t<isConst, const T*, T*> p) noexcept : p(p) {}
        public:    
            common_iterator(const common_iterator& other) noexcept : p(other.p) {}

            common_iterator operator++(int) noexcept {
                common_iterator cp = *this;
                ++p;
                return cp;
            }
            common_iterator& operator++() noexcept {
                ++p;
                return *this;
            }
            common_iterator operator--(int) noexcept {
                common_iterator cp = *this;
                --p;
                return cp;
            }
            common_iterator& operator--() noexcept {
                --p;
                return *this;
            }
            common_iterator operator+(int x) const noexcept {
                return common_iterator(p + x);
            }
            common_iterator operator-(int x) const noexcept {
                return common_iterator(p - x);
            }
            common_iterator& operator+=(int x) noexcept {
                p += x;
                return *this;
            }
            common_iterator& operator-=(int x) noexcept {
                p -= x;
                return *this;
            }
            bool operator==(const common_iterator& other) const noexcept {
                return (p == other.p); 
            }
            bool operator!=(const common_iterator& other) const noexcept {
                return (p != other.p);
            }
            
            std::conditional_t<isConst, const T&, T&> operator*() {
                return *p;
            }
            std::conditional_t<isConst, const T*, T*> operator->() {
                return p;
            }
            
            difference_type operator-(const common_iterator& other) const noexcept {
                return p - other.p;
            }
            bool operator>(const common_iterator& other) const noexcept {
                return *this - other > 0;
            }
            bool operator<(const common_iterator& other) const noexcept {
                return *this - other < 0;
            }
            bool operator>=(const common_iterator& other) const noexcept {
                return *this - other >= 0;
            }
            bool operator<=(const common_iterator& other) const noexcept {
                return *this - other <= 0;
            }
            operator common_iterator<true>() const noexcept {
                return common_iterator<true>(p);
            }
        };

    public:
        using iterator = common_iterator<false>;
        using const_iterator = common_iterator<true>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        
        iterator begin() noexcept;
        iterator end() noexcept;
        const_iterator cbegin() const noexcept;
        const_iterator cend() const noexcept;
        reverse_iterator rbegin() noexcept;
        reverse_iterator rend() noexcept;
        const_reverse_iterator crbegin() const noexcept;
        const_reverse_iterator crend() const noexcept;
       
        //main constructors
        vector(const Alloc& alloc = Alloc()); // why reference? alloc can be stateful and store a buffer. Why const? to be able to accept rvalues(std::move(alloc) or Alloc{})
        vector(std::size_t num_of_elem, const Alloc& alloc = Alloc()); // this constructor may be deleted
        vector(std::size_t num_of_elem, const T& elem, const Alloc& alloc = Alloc()); // why do we accept elem by const ref? 1) Not to copy 2) To be able to accept rvalues
        vector(std::initializer_list<T> init_l, const Alloc& alloc = Alloc()); // init_list is a lightweight object and is always rvalue
        //copy and move constructors accordingly
        vector(const vector& other); // not by value, because we would get a limitless recursion and because we don't want to make an extra copy anyway, use const to
                                     // be able to copy rvalue vectors(not true because of copy-elision) and const vectors(this is for sure)
        vector(vector&& other) noexcept(std::is_nothrow_move_constructible_v<Alloc>); // accept by rvalue reference to accept rvalues, this constructor doesn't throw exceptions
        //copy and move assignment operators
        vector& operator=(const vector& other); // could not to return, but return reference for things like vector<some_type, some_allocator> v3 = v2 = v1;
        vector& operator=(vector&& other) 
            noexcept((!std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value || std::allocator_traits<Alloc>::is_always_equal::value) || std::is_nothrow_move_assignable_v<Alloc>);
        ~vector();

        void reserve(std::size_t new_cap);
        void resize(std::size_t new_sz);
        void resize(std::size_t new_sz, const T& value);
        void shrink_to_fit(); 
        std::size_t capacity() const noexcept; 
        std::size_t size() const noexcept;
        bool empty() const noexcept; 
        
        T& operator[](std::size_t index);
        const T& operator[](std::size_t index) const;
        T& at(std::size_t index);
        const T& at(std::size_t index) const;
        T& front();  
        const T& front() const;  
        T& back(); 
        const T& back() const; 
        T* data() noexcept; 
        const T* data() const noexcept; 
        
        bool operator==(const vector& other) const noexcept; //

        template<typename... Args>
        iterator emplace(const_iterator pos, Args&&... args); // why do we accept by a universal reference here and not by a constant lvalue reference?
                                                              // the thing is that if we use the 2nd option, then constructor has to accept is by constant lvalue reference too,
                                                              // because the parameter will have type const some_type& and not some_type&& as we want
                                                              // 
        iterator insert(const_iterator pos, const T& value);  // we accept here a const_iterator, because it can be casted to iterator and not vica versa 
        
        iterator insert(const_iterator pos, T&& value);

        template<typename... Args>
        void emplace_back(Args&&... args);

        void push_back(const T& value); // we can accept constant values

        void push_back(T&& value);

        iterator erase(const_iterator pos);
        
        void pop_back();

        void clear() noexcept;
        
        void swap(vector& other) 
            noexcept(std::allocator_traits<Alloc>::is_always_equal::value || (std::allocator_traits<Alloc>::propagate_on_container_swap::value && std::is_nothrow_swappable_v<Alloc>));  //just reference, because there's no sense to accept constants or rvalues

    };



    template<typename T, typename Alloc>
    my::vector<T, Alloc>::vector(const Alloc& alloc) : alloc(alloc), arr(nullptr), sz(0), cap(0) {}

    template<typename T, typename Alloc>
    vector<T, Alloc>::vector(std::size_t num_of_elem, const Alloc& alloc) 
        : alloc(alloc),
        arr(std::allocator_traits<Alloc>::allocate(this->alloc, num_of_elem)),
        cap(num_of_elem),
        sz(num_of_elem)
    {
        for(std::size_t i = 0; i != num_of_elem; ++i) {
            try {
                std::allocator_traits<Alloc>::construct(this->alloc, arr+i, T());
            }
            catch(...) {
                for(std::size_t j = 0; j != i; ++j) {
                    std::allocator_traits<Alloc>::destroy(this->alloc, arr+j);
                }
                std::allocator_traits<Alloc>::deallocate(this->alloc, arr, num_of_elem);
                throw;
            }
        }
    }

    template<typename T, typename Alloc>
    vector<T,Alloc>::vector(std::size_t num_of_elem, const T& value, const Alloc& alloc)
       : alloc(alloc),
       arr(std::allocator_traits<Alloc>::allocate(this->alloc, num_of_elem)),
       cap(num_of_elem),
       sz(num_of_elem) 
    {
        for(std::size_t i = 0; i != num_of_elem; ++i) {
            try {
                std::allocator_traits<Alloc>::construct(this->alloc, arr+i, value);
            }
            catch(...) {
                for(std::size_t j = 0; j != i; ++j) {
                    std::allocator_traits<Alloc>::destroy(this->alloc, arr+j);
                }
                std::allocator_traits<Alloc>::deallocate(this->alloc, arr, num_of_elem);
                throw;
            }
        }

    }

    template<typename T, typename Alloc>
    vector<T, Alloc>::vector(std::initializer_list<T> init_l, const Alloc& alloc) 
        : alloc(alloc),
        arr(std::allocator_traits<Alloc>::allocate(this->alloc, init_l.size())),
        cap(init_l.size()),
        sz(init_l.size())
    {
        for(std::size_t i = 0; i != init_l.size(); ++i) {
            try {
                std::allocator_traits<Alloc>::construct(this->alloc, arr + i, *(init_l.begin() + i)); // initializer_lists' elements cannot be moved as they're constant
            }
            catch(...) {
                for(std::size_t j = 0; j != i; ++j) {
                    std::allocator_traits<Alloc>::destroy(this->alloc, arr + j);
                }
                std::allocator_traits<Alloc>::deallocate(this->alloc, arr, init_l.size());
                throw;
            }
        }
    }

    template<typename T, typename Alloc>
    vector<T, Alloc>::vector(const vector& other) 
        : alloc(std::allocator_traits<Alloc>::select_on_container_copy_construction(alloc)),
        arr(std::allocator_traits<Alloc>::allocate(this->alloc, other.cap)),
        cap(other.cap),
        sz(other.sz)
    {
        for(std::size_t i = 0; i != other.sz; ++i) {
            try {
                std::allocator_traits<Alloc>::construct(this->alloc, arr + i, *(other.arr + i));
            }
            catch(...) {
                for(std::size_t j = 0; j != i; ++j) {
                    std::allocator_traits<Alloc>::destroy(this->alloc, arr + j);
                }
                std::allocator_traits<Alloc>::deallocate(this->alloc, arr, other.cap);
                throw;
            }
        }
    }

    template<typename T, typename Alloc>
    vector<T, Alloc>::vector(vector&& other) noexcept(std::is_nothrow_move_constructible_v<Alloc>)
        : alloc(std::move(other.alloc)), // others' arr points to nullptr after all, so it's not binded with its' allocator anymore, that's why we move it
        arr(other.arr),
        cap(other.cap),
        sz(other.sz)
    {
        other.arr = nullptr;
        other.cap = 0;
        other.sz = 0;
    }

    template<typename T, typename Alloc>
    vector<T, Alloc>& vector<T, Alloc>::operator=(const vector& other) {
        Alloc new_alloc = alloc;
        if (std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value) {
            new_alloc = other.alloc;
        }
        T* new_arr = std::allocator_traits<Alloc>::allocate(new_alloc, other.cap);
        for (std::size_t i = 0; i != other.sz; ++i) {
            try {
                std::allocator_traits<Alloc>::construct(new_alloc, new_arr + i, *(other.arr + i));
            }
            catch(...) {
                for (std::size_t j = 0; j != i; ++j) {
                    std::allocator_traits<Alloc>::destroy(new_alloc, new_arr + j);
                }
                std::allocator_traits<Alloc>::deallocate(new_alloc, new_arr, other.cap);
                throw;
            }
        }
        for(std::size_t i = 0; i != sz; ++i) {
            std::allocator_traits<Alloc>::destroy(alloc, arr + i);
        }
        std::allocator_traits<Alloc>::deallocate(alloc, arr, cap);
        if (alloc != new_alloc) {
            alloc = new_alloc;
        }
        arr = new_arr;
        cap = other.cap;
        sz = other.sz;
        return *this;
    }
    
    //this one should be redone
    template<typename T, typename Alloc>
    vector<T, Alloc>& vector<T, Alloc>::operator=(vector&& other) 
        noexcept((!std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value || std::allocator_traits<Alloc>::is_always_equal::value) || std::is_nothrow_move_assignable_v<Alloc>)
    {
        if (alloc != alloc.other) {        
            
            if (std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value) {
                
                assert(std::is_nothrow_move_assignable_v<Alloc>); //exception safety cannot be guaranteed otherwise

                for(std::size_t i = 0; i != sz; ++i) {
                    std::allocator_traits<Alloc>::destroy(alloc, arr + i);
                }
                std::allocator_traits<Alloc>::deallocate(alloc, arr, cap);
                
                alloc = std::move(other.alloc);
                arr = other.arr;
                cap = other.cap;
                sz = other.sz;
                other.arr = nullptr;
                other.cap = 0;
                other.sz = 0;
                 
            }
            else {
                T* new_arr = std::allocator_traits<Alloc>::allocate(alloc, other.cap);
                for(std::size_t i = 0; i != other.sz; ++i) {
                    try {
                        std::allocator_traits<Alloc>::construct(alloc, new_arr + i, std::move_if_noexcept(*(other.arr + i)));
                    }
                    catch(...) {
                        for(std::size_t j = 0; j != i; ++j) {
                            std::allocator_traits<Alloc>::destroy(alloc, new_arr + j);
                        }
                        std::allocator_traits<Alloc>::deallocate(alloc, new_arr, other.cap);
                        throw;
                    }
                }
                
                if (!std::is_nothrow_move_constructible_v<Alloc> && std::is_copy_constructible_v<Alloc>) {
                    for(std::size_t i = 0; i != other.sz; ++i) {
                        std::allocator_traits<Alloc>::destroy(other.alloc, other.arr + i);
                    }
                }
                std::allocator_traits<Alloc>::deallocate(other.alloc, other.arr, other.cap);

                for(std::size_t i = 0; i != sz; ++i) {
                    std::allocator_traits<Alloc>::destroy(alloc, arr + i);
                }
                std::allocator_traits<Alloc>::deallocate(alloc, arr, cap);
                
                arr = new_arr;
                cap = other.cap;
                sz = other.sz;

                other.arr = nullptr;
                other.cap = 0;
                other.sz = 0;
            }
        }
        else {
            for(std::size_t i = 0; i != sz; ++i) {
                std::allocator_traits<Alloc>::destroy(alloc, arr + i);
            }
            std::allocator_traits<Alloc>::deallocate(alloc, arr, cap);
            arr = other.arr;
            cap = other.cap;
            sz = other.sz;
            other.arr = nullptr;
            other.cap = 0;
            other.sz = 0;
        }
        return *this;
    }
    
    //it's ok(actually, we can modify this, but later), even if move-constructor of T is not noexcept and it hasn't a copy-constructor (or has a deleted one)
    template<typename T, typename Alloc>
    void vector<T, Alloc>::reserve(std::size_t new_cap) {
        if (new_cap <= cap) return;
        T* new_arr = std::allocator_traits<Alloc>::allocate(alloc, new_cap);
        for(std::size_t i = 0; i != sz; ++i) {
            try {
                std::allocator_traits<Alloc>::construct(alloc, new_arr + i, std::move_if_noexcept(*(arr + i)));
            }
            catch(...) {
                for(std::size_t j = 0; j != i; ++j) {
                    std::allocator_traits<Alloc>::destroy(alloc, new_arr + j);
                }
                std::allocator_traits<Alloc>::deallocate(alloc, new_arr, new_cap);
                throw; 
            }
        }
        if (!std::is_nothrow_move_constructible_v<T> && std::is_copy_constructible_v<Alloc>) {
            for(std::size_t i = 0; i != sz; ++i) {
                std::allocator_traits<Alloc>::destroy(alloc, arr + i);
            }
        }
        std::allocator_traits<Alloc>::deallocate(alloc, arr, cap);
        arr = new_arr;
        cap = new_cap;
    }

    template<typename T, typename Alloc>
    void vector<T,Alloc>::resize(std::size_t new_sz) {
        if (new_sz > cap) {
        T* new_arr = std::allocator_traits<Alloc>::allocate(alloc, new_sz);
        
        for(std::size_t i = sz; i != new_sz; ++i) {
            try {
                std::allocator_traits<Alloc>::construct(alloc, new_arr + i, T());
            }
            catch(...) {
                for(std::size_t j = sz; j != i; ++j) {
                    std::allocator_traits<Alloc>::destroy(alloc, new_arr + j);
                }
                std::allocator_traits<Alloc>::deallocate(alloc, new_arr, new_sz);
                throw;
            }
        }

        for(std::size_t i = 0; i != sz; ++i) {
            try {
                std::allocator_traits<Alloc>::construct(alloc, new_arr + i, std::move_if_noexcept(*(arr + i)));
            }
            catch(...) {
                for(std::size_t j = 0; j != i; ++j) {
                    std::allocator_traits<Alloc>::destroy(alloc, new_arr + j);
                }
                for(std::size_t j = sz; j != new_sz; ++j) {
                    std::allocator_traits<Alloc>::destroy(alloc, new_arr + j);
                }
                std::allocator_traits<Alloc>::deallocate(alloc, new_arr, new_sz);
                throw; 
            }
        }

        if (!std::is_nothrow_move_constructible_v<T> && std::is_copy_constructible_v<Alloc>) {
            for(std::size_t i = 0; i != sz; ++i) {
                std::allocator_traits<Alloc>::destroy(alloc, arr + i);
            }
        }
        std::allocator_traits<Alloc>::deallocate(alloc, arr, cap);
        arr = new_arr;
        cap = new_sz;
        sz = new_sz;
        }
        else {
            for(std::size_t i = sz; i != new_sz; ++i) {
                try {
                    std::allocator_traits<Alloc>::construct(alloc, arr + i, T());
                }
                catch(...) {
                    for(std::size_t j = sz; j != i; ++j) {
                        std::allocator_traits<Alloc>::destroy(alloc, arr + j);
                    }
                    throw;
                }
            }
            sz = new_sz;
        }
    }
    
    template<typename T, typename Alloc>
    void vector<T, Alloc>::resize(std::size_t new_sz, const T& value) {
        if (new_sz > cap) {
        T* new_arr = std::allocator_traits<Alloc>::allocate(alloc, new_sz);
        
        for(std::size_t i = sz; i != new_sz; ++i) {
            try {
                std::allocator_traits<Alloc>::construct(alloc, new_arr + i, value);
            }
            catch(...) {
                for(std::size_t j = sz; j != i; ++j) {
                    std::allocator_traits<Alloc>::destroy(alloc, new_arr + j);
                }
                std::allocator_traits<Alloc>::deallocate(alloc, new_arr, new_sz);
                throw;
            }
        }

        for(std::size_t i = 0; i != sz; ++i) {
            try {
                std::allocator_traits<Alloc>::construct(alloc, new_arr + i, std::move_if_noexcept(*(arr + i)));
            }
            catch(...) {
                for(std::size_t j = 0; j != i; ++j) {
                    std::allocator_traits<Alloc>::destroy(alloc, new_arr + j);
                }
                for(std::size_t j = sz; j != new_sz; ++j) {
                    std::allocator_traits<Alloc>::destroy(alloc, new_arr + j);
                }
                std::allocator_traits<Alloc>::deallocate(alloc, new_arr, new_sz);
                throw; 
            }
        }

        if (!std::is_nothrow_move_constructible_v<T> && std::is_copy_constructible_v<Alloc>) {
            for(std::size_t i = 0; i != sz; ++i) {
                std::allocator_traits<Alloc>::destroy(alloc, arr + i);
            }
        }
        std::allocator_traits<Alloc>::deallocate(alloc, arr, cap);
        arr = new_arr;
        cap = new_sz;
        sz = new_sz;
        }
        else {
            for(std::size_t i = sz; i != new_sz; ++i) {
                try {
                    std::allocator_traits<Alloc>::construct(alloc, arr + i, value);
                }
                catch(...) {
                    for(std::size_t j = sz; j != i; ++j) {
                        std::allocator_traits<Alloc>::destroy(alloc, arr + j);
                    }
                    throw;
                }
            }
            sz = new_sz;
        }

    }

    template<typename T, typename Alloc>
    void vector<T, Alloc>::clear() noexcept {
        for(std::size_t i = 0; i != sz; ++i) {
            std::allocator_traits<Alloc>::destroy(alloc, arr + i);
        }
        sz = 0;
    }

    template<typename T, typename Alloc>
    vector<T, Alloc>::~vector() {
        clear();
        std::allocator_traits<Alloc>::deallocate(alloc, arr, cap);
    }

    template<typename T, typename Alloc>
    void vector<T, Alloc>::shrink_to_fit() {
        if (sz == cap) return;
        T* new_arr = std::allocator_traits<Alloc>::allocate(alloc, sz);
        for(std::size_t i = 0; i != sz; ++i) {
            try {
            std::allocator_traits<Alloc>::construct(alloc, new_arr + i, std::move_if_noexcept(*(arr + i)));
            }
            catch(...) {
                for(std::size_t j = 0; j != i; ++j) {
                    std::allocator_traits<Alloc>::destroy(alloc, new_arr + j);
                }
                std::allocator_traits<Alloc>::deallocate(alloc, new_arr, sz);
                throw;
            }
        }
        
        if (!std::is_nothrow_move_constructible_v<Alloc> && std::is_copy_constructible_v<Alloc>) {
           for(std::size_t i = 0; i != sz; ++i) {
               std::allocator_traits<Alloc>::destroy(alloc, arr + i);
           }
        }
        std::allocator_traits<Alloc>::deallocate(alloc, arr, cap);

        arr = new_arr;
        cap = sz;
    }

    template<typename T, typename Alloc>
    std::size_t vector<T, Alloc>::capacity() const noexcept {
        return cap;
    }

    template<typename T, typename Alloc>
    std::size_t vector<T, Alloc>::size() const noexcept {
        return sz;
    }

    template<typename T, typename Alloc>
    bool vector<T, Alloc>::empty() const noexcept {
        return (sz == 0);
    }
    
    template<typename T, typename Alloc>
    T& vector<T, Alloc>::operator[](std::size_t index) {
            return *(arr + index);
        }
    
    template<typename T, typename Alloc>
    const T& vector<T, Alloc>::operator[](std::size_t index) const {
        return *(arr + index);
    }
    
    template<typename T, typename Alloc>
    T& vector<T, Alloc>::at(std::size_t index) {
        if (index >= sz) {
            throw std::out_of_range("You got out of range!");
        }
        return *(arr + index);
    }
    
    template<typename T, typename Alloc>
    const T& vector<T, Alloc>::at(std::size_t index) const {
        if (index >= sz) {
            throw std::out_of_range("You got out of range!");
        }
        return *(arr + index);
    }
    
    template<typename T, typename Alloc>
    T& vector<T, Alloc>::front() {
        return *arr;
    }

    template<typename T, typename Alloc>
    const T& vector<T, Alloc>::front() const {
        return *arr;
    }

    template<typename T, typename Alloc>
    T& vector<T, Alloc>::back() {
        return *(arr + sz - 1);
    } 

    template<typename T, typename Alloc>
    const T& vector<T, Alloc>::back() const {
        return *(arr + sz - 1);
    }

    template<typename T, typename Alloc>
    T* vector<T, Alloc>::data() noexcept {
        return arr;
    }

    template<typename T, typename Alloc>
    const T* vector<T, Alloc>::data() const noexcept {
        return arr;
    }

    template<typename T, typename Alloc>    
    bool vector<T, Alloc>::operator==(const vector& other) const noexcept {
        if (sz == other.sz) {
            for(std::size_t i = 0; i != sz; ++i) {
                if (*(arr + i) != *(other.arr + i)) return false;
            }
            return true;
        }
        return false;
    }

    template<typename T, typename Alloc>
    typename vector<T,Alloc>::iterator vector<T,Alloc>::begin() noexcept {
        return iterator(arr);
    }

    template<typename T, typename Alloc>
    typename vector<T,Alloc>::iterator vector<T, Alloc>::end() noexcept {
        return iterator(arr + sz);
    }

    template<typename T, typename Alloc>
    typename vector<T, Alloc>::const_iterator vector<T, Alloc>::cbegin() const noexcept {
        return const_iterator(arr);
    }

    template<typename T, typename Alloc>
    typename vector<T, Alloc>::const_iterator vector<T, Alloc>::cend() const noexcept {
        return const_iterator(arr + sz);
    }

    template<typename T, typename Alloc>
    typename vector<T, Alloc>::reverse_iterator vector<T, Alloc>::rbegin() noexcept {
        return reverse_iterator(end());
    }

    template<typename T, typename Alloc>
    typename vector<T, Alloc>::reverse_iterator vector<T, Alloc>::rend() noexcept {
        return reverse_iterator(begin());
    }

    template<typename T, typename Alloc>
    typename vector<T, Alloc>::const_reverse_iterator vector<T, Alloc>::crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    template<typename T, typename Alloc>
    typename vector<T, Alloc>::const_reverse_iterator vector<T, Alloc>::crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }
    
    template<typename T, typename Alloc>
    template<typename... Args>
    void vector<T, Alloc>::emplace_back(Args&&... args) {
        if (sz == cap) {
            std::size_t new_cap = cap == 0 ? 1 : cap * 2;
            std::size_t new_sz = sz + 1;
            T* new_arr = std::allocator_traits<Alloc>::allocate(alloc, new_cap);
            try {
                std::allocator_traits<Alloc>::construct(alloc, new_arr + sz, std::forward<Args>(args)...);
            }
            catch(...) {
                std::allocator_traits<Alloc>::deallocate(alloc, new_arr, new_cap);
                throw;
            }
            for(std::size_t i = 0; i != sz; ++i) {
                try {
                    std::allocator_traits<Alloc>::construct(alloc, new_arr + i, std::move_if_noexcept(*(arr + i)));
                }
                catch(...) {
                    for(std::size_t j = 0; j != i; ++j) {
                        std::allocator_traits<Alloc>::destroy(alloc, new_arr + j);
                    }
                    std::allocator_traits<Alloc>::destroy(alloc, new_arr + sz);
                    std::allocator_traits<Alloc>::deallocate(alloc, new_arr, new_cap);
                    throw;
                }
            }
            
            if (!std::is_nothrow_move_constructible_v<T> && std::is_copy_constructible_v<T>) {
                for(std::size_t i = 0; i != sz; ++i) {
                    std::allocator_traits<Alloc>::destroy(alloc, arr + i);
                }
            }
            std::allocator_traits<Alloc>::deallocate(alloc, arr, cap);
            
            arr = new_arr;
            cap = new_cap;
            sz = new_sz;
        }
        else {
            std::allocator_traits<Alloc>::construct(alloc, arr + sz, std::forward<Args>(args)...);
            ++sz;
        }
    }
    
    template<typename T, typename Alloc>
    template<typename... Args>
    typename vector<T, Alloc>::iterator vector<T, Alloc>::emplace(const_iterator pos, Args&&... args) {
        typename iterator::difference_type diff = pos - cbegin();
        emplace_back(std::forward<Args>(args)...);
        iterator iter = arr + diff;
        for(reverse_iterator rit = rbegin(); (rit != reverse_iterator(iter + 1)) && (rit != --rend()); ++rit) {
            std::swap(*rit, *(rit + 1)); // is swap throws an exception, then we can't ensure exception safety
        }
        return iter;        
    }

    template<typename T, typename Alloc>
    void vector<T, Alloc>::push_back(const T& value) {
        emplace_back(value);
    }

    template<typename T, typename Alloc>
    void vector<T, Alloc>::push_back(T&& value) {
        emplace_back(std::move(value));
    }

    template<typename T, typename Alloc>
    typename vector<T, Alloc>::iterator vector<T, Alloc>::insert(const_iterator pos, const T& value) {
        return emplace(pos, value);
    }

    template<typename T, typename Alloc>
    typename vector<T, Alloc>::iterator vector<T, Alloc>::insert(const_iterator pos, T&& value) {
        return emplace(pos, std::move(value));
    }

    template<typename T, typename Alloc>
    void vector<T, Alloc>::pop_back() {
        std::allocator_traits<Alloc>::destroy(alloc, arr + sz);
        --sz;
    }

    template<typename T, typename Alloc>
    typename vector<T, Alloc>::iterator vector<T, Alloc>::erase(const_iterator pos) {
        assert(pos < end());
        typename iterator::difference_type diff = pos - cbegin();
        iterator iter = arr + diff;
        for(iterator it = iter; it != --end(); ++it) {
            std::swap(*it, *(it + 1));
        }
        pop_back();
        return iter;
    }
    
    //modify this later
    template<typename T, typename Alloc>
    void vector<T, Alloc>::swap(vector& other) 
        noexcept(std::allocator_traits<Alloc>::is_always_equal::value || (std::allocator_traits<Alloc>::propagate_on_container_swap::value && std::is_nothrow_swappable_v<Alloc>))     {
        if (std::allocator_traits<Alloc>::propagate_on_container_swap::value && (alloc != other.alloc)) {
            std::swap(alloc, other.alloc);
        }
        std::swap(arr, other.arr);
        std::swap(sz, other.sz);
        std::swap(cap, other.cap);
    }
};




















