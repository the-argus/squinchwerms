#ifndef __WERMS_LOGGING_CATEGORY_H__
#define __WERMS_LOGGING_CATEGORY_H__

enum class LoggingCategory
{
    Renderer,
    Physics,
    Gameplay,
    Hotreload,
};

constexpr const char *loggingCategoryToString(LoggingCategory category)
{
    switch (category) {
    case LoggingCategory::Hotreload:
        return "Hotreload";
    case LoggingCategory::Physics:
        return "Physics";
    case LoggingCategory::Gameplay:
        return "Physics";
    case LoggingCategory::Renderer:
        return "Renderer";
    }
    return "Unknown Category";
}

#endif
