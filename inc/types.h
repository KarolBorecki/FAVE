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

    typedef struct CoreConfig
    {
        uint32_t window_width;
        uint32_t window_height;

        float gravity;
        float density;

        int show_markers;
        int show_cubes;
        int show_sci;
        int show_air;
        int show_solids;
        int marching_cubes;
        int use_2D;

        int pressure_solver_steps;
        int particles_push_apart_steps;
        float flip_ratio;
        float over_relaxation;
        
        float spacing;
        float size_x, size_y, size_z;
        float particle_radius;
        int max_particles;

        float cam_x, cam_y, cam_z;
        float cam_rot_x, cam_rot_y, cam_rot_z;
        float cam_speed;

        float obstacle_radius;
        float obstacle_push_coefficient;
        float obstacle_speed;
    } CoreConfig_t;

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
