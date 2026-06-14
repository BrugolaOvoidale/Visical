#include "PluginPreviewWidgetList.hpp"
#include <evaluator/PluginContextInfo.hpp>
#include <evaluator/PluginInfo.hpp>
#include "PluginPreviewWidget.hpp"
#include "PluginPreviewWidgetEvents.hpp"


PluginPreviewWidgetList::PluginPreviewWidgetList(
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

void PluginPreviewWidgetList::SetPlugins(const std::vector<std::shared_ptr<PluginContextInfo>>& plugins)
{
    std::vector<std::shared_ptr<BaseWidget>> widgets;
    widgets.reserve(plugins.size());
    for (const auto& p : plugins) widgets.push_back(CreateWidget(p));

    BaseWidgetList::SetWidgets(widgets);
}

std::shared_ptr<PluginPreviewWidget> PluginPreviewWidgetList::GetWidget(const wxString& pluginId)
{
    auto it = m_pluginIdtoItem.find(pluginId);
    if (it == m_pluginIdtoItem.end())
        return nullptr;

    return it->second;
}

std::vector<std::shared_ptr<PluginPreviewWidget>> PluginPreviewWidgetList::GetAllWidgets()
{
    std::vector<std::shared_ptr<PluginPreviewWidget>> widgets;
    widgets.reserve(m_itemToPluginId.size());

    for (const auto& pair : m_itemToPluginId)
    {
        widgets.push_back(pair.first);
    }

    return widgets;
}

void PluginPreviewWidgetList::AppendPlugin(const std::shared_ptr<PluginContextInfo>& pluginCtx)
{
    std::shared_ptr<PluginPreviewWidget> plugin = std::make_shared<PluginPreviewWidget>(this, pluginCtx);

    const std::string& pluginId = pluginCtx->getPluginInfo()->id();
    m_pluginIdtoItem[pluginId] = plugin;
    m_itemToPluginId[plugin] = pluginId;

    // Bind custom event
    plugin->Bind(GUI_SELECT_PLUGIN_PREVIEW, &PluginPreviewWidgetList::OnWidgetClicked, this);

    BaseWidgetList::AppendWidget(plugin);
}

void PluginPreviewWidgetList::RemovePlugin(const wxString& toRemoveId)
{
    auto it = m_pluginIdtoItem.find(toRemoveId);
    if (it == m_pluginIdtoItem.end()) return;


    std::shared_ptr<PluginPreviewWidget> plugin = it->second;

    m_pluginIdtoItem.erase(toRemoveId);

    m_itemToPluginId.erase(plugin);

    BaseWidgetList::RemoveWidget(plugin);
}

//////////////////////////////////////////////////////////////////////////

std::shared_ptr<PluginPreviewWidget> PluginPreviewWidgetList::CreateWidget(const std::shared_ptr<PluginContextInfo>& plugin)
{
    auto it = m_pluginIdtoItem.find(plugin->getPluginInfo()->name());
    if (it != m_pluginIdtoItem.end())
    {
        it->second->Update(plugin);

        return it->second;
    }

    std::shared_ptr<PluginPreviewWidget> widget = std::make_shared<PluginPreviewWidget>(this, plugin);

    const std::string& pluginId = plugin->getPluginInfo()->id();
    m_pluginIdtoItem[pluginId] = widget;
    m_itemToPluginId[widget] = pluginId;

    // Bind custom event
    widget->Bind(GUI_SELECT_PLUGIN_PREVIEW, &PluginPreviewWidgetList::OnWidgetClicked, this);

    return widget;
}

void PluginPreviewWidgetList::RemoveSelectedWidgetImpl(const std::shared_ptr<BaseWidget>& baseWidget)
{
	std::shared_ptr<PluginPreviewWidget> toRemoveWidget = std::static_pointer_cast<PluginPreviewWidget>(baseWidget);

    const wxString toRemoveId = m_itemToPluginId.at(toRemoveWidget);

    m_itemToPluginId.erase(toRemoveWidget);

    m_pluginIdtoItem.erase(toRemoveId);
}

void PluginPreviewWidgetList::RemoveAllWidgetsImpl()
{
    m_itemToPluginId.clear();
    m_pluginIdtoItem.clear();
}