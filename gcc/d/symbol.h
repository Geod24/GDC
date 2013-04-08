// symbol.h -- D frontend for GCC.
// Copyright (C) 2011, 2012 Free Software Foundation, Inc.

// GCC is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3, or (at your option) any later
// version.

// GCC is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.

// You should have received a copy of the GNU General Public License
// along with GCC; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

#ifndef DMD_SYMBOL_H
#define DMD_SYMBOL_H

#include "config.h"
#include "coretypes.h"

#include "root.h"
#include "mtype.h"

enum SymbolStorageClass
{
  SC_INVALID,
  SCextern,
  SCstatic,
  SCauto,
  SCglobal,
  SCstruct,
  SCcomdat
};

typedef enum SymbolStorageClass enum_SC;

enum SymbolFL
{
  FL_INVALID,
  FLextern = 0x01,
  FLdata   = 0x02,
};

enum SymbolFlag
{
  SFLnodebug = 0x01,
  SFLweak    = 0x02
};

enum SymbolSegment
{
  INVALID,
  DATA,
  CDATA,
  UDATA
};

// not sure if this needs to inherit object..
// union tree_node; typedef union tree_node dt_t;
struct dt_t;

enum OutputStage
{
  NotStarted,
  InProgress,
  Finished
};

struct FuncFrameInfo;
typedef ArrayBase<struct Thunk> Thunks;

struct Symbol : Object
{
  Symbol (void);

  const char *Sident;
  const char *prettyIdent;
  SymbolStorageClass Sclass;
  SymbolFL Sfl;
  SymbolSegment Sseg;
  int Sflags;
  int Salignment;

  dt_t *Sdt;

  // Specific to GNU backend
  tree Stree;
  tree ScontextDecl; // The DECL_CONTEXT to use for child declarations, but see IRState::declContext
  tree SframeField;  // FIELD_DECL in frame struct that this variable is allocated in

  // For FuncDeclarations:
  Thunks thunks;
  OutputStage outputStage;
  FuncFrameInfo *frameInfo;
};

struct Thunk
{
  Thunk (void)
  { offset = 0; symbol = NULL; }

  int offset;
  Symbol *symbol;
};

struct Obj
{
  static void init ();
  static void term ();

  static void startaddress(Symbol *);
  static bool includelib(const char *);
  static bool allowZeroSize();
  static void moduleinfo(Symbol *);
  static void export_symbol (Symbol *, unsigned);
};

extern Obj *objmod;

Symbol *symbol_calloc (const char *string);
tree check_static_sym (Symbol *sym);
void outdata (Symbol *sym);
inline void out_readonly (Symbol *s) { s->Sseg = CDATA; }
void obj_moduleinfo (Symbol *sym);
void obj_tlssections (void);

Symbol *symbol_tree (tree);
Symbol *static_sym (void);

#endif
