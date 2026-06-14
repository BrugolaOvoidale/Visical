#include "ParameterMap.hpp"
#include <algorithm>
#include <set>
#include "Parameter.hpp"


//////////////////////////////////////////////////////////////////////////////////

static void applySorting(
    std::vector<std::shared_ptr<Parameter>>& params,
    bool inAlphabeticalOrder)
{
    if (!inAlphabeticalOrder)
        return;

    std::sort(params.begin(), params.end(),
        [&](const std::shared_ptr<Parameter>& a, const std::shared_ptr<Parameter>& b) {
            if (inAlphabeticalOrder)
            {
                return a->name() < b->name();
            }
            return false;
        });
}

//////////////////////////////////////////////////////////////////////////////////

ParameterMap::ParameterMap(const std::vector<std::shared_ptr<Parameter>>& camParams)
{
    for (const auto& param : camParams)
    {
        categoryMap_[param->category()]
            [param->visibilityLevel()]
            .push_back(param);
    }
}

//////////////////////////////////////////////////////////////////////////////////

void ParameterMap::addParameter(const std::shared_ptr<Parameter>& param)
{
    categoryMap_[param->category()][param->visibilityLevel()].push_back(param);
}

std::vector<std::shared_ptr<Parameter>> ParameterMap::getFilteredParams(
    const std::optional<std::string>& category,
    std::optional<VisibilityLevel> visibility,
    bool inAlphabeticalOrder) const
{
    std::vector<std::shared_ptr<Parameter>> result;

    if (category.has_value() && visibility.has_value())
    {
        const std::string catValue = category.value();

        // Find category
        auto categoryIt = categoryMap_.find(catValue);
        if (categoryIt == categoryMap_.end())
        {
            return result;
        }

        const VisibilityInfoMap& visMap = categoryIt->second;

        // Determine visibility levels
        std::vector<VisibilityLevel> levelsToCheck;
        switch (visibility.value())
        {
        case VisibilityLevel::ADVANCED:
            levelsToCheck.push_back(VisibilityLevel::ADVANCED);

        case VisibilityLevel::INTERMEDIATE:
            levelsToCheck.push_back(VisibilityLevel::INTERMEDIATE);

        case VisibilityLevel::BASIC:
            levelsToCheck.push_back(VisibilityLevel::BASIC);

            break;

        default:
            return result;
        }

        // Collect
        for (VisibilityLevel lvl : levelsToCheck)
        {
            auto it = visMap.find(lvl);
            if (it != visMap.end())
            {
                result.insert(result.end(), it->second.begin(), it->second.end());
            }
        }
    }
    else if (category.has_value())
    {
        // Find category
        auto categoryIt = categoryMap_.find(category.value());
        if (categoryIt == categoryMap_.end())
        {
            return result;
        }

        // Determine visibility levels
        std::vector<VisibilityLevel> levelsToCheck;
        levelsToCheck.push_back(VisibilityLevel::BASIC);
        levelsToCheck.push_back(VisibilityLevel::INTERMEDIATE);
        levelsToCheck.push_back(VisibilityLevel::ADVANCED);

        // Collect
        for (VisibilityLevel lvl : levelsToCheck)
        {
            auto it = categoryIt->second.find(lvl);
            if (it != categoryIt->second.end())
            {
                result.insert(result.end(), it->second.begin(), it->second.end());
            }
        }
    }
    else if (visibility.has_value())
    {
        // Determine visibility levels
        std::vector<VisibilityLevel> levelsToInclude;
        switch (visibility.value())
        {
        case VisibilityLevel::ADVANCED:
            levelsToInclude.push_back(VisibilityLevel::ADVANCED);

        case VisibilityLevel::INTERMEDIATE:
            levelsToInclude.push_back(VisibilityLevel::INTERMEDIATE);

        case VisibilityLevel::BASIC:
            levelsToInclude.push_back(VisibilityLevel::BASIC);

            break;

        default:
            return result;
        }

        // Collect
        for (const auto& [category, visibilityMap] : categoryMap_)
        {
            for (VisibilityLevel lvl : levelsToInclude)
            {
                auto it = visibilityMap.find(lvl);
                if (it != visibilityMap.end())
                {
                    result.insert(result.end(), it->second.begin(), it->second.end());
                }
            }
        }
    }
    else
    {
        for (const auto& [category, visibilityMap] : categoryMap_)
        {
            for (const auto& [level, params] : visibilityMap)
            {
                result.insert(result.end(), params.begin(), params.end());
            }
        }
    }

    // Apply sorting
    applySorting(result, inAlphabeticalOrder);

    return result;
}

std::shared_ptr<Parameter> ParameterMap::getParameter(
    const std::string& paramName,
    const std::string& category) const
{
    auto it = categoryMap_.find(category);
    if (it == categoryMap_.end())
    {
        return nullptr;
    }

    for (const auto& [level, params] : it->second)
    {
        auto p = std::find_if(params.begin(), params.end(),
            [&paramName](const std::shared_ptr<Parameter>& param)
            {
                return param->name() == paramName;
            });

        if (p != params.end())
        {
            return *p;  // Return found parameter
        }
    }

    return nullptr;  // Return empty
}

std::vector<std::string> ParameterMap::getAllCategories() const
{
    std::vector<std::string> categories;
    for (const auto& [category, _] : categoryMap_)
    {
        categories.push_back(category);
    }
    return categories;
}

std::vector<VisibilityLevel> ParameterMap::getAllVisibilityLevels() const
{
    std::set<VisibilityLevel> levels; // Using a set to avoid duplicates

    for (const auto& [category, visibilityMap] : categoryMap_)
    {
        for (const auto& [level, _] : visibilityMap)
        {
            levels.insert(level);
        }
    }

    return std::vector<VisibilityLevel>(levels.begin(), levels.end());
}

void ParameterMap::clear()
{
    categoryMap_.clear();
}