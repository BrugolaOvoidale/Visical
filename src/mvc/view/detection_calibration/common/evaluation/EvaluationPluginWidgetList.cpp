#include "EvaluationPluginWidgetList.hpp"
#include <evaluator/PluginContextInfo.hpp>
#include <evaluator/PluginInfo.hpp>
#include "EvaluationPluginWidget.hpp"
#include "EvaluationPluginWidgetEvents.hpp"


EvaluationPluginWidgetList::EvaluationPluginWidgetList(
    wxWindow* parent,
	wxWindowID winid,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name)
	: 
    BaseWidgetList(parent,
        winid,
        pos,
        size,
        style,
        name)
{
}

//////////////////////////////////////////////////////////////////////////

void EvaluationPluginWidgetList::AppendPlugin(const std::shared_ptr<PluginContextInfo>& pluginCtx)
{
    std::shared_ptr<EvaluationPluginWidget> plugin = std::make_shared<EvaluationPluginWidget>(this, pluginCtx);

    const std::string& pluginId = pluginCtx->getPluginInfo()->id();
    m_pluginIdtoItem[pluginId] = plugin;
    m_itemToPluginId[plugin] = pluginId;

    // Bind custom event
    plugin->Bind(GUI_SELECT_EVAL_PLUGIN, &EvaluationPluginWidgetList::OnWidgetClicked, this);
    plugin->Bind(GUI_SHOW_DEB_RESULT, &EvaluationPluginWidgetList::OnShowDebugResult, this);
    plugin->Bind(GUI_OPEN_PLUGIN_PARAMS, &EvaluationPluginWidgetList::OnOpenPluginParameters, this);

    BaseWidgetList::AppendWidget(plugin);
}

std::shared_ptr<EvaluationPluginWidget> EvaluationPluginWidgetList::GetWidget(const wxString& pluginId)
{
    auto it = m_pluginIdtoItem.find(pluginId);
    if (it == m_pluginIdtoItem.end())
        return nullptr;

	return it->second;
}

std::vector<std::shared_ptr<EvaluationPluginWidget>> EvaluationPluginWidgetList::GetAllWidgets()
{
    std::vector<std::shared_ptr<EvaluationPluginWidget>> widgets;
    widgets.reserve(m_itemToPluginId.size());

    for (const auto& pair : m_itemToPluginId)
    {
        widgets.push_back(pair.first);
    }

    return widgets;
}

void EvaluationPluginWidgetList::RemovePlugin(const wxString& toRemoveId)
{
    auto it = m_pluginIdtoItem.find(toRemoveId);
    if (it == m_pluginIdtoItem.end())
        return;

    std::shared_ptr<EvaluationPluginWidget> plugin = it->second;

    m_pluginIdtoItem.erase(toRemoveId);

    m_itemToPluginId.erase(plugin);

    BaseWidgetList::RemoveWidget(plugin);
}

std::optional<wxString> EvaluationPluginWidgetList::GetSelectedDebPlugin() const
{
    if (!m_lastShowDebPlugin)
        return std::nullopt;

    return m_lastShowDebPlugin->GetPluginId();
}

bool EvaluationPluginWidgetList::IsShowDebPluginSelected() const
{
    return m_lastShowDebPlugin != nullptr;
}

bool EvaluationPluginWidgetList::CheckDebButton(const wxString& pluginId)
{
    auto it = m_pluginIdtoItem.find(pluginId);
    if (it == m_pluginIdtoItem.end())
        return false;

    std::shared_ptr<EvaluationPluginWidget> newP = it->second;

    if (newP == m_lastShowDebPlugin)
        return true;

    // Uncheck previous
    if (m_lastShowDebPlugin)
        m_lastShowDebPlugin->SetDebugResult(false);

    newP->SetDebugResult(true);

    m_lastShowDebPlugin = newP;

	return true;
}

void EvaluationPluginWidgetList::UncheckDebButton()
{
    // Uncheck
    if (m_lastShowDebPlugin)
        m_lastShowDebPlugin->SetDebugResult(false);

    m_lastShowDebPlugin = nullptr;
}

void EvaluationPluginWidgetList::CloseLastParametersFrame()
{
    // Deselect previous
    if (m_lastParamsFrame)
        m_lastParamsFrame->CloseParametersFrame();

    m_lastParamsFrame = nullptr;
}

//////////////////////////////////////////////////////////////////////////

void EvaluationPluginWidgetList::RemoveSelectedWidgetImpl(const std::shared_ptr<BaseWidget>& baseWidget)
{
	std::shared_ptr<EvaluationPluginWidget> toRemoveWidget = std::static_pointer_cast<EvaluationPluginWidget>(baseWidget);

    const wxString toRemoveId = m_itemToPluginId.at(toRemoveWidget);

    m_itemToPluginId.erase(toRemoveWidget);

    m_pluginIdtoItem.erase(toRemoveId);
}

void EvaluationPluginWidgetList::RemoveAllWidgetsImpl()
{
    m_itemToPluginId.clear();
    m_pluginIdtoItem.clear();
}

void EvaluationPluginWidgetList::OnShowDebugResult(wxCommandEvent& event)
{
    const wxString& pluginId = event.GetString();

    auto it = m_pluginIdtoItem.find(pluginId);
    if (it == m_pluginIdtoItem.end())
    {
        event.Skip();
        return;
    }

    std::shared_ptr<EvaluationPluginWidget> plugin = it->second;

    // Check if clicking the same widget (toggle current)
    if (m_lastShowDebPlugin == plugin)
    {
        if (!m_lastShowDebPlugin->IsDebugResultShown())
            m_lastShowDebPlugin = nullptr;
    }
    else
    {
        // Deselect previous
        if (m_lastShowDebPlugin)
            m_lastShowDebPlugin->SetDebugResult(false);

        m_lastShowDebPlugin = plugin;
    }

    event.Skip();
}

void EvaluationPluginWidgetList::OnOpenPluginParameters(wxCommandEvent& event)
{
    const wxString& pluginId = event.GetString();

    auto it = m_pluginIdtoItem.find(pluginId);
    if (it == m_pluginIdtoItem.end())
    {
        event.Skip();
        return;
    }

    std::shared_ptr<EvaluationPluginWidget> plugin = it->second;

    // Deselect previous
    if (m_lastParamsFrame && m_lastParamsFrame != plugin)
        m_lastParamsFrame->CloseParametersFrame();

    m_lastParamsFrame = plugin;

    event.Skip();
}