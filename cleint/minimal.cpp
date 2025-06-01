
#include "Clnt.h"

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#ifndef wxHAS_IMAGES_IN_RESOURCES
    #include "../sample.xpm"
#endif

class MyApp : public wxApp
{
public:   
    virtual bool OnInit() wxOVERRIDE;
};

class MyFrame : public wxFrame
{
public:    
    MyFrame(const wxString& title);
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void on_save_button(wxCommandEvent& event);
    void on_load_button(wxCommandEvent& event);
    void on_clear_button(wxCommandEvent& event);
    void handle_message(const Mes& mes);
    void ShowMes(const Mes& mes);
private:    
    wxDECLARE_EVENT_TABLE();
    wxBoxSizer* main_sizer;
    wxPanel* main_panel;
    wxTextCtrl* input_field_;
    wxTextCtrl* display_field_;
    wxStatusBar* status_bar_;
    wxBoxSizer* button_sizer;
    wxButton* save_button;
    wxButton* load_button;
    wxButton* clear_button;    
    Client* client;  
};

enum
{    
    Minimal_Quit = wxID_EXIT,   
    Minimal_About = wxID_ABOUT
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(Minimal_Quit,  MyFrame::OnQuit)
    EVT_MENU(Minimal_About, MyFrame::OnAbout)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;
    MyFrame *frame = new MyFrame("Minimal wxWidgets App");
    frame->Show(true);
    return true;
}

MyFrame::MyFrame(const wxString& title)
       : wxFrame(NULL, wxID_ANY, title)       
{
    SetIcon(wxICON(sample));

#if wxUSE_MENUBAR    
    wxMenu *fileMenu = new wxMenu;
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(Minimal_About, "&About\tF1", "Show about dialog");
    fileMenu->Append(Minimal_Quit, "E&xit\tAlt-X", "Quit this program");
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");
    SetMenuBar(menuBar);
#else // !wxUSE_MENUBAR
    // If menus are not available add a button to access the about box
    wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* aboutBtn = new wxButton(this, wxID_ANY, "About...");
    aboutBtn->Bind(wxEVT_BUTTON, &MyFrame::OnAbout, this);
    sizer->Add(aboutBtn, wxSizerFlags().Center());
    SetSizer(sizer);
#endif // wxUSE_MENUBAR/!wxUSE_MENUBAR

#if wxUSE_STATUSBAR   
    CreateStatusBar(2);
    SetStatusText("Welcome to wxWidgets!");
#endif // wxUSE_STATUSBAR

    main_sizer = new wxBoxSizer(wxVERTICAL);
    main_panel = new wxPanel(this);

    display_field_ = new wxTextCtrl(main_panel, wxID_ANY, "",
        wxDefaultPosition, wxSize(-1, 200),
        wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2
    );
    main_sizer->Add(display_field_, 1, wxEXPAND | wxALL, 5);

    input_field_ = new wxTextCtrl(main_panel, wxID_ANY, "",
        wxDefaultPosition, wxSize(-1, 100),
        wxTE_MULTILINE | wxTE_PROCESS_ENTER
    );
    main_sizer->Add(input_field_, 0, wxEXPAND | wxALL, 5);

    display_field_->SetBackgroundColour(wxColour(255, 255, 255));
    input_field_->SetBackgroundColour(wxColour(255, 255, 230));

   button_sizer = new wxBoxSizer(wxHORIZONTAL);

   save_button = new wxButton(main_panel, wxID_ANY, "Start");
   load_button = new wxButton(main_panel, wxID_ANY, "Send");
   clear_button = new wxButton(main_panel, wxID_ANY, "Delete");

   save_button->Bind(wxEVT_BUTTON, &MyFrame::on_save_button, this);
   load_button->Bind(wxEVT_BUTTON, &MyFrame::on_load_button, this);
   clear_button->Bind(wxEVT_BUTTON, &MyFrame::on_clear_button, this);

   button_sizer->Add(clear_button, 0, wxRight, 10);
   button_sizer->Add(save_button, 0, wxRight, 10);
   button_sizer->Add(load_button, 0);
   main_sizer->Add(button_sizer, 0, wxALIGN_CENTRE | wxTOP | wxBOTTOM, 10);

   main_panel->SetSizer(main_sizer);

   client = new Client();
   client->set_handler([this](const Mes& msg) {
       CallAfter([this, msg]() {
           handle_message(msg);
           });
       });
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(wxString::Format
                 (
                    "Welcome to %s!\n"
                    "\n"
                    "This is the minimal wxWidgets sample\n"
                    "running under %s.",
                    wxGetLibraryVersionInfo().GetVersionString(),
                    wxGetOsDescription()
                 ),
                 "About wxWidgets minimal sample",
                 wxOK | wxICON_INFORMATION,
                 this);
}

void MyFrame::on_save_button(wxCommandEvent& event)
{
    if (client->start_client())
    {
        display_field_->AppendText(wxString("Connection OK\n"));
    }
    else
    {
        display_field_->AppendText(wxString("Connection failed\n"));
    }
    return;
}

void MyFrame::on_load_button(wxCommandEvent& event)
{
    wxString text = input_field_->GetValue();
    if (!text.IsEmpty()) {
        client->send_broadcast_message(text.ToStdString());
        input_field_->Clear();
        input_field_->SetFocus();
    }
    return;
}

void MyFrame::on_clear_button(wxCommandEvent& event)
{
    return;
}

void MyFrame::ShowMes(const Mes& mes)
{
    display_field_->AppendText(wxString(mes.content+'\n'));
}

void MyFrame::handle_message(const Mes& mes)
{
    if (mes.type == ACK)
    {
        display_field_->AppendText(wxString("Login OK\n"));
    }
    else if (mes.type == NAK)
    {
        display_field_->AppendText(wxString("Login failed\n"));
    }
    else
    {
        ShowMes(mes);
    }
}
