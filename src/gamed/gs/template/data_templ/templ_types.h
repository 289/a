#ifndef GAMED_GS_TEMPLATE_DATATEMPL_TEMPL_TYPES_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_TEMPL_TYPES_H_

#include <stdint.h>


namespace dataTempl {

typedef int32_t  TemplID;
typedef uint16_t TemplType;
typedef int32_t  SkillID;
typedef int32_t  EffectID;
typedef uint16_t LevelType;
typedef int32_t  PropType;
typedef int32_t  MapID;
typedef int32_t  BuffID;

typedef int32_t  EventID;
typedef EventID  AIEventID;

#define INS_UI_STYLE_SEG 0
#define BG_UI_STYLE_SEG 100

} // namespace dataTempl

// version
#define DATATEMPLATE_VERSION 0x00000001

#endif // GAMED_GS_TEMPLATE_DATATEMPL_TEMPL_TYPES_H_
