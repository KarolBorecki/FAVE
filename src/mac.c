#include "mac.h"
int getMarkerCellIndex(MacGrid_t *grid, Marker_t &marker)
{
    float xu = clamp(floorf(marker.position.x), 0.0f, (float)(grid->size_x - 1));
    float yu = clamp(floorf(marker.position.y), 0.0f, (float)(grid->size_y - 1));
    return xu * grid->size_y + yu;
}

GridCell_t &getMarkerCell(MacGrid_t *grid, Marker_t &marker)
{
    return grid->cells[getMarkerCellIndex(grid, marker)];
}

GridCell_t &getCell(MacGrid_t *grid, int x, int y)
{
    return grid->cells[x * grid->size_y + y];
}
void MAC_init(MacGrid_t *grid, float density, int size_x, int size_y, float cell_size)
{
    grid->density = density;
    grid->size_x = size_x;
    grid->size_y = size_y;
    grid->total_size = size_x * size_y;
    grid->num_markers = 40;
    grid->cell_size = cell_size;
    grid->inv_cell_size = 1.0f / cell_size;
    grid->marker_radius = 0.1f;
    grid->marker_inv_spacing = 1.0f / (2.2f * grid->marker_radius);
    grid->markers_rest_density = 0.0f;

    grid->cells = (GridCell_t *)malloc(grid->total_size * sizeof(GridCell_t));
    if (!grid->cells)
    {
        fprintf(stderr, "Failed to allocate memory for grid cells\n");
        exit(EXIT_FAILURE);
    }

    for (int cellIndex = 0; cellIndex < grid->total_size; ++cellIndex)
    {
        grid->cells[cellIndex].p = 0.0f;
        grid->cells[cellIndex].v = 0.0f;
        grid->cells[cellIndex].u = 0.0f;
        grid->cells[cellIndex].w = 0.0f;
        grid->cells[cellIndex].dv = 0.0f;
        grid->cells[cellIndex].du = 0.0f;
        grid->cells[cellIndex].dw = 0.0f;
        grid->cells[cellIndex].prevv = 0.0f;
        grid->cells[cellIndex].prevu = 0.0f;
        grid->cells[cellIndex].prevw = 0.0f;
        grid->cells[cellIndex].s = 1.0f;
        grid->cells[cellIndex].density = 0.0f;
    }

    for (int x = 0; x < grid->size_x; x++)
    {
        for (int y = 0; y < grid->size_y; y++)
        {
            if (x == 0 || x == grid->size_x - 1 || y == 0)
            {
                getCell(grid, x, y).s = 0.0f; // solid
            }
            else
            {
                getCell(grid, x, y).s = 1.0f; // fluid
            }
        }
    }

    grid->markers = (Marker_t *)malloc(grid->num_markers * sizeof(Marker_t));
    if (!grid->markers)
    {
        fprintf(stderr, "Failed to allocate memory for markers\n");
        exit(EXIT_FAILURE);
    }
    for (int marker_index = 0; marker_index < grid->num_markers; ++marker_index)
    {
        grid->markers[marker_index].position.x = ((float)rand() / RAND_MAX) * size_x * cell_size;
        grid->markers[marker_index].position.y = ((float)rand() / RAND_MAX) * size_y * cell_size;
        grid->markers[marker_index].position.z = 0.5f;

        grid->markers[marker_index].velocity.x = 0.0f;
        grid->markers[marker_index].velocity.y = 0.0f;
        grid->markers[marker_index].velocity.z = 0.0f;

        grid->markers[marker_index].color.x = 1.0f;
        grid->markers[marker_index].color.y = 0.0f;
        grid->markers[marker_index].color.z = 0.0f;
        grid->markers[marker_index].color.w = 1.0f;
    }

    grid->num_cell_markers = (int *)malloc(grid->total_size * sizeof(int));
    if (!grid->num_cell_markers)
    {
        fprintf(stderr, "Failed to allocate memory for num_cell_markers\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < grid->total_size; ++i)
    {

        grid->num_cell_markers[i] = 0;
    }

    grid->first_cell_marker = (int *)malloc((grid->total_size + 1) * sizeof(int));
    if (!grid->first_cell_marker)
    {
        fprintf(stderr, "Failed to allocate memory for first_cell_marker\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < grid->total_size + 1; ++i)
    {

        grid->first_cell_marker[i] = 0;
    }

    grid->cell_marker_ids = (int *)malloc(grid->num_markers * sizeof(int));
    if (!grid->cell_marker_ids)
    {
        fprintf(stderr, "Failed to allocate memory for cell_marker_ids\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < grid->num_markers; ++i)
    {
        grid->cell_marker_ids[i] = 0;
    }
}

void MAC_handleObstacle(MacGrid_t *grid, Obstacle_t *obstacle, float dt)
{
    float h = 1.0f / grid->inv_cell_size;
    float r = grid->marker_radius;
    float or2 = obstacle->radius * obstacle->radius;
    float minDist = obstacle->radius + r;
    float minDist2 = minDist * minDist;

    float minX = h + r;
    float maxX = (grid->size_x - 1) * h - r;
    float minY = h + r;
    float maxY = (grid->size_y - 1) * h - r;

    for (int marker_index = 0; marker_index < grid->num_markers; marker_index++)
    {
        Marker_t &marker = grid->markers[marker_index];

        float dx = marker.position.x - obstacle->position.x;
        float dy = marker.position.y - obstacle->position.y;
        float d2 = dx * dx + dy * dy;

        if (d2 < minDist2)
        {
            marker.velocity = (obstacle->position - obstacle->last_position) * dt;
        }

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
    for (int marker_index = 0; marker_index < grid->num_markers; marker_index++)
    {
        grid->markers[marker_index].position.x += grid->markers[marker_index].velocity.x * dt;
        grid->markers[marker_index].position.y += grid->markers[marker_index].velocity.y * dt;
    }
}

void MAC_pushParticlesApart(MacGrid_t *grid, int numIters)
{
    for (int i = 0; i < grid->total_size; i++)
    {
        grid->num_cell_markers[i] = 0;
    }

    for (int marker_index = 0; marker_index < grid->num_markers; marker_index++)
    {
        Marker_t &marker = grid->markers[marker_index];
        int cellIndex = getMarkerCellIndex(grid, marker);
        grid->num_cell_markers[cellIndex]++;
    }

    int first = 0;
    for (int i = 0; i < grid->total_size; i++)
    {
        first += grid->num_cell_markers[i];
        grid->first_cell_marker[i] = first;
    }
    grid->first_cell_marker[grid->total_size] = grid->num_markers;

    for (int marker_index = 0; marker_index < grid->num_markers; marker_index++)
    {
        Marker_t &marker = grid->markers[marker_index];
        int cellIndex = getMarkerCellIndex(grid, marker);
        grid->first_cell_marker[cellIndex]--;
        grid->cell_marker_ids[grid->first_cell_marker[cellIndex]] = marker_index;
    }

    float minDist = grid->marker_radius * 2.0f;
    float minDist2 = minDist * minDist;

    for (int iter = 0; iter < numIters; iter++)
    {
        for (int marker_index = 0; marker_index < grid->num_markers; marker_index++)
        {
            float px = grid->markers[marker_index].position.x;
            float py = grid->markers[marker_index].position.y;

            float pxi = floorf(px * grid->marker_inv_spacing);
            float pyi = floorf(py * grid->marker_inv_spacing);
            int x0 = MAX(pxi - 1, 0);
            int y0 = MAX(pyi - 1, 0);
            int x1 = MIN(pxi + 1, grid->size_x - 1);
            int y1 = MIN(pyi + 1, grid->size_y - 1);


            for (int xi = x0; xi <= x1; xi++)
            {
                for (int yi = y0; yi <= y1; yi++)
                {
                    int cellIndex = xi * grid->size_y + yi;

                    int first = grid->first_cell_marker[cellIndex];
                    int last = grid->first_cell_marker[cellIndex + 1];

                    for (int current_analyzed_marker = first; current_analyzed_marker < last; current_analyzed_marker++)
                    {
                        int other_marker_id = grid->cell_marker_ids[current_analyzed_marker];
                        if (other_marker_id == marker_index)
                            continue;

                        float qx = grid->markers[other_marker_id].position.x;
                        float qy = grid->markers[other_marker_id].position.y;

                        float dx = qx - px;
                        float dy = qy - py;
                        float d2 = dx * dx + dy * dy;

                        if (d2 > minDist2 || d2 == 0.0f)
                            continue;

                        float d = sqrtf(d2);
                        float s = 0.5f * (minDist - d) / d;
                        dx *= s;
                        dy *= s;
                        grid->markers[marker_index].position.x -= dx;
                        grid->markers[marker_index].position.y -= dy;
                        grid->markers[other_marker_id].position.x += dx;
                        grid->markers[other_marker_id].position.y += dy;
                    }
                }
            }
        }
    }
}

void MAC_transferVelocities(MacGrid_t *grid, int toGrid, float flipRatio)
{
    int n = grid->size_y;
    float h = grid->cell_size;
    float h1 = grid->inv_cell_size;
    float h2 = 0.5f * h;

    if (toGrid)
    {
        for (int cellIndex = 0; cellIndex < grid->total_size; cellIndex++)
        {
            grid->cells[cellIndex].prevu = grid->cells[cellIndex].u;
            grid->cells[cellIndex].prevv = grid->cells[cellIndex].v;
            grid->cells[cellIndex].du = 0.0f;
            grid->cells[cellIndex].dv = 0.0f;
            grid->cells[cellIndex].u = 0.0f;
            grid->cells[cellIndex].v = 0.0f;
            grid->cells[cellIndex].type = grid->cells[cellIndex].s == 0.0f ? SOLID : AIR;
        }

        for (int marker_index = 0; marker_index < grid->num_markers; marker_index++)
        {
            int cellIndex = getMarkerCellIndex(grid, grid->markers[marker_index]);
            if (grid->cells[cellIndex].type == AIR)
                grid->cells[cellIndex].type = FLUID;
        }
    }

    for (int component = 0; component < 2; component++) // 0 === u, 1 === v
    {
        float dx = component == 0 ? 0.0f : h2;
        float dy = component == 0 ? h2 : 0.0f;

        for (int marker_index = 0; marker_index < grid->num_markers; marker_index++)
        {
            float x = clampf(grid->markers[marker_index].position.x, h, (grid->size_x - 1) * h);
            float y = clampf(grid->markers[marker_index].position.y, h, (grid->size_y - 1) * h);

            int x0 = MIN(floorf((x - dx) * h1), grid->size_x - 2);
            float tx = (x - dx - x0 * h) * h1;
            int x1 = MIN(x0 + 1, grid->size_x - 2);

            int y0 = MIN(floorf((y - dy) * h1), grid->size_y - 2);
            float ty = (y - dy - y0 * h) * h1;
            int y1 = MIN(y0 + 1, grid->size_y - 2);

            float sx = 1.0f - tx;
            float sy = 1.0f - ty;

            float d0 = sx * sy;
            float d1 = tx * sy;
            float d2 = tx * ty;
            float d3 = sx * ty;

            int cellIndex0 = x0 * n + y0;
            int cellIndex1 = x1 * n + y0;
            int cellIndex2 = x1 * n + y1;
            int cellIndex3 = x0 * n + y1;

            if (toGrid)
            {
                float pv = grid->markers[marker_index].velocity[component];
                if (component == 0)
                {
                    grid->cells[cellIndex0].u += d0 * pv;
                    grid->cells[cellIndex1].u += d1 * pv;
                    grid->cells[cellIndex2].u += d2 * pv;
                    grid->cells[cellIndex3].u += d3 * pv;

                    grid->cells[cellIndex0].du += d0;
                    grid->cells[cellIndex1].du += d1;
                    grid->cells[cellIndex2].du += d2;
                    grid->cells[cellIndex3].du += d3;
                }
                else
                {
                    grid->cells[cellIndex0].v += d0 * pv;
                    grid->cells[cellIndex1].v += d1 * pv;
                    grid->cells[cellIndex2].v += d2 * pv;
                    grid->cells[cellIndex3].v += d3 * pv;

                    grid->cells[cellIndex0].dv += d0;
                    grid->cells[cellIndex1].dv += d1;
                    grid->cells[cellIndex2].dv += d2;
                    grid->cells[cellIndex3].dv += d3;
                }
            }
            else
            {
                float valid0, valid1, valid2, valid3;
                int offset = component == 0 ? n : 1;
                valid0 = grid->cells[cellIndex0].type != AIR || grid->cells[cellIndex0 - offset].type != AIR ? 1.0f : 0.0f;
                valid1 = grid->cells[cellIndex1].type != AIR || grid->cells[cellIndex1 - offset].type != AIR ? 1.0f : 0.0f;
                valid2 = grid->cells[cellIndex2].type != AIR || grid->cells[cellIndex2 - offset].type != AIR ? 1.0f : 0.0f;
                valid3 = grid->cells[cellIndex3].type != AIR || grid->cells[cellIndex3 - offset].type != AIR ? 1.0f : 0.0f;

                float v = grid->markers[marker_index].velocity[component];
                float d = valid0 * d0 + valid1 * d1 + valid2 * d2 + valid3 * d3;
                if (d > 0.0f)
                {
                    if (component == 0)
                    {
                        float picV = (valid0 * d0 * grid->cells[cellIndex0].u + valid1 * d1 * grid->cells[cellIndex1].u + valid2 * d2 * grid->cells[cellIndex2].u + valid3 * d3 * grid->cells[cellIndex3].u) / d;
                        float corr = (valid0 * d0 * (grid->cells[cellIndex0].u - grid->cells[cellIndex0].prevu) + valid1 * d1 * (grid->cells[cellIndex1].u - grid->cells[cellIndex1].prevu) + valid2 * d2 * (grid->cells[cellIndex2].u - grid->cells[cellIndex2].prevu) + valid3 * d3 * (grid->cells[cellIndex3].u - grid->cells[cellIndex3].prevu)) / d;
                        float flipV = v + corr;
                        grid->markers[marker_index].velocity[component] = flipRatio * flipV + (1.0f - flipRatio) * picV;
                    }
                    else if (component == 1)
                    {

                        float picV = (valid0 * d0 * grid->cells[cellIndex0].v + valid1 * d1 * grid->cells[cellIndex1].v + valid2 * d2 * grid->cells[cellIndex2].v + valid3 * d3 * grid->cells[cellIndex3].v) / d;
                        float corr = (valid0 * d0 * (grid->cells[cellIndex0].v - grid->cells[cellIndex0].prevv) + valid1 * d1 * (grid->cells[cellIndex1].v - grid->cells[cellIndex1].prevv) + valid2 * d2 * (grid->cells[cellIndex2].v - grid->cells[cellIndex2].prevv) + valid3 * d3 * (grid->cells[cellIndex3].v - grid->cells[cellIndex3].prevv)) / d;
                        float flipV = v + corr;
                        grid->markers[marker_index].velocity[component] = flipRatio * flipV + (1.0f - flipRatio) * picV;
                    }
                }
            }

            if (toGrid)
            {
                if (component == 0)
                {
                    for (int cellIndex = 0; cellIndex < grid->total_size; cellIndex++)
                    {

                        if (grid->cells[cellIndex].du > 0.0f)
                        {
                            grid->cells[cellIndex].u /= grid->cells[cellIndex].du;
                        }
                    }
                }
                else if (component == 1)
                {
                    for (int cellIndex = 0; cellIndex < grid->total_size; cellIndex++)
                    {
                        if (grid->cells[cellIndex].dv > 0.0f)
                        {
                            grid->cells[cellIndex].v /= grid->cells[cellIndex].dv;
                        }
                    }
                }

                for (int grix_x = 0; grix_x < grid->size_x; grix_x++)
                {
                    for (int grix_y = 0; grix_y < grid->size_y; grix_y++)
                    {
                        int cellIndex = grix_x * n + grix_y;
                        int is_solid = grid->cells[cellIndex].type == SOLID;
                        if (is_solid && grid->cells[(grix_x - 1) * n + grix_y].type == SOLID)
                        {
                            grid->cells[cellIndex].u = grid->cells[grix_x * n + grix_y].prevu;
                        }
                        if (is_solid && grid->cells[grix_x * n + (grix_y - 1)].type == SOLID)
                        {
                            grid->cells[cellIndex].v = grid->cells[grix_x * n + grix_y].prevv;
                        }
                    }
                }
            }
        }
    }
}

void MAC_updateParticleDensity(MacGrid_t *grid)
{
    int marker0CellIndex = getMarkerCellIndex(grid, grid->markers[0]);
    int n = grid->size_y;
    float h = grid->cell_size;
    float h1 = grid->inv_cell_size;
    float h2 = 0.5f * h;

    for (int cellIndex = 0; cellIndex < grid->total_size; cellIndex++)
    {
        grid->cells[cellIndex].density = 0.0f;
    }

    for (int marker_index = 0; marker_index < grid->num_markers; marker_index++)
    {
        float x = grid->markers[marker_index].position.x;
        float y = grid->markers[marker_index].position.y;

        int x0 = (int)MAX(0, MIN(grid->size_x - 1, floorf((x - h2) * h1)));
        int x1 = (int)MAX(0, MIN(grid->size_x - 1, x0 + 1));

        int y0 = (int)MAX(0, MIN(grid->size_y - 1, floorf((y - h2) * h1)));
        int y1 = (int)MAX(0, MIN(grid->size_y - 1, y0 + 1));

        float tx = ((x - h2) - x0 * h) * h1;
        float ty = ((y - h2) - y0 * h) * h1;

        float sx = 1.0f - tx;
        float sy = 1.0f - ty;

        if (x0 >= 0 && x0 < grid->size_x && y0 >= 0 && y0 < grid->size_y)
            grid->cells[x0 * n + y0].density += sx * sy;

        if (x1 >= 0 && x1 < grid->size_x && y0 >= 0 && y0 < grid->size_y)
            grid->cells[x1 * n + y0].density += tx * sy;

        if (x1 >= 0 && x1 < grid->size_x && y1 >= 0 && y1 < grid->size_y)
            grid->cells[x1 * n + y1].density += tx * ty;

        if (x0 >= 0 && x0 < grid->size_x && y1 >= 0 && y1 < grid->size_y)
            grid->cells[x0 * n + y1].density += sx * ty;

        if (grid->markers_rest_density == 0.0f)
        {
            float sum = 0.0f;
            int num_cells = 0;
            for (int cellIndex = 0; cellIndex < grid->total_size; cellIndex++)
            {
                if (grid->cells[cellIndex].type == FLUID)
                {
                    sum += grid->cells[cellIndex].density;
                    num_cells++;
                }
            }
            if (num_cells > 0)
            {
                grid->markers_rest_density = sum / num_cells;
            }
        }
    }
}

void MAC_solveIncompressibility(MacGrid_t *grid, int numIters, float dt, float overRelaxation)
{
    for (int cellIndex = 0; cellIndex < grid->total_size; cellIndex++)
    {
        grid->cells[cellIndex].p = 0.0f;
        grid->cells[cellIndex].prevu = grid->cells[cellIndex].u;
        grid->cells[cellIndex].prevv = grid->cells[cellIndex].v;
    }

    int n = grid->size_y;
    float cp = grid->density * grid->cell_size / dt;

    for (int iter = 0; iter < numIters; iter++)
    {
        for (int grid_x = 1; grid_x < grid->size_x - 1; grid_x++)
        {
            for (int grid_y = 1; grid_y < grid->size_y - 1; grid_y++)
            {
                GridCell_t *center = &grid->cells[grid_x * n + grid_y];

                if (center->type != FLUID)
                {
                    continue;
                }

                GridCell_t *right = &grid->cells[(grid_x + 1) * n + grid_y];
                GridCell_t *left = &grid->cells[(grid_x - 1) * n + grid_y];
                GridCell_t *top = &grid->cells[grid_x * n + (grid_y + 1)];
                GridCell_t *bottom = &grid->cells[grid_x * n + (grid_y - 1)];

                float sx0 = left->s;
                float sx1 = right->s;
                float sy0 = bottom->s;
                float sy1 = top->s;
                float s = sx0 + sx1 + sy0 + sy1;

                if (s == 0.0f)
                    continue;

                float div = (right->u - center->u) + (top->v - center->v);

                // if (grid->markers_rest_density > 0.0f)
                // {
                //     float k = 1.0f;
                //     float compression = center->density - grid->markers_rest_density;
                //     if (compression > 0.0f)
                //     {
                //         div -= k * compression;
                //     }
                // }

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
    MAC_pushParticlesApart(grid, 2);
    MAC_transferVelocities(grid, 1, 0.9f);
    MAC_updateParticleDensity(grid);
    MAC_solveIncompressibility(grid, 100, dt, 1.9f);
    MAC_transferVelocities(grid, 0, 0.9f);
}

Pair_t MAC_transformGridToVerticies(MacGrid_t *grid, Vertex_t *vertices, GLuint *indices)
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

    for (int x = 0; x < grid->size_x; ++x)
    {
        for (int y = 0; y < grid->size_y; ++y)
        {
            int cellIndex = x * grid->size_y + y;
            glm::vec3 cubePos = glm::vec3(x, y, 0) * grid->cell_size;
            for (int i = 0; i < 8; i++)
            {
                vertices[vert_index].position = cubePos + cubeVertices[i] * grid->cell_size;
                if (grid->cells[cellIndex].type == FLUID)
                {
                    vertices[vert_index].color = glm::vec3(0.15f, 0.4f, 0.99f);
                }
                else if (grid->cells[cellIndex].type == SOLID)
                {
                    vertices[vert_index].color = glm::vec3(1.0f, 1.0f, 1.0f);
                }
                else if (grid->cells[cellIndex].type == AIR)
                {
                    vertices[vert_index].color = glm::vec3(0.53f, 0.81f, 0.94f);
                }
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
    return {.first = vert_index, .second = ind_index};
}

#define SPHERE_LAT_SLICES 10
#define SPHERE_LON_SLICES 10

Pair_t MAC_transformMarkersToVertices(MacGrid_t *grid, Vertex_t *markerVertices, GLuint *markerIndices)
{
    int vertexOffset = 0;
    int indexOffset = 0;

    for (int m = 0; m < grid->num_markers; m++)
    {
        glm::vec3 markerPos = grid->markers[m].position;
        glm::vec3 markerColor = grid->markers[m].color;
        float radius = grid->marker_radius;

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

                glm::vec3 vertexPos = {
                    markerPos.x + radius * sinTheta * cosPhi,
                    markerPos.y + radius * cosTheta,
                    markerPos.z + radius * sinTheta * sinPhi};

                markerVertices[vertexOffset].position = vertexPos;
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

void MAC_destroy(MacGrid_t *grid)
{
    if (grid->cells)
    {
        free(grid->cells);
        grid->cells = NULL;
    }
    if (grid->markers)
    {
        free(grid->markers);
        grid->markers = NULL;
    }
    if (grid->num_cell_markers)
    {
        free(grid->num_cell_markers);
        grid->num_cell_markers = NULL;
    }
    if (grid->first_cell_marker)
    {
        free(grid->first_cell_marker);
        grid->first_cell_marker = NULL;
    }
    if (grid->cell_marker_ids)
    {
        free(grid->cell_marker_ids);
        grid->cell_marker_ids = NULL;
    }
}
