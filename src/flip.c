#include "flip.h"

#define RED_TEXT "\033[31m"
#define RESET_TEXT "\033[0m"

void printMarkers(MacGrid_t *grid)
{
    printf("\nMarkers visualization:\n");
    for (int i = 0; i < grid->num_particles; i++)
    {
        printf("[%d] velocity: (%.2f %.2f) position: (%.2f %.2f)\n",
               i,
               grid->particle_vel[2 * i], grid->particle_vel[2 * i + 1],
               grid->particle_pos[2 * i], grid->particle_pos[2 * i + 1]);
    }
}

void printGrid(MacGrid_t *grid)
{
    printf("Grid visualization:\n");

    for (int j = grid->f_num_y - 1; j >= 0; j--)
    {
        for (int i = 0; i < grid->f_num_x; i++)
        {
            printf("+----------------------");
        }
        printf("+\n");

        for (int i = 0; i < grid->f_num_x; i++)
        {
            printf("| %6.1f   %6.1f      ", grid->u[j * grid->f_num_x + i],
                   grid->p[j * grid->f_num_x + i]);
        }
        printf("|\n");

        for (int i = 0; i < grid->f_num_x; i++)
        {
            printf("|        %6.1f        ", grid->v[j * grid->f_num_x + i]);
        }
        printf("|\n");
    }

    for (int i = 0; i < grid->f_num_x; i++)
    {
        printf("+----------------------");
    }
    printf("+\n");

    printMarkers(grid);
}
// Alokacja pamięci + sprawdzanie błędów
#define ALLOC_CHECK(ptr, name)                                     \
    if (!(ptr))                                                    \
    {                                                              \
        printf("ERROR: Failed to allocate memory for " name "\n"); \
        exit(EXIT_FAILURE);                                        \
    }
void MAC_init(MacGrid_t *grid, float density, float width, float height, float spacing, float particle_radius, int max_particles)
{
    printf("Initializing MAC grid...\n");

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

    grid->max_particles = max_particles;
    grid->particle_pos = (float *)calloc(grid->max_particles * 2, sizeof(float));
    ALLOC_CHECK(grid->particle_pos, "particle_pos");
    grid->particle_vel = (float *)calloc(grid->max_particles * 2, sizeof(float));
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
    grid->cell_particle_ids = (int *)calloc(grid->max_particles, sizeof(int));
    ALLOC_CHECK(grid->cell_particle_ids, "cell_particle_ids");

    grid->num_particles = minf(grid->max_particles, grid->p_num_x * grid->p_num_y);

    int i = 1, j = 1;
    for (int w = 0; w < grid->num_particles * 2; w += 2)
    {
        if (i >= grid->f_num_x - 1)
        {
            i = 1;
            j++;
        }
        if (j >= grid->f_num_y - 1)
        {
            break;
        }
        grid->particle_pos[w] = grid->h * i;
        grid->particle_pos[w + 1] = grid->h * j;
        i++;
    }

    for (int i = 0; i < grid->f_num_x; i++)
    {
        for (int j = 0; j < grid->f_num_y; j++)
        {
            grid->s[j * grid->f_num_x + i] = (i == 0 || i == grid->f_num_x - 1 || j == 0) ? 0.0f : 1.0f;
        }
    }

    printf("MAC grid initialized successfully!\n");
    printf("f_num_x = %d, f_num_y = %d, h = %.2f, f_inv_spacing = %.2f, f_num_cells = %d\n",
           grid->f_num_x, grid->f_num_y, grid->h, grid->f_inv_spacing, grid->f_num_cells);
    printf("p_num_x = %d, p_num_y = %d, p_num_cells = %d\n", grid->p_num_x, grid->p_num_y, grid->p_num_cells);
    printf("max_particles = %d, particle_radius = %.2f, p_inv_spacing = %.2f\n",
           grid->max_particles, grid->particle_radius, grid->p_inv_spacing);
    printf("particle_rest_density = %.2f\n", grid->particle_rest_density);
    printf("density = %.2f\n", grid->density);
}

void MAC_integrateParticles(MacGrid_t *grid, float dt, float gravity)
{
    // printf("Integrating particles with dt = %.2f, gravity = %.2f\n", dt, gravity);
    for (int i = 0; i < grid->num_particles; i++)
    {
        grid->particle_vel[2 * i + 1] += gravity * dt;
        grid->particle_pos[2 * i] += grid->particle_vel[2 * i] * dt;
        grid->particle_pos[2 * i + 1] += grid->particle_vel[2 * i + 1] * dt;
    }
}

void MAC_pushParticlesApart(MacGrid_t *grid, int numIters, float dt)
{
    // printf("Pushing particles apart with numIters = %d, dt = %.2f\n", numIters, dt);
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

void MAC_handleObstacle(MacGrid_t *grid, Obstacle_t *obstacle, float dt)
{
    // printf("Handling obstacle with dt = %.2f\n", dt);
    float h = 1.0f / grid->f_inv_spacing;
    float r = grid->particle_radius;
    float or2 = obstacle->radius * obstacle->radius;
    float minDist = obstacle->radius + r;
    float minDist2 = minDist * minDist;

    float minX = h + r;
    float maxX = (grid->f_num_x - 1) * h - r;
    float minY = h + r;
    float maxY = (grid->f_num_y - 1) * h - r;

    for (int i = 0; i < grid->num_particles; i++)
    {
        float x = grid->particle_pos[2 * i];
        float y = grid->particle_pos[2 * i + 1];

        float dx = x - obstacle->position.x;
        float dy = y - obstacle->position.y;
        float d2 = dx * dx + dy * dy;

        if (d2 < minDist2)
        {
            glm::vec3 obstacle_vel = glm::vec3(obstacle->position - obstacle->last_position) * obstacle->speed * 5.0f;
            grid->particle_vel[2 * i] = obstacle_vel.x;
            grid->particle_vel[2 * i + 1] = obstacle_vel.y;
            // printf("obstacle (%.2f, %.2f) hit marker %d and resulted in its velocity = (%.2f, %.2f)\n", obstacle->position.x, obstacle->position.y, marker_index, marker.velocity.x, marker.velocity.y);
        }

        if (grid->particle_pos[2 * i] < minX)
        {
            grid->particle_pos[2 * i] = minX;
            grid->particle_vel[2 * i] = 0.0f;
        }
        if (grid->particle_pos[2 * i] > maxX)
        {
            grid->particle_pos[2 * i] = maxX;
            grid->particle_vel[2 * i] = 0.0f;
        }
        if (grid->particle_pos[2 * i + 1] < minY)
        {
            grid->particle_pos[2 * i + 1] = minY;
            grid->particle_vel[2 * i + 1] = 0.0f;
        }
        if (grid->particle_pos[2 * i + 1] > maxY)
        {
            grid->particle_pos[2 * i + 1] = maxY;
            grid->particle_vel[2 * i + 1] = 0.0f;
        }
    }
}

void MAC_updateParticleDensity(MacGrid_t *grid)
{
    // printf("Updating particle density\n");
    int n = grid->f_num_x;
    float h = grid->h;
    float h1 = grid->f_inv_spacing;
    float h2 = 0.5f * h;

    // memset(grid->particle_density, 0, sizeof(float) * grid->f_num_cells);
    for (int i = 0; i < grid->f_num_cells; i++)
    {
        grid->particle_density[i] = 0.0f;
    }

    for (int i = 0; i < grid->num_particles; i++)
    {
        float x = grid->particle_pos[2 * i];
        float y = grid->particle_pos[2 * i + 1];

        x = clampf(x, h, (grid->f_num_x - 1) * h);
        y = clampf(y, h, (grid->f_num_y - 1) * h);

        int x0 = (int)floorf((x - h2) * h1);
        float tx = ((x - h2) - x0 * h) * h1;
        int x1 = (int)min(grid->f_num_x - 2, x0 + 1);

        int y0 = (int)floorf((y - h2) * h1);
        float ty = ((y - h2) - y0 * h) * h1;
        int y1 = (int)min(grid->f_num_y - 2, y0 + 1);

        float sx = 1.0f - tx;
        float sy = 1.0f - ty;

        if (x0 < grid->f_num_x && y0 < grid->f_num_y)
            grid->particle_density[y0 * n + x0] += sx * sy;

        if (x1 < grid->f_num_x && y0 < grid->f_num_y)
            grid->particle_density[y0 * n + x1] += tx * sy;

        if (x1 < grid->f_num_x && y1 >= 0 && y1 < grid->f_num_y)
            grid->particle_density[y1 * n + x1] += tx * ty;

        if (x0 < grid->f_num_x && y1 < grid->f_num_y)
            grid->particle_density[y1 * n + x0] += sx * ty;

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
}

void MAC_transferVelocities(MacGrid_t *grid, int toGrid, float flipRatio)
{
    // printf("Transferring velocities with toGrid = %d, flipRatio = %.2f\n", toGrid, flipRatio);
    int n = grid->f_num_x;
    float h = grid->h;
    float h1 = grid->f_inv_spacing;
    float h2 = 0.5f * h;

    if (toGrid)
    {
        // memset(grid->u, 0, sizeof(float) * grid->f_num_cells);
        // memset(grid->v, 0, sizeof(float) * grid->f_num_cells);
        // memset(grid->du, 0, sizeof(float) * grid->f_num_cells);
        // memset(grid->dv, 0, sizeof(float) * grid->f_num_cells);

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
            float x = grid->particle_pos[2 * i];
            float y = grid->particle_pos[2 * i + 1];
            int xi = clamp((int)floorf(x * h1), 0, grid->f_num_x - 1);
            int yi = clamp((int)floorf(y * h1), 0, grid->f_num_y - 1);
            int cell_nr = yi * n + xi;
            if (grid->cell_type[cell_nr] == AIR)
            {
                grid->cell_type[cell_nr] = FLUID;
            }
        }
    }

    for (int component = 0; component < 2; component++) // 0 === u/x, 1 === v/y
    {
        float dx = component == 0 ? 0.0f : h2;
        float dy = component == 0 ? h2 : 0.0f;

        float *f = component == 0 ? grid->u : grid->v;
        float *prev_f = component == 0 ? grid->prev_u : grid->prev_v;
        float *d = component == 0 ? grid->du : grid->dv;

        for (int i = 0; i < grid->num_particles; i++)
        {
            float x = clampf(grid->particle_pos[2 * i], h, (grid->f_num_x - 1.0f) * h);
            float y = clampf(grid->particle_pos[2 * i + 1], h, (grid->f_num_y - 1.0f) * h);

            int x0 = min((int)floorf((x - dx) * h1), grid->f_num_x - 2);
            float tx = (x - dx - x0 * h) * h1;
            int x1 = min(x0 + 1, grid->f_num_x - 2);

            int y0 = min((int)floorf((y - dy) * h1), grid->f_num_y - 2);
            float ty = (y - dy - y0 * h) * h1;
            int y1 = min(y0 + 1, grid->f_num_y - 2);

            float sx = 1.0f - tx;
            float sy = 1.0f - ty;

            float d0 = sx * sy;
            float d1 = tx * sy;
            float d2 = tx * ty;
            float d3 = sx * ty;

            int nr0 = y0 * n + x0;
            int nr1 = y0 * n + x1;
            int nr2 = y1 * n + x1;
            int nr3 = y1 * n + x0;

            if (toGrid)
            {
                float pv = grid->particle_vel[2 * i + component];

                f[nr0] += d0 * pv;
                d[nr0] += d0;

                f[nr1] += d1 * pv;
                d[nr1] += d1;

                f[nr2] += d2 * pv;
                d[nr2] += d2;

                f[nr3] += d3 * pv;
                d[nr3] += d3;
            }
            else
            {
                int offset = component == 0 ? 1 : n; // TODO czemu zmiana nic nie daje? xd
                float valid0 = grid->cell_type[nr0] != AIR || grid->cell_type[nr0 - offset] != AIR ? 1.0f : 0.0f;
                float valid1 = grid->cell_type[nr1] != AIR || grid->cell_type[nr1 - offset] != AIR ? 1.0f : 0.0f;
                float valid2 = grid->cell_type[nr2] != AIR || grid->cell_type[nr2 - offset] != AIR ? 1.0f : 0.0f;
                float valid3 = grid->cell_type[nr3] != AIR || grid->cell_type[nr3 - offset] != AIR ? 1.0f : 0.0f;

                float v = grid->particle_vel[2 * i + component];
                float d_v = valid0 * d0 + valid1 * d1 + valid2 * d2 + valid3 * d3;
                if (d_v > 0.0f)
                {
                    float picV = (valid0 * d0 * f[nr0] + valid1 * d1 * f[nr1] + valid2 * d2 * f[nr2] + valid3 * d3 * f[nr3]) / d_v;
                    float corr = (valid0 * d0 * (f[nr0] - prev_f[nr0]) + valid1 * d1 * (f[nr1] - prev_f[nr1]) + valid2 * d2 * (f[nr2] - prev_f[nr2]) + valid3 * d3 * (f[nr3] - prev_f[nr3])) / d_v;
                    float flipV = v + corr;

                    grid->particle_vel[2 * i + component] = flipRatio * flipV + (1.0f - flipRatio) * picV;
                }
            }
        }

        if (toGrid)
        {
            for (int i = 0; i < grid->f_num_cells; i++)
            {
                if (d[i] > 0.0f)
                {
                    f[i] /= d[i];
                }
            }

            for (int i = 0; i < grid->f_num_x; i++)
            {
                for (int j = 0; j < grid->f_num_y; j++)
                {
                    int solid = grid->cell_type[j * n + i] == SOLID ? 1 : 0;
                    if (solid == 1 || (i > 0 && grid->cell_type[j * n + (i - 1)] == SOLID))
                    {
                        grid->u[j * n + i] = grid->prev_u[j * n + i];
                    }
                    if (solid == 1 || (j > 0 && grid->cell_type[j * n + i - 1] == SOLID))
                    {
                        grid->v[j * n + i] = grid->prev_v[j * n + i];
                    }
                }
            }
        }
    }
}

void MAC_solveIncompressibility(MacGrid_t *grid, int num_iters, float dt, float over_relaxation)
{
    // printf("Solving incompressibility with num_iters = %d, dt = %.2f, over_relaxation = %.2f\n", num_iters, dt, over_relaxation);
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

                double div = grid->u[right] - grid->u[center] + grid->v[top] - grid->v[center];

                if (grid->particle_rest_density > 0.0f)
                {
                    float k = 1.0f;
                    float compression = grid->particle_density[center] - grid->particle_rest_density;
                    if (compression > 0.0f)
                    {
                        div -= k * compression;
                    }
                }

                double p = -div / s;
                p *= over_relaxation;
                grid->p[center] += cp * p;
                // printf("sx0: %.2f, sx1: %.2f, sy0: %.2f, sy1: %.2f, s: %.2f, \ndiv: %.2f, p: %.2f \ncp: %.2f density: %.2f dt: %.2f\n", sx0, sx1, sy0, sy1, s, div, p, cp, center->density, dt);
                grid->u[center] -= sx0 * p;
                grid->u[right] += sx1 * p;
                grid->v[center] -= sy0 * p;
                grid->v[top] += sy1 * p;
            }
        }
    }
    // printGrid(grid);
}

Pair_t MAC_transformGridToVerticies(MacGrid_t *grid, Vertex_t *vertices, GLuint *indices, int show_sci)
{
    glm::vec3 cubeVertices[8] = {
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(0.5f, -0.5f, -0.5f),
        glm::vec3(0.5f, 0.5f, -0.5f),
        glm::vec3(-0.5f, 0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f, 0.5f),
        glm::vec3(0.5f, -0.5f, 0.5f),
        glm::vec3(0.5f, 0.5f, 0.5f),
        glm::vec3(-0.5f, 0.5f, 0.5f)};

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

    // printf("averagePressure: %f minPressure: %f, maxPressure: %f\n", (sumPressure/grid->total_size), minPressure, maxPressure);

    for (int x = 0; x < grid->f_num_x; ++x)
    {
        for (int y = 0; y < grid->f_num_y; ++y)
        {
            int cell_nr = y * grid->f_num_x + x;
            glm::vec3 cubePos = glm::vec3(x, y, 0) * grid->h;
            // printf("cellIndex: %d, cellType: %d cellX: %d, cellY: %d, cubePos: (%.2f, %.2f, %.2f)\n", cellIndex, grid->cell_type[cellIndex], x, y, cubePos.x, cubePos.y, cubePos.z);
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
int frames_Dad = -1;
Pair_t MAC_transformMarkersToVertices(MacGrid_t *grid, Vertex_t *markerVertices, GLuint *markerIndices)
{
    int vertexOffset = 0;
    int indexOffset = 0;

    for (int m = 0; m < grid->num_particles; m++)
    {
        glm::vec3 markerPos = glm::vec3(grid->particle_pos[2 * m], grid->particle_pos[2 * m + 1], grid->h);
        glm::vec3 markerColor = glm::vec3(1.0f, 0.0f, 0.0f);
        float radius = grid->particle_radius;
        if (frames_Dad > 0 || frames_Dad == -1)
        {
            printf("markerPos: (%.2f, %.2f, %.2f) with color: (%.2f, %.2f, %.2f)\n", markerPos.x, markerPos.y, markerPos.z, markerColor.x, markerColor.y, markerColor.z);
        }
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
    frames_Dad--;

    return (Pair_t){.first = vertexOffset, .second = indexOffset};
}

void MAC_destroy(MacGrid_t *grid)
{
    // if (grid->cells)
    // {
    //     free(grid->cells);
    //     grid->cells = NULL;
    // }
    // if (grid->markers)
    // {
    //     free(grid->markers);
    //     grid->markers = NULL;
    // }
    // if (grid->num_cell_markers)
    // {
    //     free(grid->num_cell_markers);
    //     grid->num_cell_markers = NULL;
    // }
    // if (grid->first_cell_marker)
    // {
    //     free(grid->first_cell_marker);
    //     grid->first_cell_marker = NULL;
    // }
    // if (grid->cell_marker_ids)
    // {
    //     free(grid->cell_marker_ids);
    //     grid->cell_marker_ids = NULL;
    // }
}