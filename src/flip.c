#include "flip.h"

void FLIP_init(FlipGrid_t *grid, float density, float width, float height, float spacing, float particle_radius, int num_particles)
{
    printf("Initializing FLIP grid...\n");

    grid->density = density;
    grid->f_num_x = (int)(floorf(width / spacing) + 1.0f);
    grid->f_num_y = (int)(floorf(height / spacing) + 1.0f);
    grid->h = maxf(width / grid->f_num_x, height / grid->f_num_y);
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
    grid->cell_color = (float *)calloc(grid->f_num_cells * 3, sizeof(float));
    ALLOC_CHECK(grid->cell_color, "cell_color");

    grid->num_particles = num_particles;
    grid->particle_pos = (float *)calloc(grid->num_particles * 2, sizeof(float));
    ALLOC_CHECK(grid->particle_pos, "particle_pos");
    grid->particle_vel = (float *)calloc(grid->num_particles * 2, sizeof(float));
    ALLOC_CHECK(grid->particle_vel, "particle_vel");
    grid->particle_density = (float *)calloc(grid->f_num_cells, sizeof(float));
    ALLOC_CHECK(grid->particle_density, "particle_density");

    grid->particle_radius = particle_radius;
    grid->p_inv_spacing = 1.0f / (2.2f * grid->particle_radius);
    grid->p_num_x = (int)(floorf(width * grid->p_inv_spacing) + 1.0f);
    grid->p_num_y = (int)(floorf(height * grid->p_inv_spacing) + 1.0f);
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
                grid->particle_pos[2 * particle_index] = x;
                grid->particle_pos[2 * particle_index + 1] = y;
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

    printf("FLIP grid initialized successfully!\n");
    printf("f_num_x = %d, f_num_y = %d, h = %.2f, f_inv_spacing = %.2f, f_num_cells = %d\n",
           grid->f_num_x, grid->f_num_y, grid->h, grid->f_inv_spacing, grid->f_num_cells);
    printf("p_num_x = %d, p_num_y = %d, p_num_cells = %d\n", grid->p_num_x, grid->p_num_y, grid->p_num_cells);
    printf("num_particles = %d, particle_radius = %.2f, p_inv_spacing = %.2f\n",
           grid->num_particles, grid->particle_radius, grid->p_inv_spacing);
    printf("particle_rest_density = %.2f\n", grid->particle_rest_density);
    printf("density = %.2f\n", grid->density);
}

void FLIP_integrateParticles(FlipGrid_t *grid, float dt, float gravity)
{
    for (int i = 0; i < grid->num_particles; i++)
    {
        grid->particle_vel[2 * i + 1] += gravity * dt;
        grid->particle_pos[2 * i] += grid->particle_vel[2 * i] * dt;
        grid->particle_pos[2 * i + 1] += grid->particle_vel[2 * i + 1] * dt;
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
        float x = grid->particle_pos[2 * i];
        float y = grid->particle_pos[2 * i + 1];

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
        float x = grid->particle_pos[2 * i];
        float y = grid->particle_pos[2 * i + 1];

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
            float px = grid->particle_pos[2 * i];
            float py = grid->particle_pos[2 * i + 1];

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

                        float qx = grid->particle_pos[2 * id];
                        float qy = grid->particle_pos[2 * id + 1];

                        float dx = qx - px;
                        float dy = qy - py;
                        float d2 = dx * dx + dy * dy;
                        if (d2 > minDist2 || d2 == 0.0f)
                            continue;

                        float d = sqrtf(d2);
                        float s = 0.5f * (minDist - d) / d;

                        dx *= s;
                        dy *= s;

                        grid->particle_pos[2 * i] -= dx;
                        grid->particle_pos[2 * i + 1] -= dy;
                        grid->particle_pos[2 * id] += dx;
                        grid->particle_pos[2 * id + 1] += dy;
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

    float obstacle_x = obstacle->x;
    float obstacle_y = obstacle->y;

    float min_dist = particle_r + obstacle->radius;
    float min_dist_2 = min_dist * min_dist;

    for (int i = 0; i < grid->num_particles; i++)
    {
        float x = grid->particle_pos[2 * i];
        float y = grid->particle_pos[2 * i + 1];

        float dx = x - obstacle_x;
        float dy = y - obstacle_y;

        float dist_2 = dx * dx + dy * dy;
        if (dist_2 < min_dist_2)
        {
            grid->particle_vel[2 * i] = Obstacle_getXVelocity(obstacle, dt) * obstacle->push_coefficient;
            grid->particle_vel[2 * i + 1] = Obstacle_getYVelocity(obstacle, dt) * obstacle->push_coefficient;
        }

        if (grid->particle_pos[2 * i] < min_x)
        {
            grid->particle_pos[2 * i] = min_x;
            grid->particle_vel[2 * i] = 0.0f;
        }
        if (grid->particle_pos[2 * i] > max_x)
        {
            grid->particle_pos[2 * i] = max_x;
            grid->particle_vel[2 * i] = 0.0f;
        }
        if (grid->particle_pos[2 * i + 1] < min_y)
        {
            grid->particle_pos[2 * i + 1] = min_y;
            grid->particle_vel[2 * i + 1] = 0.0f;
        }
        if (grid->particle_pos[2 * i + 1] > max_y)
        {
            grid->particle_pos[2 * i + 1] = max_y;
            grid->particle_vel[2 * i + 1] = 0.0f;
        }
    }
}

void FLIP_updateParticleDensity(FlipGrid_t *grid)
{
}

void FLIP_transferVelocities(FlipGrid_t *grid, int toGrid, float FLIPRatio)
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

            grid->cell_type[i] = AIR;
        }

        for (int i=0; i<grid->num_particles; i++)
        {
            float x = grid->particle_pos[2 * i];
            float y = grid->particle_pos[2 * i + 1];

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
            float x = grid->particle_pos[2 * i];
            float y = grid->particle_pos[2 * i + 1];

            float x0 = floorf((x - shift_x) * grid->f_inv_spacing);
            float y0 = floorf((y - shift_y) * grid->f_inv_spacing);

            float x1 = x0 + 1;
            float y1 = y0 + 1;

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
                float particle_vel = grid->particle_vel[2 * i + component];
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
                float velocity = grid->particle_vel[2 * i + component];

                float d_v = valid0 * d0 + valid1 * d1 + valid2 * d2 + valid3 * d3;
                if (d_v > 0.0f)
                {
                    float pic_vel = (valid0 * d0 * f[nr0] + valid1 * d1 * f[nr1] + valid2 * d2 * f[nr2] + valid3 * d3 * f[nr3]) / d_v;
                    float corr = (valid0 * d0 * (f[nr0] - prev_f[nr0]) + valid1 * d1 * (f[nr1] - prev_f[nr1]) + valid2 * d2 * (f[nr2] - prev_f[nr2]) + valid3 * d3 * (f[nr3] - prev_f[nr3])) / d_v;
                    grid->particle_vel[2 * i + component] = FLIPRatio * pic_vel + (1.0f - FLIPRatio) * (velocity + corr);
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

void FLIP_solveIncompressibility(FlipGrid_t *grid, int num_iters, float dt, float over_relaxation)
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

Pair_t FLIP_transformGridToVerticies(FlipGrid_t *grid, Vertex_t *vertices, GLuint *indices, int show_sci)
{
    glm::vec3 cubeVertices[8] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(1.0f, 0.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(0.0f, 1.0f, 1.0f)};

    GLuint cubeIndices[36] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        4, 0, 3, 3, 7, 4,
        1, 5, 6, 6, 2, 1,
        3, 2, 6, 6, 7, 3,
        4, 5, 1, 1, 0, 4};

    size_t vert_index = 0;
    size_t ind_index = 0;

    float minPressure = FLT_MAX;
    float maxPressure = FLT_MIN;
    float sumPressure = 0.0f;
    for (int i = 0; i < grid->f_num_cells; i++)
    {
        if (grid->cell_type[i] == FLUID)
        {
            float p = grid->p[i];
            if (p < minPressure)
                minPressure = p;
            if (p > maxPressure)
                maxPressure = p;
            sumPressure += p;
        }
    }

    for (int y = 0; y < grid->f_num_y; y++)
    {
        for (int x = 0; x < grid->f_num_x; x++)
        {
            int cell_nr = y * grid->f_num_x + x;
            glm::vec3 cubePos = glm::vec3(x, y, 0) * grid->h;
            float c[3] = {0.0f, 0.0f, 1.0f};
            if (grid->cell_type[cell_nr] == FLUID)
            {
                if (show_sci)
                {
                    getSciColor(grid->p[cell_nr], minPressure, maxPressure, c);
                }
            }
            else if (grid->cell_type[cell_nr] == SOLID)
            {
                c[0] = 1.0f;
                c[1] = 1.0f;
                c[2] = 1.0f;
            }
            else if (grid->cell_type[cell_nr] == AIR)
            {
                c[0] = 0.53f;
                c[1] = 0.81f;
                c[2] = 0.94f;
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
    }
    return {.first = (int)vert_index, .second = (int)ind_index};
}

#define SPHERE_LAT_SLICES 5
#define SPHERE_LON_SLICES 5
Pair_t FLIP_transformMarkersToVertices(FlipGrid_t *grid, Vertex_t *markerVertices, GLuint *markerIndices)
{
    int vertexOffset = 0;
    int indexOffset = 0;

    for (int m = 0; m < grid->num_particles; m++)
    {
        glm::vec3 markerPos = glm::vec3(grid->particle_pos[2 * m], grid->particle_pos[2 * m + 1], grid->h);
        glm::vec3 markerColor = glm::vec3(1.0f, 0.0f, 0.0f);
        float radius = grid->particle_radius;
        for (int i = 0; i <= SPHERE_LAT_SLICES; i++)
        {
            float theta = (float)i / SPHERE_LAT_SLICES * M_PI;
            float sinTheta = sinf(theta);
            float cosTheta = cosf(theta);

            for (int j = 0; j <= SPHERE_LON_SLICES; j++)
            {
                float phi = (float)j / SPHERE_LON_SLICES * 2.0f * M_PI;
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

        for (int i = 0; i < SPHERE_LAT_SLICES; i++)
        {
            for (int j = 0; j < SPHERE_LON_SLICES; j++)
            {
                int first = vertexOffset - (SPHERE_LAT_SLICES + 1) * (SPHERE_LON_SLICES + 1) + i * (SPHERE_LON_SLICES + 1) + j;
                int second = first + SPHERE_LON_SLICES + 1;

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
    // free(&grid->u);
    // grid->u = NULL;
    // free(grid->v);
    // grid->v = NULL;
    // free(grid->du);
    // grid->du = NULL;
    // free(grid->dv);
    // grid->dv = NULL;
    // free(grid->prev_u);
    // grid->prev_u = NULL;
    // free(grid->prev_v);
    // grid->prev_v = NULL;
    // free(grid->p);
    // grid->p = NULL;
    // free(grid->s);
    // grid->s = NULL;
    // free(grid->cell_type);
    // grid->cell_type = NULL;
    // free(grid->cell_color);
    // grid->cell_color = NULL;
    // free(grid->particle_pos);
    // grid->particle_pos = NULL;
    // free(grid->particle_vel);
    // grid->particle_vel = NULL;
    // free(grid->particle_density);
    // grid->particle_density = NULL;
    // free(grid->num_cell_particles);
    // grid->num_cell_particles = NULL;
    // free(grid->first_cell_particle);
    // grid->first_cell_particle = NULL;
    // free(grid->cell_particle_ids);
    // grid->cell_particle_ids = NULL;
}
