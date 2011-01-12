#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/spinctrl.h>

#include "wxraytracer.h"

#include <World.h>

#include <IRenderer.h>
#include <IBuilder.h>
#include <MultipleObjects.h>
//#include <Matte.h>
#include <Plane.h>

#include <Hammersley2D.h>
#include <Jittered2D.h>
#include <MultiJittered2D.h>
#include <NRooks2D.h>
#include <PureRandom2D.h>
#include <Regular2D.h>

#include <background.xpm>
#include <main.xpm>

#include "builders.h"



typedef void (*builderFunc)(WorldPtr);
struct BuilderSelector {
    wxString    name_;
    builderFunc func_;
};

const BuilderSelector BUILDERS[] = {
    { wxT("3-1"),   build3_1},
    { wxT("3-2"),   build3_2},
    { wxT("math"),  build_math},
    { wxT("debug"), build_debug}
};
const int NUM_BUILDERS = sizeof(BUILDERS)/sizeof(BUILDERS[0]);


enum SamplerType {
    SamplerTypeHammersley,
    SamplerTypeJitter,
    SamplerTypeMultiJitter,
    SamplerTypeNRooks,
    SamplerTypeRandom,
    SamplerTypeRegular,
};

struct SamplerSelector {
    wxString        name_;
    SamplerType     sampler;
};

const SamplerSelector SAMPLERS[] = {
    {wxT("Hammersley"),     SamplerTypeHammersley },
    {wxT("Jitter"),         SamplerTypeJitter },
    {wxT("Multijitter"),    SamplerTypeMultiJitter },
    {wxT("N Rooks"),        SamplerTypeNRooks },
    {wxT("Random"),         SamplerTypeRandom },
    {wxT("Regular"),        SamplerTypeRegular },
};
const int NUM_SAMPLERS = sizeof(SAMPLERS)/sizeof(SAMPLERS[0]);

SamplerPtr getSampler(SamplerType samplerMenuitem) {
    SamplerPtr sampler;
    switch(samplerMenuitem) {
        case SamplerTypeHammersley:
            sampler.reset(new Hammersley2D);
            break;

        case SamplerTypeJitter:
            sampler.reset(new Jittered2D);
            break;

        case SamplerTypeMultiJitter:
            sampler.reset(new MultiJittered2D);
            break;

        case SamplerTypeNRooks:
            sampler.reset(new NRooks2D);
            break;

        case SamplerTypeRandom:
            sampler.reset(new PureRandom);
            break;

        case SamplerTypeRegular:
        default:
            sampler.reset(new Regular2D);
            break;
    }
    return sampler;
}


const wxString DEFAULT_SAMPLE_NUMS[] = {
    wxT("1"),
    wxT("4"),
    wxT("9"),
    wxT("16"),
    wxT("25"),
    wxT("36"),
    wxT("49"),
    wxT("64")
};
const int NUM_DEFAULT_SAMPLE_NUMS = sizeof(DEFAULT_SAMPLE_NUMS) / sizeof (DEFAULT_SAMPLE_NUMS[0]);


const int DEBUG_FLAG_SAMPLER = 0x0001;


struct RenderParams {
    RenderParams() : builder_(0), numSamples_(1), pixelSize_(1.0f), transform_(false), debugFlags_(0) {}

    SamplerPtr  sampler_;
    builderFunc builder_;

    int numSamples_;
    float pixelSize_;
    bool transform_;
    int debugFlags_;
};


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

//IDs for menu items
enum COMMANDS {
    Menu_File_Quit = 100,
    Menu_File_Open,
    Menu_File_Save,

    Menu_Debug_Sampler,

    COMMAND_RENDER,
    COMMAND_STOP
};


BEGIN_EVENT_TABLE( wxraytracerFrame, wxFrame )
    EVT_MENU( Menu_File_Save, wxraytracerFrame::OnSaveFile )
    EVT_MENU( Menu_File_Open, wxraytracerFrame::OnOpenFile )
    EVT_MENU( Menu_File_Quit, wxraytracerFrame::OnQuit )

    EVT_COMMAND(ID_RENDER_COMPLETED, wxEVT_RENDER,
                wxraytracerFrame::OnRenderCompleted)

    EVT_BUTTON(COMMAND_RENDER, wxraytracerFrame::OnRenderStart)
    EVT_BUTTON(COMMAND_STOP, wxraytracerFrame::OnRenderStop)
    EVT_UPDATE_UI(COMMAND_RENDER, wxraytracerFrame::OnUpdateRender)


END_EVENT_TABLE()

wxraytracerFrame::wxraytracerFrame(const wxPoint& pos, const wxSize& size)
        : wxFrame((wxFrame *)NULL, -1, wxT( "Ray Tracer" ), pos, size) {
    wxMenu* menuFile = new wxMenu;

    menuFile->Append(Menu_File_Open, wxT("&Open..."   ));
    menuFile->Append(Menu_File_Save, wxT("&Save As..."));
    menuFile->AppendSeparator();
    menuFile->Append(Menu_File_Quit, wxT("E&xit"));

    menuFile->Enable(menuFile->FindItem(wxT("&Save As...")), FALSE);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, wxT("&File"  ));

    menuDebug_ = new wxMenu;
    menuDebug_->AppendCheckItem(Menu_Debug_Sampler, wxT("Shader"));
    menuBar->Append(menuDebug_, wxT("&Debug"  ));

    SetMenuBar( menuBar );

    create_toolbar();

    canvas = new RenderCanvas(this);

    CreateStatusBar();
    SetStatusText(wxT("Ready"));

    wxIcon icon(main_xpm);
    SetIcon(icon);

    wxStatusBar* statusBar = GetStatusBar();
    int widths[] = {150,300};
    statusBar->SetFieldsCount(2, widths);
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
    switch(canvas->getState()){
        case RenderCanvas::RENDERING:
            canvas->renderPause();
            return;
        case RenderCanvas::PAUSED:
            canvas->renderResume();
            return;
        case RenderCanvas::STOPPED:
        case RenderCanvas::WAITING:
        default:
            break;
    }

    wxMenu* menuFile = GetMenuBar()->GetMenu(0);
    menuFile->Enable(menuFile->FindItem(wxT( "&Open..."   )), FALSE);
    menuFile->Enable(menuFile->FindItem(wxT( "&Save As...")), TRUE );

    RenderParams rp;
    int selection = samplerCombo_->GetSelection();
    if (selection >= 0 ) {
        void* data = samplerCombo_->GetClientData(selection);
        assert(data);
        SamplerType sampleType = *reinterpret_cast<SamplerType*>(data);
        rp.sampler_ = getSampler(sampleType);
    }

    selection = builderCombo_->GetSelection();
    if (selection >= 0 ) {
        void* data = builderCombo_->GetClientData(selection);
        assert(data);
        rp.builder_ = reinterpret_cast<builderFunc>(data);
    }

    wxString numSamples = sampleNumCombo_->GetValue();
    long val = 1;
    numSamples.ToLong(&val, 10);
    rp.numSamples_  = val;
    rp.pixelSize_   = pixSizeSpin_->GetValue() / 100.0f;
    rp.transform_   = transformCheck_->IsChecked();
    rp.debugFlags_  |= menuDebug_->IsChecked(Menu_Debug_Sampler) ? DEBUG_FLAG_SAMPLER : 0x0000;

    canvas->renderStart(rp);
}

void wxraytracerFrame::OnRenderStop( wxCommandEvent& event ) {
    canvas->renderStop();
}

void wxraytracerFrame::OnRenderCompleted( wxCommandEvent& event ) {
    wxMenu* menuFile = GetMenuBar()->GetMenu(0);
    menuFile->Enable(menuFile->FindItem(wxT("&Open...")), TRUE);
    wxGetApp().SetStatusText(wxT("Rendering complete"));
}

void wxraytracerFrame::OnRenderPause( wxCommandEvent& event ) {
    canvas->renderPause();
    wxGetApp().SetStatusText( wxT( "Rendering paused" ) );
}

void wxraytracerFrame::OnRenderResume( wxCommandEvent& event ) {
    canvas->renderResume();
    wxGetApp().SetStatusText(wxT("Rendering..."));
}


void wxraytracerFrame::OnUpdateRender( wxUpdateUIEvent& event ) {
    switch(canvas->getState()){
        case RenderCanvas::RENDERING:
            event.SetText(wxT("Pause"));
            break;
        case RenderCanvas::PAUSED:
            event.SetText(wxT("Continue"));
            break;
        case RenderCanvas::STOPPED:
        case RenderCanvas::WAITING:
            event.SetText(wxT("Render"));
            break;
    }
}


void wxraytracerFrame::create_toolbar() {
    toolbar_ = CreateToolBar();

    renderBtn_ = new wxButton(toolbar_, COMMAND_RENDER, wxT("Render"));
    toolbar_->AddControl(renderBtn_);

    stopBtn_ = new wxButton(toolbar_, COMMAND_STOP, wxT("X"),
        wxDefaultPosition, wxSize(30,30));
    toolbar_->AddControl(stopBtn_);

    builderCombo_ = new wxComboBox(
        toolbar_, wxID_ANY, wxT(""),
        wxDefaultPosition, wxDefaultSize, NULL,
        wxCB_DROPDOWN | wxCB_READONLY);
    for (int i = 0; i < NUM_BUILDERS; i++) {
        const void* data = reinterpret_cast<const void*>(BUILDERS[i].func_);
        builderCombo_->Append(
            BUILDERS[i].name_,
            const_cast<void*>(data));
    }
    builderCombo_->SetSelection(0);
    toolbar_->AddControl(builderCombo_);


    samplerCombo_ = new wxComboBox(
        toolbar_, wxID_ANY, wxT(""),
        wxDefaultPosition, wxDefaultSize, NULL,
        wxCB_DROPDOWN | wxCB_READONLY);
    for (int i = 0; i < NUM_SAMPLERS; i++) {
        const void* data = reinterpret_cast<const void*>(&SAMPLERS[i].sampler);
        samplerCombo_->Append(
            SAMPLERS[i].name_,
            const_cast<void*>(data));
    }
    samplerCombo_->SetSelection(0);
    toolbar_->AddControl(samplerCombo_);

    transformCheck_ = new wxCheckBox(toolbar_, wxID_ANY, wxT("Disk"));
    toolbar_->AddControl(transformCheck_);

    sampleNumCombo_ = new wxComboBox(
        toolbar_, wxID_ANY, wxT("1"),
        wxDefaultPosition, wxDefaultSize,
        NUM_DEFAULT_SAMPLE_NUMS, DEFAULT_SAMPLE_NUMS);
    toolbar_->AddControl(sampleNumCombo_);

    pixSizeSpin_ = new wxSpinCtrl(toolbar_, wxID_ANY);
    pixSizeSpin_->SetRange(1,100); // In hundreths
    pixSizeSpin_->SetValue(100);
    toolbar_->AddControl(pixSizeSpin_);
}


RenderCanvas::RenderCanvas(wxWindow *parent) : wxScrolledWindow(parent),
        state_(WAITING), m_image(NULL), timer(NULL), updateTimer(this, ID_RENDER_UPDATE) {
    SetOwnBackgroundColour(wxColour(143,144,150));
}


RenderCanvas::~RenderCanvas() {
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


wxImage RenderCanvas::GetImage() {
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
    state_ = WAITING;
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


void RenderCanvas::renderPause() {
    if (thread != NULL)
        thread->Pause();

    updateTimer.Stop();

    if (timer != NULL)
        timer->Pause();

    state_ = PAUSED;
}


void RenderCanvas::renderStop() {
    state_ = STOPPED;
}


void RenderCanvas::renderResume() {
    if (thread != NULL)
        thread->Resume();

    updateTimer.Start();

    if (timer != NULL)
        timer->Resume();

    state_ = RENDERING;
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


void RenderCanvas::renderStart(const RenderParams& rp) {
    if ( DEBUG_FLAG_SAMPLER == (rp.debugFlags_ & DEBUG_FLAG_SAMPLER) ) {
        debugSampler(rp);
        return;
    }

    state_ = RENDERING;
    assert(rp.builder_);

    w.reset(new World());

    ViewPlane vp = w->get_viewplane();
    int width = 0, height = 0;
    GetSize(&width, &height);

    vp.hres = width;
    vp.vres = height;

    if ( rp.sampler_ ) {
        rp.sampler_->set_bundle_size(rp.numSamples_);
        vp.set_sampler(rp.sampler_);
    }

    vp.set_pixel_size( rp.pixelSize_ );
    vp.set_transform( rp.transform_ );
    w->set_viewplane(vp);

    wxGetApp().SetStatusText( wxT( "Building world..." ) );
    rp.builder_( w );

    // Builder may have reset the viewplane.
    vp = w->get_viewplane();

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


class SamplerDebug : public Tracer {
public:
    RGBColor trace_ray(const Ray& ray) const {
        return BLACK;
    }

    RGBColor trace_ray(const Ray ray, const int depth) const {
        return BLACK;
    }

};


void RenderCanvas::debugSampler(const RenderParams& rp) {
    assert ( rp.sampler_ );

    state_ = RENDERING;

    int width = 0, height = 0;
    GetSize(&width, &height);

    rp.sampler_->set_bundle_size(rp.numSamples_*16);

    ViewPlane vp;

    const int GRID_PIX_SIZE = 16;
    vp.hres = 1;
    vp.vres = 1;
//    vp.hres = width / GRID_PIX_SIZE;
//    vp.vres = height / GRID_PIX_SIZE;

    vp.set_pixel_size( 1.0f );
    vp.set_transform( rp.transform_ );
    vp.set_sampler( rp.sampler_ );

    wxBitmap bitmap(width, height, -1);
    wxMemoryDC dc;
    dc.SelectObject(bitmap);
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();

    drawGrid(dc, width, height, GRID_PIX_SIZE);

    dc.SetPen(wxNullPen);
    dc.SetBrush(*wxRED_BRUSH);

    const int GRID_PIX_SQUARE = GRID_PIX_SIZE * GRID_PIX_SIZE;
    const int GRID_PIX_QUARTER = GRID_PIX_SIZE / 4;
    for (int y = 0, counter=0; y < height; y+=GRID_PIX_SQUARE, counter++) {
        for (int x = 0; x < width; x+=GRID_PIX_SQUARE, counter++) {

/*
            // Checkerboard
            if ( 0 == (counter%2)) {
                dc.DrawRectangle(x, y, GRID_PIX_SIZE, GRID_PIX_SIZE);
            }
*/


            // One pixel per major grid pixel
//            int nx = x + GRID_PIX_SQUARE / 2;
//            int ny = y + GRID_PIX_SQUARE / 2;
//            dc.DrawRectangle(nx, ny, GRID_PIX_SIZE, GRID_PIX_SIZE);


            const SampleBundle2D& samples = vp.get_next();
            for ( SampleBundle2D::const_iterator sp = samples.begin();
                    sp != samples.end(); ++sp ) {
                int nx = (x - GRID_PIX_SQUARE/2) + GRID_PIX_SQUARE * (sp->x - 0.5f);
                int ny = (y - GRID_PIX_SQUARE/2) + GRID_PIX_SQUARE * (sp->y - 0.5f);
                dc.DrawCircle(nx+GRID_PIX_QUARTER, ny+GRID_PIX_QUARTER, GRID_PIX_QUARTER);
            }

        }
    }

    dc.SelectObject(wxNullBitmap);

    wxImage temp = bitmap.ConvertToImage();
    SetImage(temp);
    state_ = WAITING;
}


void RenderCanvas::drawGrid(wxDC& dc, int width, int height, int size) {
    for (int y=0, counter=0; y < height; y += size, counter++) {

        if ( 0 == (counter%16) )
            dc.SetPen(*wxBLACK_PEN);
        else
            dc.SetPen(*wxGREY_PEN);

        dc.DrawLine(0, y, width, y);
    }

    for (int x=0, counter=0; x < width; x += size, counter++) {
        if ( 0 == (counter%16) )
            dc.SetPen(*wxBLACK_PEN);
        else
            dc.SetPen(*wxGREY_PEN);

        dc.DrawLine(x, 0, x, height);
    }
}


void RenderCanvas::OnKeyDown( wxKeyEvent& key ){
    // const int code = key.GetKeyCode();
}


DEFINE_EVENT_TYPE(wxEVT_RENDER)

BEGIN_EVENT_TABLE( RenderCanvas, wxScrolledWindow )
    EVT_COMMAND(ID_RENDER_NEWPIXEL, wxEVT_RENDER,
                RenderCanvas::OnNewPixel)
    EVT_COMMAND(ID_RENDER_COMPLETED, wxEVT_RENDER,
                RenderCanvas::OnRenderCompleted)
    EVT_TIMER(ID_RENDER_UPDATE, RenderCanvas::OnTimerUpdate)

    EVT_KEY_DOWN(RenderCanvas::OnKeyDown)
END_EVENT_TABLE()


RenderPixel::RenderPixel(int _x, int _y, int _red, int _green, int _blue)
        : x(_x), y(_y), red(_red), green(_green), blue(_blue) {}


bool RenderThread::render(int x, int y, int red, int green, int blue) {
    if ( RenderCanvas::STOPPED == canvas->getState() )
        return false;

    pixels.push_back(RenderPixel(x, y, red, green, blue));
    if (timer->Time() - lastUpdateTime > 250)
        NotifyCanvas();

    TestDestroy();
    return true;
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
