// -*-Mode: C++;-*- // technically C99
// $Id$

// * BeginRiceCopyright *****************************************************
// 
// Copyright ((c)) 2002-2007, Rice University 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// 
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// 
// * Neither the name of Rice University (RICE) nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
// 
// This software is provided by RICE and contributors "as is" and any
// express or implied warranties, including, but not limited to, the
// implied warranties of merchantability and fitness for a particular
// purpose are disclaimed. In no event shall RICE or contributors be
// liable for any direct, indirect, incidental, special, exemplary, or
// consequential damages (including, but not limited to, procurement of
// substitute goods or services; loss of use, data, or profits; or
// business interruption) however caused and on any theory of liability,
// whether in contract, strict liability, or tort (including negligence
// or otherwise) arising in any way out of the use of this software, even
// if advised of the possibility of such damage. 
// 
// ******************************************************* EndRiceCopyright *

//***************************************************************************
//
// File:
//   $Source$
//
// Purpose:
//   Low-level types and functions for reading/writing a call path
//   profile as formatted data.
//
//   These routines *must not* allocate dynamic memory; if such memory
//   is needed, callbacks to the user's allocator should be used.
//
// Description:
//   [The set of functions, macros, etc. defined in the file]
//
//***************************************************************************

#ifndef prof_lean_hpcrun_fmt_h
#define prof_lean_hpcrun_fmt_h

//************************* System Include Files ****************************

#include <stdbool.h>

//*************************** User Include Files ****************************

#include "hpcfmt.h"

#include "lush/lush-support.h"

//*************************** Forward Declarations **************************

#if defined(__cplusplus)
extern "C" {
#endif

//***************************************************************************
//
// Types and functions for reading/writing a call stack tree from/to a
// binary file.  
//
// Basic format of HPC_CSPROF: see implementation file for more details
//   HPC_CSPROF header
//   List of data chunks describing profile metrics, etc.
//   HPC_CSTREE data
//
// Data in the file is stored in little-endian format, the ordering of
// IA-64 instructions and the default mode of an IA-64 processor.
//
//***************************************************************************

#define HPCFILE_CSPROF_MAGIC_STR     "HPC_CSPROF"
#define HPCFILE_CSPROF_MAGIC_STR_LEN 10 /* exclude '\0' */

#define HPCFILE_CSPROF_VERSION     "01.01"
#define HPCFILE_CSPROF_VERSION_LEN 5 /* exclude '\0' */

#define HPCFILE_CSPROF_ENDIAN 'l'

// ---------------------------------------------------------
// hpcfile_csprof_id_t: file identification.
//
// The size is 16 bytes and valid for both big/little endian ordering.
// Although these files will probably not have very long lives on
// disk, hopefully this will not need to be changed.
// ---------------------------------------------------------
typedef struct hpcfile_csprof_id_s {
  
  char magic_str[HPCFILE_CSPROF_MAGIC_STR_LEN]; 
  char version[HPCFILE_CSPROF_VERSION_LEN];
  char endian;  // 'b' or 'l' (currently, redundant info)
  
} hpcfile_csprof_id_t;

int hpcfile_csprof_id__init(hpcfile_csprof_id_t* x);
int hpcfile_csprof_id__fini(hpcfile_csprof_id_t* x);

int hpcfile_csprof_id__fread(hpcfile_csprof_id_t* x, FILE* fs);
int hpcfile_csprof_id__fwrite(hpcfile_csprof_id_t* x, FILE* fs);
int hpcfile_csprof_id__fprint(hpcfile_csprof_id_t* x, FILE* fs);


// ---------------------------------------------------------
// hpcfile_csprof_hdr_t:
// ---------------------------------------------------------
typedef struct hpcfile_csprof_hdr_s {

  hpcfile_csprof_id_t fid;

  // data information
  uint64_t num_data; // number of data chucks following header
  
} hpcfile_csprof_hdr_t;

int hpcfile_csprof_hdr__init(hpcfile_csprof_hdr_t* x);
int hpcfile_csprof_hdr__fini(hpcfile_csprof_hdr_t* x);

int hpcfile_csprof_hdr__fread(hpcfile_csprof_hdr_t* x, FILE* fs);
int hpcfile_csprof_hdr__fwrite(hpcfile_csprof_hdr_t* x, FILE* fs);
int hpcfile_csprof_hdr__fprint(hpcfile_csprof_hdr_t* x, FILE* fs);


/* hpcfile_csprof_metric_flag_t */

typedef uint64_t hpcfile_csprof_metric_flag_t;

#define HPCFILE_METRIC_FLAG_NULL  0x0
#define HPCFILE_METRIC_FLAG_ASYNC (1 << 1)
#define HPCFILE_METRIC_FLAG_REAL  (1 << 2)


static inline bool
hpcfile_csprof_metric_is_flag(hpcfile_csprof_metric_flag_t flagbits, 
			      hpcfile_csprof_metric_flag_t f)
{ 
  return (flagbits & f); 
}


static inline void 
hpcfile_csprof_metric_set_flag(hpcfile_csprof_metric_flag_t* flagbits, 
			       hpcfile_csprof_metric_flag_t f)
{
  *flagbits = (*flagbits | f);
}


static inline void 
hpcfile_csprof_metric_unset_flag(hpcfile_csprof_metric_flag_t* flagbits, 
				 hpcfile_csprof_metric_flag_t f)
{
  *flagbits = (*flagbits & ~f);
}


/* hpcfile_csprof_metric_t */

typedef struct hpcfile_csprof_metric_s {
    char *metric_name;          /* name of the metric */
    hpcfile_csprof_metric_flag_t flags;  /* metric flags (async, etc.) */
    uint64_t sample_period;     /* sample period of the metric */
} hpcfile_csprof_metric_t;



int hpcfile_csprof_metric__init(hpcfile_csprof_metric_t *x);
int hpcfile_csprof_metric__fini(hpcfile_csprof_metric_t *x);

int hpcfile_csprof_metric__fread(hpcfile_csprof_metric_t *x, FILE *fs);
int hpcfile_csprof_metric__fwrite(hpcfile_csprof_metric_t *x, FILE *fs);
int hpcfile_csprof_metric__fprint(hpcfile_csprof_metric_t *x, FILE *fs);



// ---------------------------------------------------------
// FIXME: 
// ---------------------------------------------------------

typedef struct ldmodule_s {
  char *name;
  uint64_t vaddr;
  uint64_t  mapaddr;
} ldmodule_t; 

typedef struct epoch_entry_s { 
  uint32_t num_loadmodule;
  ldmodule_t *loadmodule;
} epoch_entry_t;
  
typedef struct epoch_table_s {
  uint32_t num_epoch;
  epoch_entry_t *epoch_modlist;
} epoch_table_t; 


// frees the data of 'x' but not x itself
void epoch_table__free_data(epoch_table_t* x, hpcfile_cb__free_fn_t free_fn);

// ---------------------------------------------------------
// hpcfile_csprof_data_t: used only for passing data; not actually
// part of the file format
// ---------------------------------------------------------

typedef struct hpcfile_csprof_data_s {
  char* target;               // name of profiling target
  uint32_t num_metrics;       // number of metrics recorded
  hpcfile_csprof_metric_t *metrics;
  uint32_t num_ccts;          // number of CCTs
  
} hpcfile_csprof_data_t;

int hpcfile_csprof_data__init(hpcfile_csprof_data_t* x);
int hpcfile_csprof_data__fini(hpcfile_csprof_data_t* x);

int hpcfile_csprof_data__fprint(hpcfile_csprof_data_t* x, FILE* fs);



//***************************************************************************
//
// High-level types and functions for reading/writing a call stack profile
// from/to a binary file.
//
// Basic format of HPC_CSPROF: see implementation file for more details
//   HPC_CSPROF header
//   List of data chunks describing profile metrics, etc.
//   HPC_CSTREE data
//
// Users will most likely want to use the hpcfile_open_XXX() and
// hpcfile_close() functions to open and close the file streams that
// these functions use.
//
//***************************************************************************

//***************************************************************************
// hpcfile_csprof_write()
//***************************************************************************

// hpcfile_csprof_write: Writes the call stack profile data in 'data'
// to file stream 'fs'.  The user should supply and manage all data
// contained in 'data'.  Returns HPCFILE_OK upon success; HPCFILE_ERR
// on error.
//
// Note: Any corresponding call stack tree is *not* written by this
// function.  Users must also call hpcfile_cstree_write().
int
hpcfile_csprof_write(FILE* fs, hpcfile_csprof_data_t* data);

//***************************************************************************
// hpcfile_csprof_read()
//***************************************************************************

// hpcfile_csprof_read: Reads call stack profile data from the file
// stream 'fs' into 'data'.  Uses callback functions to manage any
// memory allocation.  Note that the *user* is responsible for freeing
// any memory allocated for pointers in 'data' (characer strings,
// etc.).  Returns HPCFILE_OK upon success; HPCFILE_ERR on error.
//
// Note: Any corresponding call stack tree is *not* read by this
// function.  Users must also call hpcfile_cstree_read().
int
hpcfile_csprof_read(FILE* fs, 
		    hpcfile_csprof_data_t* data, epoch_table_t* epochtbl,
		    hpcfile_cb__alloc_fn_t alloc_fn,
		    hpcfile_cb__free_fn_t free_fn);

//***************************************************************************
// hpcfile_csprof_fprint()
//***************************************************************************

// hpcfile_csprof_fprint: Given an output file stream 'outfs',
// reads profile data from the input file stream 'infs' and writes it to
// 'outfs' as text for human inspection.  This text output is not
// designed for parsing and any formatting is subject to change.
// Returns HPCFILE_OK upon success; HPCFILE_ERR on error.
//
// Note: Any corresponding call stack tree is *not* converted by this
// function.  Users must also call hpcfile_cstree_fprint().
int
hpcfile_csprof_fprint(FILE* infs, FILE* outfs, hpcfile_csprof_data_t* data);


//***************************************************************************
//
//***************************************************************************

// tallent: OBSOLETE

#define HPCFILE_CSTREE_MAGIC_STR     "HPC_CSTREE"
#define HPCFILE_CSTREE_MAGIC_STR_LEN 10 /* exclude '\0' */

#define HPCFILE_CSTREE_VERSION     "01.00"
#define HPCFILE_CSTREE_VERSION_LEN 5 /* exclude '\0' */

#define HPCFILE_CSTREE_ENDIAN 'l' /* 'l' for little, 'b' for big */

// ---------------------------------------------------------
// hpcfile_cstree_id_t: file identification.
//
// The size is 16 bytes and valid for both big/little endian ordering.
// Although these files will probably not have very long lives on disk,
// hopefully this will not need to be changed.
// ---------------------------------------------------------
typedef struct hpcfile_cstree_id_s {
  
  char magic_str[HPCFILE_CSTREE_MAGIC_STR_LEN]; 
  char version[HPCFILE_CSTREE_VERSION_LEN];
  char endian;  // 'b' or 'l' (currently, redundant info)
  
} hpcfile_cstree_id_t;

int hpcfile_cstree_id__init(hpcfile_cstree_id_t* x);
int hpcfile_cstree_id__fini(hpcfile_cstree_id_t* x);

int hpcfile_cstree_id__fread(hpcfile_cstree_id_t* x, FILE* fs);
int hpcfile_cstree_id__fwrite(hpcfile_cstree_id_t* x, FILE* fs);
int hpcfile_cstree_id__fprint(hpcfile_cstree_id_t* x, FILE* fs, 
			      const char* pre);

// ---------------------------------------------------------
// hpcfile_cstree_hdr_t:
// ---------------------------------------------------------
typedef struct hpcfile_cstree_hdr_s {
  hpcfile_cstree_id_t fid;
  
  // data type sizes (currently, redundant info)
  uint32_t vma_sz;    // 8
  uint32_t uint_sz;   // 8
  
  // data information
  uint64_t num_nodes;         /* number of tree nodes */
  uint32_t epoch;             /* epoch index */
} hpcfile_cstree_hdr_t;


int
hpcfile_cstree_read_hdr(FILE* fs, hpcfile_cstree_hdr_t* hdr);

int hpcfile_cstree_hdr__init(hpcfile_cstree_hdr_t* x);
int hpcfile_cstree_hdr__fini(hpcfile_cstree_hdr_t* x);

int hpcfile_cstree_hdr__fread(hpcfile_cstree_hdr_t* x, FILE* fs);
int hpcfile_cstree_hdr__fwrite(hpcfile_cstree_hdr_t* x, FILE* fs);
int hpcfile_cstree_hdr__fprint(hpcfile_cstree_hdr_t* x, FILE* fs);

// ---------------------------------------------------------
// hpcfile_cstree_nodedata_t:
// ---------------------------------------------------------

#define HPCFILE_TAG__CSTREE_NODE 13 /* just because */
#define HPCFILE_TAG__CSTREE_LIP  77 /* feel free to change */


// tallent: was 'size_t'.  If this should change the memcpy in
// hpcfile_cstree_write_node_hlp should be modified.

typedef union hpcfile_metric_data_u {
  uint64_t bits; // for reading/writing

  uint64_t i; // integral data
  double   r; // real
  
} hpcfile_metric_data_t;

extern hpcfile_metric_data_t hpcfile_metric_data_ZERO;
  
static inline bool hpcfile_metric_data_iszero(hpcfile_metric_data_t x) {
  return (x.bits == hpcfile_metric_data_ZERO.bits);
}

typedef struct hpcfile_cstree_nodedata_s {

  lush_assoc_info_t as_info;

  // instruction pointer: more accurately, this is an 'operation
  // pointer'.  The operation in the instruction packet is represented
  // by adding 0, 1, or 2 to the instruction pointer for the first,
  // second and third operation, respectively.
  hpcfile_vma_t ip;

  union {
    hpcfile_uint_t id;  // canonical lip id
    lush_lip_t*    ptr; // pointer
  } lip; 

  // 'sp': the stack pointer of this node
  // tallent: Why is this needed?
  hpcfile_uint_t sp;

  uint32_t cpid;

  hpcfile_uint_t num_metrics;
  hpcfile_metric_data_t* metrics;

} hpcfile_cstree_nodedata_t;

int hpcfile_cstree_nodedata__init(hpcfile_cstree_nodedata_t* x);
int hpcfile_cstree_nodedata__fini(hpcfile_cstree_nodedata_t* x);

int hpcfile_cstree_nodedata__fread(hpcfile_cstree_nodedata_t* x, FILE* fs);
int hpcfile_cstree_nodedata__fwrite(hpcfile_cstree_nodedata_t* x, FILE* fs);
int hpcfile_cstree_nodedata__fprint(hpcfile_cstree_nodedata_t* x, FILE* fs,
				    const char* pre);

// ---------------------------------------------------------
// 
// ---------------------------------------------------------


int hpcfile_cstree_as_info__fread(lush_assoc_info_t* x, FILE* fs);
int hpcfile_cstree_as_info__fwrite(lush_assoc_info_t* x, FILE* fs);


int hpcfile_cstree_lip__fread(lush_lip_t* x, FILE* fs);
int hpcfile_cstree_lip__fwrite(lush_lip_t* x, FILE* fs);
int hpcfile_cstree_lip__fprint(lush_lip_t* x, hpcfile_uint_t id, 
			       FILE* fs, const char* pre);

// ---------------------------------------------------------
// hpcfile_cstree_node_t: The root node -- the node without a parent -- is
// indicated by identical values for 'id' and 'id_parent'
// ---------------------------------------------------------
typedef struct hpcfile_cstree_node_s {

  hpcfile_cstree_nodedata_t data;

  hpcfile_uint_t id;        // persistent id of self
  hpcfile_uint_t id_parent; // persistent id of parent

} hpcfile_cstree_node_t;

int hpcfile_cstree_node__init(hpcfile_cstree_node_t* x);
int hpcfile_cstree_node__fini(hpcfile_cstree_node_t* x);

int hpcfile_cstree_node__fread(hpcfile_cstree_node_t* x, FILE* fs);
int hpcfile_cstree_node__fwrite(hpcfile_cstree_node_t* x, FILE* fs);
int hpcfile_cstree_node__fprint(hpcfile_cstree_node_t* x, FILE* f, 
				const char* pres);


//***************************************************************************
// 
//***************************************************************************

#define HPCFILE_CSTREE_NODE_ID_NULL 0

#define HPCFILE_CSTREE_ID_ROOT 1

// hpcfile_cstree_fprint: Given an input file stream 'infs',
// reads tree data from the infs and writes it to 'outfs' as text for
// human inspection.  This text output is not designed for parsing and
// any formatting is subject to change.  Returns HPCFILE_OK upon
// success; HPCFILE_ERR on error.
int
hpcfile_cstree_fprint(FILE* infs, int num_metrics, FILE* outfs);


//***************************************************************************

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif /* prof_lean_hpcrun_fmt_h */

