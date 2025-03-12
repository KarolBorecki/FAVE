#ifndef FAVE_TYPES_H
#define FAVE_TYPES_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct Pair
    {
        int first;
        int second;
    } Pair_t;

        enum CellType : int
    {
        FLUID = 0,
        SOLID = 1,
        AIR = 2
    };

    typedef struct FLIPGrid
    {
        float density;
        int f_num_x;
        int f_num_y;
        int f_num_z;
        float h;
        float f_inv_spacing;
        int f_num_cells;

        float *u;            // of size f_num_cells
        float *v;            // of size f_num_cells
        float *w;            // of size f_num_cells
        float *du;           // of size f_num_cells
        float *dv;           // of size f_num_cells
        float *dw;           // of size f_num_cells
        float *prev_u;       // of size f_num_cells
        float *prev_v;       // of size f_num_cells
        float *prev_w;       // of size f_num_cells
        float *p;            // of size f_num_cells
        float *s;            // of size f_num_cells
        CellType *cell_type; // of size f_num_cells
        float *cell_color;   // of size f_num_cells * 3

        int num_particles;
        float *particle_pos;     // of size max_particles * 3
        float *particle_vel;     // of size max_particles * 3
        float *particle_density; // of size f_num_cells
        float particle_rest_density;

        float particle_radius;
        float p_inv_spacing;
        int p_num_x;
        int p_num_y;
        int p_num_z;
        int p_num_cells; // p_num_x * p_num_y * p_num_z

        int *num_cell_particles;  // of size f_num_cells
        int *first_cell_particle; // of size f_num_cells + 1
        int *cell_particle_ids;   // of size num_particles

    } FlipGrid_t;

#ifdef __cplusplus
}
#endif

#endif // FAVE_TYPES_H
