#include"parsegerber.h"
#include<QDebug>
parseGerber::parseGerber()
{
    filePro=new fileProcess();
    dgc    =new disposeGerCode();
}

void parseGerber::calc_cirseg_mq(gerbv_net *net, int cw, double delta_cp_x, double delta_cp_y)
{
        double d1x, d1y, d2x, d2y;
        double alfa, beta;

        net->cirseg->cp_x = net->start_x + delta_cp_x;
        net->cirseg->cp_y = net->start_y + delta_cp_y;

        /*
         * Some good values
         */
        d1x = net->start_x - net->cirseg->cp_x;
        d1y = net->start_y - net->cirseg->cp_y;
        d2x = net->stop_x - net->cirseg->cp_x;
        d2y = net->stop_y - net->cirseg->cp_y;

        alfa = atan2(d1y, d1x);
        beta = atan2(d2y, d2x);

        net->cirseg->width = sqrt(delta_cp_x*delta_cp_x + delta_cp_y*delta_cp_y);
        net->cirseg->width *= 2.0;
        net->cirseg->height = net->cirseg->width;

        net->cirseg->angle1 = RAD2DEG(alfa);
        net->cirseg->angle2 = RAD2DEG(beta);

        /*
         * Make sure it's always positive angles
         */
        if (net->cirseg->angle1 < 0.0) {
        net->cirseg->angle1 += 360.0;
        net->cirseg->angle2 += 360.0;
        }

        if (net->cirseg->angle2 < 0.0)
        net->cirseg->angle2 += 360.0;

        if(net->cirseg->angle2 == 0.0)
        net->cirseg->angle2 = 360.0;

        /*
         * This is a sanity check for angles after the nature of atan2.
         * If cw we must make sure angle1-angle2 are always positive,
         * If ccw we must make sure angle2-angle1 are always negative.
         * We should really return one angle and the difference as GTK
         * uses them. But what the heck, it works for me.
         */
        if (cw) {
        if (net->cirseg->angle1 <= net->cirseg->angle2)
            net->cirseg->angle2 -= 360.0;
        } else {
        if (net->cirseg->angle1 >= net->cirseg->angle2)
            net->cirseg->angle2 += 360.0;
        }

        return;
}

void parseGerber::calc_cirseg_sq(gerbv_net *net, int cw, double delta_cp_x, double delta_cp_y)
{
    double d1x, d1y, d2x, d2y;
        double alfa, beta;
        int quadrant = 0;


        /*
         * Quadrant detection (based on ccw, converted below if cw)
         *  Y ^
         *   /!\
         *    !
         *    ---->X
         */
        if (net->start_x > net->stop_x)
        /* 1st and 2nd quadrant */
        if (net->start_y < net->stop_y)
            quadrant = 1;
        else
            quadrant = 2;
        else
        /* 3rd and 4th quadrant */
        if (net->start_y > net->stop_y)
            quadrant = 3;
        else
            quadrant = 4;

        /*
         * If clockwise, rotate quadrant
         */
        if (cw) {
        switch (quadrant) {
        case 1 :
            quadrant = 3;
            break;
        case 2 :
            quadrant = 4;
            break;
        case 3 :
            quadrant = 1;
            break;
        case 4 :
            quadrant = 2;
            break;
        default :
            ;
        }
        }

        /*
         * Calculate arc center point
         */
        switch (quadrant) {
        case 1 :
        net->cirseg->cp_x = net->start_x - delta_cp_x;
        net->cirseg->cp_y = net->start_y - delta_cp_y;
        break;
        case 2 :
        net->cirseg->cp_x = net->start_x + delta_cp_x;
        net->cirseg->cp_y = net->start_y - delta_cp_y;
        break;
        case 3 :
        net->cirseg->cp_x = net->start_x + delta_cp_x;
        net->cirseg->cp_y = net->start_y + delta_cp_y;
        break;
        case 4 :
        net->cirseg->cp_x = net->start_x - delta_cp_x;
        net->cirseg->cp_y = net->start_y + delta_cp_y;
        break;
        default :
        ;
        }

        /*
         * Some good values
         */
        d1x = fabs(net->start_x - net->cirseg->cp_x);
        d1y = fabs(net->start_y - net->cirseg->cp_y);
        d2x = fabs(net->stop_x - net->cirseg->cp_x);
        d2y = fabs(net->stop_y - net->cirseg->cp_y);

        alfa = atan2(d1y, d1x);
        beta = atan2(d2y, d2x);

        /*
         * Avoid divide by zero when sin(0) = 0 and cos(90) = 0
         */
        net->cirseg->width = alfa < beta ?
        2 * (d1x / cos(alfa)) : 2 * (d2x / cos(beta));
        net->cirseg->height = alfa > beta ?
        2 * (d1y / sin(alfa)) : 2 * (d2y / sin(beta));

        if (alfa < 0.000001 && beta < 0.000001) {
        net->cirseg->height = 0;
        }

        switch (quadrant) {
        case 1 :
        net->cirseg->angle1 = RAD2DEG(alfa);
        net->cirseg->angle2 = RAD2DEG(beta);
        break;
        case 2 :
        net->cirseg->angle1 = 180.0 - RAD2DEG(alfa);
        net->cirseg->angle2 = 180.0 - RAD2DEG(beta);
        break;
        case 3 :
        net->cirseg->angle1 = 180.0 + RAD2DEG(alfa);
        net->cirseg->angle2 = 180.0 + RAD2DEG(beta);
        break;
        case 4 :
        net->cirseg->angle1 = 360.0 - RAD2DEG(alfa);
        net->cirseg->angle2 = 360.0 - RAD2DEG(beta);
        break;
        default :
        ;
        }

        return;
}

void parseGerber::gerbv_destroy_image(gerbv_image_t *image)
{
    int i;
    gerbv_net_t *net, *tmp;

    if(image==NULL)
       return;

    for (i = 0; i < APERTURE_MAX; i++)
    {
        g_free(image->aperture[i]);
        image->aperture[i] = NULL;
    }

    if (image->format)
       g_free(image->format);

    for (net = image->netlist; net != NULL; )
    {
        tmp = net;
        net = net->next;
        if (tmp->cirseg != NULL) {
            g_free(tmp->cirseg);
            tmp->cirseg = NULL;
        }

        g_free(tmp);
        tmp = NULL;
    }

    g_free(image);
    image = NULL;

    return;
}

gerbv_image_t* parseGerber::gerbv_create_image(gerbv_image_t *image)
{
    gerbv_destroy_image(image);
    if ((image = (gerbv_image_t *)g_malloc(sizeof(gerbv_image_t))) == NULL) {
    return NULL;
    }
    memset((void *)image, 0, sizeof(gerbv_image_t));

    if ((image->netlist = (gerbv_net_t *)g_malloc(sizeof(gerbv_net_t))) == NULL) {
        g_free(image);
        return NULL;
        }
        memset((void *)image->netlist, 0, sizeof(gerbv_net_t));

    return image;
}

gerbv_net_t* parseGerber::gerber_create_new_net(gerbv_net_t *curr_net)
{
    gerbv_net_t *newNet = g_new0 (gerbv_net_t, 1);
    curr_net->next = newNet;
    return newNet;
}

bool parseGerber::gerber_parse_file_segment(gerbv_image_t *image, gerb_state_t *state, gerbv_net_t *curr_net, gerb_file_t *fd)
{
    int read, coord, len, polygonPoints=0;
    double delta_cp_x = 0.0, delta_cp_y = 0.0;
    double scale;
    bool foundEOF = false;

    while ((read = filePro->gerb_fgetc(fd)) != EOF)
    {
        if (state->unit == GERBV_UNIT_MM)
                    scale = 25.4;
                else
                    scale = 1.0;
        switch ((char)(read & 0xff))
        {
         case '%':
            qDebug()<<"... Found %% code";
            while (1)
            {
                dgc->parse_rs274x(fd,image,state,curr_net);
                int c = filePro->gerb_fgetc(fd);
                while((c == '\n')||(c == '\r')||(c == ' ')||(c == '\t')||(c == 0))
                     c = filePro->gerb_fgetc(fd);

                if(c == EOF || c == '%')
                     break;
                fd->ptr--;
            }
            break;
         case 'G':
            qDebug()<<"... Found G code";
            dgc->parse_G_code(fd, state, image);
            break;
         case 'D':
            qDebug()<<"... Found D code";
            dgc->parse_D_code(fd, state, image);
            break;
         case 'M':
            qDebug()<<"... Found M code";
                switch(dgc->parse_M_code(fd, image))
                {
                case 1 :
                case 2 :
                case 3 :
                foundEOF = true;
                break;
                default:
                }
            break;
          case 'X':
                qDebug()<<"... Found X code";
                coord = filePro->gerb_fgetint(fd, &len);
                if (image->format && image->format->omit_zeros == GERBV_OMIT_ZEROS_TRAILING) {

                switch ((image->format->x_int + image->format->x_dec) - len) {
                case 7:
                    coord *= 10;
                case 6:
                    coord *= 10;
                case 5:
                    coord *= 10;
                case 4:
                    coord *= 10;
                case 3:
                    coord *= 10;
                case 2:
                    coord *= 10;
                case 1:
                    coord *= 10;
                    break;
                default:
                    ;
                }
                }
                if (image->format && (image->format->coordinate==GERBV_COORDINATE_INCREMENTAL))
                    state->curr_x += coord;
                else
                    state->curr_x = coord;
                state->changed = 1;
                break;
            case 'Y':
                qDebug()<<"... Found Y code\n";
                coord = filePro->gerb_fgetint(fd, &len);
                if (image->format && image->format->omit_zeros == GERBV_OMIT_ZEROS_TRAILING) {

                switch ((image->format->y_int + image->format->y_dec) - len) {
                case 7:
                    coord *= 10;
                case 6:
                    coord *= 10;
                case 5:
                    coord *= 10;
                case 4:
                    coord *= 10;
                case 3:
                    coord *= 10;
                case 2:
                    coord *= 10;
                case 1:
                    coord *= 10;
                    break;
                default:
                    ;
                }
                }
                if (image->format && (image->format->coordinate==GERBV_COORDINATE_INCREMENTAL))
                    state->curr_y += coord;
                else
                    state->curr_y = coord;
                state->changed = 1;
                break;
            case 'I':
                qDebug()<<"... Found I code\n";
                coord = filePro->gerb_fgetint(fd, &len);
                if (image->format && image->format->omit_zeros == GERBV_OMIT_ZEROS_TRAILING) {

                switch ((image->format->y_int + image->format->y_dec) - len) {
                case 7:
                    coord *= 10;
                case 6:
                    coord *= 10;
                case 5:
                    coord *= 10;
                case 4:
                    coord *= 10;
                case 3:
                    coord *= 10;
                case 2:
                    coord *= 10;
                case 1:
                    coord *= 10;
                    break;
                default:
                    ;
                }
                }
                state->I = coord;
                state->changed = 1;
                break;
            case 'J':
                qDebug()<<"... Found J code";
                coord = filePro->gerb_fgetint(fd, &len);
                if (image->format && image->format->omit_zeros == GERBV_OMIT_ZEROS_TRAILING) {

                switch ((image->format->y_int + image->format->y_dec) - len) {
                case 7:
                    coord *= 10;
                case 6:
                    coord *= 10;
                case 5:
                    coord *= 10;
                case 4:
                    coord *= 10;
                case 3:
                    coord *= 10;
                case 2:
                    coord *= 10;
                case 1:
                    coord *= 10;
                    break;
                default:
                    ;
                }
                }
                state->J = coord;
                state->changed = 1;
                break;
            case '*':
                qDebug()<<"... Found * code";
                if (state->changed == 0) break;
                state->changed = 0;
                if ((state->aperture_state == GERBV_APERTURE_STATE_OFF)&&(!state->parea_fill_orNot)&&
                                (state->interpolation != GERBV_INTERPOLATION_PAREA_START)) {
                        /* save the coordinate so the next net can use it for a start point */
                        state->prev_x = state->curr_x;
                        state->prev_y = state->curr_y;
                        break;
                        }
                curr_net = gerber_create_new_net (curr_net);

                curr_net->start_x = (double)state->prev_x / scale;
                curr_net->start_y = (double)state->prev_y / scale;
                curr_net->stop_x = (double)state->curr_x / scale;
                curr_net->stop_y = (double)state->curr_y / scale;
                delta_cp_x = (double)state->I / scale;
                delta_cp_y = (double)state->J / scale;

                switch (state->interpolation) {
                        case GERBV_INTERPOLATION_CW_CIRCULAR :
                        curr_net->cirseg = g_new0 (gerbv_cirseg_t,1);
                        if (state->G75_on_orNot)
                            calc_cirseg_mq(curr_net, 1, delta_cp_x, delta_cp_y);
                        else
                            calc_cirseg_sq(curr_net, 1, delta_cp_x, delta_cp_y);
                        break;
                        case GERBV_INTERPOLATION_CCW_CIRCULAR :
                        curr_net->cirseg = g_new0 (gerbv_cirseg_t,1);
                        if (state->G75_on_orNot)
                            calc_cirseg_mq(curr_net, 0, delta_cp_x, delta_cp_y);
                        else
                            calc_cirseg_sq(curr_net, 0, delta_cp_x, delta_cp_y);
                        break;
                        case GERBV_INTERPOLATION_PAREA_START :
                        /*
                         * To be able to get back and fill in number of polygon corners
                         */
                        state->parea_start_node = curr_net;
                        state->parea_fill_orNot = 1;
                        polygonPoints = 0;
                        /* reset the bounding box */
                        break;
                        case GERBV_INTERPOLATION_PAREA_END :
                        state->parea_start_node = NULL;
                        state->parea_fill_orNot= 0;
                        polygonPoints = 0;
                        break;
                        default :
                        break;
                        }

                if (state->parea_fill_orNot && state->parea_start_node) {
                        /*
                         * "...all lines drawn with D01 are considered edges of the
                         * polygon. D02 closes and fills the polygon."
                         * p.49 rs274xrevd_e.pdf
                         * D02 -> state->aperture_state == GERBV_APERTURE_STATE_OFF
                         */

                         /* UPDATE: only end the polygon during a D02 call if we've already
                            drawn a polygon edge (with D01) */

                        if ((state->aperture_state == GERBV_APERTURE_STATE_OFF &&
                                state->interpolation != GERBV_INTERPOLATION_PAREA_START) && (polygonPoints > 0)) {
                            curr_net->interpolation = GERBV_INTERPOLATION_PAREA_END;
                            curr_net = gerber_create_new_net (curr_net);
                            curr_net->interpolation = GERBV_INTERPOLATION_PAREA_START;
                            state->parea_start_node->boundingBox = boundingBox;
                            state->parea_start_node = curr_net;
                            polygonPoints = 0;
                            curr_net = gerber_create_new_net (curr_net);
                            curr_net->start_x = (double)state->prev_x / scale;
                            curr_net->start_y = (double)state->prev_y / scale;
                            curr_net->stop_x = (double)state->curr_x / scale;
                            curr_net->stop_y = (double)state->curr_y / scale;
                        }
                        else if (state->interpolation != GERBV_INTERPOLATION_PAREA_START)
                            polygonPoints++;

                        }  /* if (state->in_parea_fill && state->parea_start_node) */

               curr_net->interpolation = state->interpolation;

               if (((state->interpolation == GERBV_INTERPOLATION_CW_CIRCULAR) ||
                        (state->interpolation == GERBV_INTERPOLATION_CCW_CIRCULAR)) &&
                       ((state->I == 0.0) && (state->J == 0.0)))
                       curr_net->interpolation = GERBV_INTERPOLATION_LINEARx1;

               if ((state->interpolation == GERBV_INTERPOLATION_PAREA_START) ||
                       (state->interpolation == GERBV_INTERPOLATION_PAREA_END))
                       state->interpolation = state->prev_interpolation;

               state->delta_cp_x = 0.0;
               state->delta_cp_y = 0.0;
               curr_net->aperture = state->curr_aperture;
               curr_net->aperture_state = state->aperture_state;

               /*
                * For next round we save the current position as
                * the previous position
                */
               state->prev_x = state->curr_x;
               state->prev_y = state->curr_y;

               /*
                * If we have an aperture defined at the moment we find
                * min and max of image with compensation for mm.
                */
               if ((curr_net->aperture == 0) && !state->in_parea_fill)
               break;
            break;

    }
  }
       return foundEOF;
}

gerbv_image_t* parseGerber::parse_gerber(gerb_file_t *fd)
{
    gerb_state_t *state = NULL;//当前坐标点的状态
    gerbv_image_t *image = NULL;//保存图层结构
    gerbv_net_t *curr_net = NULL;//用于容纳几何实体的结构
    bool foundEOF = false;

    state = g_new0 (gerb_state_t, 1);
    image = gerbv_create_image(image);
    if (image == NULL)
        qDebug()<<"create image struct failed";
    curr_net = image->netlist;

    foundEOF = gerber_parse_file_segment(image, state, curr_net,fd);

    return image;
}

void parseGerber::gerbv_open_image(gerbv_image_t *parsed_image, const char *filename)
{
    gerb_file_t *fd;//文件数据信息
    gerbv_image_t *parse_image = NULL;

    fd = filePro->gerb_fopen(filename);
    if (fd == NULL)
    {
    qDebug()<<"failed to open gerber";
    return -1;
    }

    parse_image = parse_gerber(fd);

    gerbv_net_t *currentNet;

        /* run through and find last net pointer */
    for (currentNet = parsed_image->netlist; currentNet->next; currentNet = currentNet->next)
     {
         if (parsed_image->aperture[currentNet->aperture] == NULL)
         {
             parse_image->aperture[currentNet->aperture] = g_new0 (gerbv_aperture_t, 1);
             parse_image->aperture[currentNet->aperture]->type = GERBV_APTYPE_CIRCLE;
             parse_image->aperture[currentNet->aperture]->parameter[0] = 0;
             parse_image->aperture[currentNet->aperture]->parameter[1] = 0;
         }
     }

}

