#include <stdio.h>
#include <windows.h>
/*******************************************************************************
目前仍然有很多的BUG
但是繼續修復也沒有什麼實際的意義
需要了解的關於文字編碼的知識太多
*******************************************************************************/
#define _UNICODE    1
#define _ASCI932    2
#define _ASCI936    3
#define _ASCI950    4
#define _UTF_8      5
char *modify[] = {
    "_NULL","_UNICODE","_ASCI932","_ASCI936","_ASCI950","_UTF_8",
};

FILE * FileOpen(const char * file)
{
    if(file==0){
        printf("open error\n");   
        exit(0);
    }
    FILE * fp = fopen(file,"rb");
    if(fp == 0)
    {
        perror("'rb' open error\n");
        exit(0);
    }
    return fp;
}

void FilePrint(FILE * fp)
{
    char buf[1];
    fseek(fp,0,SEEK_END);
    long end = ftell(fp);
    fseek(fp,0,SEEK_SET);
    while(ftell(fp)!=end)
    {
        fread(buf,sizeof(buf),1,fp);
        printf("%c",buf[0]);
        static int len = 0;
        //printf("%02X ",buf[0]);
        //if(++len%16==0)printf("\n");

    }
    printf("\noldsize %d %ld\n",ftell(fp),end);
    fseek(fp,0,SEEK_SET);
}

//return the location where is the last char, or
//-1:error
int FindLastCharFromStr(char c,const char * str)
{
    int len = strlen(str);
    int location = -1;
    for(int i=0;i<len;i++)
    {
        if(str[i]==c)location = i;
    }
    return location;
}
void SetFileName(char * newname,const char * oldname,const char * modify)
{
    int len = strlen(oldname);
    int dot = FindLastCharFromStr('.',oldname);
    for(int i=0,j=0;i<=len;i++,j++)
    {
        if(i==len)
        {
            newname[j] = 0;
            break;
        }
        if(i!=dot)
        {
            newname[j] = oldname[i];
        }
        else
        {
            newname[j] = 0;
            strcat(newname,modify);
            j = strlen(newname);
            newname[j] = '.';
        }
    }
}
FILE * FileCreate(const char * oldfilename,int encoding)
{
    char filename[255] = {0};
    SetFileName(filename,oldfilename,modify[encoding]);
    FILE * fp = fopen(filename,"wb");
    return fp;
}

void toUnicode(char * olddata,wchar_t * newdata,long size,int oldcode)
{
    MultiByteToWideChar(oldcode,0,olddata,-1,newdata+1,size);
    newdata[0] = 0xFEFF;
}

long CompFileSize(FILE *fp)
{
    fseek(fp,0,SEEK_END);
    long size = ftell(fp);
    fseek(fp,0,SEEK_SET);
    return size;
}
//return 0:success; -1:malloc error;
int FileWriteIn(FILE* oldfile,FILE* newfile,int oldcode,int newcode)
{
    long oldfilesize = CompFileSize(oldfile);
    char * olddata = (char*)calloc(sizeof(char),oldfilesize);
    if(olddata==NULL)
    {
        return -1;
    }
    fread(olddata,oldfilesize,1,oldfile);

    long newfilesize = MultiByteToWideChar(oldcode,0,olddata,-1,NULL,0);
    wchar_t * newdata = (wchar_t *)calloc(newfilesize,sizeof(wchar_t));
    if(newdata==NULL)
    {
        return -1;
    }
    switch(newcode){
    default:
        toUnicode(olddata,newdata,newfilesize,oldcode);
    }
    fwrite(newdata,sizeof(wchar_t),newfilesize,newfile);
    return 0;
}
int JudgeEncoding(FILE *fp)
{
    unsigned char buf[80] ={0};
    fread(buf,sizeof(char),4,fp);
    fseek(fp,0,SEEK_SET);
    if(((buf[0]<<16)+(buf[1]<<8)+buf[2])==0xEFBBBF){
        return CP_UTF8;
    }
    switch((buf[0]<<8)+buf[1]){
    default:
        return CP_ACP;
    }
}
/*******************************************************************************
*******************************************************************************/
void CheckMain()
{
    char * oldfilename = "2.txt";
    int newencod     = _UNICODE;
    FILE * fr = FileOpen(oldfilename);
    int oldencod     = CP_ACP;//JudgeEncoding(fr);
    //FilePrint(fr);
    FILE * fw = FileCreate(oldfilename,newencod);
    int errorcode = FileWriteIn(fr,fw,oldencod,newencod);
    if(errorcode==-1)printf("malloc error\n");
    fclose(fr);
    fclose(fw);
}
void CheckFindLastCharFromStrBegin(char c,const char * str)
{
    printf("[%-12s] [%c] ",str,c);
    printf(" %d\n",FindLastCharFromStr(c,str));
}
void CheckFindLastCharFromStrEnter()
{
    char * str[] = {"123456","111.111..","11.1.1...","-=/*-"    ,"     "    ,"我是誰"   };
    char c[]     = {'_'     ,'.'        ,'1'        ,'-'        ,' '        ,' '        };
    for(int i=0;i<6;i++)
    {
        CheckFindLastCharFromStrBegin(c[i],str[i]);
    }
}
void CheckSetFileNameBegin(char * newname,const char * oldname,const char * modify)
{
    printf("[%-12s] [%-10s]",oldname,modify);
    SetFileName(newname,oldname,modify);
    printf("\t[%s]\n",newname);
}
void CheckSetFileNameEnter()
{
    char * oldname[] = {"1.txt","a.cpp","a.cpp.a.cpp","haha,,..c"};
    //char * modify[]  = {"_UNICODE","ASIC","__","#"};
    char newname[255] = {0};
    for(int i=0;i<4;i++)
    {
        CheckSetFileNameBegin(newname,oldname[i],modify[i]);
    }
}
/*******************************************************************************
*******************************************************************************/
int main(int argc, char **argv){
    //CheckMain();
    //CheckFindLastCharFromStrEnter();
    //CheckSetFileNameEnter();
    int newencod     = _UNICODE;
    FILE * fr = FileOpen(argv[1]);
    int oldencod     = JudgeEncoding(fr);
    FilePrint(fr);
    FILE * fw = FileCreate(argv[1],newencod);
    FileWriteIn(fr,fw,oldencod,newencod);
    fclose(fr);
    fclose(fw);
    return 0;
}