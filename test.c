#include<stdio.h>  
#include<string.h>  
#include<errno.h> 
#include <stdlib.h>  /* exit() 函数 */
int writenonce();
int readsign();
int main(int argc,char*argv[]){  
    writenonce();
    FILE *fstream=NULL;    
    char buff[1024];  
    memset(buff,0,sizeof(buff));  
    if(NULL==(fstream=popen("tpm2_hash -H e -g 0x00B -I datain.txt -o hash.bin -t ticket.bin && tpm2_sign -k 0x81000005 -P RSAleaf123 -g 0x000B -m datain.txt -s signature.bin -t ticket.bin","r")))    
    {   
        fprintf(stderr,"execute command failed: %s",strerror(errno));    
        return -1;    
    }   
    while(NULL!=fgets(buff, sizeof(buff), fstream)) {
            printf("%s",buff);  
    }
    pclose(fstream);  
    readsign();
    return 0;   
} 
 
int writenonce()
{
   char * sentence = "helloworld";
   FILE *fptr;
 
   fptr = fopen("datain.txt", "w");
   if(fptr == NULL)
   {
      printf("Error!");
      exit(1);
   }
   
 
   fprintf(fptr,"%s", sentence);
   fclose(fptr);
 
   return 0;
}
int readsign(){
    FILE *fp;
    char ch;
   
    //如果文件不存在，给出提示并退出
    if( (fp=fopen("signature.bin","rt")) == NULL ){
        printf("Cannot open file, press any key to exit!");
        exit(1);
    }
    //每次读取一个字节，直到读取完毕
    while( (ch=fgetc(fp)) != EOF ){
        putchar(ch);
    }
    putchar('\n');  //输出换行符
    fclose(fp);
    return 0;
}

