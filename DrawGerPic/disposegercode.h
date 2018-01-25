#ifndef DISPOSEGERCODE_H
#define DISPOSEGERCODE_H
#include "structfile.h"
#include"fileprocess.h"

#define A2I(a,b) (((a & 0xff) << 8) + (b & 0xff))
class disposeGerCode
{
public:
    disposeGerCode();
    void parse_G_code(gerb_file_t *fd,gerb_state_t *state,gerbv_image_t *image);
    void parse_D_code(gerb_file_t *fd,gerb_state_t *state,gerbv_image_t *image);
    int parse_M_code(gerb_file_t *fd,gerbv_image_t *image);
    void parse_rs274x(gerb_file_t *fd, gerbv_image_t *image, gerb_state_t *state, gerbv_net_t *curr_net);
    int parse_aperture_definition(gerb_file_t *fd, gerbv_aperture_t *aperture,gerbv_image_t *image,double scale);

private:
    fileProcess* filePro;
}

#endif // DISPOSEGERCODE_H
