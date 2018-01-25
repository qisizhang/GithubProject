#ifndef FILEPROCESS_H
#define FILEPROCESS_H
#include"structfile.h"
#include<sstream>
class fileProcess
{
public:
    fileProcess();
    gerb_file_t *gerb_fopen(char const* filename);
    int gerb_fgetc(gerb_file_t *fd);
    int gerb_fgetint(gerb_file_t *fd, int *len); /* If len != NULL, returns number
                            of chars parsed in len */
    double gerb_fgetdouble(gerb_file_t *fd);
    char *gerb_fgetstring(gerb_file_t *fd, char term);
    void gerb_ungetc(gerb_file_t *fd);
    void gerb_fclose(gerb_file_t *fd);
};

#endif // FILEPROCESS_H
