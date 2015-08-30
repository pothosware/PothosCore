// Boost.Function library

//  Copyright Douglas Gregor 2001-2006
//  Copyright Emil Dotchevski 2007
//  Use, modification and distribution is subject to the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

// Note: this header is a header template and must NOT have multiple-inclusion
// protection.
#include <Pothos/serialization/impl/function/detail/prologue.hpp>
#include <Pothos/serialization/impl/detail/no_exceptions_support.hpp>

#if defined(POTHOS_MSVC)
#   pragma warning( push )
#   pragma warning( disable : 4127 ) // "conditional expression is constant"
#endif       

#define POTHOS_FUNCTION_TEMPLATE_PARMS POTHOS_PP_ENUM_PARAMS(POTHOS_FUNCTION_NUM_ARGS, typename T)

#define POTHOS_FUNCTION_TEMPLATE_ARGS POTHOS_PP_ENUM_PARAMS(POTHOS_FUNCTION_NUM_ARGS, T)

#define POTHOS_FUNCTION_PARM(J,I,D) POTHOS_PP_CAT(T,I) POTHOS_PP_CAT(a,I)

#define POTHOS_FUNCTION_PARMS POTHOS_PP_ENUM(POTHOS_FUNCTION_NUM_ARGS,POTHOS_FUNCTION_PARM,POTHOS_PP_EMPTY)

#define POTHOS_FUNCTION_ARGS POTHOS_PP_ENUM_PARAMS(POTHOS_FUNCTION_NUM_ARGS, a)

#define POTHOS_FUNCTION_ARG_TYPE(J,I,D) \
  typedef POTHOS_PP_CAT(T,I) POTHOS_PP_CAT(POTHOS_PP_CAT(arg, POTHOS_PP_INC(I)),_type);

#define POTHOS_FUNCTION_ARG_TYPES POTHOS_PP_REPEAT(POTHOS_FUNCTION_NUM_ARGS,POTHOS_FUNCTION_ARG_TYPE,POTHOS_PP_EMPTY)

// Comma if nonzero number of arguments
#if POTHOS_FUNCTION_NUM_ARGS == 0
#  define POTHOS_FUNCTION_COMMA
#else
#  define POTHOS_FUNCTION_COMMA ,
#endif // BOOST_FUNCTION_NUM_ARGS > 0

// Class names used in this version of the code
#define POTHOS_FUNCTION_FUNCTION POTHOS_JOIN(function,POTHOS_FUNCTION_NUM_ARGS)
#define POTHOS_FUNCTION_FUNCTION_INVOKER \
  POTHOS_JOIN(function_invoker,POTHOS_FUNCTION_NUM_ARGS)
#define POTHOS_FUNCTION_VOID_FUNCTION_INVOKER \
  POTHOS_JOIN(void_function_invoker,POTHOS_FUNCTION_NUM_ARGS)
#define POTHOS_FUNCTION_FUNCTION_OBJ_INVOKER \
  POTHOS_JOIN(function_obj_invoker,POTHOS_FUNCTION_NUM_ARGS)
#define POTHOS_FUNCTION_VOID_FUNCTION_OBJ_INVOKER \
  POTHOS_JOIN(void_function_obj_invoker,POTHOS_FUNCTION_NUM_ARGS)
#define POTHOS_FUNCTION_FUNCTION_REF_INVOKER \
  POTHOS_JOIN(function_ref_invoker,POTHOS_FUNCTION_NUM_ARGS)
#define POTHOS_FUNCTION_VOID_FUNCTION_REF_INVOKER \
  POTHOS_JOIN(void_function_ref_invoker,POTHOS_FUNCTION_NUM_ARGS)
#define POTHOS_FUNCTION_MEMBER_INVOKER \
  POTHOS_JOIN(function_mem_invoker,POTHOS_FUNCTION_NUM_ARGS)
#define POTHOS_FUNCTION_VOID_MEMBER_INVOKER \
  POTHOS_JOIN(function_void_mem_invoker,POTHOS_FUNCTION_NUM_ARGS)
#define POTHOS_FUNCTION_GET_FUNCTION_INVOKER \
  POTHOS_JOIN(get_function_invoker,POTHOS_FUNCTION_NUM_ARGS)
#define POTHOS_FUNCTION_GET_FUNCTION_OBJ_INVOKER \
  POTHOS_JOIN(get_function_obj_invoker,POTHOS_FUNCTION_NUM_ARGS)
#define POTHOS_FUNCTION_GET_FUNCTION_REF_INVOKER \
  POTHOS_JOIN(get_function_ref_invoker,POTHOS_FUNCTION_NUM_ARGS)
#define POTHOS_FUNCTION_GET_MEMBER_INVOKER \
  POTHOS_JOIN(get_member_invoker,POTHOS_FUNCTION_NUM_ARGS)
#define POTHOS_FUNCTION_GET_INVOKER \
  POTHOS_JOIN(get_invoker,POTHOS_FUNCTION_NUM_ARGS)
#define POTHOS_FUNCTION_VTABLE POTHOS_JOIN(basic_vtable,POTHOS_FUNCTION_NUM_ARGS)

#ifndef POTHOS_NO_VOID_RETURNS
#  define POTHOS_FUNCTION_VOID_RETURN_TYPE void
#  define POTHOS_FUNCTION_RETURN(X) X
#else
#  define POTHOS_FUNCTION_VOID_RETURN_TYPE Pothos::detail::function::unusable
#  define POTHOS_FUNCTION_RETURN(X) X; return POTHOS_FUNCTION_VOID_RETURN_TYPE ()
#endif

namespace Pothos {
  namespace detail {
    namespace function {
      template<
        typename FunctionPtr,
        typename R POTHOS_FUNCTION_COMMA
        POTHOS_FUNCTION_TEMPLATE_PARMS
        >
      struct POTHOS_FUNCTION_FUNCTION_INVOKER
      {
        static R invoke(function_buffer& function_ptr POTHOS_FUNCTION_COMMA
                        POTHOS_FUNCTION_PARMS)
        {
          FunctionPtr f = reinterpret_cast<FunctionPtr>(function_ptr.func_ptr);
          return f(POTHOS_FUNCTION_ARGS);
        }
      };

      template<
        typename FunctionPtr,
        typename R POTHOS_FUNCTION_COMMA
        POTHOS_FUNCTION_TEMPLATE_PARMS
        >
      struct POTHOS_FUNCTION_VOID_FUNCTION_INVOKER
      {
        static POTHOS_FUNCTION_VOID_RETURN_TYPE
        invoke(function_buffer& function_ptr POTHOS_FUNCTION_COMMA
               POTHOS_FUNCTION_PARMS)

        {
          FunctionPtr f = reinterpret_cast<FunctionPtr>(function_ptr.func_ptr);
          POTHOS_FUNCTION_RETURN(f(POTHOS_FUNCTION_ARGS));
        }
      };

      template<
        typename FunctionObj,
        typename R POTHOS_FUNCTION_COMMA
        POTHOS_FUNCTION_TEMPLATE_PARMS
      >
      struct POTHOS_FUNCTION_FUNCTION_OBJ_INVOKER
      {
        static R invoke(function_buffer& function_obj_ptr POTHOS_FUNCTION_COMMA
                        POTHOS_FUNCTION_PARMS)

        {
          FunctionObj* f;
          if (function_allows_small_object_optimization<FunctionObj>::value)
            f = reinterpret_cast<FunctionObj*>(&function_obj_ptr.data);
          else
            f = reinterpret_cast<FunctionObj*>(function_obj_ptr.obj_ptr);
          return (*f)(POTHOS_FUNCTION_ARGS);
        }
      };

      template<
        typename FunctionObj,
        typename R POTHOS_FUNCTION_COMMA
        POTHOS_FUNCTION_TEMPLATE_PARMS
      >
      struct POTHOS_FUNCTION_VOID_FUNCTION_OBJ_INVOKER
      {
        static POTHOS_FUNCTION_VOID_RETURN_TYPE
        invoke(function_buffer& function_obj_ptr POTHOS_FUNCTION_COMMA
               POTHOS_FUNCTION_PARMS)

        {
          FunctionObj* f;
          if (function_allows_small_object_optimization<FunctionObj>::value)
            f = reinterpret_cast<FunctionObj*>(&function_obj_ptr.data);
          else
            f = reinterpret_cast<FunctionObj*>(function_obj_ptr.obj_ptr);
          POTHOS_FUNCTION_RETURN((*f)(POTHOS_FUNCTION_ARGS));
        }
      };

      template<
        typename FunctionObj,
        typename R POTHOS_FUNCTION_COMMA
        POTHOS_FUNCTION_TEMPLATE_PARMS
      >
      struct POTHOS_FUNCTION_FUNCTION_REF_INVOKER
      {
        static R invoke(function_buffer& function_obj_ptr POTHOS_FUNCTION_COMMA
                        POTHOS_FUNCTION_PARMS)

        {
          FunctionObj* f = 
            reinterpret_cast<FunctionObj*>(function_obj_ptr.obj_ptr);
          return (*f)(POTHOS_FUNCTION_ARGS);
        }
      };

      template<
        typename FunctionObj,
        typename R POTHOS_FUNCTION_COMMA
        POTHOS_FUNCTION_TEMPLATE_PARMS
      >
      struct POTHOS_FUNCTION_VOID_FUNCTION_REF_INVOKER
      {
        static POTHOS_FUNCTION_VOID_RETURN_TYPE
        invoke(function_buffer& function_obj_ptr POTHOS_FUNCTION_COMMA
               POTHOS_FUNCTION_PARMS)

        {
          FunctionObj* f = 
            reinterpret_cast<FunctionObj*>(function_obj_ptr.obj_ptr);
          POTHOS_FUNCTION_RETURN((*f)(POTHOS_FUNCTION_ARGS));
        }
      };

#if POTHOS_FUNCTION_NUM_ARGS > 0
      /* Handle invocation of member pointers. */
      template<
        typename MemberPtr,
        typename R POTHOS_FUNCTION_COMMA
        POTHOS_FUNCTION_TEMPLATE_PARMS
      >
      struct POTHOS_FUNCTION_MEMBER_INVOKER
      {
        static R invoke(function_buffer& function_obj_ptr POTHOS_FUNCTION_COMMA
                        POTHOS_FUNCTION_PARMS)

        {
          MemberPtr* f = 
            reinterpret_cast<MemberPtr*>(&function_obj_ptr.data);
          return Pothos::mem_fn(*f)(POTHOS_FUNCTION_ARGS);
        }
      };

      template<
        typename MemberPtr,
        typename R POTHOS_FUNCTION_COMMA
        POTHOS_FUNCTION_TEMPLATE_PARMS
      >
      struct POTHOS_FUNCTION_VOID_MEMBER_INVOKER
      {
        static POTHOS_FUNCTION_VOID_RETURN_TYPE
        invoke(function_buffer& function_obj_ptr POTHOS_FUNCTION_COMMA
               POTHOS_FUNCTION_PARMS)

        {
          MemberPtr* f = 
            reinterpret_cast<MemberPtr*>(&function_obj_ptr.data);
          POTHOS_FUNCTION_RETURN(Pothos::mem_fn(*f)(POTHOS_FUNCTION_ARGS));
        }
      };
#endif

      template<
        typename FunctionPtr,
        typename R POTHOS_FUNCTION_COMMA
        POTHOS_FUNCTION_TEMPLATE_PARMS
      >
      struct POTHOS_FUNCTION_GET_FUNCTION_INVOKER
      {
        typedef typename mpl::if_c<(is_void<R>::value),
                            POTHOS_FUNCTION_VOID_FUNCTION_INVOKER<
                            FunctionPtr,
                            R POTHOS_FUNCTION_COMMA
                            POTHOS_FUNCTION_TEMPLATE_ARGS
                          >,
                          POTHOS_FUNCTION_FUNCTION_INVOKER<
                            FunctionPtr,
                            R POTHOS_FUNCTION_COMMA
                            POTHOS_FUNCTION_TEMPLATE_ARGS
                          >
                       >::type type;
      };

      template<
        typename FunctionObj,
        typename R POTHOS_FUNCTION_COMMA
        POTHOS_FUNCTION_TEMPLATE_PARMS
       >
      struct POTHOS_FUNCTION_GET_FUNCTION_OBJ_INVOKER
      {
        typedef typename mpl::if_c<(is_void<R>::value),
                            POTHOS_FUNCTION_VOID_FUNCTION_OBJ_INVOKER<
                            FunctionObj,
                            R POTHOS_FUNCTION_COMMA
                            POTHOS_FUNCTION_TEMPLATE_ARGS
                          >,
                          POTHOS_FUNCTION_FUNCTION_OBJ_INVOKER<
                            FunctionObj,
                            R POTHOS_FUNCTION_COMMA
                            POTHOS_FUNCTION_TEMPLATE_ARGS
                          >
                       >::type type;
      };

      template<
        typename FunctionObj,
        typename R POTHOS_FUNCTION_COMMA
        POTHOS_FUNCTION_TEMPLATE_PARMS
       >
      struct POTHOS_FUNCTION_GET_FUNCTION_REF_INVOKER
      {
        typedef typename mpl::if_c<(is_void<R>::value),
                            POTHOS_FUNCTION_VOID_FUNCTION_REF_INVOKER<
                            FunctionObj,
                            R POTHOS_FUNCTION_COMMA
                            POTHOS_FUNCTION_TEMPLATE_ARGS
                          >,
                          POTHOS_FUNCTION_FUNCTION_REF_INVOKER<
                            FunctionObj,
                            R POTHOS_FUNCTION_COMMA
                            POTHOS_FUNCTION_TEMPLATE_ARGS
                          >
                       >::type type;
      };

#if POTHOS_FUNCTION_NUM_ARGS > 0
      /* Retrieve the appropriate invoker for a member pointer.  */
      template<
        typename MemberPtr,
        typename R POTHOS_FUNCTION_COMMA
        POTHOS_FUNCTION_TEMPLATE_PARMS
       >
      struct POTHOS_FUNCTION_GET_MEMBER_INVOKER
      {
        typedef typename mpl::if_c<(is_void<R>::value),
                            POTHOS_FUNCTION_VOID_MEMBER_INVOKER<
                            MemberPtr,
                            R POTHOS_FUNCTION_COMMA
                            POTHOS_FUNCTION_TEMPLATE_ARGS
                          >,
                          POTHOS_FUNCTION_MEMBER_INVOKER<
                            MemberPtr,
                            R POTHOS_FUNCTION_COMMA
                            POTHOS_FUNCTION_TEMPLATE_ARGS
                          >
                       >::type type;
      };
#endif

      /* Given the tag returned by get_function_tag, retrieve the
         actual invoker that will be used for the given function
         object. 

         Each specialization contains an "apply" nested class template
         that accepts the function object, return type, function
         argument types, and allocator. The resulting "apply" class
         contains two typedefs, "invoker_type" and "manager_type",
         which correspond to the invoker and manager types. */
      template<typename Tag>
      struct POTHOS_FUNCTION_GET_INVOKER { };

      /* Retrieve the invoker for a function pointer. */
      template<>
      struct POTHOS_FUNCTION_GET_INVOKER<function_ptr_tag>
      {
        template<typename FunctionPtr,
                 typename R POTHOS_FUNCTION_COMMA POTHOS_FUNCTION_TEMPLATE_PARMS>
        struct apply
        {
          typedef typename POTHOS_FUNCTION_GET_FUNCTION_INVOKER<
                             FunctionPtr,
                             R POTHOS_FUNCTION_COMMA
                             POTHOS_FUNCTION_TEMPLATE_ARGS
                           >::type
            invoker_type;

          typedef functor_manager<FunctionPtr> manager_type;
        };

        template<typename FunctionPtr,
                 typename R POTHOS_FUNCTION_COMMA POTHOS_FUNCTION_TEMPLATE_PARMS,
                 typename Allocator>
        struct apply_a
        {
          typedef typename POTHOS_FUNCTION_GET_FUNCTION_INVOKER<
                             FunctionPtr,
                             R POTHOS_FUNCTION_COMMA
                             POTHOS_FUNCTION_TEMPLATE_ARGS
                           >::type
            invoker_type;

          typedef functor_manager<FunctionPtr> manager_type;
        };
      };

#if POTHOS_FUNCTION_NUM_ARGS > 0
      /* Retrieve the invoker for a member pointer. */
      template<>
      struct POTHOS_FUNCTION_GET_INVOKER<member_ptr_tag>
      {
        template<typename MemberPtr,
                 typename R POTHOS_FUNCTION_COMMA POTHOS_FUNCTION_TEMPLATE_PARMS>
        struct apply
        {
          typedef typename POTHOS_FUNCTION_GET_MEMBER_INVOKER<
                             MemberPtr,
                             R POTHOS_FUNCTION_COMMA
                             POTHOS_FUNCTION_TEMPLATE_ARGS
                           >::type
            invoker_type;

          typedef functor_manager<MemberPtr> manager_type;
        };

        template<typename MemberPtr,
                 typename R POTHOS_FUNCTION_COMMA POTHOS_FUNCTION_TEMPLATE_PARMS,
                 typename Allocator>
        struct apply_a
        {
          typedef typename POTHOS_FUNCTION_GET_MEMBER_INVOKER<
                             MemberPtr,
                             R POTHOS_FUNCTION_COMMA
                             POTHOS_FUNCTION_TEMPLATE_ARGS
                           >::type
            invoker_type;

          typedef functor_manager<MemberPtr> manager_type;
        };
      };
#endif

      /* Retrieve the invoker for a function object. */
      template<>
      struct POTHOS_FUNCTION_GET_INVOKER<function_obj_tag>
      {
        template<typename FunctionObj,
                 typename R POTHOS_FUNCTION_COMMA POTHOS_FUNCTION_TEMPLATE_PARMS>
        struct apply
        {
          typedef typename POTHOS_FUNCTION_GET_FUNCTION_OBJ_INVOKER<
                             FunctionObj,
                             R POTHOS_FUNCTION_COMMA
                             POTHOS_FUNCTION_TEMPLATE_ARGS
                           >::type
            invoker_type;

          typedef functor_manager<FunctionObj> manager_type;
        };

        template<typename FunctionObj,
                 typename R POTHOS_FUNCTION_COMMA POTHOS_FUNCTION_TEMPLATE_PARMS,
                 typename Allocator>
        struct apply_a
        {
          typedef typename POTHOS_FUNCTION_GET_FUNCTION_OBJ_INVOKER<
                             FunctionObj,
                             R POTHOS_FUNCTION_COMMA
                             POTHOS_FUNCTION_TEMPLATE_ARGS
                           >::type
            invoker_type;

          typedef functor_manager_a<FunctionObj, Allocator> manager_type;
        };
      };

      /* Retrieve the invoker for a reference to a function object. */
      template<>
      struct POTHOS_FUNCTION_GET_INVOKER<function_obj_ref_tag>
      {
        template<typename RefWrapper,
                 typename R POTHOS_FUNCTION_COMMA POTHOS_FUNCTION_TEMPLATE_PARMS>
        struct apply
        {
          typedef typename POTHOS_FUNCTION_GET_FUNCTION_REF_INVOKER<
                             typename RefWrapper::type,
                             R POTHOS_FUNCTION_COMMA
                             POTHOS_FUNCTION_TEMPLATE_ARGS
                           >::type
            invoker_type;

          typedef reference_manager<typename RefWrapper::type> manager_type;
        };

        template<typename RefWrapper,
                 typename R POTHOS_FUNCTION_COMMA POTHOS_FUNCTION_TEMPLATE_PARMS,
                 typename Allocator>
        struct apply_a
        {
          typedef typename POTHOS_FUNCTION_GET_FUNCTION_REF_INVOKER<
                             typename RefWrapper::type,
                             R POTHOS_FUNCTION_COMMA
                             POTHOS_FUNCTION_TEMPLATE_ARGS
                           >::type
            invoker_type;

          typedef reference_manager<typename RefWrapper::type> manager_type;
        };
      };


      /**
       * vtable for a specific Pothos::function instance. This
       * structure must be an aggregate so that we can use static
       * initialization in Pothos::function's assign_to and assign_to_a
       * members. It therefore cannot have any constructors,
       * destructors, base classes, etc.
       */
      template<typename R POTHOS_FUNCTION_COMMA POTHOS_FUNCTION_TEMPLATE_PARMS>
      struct POTHOS_FUNCTION_VTABLE
      {
#ifndef POTHOS_NO_VOID_RETURNS
        typedef R         result_type;
#else
        typedef typename function_return_type<R>::type result_type;
#endif // BOOST_NO_VOID_RETURNS

        typedef result_type (*invoker_type)(function_buffer&
                                            POTHOS_FUNCTION_COMMA
                                            POTHOS_FUNCTION_TEMPLATE_ARGS);

        template<typename F>
        bool assign_to(F f, function_buffer& functor) const
        {
          typedef typename get_function_tag<F>::type tag;
          return assign_to(f, functor, tag());
        }
        template<typename F,typename Allocator>
        bool assign_to_a(F f, function_buffer& functor, Allocator a) const
        {
          typedef typename get_function_tag<F>::type tag;
          return assign_to_a(f, functor, a, tag());
        }

        void clear(function_buffer& functor) const
        {
          if (base.manager)
            base.manager(functor, functor, destroy_functor_tag);
        }

      private:
        // Function pointers
        template<typename FunctionPtr>
        bool 
        assign_to(FunctionPtr f, function_buffer& functor, function_ptr_tag) const
        {
          this->clear(functor);
          if (f) {
            // should be a reinterpret cast, but some compilers insist
            // on giving cv-qualifiers to free functions
            functor.func_ptr = reinterpret_cast<void (*)()>(f);
            return true;
          } else {
            return false;
          }
        }
        template<typename FunctionPtr,typename Allocator>
        bool 
        assign_to_a(FunctionPtr f, function_buffer& functor, Allocator, function_ptr_tag) const
        {
          return assign_to(f,functor,function_ptr_tag());
        }

        // Member pointers
#if POTHOS_FUNCTION_NUM_ARGS > 0
        template<typename MemberPtr>
        bool assign_to(MemberPtr f, function_buffer& functor, member_ptr_tag) const
        {
          // DPG TBD: Add explicit support for member function
          // objects, so we invoke through mem_fn() but we retain the
          // right target_type() values.
          if (f) {
            this->assign_to(Pothos::mem_fn(f), functor);
            return true;
          } else {
            return false;
          }
        }
        template<typename MemberPtr,typename Allocator>
        bool assign_to_a(MemberPtr f, function_buffer& functor, Allocator a, member_ptr_tag) const
        {
          // DPG TBD: Add explicit support for member function
          // objects, so we invoke through mem_fn() but we retain the
          // right target_type() values.
          if (f) {
            this->assign_to_a(Pothos::mem_fn(f), functor, a);
            return true;
          } else {
            return false;
          }
        }
#endif // BOOST_FUNCTION_NUM_ARGS > 0

        // Function objects
        // Assign to a function object using the small object optimization
        template<typename FunctionObj>
        void 
        assign_functor(FunctionObj f, function_buffer& functor, mpl::true_) const
        {
          new (reinterpret_cast<void*>(&functor.data)) FunctionObj(f);
        }
        template<typename FunctionObj,typename Allocator>
        void 
        assign_functor_a(FunctionObj f, function_buffer& functor, Allocator, mpl::true_) const
        {
          assign_functor(f,functor,mpl::true_());
        }

        // Assign to a function object allocated on the heap.
        template<typename FunctionObj>
        void 
        assign_functor(FunctionObj f, function_buffer& functor, mpl::false_) const
        {
          functor.obj_ptr = new FunctionObj(f);
        }
        template<typename FunctionObj,typename Allocator>
        void 
        assign_functor_a(FunctionObj f, function_buffer& functor, Allocator a, mpl::false_) const
        {
          typedef functor_wrapper<FunctionObj,Allocator> functor_wrapper_type;
          typedef typename Allocator::template rebind<functor_wrapper_type>::other
            wrapper_allocator_type;
          typedef typename wrapper_allocator_type::pointer wrapper_allocator_pointer_type;
          wrapper_allocator_type wrapper_allocator(a);
          wrapper_allocator_pointer_type copy = wrapper_allocator.allocate(1);
          wrapper_allocator.construct(copy, functor_wrapper_type(f,a));
          functor_wrapper_type* new_f = static_cast<functor_wrapper_type*>(copy);
          functor.obj_ptr = new_f;
        }

        template<typename FunctionObj>
        bool 
        assign_to(FunctionObj f, function_buffer& functor, function_obj_tag) const
        {
          if (!Pothos::detail::function::has_empty_target(Pothos::addressof(f))) {
            assign_functor(f, functor, 
                           mpl::bool_<(function_allows_small_object_optimization<FunctionObj>::value)>());
            return true;
          } else {
            return false;
          }
        }
        template<typename FunctionObj,typename Allocator>
        bool 
        assign_to_a(FunctionObj f, function_buffer& functor, Allocator a, function_obj_tag) const
        {
          if (!Pothos::detail::function::has_empty_target(Pothos::addressof(f))) {
            assign_functor_a(f, functor, a,
                           mpl::bool_<(function_allows_small_object_optimization<FunctionObj>::value)>());
            return true;
          } else {
            return false;
          }
        }

        // Reference to a function object
        template<typename FunctionObj>
        bool 
        assign_to(const reference_wrapper<FunctionObj>& f, 
                  function_buffer& functor, function_obj_ref_tag) const
        {
          functor.obj_ref.obj_ptr = (void *)(f.get_pointer());
          functor.obj_ref.is_const_qualified = is_const<FunctionObj>::value;
          functor.obj_ref.is_volatile_qualified = is_volatile<FunctionObj>::value;
          return true;
        }
        template<typename FunctionObj,typename Allocator>
        bool 
        assign_to_a(const reference_wrapper<FunctionObj>& f, 
                  function_buffer& functor, Allocator, function_obj_ref_tag) const
        {
          return assign_to(f,functor,function_obj_ref_tag());
        }

      public:
        vtable_base base;
        invoker_type invoker;
      };
    } // end namespace function
  } // end namespace detail

  template<
    typename R POTHOS_FUNCTION_COMMA
    POTHOS_FUNCTION_TEMPLATE_PARMS
  >
  class POTHOS_FUNCTION_FUNCTION : public function_base

#if POTHOS_FUNCTION_NUM_ARGS == 1

    , public std::unary_function<T0,R>

#elif POTHOS_FUNCTION_NUM_ARGS == 2

    , public std::binary_function<T0,T1,R>

#endif

  {
  public:
#ifndef POTHOS_NO_VOID_RETURNS
    typedef R         result_type;
#else
    typedef  typename Pothos::detail::function::function_return_type<R>::type
      result_type;
#endif // BOOST_NO_VOID_RETURNS

  private:
    typedef Pothos::detail::function::POTHOS_FUNCTION_VTABLE<
              R POTHOS_FUNCTION_COMMA POTHOS_FUNCTION_TEMPLATE_ARGS>
      vtable_type;

    vtable_type* get_vtable() const {
      return reinterpret_cast<vtable_type*>(
               reinterpret_cast<std::size_t>(vtable) & ~static_cast<std::size_t>(0x01));
    }

    struct clear_type {};

  public:
    POTHOS_STATIC_CONSTANT(int, args = POTHOS_FUNCTION_NUM_ARGS);

    // add signature for boost::lambda
    template<typename Args>
    struct sig
    {
      typedef result_type type;
    };

#if POTHOS_FUNCTION_NUM_ARGS == 1
    typedef T0 argument_type;
#elif POTHOS_FUNCTION_NUM_ARGS == 2
    typedef T0 first_argument_type;
    typedef T1 second_argument_type;
#endif

    POTHOS_STATIC_CONSTANT(int, arity = POTHOS_FUNCTION_NUM_ARGS);
    POTHOS_FUNCTION_ARG_TYPES

    typedef POTHOS_FUNCTION_FUNCTION self_type;

    POTHOS_FUNCTION_FUNCTION() : function_base() { }

    // MSVC chokes if the following two constructors are collapsed into
    // one with a default parameter.
    template<typename Functor>
    POTHOS_FUNCTION_FUNCTION(Functor POTHOS_FUNCTION_TARGET_FIX(const &) f
#ifndef POTHOS_NO_SFINAE
                            ,typename enable_if_c<
                            (Pothos::type_traits::ice_not<
                             (is_integral<Functor>::value)>::value),
                                        int>::type = 0
#endif // BOOST_NO_SFINAE
                            ) :
      function_base()
    {
      this->assign_to(f);
    }
    template<typename Functor,typename Allocator>
    POTHOS_FUNCTION_FUNCTION(Functor POTHOS_FUNCTION_TARGET_FIX(const &) f, Allocator a
#ifndef POTHOS_NO_SFINAE
                            ,typename enable_if_c<
                            (Pothos::type_traits::ice_not<
                             (is_integral<Functor>::value)>::value),
                                        int>::type = 0
#endif // BOOST_NO_SFINAE
                            ) :
      function_base()
    {
      this->assign_to_a(f,a);
    }

#ifndef POTHOS_NO_SFINAE
    POTHOS_FUNCTION_FUNCTION(clear_type*) : function_base() { }
#else
    POTHOS_FUNCTION_FUNCTION(int zero) : function_base()
    {
      POTHOS_ASSERT(zero == 0);
    }
#endif

    POTHOS_FUNCTION_FUNCTION(const POTHOS_FUNCTION_FUNCTION& f) : function_base()
    {
      this->assign_to_own(f);
    }
    
#ifndef POTHOS_NO_CXX11_RVALUE_REFERENCES
    POTHOS_FUNCTION_FUNCTION(POTHOS_FUNCTION_FUNCTION&& f) : function_base()
    {
      this->move_assign(f);
    }
#endif
    
    ~POTHOS_FUNCTION_FUNCTION() { clear(); }

    result_type operator()(POTHOS_FUNCTION_PARMS) const
    {
      if (this->empty())
        Pothos::throw_exception(bad_function_call());

      return get_vtable()->invoker
               (this->functor POTHOS_FUNCTION_COMMA POTHOS_FUNCTION_ARGS);
    }

    // The distinction between when to use BOOST_FUNCTION_FUNCTION and
    // when to use self_type is obnoxious. MSVC cannot handle self_type as
    // the return type of these assignment operators, but Borland C++ cannot
    // handle BOOST_FUNCTION_FUNCTION as the type of the temporary to
    // construct.
    template<typename Functor>
#ifndef POTHOS_NO_SFINAE
    typename enable_if_c<
               (Pothos::type_traits::ice_not<
                 (is_integral<Functor>::value)>::value),
               POTHOS_FUNCTION_FUNCTION&>::type
#else
    POTHOS_FUNCTION_FUNCTION&
#endif
    operator=(Functor POTHOS_FUNCTION_TARGET_FIX(const &) f)
    {
      this->clear();
      POTHOS_TRY  {
        this->assign_to(f);
      } POTHOS_CATCH (...) {
        vtable = 0;
        POTHOS_RETHROW;
      }
      POTHOS_CATCH_END
      return *this;
    }
    template<typename Functor,typename Allocator>
    void assign(Functor POTHOS_FUNCTION_TARGET_FIX(const &) f, Allocator a)
    {
      this->clear();
      POTHOS_TRY{
        this->assign_to_a(f,a);
      } POTHOS_CATCH (...) {
        vtable = 0;
        POTHOS_RETHROW;
      }
      POTHOS_CATCH_END
    }

#ifndef POTHOS_NO_SFINAE
    POTHOS_FUNCTION_FUNCTION& operator=(clear_type*)
    {
      this->clear();
      return *this;
    }
#else
    POTHOS_FUNCTION_FUNCTION& operator=(int zero)
    {
      POTHOS_ASSERT(zero == 0);
      this->clear();
      return *this;
    }
#endif

    // Assignment from another BOOST_FUNCTION_FUNCTION
    POTHOS_FUNCTION_FUNCTION& operator=(const POTHOS_FUNCTION_FUNCTION& f)
    {
      if (&f == this)
        return *this;

      this->clear();
      POTHOS_TRY {
        this->assign_to_own(f);
      } POTHOS_CATCH (...) {
        vtable = 0;
        POTHOS_RETHROW;
      }
      POTHOS_CATCH_END
      return *this;
    }
    
#ifndef POTHOS_NO_CXX11_RVALUE_REFERENCES
    // Move assignment from another BOOST_FUNCTION_FUNCTION
    POTHOS_FUNCTION_FUNCTION& operator=(POTHOS_FUNCTION_FUNCTION&& f)
    {
      
      if (&f == this)
        return *this;

      this->clear();
      POTHOS_TRY {
        this->move_assign(f);
      } POTHOS_CATCH (...) {
        vtable = 0;
        POTHOS_RETHROW;
      }
      POTHOS_CATCH_END
      return *this;
    }
#endif

    void swap(POTHOS_FUNCTION_FUNCTION& other)
    {
      if (&other == this)
        return;

      POTHOS_FUNCTION_FUNCTION tmp;
      tmp.move_assign(*this);
      this->move_assign(other);
      other.move_assign(tmp);
    }

    // Clear out a target, if there is one
    void clear()
    {
      if (vtable) {
        if (!this->has_trivial_copy_and_destroy())
          get_vtable()->clear(this->functor);
        vtable = 0;
      }
    }

#if (defined __SUNPRO_CC) && (__SUNPRO_CC <= 0x530) && !(defined POTHOS_NO_COMPILER_CONFIG)
    // Sun C++ 5.3 can't handle the safe_bool idiom, so don't use it
    operator bool () const { return !this->empty(); }
#else
  private:
    struct dummy {
      void nonnull() {}
    };

    typedef void (dummy::*safe_bool)();

  public:
    operator safe_bool () const
      { return (this->empty())? 0 : &dummy::nonnull; }

    bool operator!() const
      { return this->empty(); }
#endif

  private:
    void assign_to_own(const POTHOS_FUNCTION_FUNCTION& f)
    {
      if (!f.empty()) {
        this->vtable = f.vtable;
        if (this->has_trivial_copy_and_destroy())
          this->functor = f.functor;
        else
          get_vtable()->base.manager(f.functor, this->functor,
                                     Pothos::detail::function::clone_functor_tag);
      }
    }

    template<typename Functor>
    void assign_to(Functor f)
    {
      using detail::function::vtable_base;

      typedef typename detail::function::get_function_tag<Functor>::type tag;
      typedef detail::function::POTHOS_FUNCTION_GET_INVOKER<tag> get_invoker;
      typedef typename get_invoker::
                         template apply<Functor, R POTHOS_FUNCTION_COMMA 
                        POTHOS_FUNCTION_TEMPLATE_ARGS>
        handler_type;
      
      typedef typename handler_type::invoker_type invoker_type;
      typedef typename handler_type::manager_type manager_type;

      // Note: it is extremely important that this initialization use
      // static initialization. Otherwise, we will have a race
      // condition here in multi-threaded code. See
      // http://thread.gmane.org/gmane.comp.lib.boost.devel/164902/.
      static const vtable_type stored_vtable = 
        { { &manager_type::manage }, &invoker_type::invoke };

      if (stored_vtable.assign_to(f, functor)) {
        std::size_t value = reinterpret_cast<std::size_t>(&stored_vtable.base);
        if (Pothos::has_trivial_copy_constructor<Functor>::value &&
            Pothos::has_trivial_destructor<Functor>::value &&
            detail::function::function_allows_small_object_optimization<Functor>::value)
          value |= static_cast<size_t>(0x01);
        vtable = reinterpret_cast<detail::function::vtable_base *>(value);
      } else 
        vtable = 0;
    }

    template<typename Functor,typename Allocator>
    void assign_to_a(Functor f,Allocator a)
    {
      using detail::function::vtable_base;

      typedef typename detail::function::get_function_tag<Functor>::type tag;
      typedef detail::function::POTHOS_FUNCTION_GET_INVOKER<tag> get_invoker;
      typedef typename get_invoker::
                         template apply_a<Functor, R POTHOS_FUNCTION_COMMA 
                         POTHOS_FUNCTION_TEMPLATE_ARGS,
                         Allocator>
        handler_type;
      
      typedef typename handler_type::invoker_type invoker_type;
      typedef typename handler_type::manager_type manager_type;

      // Note: it is extremely important that this initialization use
      // static initialization. Otherwise, we will have a race
      // condition here in multi-threaded code. See
      // http://thread.gmane.org/gmane.comp.lib.boost.devel/164902/.
      static const vtable_type stored_vtable =
        { { &manager_type::manage }, &invoker_type::invoke };

      if (stored_vtable.assign_to_a(f, functor, a)) { 
        std::size_t value = reinterpret_cast<std::size_t>(&stored_vtable.base);
        if (Pothos::has_trivial_copy_constructor<Functor>::value &&
            Pothos::has_trivial_destructor<Functor>::value &&
            detail::function::function_allows_small_object_optimization<Functor>::value)
          value |= static_cast<std::size_t>(0x01);
        vtable = reinterpret_cast<detail::function::vtable_base *>(value);
      } else 
        vtable = 0;
    }

    // Moves the value from the specified argument to *this. If the argument 
    // has its function object allocated on the heap, move_assign will pass 
    // its buffer to *this, and set the argument's buffer pointer to NULL. 
    void move_assign(POTHOS_FUNCTION_FUNCTION& f) 
    { 
      if (&f == this)
        return;

      POTHOS_TRY {
        if (!f.empty()) {
          this->vtable = f.vtable;
          if (this->has_trivial_copy_and_destroy())
            this->functor = f.functor;
          else
            get_vtable()->base.manager(f.functor, this->functor,
                                     Pothos::detail::function::move_functor_tag);
          f.vtable = 0;
        } else {
          clear();
        }
      } POTHOS_CATCH (...) {
        vtable = 0;
        POTHOS_RETHROW;
      }
      POTHOS_CATCH_END
    }
  };

  template<typename R POTHOS_FUNCTION_COMMA POTHOS_FUNCTION_TEMPLATE_PARMS>
  inline void swap(POTHOS_FUNCTION_FUNCTION<
                     R POTHOS_FUNCTION_COMMA
                     POTHOS_FUNCTION_TEMPLATE_ARGS
                   >& f1,
                   POTHOS_FUNCTION_FUNCTION<
                     R POTHOS_FUNCTION_COMMA
                     POTHOS_FUNCTION_TEMPLATE_ARGS
                   >& f2)
  {
    f1.swap(f2);
  }

// Poison comparisons between boost::function objects of the same type.
template<typename R POTHOS_FUNCTION_COMMA POTHOS_FUNCTION_TEMPLATE_PARMS>
  void operator==(const POTHOS_FUNCTION_FUNCTION<
                          R POTHOS_FUNCTION_COMMA
                          POTHOS_FUNCTION_TEMPLATE_ARGS>&,
                  const POTHOS_FUNCTION_FUNCTION<
                          R POTHOS_FUNCTION_COMMA
                          POTHOS_FUNCTION_TEMPLATE_ARGS>&);
template<typename R POTHOS_FUNCTION_COMMA POTHOS_FUNCTION_TEMPLATE_PARMS>
  void operator!=(const POTHOS_FUNCTION_FUNCTION<
                          R POTHOS_FUNCTION_COMMA
                          POTHOS_FUNCTION_TEMPLATE_ARGS>&,
                  const POTHOS_FUNCTION_FUNCTION<
                          R POTHOS_FUNCTION_COMMA
                          POTHOS_FUNCTION_TEMPLATE_ARGS>& );

#if !defined(POTHOS_FUNCTION_NO_FUNCTION_TYPE_SYNTAX)

#if POTHOS_FUNCTION_NUM_ARGS == 0
#define POTHOS_FUNCTION_PARTIAL_SPEC R (void)
#else
#define POTHOS_FUNCTION_PARTIAL_SPEC R (POTHOS_PP_ENUM_PARAMS(POTHOS_FUNCTION_NUM_ARGS,T))
#endif

template<typename R POTHOS_FUNCTION_COMMA
         POTHOS_FUNCTION_TEMPLATE_PARMS>
class function<POTHOS_FUNCTION_PARTIAL_SPEC>
  : public POTHOS_FUNCTION_FUNCTION<R POTHOS_FUNCTION_COMMA POTHOS_FUNCTION_TEMPLATE_ARGS>
{
  typedef POTHOS_FUNCTION_FUNCTION<R POTHOS_FUNCTION_COMMA POTHOS_FUNCTION_TEMPLATE_ARGS> base_type;
  typedef function self_type;

  struct clear_type {};

public:

  function() : base_type() {}

  template<typename Functor>
  function(Functor f
#ifndef POTHOS_NO_SFINAE
           ,typename enable_if_c<
                            (Pothos::type_traits::ice_not<
                          (is_integral<Functor>::value)>::value),
                       int>::type = 0
#endif
           ) :
    base_type(f)
  {
  }
  template<typename Functor,typename Allocator>
  function(Functor f, Allocator a
#ifndef POTHOS_NO_SFINAE
           ,typename enable_if_c<
                            (Pothos::type_traits::ice_not<
                          (is_integral<Functor>::value)>::value),
                       int>::type = 0
#endif
           ) :
    base_type(f,a)
  {
  }

#ifndef POTHOS_NO_SFINAE
  function(clear_type*) : base_type() {}
#endif

  function(const self_type& f) : base_type(static_cast<const base_type&>(f)){}

  function(const base_type& f) : base_type(static_cast<const base_type&>(f)){}

#ifndef POTHOS_NO_CXX11_RVALUE_REFERENCES
  // Move constructors
  function(self_type&& f): base_type(static_cast<base_type&&>(f)){}
  function(base_type&& f): base_type(static_cast<base_type&&>(f)){}
#endif
  
  self_type& operator=(const self_type& f)
  {
    self_type(f).swap(*this);
    return *this;
  }

#ifndef POTHOS_NO_CXX11_RVALUE_REFERENCES
  self_type& operator=(self_type&& f)
  {
    self_type(static_cast<self_type&&>(f)).swap(*this);
    return *this;
  }
#endif  

  template<typename Functor>
#ifndef POTHOS_NO_SFINAE
  typename enable_if_c<
                            (Pothos::type_traits::ice_not<
                         (is_integral<Functor>::value)>::value),
                      self_type&>::type
#else
  self_type&
#endif
  operator=(Functor f)
  {
    self_type(f).swap(*this);
    return *this;
  }

#ifndef POTHOS_NO_SFINAE
  self_type& operator=(clear_type*)
  {
    this->clear();
    return *this;
  }
#endif

  self_type& operator=(const base_type& f)
  {
    self_type(f).swap(*this);
    return *this;
  }
  
#ifndef POTHOS_NO_CXX11_RVALUE_REFERENCES
  self_type& operator=(base_type&& f)
  {
    self_type(static_cast<base_type&&>(f)).swap(*this);
    return *this;
  }
#endif 
};

#undef POTHOS_FUNCTION_PARTIAL_SPEC
#endif // have partial specialization

} // end namespace boost

// Cleanup after ourselves...
#undef POTHOS_FUNCTION_VTABLE
#undef POTHOS_FUNCTION_COMMA
#undef POTHOS_FUNCTION_FUNCTION
#undef POTHOS_FUNCTION_FUNCTION_INVOKER
#undef POTHOS_FUNCTION_VOID_FUNCTION_INVOKER
#undef POTHOS_FUNCTION_FUNCTION_OBJ_INVOKER
#undef POTHOS_FUNCTION_VOID_FUNCTION_OBJ_INVOKER
#undef POTHOS_FUNCTION_FUNCTION_REF_INVOKER
#undef POTHOS_FUNCTION_VOID_FUNCTION_REF_INVOKER
#undef POTHOS_FUNCTION_MEMBER_INVOKER
#undef POTHOS_FUNCTION_VOID_MEMBER_INVOKER
#undef POTHOS_FUNCTION_GET_FUNCTION_INVOKER
#undef POTHOS_FUNCTION_GET_FUNCTION_OBJ_INVOKER
#undef POTHOS_FUNCTION_GET_FUNCTION_REF_INVOKER
#undef POTHOS_FUNCTION_GET_MEM_FUNCTION_INVOKER
#undef POTHOS_FUNCTION_GET_INVOKER
#undef POTHOS_FUNCTION_TEMPLATE_PARMS
#undef POTHOS_FUNCTION_TEMPLATE_ARGS
#undef POTHOS_FUNCTION_PARMS
#undef POTHOS_FUNCTION_PARM
#undef POTHOS_FUNCTION_ARGS
#undef POTHOS_FUNCTION_ARG_TYPE
#undef POTHOS_FUNCTION_ARG_TYPES
#undef POTHOS_FUNCTION_VOID_RETURN_TYPE
#undef POTHOS_FUNCTION_RETURN

#if defined(POTHOS_MSVC)
#   pragma warning( pop )
#endif       
