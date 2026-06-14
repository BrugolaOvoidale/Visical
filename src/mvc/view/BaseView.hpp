#pragma once
#include <wx/filedlg.h>
#include <optional>


// Forward declarations
class wxWindow;


/**
 * @brief Wrapper class for common wxWidgets UI dialogs and user interactions.
 *
 * BaseView provides a simplified interface for file operations, directory selection,
 * and user notifications, abstracting the boilerplate code of wxWidgets dialogs.
 */
class BaseView
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~BaseView() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Displays a system dialog to open a file.
     * @param message The title or prompt of the dialog.
     * @param wildCard Filter string.
     * @param defaultPath Initial directory to show.
     * @return std::optional containing the full path if a file was selected,
     * or std::nullopt if the user cancelled.
     */
    std::optional<wxString> OpenFileDialog(
        const wxString& message,
        const wxString& wildCard,
        const wxString& defaultPath = wxEmptyString
    );

    /**
     * @brief Displays a system dialog to save a file.
     * @param message The title or prompt of the dialog.
     * @param wildCard Filter string.
     * @param defaultPath Initial directory.
     * @param defaultFileName Initial filename to suggest.
     * @return std::optional containing the chosen path, or std::nullopt if cancelled.
     */
    std::optional<wxString> SaveFileDialog(
        const wxString& message,
        const wxString& wildCard,
        const wxString& defaultPath = wxEmptyString,
        const wxString& defaultFileName = wxEmptyString
    );

    /**
     * @brief Displays a system dialog to select a directory.
     * @param message Prompt for the user.
     * @param style Dialog style flags.
     * @param defaultPath Initial directory.
     * @return std::optional containing the directory path, or std::nullopt if cancelled.
     */
    std::optional<wxString> OpenDirDialog(
        const wxString& message,
        int style = wxFD_DEFAULT_STYLE,
        const wxString& defaultPath = wxEmptyString
    );

    /**
     * @brief Prompts the user with a Yes/No question dialog.
     * @param question The question to display.
     * @return true if the user clicked "Yes", false otherwise.
     */
    bool AskYesNo(const wxString& question);

    /**
     * @brief Shows a modal error message box.
     * @param message The error details.
     * @param title The title of the error window.
     */
    void ShowError(
        const wxString& message,
        const wxString& title = "Error"
    );

    /**
     * @brief Shows a modal success/information message box.
     * @param message The information to display.
     * @param title The title of the window.
     */
    void ShowSuccess(
        const wxString& message,
        const wxString& title = "Success"
    );

protected:
    /**
     * @brief Constructs a BaseView associated with a parent window.
     * @param self Pointer to self.
     * @throw std::invalid_argument if self is nullptr.
     */
    BaseView(wxWindow* self);

private:
    // Pointer to self.
    wxWindow* m_self;
};
