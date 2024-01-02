#include "_fabvars.h"

float
koko_fabvars_xmin(struct fabvars_t *f) {
    if(f->_shapes.length <=0 )
        return 0; 
    float xmax = vec_first(&f->_shapes)->bounds.xmax;
    float xmin = vec_first(&f->_shapes)->bounds.xmin;
    for(int i=1; i< f->_shapes.length; i++) {
        xmax = fmax(xmax, f->_shapes.data[i]->bounds.xmax);
        xmin = fmin(xmin, f->_shapes.data[i]->bounds.xmin);
    }
    float dx = xmax-xmin;
    return (xmin-dx*f->border/2.0);
}
float
koko_fabvars_xmax(struct fabvars_t *f) {
   if(f->_shapes.length <=0 )
        return 0; 
    float xmax = vec_first(&f->_shapes)->bounds.xmax;
    float xmin = vec_first(&f->_shapes)->bounds.xmin;
    for(int i=1; i< f->_shapes.length; i++) {
        xmax = fmax(xmax, f->_shapes.data[i]->bounds.xmax);
        xmin = fmin(xmin, f->_shapes.data[i]->bounds.xmin);
    }
    float dx = xmax-xmin;
    return (xmax+dx*f->border/2.0);
}
float
koko_fabvars_dx(struct fabvars_t *f) {
    return koko_fabvars_xmax(f) - koko_fabvars_xmin(f);
}
float
koko_fabvars_ymin(struct fabvars_t *f) {
   if(f->_shapes.length <=0 )
        return 0; 
    float ymax = vec_first(&f->_shapes)->bounds.ymax;
    float ymin = vec_first(&f->_shapes)->bounds.ymin;
    for(int i=1; i< f->_shapes.length; i++) {
        ymax = fmax(ymax, f->_shapes.data[i]->bounds.ymax);
        ymin = fmin(ymin, f->_shapes.data[i]->bounds.ymin);
    }
    float dy = ymax-ymin;
    return (ymin-dy*f->border/2.0);
}
float
koko_fabvars_ymax(struct fabvars_t *f) {
   if(f->_shapes.length <=0 )
        return 0; 
    float ymax = vec_first(&f->_shapes)->bounds.ymax;
    float ymin = vec_first(&f->_shapes)->bounds.ymin;
    for(int i=1; i< f->_shapes.length; i++) {
        ymax = fmax(ymax, f->_shapes.data[i]->bounds.ymax);
        ymin = fmin(ymin, f->_shapes.data[i]->bounds.ymin);
    }
    float dy = ymax-ymin;
    return (ymax+dy*f->border/2.0);
}
float
koko_fabvars_dy(struct fabvars_t *f) {
    return koko_fabvars_ymax(f) - koko_fabvars_ymin(f);
}

float
koko_fabvars_zmin(struct fabvars_t *f) {
   if(f->_shapes.length <=0 )
        return 0; 
    float zmax = vec_first(&f->_shapes)->bounds.zmax;
    float zmin = vec_first(&f->_shapes)->bounds.zmin;
    for(int i=1; i< f->_shapes.length; i++) {
        zmax = fmax(zmax, f->_shapes.data[i]->bounds.zmax);
        zmin = fmin(zmin, f->_shapes.data[i]->bounds.zmin);
    }
    float dz = zmax-zmin;
    return (zmin-dz*f->border/2.0);
}
float
koko_fabvars_zmax(struct fabvars_t *f) {
   if(f->_shapes.length <=0 )
        return 0; 
    float zmax = vec_first(&f->_shapes)->bounds.zmax;
    float zmin = vec_first(&f->_shapes)->bounds.zmin;
    for(int i=1; i< f->_shapes.length; i++) {
        zmax = fmax(zmax, f->_shapes.data[i]->bounds.zmax);
        zmin = fmin(zmin, f->_shapes.data[i]->bounds.zmin);
    }
    float dz = zmax-zmin;
    return (zmax+dz*f->border/2.0);
}
float
koko_fabvars_dz(struct fabvars_t *f) {
     return koko_fabvars_zmax(f) - koko_fabvars_zmin(f);
}
bool
koko_fabvars_bounded(struct fabvars_t *f) {
    return (isnormal(koko_fabvars_xmin(f)) &&
                isnormal(koko_fabvars_xmax(f)) &&
                    isnormal(koko_fabvars_ymin(f)) && 
                isnormal(koko_fabvars_ymax(f)) &&
            isnormal(koko_fabvars_zmin(f)) &&
   isnormal(koko_fabvars_zmax(f)));
}