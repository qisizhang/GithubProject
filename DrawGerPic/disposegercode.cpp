#include"disposegercode.h"
#include<QDebug>

disposeGerCode::disposeGerCode()
{
    filePro=new fileProcess();
}

void disposeGerCode::parse_D_code(gerb_file_t *fd, gerb_state_t *state, gerbv_image_t *image)
{
    int a;
    a = filePro->gerb_fgetint(fd, NULL);

    switch(a) {
        case 0 : /* Invalid code */
        break;
        case 1 : /* Exposure on */
        state->aperture_state = GERBV_APERTURE_STATE_ON;
        state->changed = 1;
        break;
        case 2 : /* Exposure off */
        state->aperture_state = GERBV_APERTURE_STATE_OFF;
        state->changed = 1;
        break;
        case 3 : /* Flash aperture */
        state->aperture_state = GERBV_APERTURE_STATE_FLASH;
        state->changed = 1;
        break;
        default: /* Aperture in use */
        if ((a >= 0) && (a <= APERTURE_MAX)) {
            state->curr_aperture = a;
        }
        state->changed = 0;
        break;
        }

        return;
}

void disposeGerCode::parse_G_code(gerb_file_t *fd, gerb_state_t *state, gerbv_image_t *image)
{
    int op_int;
    gerbv_format_t *format = image->format;
    int c;
    char *string;

    op_int=filePro->gerb_fgetint(fd, NULL);

    switch(op_int)
    {
    case 0:  /* Move */
        break;
    case 1:  /* Linear Interpolation (1X scale) */
        state->interpolation = GERBV_INTERPOLATION_LINEARx1;
        break;
     case 2:  /* Clockwise Linear Interpolation */
        state->interpolation = GERBV_INTERPOLATION_CW_CIRCULAR;
        break;
     case 3:  /* Counter Clockwise Linear Interpolation */
        state->interpolation = GERBV_INTERPOLATION_CCW_CIRCULAR;
        break;
     case 4:  /* Ignore Data Block */
        /* Don't do anything, just read 'til * */
            /* SDB asks:  Should we look for other codes while reading G04 in case
         * user forgot to put * at end of comment block? */
        c = filePro->gerb_fgetc(fd);
        while ((c != EOF) && (c != '*')) {
            c = filePro->gerb_fgetc(fd);
        }
        break;
     case 10: /* Linear Interpolation (10X scale) */
        state->interpolation = GERBV_INTERPOLATION_x10;
        break;
     case 11: /* Linear Interpolation (0.1X scale) */
        state->interpolation = GERBV_INTERPOLATION_LINEARx01;
        break;
     case 12: /* Linear Interpolation (0.01X scale) */
        state->interpolation = GERBV_INTERPOLATION_LINEARx001;
        break;
     case 36: /* Turn on Polygon Area Fill */
        state->prev_interpolation = state->interpolation;
        state->interpolation = GERBV_INTERPOLATION_PAREA_START;
        state->changed = 1;
        break;
     case 37: /* Turn off Polygon Area Fill */
        state->interpolation = GERBV_INTERPOLATION_PAREA_END;
        state->changed = 1;
        break;
        case 54: /* Tool prepare */
        /* XXX Maybe uneccesary??? */
        if (filePro->gerb_fgetc(fd) == 'D')
        {
            int a = filePro->gerb_fgetint(fd, NULL);
            if ((a >= 0) && (a <= APERTURE_MAX))
            {
            state->curr_aperture = a;
            } else {
            qDebug()<<"Found aperture D%d out of bounds while parsing G code in file";
            g_free(string);
            }
        } else {
            qDebug()<<"Found unexpected code after G54 in file";
        }
        break;
     case 55: /* Prepare for flash */
        break;
     case 70: /* Specify inches */
        state->unit = GERBV_UNIT_INCH;
        break;
     case 71: /* Specify millimeters */
        state->state->unit = GERBV_UNIT_MM;
        break;
     case 74: /* Disable 360 circular interpolation */
        state->G75_on_orNot = 0;
        break;
     case 75: /* Enable 360 circular interpolation */
        state->G75_on_orNot = 1;
        break;
     case 90: /* Specify absolut format */
        if (format) format->coordinate = GERBV_COORDINATE_ABSOLUTE;
        break;
     case 91: /* Specify incremental format */
        if (format) format->coordinate = GERBV_COORDINATE_INCREMENTAL;
        break;
        default:
        /* Enter error count here */
        break;
    }

    return;
}

int disposeGerCode::parse_M_code(gerb_file_t *fd, gerbv_image_t *image)
{
    int op_int;

    op_int=filePro->gerb_fgetint(fd, NULL);

    switch (op_int)
    {
    case 0:  /* Program stop */
        return 1;
    case 1:  /* Optional stop */
        return 2;
    case 2:  /* End of program */
        return 3;
    default:
    }
    return 0;
}

int disposeGerCode::parse_aperture_definition(gerb_file_t *fd, gerbv_aperture_t *aperture, gerbv_image_t *image, double scale)
{
    int ano, i;
    char *ad;
    char *token;
    double tempHolder;

    if (filePro->gerb_fgetc(fd) != 'D')
    {
        return -1;
    }

    ano = filePro->gerb_fgetint(fd, NULL);
    ad  = filePro->gerb_fgetstring(fd, '*');
    token = strtok(ad, ",");
    if (token == NULL)
    {
        return -1;
    }

    if (strlen(token) == 1)
    {
    switch (token[0])
        {
    case 'C':
        aperture->type = GERBV_APTYPE_CIRCLE;
        break;
    case 'R' :
        aperture->type = GERBV_APTYPE_RECTANGLE;
        break;
    case 'O' :
        aperture->type = GERBV_APTYPE_OVAL;
        break;
    case 'P' :
        aperture->type = GERBV_APTYPE_POLYGON;
        break;
        }
    }

    for(token = strtok(NULL, "X"), i = 0; token != NULL; token = strtok(NULL, "X"), i++)
    {
        tempHolder = strtod(token, NULL);

        if(!((aperture->type == GERBV_APTYPE_POLYGON) && ((i==1) || (i==2))))
            tempHolder /= scale;

        aperture->parameter[i] = tempHolder;

    }

    aperture->nuf_parameters = i;

    filePro->gerb_ungetc(fd);

    g_free(ad);

    return ano;
}

void disposeGerCode::parse_rs274x(gerb_file_t *fd, gerbv_image_t *image, gerb_state_t *state, gerbv_net_t *curr_net)
{
    int op[2];
    char str[3];
    int tmp;
    int ano;
    gerbv_aperture_t *a = NULL;
    double scale = 1.0;
    if (state->unit == GERBV_UNIT_MM)
            scale = 25.4;

    op[0] = filePro->gerb_fgetc(fd);
    op[1] = filePro->gerb_fgetc(fd);
    if ((op[0] == EOF) || (op[1] == EOF))
    {
        qDebug()<<"Unexpected EOF found in file";
    }

    switch (A2I(op[0], op[1]))
    {
        case A2I('F','S'):
        image->format = g_new0 (gerbv_format_t,1);
        switch (filePro->gerb_fgetc(fd))
        {
            case 'L':
                image->format->omit_zeros = GERBV_OMIT_ZEROS_LEADING;
                break;
            case 'T':
                image->format->omit_zeros = GERBV_OMIT_ZEROS_TRAILING;
                break;
            case 'D':
                image->format->omit_zeros = GERBV_OMIT_ZEROS_EXPLICIT;
                break;
            default:
                image->format->omit_zeros = GERBV_OMIT_ZEROS_LEADING;
         }

        switch (filePro->gerb_fgetc(fd))
        {
            case 'A':
                image->format->coordinate = GERBV_COORDINATE_ABSOLUTE;
                break;
            case 'I':
                image->format->coordinate = GERBV_COORDINATE_INCREMENTAL;
                break;
            default:
                image->format->coordinate = GERBV_COORDINATE_ABSOLUTE;
        }

        op[0] = filePro->gerb_fgetc(fd);
        while((op[0] != '*')&&(op[0] != EOF))
        {
            switch (op[0])
            {
                case 'X' :
                op[0] = filePro->gerb_fgetc(fd);
                if ((op[0] < '0') || (op[0] > '6'))
                {
                    qDebug()<<"Illegal format size %c in file";
                }
                image->format->x_int = op[0] - '0';
                op[0] = filePro->gerb_fgetc(fd);
                image->format->x_dec = op[0] - '0';
                break;
                case 'Y':
                op[0] = filePro->gerb_fgetc(fd);
                if ((op[0] < '0') || (op[0] > '6'))
                {
                    qDebug()<<"Illegal format size %c in file";
                }
                image->format->y_int = op[0] - '0';
                op[0] = filePro->gerb_fgetc(fd);
                image->format->y_dec = op[0] - '0';
                break;
                default :

            }
            op[0] = filePro->gerb_fgetc(fd);
        }
        break;

        case A2I('M','O'): /* Mode of Units */
        op[0] = gerb_fgetc(fd);
        op[1] = gerb_fgetc(fd);
        switch (A2I(op[0],op[1]))
        {
            case A2I('I','N'):
            state->unit = GERBV_UNIT_INCH;
            break;
            case A2I('M','M'):
            state->unit = GERBV_UNIT_MM;
            break;
            default:
        }
        break;

        case A2I('A','D'): /* Aperture Description */
        a = (gerbv_aperture_t *) g_new0 (gerbv_aperture_t,1);
        ano = parse_aperture_definition(fd, a, image, scale);
        if((ano >= 0) && (ano <= APERTURE_MAX))
        {
            a->unit = state->unit;
            image->aperture[ano] = a;
            if (ano < APERTURE_MIN)
            {
                qDebug()<<"In file,aperture number out of bounds";
            }
        }
        break;

        default:
        fd->ptr--;
        int c = gerb_fgetc(fd);
        while ((c != EOF) && (c != '*'))
            c = filePro->gerb_fgetc(fd);
        return;
    }

}
