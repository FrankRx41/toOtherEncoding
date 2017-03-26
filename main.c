#include <stdio.h>
#include <windows.h>

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
    FILE * fp = fopen(file,"rb");
    if(fp == 0)
    {
        perror("'rb' open error");
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

int FileWriteIn(FILE* oldfile,FILE* newfile,int oldcode,int newcode)
{
    fseek(oldfile,0,SEEK_END);
    long oldfilesize = ftell(oldfile);
    fseek(oldfile,0,SEEK_SET);
    printf("size %d\n",oldfilesize);
    char * olddata = (char*)calloc(sizeof(char),oldfilesize);
    if(olddata==NULL)
    {
        perror("olddata error");
        return -1;
    }
    fread(olddata,oldfilesize,1,oldfile);
    for(int i=0;i<oldfilesize;i++)
    {
        printf("%02X ",olddata[i]);
    }
    printf("%s\n",olddata);
    long size = MultiByteToWideChar(oldcode,0,olddata,-1,NULL,0);
    wchar_t * newdata = (wchar_t *)calloc(size,sizeof(wchar_t));
    if(newdata==NULL)
    {
        perror("newdata error");
        return -1;
    }
    MultiByteToWideChar(oldcode,0,olddata,-1,newdata,size);
    unsigned char buf  = 0;
    buf = 0xFF;
    fwrite(&buf,sizeof(buf),1,newfile);
    buf = 0xFE;
    fwrite(&buf,sizeof(buf),1,newfile);

    fwrite(newdata,sizeof(wchar_t),size-1,newfile);
    printf("new size %ld\n",size);
    //MultiByteToWideChar(CP_UTF8,0,(LPCSTR)data,4,wch,1024);
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
    FilePrint(fr);
    FILE * fw = FileCreate(oldfilename,newencod);
    FileWriteIn(fr,fw,oldencod,newencod);
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
    char * str[] = {"123456","111.111..","11.1.1...","-=/*-"    ,"     "    ,"‰ä¥’N"   };
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
    CheckMain();
    //CheckFindLastCharFromStrEnter();
    //CheckSetFileNameEnter();
    return 0;
}