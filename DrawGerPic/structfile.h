#ifndef STRUCTFILE_H
#define STRUCTFILE_H
#include <stdio.h>

#define APERTURE_PARAMETERS_MAX 102
#define APERTURE_MAX 9999
#define APERTURE_MIN 10

typedef enum {GERBV_APERTURE_STATE_OFF,
        GERBV_APERTURE_STATE_ON,
        GERBV_APERTURE_STATE_FLASH
} gerbv_aperture_state_t;

typedef enum {GERBV_INTERPOLATION_LINEARx1, /*!< draw a line */
        GERBV_INTERPOLATION_x10, /*!< draw a line */
        GERBV_INTERPOLATION_LINEARx01, /*!< draw a line */
        GERBV_INTERPOLATION_LINEARx001, /*!< draw a line */
        GERBV_INTERPOLATION_CW_CIRCULAR, /*!< draw an arc in the clockwise direction */
        GERBV_INTERPOLATION_CCW_CIRCULAR, /*!< draw an arc in the counter-clockwise direction */
        GERBV_INTERPOLATION_PAREA_START, /*!< start a polygon draw */
        GERBV_INTERPOLATION_PAREA_END, /*!< end a polygon draw */
        GERBV_INTERPOLATION_DELETED /*!< the net has been deleted by the user, and will not be drawn */
} gerbv_interpolation_t;

typedef enum {GERBV_OMIT_ZEROS_LEADING, /*!< omit extra zeros before the decimal point */
        GERBV_OMIT_ZEROS_TRAILING, /*!< omit extra zeros after the decimal point */
        GERBV_OMIT_ZEROS_EXPLICIT, /*!< explicitly specify how many decimal places are used */
        GERBV_OMIT_ZEROS_UNSPECIFIED /*!< use the default parsing style */
} gerbv_omit_zeros_t;

typedef enum {GERBV_COORDINATE_ABSOLUTE, /*!< all coordinates are absolute from a common origin */
        GERBV_COORDINATE_INCREMENTAL /*!< all coordinates are relative to the previous coordinate */
} gerbv_coordinate_t;

typedef enum {GERBV_APTYPE_NONE, /*!< no aperture used */
        GERBV_APTYPE_CIRCLE, /*!< a round aperture */
        GERBV_APTYPE_RECTANGLE, /*!< a rectangular aperture */
        GERBV_APTYPE_OVAL, /*!< an ovular (obround) aperture */
        GERBV_APTYPE_POLYGON, /*!< a polygon aperture */
} gerbv_aperture_type_t;

typedef enum {GERBV_UNIT_INCH, /*!< inches */
        GERBV_UNIT_MM, /*!< mm */
        GERBV_UNIT_UNSPECIFIED /*!< use default units */
} gerbv_unit_t;

    typedef struct file {
        FILE *fd;     /* File descriptor */
        int   fileno; /* The integer version of fd */
        char *data;   /* Pointer to data mmaped in. May not be changed, use ptr */
        int   datalen;/* Length of mmaped data ie file length */
        int   ptr;    /* Index in data where we are reading */
    } gerb_file_t;

typedef struct gerbv_cirseg {
    double cp_x;   /* center point x */
    double cp_y;   /* center point y */
    double width;  /* of oval */
    double height; /* of oval */
    double angle1;
    double angle2;
} gerbv_cirseg_t;

typedef struct gerbv_aperture {
    gerbv_aperture_type_t type;
    double parameter[APERTURE_PARAMETERS_MAX];
    int nuf_parameters;
    gerbv_unit_t unit;
} gerbv_aperture_t;

 struct gerb_state_t{
    int curr_x;
    int curr_y;
    int prev_x;
    int prev_y;
    int I;
    int J;
    int curr_aperture;
    int changed;
    int parea_fill_orNot;
    int G75_on_orNot;
    gerbv_unit_t unit;
    gerbv_aperture_state_t aperture_state;
    gerbv_interpolation_t interpolation;
    gerbv_interpolation_t prev_interpolation;
    gerbv_net_t *parea_start_node;
};

typedef struct gerbv_net{
    double start_x;
    double start_y;
    double stop_x;
    double stop_y;
    int aperture;
    gerbv_aperture_state_t aperture_state; /*!< the state of the aperture tool (on/off/etc) */
    gerbv_interpolation_t interpolation; /*!< the path interpolation method (linear/etc) */
    gerbv_cirseg_t *cirseg; /*!< information for arc nets */
    struct gerbv_net *next;
} gerbv_net_t;

 typedef struct gerbv_format {
     gerbv_omit_zeros_t omit_zeros;
     gerbv_coordinate_t coordinate;
     int x_int;
     int x_dec;
     int y_int;
     int y_dec;
 } gerbv_format_t;

 typedef struct {
   gerbv_aperture_t *aperture[APERTURE_MAX]; /*!< an array with all apertures used */
   gerbv_format_t *format; /*!< formatting info */
   gerbv_net_t *netlist; /*!< an array of all geometric entities in the layer */
 } gerbv_image_t;
#endif // STRUCTFILE_H
