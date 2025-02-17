#include "mac.h"
int getMarkerCellIndex(MacGrid_t *grid, Marker_t &marker)
{
    // printf("marker.position.x: %f, marker.position.y: %f\n", marker.position.x, marker.position.y);
    // printf("marker grid x: %d, marker grid d: %d\n", (int)clamp((int)floorf(marker.position.x * grid->inv_cell_size), 0, grid->size_x - 1), (int)clamp((int)floorf(marker.position.y * grid->inv_cell_size), 0, grid->size_y - 1));
    return clamp((int)floorf(marker.position.x * grid->inv_cell_size), 0, grid->size_x - 1) * grid->size_y + clamp((int)floorf(marker.position.y * grid->inv_cell_size), 0, grid->size_y - 1);
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
    grid->num_markers = grid->total_size * 4;
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
    for (int i = 0; i < grid->num_markers; ++i)
    {
        grid->markers[i].position.x = fmodf(i * 1.724356f, size_x * cell_size);
        grid->markers[i].position.y = fmodf(i * 1.863312f, size_y * cell_size);
        if (i == 0)
        {
            grid->markers[i].position.x = 3.5f;
            grid->markers[i].position.y = 3.5f;
        }
        grid->markers[i].position.z = 0.0f;

        grid->markers[i].velocity.x = 0.0f;
        grid->markers[i].velocity.y = 0.0f;
        grid->markers[i].velocity.z = 0.0f;

        grid->markers[i].color.x = 0.67f;
        grid->markers[i].color.y = 0.3f;
        grid->markers[i].color.z = 1.0f;
        grid->markers[i].color.w = 1.0f;

        int cellIndex = getMarkerCellIndex(grid, grid->markers[i]);
        // grid->cells[cellIndex].type = FLUID;
        // grid->cells[cellIndex].s = 1.0f;
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

    for (int cellIndex = 0; cellIndex < grid->total_size; cellIndex++)
    {
        if (isnan(grid->cells[cellIndex].v) || isnan(grid->cells[cellIndex].u))
        {
            printf("[MAC_init] NaN detected in div at [%d] -> u: %f, v: %f\n", cellIndex, grid->cells[cellIndex].u, grid->cells[cellIndex].v);
        }
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

    for (int i = 0; i < grid->num_markers; i++)
    {
        Marker_t &marker = grid->markers[i];

        float dx = marker.position.x - obstacle->position.x;
        float dy = marker.position.y - obstacle->position.y;
        float d2 = dx * dx + dy * dy;

        if (d2 < minDist2)
        {
            printf("d2: %f, minDist2: %f\n", d2, minDist2);
            marker.velocity = obstacle->velocity;
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
    printf("\n______MAC_integrateParticles\n");
    for (int marker_index = 0; marker_index < grid->num_markers; marker_index++)
    {
        // grid->markers[marker_index].velocity.y += dt * gravity; WTF is this? why causing NaNs?
        grid->markers[marker_index].position.x += grid->markers[marker_index].velocity.x * dt;
        grid->markers[marker_index].position.y += grid->markers[marker_index].velocity.y * dt;
    }
    for (int cellIndex = 0; cellIndex < grid->total_size; cellIndex++)
    {
        if (isnan(grid->cells[cellIndex].v) || isnan(grid->cells[cellIndex].u))
        {
            printf("[MAC_integrateParticles] NaN detected in div at [%d] -> u: %f, v: %f\n", cellIndex, grid->cells[cellIndex].u, grid->cells[cellIndex].v);
        }
    }
}

void MAC_pushParticlesApart(MacGrid_t *grid, int numIters)
{
    printf("\n______MAC_pushParticlesApart\n");
    float colorDiffusionCoeff = 0.001f;

    for (int i = 0; i < grid->total_size; i++)
    {
        grid->num_cell_markers[i] = 0;
    }

    // KROK 1 zliczenie ile markerów jest w każdej komórce
    // printf("BUILDING num_cell_markers\n");
    for (int marker_index = 0; marker_index < grid->num_markers; marker_index++)
    {
        Marker_t &marker = grid->markers[marker_index];
        int cellIndex = getMarkerCellIndex(grid, marker);
        grid->num_cell_markers[cellIndex]++;
        // printf("cellIndexes.x: %d, cellIndexes.y: %d\n", cellIndexes.x, cellIndexes.y);
        // printf("grid->num_cell_markers[%d]: %d\n", cellIndex, grid->num_cell_markers[cellIndex]);
    }

    // KROK 2
    int first = 0;
    // printf("BUILDING  \n");
    for (int i = 0; i < grid->total_size; i++)
    {
        first += grid->num_cell_markers[i];
        grid->first_cell_marker[i] = first;
        // printf("grid->first_cell_marker[%d]: %d\n", i, grid->first_cell_marker[i]);
    }
    grid->first_cell_marker[grid->total_size] = grid->num_markers;

    // printf("BUILDING cell_marker_ids\n");
    for (int marker_index = 0; marker_index < grid->num_markers; marker_index++)
    {
        Marker_t &marker = grid->markers[marker_index];
        int cellIndex = getMarkerCellIndex(grid, marker);
        grid->first_cell_marker[cellIndex]--;
        if (grid->first_cell_marker[cellIndex] >= grid->num_markers || grid->first_cell_marker[cellIndex] < 0)
        {
            printf("!!! ERROR !!! grid->first_cell_marker[%d]: %d\n", cellIndex, grid->first_cell_marker[cellIndex]);
        }
        grid->cell_marker_ids[grid->first_cell_marker[cellIndex]] = marker_index;
        // printf("grid->first_cell_marker[%d]: %d for marker index = %d\n", grid->first_cell_marker[cellIndex], grid->cell_marker_ids[grid->first_cell_marker[cellIndex]], marker_index);
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

            // printf("analysing marker nr %d x1 = %d   y1 = %d     x0 = %d  y0 = %d\n", marker_index, x1, y1, x0, y0);

            for (int xi = x0; xi <= x1; xi++)
            {
                for (int yi = y0; yi <= y1; yi++)
                {
                    int cellIndex = xi * grid->size_y + yi;
                    int first = grid->first_cell_marker[cellIndex];
                    int last = grid->first_cell_marker[cellIndex + 1];
                    // printf("cellIndex: %d, first: %d, last: %d\n", cellIndex, first, last);

                    for (int cell_marker_index = first; cell_marker_index < last; cell_marker_index++)
                    {
                        int other_marker_id = grid->cell_marker_ids[cell_marker_index];
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

                        // int other_marker_index = grid->cell_marker_ids[cell_marker_index];
                        // if (other_marker_index == marker_index)
                        //     continue;

                        // Marker_t &other_marker = grid->markers[other_marker_index];
                        // float dx = other_marker.position.x - px;
                        // float dy = other_marker.position.y - py;
                        // float d2 = dx * dx + dy * dy;

                        // if (d2 < minDist2)
                        // {
                        //     float d = sqrtf(d2);
                        //     if (d > 0.0f)
                        //     {
                        //         float s = (minDist - d) / d;
                        //         float sx = s * dx;
                        //         float sy = s * dy;

                        //         grid->markers[marker_index].position.x -= 0.5f * sx;
                        //         grid->markers[marker_index].position.y -= 0.5f * sy;
                        //         grid->markers[other_marker_index].position.x += 0.5f * sx;
                        //         grid->markers[other_marker_index].position.y += 0.5f * sy;
                        //     }
                        // }
                    }
                }
            }
        }
    }
    for (int cellIndex = 0; cellIndex < grid->total_size; cellIndex++)
    {
        if (isnan(grid->cells[cellIndex].v) || isnan(grid->cells[cellIndex].u))
        {
            printf("[MAC_pushParticlesApart] NaN detected in div at [%d] -> u: %f, v: %f\n", cellIndex, grid->cells[cellIndex].u, grid->cells[cellIndex].v);
        }
    }
}

void MAC_transferVelocities(MacGrid_t *grid, int toGrid, float flipRatio)
{
    printf("\n______MAC_transferVelocities %s\n", toGrid ? "TO GRID" : "FROM GRID");
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
            if (grid->cells[cellIndex].type != SOLID)
                grid->cells[cellIndex].type = FLUID;
        }
    }

    for (int component = 0; component < 2; component++)
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

            int cellIndex0 = x0 + n * y0;
            int cellIndex1 = x1 + n * y0;
            int cellIndex2 = x1 + n * y1;
            int cellIndex3 = x0 + n * y1;

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
    for (int cellIndex = 0; cellIndex < grid->total_size; cellIndex++)
    {
        if (isnan(grid->cells[cellIndex].v) || isnan(grid->cells[cellIndex].u))
        {
            printf("[MAC_transferVelocities] NaN detected in div at [%d] -> u: %f, v: %f\n", cellIndex, grid->cells[cellIndex].u, grid->cells[cellIndex].v);
        }
    }
}

void MAC_updateParticleDensity(MacGrid_t *grid)
{
    printf("\n______MAC_updateParticleDensity\n");
    int marker0CellIndex = getMarkerCellIndex(grid, grid->markers[0]);
    int n = grid->size_y;
    float h = grid->cell_size;
    float h1 = grid->inv_cell_size;
    float h2 = 0.5f * h;

    for (int cellIndex = 0; cellIndex < grid->total_size; cellIndex++)
    {
        grid->cells[cellIndex].density = 0.0f;
        if (isnan(grid->cells[cellIndex].v) || isnan(grid->cells[cellIndex].u))
        {
            printf("[MAC_updateParticleDensity] 1 NaN detected in div at [%d] -> u: %f, v: %f\n", cellIndex, grid->cells[cellIndex].u, grid->cells[cellIndex].v);
        }
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

        // printf("REST DENSITY UPDATED\n");
        // printf("grid->markers_rest_density: %f\n", grid->markers_rest_density);
    }
    // printf("DONE!\n");
    for (int cellIndex = 0; cellIndex < grid->total_size; cellIndex++)
    {
        if (isnan(grid->cells[cellIndex].v) || isnan(grid->cells[cellIndex].u))
        {
            printf("[MAC_transferVelocities] 2 NaN detected in div at [%d] -> u: %f, v: %f\n", cellIndex, grid->cells[cellIndex].u, grid->cells[cellIndex].v);
        }
    }
}

void MAC_solveIncompressibility(MacGrid_t *grid, int numIters, float dt, float overRelaxation)
{
    int marker0CellIndex = getMarkerCellIndex(grid, grid->markers[0]);
    printf("\n______MAC_solveIncompressibility\n");
    printf("marker0CellIndex: %d\n", marker0CellIndex);
    for (int cellIndex = 0; cellIndex < grid->total_size; cellIndex++)
    {
        grid->cells[cellIndex].p = 0.0f;
        grid->cells[cellIndex].prevu = grid->cells[cellIndex].u;
        grid->cells[cellIndex].prevv = grid->cells[cellIndex].v;

        if (isnan(grid->cells[cellIndex].v) || isnan(grid->cells[cellIndex].u))
        {
            printf("[MAC_solveIncompressibility] 1 NaN detected in CELL at [%d] -> u: %f, v: %f\n", cellIndex, grid->cells[cellIndex].u, grid->cells[cellIndex].v);
        }
    }

    for (int cellIndex = 0; cellIndex < grid->total_size; cellIndex++)
    {
        if (isnan(grid->cells[cellIndex].v) || isnan(grid->cells[cellIndex].u))
        {
            printf("[MAC_solveIncompressibility] 1 NaN detected in CELL at [%d] -> u: %f, v: %f\n", cellIndex, grid->cells[cellIndex].u, grid->cells[cellIndex].v);
        }
    }

    int n = grid->size_y;
    float cp = grid->density * grid->cell_size / dt;

    for (int iter = 0; iter < numIters; iter++)
    {
        printf("ITER: %d\n", iter);
        for (int grid_x = 1; grid_x < grid->size_x - 1; grid_x++)
        {
            for (int grid_y = 1; grid_y < grid->size_y - 1; grid_y++)
            {
                if (isnan(grid->cells[grid_x * n + grid_y].v) || isnan(grid->cells[grid_x * n + grid_y].u))
                {
                    printf("[MAC_solveIncompressibility] NaN detected in div at [%d, %d] -> center->u: %f center->v: %f\n",
                           grid_x, grid_y, grid->cells[grid_x * n + grid_y].u, grid->cells[grid_x * n + grid_y].v);
                }
                GridCell_t *center = &grid->cells[grid_x * n + grid_y];
                GridCell_t *right = &grid->cells[(grid_x + 1) * n + grid_y];
                GridCell_t *left = &grid->cells[(grid_x - 1) * n + grid_y];
                GridCell_t *top = &grid->cells[grid_x * n + (grid_y + 1)];
                GridCell_t *bottom = &grid->cells[grid_x * n + (grid_y - 1)];

                if (center->type != FLUID)
                {
                    continue;
                }
                // float s = grid->cells[x][y].s;
                if (center->s == 0.0f)
                    continue;

                float sx0 = left->s;
                float sx1 = right->s;
                float sy0 = bottom->s;
                float sy1 = top->s;
                float s = sx0 + sx1 + sy0 + sy1;
                if (isnan(s))
                {
                    printf("[MAC_solveIncompressibility] NaN detected in s at [%d, %d]\n", grid_x, grid_y);
                }

                if (s == 0.0f)
                    continue;

                float div = (right->u - center->u) + (top->v - center->v);
                if (isnan(div))
                {
                    printf("[MAC_solveIncompressibility] NaN detected in div at [%d, %d] -> right->u: %f, center->u: %f, top->v: %f, center->v: %f\n",
                           grid_x, grid_y, right->u, center->u, top->v, center->v);
                }

                // if (grid->markers_rest_density > 0.0f) // compensate drift
                // {
                //     float k = 1.0f;
                //     float compression = center->density - grid->markers_rest_density;
                //     if (compression > 0.0f)
                //     {
                //         div = div - k * compression;
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
    MAC_pushParticlesApart(grid, 3);
    MAC_transferVelocities(grid, 1, 1.9f);
    MAC_updateParticleDensity(grid);
    MAC_solveIncompressibility(grid, 50, dt, 1.9f);
    MAC_transferVelocities(grid, 0, 0.95f);
    printf("---------------------------------------------------------------------\n\n");
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
                if (vert_index > VERTICIES_SIZE)
                {
                    printf("!!! ERROR !!! vert_index: %d\n", vert_index);
                    exit(EXIT_FAILURE);
                }
            }

            size_t offset = vert_index - 8;
            for (int i = 0; i < 36; i++)
            {
                indices[ind_index++] = offset + cubeIndices[i];
                if (ind_index > INDICIES_SIZE)
                {
                    printf("!!! ERROR !!! ind_index: %d\n", ind_index);
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    return {.first = vert_index, .second = ind_index};
}

Pair_t MAC_transformMarkersToVertices(MacGrid_t *grid, Vertex_t *markerVertices, GLuint *markerIndices)
{
    for (int i = 0; i < grid->num_markers; i++)
    {
        markerVertices[i].position = grid->markers[i].position;
        markerVertices[i].color = grid->markers[i].color;
        markerIndices[i] = i;
    }
    return {.first = grid->num_markers, .second = grid->num_markers};
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
