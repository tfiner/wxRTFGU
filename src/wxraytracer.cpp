#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include "wxraytracer.h"

#include <World.h>

#include <IRenderer.h>
#include <IBuilder.h>
#include <MultipleObjects.h>
#include <Matte.h>
#include <Plane.h>

#include <Hammersley.h>
#include <Jittered.h>
#include <MultiJittered.h>
#include <NRooks.h>
#include <PureRandom.h>
#include <Regular.h>

#include <background.xpm>
#include <main.xpm>

#include "builders.h"




BEGIN_EVENT_TABLE(wxraytracerapp, wxApp)
END_EVENT_TABLE()

IMPLEMENT_APP(wxraytracerapp)

bool wxraytracerapp::OnInit() {
    wxInitAllImageHandlers();

    frame = new wxraytracerFrame(wxPoint(200,200), wxSize(700,500) );
    frame->Centre();
    frame->Show(TRUE);
    SetTopWindow(frame);
    return TRUE;
}

int wxraytracerapp::OnExit() {
    return 0;
}

void wxraytracerapp::SetStatusText(const wxString&  text, int number) {
    frame->SetStatusText(text, number);
}

/******************************************************************************/
/********************* wxraytracerFrame ***************************************/
/******************************************************************************/

BEGIN_EVENT_TABLE( wxraytracerFrame, wxFrame )
    EVT_MENU( Menu_Render_Start3_1, wxraytracerFrame::OnRenderStart )
    EVT_MENU( Menu_Render_Start3_2, wxraytracerFrame::OnRenderStart )
    EVT_MENU( Menu_Render_Start4_4a, wxraytracerFrame::OnRenderStart )
    EVT_MENU( Menu_Render_Math, wxraytracerFrame::OnRenderStart )
    EVT_MENU( Menu_Render_Pause, wxraytracerFrame::OnRenderPause )
    EVT_MENU( Menu_Render_Resume, wxraytracerFrame::OnRenderResume )

    EVT_MENU( Menu_File_Save, wxraytracerFrame::OnSaveFile )
    EVT_MENU( Menu_File_Open, wxraytracerFrame::OnOpenFile )
    EVT_MENU( Menu_File_Quit, wxraytracerFrame::OnQuit )
    EVT_COMMAND(ID_RENDER_COMPLETED, wxEVT_RENDER,
                wxraytracerFrame::OnRenderCompleted)

    EVT_MENU_RANGE( Menu_Sampler_Hammersley, Menu_Sampler_Regular, wxraytracerFrame::OnSamplerMenu )

END_EVENT_TABLE()

wxraytracerFrame::wxraytracerFrame(const wxPoint& pos, const wxSize& size)
        : wxFrame((wxFrame *)NULL, -1, wxT( "Ray Tracer" ), pos, size) {
    wxMenu *menuFile = new wxMenu;

    menuFile->Append(Menu_File_Open, wxT("&Open..."   ));
    menuFile->Append(Menu_File_Save, wxT("&Save As..."));
    menuFile->AppendSeparator();
    menuFile->Append(Menu_File_Quit, wxT("E&xit"));

    menuFile->Enable(menuFile->FindItem(wxT("&Save As...")), FALSE);

    wxMenu *menuRender = new wxMenu;

    menuRender->Append(Menu_Render_Start3_1 , wxT("&Start3_1" ));
    menuRender->Append(Menu_Render_Start3_2 , wxT("&Start3_2" ));
    menuRender->Append(Menu_Render_Start4_4a , wxT("&Start4_4a" ));
    menuRender->Append(Menu_Render_Math, wxT("&Start Math" ));

    menuRender->Append(Menu_Render_Pause , wxT("&Pause" ));
    menuRender->Append(Menu_Render_Resume, wxT("&Resume"));

    menuRender->Enable(menuRender->FindItem(wxT("&Start" )), TRUE );
    menuRender->Enable(menuRender->FindItem(wxT("&Pause" )), FALSE);
    menuRender->Enable(menuRender->FindItem(wxT("&Resume")), FALSE);

    wxMenu* menuSampler = new wxMenu;

    menuSampler->AppendCheckItem(Menu_Sampler_Hammersley, wxT("Hammersley"));
    menuSampler->AppendCheckItem(Menu_Sampler_Jitter, wxT("Jitter"));
    menuSampler->AppendCheckItem(Menu_Sampler_MultiJitter, wxT("MultiJitter"));
    menuSampler->AppendCheckItem(Menu_Sampler_NRooks, wxT("NRooks"));
    menuSampler->AppendCheckItem(Menu_Sampler_Random, wxT("Pure Random"));
    menuSampler->AppendCheckItem(Menu_Sampler_Regular, wxT("Regular"));

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile  , wxT("&File"  ));
    menuBar->Append(menuRender, wxT("&Render"));
    menuBar->Append(menuSampler, wxT("&Sampler"));

    SetMenuBar( menuBar );

    canvas = new RenderCanvas(this);

    CreateStatusBar();
    SetStatusText(wxT("Ready"));

    wxIcon icon(main_xpm);
    SetIcon(icon);

    wxStatusBar* statusBar = GetStatusBar();
    int widths[] = {150,300};
    statusBar->SetFieldsCount(2, widths);
}


void wxraytracerFrame::OnSamplerMenu( wxCommandEvent& event ) {
    const int ID = event.GetId();
    wxLogDebug(
        wxString::Format(wxT("Setting sampler to %d: %s"), ID, event.GetString().c_str() )
    );

    wxWindow* top = wxGetApp().GetTopWindow();
    wxFrame* frame = dynamic_cast<wxFrame*>(top);
    wxMenu* menu = 0;
    if ( frame )
        menu = frame->GetMenuBar()->GetMenu(2);

    if (!menu)
        return;

    const size_t NUM_ITEMS = menu->GetMenuItemCount();
    for (size_t i = 0; i < NUM_ITEMS; i++) {
        wxMenuItem* item = menu->FindItemByPosition(i);
        if ( ID != item->GetId() ) {
            item->Check(false);
        }
    }

    canvas->setSamplerMenu( static_cast<MenuEnums>(ID) );
}



void wxraytracerFrame::OnQuit( wxCommandEvent& WXUNUSED( event ) ) {
    Close();
}

void wxraytracerFrame::OnSaveFile( wxCommandEvent& WXUNUSED( event ) ) {
    wxString caption = wxT("Save File");

    wxString wildcard = wxT("BMP files (*.bmp)|*.bmp|"
                            "PNG files (*.png)|*.png|"
                            "JPEG files (*.jpg)|*.jpg|"
                            "TIFF files (*.tif)|*.tif");

    wxString defaultDir = wxEmptyString;
    wxString defaultFilename = wxT("render.bmp");

    wxFileDialog dialog(this, caption, defaultDir, defaultFilename, wildcard,
                        wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    dialog.SetPath(currentPath);

    if (dialog.ShowModal() == wxID_OK) {
        currentPath = dialog.GetPath();

        wxImage theImage = canvas->GetImage();
        theImage.SaveFile(currentPath);
    }
}

void wxraytracerFrame::OnOpenFile( wxCommandEvent& WXUNUSED( event ) ) {
    wxString caption = wxT("Choose a file");

    wxString wildcard = wxT("BMP files (*.bmp)|*.bmp|"
                            "PNG files (*.png)|*.png|"
                            "JPEG files (*.jpg)|*.jpg|"
                            "TIFF files (*.tif)|*.tif");

    wxString defaultDir = wxEmptyString;
    wxString defaultFilename = wxEmptyString;

    wxFileDialog dialog(this, caption, defaultDir, defaultFilename, wildcard,
                        wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    dialog.SetPath(currentPath);

    if (dialog.ShowModal() == wxID_OK) {
        currentPath = dialog.GetPath();
        wxImage* theImage = new wxImage();
        theImage->LoadFile(currentPath);

        if (!theImage->IsOk()) {
            delete theImage;
            wxMessageBox(wxT("Sorry, could not load file."));
        } else {
            canvas->SetImage(*theImage);
            wxMenu* menuFile = GetMenuBar()->GetMenu(0);
            menuFile->Enable(menuFile->FindItem(wxT( "&Save As..."  )) , TRUE);
        }
    }
}

void wxraytracerFrame::OnRenderStart( wxCommandEvent& event ) {
    wxMenu* menu = GetMenuBar()->GetMenu(1);
    menu->Enable(menu->FindItem(wxT("&Start" )), FALSE);
    menu->Enable(menu->FindItem(wxT("&Pause" )), TRUE );
    menu->Enable(menu->FindItem(wxT("&Resume")), FALSE);

    wxMenu* menuFile = GetMenuBar()->GetMenu(0);
    menuFile->Enable(menuFile->FindItem(wxT( "&Open..."   )), FALSE);
    menuFile->Enable(menuFile->FindItem(wxT( "&Save As...")), TRUE );

    canvas->renderStart(event.GetId());
}

void wxraytracerFrame::OnRenderCompleted( wxCommandEvent& event ) {
    wxMenu* menu = GetMenuBar()->GetMenu(1);
    menu->Enable(menu->FindItem(wxT("&Start" )), TRUE );
    menu->Enable(menu->FindItem(wxT("&Pause" )), FALSE);
    menu->Enable(menu->FindItem(wxT("&Resume")), FALSE);

    wxMenu* menuFile = GetMenuBar()->GetMenu(0);
    menuFile->Enable(menuFile->FindItem(wxT("&Open...")), TRUE);

    wxGetApp().SetStatusText(wxT("Rendering complete"));
}

void wxraytracerFrame::OnRenderPause( wxCommandEvent& event ) {
    wxMenu* menu = GetMenuBar()->GetMenu(1);
    menu->Enable(menu->FindItem(wxT("&Start" )), FALSE);
    menu->Enable(menu->FindItem(wxT("&Pause" )), FALSE);
    menu->Enable(menu->FindItem(wxT("&Resume")), TRUE );

    canvas->renderPause();

    wxGetApp().SetStatusText( wxT( "Rendering paused" ) );
}

void wxraytracerFrame::OnRenderResume( wxCommandEvent& event ) {
    wxMenu* menu = GetMenuBar()->GetMenu(1);
    menu->Enable(menu->FindItem(wxT("&Start" )), FALSE);
    menu->Enable(menu->FindItem(wxT("&Pause" )), TRUE );
    menu->Enable(menu->FindItem(wxT("&Resume")), FALSE);

    canvas->renderResume();

    wxGetApp().SetStatusText(wxT("Rendering..."));
}


RenderCanvas::RenderCanvas(wxWindow *parent)
        : wxScrolledWindow(parent),
        m_image(NULL),
        samplerEnum_(Menu_Sampler_Regular),
        timer(NULL),
        updateTimer(this, ID_RENDER_UPDATE) {
    SetOwnBackgroundColour(wxColour(143,144,150));
}

RenderCanvas::~RenderCanvas(void) {
    if (m_image != NULL)
        delete m_image;

    if (timer != NULL)
        delete timer;
}

void RenderCanvas::SetImage(wxImage& image) {
    if (m_image != NULL)
        delete m_image;

    m_image = new wxBitmap(image);

    SetScrollbars(10, 10, (int)(m_image->GetWidth()  / 10.0f),
                  (int)(m_image->GetHeight() / 10.0f), 0, 0, true);

    Refresh();
}

wxImage RenderCanvas::GetImage(void) {
    if (m_image != NULL)
        return m_image->ConvertToImage();

    return wxImage();
}

void RenderCanvas::OnDraw(wxDC& dc) {
    if (m_image != NULL && m_image->IsOk())
        wxBufferedDC bdc(&dc, *m_image);
}

void RenderCanvas::OnRenderCompleted( wxCommandEvent& event ) {
    if (timer != NULL) {
        long interval = timer->Time();

        wxTimeSpan timeElapsed(0, 0, 0, interval);
        wxString timeString = timeElapsed.Format(wxT("Elapsed Time: %H:%M:%S"));
        wxGetApp().SetStatusText( timeString, 1);

        delete timer;
        timer = NULL;
    }
}

void RenderCanvas::OnNewPixel( wxCommandEvent& event ) {
    //set up double buffered device context
    wxClientDC cdc(this);
    DoPrepareDC(cdc);
    wxBufferedDC bufferedDC(&cdc, *m_image);

    //iterate over all pixels in the event
    RenderPixels *pixelsUpdate =
        (RenderPixels *)event.GetClientData();

    for (RenderPixels::iterator itr = pixelsUpdate->begin();
            itr != pixelsUpdate->end(); ++itr) {
        RenderPixel& pixel = *itr;

        wxPen pen(wxColour(pixel.red, pixel.green, pixel.blue));
        bufferedDC.SetPen(pen);
        bufferedDC.DrawPoint(pixel.x, pixel.y);

        pixelsRendered++;
    }

    delete pixelsUpdate;
}


SamplerPtr getSampler(MenuEnums samplerMenuitem) {
    SamplerPtr sampler;
    switch(samplerMenuitem) {

        case Menu_Sampler_Hammersley:
            sampler.reset(new Hammersley);
            break;

        case Menu_Sampler_Jitter:
            sampler.reset(new Jittered);
            break;

        case Menu_Sampler_MultiJitter:
            sampler.reset(new MultiJittered);
            break;

        case Menu_Sampler_NRooks:
            sampler.reset(new NRooks);
            break;

        case Menu_Sampler_Random:
            sampler.reset(new PureRandom);
            break;

        case Menu_Sampler_Regular:
        default:
            sampler.reset(new Regular);
            break;
    }
    return sampler;
}


void RenderCanvas::setSamplerMenu( MenuEnums samplerMenuitem ) {
    samplerEnum_ = samplerMenuitem;
}


void RenderCanvas::renderPause(void) {
    if (thread != NULL)
        thread->Pause();

    updateTimer.Stop();

    if (timer != NULL)
        timer->Pause();
}

void RenderCanvas::renderResume(void) {
    if (thread != NULL)
        thread->Resume();

    updateTimer.Start();

    if (timer != NULL)
        timer->Resume();
}

void RenderCanvas::OnTimerUpdate( wxTimerEvent& event ) {
    if (timer == NULL)
        return;

    //percent
    float completed = (float)pixelsRendered / (float)pixelsToRender;

    wxString progressString = wxString::Format(wxT("Rendering...%d%%"),
                              (int)(completed*100));
    wxGetApp().SetStatusText( progressString , 0);

    //time elapsed
    long interval = timer->Time();

    wxTimeSpan timeElapsed(0, 0, 0, interval);

    //time remaining
    float remaining = 1.0f - completed;
    long msecRemain = (long)
                      (((double)interval / (completed*100)) * 100 * remaining);

    wxTimeSpan timeRemaining(0, 0, 0, msecRemain);

    wxString timeRemainString = timeRemaining.Format(wxT(" / ETA: %H:%M:%S"));
    wxString timeString = timeElapsed.Format(wxT("Elapsed Time: %H:%M:%S"));

    //only display ETA if something has been completed
    if (msecRemain >= 0)
        wxGetApp().SetStatusText( timeString + timeRemainString, 1);
    else
        wxGetApp().SetStatusText( timeString, 1);
}


void RenderCanvas::renderStart(int id) {
    w.reset(new World());

    ViewPlane vp = w->get_viewplane();
    int width = 0, height = 0;
    GetSize(&width, &height);

    vp.hres = width;
    vp.vres = height;

    vp.set_samples(16);
    vp.set_sampler(getSampler(samplerEnum_));

    w->set_viewplane(vp);


    wxGetApp().SetStatusText( wxT( "Building world..." ) );
    if (Menu_Render_Start3_1 == id )
        build3_1(w);
    else if (Menu_Render_Start3_2 == id )
        build3_2(w);
    else if (Menu_Render_Start4_4a == id )
        build4_4a(w);
    else if (Menu_Render_Math == id )
        build_math(w);

    wxGetApp().SetStatusText( wxT( "Rendering..." ) );

    pixelsRendered = 0;
    pixelsToRender = vp.hres * vp.vres;

    //set the background
    wxBitmap bitmap(vp.hres, vp.vres, -1);
    wxMemoryDC dc;
    dc.SelectObject(bitmap);
    dc.SetBackground(*wxGREY_BRUSH);
    dc.Clear();


    wxBitmap tile(background_xpm);

    for (int x = 0; x < vp.hres; x += 16) {
        for (int y = 0; y < vp.vres; y += 16)
            dc.DrawBitmap(tile, x, y, FALSE);
    }

    dc.SelectObject(wxNullBitmap);

    wxImage temp = bitmap.ConvertToImage();
    SetImage(temp);

    updateTimer.Start(250);

    //start timer
    timer = new wxStopWatch();

    thread.reset(new RenderThread(this, w));
    thread->Create();

    RendererPtr renderer(boost::dynamic_pointer_cast<IRenderer>(thread));

    w->set_renderer(renderer);
    thread->SetPriority(20);
    thread->Run();
}


DEFINE_EVENT_TYPE(wxEVT_RENDER)

BEGIN_EVENT_TABLE( RenderCanvas, wxScrolledWindow )
    EVT_COMMAND(ID_RENDER_NEWPIXEL, wxEVT_RENDER,
                RenderCanvas::OnNewPixel)
    EVT_COMMAND(ID_RENDER_COMPLETED, wxEVT_RENDER,
                RenderCanvas::OnRenderCompleted)
    EVT_TIMER(ID_RENDER_UPDATE, RenderCanvas::OnTimerUpdate)
END_EVENT_TABLE()



RenderPixel::RenderPixel(int _x, int _y, int _red, int _green, int _blue)
        : x(_x), y(_y), red(_red), green(_green), blue(_blue) {}


void RenderThread::render(int x, int y, int red, int green, int blue) {
    pixels.push_back(RenderPixel(x, y, red, green, blue));

    if (timer->Time() - lastUpdateTime > 250)
        NotifyCanvas();

    TestDestroy();
}

void RenderThread::NotifyCanvas() {
    lastUpdateTime = timer->Time();

    //copy rendered pixels into a new vector and reset
    RenderPixels *pixelsUpdate = new RenderPixels(pixels);
    pixels.clear();
    pixels.reserve(500);

    wxCommandEvent event(wxEVT_RENDER, ID_RENDER_NEWPIXEL);
    event.SetClientData(pixelsUpdate);
    canvas->GetEventHandler()->AddPendingEvent(event);
}

void RenderThread::OnExit() {
    NotifyCanvas();

    wxCommandEvent event(wxEVT_RENDER, ID_RENDER_COMPLETED);

    canvas->GetEventHandler()->AddPendingEvent(event);

    canvas->GetParent()->GetEventHandler()->AddPendingEvent(event);
}

void *RenderThread::Entry() {
    lastUpdateTime = 0;
    timer = new wxStopWatch();

    world->render_scene(); //for bare bones ray tracer only

    return NULL;
}
