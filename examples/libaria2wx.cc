/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
//
// Multi-threaded GUI program example for libaria2.  The downloads can
// be added using Download -> Add URI menu. The progress is shown in
// the main window.
//
// Compile and link like this:
// $ g++ -O2 -Wall -g -std=c++11 `wx-config --cflags` -o libaria2wx
// libaria2wx.cc `wx-config --libs` -laria2 -pthread
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <queue>

#include <wx/wx.h>

#include <aria2/aria2.h>

// Interface to send message to downloader thread from UI thread
struct Job {
  virtual ~Job(){};
  virtual void execute(aria2::Session* session) = 0;
};

class MainFrame;

// Interface to report back to UI thread from downloader thread
struct Notification {
  virtual ~Notification(){};
  virtual void notify(MainFrame* frame) = 0;
};

// std::queue<T> wrapper synchronized by mutex. In this example
// program, only one thread consumes from the queue, so separating
// empty() and pop() is not a problem.
template <typename T> class SynchronizedQueue {
public:
  SynchronizedQueue() {}
  ~SynchronizedQueue() {}
  void push(std::unique_ptr<T>&& t)
  {
    std::lock_guard<std::mutex> l(m_);
    q_.push(std::move(t));
  }
  std::unique_ptr<T> pop()
  {
    std::lock_guard<std::mutex> l(m_);
    std::unique_ptr<T> t = std::move(q_.front());
    q_.pop();
    return t;
  }
  bool empty()
  {
    std::lock_guard<std::mutex> l(m_);
    return q_.empty();
  }

private:
  std::queue<std::unique_ptr<T>> q_;
  std::mutex m_;
};

typedef SynchronizedQueue<Job> JobQueue;
typedef SynchronizedQueue<Notification> NotifyQueue;

// Job to shutdown downloader thread
struct ShutdownJob : public Job {
  ShutdownJob(bool force) : force(force) {}
  virtual void execute(aria2::Session* session)
  {
    aria2::shutdown(session, force);
  }
  bool force;
};

// Job to send URI to download and options to downloader thread
struct AddUriJob : public Job {
  AddUriJob(std::vector<std::string>&& uris, aria2::KeyVals&& options)
      : uris(uris), options(options)
  {
  }
  virtual void execute(aria2::Session* session)
  {
    // TODO check return value
    aria2::addUri(session, 0, uris, options);
  }
  std::vector<std::string> uris;
  aria2::KeyVals options;
};

int downloaderJob(JobQueue& jobq, NotifyQueue& notifyq);

// This struct is used to report download progress for active
// downloads from downloader thread to UI thread.
struct DownloadStatus {
  aria2::A2Gid gid;
  int64_t totalLength;
  int64_t completedLength;
  int downloadSpeed;
  int uploadSpeed;
  std::string filename;
};

class Aria2App : public wxApp {
public:
  virtual bool OnInit();
  virtual int OnExit();
};

class MainFrame : public wxFrame {
public:
  MainFrame(const wxString& title);
  void OnQuit(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);
  void OnCloseWindow(wxCloseEvent& event);
  void OnTimer(wxTimerEvent& event);
  void OnAddUri(wxCommandEvent& event);
  void UpdateActiveStatus(const std::vector<DownloadStatus>& v);

private:
  wxTextCtrl* text_;
  wxTimer timer_;
  JobQueue jobq_;
  NotifyQueue notifyq_;
  std::thread downloaderThread_;
  DECLARE_EVENT_TABLE()
};

enum { TIMER_ID = 1 };

enum { MI_ADD_URI = 1 };

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_CLOSE(MainFrame::OnCloseWindow)
EVT_TIMER(TIMER_ID, MainFrame::OnTimer)
EVT_MENU(MI_ADD_URI, MainFrame::OnAddUri)
END_EVENT_TABLE()

class AddUriDialog : public wxDialog {
public:
  AddUriDialog(wxWindow* parent);
  void OnButton(wxCommandEvent& event);
  wxString GetUri();
  wxString GetOption();

private:
  wxTextCtrl* uriText_;
  wxTextCtrl* optionText_;
  wxButton* okBtn_;
  wxButton* cancelBtn_;
  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(AddUriDialog, wxDialog)
EVT_BUTTON(wxID_ANY, AddUriDialog::OnButton)
END_EVENT_TABLE()

IMPLEMENT_APP(Aria2App)

bool Aria2App::OnInit()
{
  if (!wxApp::OnInit())
    return false;
  aria2::libraryInit();
  MainFrame* frame = new MainFrame(wxT("libaria2 GUI example"));
  frame->Show(true);
  return true;
}

int Aria2App::OnExit()
{
  aria2::libraryDeinit();
  return wxApp::OnExit();
}

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(640, 400)),
      timer_(this, TIMER_ID),
      downloaderThread_(downloaderJob, std::ref(jobq_), std::ref(notifyq_))
{
  wxMenu* downloadMenu = new wxMenu;
  downloadMenu->Append(MI_ADD_URI, wxT("&Add URI"), wxT("Add URI to download"));

  wxMenuBar* menuBar = new wxMenuBar();
  menuBar->Append(downloadMenu, wxT("&Download"));

  SetMenuBar(menuBar);

  // Show active downloads in textual manner
  wxPanel* panel = new wxPanel(this, wxID_ANY);
  wxBoxSizer* box = new wxBoxSizer(wxVERTICAL);
  box->Add(new wxStaticText(panel, wxID_ANY, wxT("Active Download(s)")));
  text_ = new wxTextCtrl(panel, wxID_ANY, wxT(""), wxDefaultPosition,
                         wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
  box->Add(text_, wxSizerFlags().Expand().Proportion(1));
  panel->SetSizer(box);
  // Finally start time here
  timer_.Start(900);
}

void MainFrame::OnAddUri(wxCommandEvent& WXUNUSED(event))
{
  AddUriDialog dlg(this);
  int ret = dlg.ShowModal();
  if (ret == 0) {
    if (dlg.GetUri().IsEmpty()) {
      return;
    }
    std::vector<std::string> uris = {std::string(dlg.GetUri().mb_str())};
    std::string optstr(dlg.GetOption().mb_str());
    aria2::KeyVals options;
    int keyfirst = 0;
    for (int i = 0; i < (int)optstr.size(); ++i) {
      if (optstr[i] == '\n') {
        keyfirst = i + 1;
      }
      else if (optstr[i] == '=') {
        int j;
        for (j = i + 1; j < (int)optstr.size(); ++j) {
          if (optstr[j] == '\n') {
            break;
          }
        }
        if (i - keyfirst > 0) {
          options.push_back(
              std::make_pair(optstr.substr(keyfirst, i - keyfirst),
                             optstr.substr(i + 1, j - i - 1)));
        }
        keyfirst = j + 1;
        i = j;
      }
    }
    jobq_.push(std::unique_ptr<Job>(
        new AddUriJob(std::move(uris), std::move(options))));
  }
}

void MainFrame::OnCloseWindow(wxCloseEvent& WXUNUSED(event))
{
  // On exit, we have to shutdown downloader thread and wait for it to
  // join. This is needed to execute graceful shutdown sequence of
  // aria2 session.
  jobq_.push(std::unique_ptr<Job>(new ShutdownJob(true)));
  downloaderThread_.join();
  Destroy();
}

void MainFrame::OnTimer(wxTimerEvent& event)
{
  while (!notifyq_.empty()) {
    std::unique_ptr<Notification> nt = notifyq_.pop();
    nt->notify(this);
  }
}

template <typename T> std::string abbrevsize(T size)
{
  if (size >= 1024 * 1024 * 1024) {
    return std::to_string(size / 1024 / 1024 / 1024) + "G";
  }
  else if (size >= 1024 * 1024) {
    return std::to_string(size / 1024 / 1024) + "M";
  }
  else if (size >= 1024) {
    return std::to_string(size / 1024) + "K";
  }
  else {
    return std::to_string(size);
  }
}

wxString towxs(const std::string& s) { return wxString(s.c_str(), wxConvUTF8); }

void MainFrame::UpdateActiveStatus(const std::vector<DownloadStatus>& v)
{
  text_->Clear();
  for (auto& a : v) {
    *text_ << wxT("[") << towxs(aria2::gidToHex(a.gid)) << wxT("] ")
           << towxs(abbrevsize(a.completedLength)) << wxT("/")
           << towxs(abbrevsize(a.totalLength)) << wxT("(")
           << (a.totalLength != 0 ? a.completedLength * 100 / a.totalLength : 0)
           << wxT("%)") << wxT(" D:") << towxs(abbrevsize(a.downloadSpeed))
           << wxT(" U:") << towxs(abbrevsize(a.uploadSpeed)) << wxT("\n")
           << wxT("File:") << towxs(a.filename) << wxT("\n");
  }
}

AddUriDialog::AddUriDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, wxT("Add URI"), wxDefaultPosition,
               wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
  wxPanel* panel = new wxPanel(this, wxID_ANY);
  wxBoxSizer* box = new wxBoxSizer(wxVERTICAL);
  // URI text input
  box->Add(new wxStaticText(panel, wxID_ANY, wxT("URI")));
  uriText_ = new wxTextCtrl(panel, wxID_ANY);
  box->Add(uriText_, wxSizerFlags().Align(wxGROW));
  // Option multi text input
  box->Add(new wxStaticText(
      panel, wxID_ANY, wxT("Options (key=value pair per line, e.g. dir=/tmp")));
  optionText_ = new wxTextCtrl(panel, wxID_ANY, wxT(""), wxDefaultPosition,
                               wxDefaultSize, wxTE_MULTILINE);
  box->Add(optionText_, wxSizerFlags().Align(wxGROW));
  // buttons
  wxPanel* btnpanel = new wxPanel(panel, wxID_ANY);
  box->Add(btnpanel);
  wxBoxSizer* btnbox = new wxBoxSizer(wxHORIZONTAL);
  // OK button
  okBtn_ = new wxButton(btnpanel, wxID_ANY, wxT("OK"));
  btnbox->Add(okBtn_);
  // Cancel button
  cancelBtn_ = new wxButton(btnpanel, wxID_ANY, wxT("Cancel"));
  btnbox->Add(cancelBtn_);

  panel->SetSizer(box);
  btnpanel->SetSizer(btnbox);
}

void AddUriDialog::OnButton(wxCommandEvent& event)
{
  int ret = -1;
  if (event.GetEventObject() == okBtn_) {
    ret = 0;
  }
  EndModal(ret);
}

wxString AddUriDialog::GetUri() { return uriText_->GetValue(); }

wxString AddUriDialog::GetOption() { return optionText_->GetValue(); }

struct DownloadStatusNotification : public Notification {
  DownloadStatusNotification(std::vector<DownloadStatus>&& v) : v(v) {}
  virtual void notify(MainFrame* frame) { frame->UpdateActiveStatus(v); }
  std::vector<DownloadStatus> v;
};

struct ShutdownNotification : public Notification {
  ShutdownNotification() {}
  virtual void notify(MainFrame* frame) { frame->Close(); }
};

int downloaderJob(JobQueue& jobq, NotifyQueue& notifyq)
{
  // session is actually singleton: 1 session per process
  aria2::Session* session;
  // Use default configuration
  aria2::SessionConfig config;
  config.keepRunning = true;
  session = aria2::sessionNew(aria2::KeyVals(), config);
  auto start = std::chrono::steady_clock::now();
  for (;;) {
    int rv = aria2::run(session, aria2::RUN_ONCE);
    if (rv != 1) {
      break;
    }
    auto now = std::chrono::steady_clock::now();
    auto count =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - start)
            .count();
    while (!jobq.empty()) {
      std::unique_ptr<Job> job = jobq.pop();
      job->execute(session);
    }
    if (count >= 900) {
      start = now;
      std::vector<aria2::A2Gid> gids = aria2::getActiveDownload(session);
      std::vector<DownloadStatus> v;
      for (auto gid : gids) {
        aria2::DownloadHandle* dh = aria2::getDownloadHandle(session, gid);
        if (dh) {
          DownloadStatus st;
          st.gid = gid;
          st.totalLength = dh->getTotalLength();
          st.completedLength = dh->getCompletedLength();
          st.downloadSpeed = dh->getDownloadSpeed();
          st.uploadSpeed = dh->getUploadSpeed();
          if (dh->getNumFiles() > 0) {
            aria2::FileData file = dh->getFile(1);
            st.filename = file.path;
          }
          v.push_back(std::move(st));
          aria2::deleteDownloadHandle(dh);
        }
      }
      notifyq.push(std::unique_ptr<Notification>(
          new DownloadStatusNotification(std::move(v))));
    }
  }
  int rv = aria2::sessionFinal(session);
  // Report back to the UI thread that this thread is going to
  // exit. This is needed when user pressed ctrl-C in the terminal.
  notifyq.push(std::unique_ptr<Notification>(new ShutdownNotification()));
  return rv;
}
