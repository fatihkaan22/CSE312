/* SPIM S20 MIPS simulator.
   Execute SPIM syscalls, both in simulator and bare mode.

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


/* Exported functions. */
void SPIM_timerHandler();
int do_syscall ();
void handle_exception ();

enum thread_state { READY=0, RUNNING, BLOCKED, TERMINATED };
static const char* thread_state_str[] = {"READY", "RUNNING", "BLOCKED", "TERMINATED"};

typedef struct thread {
  int thread_id;
  reg_word R[R_LENGTH];
  reg_word HI, LO; // NOTE: do i really need this?
  // NOTE: consider adding FPR, FGR, FWR, CCR, CPR
  mem_addr PC, nPC;
  double *FPR;
  float *FGR;
  int *FWR;
  reg_word CCR[4][32], CPR[4][32];
  // NOTE: consider removing unnecassary stack variables
  mem_word *stack_seg;
  short *stack_seg_h;
  BYTE_TYPE *stack_seg_b;
  mem_addr stack_bot;
  thread_state state;
  thread* join;
}thread;

enum mutex_state { LOCKED, UNLOCKED };

typedef struct mutex {
  reg_word addr;
  mutex_state state;
  int owner_id;
  thread* waiting_threads[128];
}mutex;

typedef struct cond {
  reg_word addr;
  thread* wait_on;
}cond;


void switch_thread(bool);
void init_table();
struct thread* new_thread(reg_word start_routine);
struct thread* get_thread(int thread_id);
void print_thread_table();
int exit_thread();
bool all_terminated();
mutex* get_mutex(reg_word mutex_addr);
void mutex_init(reg_word mutex_addr);
void mutex_lock(reg_word mutex_addr);
void mutex_unlock(reg_word mutex_addr);
void manual_switch_thread();

#define PRINT_INT_SYSCALL	1
#define PRINT_FLOAT_SYSCALL	2
#define PRINT_DOUBLE_SYSCALL	3
#define PRINT_STRING_SYSCALL	4

#define READ_INT_SYSCALL	5
#define READ_FLOAT_SYSCALL	6
#define READ_DOUBLE_SYSCALL	7
#define READ_STRING_SYSCALL	8

#define SBRK_SYSCALL		9

#define EXIT_SYSCALL		10

#define PRINT_CHARACTER_SYSCALL	11
#define READ_CHARACTER_SYSCALL	12

#define OPEN_SYSCALL		13
#define READ_SYSCALL		14
#define WRITE_SYSCALL		15
#define CLOSE_SYSCALL		16

#define EXIT2_SYSCALL		17

#define T_CREATE_SYSCALL 18
#define T_JOIN_SYSCALL 19
#define T_EXIT_SYSCALL 20
#define T_MUTEX_INIT 21
#define T_MUTEX_LOCK_SYSCALL 22
#define T_MUTEX_UNLOCK_SYSCALL 23
