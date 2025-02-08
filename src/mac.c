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

    grid->markers = (Marker_t *)malloc(size_x * size_y * 4 * sizeof(Marker));
    uint16_t current_marker = 0;
    for (uint16_t x = 1; x < size_x - 1; ++x)
    {
        for (uint16_t y = 1; y < size_y - 1; ++y)
        {
            if (grid->cells[x][y].type == FLUID)
            {
                for (int i = 0; i < 4; ++i)
                {
                    Marker_t marker;
                    marker.position = glm::vec3(
                        (x + 0.25f * (i % 2)) * grid->cell_size,
                        (y + 0.25f * (i / 2)) * grid->cell_size,
                        0.0f);
                    marker.velocity = glm::vec3(0.0f);
                    marker.density = 1.0f;
                    marker.rest_density = 1.0f;
                    marker.color = glm::vec4(1.0f);
                    grid->markers[current_marker++] = marker;
                }
            }
        }
    }
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

void MAC_update(MacGrid_t *grid, float dt)
{
    // 1. Zachowanie poprzednich wartości prędkości
    for (uint16_t x = 0; x < grid->size_x; ++x)
    {
        for (uint16_t y = 0; y < grid->size_y; ++y)
        {
            GridCell_t *cell = &grid->cells[x][y];
            if (cell->type == FLUID)
            {
                cell->prevu = cell->u;
                cell->prevv = cell->v;
                cell->prevw = cell->w;
                // Zerowanie siły na nową iterację
                cell->du = 0.0f;
                cell->dv = 0.0f;
                cell->dw = 0.0f;
            }
        }
    }

    // 2. Aktualizacja prędkości zgodnie z siłami
    for (uint16_t x = 0; x < grid->size_x; ++x)
    {
        for (uint16_t y = 0; y < grid->size_y; ++y)
        {
            GridCell_t *cell = &grid->cells[x][y];
            if (cell->type == FLUID)
            {
                cell->u += dt * (-cell->du + cell->s * (cell->prevu - cell->u));
                cell->v += dt * (-cell->dv + cell->s * (cell->prevv - cell->v));
                cell->w += dt * (-cell->dw + cell->s * (cell->prevw - cell->w));
            }
        }
    }

    // 3. Korekcja ciśnienia w oparciu o zmiany prędkości
    for (uint16_t x = 0; x < grid->size_x; ++x)
    {
        for (uint16_t y = 0; y < grid->size_y; ++y)
        {
            GridCell_t *cell = &grid->cells[x][y];
            if (cell->type == FLUID)
            {
                cell->p += dt * ((cell->u - cell->prevu) + (cell->v - cell->prevv) + (cell->w - cell->prevw));
            }
        }
    }

    // 4. Aktualizacja znaczników (Markerów) zgodnie z prędkością komórek
    for (uint16_t i = 0; i < grid->size_x * grid->size_y * 4; ++i)
    {
        Marker_t *marker = &grid->markers[i];
        marker->position += marker->velocity * dt;

        uint16_t x = static_cast<uint16_t>(marker->position.x / grid->cell_size);
        uint16_t y = static_cast<uint16_t>(marker->position.y / grid->cell_size);

        if (x < grid->size_x && y < grid->size_y)
        {
            GridCell_t *cell = &grid->cells[x][y];
            if (cell->type == FLUID)
            {
                marker->velocity += glm::vec3(cell->du, cell->dv, cell->dw) * dt;
            }
        }
    }
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