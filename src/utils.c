#include "utils.h"

// void getSciColor(float val, float minVal, float maxVal, float color[3])
// {
//     val = fmin(fmax(val, minVal), maxVal - 0.0001);
//     float d = maxVal - minVal;
//     val = d == 0.0 ? 0.5 : (val - minVal) / d;
//     float m = 0.25;
//     uint8_t num = (uint8_t)floor(val / m);
//     float s = (val - num * m) / m;
//     float r = 0.0, g = 0.0, b = 0.0;

//     switch (num)
//     {
//     case 0:
//         r = 0.0;
//         g = s;
//         b = 1.0;
//         break;
//     case 1:
//         r = 0.0;
//         g = 1.0;
//         b = 1.0 - s;
//         break;
//     case 2:
//         r = s;
//         g = 1.0;
//         b = 0.0;
//         break;
//     case 3:
//         r = 1.0;
//         g = 1.0 - s;
//         b = 0.0;
//         break;
//     }

//     color[0] = r;
//     color[1] = g;
//     color[2] = b;
// }

char *loadResourceFileContent(const char *filename)
{
    size_t pathLen = strlen(RESOURCE_PATH) + strlen(filename) + 1;
    char *fullPath = (char *)malloc(pathLen);
    if (!fullPath)
    {
        fprintf(stderr, "Memory allocation failed for shader file path\n");
        return NULL;
    }

    snprintf(fullPath, pathLen, "%s%s", RESOURCE_PATH, filename);

    FILE *file = fopen(fullPath, "r");
    if (!file)
    {
        fprintf(stderr, "Error loading shader: %s\n", fullPath);
        return NULL;
    }
    free(fullPath);

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(fileSize + 1);
    if (!buffer)
    {
        fprintf(stderr, "Memory allocation failed for shader content: %s\n", filename);
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, fileSize, file);
    buffer[fileSize] = '\0';

    fclose(file);
    return buffer;
}