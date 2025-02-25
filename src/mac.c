#include "mac.h"

#define RED_TEXT "\033[31m" // ANSI escape sequence for red
#define RESET_TEXT "\033[0m"

void printMarkers(MacGrid_t *grid)
{
    printf("\nMarkers visualization:\n");
    for (int marker_index = 0; marker_index < grid->num_markers; marker_index++)
    {
        printf("marker[%d]  p(%.1f; %.1f; %.1f)   v(%.3f; %.3f; %.3f\n", marker_index, grid->markers[marker_index].position.x, grid->markers[marker_index].position.y, grid->markers[marker_index].position.z, grid->markers[marker_index].velocity.x, grid->markers[marker_index].velocity.y, grid->markers[marker_index].velocity.z);
    }
}
void printGrid(MacGrid_t *grid)
{
    printf("Grid visualization:\n");

    for (int grid_y = grid->size_y - 1; grid_y >= 0; grid_y--)
    {
        // Górna linia oddzielająca
        for (int grid_x = 0; grid_x < grid->size_x; grid_x++)
        {
            printf("+----------------------");
        }
        printf("\n");

        // Pusty wiersz dla separacji
        for (int grid_x = 0; grid_x < grid->size_x; grid_x++)
        {
            printf("|                      ");
        }
        printf("|\n");

        // Wartości `u` (prędkość w osi X) oraz `p` (ciśnienie)
        for (int grid_x = 0; grid_x < grid->size_x; grid_x++)
        {
            // if (isnan(grid->cells[grid_x * grid->size_y + grid_y].u) || isnan(grid->cells[grid_x * grid->size_y + grid_y].p))
            // {
            //     printf(RED_TEXT);
            //     printf("| %6.1f   %6.1f      ", grid->cells[grid_x * grid->size_y + grid_y].u,
            //            grid->cells[grid_x * grid->size_y + grid_y].p);
            //     printf(RESET_TEXT);
            // }
            // else if (grid->cells[grid_x * grid->size_y + grid_y].u > -9999.0 && grid->cells[grid_x * grid->size_y + grid_y].u < 9999.0 && grid->cells[grid_x * grid->size_y + grid_y].p < 9999.0 && grid->cells[grid_x * grid->size_y + grid_y].p > -1000.0)
            // {
            //     printf("| %6.1f   %6.1f      ", grid->cells[grid_x * grid->size_y + grid_y].u,
            //            grid->cells[grid_x * grid->size_y + grid_y].p);
            // }
            // else if (grid->cells[grid_x * grid->size_y + grid_y].u > -9999.0 && grid->cells[grid_x * grid->size_y + grid_y].u < 9999.0)
            // {
            //     printf("| %6.1f   TWN         ", grid->cells[grid_x * grid->size_y + grid_y].u);
            // }
            // else if (grid->cells[grid_x * grid->size_y + grid_y].p < 9999.0 && grid->cells[grid_x * grid->size_y + grid_y].p > -1000.0)
            // {
            //     printf("| TWN   %6.1f         ", grid->cells[grid_x * grid->size_y + grid_y].p);
            // }
            // else
            // {
            //     printf("| TWN   TWN            ");
            // }
            printf("| %6.1f   %6.1f      ", grid->cells[grid_x * grid->size_y + grid_y].u,
                   grid->cells[grid_x * grid->size_y + grid_y].p);
        }
        printf("|\n");

        // Pusty wiersz dla separacji
        for (int grid_x = 0; grid_x < grid->size_x; grid_x++)
        {
            printf("|                      ");
        }
        printf("|\n");

        // Wartości `v` (prędkość w osi Y)
        for (int grid_x = 0; grid_x < grid->size_x; grid_x++)
        {
            // if (isnan(grid->cells[grid_x * grid->size_y + grid_y].v))
            // {
            //     printf(RED_TEXT);
            //     printf("|        %6.1f        ", grid->cells[grid_x * grid->size_y + grid_y].v);
            //     printf(RESET_TEXT);
            // }
            // else if (grid->cells[grid_x * grid->size_y + grid_y].v > -9999.0 && grid->cells[grid_x * grid->size_y + grid_y].v < 9999.0)
            // {
            //     printf("|        %6.1f        ", grid->cells[grid_x * grid->size_y + grid_y].v);
            // }
            // else
            // {
            //     printf("|       TWN            ");
            // }
            printf("|        %6.1f        ", grid->cells[grid_x * grid->size_y + grid_y].v);
        }
        printf("|\n");

        // Dolna linia oddzielająca
        for (int grid_x = 0; grid_x < grid->size_x; grid_x++)
        {
            printf("+----------------------");
        }
        printf("\n");
    }
    printMarkers(grid);
}

int getMarkerCellIndex(MacGrid_t *grid, Marker_t &marker)
{
    int xu = (int)clamp(floorf(marker.position.x * grid->inv_cell_size), 0.0f, (float)(grid->size_x - 1));
    int yu = (int)clamp(floorf(marker.position.y * grid->inv_cell_size), 0.0f, (float)(grid->size_y - 1));
    return xu * grid->size_y + yu;
}

int getMarkerCellIndexForMarkerInvCellSize(MacGrid_t *grid, Marker_t &marker)
{
    int xu = (int)clamp(floorf(marker.position.x * grid->marker_inv_spacing), 0.0f, (float)(grid->size_x - 1));
    int yu = (int)clamp(floorf(marker.position.y * grid->marker_inv_spacing), 0.0f, (float)(grid->size_y - 1));
    return xu * grid->size_y + yu;
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
    grid->num_markers = (int)floorf(grid->total_size / 2);
    grid->cell_size = cell_size;
    grid->inv_cell_size = 1.0f / cell_size;
    grid->marker_radius = grid->cell_size / 3.0f;
    grid->marker_inv_spacing = 1.0f / (2.2f * grid->marker_radius);
    // grid->marker_inv_spacing = 1.0f / (1.0f * grid->marker_radius);
    grid->markers_rest_density = 0.0f;

    grid->cells = (GridCell_t *)calloc(grid->total_size, sizeof(GridCell_t));
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
        grid->cells[cellIndex].s = 0.0f;
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

    grid->markers = (Marker_t *)calloc(grid->num_markers, sizeof(Marker_t));
    if (!grid->markers)
    {
        fprintf(stderr, "Failed to allocate memory for markers\n");
        exit(EXIT_FAILURE);
    }
    // for (int marker_index = 0; marker_index < grid->num_markers; ++marker_index)
    // {
    //     grid->markers[marker_index].position.x = ((float)rand() / RAND_MAX) * size_x * cell_size;
    //     grid->markers[marker_index].position.y = ((float)rand() / RAND_MAX) * size_y * cell_size;
    //     grid->markers[marker_index].position.z = grid->cell_size + 0.1f;

    //     grid->markers[marker_index].velocity.x = 0.0f;
    //     grid->markers[marker_index].velocity.y = 0.0f;
    //     grid->markers[marker_index].velocity.z = 0.0f;

    //     grid->markers[marker_index].color.x = 1.0f;
    //     grid->markers[marker_index].color.y = 0.0f;
    //     grid->markers[marker_index].color.z = 0.0f;
    //     grid->markers[marker_index].color.w = 1.0f;
    // }

    int marker_index = 0;
    for (int y = 0; y < size_y; ++y)
    {
        for (int x = 0; x < size_x; ++x)
        {
            for (int i = 0; i < 1; ++i) // Dwa markery na komórkę
            {
                if (marker_index >= grid->num_markers)
                    break;

                grid->markers[marker_index].position.x = (x + ((float)rand() / RAND_MAX)) * cell_size;
                grid->markers[marker_index].position.y = (y + ((float)rand() / RAND_MAX)) * cell_size;
                grid->markers[marker_index].position.z = cell_size + 0.1f; // Dolna warstwa

                grid->markers[marker_index].velocity.x = 0.0f;
                grid->markers[marker_index].velocity.y = 0.0f;
                grid->markers[marker_index].velocity.z = 0.0f;

                grid->markers[marker_index].color.x = 1.0f;
                grid->markers[marker_index].color.y = 0.0f;
                grid->markers[marker_index].color.z = 0.0f;
                grid->markers[marker_index].color.w = 1.0f;

                    ++ marker_index;
            }
        }
    }

    grid->num_cell_markers = (int *)calloc(grid->total_size, sizeof(int));
    if (!grid->num_cell_markers)
    {
        fprintf(stderr, "Failed to allocate memory for num_cell_markers\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < grid->total_size; ++i)
    {

        grid->num_cell_markers[i] = 0;
    }

    grid->first_cell_marker = (int *)calloc((grid->total_size + 1), sizeof(int));
    if (!grid->first_cell_marker)
    {
        fprintf(stderr, "Failed to allocate memory for first_cell_marker\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < grid->total_size + 1; ++i)
    {

        grid->first_cell_marker[i] = 0;
    }

    grid->cell_marker_ids = (int *)calloc(grid->num_markers, sizeof(int));
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
    float h = grid->cell_size;
    float r = grid->marker_radius;
    float or2 = obstacle->radius * obstacle->radius;
    float minDist = obstacle->radius + r;
    float minDist2 = minDist * minDist;

    for (int marker_index = 0; marker_index < grid->num_markers; marker_index++)
    {
        Marker_t &marker = grid->markers[marker_index];

        float dx = marker.position.x - obstacle->position.x;
        float dy = marker.position.y - obstacle->position.y;
        float d2 = dx * dx + dy * dy;

        if (d2 < minDist2)
        {
            marker.velocity += (obstacle->position - obstacle->last_position) * obstacle->speed * 5.0f;
            printf("obstacle (%.2f, %.2f) hit marker %d and resulted in its velocity = (%.2f, %.2f)\n", obstacle->position.x, obstacle->position.y, marker_index, marker.velocity.x, marker.velocity.y);
        }
    }
}

void MAC_integrateParticles(MacGrid_t *grid, float dt, float gravity)
{
    float minX = grid->cell_size;
    float maxX = (grid->size_x - 1) * grid->cell_size;
    float minY = grid->cell_size;
    float maxY = (grid->size_y - 1) * grid->cell_size;
    for (int marker_index = 0; marker_index < grid->num_markers; marker_index++)
    {
        Marker_t &marker = grid->markers[marker_index];
        marker.velocity.y += gravity * dt;
        marker.position.x += marker.velocity.x * dt;
        marker.position.y += marker.velocity.y * dt;

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

void MAC_pushParticlesApart(MacGrid_t *grid, int numIters, float dt)
{
    for (int i = 0; i < grid->total_size; i++)
    {
        grid->num_cell_markers[i] = 0;
    }

    for (int marker_index = 0; marker_index < grid->num_markers; marker_index++)
    {
        Marker_t &marker = grid->markers[marker_index];
        int cellIndex = getMarkerCellIndexForMarkerInvCellSize(grid, marker);
        grid->num_cell_markers[cellIndex]++;
    }

    int first = 0;
    for (int i = 0; i < grid->total_size; i++)
    {
        first += grid->num_cell_markers[i];
        grid->first_cell_marker[i] = first;
    }
    grid->first_cell_marker[grid->total_size] = first;

    for (int marker_index = 0; marker_index < grid->num_markers; marker_index++)
    {
        Marker_t &marker = grid->markers[marker_index];
        int cellIndex = getMarkerCellIndexForMarkerInvCellSize(grid, marker);
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
            int x0 = max(pxi - 1, 0);
            int y0 = max(pyi - 1, 0);
            int x1 = min(pxi + 1, grid->size_x - 1);
            int y1 = min(pyi + 1, grid->size_y - 1);

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
                        // float s = minf(0.5f * (minDist - d) / d, 0.1f);
                        dx *= s * 0.1f;
                        dy *= s * 0.1f;
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
            if (grid->cells[cellIndex].type != SOLID)
                grid->cells[cellIndex].type = FLUID;
        }
    }

    for (int component = 0; component < 2; component++) // 0 === u/x, 1 === v/y
    {
        float dx = component == 0 ? 0.0f : h2;
        float dy = component == 0 ? h2 : 0.0f;

        for (int marker_index = 0; marker_index < grid->num_markers; marker_index++)
        {
            // TODO this can be move before loop
            float x = clampf(grid->markers[marker_index].position.x, h, (grid->size_x - 1.0f) * h);
            float y = clampf(grid->markers[marker_index].position.y, h, (grid->size_y - 1.0f) * h);

            int x0 = min((int)floorf((x - dx) * h1), grid->size_x - 2);
            float tx = (x - dx - x0 * h) * h1;
            int x1 = min(x0 + 1, grid->size_x - 2);

            int y0 = min((int)floorf((y - dy) * h1), grid->size_y - 2);
            float ty = (y - dy - y0 * h) * h1;
            int y1 = min(y0 + 1, grid->size_y - 2);

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
            // TODO this can be move before loop

            // if (component == 1)
            // {
            //     printf("\n%s marker_index: %d\n", toGrid ? "TO GRID" : "FROM GRID", marker_index);
            //     printf("x: %f, y: %f h1: %f h: %f  dx: %f  dy: %f\n", x, y, h1, h, dx, dy);
            //     printf("x0: %d, y0: %d, x1: %d, y1: %d\n", x0, y0, x1, y1);
            //     printf("tx: %f, ty: %f, sx: %f, sy: %f\n", tx, ty, sx, sy);
            //     printf("d0: %f, d1: %f, d2: %f, d3: %f\n", d0, d1, d2, d3);
            //     printf("cellIndex0: %d, cellIndex1: %d, cellIndex2: %d, cellIndex3: %d\n", cellIndex0, cellIndex1, cellIndex2, cellIndex3);
            // }

            if (toGrid)
            {
                if (component == 0)
                {
                    float pv = grid->markers[marker_index].velocity.x;
                    // printf("pv: %f\n", pv);
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
                    float pv = grid->markers[marker_index].velocity.y;
                    // printf("pv: %f\n", pv);
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
                        // if (isnan(grid->markers[marker_index].velocity.x) || isnan(grid->markers[marker_index].velocity.y))
                        // {
                        //     // marker.velocity = glm::vec3(0.0f);
                        //     printf("picV = %f, corr = %f, flipV = %f\n", picV, corr, flipV);
                        //     printf("valid0 = %f, d0 = %f, grid->cells[cellIndex0].u = %f\n", valid0, d0, grid->cells[cellIndex0].u);
                        //     printf("valid1 = %f, d1 = %f, grid->cells[cellIndex1].u = %f\n", valid1, d1, grid->cells[cellIndex1].u);
                        //     printf("valid2 = %f, d2 = %f, grid->cells[cellIndex2].u = %f\n", valid2, d2, grid->cells[cellIndex2].u);
                        //     printf("valid3 = %f, d3 = %f, grid->cells[cellIndex3].u = %f\n", valid3, d3, grid->cells[cellIndex3].u);
                        //     printf("[transfer velocity component == 0] marker[%d].velocity is nan: (%f, %f)\n", marker_index, grid->markers[marker_index].velocity.x, grid->markers[marker_index].velocity.y);
                        //     exit(EXIT_FAILURE);
                        // }
                    }
                    else if (component == 1)
                    {

                        float picV = (valid0 * d0 * grid->cells[cellIndex0].v + valid1 * d1 * grid->cells[cellIndex1].v + valid2 * d2 * grid->cells[cellIndex2].v + valid3 * d3 * grid->cells[cellIndex3].v) / d;
                        float corr = (valid0 * d0 * (grid->cells[cellIndex0].v - grid->cells[cellIndex0].prevv) + valid1 * d1 * (grid->cells[cellIndex1].v - grid->cells[cellIndex1].prevv) + valid2 * d2 * (grid->cells[cellIndex2].v - grid->cells[cellIndex2].prevv) + valid3 * d3 * (grid->cells[cellIndex3].v - grid->cells[cellIndex3].prevv)) / d;
                        float flipV = v + corr;
                        grid->markers[marker_index].velocity[component] = flipRatio * flipV + (1.0f - flipRatio) * picV;
                        // if (isnan(grid->markers[marker_index].velocity.x) || isnan(grid->markers[marker_index].velocity.y))
                        // {
                        //     // marker.velocity = glm::vec3(0.0f);
                        //     printf("[transfer velocity component == 1] marker[%d].velocity is nan: (%f, %f)\n", marker_index, grid->markers[marker_index].velocity.x, grid->markers[marker_index].velocity.y);
                        //     exit(EXIT_FAILURE);
                        // }
                    }
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
                        // printf("grid->cells[%d].u: %f\n", cellIndex, grid->cells[cellIndex].u);
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
                        // printf("grid->cells[%d].v: %f\n", cellIndex, grid->cells[cellIndex].v);
                    }
                }
            }

            for (int grix_x = 1; grix_x < grid->size_x - 1; grix_x++)
            {
                for (int grix_y = 1; grix_y < grid->size_y - 1; grix_y++)
                {
                    int cellIndex = grix_x * n + grix_y;
                    int is_solid = grid->cells[cellIndex].type == SOLID;
                    if (is_solid && grid->cells[(grix_x - 1) * n + grix_y].type == SOLID)
                    {
                        grid->cells[cellIndex].u = grid->cells[cellIndex].prevu;
                    }
                    if (is_solid && grid->cells[grix_x * n + (grix_y - 1)].type == SOLID)
                    {
                        grid->cells[cellIndex].v = grid->cells[cellIndex].prevv;
                    }
                }
            }
        }
    }
}

void MAC_updateParticleDensity(MacGrid_t *grid)
{
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

        int x0 = (int)max(0, min(grid->size_x - 1, floorf((x - h2) * h1)));
        // int x0 = (int)floorf((x - h2) * h1);
        int x1 = (int)max(0, min(grid->size_x - 1, x0 + 1));
        // int x1 = (int)max(grid->size_x - 2, x0 + 1);

        int y0 = (int)max(0, min(grid->size_y - 1, floorf((y - h2) * h1)));
        // int y0 = (int)floorf((y - h2) * h1);
        int y1 = (int)max(0, min(grid->size_y - 1, y0 + 1));
        // int y1 = (int)min(grid->size_y - 2, y0 + 1);

        float tx = ((x - h2) - x0 * h) * h1;
        float ty = ((y - h2) - y0 * h) * h1;

        float sx = 1.0f - tx;
        float sy = 1.0f - ty;

        if (x0 < grid->size_x && y0 < grid->size_y)
            grid->cells[x0 * n + y0].density += sx * sy;

        if (x1 < grid->size_x && y0 < grid->size_y)
            grid->cells[x1 * n + y0].density += tx * sy;

        if (x1 < grid->size_x && y1 >= 0 && y1 < grid->size_y)
            grid->cells[x1 * n + y1].density += tx * ty;

        if (x0 < grid->size_x && y1 < grid->size_y)
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
    double cp = grid->density * grid->cell_size / dt;

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

                double div = (right->u - center->u) + (top->v - center->v);

                if (grid->markers_rest_density > 0.0f)
                {
                    float k = 1.0f;
                    float compression = center->density - grid->markers_rest_density;
                    if (compression > 0.0f)
                    {
                        div -= k * compression;
                    }
                }

                double p = -div / s;
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

    float minPressure = FLT_MAX;
    float maxPressure = FLT_MIN;

    for (int i = 0; i < grid->total_size; i++)
    {
        if (grid->cells[i].type == FLUID)
        {
            float p = grid->cells[i].p;
            if (p < minPressure)
                minPressure = p;
            if (p > maxPressure)
                maxPressure = p;
        }
    }

    for (int x = 0; x < grid->size_x; ++x)
    {
        for (int y = 0; y < grid->size_y; ++y)
        {
            int cellIndex = x * grid->size_y + y;
            glm::vec3 cubePos = glm::vec3(x, y, 0) * grid->cell_size;

            glm::vec3 color;
            if (grid->cells[cellIndex].type == FLUID)
            {
                float c[3] = {0.0f, 0.0f, 1.0f};
                getSciColor(grid->cells[cellIndex].p, minPressure, maxPressure, c);
                color.x = c[0];
                color.y = c[1];
                color.z = c[2];
            }
            else if (grid->cells[cellIndex].type == SOLID)
            {
                color = glm::vec3(1.0f, 1.0f, 1.0f);
            }
            else if (grid->cells[cellIndex].type == AIR)
            {
                color = glm::vec3(0.53f, 0.81f, 0.94f);
            }

            for (int i = 0; i < 8; i++)
            {
                vertices[vert_index].position = cubePos + cubeVertices[i] * grid->cell_size;
                vertices[vert_index].color = color;
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
