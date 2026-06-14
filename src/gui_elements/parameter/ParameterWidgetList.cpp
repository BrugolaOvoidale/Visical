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

std::shared_ptr<ParameterWidget> ParameterWidgetList::GetWidget(const wxString& parameterId)
{
    auto it = m_parameterIdtoItem.find(parameterId);
    if (it == m_parameterIdtoItem.end())
        return nullptr;

	return it->second;
}

void ParameterWidgetList::RemoveParameter(const wxString& toRemoveId)
{
    auto it = m_parameterIdtoItem.find(toRemoveId);
    if (it == m_parameterIdtoItem.end()) return;


    std::shared_ptr<ParameterWidget> parameter = it->second;

    m_parameterIdtoItem.erase(toRemoveId);

    m_itemToParameterId.erase(parameter);

    BaseWidgetList::RemoveWidget(parameter);
}

bool ParameterWidgetList::SelectParameter(const wxString& parameterId)
{
    auto it = m_parameterIdtoItem.find(parameterId);
    if (it == m_parameterIdtoItem.end()) return false;

    it->second->SetSelected(true);
    m_selectedWidget = it->second;

    return true;
}

//////////////////////////////////////////////////////////////////////////

std::shared_ptr<ParameterWidget> ParameterWidgetList::CreateWidget(const std::shared_ptr<ParameterInfo>& param)
{
    auto it = m_parameterIdtoItem.find(param->name());
    if (it != m_parameterIdtoItem.end())
    {
        it->second->Update(param);

        return it->second;
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


    m_parameterIdtoItem[param->name()] = parameter;
    m_itemToParameterId[parameter] = param->name();

    // Bind
    parameter->Bind(GUI_SELECT_PARAM, &ParameterWidgetList::OnWidgetClicked, this);

    return parameter;
}

void ParameterWidgetList::RemoveSelectedWidgetImpl(const std::shared_ptr<BaseWidget>& baseWidget)
{
    std::shared_ptr<ParameterWidget> toRemoveWidget = std::static_pointer_cast<ParameterWidget>(baseWidget);

    const wxString toRemoveId = m_itemToParameterId.at(toRemoveWidget);

    m_itemToParameterId.erase(toRemoveWidget);

    m_parameterIdtoItem.erase(toRemoveId);
}

void ParameterWidgetList::RemoveAllWidgetsImpl()
{
    m_parameterIdtoItem.clear();
    m_itemToParameterId.clear();
}