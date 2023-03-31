#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

extern int total_toggles;
extern int syscalls_number[28];
extern int tempss[28];
int*
sys_print_count(void)
{
  for(int j = 0; j < 28; j++){
    int i = tempss[j]-1;
    if(syscalls_number[i]==0 || tempss[j]==22 || tempss[j]==23){
      continue;
    }
    if (i==0){
      cprintf("sys_fork: %d\n", syscalls_number[i]);
    }else if (i==1){
      cprintf("sys_exit: %d\n", syscalls_number[i]);
    }else if (i==2){
      cprintf("sys_wait: %d\n", syscalls_number[i]);
    }else if (i==3){
      cprintf("sys_pipe: %d\n", syscalls_number[i]);
    }else if (i==4){
      cprintf("sys_read: %d\n", syscalls_number[i]);
    }else if (i==5){
      cprintf("sys_kill: %d\n", syscalls_number[i]);
    }else if (i==6){
      cprintf("sys_exec: %d\n", syscalls_number[i]);
    }else if (i==7){
      cprintf("sys_fstat: %d\n", syscalls_number[i]);
    }else if (i==8){
      cprintf("sys_chdir: %d\n", syscalls_number[i]);
    }else if (i==9){
      cprintf("sys_dup: %d\n", syscalls_number[i]);
    }else if (i==10){
      cprintf("sys_getpid: %d\n", syscalls_number[i]);
    }else if (i==11){
      cprintf("sys_sbrk: %d\n", syscalls_number[i]);
    }else if (i==12){
      cprintf("sys_sleep: %d\n", syscalls_number[i]);
    }else if (i==13){
      cprintf("sys_uptime: %d\n", syscalls_number[i]);
    }else if (i==14){
      cprintf("sys_open: %d\n", syscalls_number[i]);
    }else if (i==15){
      cprintf("sys_write: %d\n", syscalls_number[i]);
    }else if (i==16){
      cprintf("sys_mknod: %d\n", syscalls_number[i]);
    }else if (i==17){
      cprintf("sys_unlink: %d\n", syscalls_number[i]);
    }else if (i==18){
      cprintf("sys_link: %d\n", syscalls_number[i]);
    }else if (i==19){
      cprintf("sys_mkdir: %d\n", syscalls_number[i]);
    }else if (i==20){
      cprintf("sys_close: %d\n", syscalls_number[i]);
    }else if (i==21){
      cprintf("sys_toggle: %d\n", syscalls_number[i]);
    }else if (i==22){
      cprintf("sys_print_count: %d\n", syscalls_number[i]);
    }else if(i==23){
      cprintf("sys_add: %d\n", syscalls_number[i]);
    }else if(i==24){
      cprintf("sys_ps: %d\n", syscalls_number[i]);
    }else if(i==25){
      cprintf("sys_send: %d\n", syscalls_number[i]);
    }else if(i==26){
      cprintf("sys_recv: %d\n", syscalls_number[i]);
    }else if(i==27){
      cprintf("sys_send_multi: %d\n", syscalls_number[i]);
    }

  }
    return 0;
}

int sys_toggle(void){
  if(total_toggles%2==0){
    for(int i = 0; i < 27; i++){
        syscalls_number[i] = 0;
      }
  }
  total_toggles += 1 ;
  return 0;
}

int sys_add(void){
  int x,y;
  argint(0,&x);
  argint(1,&y);
  return x+y;
}

int sys_ps(void){
  // print a list of all current running processes with their pid and name
  print_processes();
  return 0;
}

int sys_send(void){
    int sender_pid ; int rec_pid ; char*msg;
    // argint(0,&sender_pid);
    // argint(1,&rec_pid);
    // argstr(2,&msg);
    if(argint(0,&sender_pid) < 0 || argint(1,&rec_pid) < 0 || argstr(2,&msg) < 0 )
      return -1;
    return send_unicast(sender_pid,rec_pid,msg);

}

int sys_recv(void){
  char*msg;
  if(argstr(0,&msg) < 0)
    return -1;
  return receive(msg);
  return 0;
}


int sys_send_multi(void){
  // implement multicast model of communication
  int sender_pid; int* rec_pids_st ; char *msg;
  argint(0,&sender_pid);
  argptr(1,(char**) &rec_pids_st, 64);
  argstr(2,&msg);
  return send_multicast(sender_pid,rec_pids_st,8,msg);
  return 0;
}











// Scheduling related system calls

int sys_exec_time(void){
  int pid;
  argint(0,&pid);
  int exectime;
  argint(1,&exectime);
  return set_exec_time(pid,exectime);
  
} 


int sys_deadline(void){
  int pid;
  argint(0,&pid);
  int deadline;
  argint(1,&deadline);
  return set_deadline(pid,deadline);
}


int sys_rate(void){
  int pid;
  argint(0,&pid);
  int rate;
  argint(1,&rate);
  return set_rate(pid,rate);
}

int sys_sched_policy(void){
  int pid;
  argint(0,&pid);
  int policy;
  argint(1,&policy);
  return set_policy(pid, policy);
}


int sys_getpinfo(void){
  return get_info();
}