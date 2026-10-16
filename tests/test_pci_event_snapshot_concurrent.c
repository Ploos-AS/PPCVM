#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "ppcvm/pci_event_device.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>
#endif
#ifdef _WIN32
typedef struct {
  ppcvm_pci_event_device *device;
  const char *path;
} writer_args;
static DWORD WINAPI writer(LPVOID opaque) {
  writer_args *args=(writer_args *)opaque;
  unsigned i;
  for(i=0;i<16u;i++) {
    if(ppcvm_pci_event_device_save_file(args->device,args->path)!=0) {
      fprintf(stderr,"Windows thread writer failed at save %u (winerr=%lu)\\n",
          i+1u,(unsigned long)GetLastError());
      fflush(stderr);
      return 1u;
    }
    if((i+1u)%4u==0u) {
      fprintf(stderr,"Windows thread writer %p completed %u saves\\n",
          (void *)args->device,i+1u);
      fflush(stderr);
    }
  }
  return 0u;
}
#endif
int main(int argc,char **argv) {
#ifdef _WIN32
  const char *path="pci-event-concurrent.pevt";
  ppcvm_pci_bus pci;
  ppcvm_pci_device config;
  ppcvm_discovery_ii controller;
  ppcvm_pci_irq_matrix matrix;
  ppcvm_pci_event_device first,second;
  writer_args args[2];
  HANDLE threads[2];
  DWORD result;
  uint8_t frame[PPCVM_PCI_EVENT_WIRE_SIZE];
  ppcvm_pci_event_snapshot snapshot;
  FILE *f;
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&config,0x1234,0x5681,2,0,1);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&config)==0);
  ppcvm_discovery_ii_init(&controller);
  assert(ppcvm_pci_irq_matrix_init(&matrix,&pci,&controller)==0);
  assert(ppcvm_pci_irq_matrix_add(&matrix,0,2,0,1,0)==0);
  assert(ppcvm_pci_event_device_init(&first,&matrix,0,2,0,1)==0);
  assert(ppcvm_pci_event_device_init(&second,&matrix,0,2,0,1)==0);
  assert(ppcvm_pci_event_device_write32(&first,0,12,1u)==0);
  assert(ppcvm_pci_event_device_write32(&second,0,12,2u)==0);
  assert(ppcvm_pci_event_device_write32(&first,0,4,1u)==0);
  assert(ppcvm_pci_event_device_write32(&second,0,4,2u)==0);
  /* Child mode permits a separate-process Windows writer test. */
  if(argc==2 && (strcmp(argv[1],"--child-a")==0 ||
      strcmp(argv[1],"--child-b")==0)) {
    ppcvm_pci_event_device *source=strcmp(argv[1],"--child-b")==0?
        &second:&first;
    unsigned i;
    for(i=0;i<16u;i++)
      if(ppcvm_pci_event_device_save_file(source,path)!=0) return 1;
    return 0;
  }
  args[0].device=&first;args[0].path=path;
  args[1].device=&second;args[1].path=path;
  threads[0]=CreateThread(NULL,0,writer,&args[0],0,NULL);
  threads[1]=CreateThread(NULL,0,writer,&args[1],0,NULL);
  assert(threads[0]!=NULL && threads[1]!=NULL);
  {
    DWORD wait_result=WaitForMultipleObjects(2,threads,TRUE,60000u);
    if(wait_result!=WAIT_OBJECT_0) {
      fprintf(stderr,"thread writer wait failed: result=%lu, winerr=%lu\\n",
          (unsigned long)wait_result,(unsigned long)GetLastError());
      fflush(stderr);
      return 2;
    }
  }
  assert(GetExitCodeThread(threads[0],&result) && result==0u);
  assert(GetExitCodeThread(threads[1],&result) && result==0u);
  assert(CloseHandle(threads[0]) && CloseHandle(threads[1]));
  /* Verify the thread race before starting the separate-process race. */
  {
    FILE *thread_file=fopen(path,"rb");
    assert(thread_file!=NULL);
    assert(fread(frame,1,sizeof(frame),thread_file)==sizeof(frame));
    assert(fgetc(thread_file)==EOF);
    assert(fclose(thread_file)==0);
    assert(ppcvm_pci_event_snapshot_decode(&snapshot,frame,sizeof(frame))==0);
    assert((snapshot.pending==1u && snapshot.enable==1u) ||
        (snapshot.pending==2u && snapshot.enable==2u));
  }
  /* Exercise two independent Windows processes with distinct PIDs. */
  {
    char exe[MAX_PATH], command[MAX_PATH+32];
    STARTUPINFOA startup={0};
    PROCESS_INFORMATION children[2]={{0},{0}};
    HANDLE waits[2];
    unsigned i;
    {
      DWORD length=GetModuleFileNameA(NULL,exe,MAX_PATH);
      assert(length>0 && length<MAX_PATH);
    }
    startup.cb=sizeof(startup);
    for(i=0;i<2u;i++) {
      int length=snprintf(command,sizeof(command),"\"%s\" --child-%c",
          exe,i==0u?'a':'b');
      assert(length>0 && (size_t)length<sizeof(command));
      assert(CreateProcessA(NULL,command,NULL,NULL,FALSE,0,NULL,NULL,
          &startup,&children[i])!=0);
      waits[i]=children[i].hProcess;
    }
    assert(WaitForMultipleObjects(2,waits,TRUE,60000u)==WAIT_OBJECT_0);
    for(i=0;i<2u;i++) {
      assert(GetExitCodeProcess(children[i].hProcess,&result) && result==0u);
      assert(CloseHandle(children[i].hThread));
      assert(CloseHandle(children[i].hProcess));
    }
  }
  f=fopen(path,"rb");assert(f!=NULL);
  assert(fread(frame,1,sizeof(frame),f)==sizeof(frame));
  assert(fgetc(f)==EOF);assert(fclose(f)==0);
  assert(ppcvm_pci_event_snapshot_decode(&snapshot,frame,sizeof(frame))==0);
  assert((snapshot.pending==1u && snapshot.enable==1u) ||
      (snapshot.pending==2u && snapshot.enable==2u));
  assert(remove(path)==0);
  /* Verify no unique Windows staging file survived either writer. */
  {
    WIN32_FIND_DATAA entry;
    HANDLE search=FindFirstFileA("pci-event-concurrent.pevt.tmp.*",&entry);
    if(search!=INVALID_HANDLE_VALUE) {
      FindClose(search);
      assert(!"Windows snapshot staging file left behind");
    } else {
      DWORD error=GetLastError();
      assert(error==ERROR_FILE_NOT_FOUND);
    }
  }
#else
  const char *path="pci-event-concurrent.pevt";
  ppcvm_pci_bus pci;
  ppcvm_pci_device config;
  ppcvm_discovery_ii controller;
  ppcvm_pci_irq_matrix matrix;
  ppcvm_pci_event_device event;
  uint8_t frame[PPCVM_PCI_EVENT_WIRE_SIZE];
  FILE *f;
  pid_t child;
  ppcvm_pci_bus_init(&pci);
  ppcvm_pci_device_init(&config,0x1234,0x5681,2,0,1);
  assert(ppcvm_pci_bus_add(&pci,0,2,0,&config)==0);
  ppcvm_discovery_ii_init(&controller);
  assert(ppcvm_pci_irq_matrix_init(&matrix,&pci,&controller)==0);
  assert(ppcvm_pci_irq_matrix_add(&matrix,0,2,0,1,0)==0);
  assert(ppcvm_pci_event_device_init(&event,&matrix,0,2,0,1)==0);
  assert(ppcvm_pci_event_device_write32(&event,0,12,1u)==0);
  child=fork();
  assert(child>=0);
  if(child==0) {
    for(unsigned i=0;i<32;i++)
      if(ppcvm_pci_event_device_save_file(&event,path)!=0) _exit(1);
    _exit(0);
  }
  assert(ppcvm_pci_event_device_write32(&event,0,12,2u)==0);
  for(unsigned i=0;i<32;i++)
    assert(ppcvm_pci_event_device_save_file(&event,path)==0);
  {
    int status=0;
    assert(waitpid(child,&status,0)==child);
    assert(WIFEXITED(status) && WEXITSTATUS(status)==0);
  }
  f=fopen(path,"rb");assert(f!=NULL);
  assert(fread(frame,1,sizeof(frame),f)==sizeof(frame));
  assert(fgetc(f)==EOF);assert(fclose(f)==0);
  {
    ppcvm_pci_event_snapshot snapshot;
    assert(ppcvm_pci_event_snapshot_decode(&snapshot,frame,sizeof(frame))==0);
    assert(snapshot.pending==1u || snapshot.pending==3u);
  }
  assert(remove(path)==0);
  /* Unique staging files must be removed after both writers finish. */
  {
    DIR *directory=opendir(".");
    struct dirent *entry;
    assert(directory!=NULL);
    while((entry=readdir(directory))!=NULL)
      assert(strncmp(entry->d_name,
          "pci-event-concurrent.pevt.tmp.",30u)!=0);
    assert(closedir(directory)==0);
  }
#endif
  return 0;
}
