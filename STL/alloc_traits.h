#include <limits>
#include <type_traits>
namespace my {
    
    template<typename Alloc>
    class allocator_traits {
    
        template<typename AAlloc>
        class has_pointer;

        template<typename AAlloc>
        class has_const_pointer;

        template<typename AAlloc>
        class has_difference_type;

        template<typename AAlloc>
        class has_size_type;

        template<typename AAlloc>
        class has_propagate_on_container_copy_assignment;

        template<typename AAlloc>
        class has_propagate_on_container_move_assignment;
       
        template<typename AAlloc>
        class has_propagate_on_container_swap;

        template<typename AAlloc>
        class has_is_always_equal;

    public:
        using allocator_type = Alloc;
        using value_type = typename Alloc::value_type;
        using pointer = typename has_pointer<Alloc>::type;
        using const_pointer = typename has_const_pointer<Alloc>::type;
        using difference_type = typename has_difference_type<Alloc>::type;
        using size_type = typename has_size_type<Alloc>::type;
        using propagate_on_container_copy_assignment = typename has_propagate_on_container_copy_assignment<Alloc>::type;
        using propagate_on_container_move_assignment = typename has_propagate_on_container_move_assignment<Alloc>::type;
        using propagate_on_container_swap = typename has_propagate_on_container_swap<Alloc>::type;
        using is_always_equal = typename has_is_always_equal<Alloc>::type;

    private: 
        
        template<typename AAlloc>
        class has_pointer {
            template<typename AAAlloc>
            static typename AAAlloc::pointer f(int);

            template<typename...>
            static value_type* f(...);
        public:
            using type = decltype(f<AAlloc>(0));
        };
        
    
        template<typename AAlloc>
        class has_const_pointer {
            template<typename AAAlloc>
            static typename AAAlloc::const_pointer f(int);

            template<typename...>
            static typename std::pointer_traits<pointer>::rebind<const value_type> f(...);
        public:
            using type = decltype(f<AAlloc>(0));
        };


        template<typename AAlloc>
        class has_difference_type {
            template<typename AAAlloc>
            static typename AAAlloc::difference_type f(int);
            
            template<typename...>
            static typename std::pointer_traits<pointer>::difference_type f(...); // long long
        public:
            using type = decltype(f<AAlloc>(0));
        };

    
        template<typename AAlloc>
        class has_size_type {
            template<typename AAAlloc>
            static typename AAAlloc::size_type f(int);

            template<typename AAAlloc>
            static typename std::make_unsigned<difference_type>::type f(...); //std::size_t 
        public:
            using type = decltype(f<AAlloc>(0));
        };

    
        template<typename AAlloc>
        class has_propagate_on_container_copy_assignment {
            template<typename AAAlloc>
            static typename AAAlloc::propagate_on_container_copy_assignment f(int);

            template<typename...>
            static std::false_type f(...);

        public:
            using type = decltype(f<AAlloc>(0));
        };

    
        template<typename AAlloc>
        class has_propagate_on_container_move_assignment {
            template<typename AAAlloc>
            static typename AAAlloc::propagate_on_container_move_assignment f(int);

            template<typename...>
            static std::false_type f(...);

        public:
            using type = decltype(f<AAlloc>(0));
        };

    
        template<typename AAlloc>
        class has_propagate_on_container_swap {
            template<typename AAAlloc>
            static typename AAAlloc::propagate_on_container_swap f(int);

            template<typename...>
            static std::false_type f(...);
        public:
            using type = decltype(f<AAlloc>(0));
        };
        
    
        template<typename AAlloc>
        class has_is_always_equal {
            template<typename AAAlloc>
            static typename AAAlloc::is_always_equal f(int);

            template<typename...>
            static std::false_type f(...);

        public:
            using type = decltype(f<AAlloc>(0));
        };

        
        template<typename AAlloc, typename T, typename... Args>
        class has_construct {
            template<typename AAAlloc, typename TT, typename... AArgs, typename = decltype(std::declval<AAAlloc>().construct(std::declval<TT*>(), std::declval<AArgs>()...))>
            static std::true_type f(int);

            template<typename...>
            static std::false_type f(...);
        
        public:
            static constexpr bool value = decltype(f<AAlloc, T, Args...>(0))::value;
        };
        
        template<typename AAlloc, typename T>
        class has_destroy {
            template<typename AAAlloc, typename TT, typename = decltype(std::declval<AAAlloc>().destroy(std::declval<TT*>()))>
            static std::true_type f(int);

            template<typename...>
            static std::false_type f(...);

        public:
            static constexpr bool value = decltype(f<AAlloc, T>(0))::value;
        };

        
        template<typename AAlloc>
        class has_select_on_container_copy_construction {
            template<typename AAAlloc, typename = decltype(std::declval<AAAlloc>().select_on_container_copy_construction())>
            static std::true_type f(int);

            template<typename...>
            static std::false_type f(...);
        public:
            static constexpr bool value = decltype(f<AAlloc>(0))::value;
        };

        
        template<typename AAlloc>
        class has_max_size {
            template<typename AAAlloc, typename = decltype(std::declval<AAAlloc>().max_size())>
            static std::true_type f(int);

            template<typename...>
            static std::false_type f(...);

        public:
            static constexpr bool value = decltype(f<AAlloc>(0))::value;
        };


        template<typename AAlloc, typename T>
        class has_rebind {
            template<typename AAAlloc, typename TT>
            static typename AAAlloc::template rebind<TT>::other f(int);

            template<typename...>
            static std::allocator<T> f(...);

        public:
            using type = decltype(f<AAlloc, T>(0));
        };
    public:
        
        static constexpr pointer allocate(Alloc& alloc, std::size_t num_of_elem) {
            return alloc.allocate(num_of_elem);
        }

        static constexpr void deallocate(Alloc& alloc, pointer p, std::size_t num_of_elem) noexcept {
            alloc.deallocate(p, num_of_elem);
        }

        template<typename T, typename... Args>
        static constexpr T* construct(Alloc& alloc, T* p, Args&&... args) {
            if constexpr (has_construct<Alloc, T, Args...>::value) {
                return alloc.construct(p, std::forward<Args>(args)...);
            }
            else {
                return std::construct_at(p, std::forward<Args>(args)...);
            }
        }
        
        template<typename T>
        static constexpr void destroy(Alloc& alloc, T* p) noexcept {
            if constexpr (has_destroy<Alloc,T>::value) {
                alloc.destroy(p);
            }
            else {
                //p->~T(); untill C++20
                std::destroy_at(p); // actually same with p->~T() for not arrays
            }
        }
        
        static constexpr Alloc select_on_container_copy_construction(const Alloc& alloc) {
            if constexpr (has_select_on_container_copy_construction<Alloc>::value) {
                return alloc.select_on_container_copy_construction();
            }
            else {
                return alloc;
            }
        }

        static constexpr size_type max_size(const Alloc& alloc) noexcept {
            if constexpr (has_max_size<Alloc>::value) {
                return alloc.max_size();
            }
            else {
                return std::numeric_limits<size_type>::max() / sizeof(value_type);
            }
        }
        
        template<typename T>
        using rebind_alloc = typename has_rebind<Alloc, T>::type;

        template<typename T>
        using rebind_traits = allocator_traits<rebind_alloc<T>>;
    };



};
