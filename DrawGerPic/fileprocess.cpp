#include"fileprocess.h"
#include<QDebug>
fileProcess::fileProcess()
{

}

gerb_file_t fileProcess::gerb_fopen(const char *filename)
{
    gerb_file_t *fd;
    struct stat statinfo;
    fd = (gerb_file_t *)g_malloc(sizeof(gerb_file_t));
        if (fd == NULL)
        return NULL;

//将参数filename所指的文件状态, 复制到参数statinfo所指的结构中
    int i=stat(filename, &statinfo);
        if (i < 0)
        {
            fclose(fd->fd);
            g_free(fd);
            return NULL;
        }

     fd->fd = fopen(filename, "rb");//文件流
     fd->ptr = 0;//正在读取的数据的索引
     fd->fileno = _fileno(fd->fd);//The integer version of fd

     int j=fstat(fd->fileno, &statinfo);
     if (j < 0)
     {
         fclose(fd->fd);
         g_free(fd);
         return NULL;
     }

     fd->datalen = (int)statinfo.st_size;//文件大小
     fd->data = calloc(1, statinfo.st_size + 1);//指向文件数据的指针

     int size=fread((void*)fd->data, 1, statinfo.st_size, fd->fd);
     if ( size!= statinfo.st_size)
     {
         fclose(fd->fd);
         g_free(fd->data);
         g_free(fd);
     return NULL;
     }

     rewind (fd->fd); //将文件内部位置指针重新指向文件开头

     qDebug()<<"open gerber successfully!";
     return fd;

}

int fileProcess::gerb_fgetc(gerb_file_t *fd)
{
    if (fd->ptr >= fd->datalen)
    return EOF;

    return (int) fd->data[fd->ptr++];
}

int fileProcess::gerb_fgetint(gerb_file_t *fd, int *len)
{
      long int result;
      char *end;

      result = strtol(fd->data + fd->ptr, &end, 10);

      if (len)
      {
        *len = end - (fd->data + fd->ptr);
      }

      fd->ptr = end - fd->data;

      if (len && (result < 0))
      *len -= 1;

      return (int)result;
}

double fileProcess::gerb_fgetdouble(gerb_file_t *fd)
{
        double result;
        char *end;

        result = strtod(fd->data + fd->ptr, &end);

        fd->ptr = end - fd->data;

        return result;
}

char* fileProcess::gerb_fgetstring(gerb_file_t *fd, char term)
{
    char *strend = NULL;
    char *newstr;
    char *i, *iend;
    int len;

    iend = fd->data + fd->datalen;
    for (i = fd->data + fd->ptr; i < iend; i++) {
    if (*i == term) {
        strend = i;
        break;
    }
    }

    if (strend == NULL)
    return NULL;

    len = strend - (fd->data + fd->ptr);

    newstr = (char *)g_malloc(len + 1);
    if (newstr == NULL)
    return NULL;
    strncpy(newstr, fd->data + fd->ptr, len);
    newstr[len] = '\0';
    fd->ptr += len;

    return newstr;
}

void fileProcess::gerb_ungetc(gerb_file_t *fd)
{
    if (fd->ptr)
        fd->ptr--;

     return;
}

void fileProcess::gerb_fclose(gerb_file_t *fd)
{
    free(fd->data);
    g_free(fd);
}
