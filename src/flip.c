#include "flip.h"

void FLIP_init(FlipGrid_t *grid, float density, float size_x, float size_y, float size_z, float spacing, float particle_radius, int num_particles)
{
    grid->density = density;
    grid->f_num_x = (int)(floorf(size_x / spacing) + 1.0f);
    grid->f_num_y = (int)(floorf(size_y / spacing) + 1.0f);
    grid->f_num_z = (int)(floorf(size_z / spacing) + 1.0f);
    grid->h = maxf(size_x / grid->f_num_x, size_y / grid->f_num_y);
    grid->f_inv_spacing = 1.0f / grid->h;
    grid->f_num_cells = grid->f_num_x * grid->f_num_y * grid->f_num_z;

    grid->u = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->u, "u");
    grid->v = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->v, "v");
    grid->w = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->w, "w");
    grid->du = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->du, "du");
    grid->dv = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->dv, "dv");
    grid->dw = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->dw, "dw");
    grid->prev_u = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->prev_u, "prev_u");
    grid->prev_v = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->prev_v, "prev_v");
    grid->prev_w = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->prev_w, "prev_w");
    grid->p = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->p, "p");
    grid->s = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->s, "s");
    grid->cell_type = (CellType *)calloc(grid->f_num_cells, sizeof(CellType));
    ALLOC_CHECK(grid->cell_type, "cell_type");

    grid->num_particles = num_particles;
    grid->particle_pos = (float *)calloc(grid->num_particles * 3, sizeof(float));
    ALLOC_CHECK(grid->particle_pos, "particle_pos");
    grid->particle_vel = (float *)calloc(grid->num_particles * 3, sizeof(float));
    ALLOC_CHECK(grid->particle_vel, "particle_vel");
    grid->particle_density = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->particle_density, "particle_density");

    grid->particle_radius = particle_radius;
    grid->p_inv_spacing = 1.0f / (2.2f * grid->particle_radius);
    grid->p_num_x = (int)(floorf(size_x * grid->p_inv_spacing) + 1.0f);
    grid->p_num_y = (int)(floorf(size_y * grid->p_inv_spacing) + 1.0f);
    grid->p_num_z = (int)(floorf(size_z * grid->p_inv_spacing) + 1.0f);
    grid->p_num_cells = grid->p_num_x * grid->p_num_y * grid->p_num_z;

    grid->num_cell_particles = (int *)calloc(grid->p_num_cells, sizeof(int));
    ALLOC_CHECK(grid->num_cell_particles, "num_cell_particles");
    grid->first_cell_particle = (int *)calloc(grid->p_num_cells + 1, sizeof(int));
    ALLOC_CHECK(grid->first_cell_particle, "first_cell_particle");
    grid->cell_particle_ids = (int *)calloc(grid->num_particles, sizeof(int));
    ALLOC_CHECK(grid->cell_particle_ids, "cell_particle_ids");

    grid->num_particles = minf(grid->num_particles, grid->p_num_x * grid->p_num_y * grid->p_num_z);
    float min_x = grid->h + grid->particle_radius;
    float max_x = (grid->f_num_x - 1) * grid->h - grid->particle_radius;
    float min_y = grid->h + grid->particle_radius;
    float max_y = (grid->f_num_y - 1) * grid->h - grid->particle_radius;
    float min_z = grid->h + grid->particle_radius;
    float max_z = (grid->f_num_z - 1) * grid->h - grid->particle_radius;

    float x = min_x;
    float y = min_y;
    float z = min_z;

    for (int i = 0; i < grid->num_particles; i++)
    {
        x += grid->particle_radius * 2;
        if (x >= max_x)
        {
            x = min_x;
            y += grid->particle_radius * 2;
        }
        if (y >= max_y)
        {
            y = min_y;
            z += grid->particle_radius * 2;
        }

        if (z >= max_z)
        {
            break;
            grid->num_particles = i + 1;
        }

        grid->particle_pos[3 * i] = x;
        grid->particle_pos[3 * i + 1] = y;
        grid->particle_pos[3 * i + 2] = z;
    }

    for (int i = 0; i < grid->f_num_x; i++)
    {
        for (int j = 0; j < grid->f_num_y; j++)
        {
            for (int k = 0; k < grid->f_num_z; k++)
            {
                int cell_nr = k * grid->f_num_x * grid->f_num_y + j * grid->f_num_x + i;
                grid->s[cell_nr] = (i == 0 || i == grid->f_num_x - 1 || j == 0 || k == grid->f_num_z - 1 || k == 0) ? 0.0f : 1.0f;
            }
        }
    }
}

void FLIP_integrateParticles(FlipGrid_t *grid, float dt, float gravity)
{
    for (int i = 0; i < grid->num_particles; i++)
    {
        grid->particle_vel[3 * i + 1] += gravity * dt;
        grid->particle_pos[3 * i] += grid->particle_vel[3 * i] * dt;
        grid->particle_pos[3 * i + 1] += grid->particle_vel[3 * i + 1] * dt;
        grid->particle_pos[3 * i + 2] += grid->particle_vel[3 * i + 2] * dt;
    }
}

void FLIP_pushParticlesApart(FlipGrid_t *grid, int numIters, float dt)
{

    for (int i = 0; i < grid->p_num_cells; i++)
    {
        grid->num_cell_particles[i] = 0;
    }

    for (int i = 0; i < grid->num_particles; i++)
    {
        float x = grid->particle_pos[3 * i];
        float y = grid->particle_pos[3 * i + 1];
        float z = grid->particle_pos[3 * i + 2];

        int xi = clamp((int)floorf(x * grid->p_inv_spacing), 0, grid->p_num_x - 1);
        int yi = clamp((int)floorf(y * grid->p_inv_spacing), 0, grid->p_num_y - 1);
        int zi = clamp((int)floorf(z * grid->p_inv_spacing), 0, grid->p_num_z - 1);
        int cell_nr = zi * grid->p_num_x * grid->p_num_y + yi * grid->p_num_x + xi;
        grid->num_cell_particles[cell_nr]++;
    }

    int first = 0;
    for (int i = 0; i < grid->p_num_cells; i++)
    {
        first += grid->num_cell_particles[i];
        grid->first_cell_particle[i] = first;
    }
    grid->first_cell_particle[grid->p_num_cells] = first;

    for (int i = 0; i < grid->num_particles; i++)
    {
        float x = grid->particle_pos[3 * i];
        float y = grid->particle_pos[3 * i + 1];
        float z = grid->particle_pos[3 * i + 2];

        int xi = clamp((int)floorf(x * grid->p_inv_spacing), 0, grid->p_num_x - 1);
        int yi = clamp((int)floorf(y * grid->p_inv_spacing), 0, grid->p_num_y - 1);
        int zi = clamp((int)floorf(z * grid->p_inv_spacing), 0, grid->p_num_z - 1);
        int cell_nr = zi * grid->p_num_x * grid->p_num_y + yi * grid->p_num_x + xi;
        grid->first_cell_particle[cell_nr]--;
        grid->cell_particle_ids[grid->first_cell_particle[cell_nr]] = i;
    }

    float minDist = 2.07f * grid->particle_radius;
    float minDist2 = minDist * minDist;

    for (int iter = 0; iter < numIters; iter++)
    {
        for (int i = 0; i < grid->num_particles; i++)
        {
            float px = grid->particle_pos[3 * i];
            float py = grid->particle_pos[3 * i + 1];
            float pz = grid->particle_pos[3 * i + 2];

            int pxi = (int)floorf(px * grid->p_inv_spacing);
            int pyi = (int)floorf(py * grid->p_inv_spacing);
            int pzi = (int)floorf(pz * grid->p_inv_spacing);

            int x0 = max(pxi - 1, 0);
            int y0 = max(pyi - 1, 0);
            int z0 = max(pzi - 1, 0);

            int x1 = min(pxi + 1, grid->p_num_x - 1);
            int y1 = min(pyi + 1, grid->p_num_y - 1);
            int z1 = min(pzi + 1, grid->p_num_z - 1);

            for (int xi = x0; xi <= x1; xi++)
            {
                for (int yi = y0; yi <= y1; yi++)
                {
                    for (int zi = z0; zi <= z1; zi++)
                    {
                        int cell_nr = zi * grid->p_num_x * grid->p_num_y + yi * grid->p_num_x + xi;

                        int first = grid->first_cell_particle[cell_nr];
                        int last = grid->first_cell_particle[cell_nr + 1];

                        for (int j = first; j < last; j++)
                        {
                            int id = grid->cell_particle_ids[j];

                            if (id == i)
                                continue;

                            float qx = grid->particle_pos[3 * id];
                            float qy = grid->particle_pos[3 * id + 1];
                            float qz = grid->particle_pos[3 * id + 2];

                            float dx = qx - px;
                            float dy = qy - py;
                            float dz = qz - pz;

                            float d2 = dx * dx + dy * dy + dz * dz;
                            if (d2 > minDist2 || d2 < 1e-8f)
                                continue;

                            float d = sqrtf(d2);
                            if (d < 1e-5f)
                                d = 1e-5f;
                            float s = 0.5f * (minDist - d) / d;

                            dx *= s;
                            dy *= s;
                            dz *= s;

                            grid->particle_pos[3 * i] -= dx;
                            grid->particle_pos[3 * i + 1] -= dy;
                            grid->particle_pos[3 * i + 2] -= dz;

                            grid->particle_pos[3 * id] += dx;
                            grid->particle_pos[3 * id + 1] += dy;
                            grid->particle_pos[3 * id + 2] += dz;
                        }
                    }
                }
            }
        }
    }
}

void FLIP_handleObstacle(FlipGrid_t *grid, Obstacle_t *obstacle, float dt)
{
    float h = grid->h;
    float particle_r = grid->particle_radius;

    float min_x = h + particle_r;
    float max_x = (grid->f_num_x - 1) * h - particle_r;
    float min_y = h + particle_r;
    float max_y = (grid->f_num_y - 1) * h - particle_r;
    float min_z = h + particle_r;
    float max_z = (grid->f_num_z - 1) * h - particle_r;

    float obstacle_x = obstacle->x;
    float obstacle_y = obstacle->y;
    float obstacle_z = obstacle->z;

    float min_dist = particle_r + obstacle->radius;
    float min_dist_2 = min_dist * min_dist;

    for (int i = 0; i < grid->num_particles; i++)
    {
        float x = grid->particle_pos[3 * i];
        float y = grid->particle_pos[3 * i + 1];
        float z = grid->particle_pos[3 * i + 2];

        float dx = x - obstacle_x;
        float dy = y - obstacle_y;
        float dz = z - obstacle_z;

        float dist_2 = dx * dx + dy * dy + dz * dz;
        if (dist_2 < min_dist_2)
        {
            grid->particle_vel[3 * i] = Obstacle_getXVelocity(obstacle, dt) * obstacle->push_coefficient;
            grid->particle_vel[3 * i + 1] = Obstacle_getYVelocity(obstacle, dt) * obstacle->push_coefficient;
            grid->particle_vel[3 * i + 2] = Obstacle_getZVelocity(obstacle, dt) * obstacle->push_coefficient;
        }

        if (grid->particle_pos[3 * i] < min_x)
        {
            grid->particle_pos[3 * i] = min_x;
            grid->particle_vel[3 * i] = 0.0f;
        }
        if (grid->particle_pos[3 * i] > max_x)
        {
            grid->particle_pos[3 * i] = max_x;
            grid->particle_vel[3 * i] = 0.0f;
        }
        if (grid->particle_pos[3 * i + 1] < min_y)
        {
            grid->particle_pos[3 * i + 1] = min_y;
            grid->particle_vel[3 * i + 1] = 0.0f;
        }
        if (grid->particle_pos[3 * i + 1] > max_y)
        {
            grid->particle_pos[3 * i + 1] = max_y;
            grid->particle_vel[3 * i + 1] = 0.0f;
        }
        if (grid->particle_pos[3 * i + 2] < min_z)
        {
            grid->particle_pos[3 * i + 2] = min_z;
            grid->particle_vel[3 * i + 2] = 0.0f;
        }
        if (grid->particle_pos[3 * i + 2] > max_z)
        {
            grid->particle_pos[3 * i + 2] = max_z;
            grid->particle_vel[3 * i + 2] = 0.0f;
        }
    }
}

void FLIP_updateParticleDensity(FlipGrid_t *grid)
{
    float shift = 0.5f * grid->h;
    for (int i = 0; i < grid->f_num_cells; i++)
    {
        grid->particle_density[i] = 0.0f;
    }
    for (int i = 0; i < grid->num_particles; i++)
    {
        float x = grid->particle_pos[3 * i];
        float y = grid->particle_pos[3 * i + 1];
        float z = grid->particle_pos[3 * i + 2];

        x = clamp(x, grid->h, (grid->f_num_x - 1) * grid->h);
        y = clamp(y, grid->h, (grid->f_num_y - 1) * grid->h);
        z = clamp(z, grid->h, (grid->f_num_z - 1) * grid->h);

        int x0 = (int)floorf((x - shift) * grid->f_inv_spacing);
        int y0 = (int)floorf((y - shift) * grid->f_inv_spacing);
        int z0 = (int)floorf((z - shift) * grid->f_inv_spacing);

        int x1 = x0 + 1;
        int y1 = y0 + 1;
        int z1 = z0 + 1;

        float tx = (x - shift - x0 * grid->h) * grid->f_inv_spacing; // w0
        float ty = (y - shift - y0 * grid->h) * grid->f_inv_spacing; // w1
        float tz = (z - shift - z0 * grid->h) * grid->f_inv_spacing; // w2

        float sx = 1.0f - tx; // (1 - w0)
        float sy = 1.0f - ty; // (1 - w1)
        float sz = 1.0f - tz; // (1 - w2)

        int nr[8] = {
            z0 * grid->f_num_x * grid->f_num_y + y0 * grid->f_num_x + x0,
            z0 * grid->f_num_x * grid->f_num_y + y0 * grid->f_num_x + x1,
            z0 * grid->f_num_x * grid->f_num_y + y1 * grid->f_num_x + x0,
            z0 * grid->f_num_x * grid->f_num_y + y1 * grid->f_num_x + x1,
            z1 * grid->f_num_x * grid->f_num_y + y0 * grid->f_num_x + x0,
            z1 * grid->f_num_x * grid->f_num_y + y0 * grid->f_num_x + x1,
            z1 * grid->f_num_x * grid->f_num_y + y1 * grid->f_num_x + x0,
            z1 * grid->f_num_x * grid->f_num_y + y1 * grid->f_num_x + x1};

        float d0 = sx * sy * sz;
        float d1 = tx * sy * sz;
        float d2 = tx * ty * sz;
        float d3 = sx * ty * sz;
        float d4 = sx * sy * tz;
        float d5 = tx * sy * tz;
        float d6 = tx * ty * tz;
        float d7 = sx * ty * tz;

        float d_p[8] = {d0, d1, d2, d3, d4, d5, d6, d7};

        for (int j = 0; j < 8; j++)
        {
            grid->particle_density[nr[j]] += d_p[j];
        }
    }

    if (grid->particle_rest_density == 0.0f)
    {
        float sum = 0.0f;
        int num_fluid_cells = 0;
        for (int i = 0; i < grid->f_num_cells; i++)
        {
            if (grid->cell_type[i] == FLUID)
            {
                sum += grid->particle_density[i];
                num_fluid_cells++;
            }
        }
        if (num_fluid_cells > 0)
        {
            grid->particle_rest_density = sum / num_fluid_cells;
        }
    }
}

void FLIP_transferVelocities(FlipGrid_t *grid, int toGrid, float FLIPRatio)
{
    if (toGrid)
    {
        for (int i = 0; i < grid->f_num_cells; i++)
        {
            grid->prev_u[i] = grid->u[i];
            grid->prev_v[i] = grid->v[i];
            grid->prev_w[i] = grid->w[i];
            grid->du[i] = 0.0f;
            grid->dv[i] = 0.0f;
            grid->dw[i] = 0.0f;
            grid->u[i] = 0.0f;
            grid->v[i] = 0.0f;
            grid->w[i] = 0.0f;

            grid->cell_type[i] = grid->s[i] == 0.0f ? SOLID : AIR;
        }

        for (int i = 0; i < grid->num_particles; i++)
        {
            float x = grid->particle_pos[3 * i];
            float y = grid->particle_pos[3 * i + 1];
            float z = grid->particle_pos[3 * i + 2];

            int xi = clamp((int)floorf(x * grid->f_inv_spacing), 0, grid->f_num_x - 1);
            int yi = clamp((int)floorf(y * grid->f_inv_spacing), 0, grid->f_num_y - 1);
            int zi = clamp((int)floorf(z * grid->f_inv_spacing), 0, grid->f_num_z - 1);
            int cell_nr = zi * grid->f_num_x * grid->f_num_y + yi * grid->f_num_x + xi;
            if (grid->cell_type[cell_nr] == AIR)
            {
                grid->cell_type[cell_nr] = FLUID;
            }
        }
    }

    for (int component = 0; component < 3; component++)
    {
        float *f = component == 0 ? grid->u : (component == 1 ? grid->v : grid->w);
        float *d = component == 0 ? grid->du : (component == 1 ? grid->dv : grid->dw);
        float *prev_f = component == 0 ? grid->prev_u : (component == 1 ? grid->prev_v : grid->prev_w);

        float shift_x = component == 0 ? 0.0f : 0.5f * grid->h;
        float shift_y = component == 1 ? 0.0f : 0.5f * grid->h;
        float shift_z = component == 2 ? 0.0f : 0.5f * grid->h;

        for (int i = 0; i < grid->num_particles; i++)
        {
            float x = clampf(grid->particle_pos[3 * i], grid->h, (grid->f_num_x - 1) * grid->h);
            float y = clampf(grid->particle_pos[3 * i + 1], grid->h, (grid->f_num_y - 1) * grid->h);
            float z = clampf(grid->particle_pos[3 * i + 2], grid->h, (grid->f_num_z - 1) * grid->h);

            int x0 = clamp((int)floorf((x - shift_x) * grid->f_inv_spacing), 0, grid->f_num_x - 1);
            int y0 = clamp((int)floorf((y - shift_y) * grid->f_inv_spacing), 0, grid->f_num_y - 1);
            int z0 = clamp((int)floorf((z - shift_z) * grid->f_inv_spacing), 0, grid->f_num_z - 1);

            int x1 = min(x0 + 1, grid->f_num_x - 1);
            int y1 = min(y0 + 1, grid->f_num_y - 1);
            int z1 = min(z0 + 1, grid->f_num_z - 1);

            float tx = (x - shift_x - x0 * grid->h) * grid->f_inv_spacing; // w0
            float ty = (y - shift_y - y0 * grid->h) * grid->f_inv_spacing; // w1
            float tz = (z - shift_z - z0 * grid->h) * grid->f_inv_spacing; // w2

            float sx = 1.0f - tx; // (1 - w0)
            float sy = 1.0f - ty; // (1 - w1)
            float sz = 1.0f - tz; // (1 - w2)

            int nr[8] = {
                z0 * grid->f_num_x * grid->f_num_y + y0 * grid->f_num_x + x0,
                z0 * grid->f_num_x * grid->f_num_y + y0 * grid->f_num_x + x1,
                z0 * grid->f_num_x * grid->f_num_y + y1 * grid->f_num_x + x0,
                z0 * grid->f_num_x * grid->f_num_y + y1 * grid->f_num_x + x1,
                z1 * grid->f_num_x * grid->f_num_y + y0 * grid->f_num_x + x0,
                z1 * grid->f_num_x * grid->f_num_y + y0 * grid->f_num_x + x1,
                z1 * grid->f_num_x * grid->f_num_y + y1 * grid->f_num_x + x0,
                z1 * grid->f_num_x * grid->f_num_y + y1 * grid->f_num_x + x1};

            float d0 = sx * sy * sz;
            float d1 = tx * sy * sz;
            float d2 = tx * ty * sz;
            float d3 = sx * ty * sz;
            float d4 = sx * sy * tz;
            float d5 = tx * sy * tz;
            float d6 = tx * ty * tz;
            float d7 = sx * ty * tz;

            float d_p[8] = {d0, d1, d2, d3, d4, d5, d6, d7};

            if (toGrid)
            {
                float particle_vel = grid->particle_vel[3 * i + component];
                for (int j = 0; j < 8; j++)
                {
                    f[nr[j]] += particle_vel * d_p[j];
                    d[nr[j]] += d_p[j];
                }
            }
            else
            {
                float valid[8];
                for (int j = 0; j < 8; j++)
                {
                    valid[j] = grid->cell_type[nr[j]] != AIR ? 1.0f : 0.0f;
                }

                float velocity = grid->particle_vel[3 * i + component];
                float d_v = 0.0f;
                for (int j = 0; j < 8; j++)
                {
                    d_v += valid[j];
                }

                if (d_v > 0.0f)
                {
                    float pic_vel = 0.0f;
                    for (int j = 0; j < 8; j++)
                    {
                        pic_vel += valid[j] * f[nr[j]];
                    }
                    pic_vel /= d_v;

                    float corr = 0.0f;
                    for (int j = 0; j < 8; j++)
                    {
                        corr += valid[j] * (f[nr[j]] - prev_f[nr[j]]);
                    }
                    corr /= d_v;

                    grid->particle_vel[3 * i + component] = FLIPRatio * pic_vel + (1.0f - FLIPRatio) * (velocity + corr);
                }
            }
        }

        if (toGrid)
        {
            for (int i = 0; i < grid->f_num_cells; i++)
            {
                if (grid->cell_type[i] == FLUID)
                {
                    f[i] /= d[i];
                }
            }

            for (int i = 0; i < grid->f_num_x; i++)
            {
                for (int j = 0; j < grid->f_num_y; j++)
                {
                    for (int k = 0; k < grid->f_num_z; k++)
                    {
                        int solid = grid->cell_type[k * grid->f_num_x * grid->f_num_y + j * grid->f_num_x + i] == SOLID ? 1 : 0;
                        if (solid || (i > 0 && grid->cell_type[k * grid->f_num_x * grid->f_num_y + j * grid->f_num_x + i - 1] == SOLID))
                        {
                            f[k * grid->f_num_x * grid->f_num_y + j * grid->f_num_x + i] = grid->prev_u[k * grid->f_num_x * grid->f_num_y + j * grid->f_num_x + i];
                        }
                        if (solid || (j > 0 && grid->cell_type[k * grid->f_num_x * grid->f_num_y + (j - 1) * grid->f_num_x + i] == SOLID))
                        {
                            f[k * grid->f_num_x * grid->f_num_y + j * grid->f_num_x + i] = grid->prev_v[k * grid->f_num_x * grid->f_num_y + j * grid->f_num_x + i];
                        }
                        if (solid || (k > 0 && grid->cell_type[(k - 1) * grid->f_num_x * grid->f_num_y + j * grid->f_num_x + i] == SOLID))
                        {
                            f[k * grid->f_num_x * grid->f_num_y + j * grid->f_num_x + i] = grid->prev_w[k * grid->f_num_x * grid->f_num_y + j * grid->f_num_x + i];
                        }
                    }
                }
            }
        }
    }
}

void FLIP_solveIncompressibility(FlipGrid_t *grid, int num_iters, float dt, float over_relaxation)
{
    for (int i = 0; i < grid->f_num_cells; i++)
    {
        grid->p[i] = 0.0f;
        grid->prev_u[i] = grid->u[i];
        grid->prev_v[i] = grid->v[i];
        grid->prev_w[i] = grid->w[i];
    }

    double cp = grid->density * grid->h / dt;

    for (int iter = 0; iter < num_iters; iter++)
    {
        for (int i = 1; i < grid->f_num_x - 1; i++)
        {
            for (int j = 1; j < grid->f_num_y - 1; j++)
            {
                for (int k = 1; k < grid->f_num_z - 1; k++)
                {
                    int center = k * grid->f_num_x * grid->f_num_y + j * grid->f_num_x + i;
                    if (grid->cell_type[center] != FLUID)
                    {
                        continue;
                    }

                    int left = k * grid->f_num_x * grid->f_num_y + j * grid->f_num_x + (i - 1);
                    int right = k * grid->f_num_x * grid->f_num_y + j * grid->f_num_x + (i + 1);
                    int bottom = k * grid->f_num_x * grid->f_num_y + (j - 1) * grid->f_num_x + i;
                    int top = k * grid->f_num_x * grid->f_num_y + (j + 1) * grid->f_num_x + i;
                    int back = (k - 1) * grid->f_num_x * grid->f_num_y + j * grid->f_num_x + i;
                    int front = (k + 1) * grid->f_num_x * grid->f_num_y + j * grid->f_num_x + i;

                    float sx0 = grid->s[left];
                    float sx1 = grid->s[right];
                    float sy0 = grid->s[bottom];
                    float sy1 = grid->s[top];
                    float sz0 = grid->s[back];
                    float sz1 = grid->s[front];
                    float s = sx0 + sx1 + sy0 + sy1 + sz0 + sz1;

                    if (s == 0.0f)
                        continue;

                    float div = grid->u[right] - grid->u[center] +
                                grid->v[top] - grid->v[center] +
                                grid->w[front] - grid->w[center];

                    if (grid->particle_rest_density > 0.0f)
                    {
                        float k = 1.0f;
                        float compression = grid->particle_density[center] - grid->particle_rest_density;
                        if (compression > 0.0f)
                        {
                            div = div - k * compression;
                        }
                    }

                    float p = -(div / s);
                    p *= over_relaxation;
                    grid->p[center] += cp * p;

                    grid->u[center] -= sx0 * p;
                    grid->u[right] += sx1 * p;
                    grid->v[center] -= sy0 * p;
                    grid->v[top] += sy1 * p;
                    grid->w[center] -= sz0 * p;
                    grid->w[front] += sz1 * p;
                }
            }
        }
    }
}

Pair_t FLIP_transformGridToVerticies(FlipGrid_t *grid, Vertex_t *vertices, GLuint *indices,
                                     int show_sci, int show_air, int show_solids)
{
    static glm::vec3 cubeVertices[8] = {
        {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, 
        {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 
        {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, 
        {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}};

    static GLuint cubeIndices[36] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        4, 0, 3, 3, 7, 4,
        1, 5, 6, 6, 2, 1,
        3, 2, 6, 6, 7, 3,
        4, 5, 1, 1, 0, 4};

    size_t vert_index = 0;
    size_t ind_index = 0;

    float minPressure = FLT_MAX, maxPressure = FLT_MIN, sumPressure = 0.0f;
    for (int i = 0; i < grid->f_num_cells; i++)
    {
        if (grid->cell_type[i] == FLUID)
        {
            float p = grid->p[i];
            minPressure = fminf(minPressure, p);
            maxPressure = fmaxf(maxPressure, p);
            sumPressure += p;
        }
    }

    for (int cell_nr = 0; cell_nr < grid->f_num_cells; cell_nr++)
    {

        glm::vec3 cubePos = glm::vec3(cell_nr % grid->f_num_x,
                                      (cell_nr / grid->f_num_x) % grid->f_num_y,
                                      cell_nr / (grid->f_num_x * grid->f_num_y)) *
                            grid->h;

        float c[3] = {0.0f, 0.0f, 0.0f};
        switch (grid->cell_type[cell_nr])
        {
        case FLUID:
            if (show_sci)
                getSciColor(grid->p[cell_nr], minPressure, maxPressure, c);
            else
            {
                c[0] = 0.113f;
                c[1] = 0.353f;
                c[2] = 0.403f;
            }
            break;
        case SOLID:
            if (!show_solids)
                continue;
            c[0] = c[1] = c[2] = 1.0f;
            break;
        case AIR:
            if (!show_air)
                continue;
            c[0] = 0.53f;
            c[1] = 0.81f;
            c[2] = 0.94f;
            break;
        }

        for (int i = 0; i < 8; i++)
        {
            vertices[vert_index].position = cubePos + cubeVertices[i] * grid->h;
            vertices[vert_index].color.x = c[0];
            vertices[vert_index].color.y = c[1];
            vertices[vert_index].color.z = c[2];
            vertices[vert_index].normal = glm::normalize(cubeVertices[i]);
            vert_index++;
        }

        size_t offset = vert_index - 8;
        for (int i = 0; i < 36; i++)
        {
            indices[ind_index++] = offset + cubeIndices[i];
        }
    }

    return {.first = (int)vert_index, .second = (int)ind_index};
}

Pair_t FLIP_transformGridToVerticiesMarchingCubes(
    FlipGrid_t *grid, Vertex_t *vertices, GLuint *indices,
    int show_sci, int show_air, int show_solids)
{
    size_t vert_index = 0;
    size_t ind_index = 0;

    float minPressure = FLT_MAX, maxPressure = FLT_MIN;

    glm::vec3 vertexList[12];
    float cornerValues[8];

    for (int z = 0; z < grid->f_num_z; z++)
    {
        for (int y = 0; y < grid->f_num_y; y++)
        {
            for (int x = 0; x < grid->f_num_x; x++)
            {
                int cellNr = z * grid->f_num_x * grid->f_num_y + y * grid->f_num_x + x;

                glm::vec3 cubePos = glm::vec3(x, y, z) * grid->h;

                int cubeIndex = 0;

                // Corner values & Cube Index Calculation
                for (int i = 0; i < 8; i++)
                {
                    int cornerCell = cellNr +
                                     (int)cornerOffsets[i].x +
                                     (int)cornerOffsets[i].y * grid->f_num_x +
                                     (int)cornerOffsets[i].z * grid->f_num_x * grid->f_num_y;

                    cornerValues[i] = grid->p[cornerCell];

                    if (cornerValues[i] <= 0.0f)
                        cubeIndex |= (1 << i);

                    // Aktualizacja min/max ciśnienia
                    if (grid->cell_type[cornerCell] == FLUID)
                    {
                        minPressure = std::min(minPressure, cornerValues[i]);
                        maxPressure = std::max(maxPressure, cornerValues[i]);
                    }
                }

                // Ominięcie zbędnego renderowania
                if (cubeIndex == 0 || cubeIndex == 255)
                    continue;

                // Vertex Interpolation
                for (int i = 0; i < 12; i++)
                {
                    if (edgeTable[cubeIndex] & (1 << i))
                    {
                        int idxA = cornerIndexAFromEdge[i];
                        int idxB = cornerIndexBFromEdge[i];

                        float valA = cornerValues[idxA];
                        float valB = cornerValues[idxB];

                        float t = glm::clamp((0.0f - valA) / (valB - valA), 0.0f, 1.0f);

                        vertexList[i] = glm::mix(
                            cubePos + cornerOffsets[idxA] * grid->h,
                            cubePos + cornerOffsets[idxB] * grid->h,
                            t);
                    }
                }

                // Triangle Generation
                for (int i = 0; triTable[cubeIndex][i] != -1; i += 3)
                {
                    glm::vec3 v0 = vertexList[triTable[cubeIndex][i]];
                    glm::vec3 v1 = vertexList[triTable[cubeIndex][i + 1]];
                    glm::vec3 v2 = vertexList[triTable[cubeIndex][i + 2]];

                    glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

                    for (int j = 0; j < 3; j++)
                    {
                        int vertID = triTable[cubeIndex][i + j];
                        float color[3] = {0.113f, 0.353f, 0.403f};

                        if (show_sci)
                        {
                            getSciColor(cornerValues[vertID], minPressure, maxPressure, color);
                        }

                        vertices[vert_index].position = vertexList[vertID];
                        vertices[vert_index].color = glm::vec3(color[0], color[1], color[2]);
                        vertices[vert_index].normal = normal;

                        indices[ind_index++] = vert_index++;
                    }
                }
            }
        }
    }

    return {.first = (int)vert_index, .second = (int)ind_index};
}

Pair_t FLIP_transformMarkersToVertices(FlipGrid_t *grid, Vertex_t *markerVertices, GLuint *markerIndices)
{
    int sphere_lat_slices = 3;
    int sphere_lon_slices = 3;

    int indexOffset = 0;
    int vertexOffset = 0;

    float min_velocity = FLT_MAX;
    float max_velocity = FLT_MIN;
    float sum_velocity = 0.0f;
    for (int i = 0; i < grid->f_num_cells; i++)
    {
        if (grid->cell_type[i] == FLUID)
        {
            float vel = grid->particle_vel[3 * i] * grid->particle_vel[3 * i] +
                        grid->particle_vel[3 * i + 1] * grid->particle_vel[3 * i + 1] +
                        grid->particle_vel[3 * i + 2] * grid->particle_vel[3 * i + 2];
            vel = sqrtf(vel);
            if (vel < min_velocity)
                min_velocity = vel;
            if (vel > max_velocity)
                max_velocity = vel;
            sum_velocity += vel;
        }
    }

    for (int m = 0; m < grid->num_particles; m++)
    {
        glm::vec3 markerPos = glm::vec3(grid->particle_pos[3 * m], grid->particle_pos[3 * m + 1], grid->particle_pos[3 * m + 2]);
        if (markerPos.x < grid->h + grid->particle_radius * 2 || markerPos.x > (grid->f_num_x - 1) * grid->h - grid->particle_radius * 2 ||
            markerPos.y < grid->h + grid->particle_radius * 2 || markerPos.y > (grid->f_num_y - 1) * grid->h - grid->particle_radius * 2 ||
            markerPos.z < grid->h + grid->particle_radius * 2 || markerPos.z > (grid->f_num_z - 1) * grid->h - grid->particle_radius * 2)
        {
            continue;
        }
        float c[3] = {0.0f, 0.0f, 1.0f};
        getSciColor(glm::length(glm::vec3(grid->particle_vel[3 * m], grid->particle_vel[3 * m + 1], grid->particle_vel[3 * m + 2])), min_velocity, max_velocity, c);
        glm::vec3 markerColor = glm::vec3(c[0], c[1], c[2]);
        float radius = grid->particle_radius;
        for (int i = 0; i <= sphere_lat_slices; i++)
        {
            float theta = (float)i / sphere_lat_slices * M_PI;
            float sinTheta = sinf(theta);
            float cosTheta = cosf(theta);

            for (int j = 0; j <= sphere_lon_slices; j++)
            {
                float phi = (float)j / sphere_lon_slices * 2.0f * M_PI;
                float sinPhi = sinf(phi);
                float cosPhi = cosf(phi);

                glm::vec3 vertexPos;

                vertexPos.x = markerPos.x + radius * sinTheta * cosPhi;
                vertexPos.y = markerPos.y + radius * cosTheta;
                vertexPos.z = markerPos.z + radius * sinTheta * sinPhi;

                markerVertices[vertexOffset]
                    .position = vertexPos;
                markerVertices[vertexOffset].color = markerColor;
                vertexOffset++;
            }
        }

        for (int i = 0; i < sphere_lat_slices; i++)
        {
            for (int j = 0; j < sphere_lon_slices; j++)
            {
                int first = vertexOffset - (sphere_lat_slices + 1) * (sphere_lon_slices + 1) + i * (sphere_lon_slices + 1) + j;
                int second = first + sphere_lon_slices + 1;

                markerIndices[indexOffset++] = first;
                markerIndices[indexOffset++] = second;
                markerIndices[indexOffset++] = first + 1;

                markerIndices[indexOffset++] = second;
                markerIndices[indexOffset++] = second + 1;
                markerIndices[indexOffset++] = first + 1;
            }
        }
    }

    return (Pair_t){.first = vertexOffset, .second = indexOffset};
}

void FLIP_destroy(FlipGrid_t *grid)
{
    // free(grid->u);
    // free(grid->v);
    // free(grid->w);
    // free(grid->du);
    // free(grid->dv);
    // free(grid->dw);
    // free(grid->prev_u);
    // free(grid->prev_v);
    // free(grid->prev_w);
    // free(grid->p);
    // free(grid->s);
    // free(grid->cell_type);
    // free(grid->particle_pos);
    // free(grid->particle_vel);
    // free(grid->particle_density);
    // free(grid->num_cell_particles);
    // free(grid->first_cell_particle);
    // free(grid->cell_particle_ids);
}
