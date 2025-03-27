#include "flip_2D.h"

void FLIP2D_init(FlipGrid_t *grid, float density, float size_x, float size_y, float spacing, float particle_radius, int num_particles)
{
    grid->density = density;
    grid->f_num_x = (int)(floorf(size_x / spacing) + 1.0f);
    grid->f_num_y = (int)(floorf(size_y / spacing) + 1.0f);
    grid->f_num_z = 1;
    grid->h = maxf(size_x / grid->f_num_x, size_y / grid->f_num_y);
    grid->f_inv_spacing = 1.0f / grid->h;
    grid->f_num_cells = grid->f_num_x * grid->f_num_y;

    grid->u = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->u, "u");
    grid->v = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->v, "v");
    grid->du = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->du, "du");
    grid->dv = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->dv, "dv");
    grid->prev_u = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->prev_u, "prev_u");
    grid->prev_v = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->prev_v, "prev_v");
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
    grid->p_num_z = 1;
    grid->p_num_cells = grid->p_num_x * grid->p_num_y;

    grid->num_cell_particles = (int *)calloc(grid->p_num_cells, sizeof(int));
    ALLOC_CHECK(grid->num_cell_particles, "num_cell_particles");
    grid->first_cell_particle = (int *)calloc(grid->p_num_cells + 1, sizeof(int));
    ALLOC_CHECK(grid->first_cell_particle, "first_cell_particle");
    grid->cell_particle_ids = (int *)calloc(grid->num_particles, sizeof(int));
    ALLOC_CHECK(grid->cell_particle_ids, "cell_particle_ids");

    grid->num_particles = minf(grid->num_particles, grid->p_num_x * grid->p_num_y);
    int particles_per_cell = (int)ceilf((float)grid->num_particles / (float)((grid->f_num_x - 2) * (grid->f_num_y - 1)));
    int particle_index = 0;
    float particle_spacing_inside_cell = grid->h / 10.0f;
    for (int j = 1; j < grid->f_num_y - 1; j++)
    {
        for (int i = 1; i < grid->f_num_x - 1; i++)
        {
            float start_x = i * grid->h + grid->particle_radius;
            float start_y = j * grid->h + grid->particle_radius;
            int x_index = 0;
            int y_index = 0;
            float x = start_x;
            float y = start_y;
            for (int k = 0; k < particles_per_cell; k++)
            {
                if (x < start_x + grid->h)
                {
                    x += ((grid->particle_radius * 2) + particle_spacing_inside_cell) * x_index;
                    x_index++;
                }
                else
                {
                    x = start_x;
                    y += ((grid->particle_radius * 2) + particle_spacing_inside_cell) * y_index;
                    y_index++;
                }
                grid->particle_pos[3 * particle_index] = x;
                grid->particle_pos[3 * particle_index + 1] = y;
                grid->particle_pos[3 * particle_index + 2] = grid->h;
                particle_index++;
            }
        }
    }

    for (int i = 0; i < grid->f_num_x; i++)
    {
        for (int j = 0; j < grid->f_num_y; j++)
        {
            grid->s[j * grid->f_num_x + i] = (i == 0 || i == grid->f_num_x - 1 || j == 0) ? 0.0f : 1.0f;
        }
    }
}

void FLIP2D_integrateParticles(FlipGrid_t *grid, float dt, float gravity)
{
    for (int i = 0; i < grid->num_particles; i++)
    {
        grid->particle_vel[3 * i + 1] += gravity * dt;
        grid->particle_pos[3 * i] += grid->particle_vel[3 * i] * dt;
        grid->particle_pos[3 * i + 1] += grid->particle_vel[3 * i + 1] * dt;
    }
}

void FLIP2D_pushParticlesApart(FlipGrid_t *grid, int numIters, float dt)
{

    for (int i = 0; i < grid->p_num_cells; i++)
    {
        grid->num_cell_particles[i] = 0;
    }

    for (int i = 0; i < grid->num_particles; i++)
    {
        float x = grid->particle_pos[3 * i];
        float y = grid->particle_pos[3 * i + 1];

        int xi = clamp((int)floorf(x * grid->p_inv_spacing), 0, grid->p_num_x - 1);
        int yi = clamp((int)floorf(y * grid->p_inv_spacing), 0, grid->p_num_y - 1);
        int cell_nr = yi * grid->p_num_x + xi;
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

        int xi = clamp((int)floorf(x * grid->p_inv_spacing), 0, grid->p_num_x - 1);
        int yi = clamp((int)floorf(y * grid->p_inv_spacing), 0, grid->p_num_y - 1);
        int cell_nr = yi * grid->p_num_x + xi;
        grid->first_cell_particle[cell_nr]--;
        grid->cell_particle_ids[grid->first_cell_particle[cell_nr]] = i;
    }

    float minDist = 2.0f * grid->particle_radius;
    float minDist2 = minDist * minDist;

    for (int iter = 0; iter < numIters; iter++)
    {
        for (int i = 0; i < grid->num_particles; i++)
        {
            float px = grid->particle_pos[3 * i];
            float py = grid->particle_pos[3 * i + 1];

            int pxi = (int)floorf(px * grid->p_inv_spacing);
            int pyi = (int)floorf(py * grid->p_inv_spacing);

            int x0 = max(pxi - 1, 0);
            int y0 = max(pyi - 1, 0);
            int x1 = min(pxi + 1, grid->p_num_x - 1);
            int y1 = min(pyi + 1, grid->p_num_y - 1);

            for (int xi = x0; xi <= x1; xi++)
            {
                for (int yi = y0; yi <= y1; yi++)
                {
                    int cell_nr = yi * grid->p_num_x + xi;

                    int first = grid->first_cell_particle[cell_nr];
                    int last = grid->first_cell_particle[cell_nr + 1];

                    for (int j = first; j < last; j++)
                    {
                        int id = grid->cell_particle_ids[j];

                        if (id == i)
                            continue;

                        float qx = grid->particle_pos[3 * id];
                        float qy = grid->particle_pos[3 * id + 1];

                        float dx = qx - px;
                        float dy = qy - py;
                        float d2 = dx * dx + dy * dy;
                        if (d2 > minDist2 || d2 == 0.0f)
                            continue;

                        float d = sqrtf(d2);
                        float s = 0.5f * (minDist - d) / d;

                        dx *= s;
                        dy *= s;

                        grid->particle_pos[3 * i] -= dx;
                        grid->particle_pos[3 * i + 1] -= dy;
                        grid->particle_pos[3 * id] += dx;
                        grid->particle_pos[3 * id + 1] += dy;
                    }
                }
            }
        }
    }
}

void FLIP2D_handleObstacle(FlipGrid_t *grid, Obstacle_t *obstacle, float dt)
{
    float h = grid->h;
    float particle_r = grid->particle_radius;

    float min_x = h + particle_r;
    float max_x = (grid->f_num_x - 1) * h - particle_r;
    float min_y = h + particle_r;
    float max_y = (grid->f_num_y - 1) * h - particle_r;

    float obstacle_x = obstacle->x;
    float obstacle_y = obstacle->y;

    float min_dist = particle_r + obstacle->radius;
    float min_dist_2 = min_dist * min_dist;

    for (int i = 0; i < grid->num_particles; i++)
    {
        float x = grid->particle_pos[3 * i];
        float y = grid->particle_pos[3 * i + 1];

        float dx = x - obstacle_x;
        float dy = y - obstacle_y;

        float dist_2 = dx * dx + dy * dy;
        if (dist_2 < min_dist_2)
        {
            grid->particle_vel[3 * i] = Obstacle_getXVelocity(obstacle, dt) * obstacle->push_coefficient;
            grid->particle_vel[3 * i + 1] = Obstacle_getYVelocity(obstacle, dt) * obstacle->push_coefficient;
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
    }
}

void FLIP2D_updateParticleDensity(FlipGrid_t *grid)
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

        x = clamp(x, grid->h, (grid->f_num_x - 1) * grid->h);
        y = clamp(y, grid->h, (grid->f_num_y - 1) * grid->h);

        int x0 = (int)floorf((x - shift) * grid->f_inv_spacing);
        int y0 = (int)floorf((y - shift) * grid->f_inv_spacing);

        int x1 = x0 + 1;
        int y1 = y0 + 1;

        float tx = (x - shift - x0 * grid->h) * grid->f_inv_spacing; // w0
        float ty = (y - shift - y0 * grid->h) * grid->f_inv_spacing; // w1

        float sx = 1.0f - tx; // (1 - w0)
        float sy = 1.0f - ty; // (1 - w1)

        float d0 = sx * sy; // (1-w0)(1-w1)
        float d1 = tx * sy; // w0 * (1 - w1)
        float d2 = tx * ty; // w0 * w1
        float d3 = sx * ty; // (1 - w0) * w1

        int nr0 = y0 * grid->f_num_x + x0;
        int nr1 = y0 * grid->f_num_x + x1;
        int nr2 = y1 * grid->f_num_x + x1;
        int nr3 = y1 * grid->f_num_x + x0;

        grid->particle_density[nr0] += d0;
        grid->particle_density[nr1] += d1;
        grid->particle_density[nr2] += d2;
        grid->particle_density[nr3] += d3;
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

void FLIP2D_transferVelocities(FlipGrid_t *grid, int toGrid, float FLIP2DRatio)
{

    if (toGrid)
    {
        for (int i = 0; i < grid->f_num_cells; i++)
        {
            grid->prev_u[i] = grid->u[i];
            grid->prev_v[i] = grid->v[i];
            grid->du[i] = 0.0f;
            grid->dv[i] = 0.0f;
            grid->u[i] = 0.0f;
            grid->v[i] = 0.0f;

            grid->cell_type[i] = grid->s[i] == 0.0f ? SOLID : AIR;
        }

        for (int i = 0; i < grid->num_particles; i++)
        {
            float x = grid->particle_pos[3 * i];
            float y = grid->particle_pos[3 * i + 1];

            int xi = clamp((int)floorf(x * grid->f_inv_spacing), 0, grid->f_num_x - 1);
            int yi = clamp((int)floorf(y * grid->f_inv_spacing), 0, grid->f_num_y - 1);
            int cell_nr = yi * grid->f_num_x + xi;
            if (grid->cell_type[cell_nr] == AIR)
            {
                grid->cell_type[cell_nr] = FLUID;
            }
        }
    }

    for (int component = 0; component < 2; component++)
    {
        float *f = component == 0 ? grid->u : grid->v;
        float *d = component == 0 ? grid->du : grid->dv;
        float *prev_f = component == 0 ? grid->prev_u : grid->prev_v;

        float shift_x = component == 0 ? 0.0f : 0.5f * grid->h;
        float shift_y = component == 0 ? 0.5f * grid->h : 0.0f;

        for (int i = 0; i < grid->num_particles; i++)
        {
            float x = clampf(grid->particle_pos[3 * i], grid->h, (grid->f_num_x - 1) * grid->h);
            float y = clampf(grid->particle_pos[3 * i + 1], grid->h, (grid->f_num_y - 1) * grid->h);

            int x0 = clamp((int)floorf((x - shift_x) * grid->f_inv_spacing), 0, grid->f_num_x - 1);
            int y0 = clamp((int)floorf((y - shift_y) * grid->f_inv_spacing), 0, grid->f_num_y - 1);

            // int x0 = (int)floorf((x - shift_x) * grid->f_inv_spacing);
            // int y0 = (int)floorf((y - shift_y) * grid->f_inv_spacing);

            int x1 = min(x0 + 1, grid->f_num_x - 1);
            int y1 = min(y0 + 1, grid->f_num_y - 1);

            float tx = (x - shift_x - x0 * grid->h) * grid->f_inv_spacing; // w0
            float ty = (y - shift_y - y0 * grid->h) * grid->f_inv_spacing; // w1

            float sx = 1.0f - tx; // (1 - w0)
            float sy = 1.0f - ty; // (1 - w1)

            float d0 = sx * sy; // (1-w0)(1-w1)
            float d1 = tx * sy; // w0 * (1 - w1)
            float d2 = tx * ty; // w0 * w1
            float d3 = sx * ty; // (1 - w0) * w1

            int nr0 = y0 * grid->f_num_x + x0;
            int nr1 = y0 * grid->f_num_x + x1;
            int nr2 = y1 * grid->f_num_x + x1;
            int nr3 = y1 * grid->f_num_x + x0;

            if (toGrid)
            {
                float particle_vel = grid->particle_vel[3 * i + component];
                f[nr0] += d0 * particle_vel;
                f[nr1] += d1 * particle_vel;
                f[nr2] += d2 * particle_vel;
                f[nr3] += d3 * particle_vel;

                d[nr0] += d0;
                d[nr1] += d1;
                d[nr2] += d2;
                d[nr3] += d3;
            }
            else
            {
                int offset = component == 0 ? 0 : grid->f_num_cells;

                float valid0 = grid->cell_type[nr0] != AIR || grid->cell_type[nr0 + offset] != AIR ? 1.0f : 0.0f;
                float valid1 = grid->cell_type[nr1] != AIR || grid->cell_type[nr1 + offset] != AIR ? 1.0f : 0.0f;
                float valid2 = grid->cell_type[nr2] != AIR || grid->cell_type[nr2 + offset] != AIR ? 1.0f : 0.0f;
                float valid3 = grid->cell_type[nr3] != AIR || grid->cell_type[nr3 + offset] != AIR ? 1.0f : 0.0f;
                float velocity = grid->particle_vel[3 * i + component];

                float d_v = valid0 * d0 + valid1 * d1 + valid2 * d2 + valid3 * d3;
                if (d_v > 0.0f)
                {
                    float pic_vel = (valid0 * d0 * f[nr0] + valid1 * d1 * f[nr1] + valid2 * d2 * f[nr2] + valid3 * d3 * f[nr3]) / d_v;
                    float corr = (valid0 * d0 * (f[nr0] - prev_f[nr0]) + valid1 * d1 * (f[nr1] - prev_f[nr1]) + valid2 * d2 * (f[nr2] - prev_f[nr2]) + valid3 * d3 * (f[nr3] - prev_f[nr3])) / d_v;
                    grid->particle_vel[3 * i + component] = FLIP2DRatio * pic_vel + (1.0f - FLIP2DRatio) * (velocity + corr);
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
                    int solid = grid->cell_type[j * grid->f_num_x + i] == SOLID ? 1 : 0;
                    if (solid || (i > 0 && grid->cell_type[j * grid->f_num_x + i - 1] == SOLID))
                    {
                        f[j * grid->f_num_x + i] = grid->prev_u[j * grid->f_num_x + i];
                    }
                    if (solid || (j > 0 && grid->cell_type[(j - 1) * grid->f_num_x + i] == SOLID))
                    {
                        f[j * grid->f_num_x + i] = grid->prev_v[j * grid->f_num_x + i];
                    }
                }
            }
        }
    }
}

void FLIP2D_solveIncompressibility(FlipGrid_t *grid, int num_iters, float dt, float over_relaxation)
{
    for (int i = 0; i < grid->f_num_cells; i++)
    {
        grid->p[i] = 0.0f;
        grid->prev_u[i] = grid->u[i];
        grid->prev_v[i] = grid->v[i];
    }

    int n = grid->f_num_x;
    double cp = grid->density * grid->h / dt;

    for (int iter = 0; iter < num_iters; iter++)
    {
        for (int i = 1; i < grid->f_num_x - 1; i++)
        {
            for (int j = 1; j < grid->f_num_y - 1; j++)
            {
                int center = j * n + i;
                if (grid->cell_type[center] != FLUID)
                {
                    continue;
                }

                int left = j * n + (i - 1);
                int right = j * n + (i + 1);
                int bottom = (j - 1) * n + i;
                int top = (j + 1) * n + i;

                float sx0 = grid->s[left];
                float sx1 = grid->s[right];
                float sy0 = grid->s[bottom];
                float sy1 = grid->s[top];
                float s = sx0 + sx1 + sy0 + sy1;

                if (s == 0.0f)
                    continue;

                float div = grid->u[right] - grid->u[center] + grid->v[top] - grid->v[center];

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
            }
        }
    }
}
Pair_t FLIP2D_transformGridToVerticiesMarchingSquares(
    FlipGrid_t *grid, Vertex_t *vertices, GLuint *indices,
    int show_sci, int show_air, int show_solids)
{
    size_t vert_index = 0;
    size_t ind_index = 0;

    float minPressure = FLT_MAX, maxPressure = FLT_MIN;

    const vec3s cornerOffsets2D[4] = {{{0.0f, 0.0f, 0.0f}}, {{1.0f, 0.0f, 0.0f}}, {{1.0f, 1.0f, 0.0f}}, {{0.0f, 1.0f, 0.0f}}};

    vec3s vertexList[4];
    float cornerValues[4];

    for (int y = 0; y < grid->f_num_y; y++)
    {
        for (int x = 0; x < grid->f_num_x; x++)
        {
            int cellNr = y * grid->f_num_x + x;
            vec3s squarePos = {{x * grid->h, y * grid->h, 0}};

            int squareIndex = 0;

            for (int i = 0; i < 4; i++)
            {
                int cornerCell = cellNr +
                                 (int)cornerOffsets2D[i].raw[0] +
                                 (int)cornerOffsets2D[i].raw[1] * grid->f_num_x;

                cornerValues[i] = grid->p[cornerCell];

                if (isnan(cornerValues[i]) || isinf(cornerValues[i]))
                    cornerValues[i] = 0.0f;

                if (cornerValues[i] <= 0.0f)
                    squareIndex |= (1 << i);

                if (grid->cell_type[cornerCell] == FLUID)
                {
                    minPressure = minf(minPressure, cornerValues[i]);
                    maxPressure = maxf(maxPressure, cornerValues[i]);
                }
            }

            if (squareIndex == 0 || squareIndex == 15)
                continue;

            for (int i = 0; i < 4; i++)
            {
                vertexList[i] = {{0.0f, 0.0f, 0.0f}};
            }

            for (int i = 0; i < 4; i++)
            {
                if (edgeTable2D[squareIndex] & (1 << i))
                {
                    int idxA = cornerIndexAFromEdge2D[i];
                    int idxB = cornerIndexBFromEdge2D[i];

                    float valA = cornerValues[idxA];
                    float valB = cornerValues[idxB];

                    float t = (valA == valB) ? 0.5f : clampf((0.0f - valA) / (valB - valA), 0.0f, 1.0f);
                    vec3s a = glms_vec3_add(squarePos, glms_vec3_scale(cornerOffsets2D[idxA], grid->h));
                    vec3s b = glms_vec3_add(squarePos, glms_vec3_scale(cornerOffsets2D[idxB], grid->h));
                    vertexList[i] = glms_vec3_lerp(a, b, t);
                }
            }

            for (int i = 0; triTable2D[squareIndex][i] != -1 && i < 4; i += 3)
            {
                vec3s v0 = vertexList[triTable2D[squareIndex][i]];
                vec3s v1 = vertexList[triTable2D[squareIndex][i + 1]];
                vec3s v2 = vertexList[triTable2D[squareIndex][i + 2]];

                vec3s edge1 = glms_vec3_sub(v1, v0);
                vec3s edge2 = glms_vec3_sub(v2, v0);
                vec3s normal = glms_vec3_normalize(glms_vec3_cross(edge1, edge2));

                for (int j = 0; j < 3; j++)
                {
                    int vertID = triTable2D[squareIndex][i + j];
                    vec3s color = {{0.113f, 0.353f, 0.403f}};

                    if (show_sci)
                    {
                        getSciColor(cornerValues[vertID], minPressure, maxPressure, color.raw);
                    }

                    vertices[vert_index].position = vertexList[vertID];
                    vertices[vert_index].color = color;
                    vertices[vert_index].normal = normal;

                    indices[ind_index++] = vert_index++;
                }
            }
        }
    }

    return {.first = (int)vert_index, .second = (int)ind_index};
}
