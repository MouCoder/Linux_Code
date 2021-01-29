#include"proc.h"

void proc()
{
  char arr[MAX_SIZE];
  char tag[4]={'|','/','-','\\'};
  //初始化为字符串结束标志符'\0'
  memset(arr,'\0',sizeof(arr));
  //打印进度条
  int i = 0;
  printf("[%-100s][%d%%][%c]\r",arr,0,'|');
  fflush(stdout);
  sleep(1);
  for(i = 0;i < 100;i++)
  {
    arr[i] = '#';
    printf("[%-100s][%d%%][%c]\r",arr,i+1,tag[i%4]);
    fflush(stdout);
    usleep(500000);
  }
  printf("\n");
}


