#include "ImagePreprocess.hpp"
#include <wx/image.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <gui_elements/parameter/ParameterWidget.hpp>
#include <gui_elements/parameter/ParameterWidgetList.hpp>
#include <parameter/ParameterInfo.hpp>


ImagePreprocess::ImagePreprocess(
    wxWindow* parent,
    const wxString& title)
    : wxFrame(parent, wxID_ANY, title)
{
    wxIcon icon;
    icon.LoadFile("resources/appIcon.ico", wxBITMAP_TYPE_ICO);
    SetIcon(icon);


    BuildUI();

    Hide();

    Bind(wxEVT_CLOSE_WINDOW, &ImagePreprocess::OnClose, this);
}

////////////////////////////////////////////////////////////

void ImagePreprocess::Open()
{
    if (!IsIconized())  Restore();

    if (!IsShown())     Show();

    if (!HasFocus())    SetFocus();
}

void ImagePreprocess::SetParameters(
    const std::vector<std::shared_ptr<ParameterInfo>>& params,
    DetectionView::ParameterLocation where)
{
    switch (where)
    {
        case DetectionView::ParameterLocation::PreProcessing:
            m_preProcParamsList->SetParameters(params);

            break;

        default:
			break;
    }
}

void ImagePreprocess::UpdateParameter(const std::shared_ptr<ParameterInfo>& param)
{
    std::shared_ptr<ParameterWidget> widget = m_preProcParamsList->GetWidget(param->name());
    
    if (widget)
    {
        widget->Update(param);
    }
}

void ImagePreprocess::MarkParameterAsDirty(
    const wxString& paramId,
    bool isDirty)
{
    std::shared_ptr<ParameterWidget> widget = m_preProcParamsList->GetWidget(paramId);

    if (widget)
    {
        widget->MarkAsDirty(isDirty);
    }
}

///////////////////////////////////////////////////////////

void ImagePreprocess::BuildUI()
{
    // Use a vertical BoxSizer to stack the sections
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    // Camera parameters
    wxStaticBoxSizer* preProcParamsBox = new wxStaticBoxSizer(wxVERTICAL, this, "Pre-detection");

    m_preProcParamsList = new ParameterWidgetList(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    m_preProcParamsList->SetMinSize(FromDIP(wxSize(500, 200)));

    preProcParamsBox->Add(m_preProcParamsList, 1, wxEXPAND | wxALL, 0);

    vbox->Add(preProcParamsBox, 1, wxEXPAND | wxALL, 0);  // Add to vertical layout

    SetBackgroundColour(*wxWHITE);

    // Set the sizer to the panel
    SetSizerAndFit(vbox);

    SetMinSize(GetSize());

    SetSize(GetSize());
}

void ImagePreprocess::OnClose(wxCloseEvent& event)
{
    if (event.CanVeto())
    {
        event.Veto();

        Hide();

        return;
    }

    event.Skip(); // allow normal destruction
}
