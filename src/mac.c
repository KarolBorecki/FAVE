#include "mac.h"

void MAC_init(MacGrid_t *grid, uint16_t size_x, uint16_t size_y, float cell_size)
{
    grid->size_x = size_x;
    grid->size_y = size_y;
    grid->cell_size = cell_size;
    grid->inv_cell_size = 1.0f / cell_size;
    grid->marker_radius = 0.1f;
    grid->marker_inv_radius = 1.0f / (2.2f * grid->marker_radius);

    grid->cells = (GridCell **)malloc(size_x * sizeof(GridCell *));
    if (!grid->cells)
    {
        fprintf(stderr, "Failed to allocate memory for grid cells\n");
        exit(EXIT_FAILURE);
    }

    for (uint16_t x = 0; x < size_x; ++x)
    {
        grid->cells[x] = (GridCell *)malloc(size_y * sizeof(GridCell));
        if (!grid->cells[x])
        {
            fprintf(stderr, "Failed to allocate memory for grid cells\n");
            exit(EXIT_FAILURE);
        }
    }

    for (uint16_t x = 0; x < size_x; ++x)
    {
        for (uint16_t y = 0; y < size_y; ++y)
        {
            GridCell_t cell = {
                .p = 0.0f,
                .v = 0.0f,
                .u = 0.0f,
                .w = 0.0f,
                .dv = 0.0f,
                .du = 0.0f,
                .dw = 0.0f,
                .prevv = 0.0f,
                .prevu = 0.0f,
                .prevw = 0.0f,
                .s = 1.0f,
                .type = FLUID};
            grid->cells[x][y] = cell;
        }
    }
    grid->num_markers = size_x * size_y * 4;
    grid->markers = (Marker_t *)malloc(grid->num_markers * sizeof(Marker));

    for (int i = 0; i < grid->num_markers; ++i)
    {
        Marker_t marker;
        marker.position = glm::vec3(
            random_float(0.0f, size_x * cell_size),
            random_float(0.0f, size_y * cell_size),
            0.0f);
        marker.velocity = glm::vec3(0.0f);
        marker.color = glm::vec4(0.4f);
        grid->markers[i] = marker;
    }
}

GridCell_t &getMarkerCell(MacGrid_t *grid, Marker_t &marker)
{
    return grid->cells[clamp((int)floorf(marker.position.x * grid->inv_cell_size), 0, grid->size_x - 1)][clamp((int)floorf(marker.position.y * grid->inv_cell_size), 0, grid->size_y - 1)];
}

void MAC_handleObstacle(MacGrid_t *grid, glm::vec3 obstaclePos, float obstacleRadius)
{
    float h = 1.0f / grid->inv_cell_size;
    float r = grid->marker_radius;
    float or2 = obstacleRadius * obstacleRadius;
    float minDist = obstacleRadius + r;
    float minDist2 = minDist * minDist;

    float minX = h + r;
    float maxX = (grid->size_x - 1) * h - r;
    float minY = h + r;
    float maxY = (grid->size_y - 1) * h - r;

    for (uint16_t i = 0; i < grid->size_x * grid->size_y * 4; i++)
    {
        Marker_t &marker = grid->markers[i];

        float dx = marker.position.x - obstaclePos.x;
        float dy = marker.position.y - obstaclePos.y;
        float d2 = dx * dx + dy * dy;

        // Kolizja z przeszkodą
        if (d2 < minDist2)
        {
            float d = sqrtf(d2);
            if (d > 0.0f) // Unikanie dzielenia przez zero
            {
                float s = (minDist - d) / d;
                marker.position.x += dx * s;
                marker.position.y += dy * s;
            }

            // Można dodać prędkość przeszkody, jeśli jest dynamiczna
            marker.velocity = glm::vec3(0.0f, 0.0f, 0.0f); // Przykładowe wyzerowanie prędkości po kolizji
        }

        // Kolizje ze ścianami siatki
        if (marker.position.x < minX)
        {
            marker.position.x = minX;
            marker.velocity.x = 0.0f;
        }
        if (marker.position.x > maxX)
        {
            marker.position.x = maxX;
            marker.velocity.x = 0.0f;
        }
        if (marker.position.y < minY)
        {
            marker.position.y = minY;
            marker.velocity.y = 0.0f;
        }
        if (marker.position.y > maxY)
        {
            marker.position.y = maxY;
            marker.velocity.y = 0.0f;
        }
    }
}
void MAC_integrateParticles(MacGrid_t *grid, float dt, float gravity)
{
    for (int i = 1; i < grid->size_x - 1; i++)
    {
        for (int j = 1; j < grid->size_y - 1; j++)
        {
            if (grid->cells[i][j].type == FLUID)
            {
                grid->cells[i][j].v += dt * gravity;
                grid->cells[i][j].u += grid->cells[i][j].du * dt;
                grid->cells[i][j].v += grid->cells[i][j].dv * dt;
            }
        }
    }
}

void MAC_pushParticlesApart(MacGrid_t *grid, int numIters)
{
    float minDist = 2.0f * grid->marker_radius;
    float minDist2 = minDist * minDist;
    float colorDiffusionCoeff = 0.001f;

    for (int iter = 0; iter < numIters; iter++)
    {
        for (uint16_t i = 0; i < grid->num_markers; i++)
        {
            Marker_t &marker = grid->markers[i];
            glm::vec3 &pos = marker.position;

            int xi = clamp((int)floorf(pos.x * grid->inv_cell_size), 0, grid->size_x - 1);
            int yi = clamp((int)floorf(pos.y * grid->inv_cell_size), 0, grid->size_y - 1);

            for (int dx = -1; dx <= 1; dx++)
            {
                for (int dy = -1; dy <= 1; dy++)
                {
                    int nx = xi + dx;
                    int ny = yi + dy;
                    if (nx < 0 || ny < 0 || nx >= grid->size_x || ny >= grid->size_y)
                        continue;

                    for (uint16_t j = 0; j < grid->num_markers; j++)
                    {
                        if (i == j)
                            continue;

                        Marker_t &neighbor = grid->markers[j];
                        glm::vec3 &neighborPos = neighbor.position;

                        glm::vec3 delta = neighborPos - pos;
                        float d2 = glm::dot(delta, delta);
                        if (d2 > minDist2 || d2 == 0.0f)
                            continue;

                        float d = sqrtf(d2);
                        glm::vec3 displacement = 0.5f * (minDist - d) / d * delta;

                        pos -= displacement;
                        neighborPos += displacement;

                        // Diffuse colors
                        for (int k = 0; k < 3; k++)
                        {
                            float color0 = marker.color[k];
                            float color1 = neighbor.color[k];
                            float mixedColor = (color0 + color1) * 0.5f;
                            marker.color[k] += (mixedColor - color0) * colorDiffusionCoeff;
                            neighbor.color[k] += (mixedColor - color1) * colorDiffusionCoeff;
                        }
                    }
                }
            }
        }
    }
}

void MAC_handleParticleCollisions(MacGrid_t *grid, float obstacleX, float obstacleY, float obstacleRadius)
{
    float h = grid->cell_size;
    float r = grid->marker_radius;
    float or2 = obstacleRadius * obstacleRadius;
    float minDist = obstacleRadius + r;
    float minDist2 = minDist * minDist;

    float minX = h + r;
    float maxX = (grid->size_x - 1) * h - r;
    float minY = h + r;
    float maxY = (grid->size_y - 1) * h - r;

    for (uint16_t i = 0; i < grid->num_markers; i++)
    {
        Marker_t &marker = grid->markers[i];

        float dx = marker.position.x - obstacleX;
        float dy = marker.position.y - obstacleY;
        float d2 = dx * dx + dy * dy;

        // Kolizja z przeszkodą
        if (d2 < minDist2)
        {
            float d = sqrtf(d2);
            if (d > 0.0f) // Unikanie dzielenia przez zero
            {
                float s = (minDist - d) / d;
                marker.position.x += dx * s;
                marker.position.y += dy * s;
            }

            // // Można dodać prędkość przeszkody, jeśli jest dynamiczna
            // marker.velocity = glm::vec3(0.0f, 0.0f, 0.0f); // Przykładowe wyzerowanie prędkości po kolizji
        }

        // Kolizje ze ścianami siatki
        if (marker.position.x < minX)
        {
            marker.position.x = minX;
            marker.velocity.x = 0.0f;
        }
        if (marker.position.x > maxX)
        {
            marker.position.x = maxX;
            marker.velocity.x = 0.0f;
        }
        if (marker.position.y < minY)
        {
            marker.position.y = minY;
            marker.velocity.y = 0.0f;
        }
        if (marker.position.y > maxY)
        {
            marker.position.y = maxY;
            marker.velocity.y = 0.0f;
        }
    }
}

void MAC_transferVelocities(MacGrid_t *grid, int toGrid, float flipRatio)
{
    uint16_t n = grid->size_y;
    float h = grid->cell_size;
    float h1 = grid->inv_cell_size;
    float h2 = 0.5f * h;

    if (toGrid)
    {
        for (uint16_t grid_x = 0; grid_x < grid->size_x; grid_x++)
        {
            for (uint16_t grid_y = 0; grid_y < grid->size_y; grid_y++)
            {
                GridCell &cell = grid->cells[grid_x][grid_y];
                cell.prevu = cell.u;
                cell.prevv = cell.v;
                cell.du = 0.0f;
                cell.dv = 0.0f;
                cell.u = 0.0f;
                cell.v = 0.0f;
                cell.type = cell.s == 0.0f ? SOLID : AIR;
            }
        }

        for (uint16_t marker_index = 0; marker_index < grid->num_markers; marker_index++)
        {
            GridCell_t &cell = getMarkerCell(grid, grid->markers[marker_index]);
            if (cell.type != SOLID)
                cell.type = FLUID;
        }
    }

    for (uint8_t component = 0; component < 2; component++)
    {
        float dx = component == 0 ? 0.0f : 1.0f;
        float dy = component == 0 ? h2 : 0.0f;

        for (uint16_t marker_index; marker_index < grid->num_markers; marker_index++)
        {
            float x = clampf(grid->markers[marker_index].position.x, h, (grid->size_x - 1) * h);
            float y = clampf(grid->markers[marker_index].position.y, h, (grid->size_y - 1) * h);

            uint16_t x0 = MIN(floorf((x - dx) * h1), grid->size_x - 2);
            float tx = (x - dx - x0 * h) * h1;
            uint16_t x1 = MIN(x0 + 1, grid->size_x - 2);

            uint16_t y0 = MIN(floorf((y - dy) * h1), grid->size_y - 2);
            float ty = (y - dy - y0 * h) * h1;
            uint16_t y1 = MIN(y0 + 1, grid->size_y - 2);

            float sx = 1.0f - tx;
            float sy = 1.0f - ty;

            float d0 = sx * sy;
            float d1 = tx * sy;
            float d2 = tx * ty;
            float d3 = sx * ty;

            if (toGrid)
            {
                float pv = grid->markers[marker_index].velocity[component];
                if (component == 0)
                {
                    grid->cells[x0][y0].u += d0 * pv;
                    grid->cells[x1][y0].u += d1 * pv;
                    grid->cells[x1][y1].u += d2 * pv;
                    grid->cells[x0][y1].u += d3 * pv;

                    grid->cells[x0][y0].du += d0;
                    grid->cells[x1][y0].du += d1;
                    grid->cells[x1][y1].du += d2;
                    grid->cells[x0][y1].du += d3;
                }
                else
                {
                    grid->cells[x0][y0].v += d0 * pv;
                    grid->cells[x1][y0].v += d1 * pv;
                    grid->cells[x1][y1].v += d2 * pv;
                    grid->cells[x0][y1].v += d3 * pv;

                    grid->cells[x0][y0].dv += d0;
                    grid->cells[x1][y0].dv += d1;
                    grid->cells[x1][y1].dv += d2;
                    grid->cells[x0][y1].dv += d3;
                }
            }
            else
            {
                float valid0, valid1, valid2, valid3;
                if (component == 0)
                {
                    valid0 = grid->cells[x0][y0].type != AIR || grid->cells[x0 - 1][y0].type != AIR ? 1.0f : 0.0f;
                    valid1 = grid->cells[x1][y0].type != AIR || grid->cells[x1 - 1][y0].type != AIR ? 1.0f : 0.0f;
                    valid2 = grid->cells[x1][y1].type != AIR || grid->cells[x1 - 1][y1].type != AIR ? 1.0f : 0.0f;
                    valid3 = grid->cells[x0][y1].type != AIR || grid->cells[x0 - 1][y1].type != AIR ? 1.0f : 0.0f;
                }
                else if (component == 1)
                {
                    valid0 = grid->cells[x0][y0].type != AIR || grid->cells[x0][y0 - 1].type != AIR ? 1.0f : 0.0f;
                    valid1 = grid->cells[x1][y0].type != AIR || grid->cells[x1][y0 - 1].type != AIR ? 1.0f : 0.0f;
                    valid2 = grid->cells[x1][y1].type != AIR || grid->cells[x1][y1 - 1].type != AIR ? 1.0f : 0.0f;
                    valid3 = grid->cells[x0][y1].type != AIR || grid->cells[x0][y1 - 1].type != AIR ? 1.0f : 0.0f;
                }
                float v = grid->markers[marker_index].velocity[component];
                float d = valid0 * d0 + valid1 * d1 + valid2 * d2 + valid3 * d3;
                if (d > 0.0f)
                {
                    if (component == 0)
                    {
                        float picV = (valid0 * d0 * grid->cells[x0][y0].u + valid1 * d1 * grid->cells[x1][y0].u + valid2 * d2 * grid->cells[x1][y1].u + valid3 * d3 * grid->cells[x0][y1].u) / d;
                        float corr = (valid0 * d0 * (grid->cells[x0][y0].u - grid->cells[x0][y0].prevu) + valid1 * d1 * (grid->cells[x1][y0].u - grid->cells[x1][y0].prevu) + valid2 * d2 * (grid->cells[x1][y1].u - grid->cells[x1][y1].prevu) + valid3 * d3 * (grid->cells[x0][y1].u - grid->cells[x0][y1].prevu)) / d;
                        float flipV = v + corr;
                        grid->markers[marker_index].velocity[component] = flipRatio * flipV + (1.0f - flipRatio) * picV;
                    }
                    else if (component == 1)
                    {
                        float picV = (valid0 * d0 * grid->cells[x0][y0].v + valid1 * d1 * grid->cells[x1][y0].v + valid2 * d2 * grid->cells[x1][y1].v + valid3 * d3 * grid->cells[x0][y1].v) / d;
                        float corr = (valid0 * d0 * (grid->cells[x0][y0].v - grid->cells[x0][y0].prevv) + valid1 * d1 * (grid->cells[x1][y0].v - grid->cells[x1][y0].prevv) + valid2 * d2 * (grid->cells[x1][y1].v - grid->cells[x1][y1].prevv) + valid3 * d3 * (grid->cells[x0][y1].v - grid->cells[x0][y1].prevv)) / d;
                        float flipV = v + corr;
                        grid->markers[marker_index].velocity[component] = flipRatio * flipV + (1.0f - flipRatio) * picV;
                    }
                }
            }

            if (toGrid)
            {
                if (component == 0)
                {
                    for (uint16_t grid_x = 0; grid_x < grid->size_x; grid_x++)
                    {
                        for (uint16_t grid_y = 0; grid_y < grid->size_y; grid_y++)
                        {
                            grid->cells[grid_x][grid_y].u /= grid->cells[grid_x][grid_y].du;
                        }
                    }
                }
                else if (component == 1)
                {
                    for (uint16_t grid_x = 0; grid_x < grid->size_x; grid_x++)
                    {
                        for (uint16_t grid_y = 0; grid_y < grid->size_y; grid_y++)
                        {
                            grid->cells[grid_x][grid_y].v /= grid->cells[grid_x][grid_y].dv;
                        }
                    }
                }

                for (uint16_t grid_x = 0; grid_x < grid->size_x; grid_x++)
                {
                    for (uint16_t grid_y = 0; grid_y < grid->size_y; grid_y++)
                    {
                        if (grid->cells[grid_x][grid_y].type == SOLID && grid->cells[grid_x - 1][grid_y].type == SOLID)
                        {
                            grid->cells[grid_x][grid_y].u = grid->cells[grid_x][grid_y].prevu;
                        }
                        if (grid->cells[grid_x][grid_y].type == SOLID && grid->cells[grid_x][grid_y - 1].type == SOLID)
                        {
                            grid->cells[grid_x][grid_y].v = grid->cells[grid_x][grid_y].prevv;
                        }
                    }
                }
            }
        }
    }
}

void MAC_updateParticleDensity(MacGrid_t *grid)
{
    float h = grid->cell_size;
    float h1 = 1.0f / h;
    float h2 = 0.5f * h;

    for (uint32_t grid_x = 0; grid_x < grid->size_x; grid_x++)
    {
        for (uint32_t grid_y = 0; grid_y < grid->size_y; grid_y++)
        {
            GridCell &cell = grid->cells[grid_x][grid_y];
            cell.density = 0.0f;
        }
    }

    for (uint32_t marker_index = 0; marker_index < grid->num_markers; marker_index++)
    {

        Marker_t &marker = grid->markers[marker_index];
        float x = marker.position.x;
        float y = marker.position.y;

        int x0 = (int)floorf((x - h2) * h1);
        float tx = ((x - h2) - x0 * h) * h1;
        int x1 = (int)MIN(x0 + 1, grid->size_x - 2);

        int y0 = (int)floorf((y - h2) * h1);
        float ty = ((y - h2) - y0 * h) * h1;
        int y1 = (int)MIN(y0 + 1, grid->size_y - 2);

        float sx = 1.0f - tx;
        float sy = 1.0f - ty;

        if (x0 < grid->size_x && y0 < grid->size_y)
        {
            // float d00 = grid->cells[x0][y0].s;
            // float d01 = grid->cells[x0][y1].s;
            // float d10 = grid->cells[x1][y0].s;
            // float d11 = grid->cells[x1][y1].s;

            // float d0 = sx * d00 + tx * d10;
            // float d1 = sx * d01 + tx * d11;

            // float d = sy * d0 + ty * d1;

            // marker.density += d;
            grid->cells[x0][y0].density += sx * sy;
        }
        if (x1 < grid->size_x && y0 < grid->size_y)
        {
            // float d00 = grid->cells[x1][y0].s;
            // float d01 = grid->cells[x1][y1].s;

            // float d = sx * d00 + tx * d01;

            // marker.density += d;
            grid->cells[x1][y0].density += tx * sy;
        }
        if (x1 < grid->size_x && y1 < grid->size_y)
        {
            // float d00 = grid->cells[x1][y1].s;

            // float d = sx * d00 + tx * d00;

            // marker.density += d;
            grid->cells[x1][y1].density += tx * ty;
        }
        if (x0 < grid->size_x && y1 < grid->size_y)
        {
            // float d00 = grid->cells[x0][y1].s;
            // float d10 = grid->cells[x1][y1].s;

            // float d = sx * d00 + tx * d10;

            // marker.density += d;
            grid->cells[x0][y1].density += sx * ty;
        }

        if (grid->rest_density == 0.0f)
        {
            float sum = 0.0f;
            uint16_t num_cells = 0;
            for (uint16_t grid_x = 0; grid_x < grid->size_x; grid_x++)
            {
                for (uint16_t grid_y = 0; grid_y < grid->size_y; grid_y++)
                {
                    if (grid->cells[grid_x][grid_y].type == FLUID)
                    {
                        sum += grid->cells[grid_x][grid_y].density;
                        num_cells++;
                    }
                }
            }
            if (num_cells > 0)
            {
                grid->rest_density = sum / num_cells;
            }
        }
    }
}

void MAC_solveIncompressibility(MacGrid_t *grid, int numIters, float dt, float overRelaxation)
{
    for (uint32_t grid_x = 0; grid_x < grid->size_x; grid_x++)
    {
        for (uint32_t grid_y = 0; grid_y < grid->size_y; grid_y++)
        {
            GridCell_t *cell = &grid->cells[grid_x][grid_y];
            cell->p = 0.0f;
            cell->prevu = cell->u;
            cell->prevv = cell->v;
        }
    }

    float n = grid->size_y;
    float cp = grid->density * grid->cell_size / dt;

    for (uint8_t iter = 0; iter < numIters; iter++)
    {
        for (uint16_t grid_x = 1; grid_x < grid->size_x - 1; grid_x++)
        {
            for (uint16_t grid_y = 1; grid_y < grid->size_y - 1; grid_y++)
            {
                GridCell_t *center = &grid->cells[grid_x][grid_y];
                GridCell_t *right = &grid->cells[grid_x + 1][grid_y];
                GridCell_t *left = &grid->cells[grid_x - 1][grid_y];
                GridCell_t *top = &grid->cells[grid_x][grid_y + 1];
                GridCell_t *bottom = &grid->cells[grid_x][grid_y - 1];

                if (center->type != FLUID)
                {
                    continue;
                }
                // float s = grid->cells[x][y].s;
                float sx0 = left->s;
                float sx1 = right->s;
                float sy0 = bottom->s;
                float sy1 = top->s;
                float s = sx0 + sx1 + sy0 + sy1;

                if (s == 0.0f)
                    continue;

                float div = right->u - center->u + top->v - center->v;

                if (grid->rest_density > 0.0f) // compensate drift
                {
                    float k = 1.0f;
                    float compression = center->density - grid->rest_density;
                    if (compression > 0.0f)
                    {
                        div = div - k * compression;
                    }
                }

                float p = -div / s;
                p *= overRelaxation;
                center->p += cp * p;
                center->u -= sx0 * p;
                right->u += sx1 * p;
                center->v -= sy0 * p;
                top->v += sy1 * p;
            }
        }
    }
}

void MAC_update(MacGrid_t *grid, float dt)
{
    MAC_integrateParticles(grid, dt, -9.81f);
    MAC_pushParticlesApart(grid, 10);
    MAC_handleParticleCollisions(grid, 0.0f, 0.0f, 1.0f);
    MAC_transferVelocities(grid, 1, 1.9f);
    MAC_updateParticleDensity(grid);
    MAC_solveIncompressibility(grid, 10, dt, 1.2f);
    MAC_transferVelocities(grid, 0, 0.95f);
}

void MAC_transformGridToVerticies(MacGrid_t *grid, Vertex_t *vertices, GLuint *indices)
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

    for (uint16_t x = 0; x < grid->size_x; ++x)
    {
        for (uint16_t y = 0; y < grid->size_y; ++y)
        {
            glm::vec3 cubePos = glm::vec3(x, y, 0) * grid->cell_size;
            for (int i = 0; i < 8; i++)
            {
                vertices[vert_index].position = cubePos + cubeVertices[i] * grid->cell_size;
                vertices[vert_index].color = glm::vec3(1.0f, 1.0f, 1.0f);
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
}

void MAC_destroy(MacGrid_t *grid)
{
    for (uint16_t x = 0; x < grid->size_x; ++x)
    {
        free(grid->cells[x]);
    }
    free(grid->cells);
    free(grid->markers);
}