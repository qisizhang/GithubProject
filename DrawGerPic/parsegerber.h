#ifndef PARSEGERBER_H
#define PARSEGERBER_H
#include"fileprocess.h"
#include"disposegercode.h"

#define M_PI 3.14159265358979323846
#define RAD2DEG(a) (a * 180 / M_PI)
class parseGerber
{
public:
    parseGerber();

    void calc_cirseg_mq(struct gerbv_net *net, int cw, double delta_cp_x, double delta_cp_y);
    void calc_cirseg_sq(struct gerbv_net *net, int cw, double delta_cp_x, double delta_cp_y);

    void gerbv_destroy_image(gerbv_image_t *image);
    gerbv_image_t* gerbv_create_image(gerbv_image_t* image);
    gerbv_net_t* gerber_create_new_net(gerbv_net_t *curr_net);
    bool gerber_parse_file_segment(gerbv_image_t *image,gerb_state_t *state,gerbv_net_t *curr_net,gerb_file_t *fd);
    void gerbv_open_image(gerbv_image_t *parsed_image, const char *filename);
    gerbv_image_t* parse_gerber(gerb_file_t *fd);

private:
    fileProcess* filePro;
    disposeGerCode* dgc;

};

#endif // PARSEGERBER_H
