#include <iostream>

namespace my {

    template<typename T, typename... Args>
    class is_constructible {
        template<typename TT, typename... AArgs, typename = decltype(TT(std::declval<AArgs>()...))>
        static std::true_type f(int);
        
        template<typename...>
        static std::false_type f(...);
    public:
        static const int value = decltype(f<T,Args...>(0))::value;
    };

    template<typename T, typename... Args>
    constexpr bool is_constructible_v = is_constructible<T,Args...>::value;
    

    template<typename T>
    class is_default_constructible {
        template<typename TT, typename = decltype(TT())>
        static std::true_type f(int);

        template<typename...>
        static std::false_type f(...);
    public:
        static constexpr bool value = decltype(f<T>(0))::value;
    };

    template<typename T>
    constexpr bool is_default_constructible_v = is_default_constructible<T>::value;

    
    template<typename T>
    class is_copy_constructible {
        template<typename TT, typename = decltype(TT(std::declval<TT&>()))>
        static std::true_type f(int);

        template<typename...>
        static std::false_type f(...);

    public:
        static constexpr bool value = decltype(f<T>(0))::value;
    };
    
    template<typename T>
    constexpr bool is_copy_constructible_v = is_copy_constructible<T>::value;
    

    template<typename T>
    class is_move_constructible {
        template<typename TT, typename = decltype(TT(std::declval<std::remove_reference_t<TT>>()))>
        static std::true_type f(int);

        template<typename...>
        static std::false_type f(...);
    public:
        static constexpr bool value = decltype(f<T>(0))::value; // there're no class examples in compile-time, hence there's no f() function (if it's non-static)
    };

    template<typename T>
    constexpr bool is_move_constructible_v = is_move_constructible<T>::value;


    template<typename T>
    class is_copy_assignable {
        template<typename TT, typename = decltype(std::declval<TT&>() = std::declval<TT&>())> // lhs has type TT& and not TT as the second option can be treated as rvalue(TEST!)Example: int() is rvalue and has type int
        static std::true_type f(int); // static for 2 reasons - the sense, we use it in the expression by which we initialize a static constexpr class field

        template<typename...>
        static std::false_type f(...);

    public:
        static constexpr bool value = decltype(f<T>(0))::value; // static at least for 3 reasons - the sense, must be initialized in compile-time, convinient 
    };

    template<typename T>
    constexpr bool is_copy_assignable_v = is_copy_assignable<T>::value;


    template<typename T>
    class is_move_assignable {
        template<typename TT, typename = decltype(std::declval<TT&>() = std::declval<std::remove_reference_t<TT>&&>)>
        static std::true_type f(int);
        
        template<typename...>
        static std::false_type f(...);

    public:
        static constexpr bool value = decltype(f<T>(0))::value;
    };

    template<typename T>
    constexpr bool is_move_assignable_v = is_move_assignable<T>::value;


    template<typename T, typename... Args>
    class is_nothrow_constructible {
        template<typename TT, typename... AArgs, typename = std::enable_if<noexcept(TT(std::declval<AArgs>()...))>>
        static std::true_type f(int);

        template<typename...>
        static std::false_type f(...);

    public:
        static constexpr bool value = decltype(f<T, Args...>(0))::value;
    };

    template<typename T>
    constexpr bool is_nothrow_constructible_v = is_nothrow_constructible<T>::value;
    
};














