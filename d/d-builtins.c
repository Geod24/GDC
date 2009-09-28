/* GDC -- D front-end for GCC
   Copyright (C) 2004 David Friedman
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* This file is mostly a copy of gcc/c-common.c */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "intl.h"
#include "tree.h"
#include "flags.h"
#include "output.h"
#include "rtl.h"
#include "ggc.h"
#include "varray.h"
#include "expr.h"
#include "diagnostic.h"
#include "tm_p.h"
#include "target.h"
#include "langhooks.h"
#include "tree-inline.h"
#include "toplev.h"
#if D_GCC_VER >= 40
/* Only used in >= 4.1.x */
#include "cgraph.h"
#endif

#include "d-lang.h"

tree intmax_type_node;
tree uintmax_type_node;
tree signed_size_type_node;
tree string_type_node;
tree const_string_type_node;

#if D_GCC_VER >= 41
tree null_node;
extern void dkeep(tree t);
#endif


#if D_GCC_VER < 40
# ifdef D_GCC_VER341
#  include "d-bi-attrs-341.h"
# else
#  include "d-bi-attrs-34.h"
# endif
#elif D_GCC_VER < 41
#include "d-bi-attrs-40.h"
#elif D_GCC_VER < 43
#include "d-bi-attrs-41.h"
#else
#include "d-bi-attrs-43.h"
#endif

#if D_GCC_VER >= 41
/* Nonzero means enable C89 Amendment 1 features.  */

/*extern*/ int flag_isoc94;

/* Nonzero means use the ISO C99 dialect of C.  */

/*extern*/ int flag_isoc99;
#endif

/* Used to help initialize the builtin-types.def table.  When a type of
   the correct size doesn't exist, use error_mark_node instead of NULL.
   The later results in segfaults even when a decl using the type doesn't
   get invoked.  */

tree
builtin_type_for_size (int size, bool unsignedp)
{
  tree type = lang_hooks.types.type_for_size (size, unsignedp);
  return type ? type : error_mark_node;
}

enum built_in_attribute
{
#define DEF_ATTR_NULL_TREE(ENUM) ENUM,
#define DEF_ATTR_INT(ENUM, VALUE) ENUM,
#define DEF_ATTR_IDENT(ENUM, STRING) ENUM,
#define DEF_ATTR_TREE_LIST(ENUM, PURPOSE, VALUE, CHAIN) ENUM,
#include "builtin-attrs.def"
#undef DEF_ATTR_NULL_TREE
#undef DEF_ATTR_INT
#undef DEF_ATTR_IDENT
#undef DEF_ATTR_TREE_LIST
  ATTR_LAST
};

static GTY(()) tree built_in_attributes[(int) ATTR_LAST];

static void
d_init_attributes (void)
{
  /* Fill in the built_in_attributes array.  */
#define DEF_ATTR_NULL_TREE(ENUM)		\
  built_in_attributes[(int) ENUM] = NULL_TREE;
#if D_GCC_VER < 40
# define DEF_ATTR_INT(ENUM, VALUE)					     \
  built_in_attributes[(int) ENUM] = build_int_2 (VALUE, VALUE < 0 ? -1 : 0);
#else
# define DEF_ATTR_INT(ENUM, VALUE)					     \
  built_in_attributes[(int) ENUM] = build_int_cst (NULL_TREE, VALUE);
#endif
#define DEF_ATTR_IDENT(ENUM, STRING)				\
  built_in_attributes[(int) ENUM] = get_identifier (STRING);
#define DEF_ATTR_TREE_LIST(ENUM, PURPOSE, VALUE, CHAIN)	\
  built_in_attributes[(int) ENUM]			\
    = tree_cons (built_in_attributes[(int) PURPOSE],	\
		 built_in_attributes[(int) VALUE],	\
		 built_in_attributes[(int) CHAIN]);
#include "builtin-attrs.def"
#undef DEF_ATTR_NULL_TREE
#undef DEF_ATTR_INT
#undef DEF_ATTR_IDENT
#undef DEF_ATTR_TREE_LIST
}


#if D_GCC_VER >= 40
#ifndef WINT_TYPE
#define WINT_TYPE "unsigned int"
#endif
#ifndef PID_TYPE
#define PID_TYPE "int"
#endif

static tree
lookup_C_type_name(const char * p)
{
    // These are the names used in c_common_nodes_and_builtins
    if (strcmp(p,"char")) return char_type_node;
    else if (strcmp(p,"signed char")) return signed_char_type_node;
    else if (strcmp(p,"unsigned char")) return unsigned_char_type_node;
    else if (strcmp(p,"short int")) return short_integer_type_node;
    else if (strcmp(p,"short unsigned int ")) return short_unsigned_type_node;//cxx! -- affects ming/c++?
    else if (strcmp(p,"int")) return integer_type_node;
    else if (strcmp(p,"unsigned int")) return unsigned_type_node;
    else if (strcmp(p,"long int")) return long_integer_type_node;
    else if (strcmp(p,"long unsigned int")) return long_unsigned_type_node; // cxx!
    else if (strcmp(p,"long long int")) return long_integer_type_node;
    else if (strcmp(p,"long long unsigned int")) return long_unsigned_type_node; // cxx!
    internal_error("unsigned C type '%s'", p);
}
#endif

#if D_GCC_VER >= 41
void do_build_builtin_fn(enum built_in_function fncode,
    const char *name,
    enum built_in_class fnclass,
    int fntype_index, tree fntype, int libtype_index,
    bool both_p, bool fallback_p, bool nonansi_p,
    tree fnattrs, bool implicit_p)
{
    tree decl;							

    const char *libname; 

    gcc_assert ((!(both_p) && !(fallback_p))     
	|| !strncmp (name, "__builtin_", 
	    strlen ("__builtin_"))); 
    libname = name + strlen ("__builtin_"); 


    /*if (!BOTH_P)*/
#if D_GCC_VER >= 43
    decl = add_builtin_function(name, fntype, fncode, fnclass,
	fallback_p ?libname : NULL, fnattrs);
#else
    decl = lang_hooks.builtin_function (name, fntype,	
	fncode,					
	fnclass,					
	fallback_p ?libname : NULL,				
	fnattrs);
#endif

    built_in_decls[(int) fncode] = decl;				
    if (implicit_p)							
	implicit_built_in_decls[(int) fncode] = decl;			
}
#endif

void d_init_builtins(void)
{
  enum builtin_type
  {
#define DEF_PRIMITIVE_TYPE(NAME, VALUE) NAME,
#define DEF_FUNCTION_TYPE_0(NAME, RETURN) NAME,
#define DEF_FUNCTION_TYPE_1(NAME, RETURN, ARG1) NAME,
#define DEF_FUNCTION_TYPE_2(NAME, RETURN, ARG1, ARG2) NAME,
#define DEF_FUNCTION_TYPE_3(NAME, RETURN, ARG1, ARG2, ARG3) NAME,
#define DEF_FUNCTION_TYPE_4(NAME, RETURN, ARG1, ARG2, ARG3, ARG4) NAME,
#define DEF_FUNCTION_TYPE_5(NAME, RETURN, ARG1, ARG2, ARG3, ARG4, ARG5) NAME,
#define DEF_FUNCTION_TYPE_6(NAME, RETURN, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) NAME,
#define DEF_FUNCTION_TYPE_7(NAME, RETURN, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) NAME,
#define DEF_FUNCTION_TYPE_VAR_0(NAME, RETURN) NAME,
#define DEF_FUNCTION_TYPE_VAR_1(NAME, RETURN, ARG1) NAME,
#define DEF_FUNCTION_TYPE_VAR_2(NAME, RETURN, ARG1, ARG2) NAME,
#define DEF_FUNCTION_TYPE_VAR_3(NAME, RETURN, ARG1, ARG2, ARG3) NAME,
#define DEF_FUNCTION_TYPE_VAR_4(NAME, RETURN, ARG1, ARG2, ARG3, ARG4) NAME,
#define DEF_FUNCTION_TYPE_VAR_5(NAME, RETURN, ARG1, ARG2, ARG3, ARG4, ARG6) NAME,
#define DEF_POINTER_TYPE(NAME, TYPE) NAME,
#include "builtin-types.def"
#undef DEF_PRIMITIVE_TYPE
#undef DEF_FUNCTION_TYPE_0
#undef DEF_FUNCTION_TYPE_1
#undef DEF_FUNCTION_TYPE_2
#undef DEF_FUNCTION_TYPE_3
#undef DEF_FUNCTION_TYPE_4
#undef DEF_FUNCTION_TYPE_5
#undef DEF_FUNCTION_TYPE_6
#undef DEF_FUNCTION_TYPE_7
#undef DEF_FUNCTION_TYPE_VAR_0
#undef DEF_FUNCTION_TYPE_VAR_1
#undef DEF_FUNCTION_TYPE_VAR_2
#undef DEF_FUNCTION_TYPE_VAR_3
#undef DEF_FUNCTION_TYPE_VAR_4
#undef DEF_FUNCTION_TYPE_VAR_5
#undef DEF_POINTER_TYPE
    BT_LAST
  };
  typedef enum builtin_type builtin_type;

  tree builtin_types[(int) BT_LAST];
  
  tree va_list_ref_type_node;
  tree va_list_arg_type_node;

  d_bi_init((int) BT_LAST, (int) END_BUILTINS);
    
    
  if (TREE_CODE (va_list_type_node) == ARRAY_TYPE)
    {
	/* It might seem natural to make the reference type a pointer,
	   but this will not work in D: There is no implicit casting from
	   an array to a pointer. */
	va_list_arg_type_node = va_list_ref_type_node = va_list_type_node;
    }
  else
    {
	va_list_arg_type_node = va_list_type_node;
	va_list_ref_type_node = build_reference_type (va_list_type_node);
    }

  
  intmax_type_node = intDI_type_node;
  uintmax_type_node = unsigned_intDI_type_node;
  signed_size_type_node = d_signed_type(size_type_node);
  string_type_node = build_pointer_type (char_type_node);
  const_string_type_node = build_pointer_type (build_qualified_type
      (char_type_node, TYPE_QUAL_CONST));

  void_list_node = tree_cons(NULL_TREE, void_type_node, NULL_TREE);

#if D_GCC_VER >= 40
  /* WINT_TYPE is a C type name, not an itk_ constant or something useful
     like that... */
  tree wint_type_node = lookup_C_type_name(WINT_TYPE);
  pid_type_node = lookup_C_type_name(PID_TYPE);
#endif

#define DEF_PRIMITIVE_TYPE(ENUM, VALUE) \
  builtin_types[(int) ENUM] = VALUE;
#define DEF_FUNCTION_TYPE_0(ENUM, RETURN)		\
  builtin_types[(int) ENUM]				\
    = build_function_type (builtin_types[(int) RETURN],	\
			   void_list_node);
#define DEF_FUNCTION_TYPE_1(ENUM, RETURN, ARG1)				\
  builtin_types[(int) ENUM]						\
    = build_function_type (builtin_types[(int) RETURN],			\
			   tree_cons (NULL_TREE,			\
				      builtin_types[(int) ARG1],	\
				      void_list_node));
#define DEF_FUNCTION_TYPE_2(ENUM, RETURN, ARG1, ARG2)	\
  builtin_types[(int) ENUM]				\
    = build_function_type				\
      (builtin_types[(int) RETURN],			\
       tree_cons (NULL_TREE,				\
		  builtin_types[(int) ARG1],		\
		  tree_cons (NULL_TREE,			\
			     builtin_types[(int) ARG2],	\
			     void_list_node)));
#define DEF_FUNCTION_TYPE_3(ENUM, RETURN, ARG1, ARG2, ARG3)		 \
  builtin_types[(int) ENUM]						 \
    = build_function_type						 \
      (builtin_types[(int) RETURN],					 \
       tree_cons (NULL_TREE,						 \
		  builtin_types[(int) ARG1],				 \
		  tree_cons (NULL_TREE,					 \
			     builtin_types[(int) ARG2],			 \
			     tree_cons (NULL_TREE,			 \
					builtin_types[(int) ARG3],	 \
					void_list_node))));
#define DEF_FUNCTION_TYPE_4(ENUM, RETURN, ARG1, ARG2, ARG3, ARG4)	\
  builtin_types[(int) ENUM]						\
    = build_function_type						\
      (builtin_types[(int) RETURN],					\
       tree_cons (NULL_TREE,						\
		  builtin_types[(int) ARG1],				\
		  tree_cons (NULL_TREE,					\
			     builtin_types[(int) ARG2],			\
			     tree_cons					\
			     (NULL_TREE,				\
			      builtin_types[(int) ARG3],		\
			      tree_cons (NULL_TREE,			\
					 builtin_types[(int) ARG4],	\
					 void_list_node)))));
#define DEF_FUNCTION_TYPE_5(ENUM, RETURN, ARG1, ARG2, ARG3, ARG4, ARG5)	\
  builtin_types[(int) ENUM]						\
    = build_function_type						\
      (builtin_types[(int) RETURN],					\
       tree_cons (NULL_TREE,						\
		  builtin_types[(int) ARG1],				\
		  tree_cons (NULL_TREE,					\
			     builtin_types[(int) ARG2],			\
			     tree_cons					\
			     (NULL_TREE,				\
			      builtin_types[(int) ARG3],		\
			      tree_cons (NULL_TREE,			\
					 builtin_types[(int) ARG4],	\
					 tree_cons (NULL_TREE,		\
					      builtin_types[(int) ARG5],\
					      void_list_node))))));
#define DEF_FUNCTION_TYPE_6(ENUM, RETURN, ARG1, ARG2, ARG3, ARG4, ARG5, \
			    ARG6)					\
  builtin_types[(int) ENUM]						\
    = build_function_type						\
      (builtin_types[(int) RETURN],					\
       tree_cons (NULL_TREE,						\
		  builtin_types[(int) ARG1],				\
		  tree_cons (NULL_TREE,					\
			     builtin_types[(int) ARG2],			\
			     tree_cons					\
			     (NULL_TREE,				\
			      builtin_types[(int) ARG3],		\
			      tree_cons					\
			      (NULL_TREE,				\
			       builtin_types[(int) ARG4],		\
			       tree_cons (NULL_TREE,			\
					 builtin_types[(int) ARG5],	\
					 tree_cons (NULL_TREE,		\
					      builtin_types[(int) ARG6],\
					      void_list_node)))))));
#define DEF_FUNCTION_TYPE_7(ENUM, RETURN, ARG1, ARG2, ARG3, ARG4, ARG5, \
			    ARG6, ARG7)					\
  builtin_types[(int) ENUM]						\
    = build_function_type						\
      (builtin_types[(int) RETURN],					\
       tree_cons (NULL_TREE,						\
		  builtin_types[(int) ARG1],				\
		  tree_cons (NULL_TREE,					\
			     builtin_types[(int) ARG2],			\
			     tree_cons					\
			     (NULL_TREE,				\
			      builtin_types[(int) ARG3],		\
			      tree_cons					\
			      (NULL_TREE,				\
			       builtin_types[(int) ARG4],		\
			       tree_cons (NULL_TREE,			\
					 builtin_types[(int) ARG5],	\
					 tree_cons (NULL_TREE,		\
					      builtin_types[(int) ARG6],\
					      tree_cons(NULL_TREE,builtin_types[(int) ARG7], \
					      void_list_node))))))));
#define DEF_FUNCTION_TYPE_VAR_0(ENUM, RETURN)				\
  builtin_types[(int) ENUM]						\
    = build_function_type (builtin_types[(int) RETURN], NULL_TREE);
#define DEF_FUNCTION_TYPE_VAR_1(ENUM, RETURN, ARG1)			 \
   builtin_types[(int) ENUM]						 \
    = build_function_type (builtin_types[(int) RETURN],		 \
			   tree_cons (NULL_TREE,			 \
				      builtin_types[(int) ARG1],	 \
				      NULL_TREE));

#define DEF_FUNCTION_TYPE_VAR_2(ENUM, RETURN, ARG1, ARG2)	\
   builtin_types[(int) ENUM]					\
    = build_function_type					\
      (builtin_types[(int) RETURN],				\
       tree_cons (NULL_TREE,					\
		  builtin_types[(int) ARG1],			\
		  tree_cons (NULL_TREE,				\
			     builtin_types[(int) ARG2],		\
			     NULL_TREE)));

#define DEF_FUNCTION_TYPE_VAR_3(ENUM, RETURN, ARG1, ARG2, ARG3)		\
   builtin_types[(int) ENUM]						\
    = build_function_type						\
      (builtin_types[(int) RETURN],					\
       tree_cons (NULL_TREE,						\
		  builtin_types[(int) ARG1],				\
		  tree_cons (NULL_TREE,					\
			     builtin_types[(int) ARG2],			\
			     tree_cons (NULL_TREE,			\
					builtin_types[(int) ARG3],	\
					NULL_TREE))));
#define DEF_FUNCTION_TYPE_VAR_4(ENUM, RETURN, ARG1, ARG2, ARG3, ARG4)	\
   builtin_types[(int) ENUM]						\
    = build_function_type						\
      (builtin_types[(int) RETURN],					\
       tree_cons (NULL_TREE,						\
		  builtin_types[(int) ARG1],				\
		  tree_cons (NULL_TREE,					\
			     builtin_types[(int) ARG2],			\
			     tree_cons (NULL_TREE,			\
					builtin_types[(int) ARG3],	\
					tree_cons (NULL_TREE,		\
					      builtin_types[(int) ARG4],\
					      NULL_TREE)))));

#define DEF_FUNCTION_TYPE_VAR_5(ENUM, RETURN, ARG1, ARG2, ARG3, ARG4,	\
				ARG5)					\
   builtin_types[(int) ENUM]						\
    = build_function_type						\
      (builtin_types[(int) RETURN],					\
       tree_cons (NULL_TREE,						\
		  builtin_types[(int) ARG1],				\
		  tree_cons (NULL_TREE,					\
			     builtin_types[(int) ARG2],			\
			     tree_cons					\
			     (NULL_TREE,				\
			      builtin_types[(int) ARG3],		\
			      tree_cons (NULL_TREE,			\
					builtin_types[(int) ARG4],	\
					tree_cons (NULL_TREE,		\
					      builtin_types[(int) ARG5],\
					      NULL_TREE))))));


#define DEF_POINTER_TYPE(ENUM, TYPE)			\
  builtin_types[(int) ENUM]				\
    = build_pointer_type (builtin_types[(int) TYPE]);
#include "builtin-types.def"
#undef DEF_PRIMITIVE_TYPE
#undef DEF_FUNCTION_TYPE_1
#undef DEF_FUNCTION_TYPE_2
#undef DEF_FUNCTION_TYPE_3
#undef DEF_FUNCTION_TYPE_4
#undef DEF_FUNCTION_TYPE_5
#undef DEF_FUNCTION_TYPE_6
#undef DEF_FUNCTION_TYPE_VAR_0
#undef DEF_FUNCTION_TYPE_VAR_1
#undef DEF_FUNCTION_TYPE_VAR_2
#undef DEF_FUNCTION_TYPE_VAR_3
#undef DEF_FUNCTION_TYPE_VAR_4
#undef DEF_FUNCTION_TYPE_VAR_5
#undef DEF_POINTER_TYPE

  d_init_attributes ();

#if D_GCC_VER == 34
#define DEF_BUILTIN(ENUM, NAME, CLASS, TYPE, LIBTYPE,			\
		    BOTH_P, FALLBACK_P, NONANSI_P, ATTRS, IMPLICIT)	\
  if (NAME)								\
    {									\
      tree decl;							\
									\
      if (strncmp (NAME, "__builtin_", strlen ("__builtin_")) != 0)	\
	abort ();							\
									\
      /*if (!BOTH_P)*/							\
	decl = builtin_function (NAME, builtin_types[TYPE], ENUM,	\
				 CLASS,					\
				 (FALLBACK_P				\
				  ? (NAME + strlen ("__builtin_"))	\
				  : NULL),				\
				 built_in_attributes[(int) ATTRS]);	\
      /*else*/  			        			\
	/*decl = builtin_function_2 (NAME,*/      			\
	/*        		   NAME + strlen ("__builtin_"),*/	\
	/*        		   builtin_types[TYPE],*/		        \
	/*        		   builtin_types[LIBTYPE],*/		\
	/*        		   ENUM,*/        	        	\
	/*        		   CLASS,*/       			\
	/*        		   FALLBACK_P,*/				\
	/*        		   NONANSI_P,*/			        \
	/*        		   built_in_attributes[(int) ATTRS]);*/	\
									\
      built_in_decls[(int) ENUM] = decl;				\
      if (IMPLICIT)							\
        implicit_built_in_decls[(int) ENUM] = decl;			\
    }
#elif D_GCC_VER == 40
#define DEF_BUILTIN(ENUM, NAME, CLASS, TYPE, LIBTYPE, BOTH_P, FALLBACK_P, \
		    NONANSI_P, ATTRS, IMPLICIT, COND)			\
  if (NAME && COND)							\
    {									\
      tree decl;							\
									\
      gcc_assert (!strncmp (NAME, "__builtin_",				\
			    strlen ("__builtin_")));			\
									\
      /*if (!BOTH_P)*/							\
	decl = lang_hooks.builtin_function (NAME, builtin_types[TYPE],	\
				 ENUM,					\
				 CLASS,					\
				 (FALLBACK_P				\
				  ? (NAME + strlen ("__builtin_"))	\
				  : NULL),				\
				 built_in_attributes[(int) ATTRS]);	\
      /*else*/								\
	/*decl = builtin_function_2 (NAME,*/				\
	/*			   NAME + strlen ("__builtin_"),*/	\
	/*			   builtin_types[TYPE],*/			\
	/*			   builtin_types[LIBTYPE],*/		\
	/*			   ENUM,*/				\
	/*			   CLASS,*/				\
	/*			   FALLBACK_P,*/				\
	/*			   NONANSI_P,*/				\
	/*			   built_in_attributes[(int) ATTRS]);*/	\
									\
      built_in_decls[(int) ENUM] = decl;				\
      if (IMPLICIT)							\
	implicit_built_in_decls[(int) ENUM] = decl;			\
    }
#elif D_GCC_VER >= 41
#define DEF_BUILTIN(ENUM, NAME, CLASS, TYPE, LIBTYPE, BOTH_P, FALLBACK_P, \
		    NONANSI_P, ATTRS, IMPLICIT, COND)			\
  if (NAME && COND)							\
      do_build_builtin_fn(ENUM, NAME, CLASS, TYPE, builtin_types[TYPE], LIBTYPE, BOTH_P, \
	  FALLBACK_P, NONANSI_P, built_in_attributes[(int) ATTRS], IMPLICIT);
#endif
  
#include "builtins.def"
#undef DEF_BUILTIN

#if D_GCC_VER >= 40
  build_common_builtin_nodes ();
#endif
  
  (*targetm.init_builtins) ();
    
  main_identifier_node = get_identifier ("main");

#if D_GCC_VER >= 41
  /* Create the built-in __null node.  It is important that this is
     not shared.  */
  null_node = make_node (INTEGER_CST);
  TREE_TYPE (null_node) = d_type_for_size (POINTER_SIZE, 0);
  dkeep(null_node);
#endif
}

/* Return a definition for a builtin function named NAME and whose data type
   is TYPE.  TYPE should be a function type with argument types.
   FUNCTION_CODE tells later passes how to compile calls to this function.
   See tree.h for its possible values.

   If LIBRARY_NAME is nonzero, use that for DECL_ASSEMBLER_NAME,
   the name to be called if we can't opencode the function.  If
   ATTRS is nonzero, use that for the function's attribute list.  */

#if D_GCC_VER >= 43

tree
d_builtin_function43 (tree decl)
{
  d_bi_builtin_func(decl);

  return decl;
}

#else

tree
d_builtin_function (const char *name, tree type, int function_code,
		    enum built_in_class klass, const char *library_name,
		    tree attrs)
{
  // As of 4.3.x, this is done by add_builtin_fucntion 
    //%% for D, just use library_name?
  tree decl = build_decl (FUNCTION_DECL, get_identifier (name), type);
  DECL_EXTERNAL (decl) = 1;
  TREE_PUBLIC (decl) = 1;
  if (library_name)
    SET_DECL_ASSEMBLER_NAME (decl, get_identifier (library_name));
#if D_GCC_VER < 40
  make_decl_rtl (decl, NULL);
#endif
  // %% gcc4 -- is make_decl_rtl needed? why are we doing it in gcc3?
  // shouldn't it go after attributes?
  
  pushdecl (decl);
  DECL_BUILT_IN_CLASS (decl) = klass;
  DECL_FUNCTION_CODE (decl) = function_code;

  /* Warn if a function in the namespace for users
     is used without an occasion to consider it declared.  */
  /*
  if (name[0] != '_' || name[1] != '_')
    C_DECL_INVISIBLE (decl) = 1;
  */

  /* Possibly apply some default attributes to this built-in function.  */
  if (attrs)
    decl_attributes (&decl, attrs, ATTR_FLAG_BUILT_IN);
  else
    decl_attributes (&decl, NULL_TREE, 0);

  d_bi_builtin_func(decl);

  return decl;
}

#endif

#include "gt-d-d-builtins.h"