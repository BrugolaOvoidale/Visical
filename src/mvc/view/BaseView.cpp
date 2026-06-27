#include "BaseView.hpp"
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/window.h>
#include <stdexcept>


BaseView::BaseView(wxWindow* self)
{
    if (!self)
		throw std::invalid_argument("self is nullptr");

    m_self = self;
}

/////////////////////////////////////////////////

std::optional<wxString> BaseView::OpenFileDialog(
    const wxString& message,
    const wxString& wildCard,
    const wxString& defaultPath)
{
    wxFileDialog openFileDialog(
        m_self,         // Parent window
        message,        // Dialog title
        defaultPath,    // Default directory (empty = current dir)
        wxEmptyString,  // Default filename
        wildCard,       // File wildcard filter
        wxFD_OPEN       // wxFD_SAVE | wxFD_OVERWRITE_PROMPT, etc.
    );

    if (openFileDialog.ShowModal() == wxID_OK)
    {
        return openFileDialog.GetPath();  // Returns the selected path
    }

    return std::nullopt;  // User canceled
}

std::optional<wxString> BaseView::SaveFileDialog(
    const wxString& message,
    const wxString& wildCard,
    const wxString& defaultPath,
    const wxString& defaultFileName)
{
    // Always include wxFD_SAVE and wxFD_OVERWRITE_PROMPT
    wxFileDialog saveFileDialog(
        m_self,             // Parent window
        message,            // Dialog title
        defaultPath,        // Default directory (empty = current dir)
        defaultFileName,    // Default filename
        wildCard,           // File wildcard filter
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT
    );

    if (saveFileDialog.ShowModal() == wxID_OK)
    {
        return saveFileDialog.GetPath();  // Returns the selected path
    }

    return std::nullopt;  // User canceled
}

std::optional<wxString> BaseView::OpenDirDialog(
    const wxString& message,
    int style,
    const wxString& defaultPath)
{
    wxDirDialog openFolderDialog(
        m_self,         // Parent window
        message,        // Dialog title
        defaultPath,    // Default directory (empty = current dir)
        style           // wxFD_SAVE | wxFD_OVERWRITE_PROMPT, etc.
    );

    if (openFolderDialog.ShowModal() == wxID_OK)
    {
        return openFolderDialog.GetPath();  // Returns the selected path
    }

    return std::nullopt;  // User canceled
}

bool BaseView::AskYesNo(const wxString& question)
{
    return wxMessageBox(question, "Confirm", wxICON_WARNING | wxYES_NO, m_self) == wxYES;
}

void BaseView::ShowError(const wxString& message, const wxString& title)
{
    wxMessageBox(message, title, wxICON_ERROR | wxOK, m_self);
}

void BaseView::ShowSuccess(const wxString& message, const wxString& title)
{
    wxMessageBox(message, title, wxICON_INFORMATION | wxOK, m_self);
}