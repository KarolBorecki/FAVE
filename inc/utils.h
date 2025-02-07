#ifndef FAVE_UTILS_H
#define FAVE_UTILS_H

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

#endif // FAVE_UTILS_H