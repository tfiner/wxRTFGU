#ifndef _WXRAYTRACER_H_
#define _WXRAYTRACER_H_

/**
 * Ray Tracer skeleton
 *
 * Author : Sverre Kvaale <sverre@kvaale.com>
 * Version: 0.8
 *
 */

#include <wx/wx.h>
#include <IRenderer.h>

#include <vector>
#include <boost/shared_ptr.hpp>

using namespace std;


class wxraytracerFrame;
class RenderCanvas;
class RenderThread;

class RenderPixel;
typedef vector<RenderPixel> RenderPixels;

class World;
typedef boost::shared_ptr<World> WorldPtr;

class RenderThread : public wxThread, public IRenderer {
public:
    RenderThread(RenderCanvas* c, WorldPtr w) : wxThread(wxTHREAD_JOINABLE), world(w), canvas(c) {}
    virtual void *Entry();
    virtual void OnExit();
    virtual void render(int x, int y, int red, int green, int blue);

private:
    void NotifyCanvas();

    WorldPtr world;
    RenderCanvas* canvas;

    RenderPixels pixels;
    wxStopWatch* timer;
    long lastUpdateTime;
};

typedef boost::shared_ptr<RenderThread> RenderThreadPtr;


class wxraytracerapp : public wxApp {
public:
    virtual bool OnInit();
    virtual int OnExit();
    virtual void SetStatusText(const wxString&  text, int number = 0);

private:
    wxraytracerFrame *frame;
    DECLARE_EVENT_TABLE()
};

class wxraytracerFrame : public wxFrame {
public:
    wxraytracerFrame(const wxPoint& pos, const wxSize& size);

    void OnSamplerMenu( wxCommandEvent& event );
    void OnQuit( wxCommandEvent& event );
    void OnOpenFile( wxCommandEvent& event );
    void OnSaveFile( wxCommandEvent& event );
    void OnRenderStart( wxCommandEvent& event );
    void OnRenderCompleted( wxCommandEvent& event );
    void OnRenderPause( wxCommandEvent& event );
    void OnRenderResume( wxCommandEvent& event );
    void OnUpdateRender( wxUpdateUIEvent& event );

private:
    wxToolBar* toolbar_;
    wxButton*   renderBtn_;
    wxComboBox* samplerCombo_;
    wxComboBox* builderCombo_;
    wxComboBox* sampleNumCombo_;
    wxSpinCtrl* pixSizeSpin_;
    RenderCanvas *canvas; //where the rendering takes place
    wxString currentPath; //for file dialogues
    DECLARE_EVENT_TABLE()
};


struct RenderParams;

class RenderCanvas: public wxScrolledWindow {
public:
    RenderCanvas(wxWindow *parent);
    virtual ~RenderCanvas();

    void SetImage(wxImage& image);
    wxImage GetImage();

    virtual void OnDraw(wxDC& dc);

    void renderIncreasePixelSize();
    void renderDecreasePixelSize();

    void renderStart(const RenderParams& rp);
    void renderPause();
    void renderResume();
    void OnRenderCompleted( wxCommandEvent& event );
    void OnTimerUpdate( wxTimerEvent& event );
    void OnNewPixel( wxCommandEvent& event );
    void OnKeyDown( wxKeyEvent& key );

    enum RenderState { WAITING, RENDERING, PAUSED };
    RenderState getState() const { return state_; }



private:
    RenderState state_;
    wxBitmap *m_image;
    WorldPtr w;

    RenderThreadPtr thread;
    wxStopWatch* timer;
    long pixelsRendered;
    long pixelsToRender;
    wxTimer updateTimer;

    DECLARE_EVENT_TABLE()
};


class RenderPixel {
public:
    RenderPixel(int x, int y, int red, int green, int blue);

public:
    int x, y;
    int red, green, blue;
};


DECLARE_EVENT_TYPE(wxEVT_RENDER, -1)
#define ID_RENDER_COMPLETED 100
#define ID_RENDER_NEWPIXEL  101
#define ID_RENDER_UPDATE    102


#endif
