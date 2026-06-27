#include "ParameterWidgetList.hpp"
#include <parameter/bool/ParameterBoolInfo.hpp>
#include <parameter/numeric/ParameterNumericInfo.hpp>
#include <parameter/enum/ParameterEnumInfo.hpp>
#include <parameter/string/ParameterStringInfo.hpp>
#include "ParameterWidget.hpp"
#include "BoolParameterWidget.hpp"
#include "NumericParameterWidget.hpp"
#include "EnumParameterWidget.hpp"
#include "CommandParameterWidget.hpp"
#include "StringParameterWidget.hpp"



ParameterWidgetList::ParameterWidgetList(
	wxWindow* parent,
	wxWindowID winid,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name)
	: BaseWidgetList(parent,
		winid,
		pos,
		size,
        style,
		name)
{
}

//////////////////////////////////////////////////////////////////////////

void ParameterWidgetList::SetParameters(const std::vector<std::shared_ptr<ParameterInfo>>& params)
{
    std::vector<std::shared_ptr<BaseWidget>> widgets;
    widgets.reserve(params.size());

    for (const auto& p : params) widgets.push_back(CreateWidget(p));

    BaseWidgetList::SetWidgets(widgets);
}

std::shared_ptr<ParameterWidget> ParameterWidgetList::GetWidget(
    const wxString& parameterId,
    const wxString& categoryId)
{
    auto it = m_parameterIdtoItem.find(categoryId);
    if (it == m_parameterIdtoItem.end())
        return nullptr;

    auto pIt = it->second.find(parameterId);
    if (pIt == it->second.end())
        return nullptr;

	return pIt->second;
}

void ParameterWidgetList::RemoveParameter(
    const wxString& parameterId,
    const wxString& categoryId)
{
    auto it = m_parameterIdtoItem.find(categoryId);
    if (it == m_parameterIdtoItem.end())
        return;

    auto pIt = it->second.find(parameterId);
    if (pIt == it->second.end())
        return;


    std::shared_ptr<ParameterWidget> parameter = pIt->second;

    it->second.erase(parameterId);

    m_itemToParameterId.at(categoryId).erase(parameter);

    BaseWidgetList::RemoveWidget(parameter);
}

bool ParameterWidgetList::SelectParameter(
    const wxString& parameterId,
    const wxString& categoryId)
{
    auto it = m_parameterIdtoItem.find(categoryId);
    if (it == m_parameterIdtoItem.end())
        return false;

    auto pIt = it->second.find(parameterId);
    if (pIt == it->second.end())
        return false;

    pIt->second->SetSelected(true);
    m_selectedWidget = pIt->second;

    return true;
}

//////////////////////////////////////////////////////////////////////////

std::shared_ptr<ParameterWidget> ParameterWidgetList::CreateWidget(const std::shared_ptr<ParameterInfo>& param)
{
    auto it = m_parameterIdtoItem.find(param->category());
    if (it != m_parameterIdtoItem.end())
    {
        auto pIt = it->second.find(param->name());
        if (pIt != it->second.end())
        {
            pIt->second->Update(param);

            return pIt->second;
        }
    }

    std::shared_ptr<ParameterWidget> parameter;

    switch (param->dataType())
    {
        case DataType::BOOLEAN:
            parameter = std::make_shared<BoolParameterWidget>(
                this,
				std::dynamic_pointer_cast<ParameterBoolInfo>(param)
            );

            break;

        case DataType::DOUBLE:
        {
            parameter = std::make_shared<DoubleParameterWidget>(
                this,
                std::dynamic_pointer_cast<ParameterDoubleInfo>(param)
            );

            break;
        }

        case DataType::ENUMERATION:
        {
            parameter = std::make_shared<EnumParameterWidget>(
                this,
                std::dynamic_pointer_cast<ParameterEnumInfo>(param)
            );

            break;
        }

        case DataType::COMMAND:
        {
            parameter = std::make_shared<CommandParameterWidget>(
                this,
                param
            );

            break;
        }

        case DataType::INTEGER:
        {
            parameter = std::make_shared<IntParameterWidget>(
                this,
                std::dynamic_pointer_cast<ParameterIntInfo>(param)
            );

            break;
        }

        case DataType::STRING:
        {
            parameter = std::make_shared<StringParameterWidget>(
                this,
                std::dynamic_pointer_cast<ParameterStringInfo>(param)
            );

            break;
        }

        default:
            return nullptr;
    }


    m_parameterIdtoItem[param->category()][param->name()] = parameter;
    m_itemToParameterId[param->category()][parameter] = param->name();

    // Bind
    parameter->Bind(GUI_SELECT_PARAM, &ParameterWidgetList::OnWidgetClicked, this);

    return parameter;
}

void ParameterWidgetList::RemoveSelectedWidgetImpl(const std::shared_ptr<BaseWidget>& baseWidget)
{
    std::shared_ptr<ParameterWidget> toRemoveWidget = std::static_pointer_cast<ParameterWidget>(baseWidget);

    const wxString categoryId = toRemoveWidget->GetCategoryId();
    const wxString toRemoveId = toRemoveWidget->GetParameterId();

    m_itemToParameterId.at(categoryId).erase(toRemoveWidget);

    m_parameterIdtoItem.at(categoryId).erase(toRemoveId);
}

void ParameterWidgetList::RemoveAllWidgetsImpl()
{
    m_parameterIdtoItem.clear();
    m_itemToParameterId.clear();
}