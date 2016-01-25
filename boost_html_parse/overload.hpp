#pragma once
#include <type_traits>
namespace std_future {
	//P0051: C++ generic overload function
	//http://cpplover.blogspot.jp/2015/11/c-p0050r0-p0059r0.html
	//http://cpplover.blogspot.jp/2016/01/c-p0022r1-p0092r1.html
	//http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0051r0.pdf
	//http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0051r1.pdf
	// std::invoke
	template<typename Functor, typename... Args>
	typename std::enable_if<
		std::is_member_pointer<typename std::decay<Functor>::type>::value,
		typename std::result_of<Functor && (Args&&...)>::type
	>::type invoke(Functor&& f, Args&&... args)
	{
		return std::mem_fn(f)(std::forward<Args>(args)...);
	}

	template<typename Functor, typename... Args>
	typename std::enable_if<
		!std::is_member_pointer<typename std::decay<Functor>::type>::value,
		typename std::result_of<Functor && (Args&&...)>::type
	>::type invoke(Functor&& f, Args&&... args)
	{
		return std::forward<Functor>(f)(std::forward<Args>(args)...);
	}

	template<typename Return, typename Functor, typename... Args>
	Return invoke(Functor&& f, Args&&... args)
	{
		return invoke(std::forward<Functor>(f), std::forward<Args>(args)...);
	}

	// simple callable wrapper
	template < typename Functor, typename ... Args >
	struct sfun_impl
	{
		Functor f;

		sfun_impl(Functor f) : f(f) { }

		auto operator() (Args && ... args)
		{
			return invoke(f, std::forward<Args>(args)...);
		}
	};

	// function pointer
	template < typename R, typename ... Args >
	sfun_impl< R(*)(Args ...), Args ... >
		sfun(R(*p)(Args ...))
	{
		return sfun_impl< R(*)(Args ...), Args ... >(p);
	}

	// member function pointer with class pointer
	template < typename R, typename C, typename ... Args >
	sfun_impl< R(C::*)(C *, Args ...), C *, Args ... >
		sfun(R(C::* p)(C *, Args ...))
	{
		return sfun_impl< R(C::*)(C *, Args...), C *, Args ... >(p);
	}

	// member function pointer with class reference
	template < typename R, typename C, typename ... Args >
	sfun_impl< R(C::*)(C &&, Args ...), C &&, Args ... >
		sfun(R(C::* p)(C &&, Args ...))
	{
		return sfun_impl< R(C::*)(C &&, Args...), C &&, Args ... >(p);
	}

	// just to pass class type T
	// lazy conditional can be used instead.
	template < typename T >
	void sfun(T &&);


	template < typename T >
	using sfun_if =
		typename std::conditional<
		std::is_function< typename std::remove_pointer< typename std::remove_reference<T>::type >::type >::value ||
		std::is_member_function_pointer<T>::value ||
		std::is_member_object_pointer<T>::value,

		decltype(sfun(std::declval<T>())), T >::type;

	// primary template
	template < typename ... Functors >
	struct overloader;

	// terminator
	template < >
	struct overloader < >
	{
		void operator () () { }
	};

	template < typename Functor0, typename ... Functors >
	struct overloader< Functor0, Functors ... > : sfun_if<Functor0>, overloader<Functors...>
	{
		overloader(Functor0 && func0, Functors && ... funcs)
			: sfun_if<Functor0>(std::forward<Functor0>(func0)),
			overloader<Functors ...>(std::forward<Functors>(funcs) ...)
		{ }

		using sfun_if<Functor0>::operator ();
		using overloader<Functors...>::operator ();
	};



	template < typename ... Functors >
	overloader< Functors ... >
		overload(Functors && ... functors)
	{
		return overloader< Functors ... >(std::forward<Functors>(functors) ...);
	}
}