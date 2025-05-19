
#ifndef LIGHT_TYPES_H
#define LIGHT_TYPES_H

#ifdef __cplusplus
constexpr Int32 LightType_None{ 0 };
constexpr Int32 LightType_Directional{ 1 };
constexpr Int32 LightType_Spot{ 2 };
constexpr Int32 LightType_Point{ 3 };
#else
static const int LightType_None = 0;
static const int LightType_Directional = 1;
static const int LightType_Spot = 2;
static const int LightType_Point = 3;
#endif

#endif // LIGHT_CB_H