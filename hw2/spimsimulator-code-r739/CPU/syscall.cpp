/* SPIM S20 MIPS simulator.
   Execute SPIM syscalls, both in simulator and bare mode.
   Execute MIPS syscalls in bare mode, when running on MIPS systems.
   Copyright (c) 1990-2010, James R. Larus.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   Neither the name of the James R. Larus nor the names of its contributors may be
   used to endorse or promote products derived from this software without specific
   prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef _WIN32
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef _WIN32
#include <io.h>
#endif

#include "spim.h"
#include "string-stream.h"
#include "inst.h"
#include "reg.h"
#include "mem.h"
#include "sym-tbl.h"
#include "syscall.h"
#include "spim-utils.h"

#include <iostream>
#include <vector>
#include <iomanip>
using namespace std;

static vector<struct thread *> thread_table;
static int next_id = 1;
bool main_thread_initialized = false;
static thread *current_thread = NULL;

static vector<struct mutex> mutex_table;

#ifdef _WIN32
/* Windows has an handler that is invoked when an invalid argument is passed to a system
   call. https://msdn.microsoft.com/en-us/library/a9yf33zb(v=vs.110).aspx

   All good, except that the handler tries to invoke Watson and then kill spim with an exception.

   Override the handler to just report an error.
*/

#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>


void myInvalidParameterHandler(const wchar_t* expression,
   const wchar_t* function, 
   const wchar_t* file, 
   unsigned int line, 
   uintptr_t pReserved)
{
  if (function != NULL)
    {
      run_error ("Bad parameter to system call: %s\n", function);
    }
  else
    {
      run_error ("Bad parameter to system call\n");
    }
}

static _invalid_parameter_handler oldHandler;

void windowsParameterHandlingControl(int flag )
{
  static _invalid_parameter_handler oldHandler;
  static _invalid_parameter_handler newHandler = myInvalidParameterHandler;

  if (flag == 0)
    {
      oldHandler = _set_invalid_parameter_handler(newHandler);
      _CrtSetReportMode(_CRT_ASSERT, 0); // Disable the message box for assertions.
    }
  else
    {
      newHandler = _set_invalid_parameter_handler(oldHandler);
      _CrtSetReportMode(_CRT_ASSERT, 1);  // Enable the message box for assertions.
    }
}
#endif


/*You implement your handler here*/
void 
SPIM_timerHandler() {
  // Implement your handler..
  try {
  switch_thread(false);
  } catch (exception &e) {
    cerr << endl << "Caught: " << e.what() << endl;
  };
}

/* Decides which syscall to execute or simulate.  Returns zero upon
   exit syscall and non-zero to continue execution. */
int
do_syscall ()
{
#ifdef _WIN32
    windowsParameterHandlingControl(0);
#endif

  /* Syscalls for the source-language version of SPIM.  These are easier to
     use than the real syscall and are portable to non-MIPS operating
     systems. */
  switch (R[REG_V0])
    {
    case PRINT_INT_SYSCALL:
      write_output (console_out, "%d", R[REG_A0]);
      break;

    case PRINT_FLOAT_SYSCALL:
      {
	float val = FPR_S (REG_FA0);

	write_output (console_out, "%.8f", val);
	break;
      }

    case PRINT_DOUBLE_SYSCALL:
      write_output (console_out, "%.18g", FPR[REG_FA0 / 2]);
      break;

    case PRINT_STRING_SYSCALL:
      write_output (console_out, "%s", mem_reference (R[REG_A0]));
      break;

    case READ_INT_SYSCALL:
      {
	static char str [256];

	read_input (str, 256);
	R[REG_RES] = atol (str);
	break;
      }

    case READ_FLOAT_SYSCALL:
      {
	static char str [256];

	read_input (str, 256);
	FPR_S (REG_FRES) = (float) atof (str);
	break;
      }

    case READ_DOUBLE_SYSCALL:
      {
	static char str [256];

	read_input (str, 256);
	FPR [REG_FRES] = atof (str);
	break;
      }

    case READ_STRING_SYSCALL:
      {
	read_input ( (char *) mem_reference (R[REG_A0]), R[REG_A1]);
	data_modified = true;
	break;
      }

    case SBRK_SYSCALL:
      {
	mem_addr x = data_top;
	expand_data (R[REG_A0]);
	R[REG_RES] = x;
	data_modified = true;
	break;
      }

    case PRINT_CHARACTER_SYSCALL:
      write_output (console_out, "%c", R[REG_A0]);
      break;

    case READ_CHARACTER_SYSCALL:
      {
	static char str [2];

	read_input (str, 2);
	if (*str == '\0') *str = '\n';      /* makes xspim = spim */
	R[REG_RES] = (long) str[0];
	break;
      }

    case EXIT_SYSCALL:
      spim_return_value = 0;
      return (0);

    case EXIT2_SYSCALL:
      spim_return_value = R[REG_A0];	/* value passed to spim's exit() call */
      return (0);

    case OPEN_SYSCALL:
      {
#ifdef _WIN32
        R[REG_RES] = _open((char*)mem_reference (R[REG_A0]), R[REG_A1], R[REG_A2]);
#else
	R[REG_RES] = open((char*)mem_reference (R[REG_A0]), R[REG_A1], R[REG_A2]);
#endif
	break;
      }

    case READ_SYSCALL:
      {
	/* Test if address is valid */
	(void)mem_reference (R[REG_A1] + R[REG_A2] - 1);
#ifdef _WIN32
	R[REG_RES] = _read(R[REG_A0], mem_reference (R[REG_A1]), R[REG_A2]);
#else
	R[REG_RES] = read(R[REG_A0], mem_reference (R[REG_A1]), R[REG_A2]);
#endif
	data_modified = true;
	break;
      }

    case WRITE_SYSCALL:
      {
	/* Test if address is valid */
	(void)mem_reference (R[REG_A1] + R[REG_A2] - 1);
#ifdef _WIN32
	R[REG_RES] = _write(R[REG_A0], mem_reference (R[REG_A1]), R[REG_A2]);
#else
	R[REG_RES] = write(R[REG_A0], mem_reference (R[REG_A1]), R[REG_A2]);
#endif
	break;
      }

    case CLOSE_SYSCALL:
      {
#ifdef _WIN32
	R[REG_RES] = _close(R[REG_A0]);
#else
	R[REG_RES] = close(R[REG_A0]);
#endif
	break;
      }
		
    case T_CREATE_SYSCALL: 
      {
#ifdef DEBUG
        cout << "T_CREATE_SYSCALL\n"; 
#endif
      reg_word start_routine = R[16]; // s0 register
      thread *t = new_thread(start_routine);
      R[REG_V0] = t->thread_id;
	    break;
      }

    case T_JOIN_SYSCALL: 
      {
#ifdef DEBUG
        cout << "T_JOIN_SYSCALL\n";
#endif
        reg_word wait_for = R[REG_A0];
#ifdef DEBUG
        cout << "wait_for: " << wait_for << endl;
#endif
        thread* wait = get_thread(wait_for);
        if (wait->state != TERMINATED) {
          wait->join = current_thread;
          current_thread->state = BLOCKED;
          manual_switch_thread();
	      }
        break;
      }
    case T_EXIT_SYSCALL: 
      {
        if (exit_thread() == 0) {
          for (size_t i = 0; i < thread_table.size(); ++i) {
            free(thread_table[i]);
          }
          spim_return_value = 0;
          return 0;
        }
	    break;
      }
    case T_MUTEX_INIT : 
      {
        mutex_init(R[REG_A0]);
	    break;
      }
    case T_MUTEX_LOCK_SYSCALL : 
      {
        reg_word mutex_addr = R[REG_A0];
        /* cout << "MUTEX_ADDR: " << mutex_addr << endl; */
        mutex_lock(mutex_addr);
	    break;
      }
    case T_MUTEX_UNLOCK_SYSCALL : 
      {
#ifdef DEBUG
      cout << "MUTEX UNLOCK\n";
#endif
      reg_word mutex_addr = R[REG_A0];
      /* cout << "MUTEX_UNLOCK: " << mutex_addr << endl; */
      mutex_unlock(mutex_addr);
      break;
    }
    default:
      run_error ("Unknown system call: %d\n", R[REG_V0]);
      break;
    }

#ifdef _WIN32
    windowsParameterHandlingControl(1);
#endif
  return (1);
}


void
handle_exception ()
{
  if (!quiet && CP0_ExCode != ExcCode_Int)
    error ("Exception occurred at PC=0x%08x\n", CP0_EPC);

  exception_occurred = 0;
  PC = EXCEPTION_ADDR;

  switch (CP0_ExCode)
    {
    case ExcCode_Int:
      break;

    case ExcCode_AdEL:
      if (!quiet)
	error ("  Unaligned address in inst/data fetch: 0x%08x\n", CP0_BadVAddr);
      break;

    case ExcCode_AdES:
      if (!quiet)
	error ("  Unaligned address in store: 0x%08x\n", CP0_BadVAddr);
      break;

    case ExcCode_IBE:
      if (!quiet)
	error ("  Bad address in text read: 0x%08x\n", CP0_BadVAddr);
      break;

    case ExcCode_DBE:
      if (!quiet)
	error ("  Bad address in data/stack read: 0x%08x\n", CP0_BadVAddr);
      break;

    case ExcCode_Sys:
      if (!quiet)
	error ("  Error in syscall\n");
      break;

    case ExcCode_Bp:
      exception_occurred = 0;
      return;

    case ExcCode_RI:
      if (!quiet)
	error ("  Reserved instruction execution\n");
      break;

    case ExcCode_CpU:
      if (!quiet)
	error ("  Coprocessor unuable\n");
      break;

    case ExcCode_Ov:
      if (!quiet)
	error ("  Arithmetic overflow\n");
      break;

    case ExcCode_Tr:
      if (!quiet)
	error ("  Trap\n");
      break;

    case ExcCode_FPE:
      if (!quiet)
	error ("  Floating point\n");
      break;

    default:
      if (!quiet)
	error ("Unknown exception: %d\n", CP0_ExCode);
      break;
    }
}

thread* get_next_thread() {
  int curr_idx = -1;
  for (size_t i = 0; i < thread_table.size(); ++i) 
    if (current_thread == thread_table[i]) {
      curr_idx = i;
      break;
    }
  if (curr_idx == -1) {
    cerr << "ERROR: thread not found\n";
    return NULL;
  }
  int next_idx = curr_idx;
  do {
    next_idx = (next_idx + 1) % thread_table.size();
  } while(thread_table[next_idx]->state == BLOCKED || 
          thread_table[next_idx]->state == TERMINATED);
  return thread_table[next_idx];
}

void switch_thread(bool manual) {
  if (!main_thread_initialized)
    init_table();

  thread *next_thread = get_next_thread();

#ifdef DEBUG
  if (next_thread == current_thread) {
    cout << "NEXT == current_thread\n";
    return;
  }
#endif

  // switch thread: from -> to
  cout << endl
       << "switching: " << current_thread->thread_id << "->"
       << next_thread->thread_id << endl;

  cout << endl;
  // backup necessary info
  memcpy(current_thread->R, R, sizeof(R)); // registers
  current_thread->HI = HI;
  current_thread->LO = LO;
  current_thread->PC = PC;
  current_thread->nPC = nPC;
  current_thread->FPR = FPR;
  current_thread->FGR = FGR;
  current_thread->FWR = FWR;
  memcpy(current_thread->CCR, CCR, sizeof(CCR));
  memcpy(current_thread->CPR, CPR, sizeof(CPR));
  current_thread->stack_seg = stack_seg;
  current_thread->stack_seg_h = stack_seg_h;
  current_thread->stack_seg_b = stack_seg_b;
  current_thread->stack_bot = stack_bot;

  // change state
  if (current_thread->state == RUNNING)
    current_thread->state = READY;

  current_thread = next_thread;
  current_thread->state = RUNNING;
  // restore
  memcpy(R, current_thread->R, sizeof(R)); // registers
  PC = current_thread->PC;
  HI = current_thread->HI;
  LO = current_thread->LO;
  PC = current_thread->PC;
  nPC =current_thread->nPC;
  FPR =current_thread->FPR;
  FGR =current_thread->FGR;
  FWR =current_thread->FWR;
  memcpy(CCR, current_thread->CCR, sizeof(CCR));
  memcpy(CPR, current_thread->CPR, sizeof(CPR));
  stack_seg = current_thread->stack_seg;
  stack_seg_h = current_thread->stack_seg_h;
  stack_seg_b = current_thread->stack_seg_b;
  stack_bot = current_thread->stack_bot;
  
  if (manual) {
    PC -= BYTES_PER_WORD;
  }

  print_thread_table();

  return;
}

struct thread* get_thread(int thread_id) {
  for (size_t i = 0; i < thread_table.size(); ++i)
    if (thread_table[i]->thread_id == thread_id)
      return thread_table[i];
  return NULL;
}

void init_table() {
  thread *main_thread =  (thread *)malloc(sizeof(thread));
  main_thread->thread_id = 0;
  main_thread->state = RUNNING;
  thread_table.push_back(main_thread);
  current_thread = main_thread;
  main_thread_initialized = true;
}

// start_routine: address of the rotuine to run
thread* new_thread(reg_word start_routine) {
  // init main thread table the first time
  if (!main_thread_initialized)
    init_table();
  thread *t = (thread *)malloc(sizeof(thread));

  t->thread_id = next_id++;
  t->PC = start_routine;
  t->FPR = (double *)malloc(FPR_LENGTH * sizeof(double));
  t->stack_seg = (mem_word *)malloc(STACK_SIZE);
  t->stack_seg_h = (short *) t->stack_seg;
  t->stack_seg_b = (BYTE_TYPE *) t->stack_seg;
	t->stack_bot = stack_bot;

  t->join = NULL;
  t->HI = 0;
  t->LO = 0;

  /* t->R[REG_SP] = STACK_TOP - 1; /1* Initialize $sp *1/ */
  t->R[REG_SP] = STACK_TOP - BYTES_PER_WORD - 4096; /* Initialize $sp */

  // get a0, a1, a2 (to pass args to thread)
  t->R[REG_A0] = R[REG_A0];
  t->R[REG_A1] = R[REG_A1];
  t->R[REG_A2] = R[REG_A2];

  // put new thread to table
  thread_table.push_back(t);
  return t;
}

void print_thread_table() {
  cout << endl;
  cout << "| thread id |      PC | stack pointer | thread state |\n";
  cout << "|-----------|---------|---------------|--------------|\n";
  for (size_t i = 0; i < thread_table.size(); ++i) {
    cout << "|" << setw(11) << thread_table[i]->thread_id
         << "|" << setw(9)  << thread_table[i]->PC
         << "|" << setw(15) << hex<< thread_table[i]->R[REG_SP]
         << "|" << setw(14) << thread_state_str[thread_table[i]->state] << "|\n";
  }
}

bool all_terminated() {
  for (size_t i = 0; i < thread_table.size(); ++i)
    if (thread_table[i]->state != TERMINATED)
      return false;
  return true;
}

// return 0 if no threads left, 1 otherwise
int exit_thread() {
#ifdef DEBUG
  cout << "\nT_EXIT_SYSCALL" << endl;
#endif
  current_thread->state = TERMINATED;

  for (size_t i = 0; i < mutex_table.size(); ++i) {
    if (mutex_table[i].owner_id == current_thread->thread_id) {
      mutex_unlock(mutex_table[i].addr);
    }
  }

  // free thread
  free(current_thread->FPR);
  free(current_thread->stack_seg);

  if (current_thread->join != NULL) {
    current_thread->join->state = READY;
  }

  if (all_terminated()) {
#ifdef DEBUG
    cout << "\nALL_TERMINATED\n";
#endif
    return 0;
  } else {
    manual_switch_thread();
  }
  return 1;
}

mutex* get_mutex(reg_word mutex_addr) {
  for (size_t i = 0; i < mutex_table.size(); ++i)
    if (mutex_table[i].addr == mutex_addr)
      return &mutex_table[i];
  return NULL;
}

void mutex_init(reg_word mutex_addr) {
  mutex m;
  m.addr = mutex_addr;
  m.state = UNLOCKED;
  m.owner_id = -1; // only can be owned if mutex is locked
  m.waiting_threads[0] = NULL;
  mutex_table.push_back(m);
}

void mutex_lock(reg_word mutex_addr) {
  /* cout <<"mutex_addr: " << mutex_addr << endl; */
  mutex* m = get_mutex(mutex_addr);

  if (m->owner_id == current_thread->thread_id)
    return;

  if (m->state == UNLOCKED) {
    m->owner_id = current_thread->thread_id;
    m->state = LOCKED;
  } else {
    /* cout << "WAIT ON MUTEX" << endl; */
    current_thread->state = BLOCKED;
    int i = 0;
    while(m->waiting_threads[i]) ++i; // iterate first free position
    m->waiting_threads[i] = current_thread;
    m->waiting_threads[i+1] = NULL;
    PC -= 4; // to retry to take the mutex
    manual_switch_thread();
  }
}

void mutex_unlock(reg_word mutex_addr) {
  /* cout << "MUTEX_UNLOCK" << endl; */
  /* cout <<"mutex_addr: " << mutex_addr << endl; */
  mutex* m = get_mutex(mutex_addr);

  if (m->state == UNLOCKED) {
#ifdef DEBUG
    cout << "MUTEX ALREADY UNLOCKED" << endl;
#endif
    return;
  }

  if (m->waiting_threads[0] == NULL) { // no thread waiting on
    m->state = UNLOCKED;
    m->owner_id = -1;
    return;
  }
  // iterate last thread
  int i = 0;
  m->state = UNLOCKED;
  while(m->waiting_threads[i]) {
    m->waiting_threads[i]->state = READY;
    ++i;
  }
  m->owner_id = -1;
  m->waiting_threads[0] = NULL; // empty queue
}

void manual_switch_thread() {
#ifdef DEBUG
  cout << "MANUAL_SWITCH_THREAD\n";
#endif
  switch_thread(true);
}
