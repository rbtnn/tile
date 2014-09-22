
#include "../common_headers.h"
#include "../common_functions.h"
#include "../wndproc_functions.h"
#include "./TilingWindowManager.h"
#include "./ConfigReader.h"

namespace Tile{
  void TilingWindowManager::init_main(){
    WNDCLASSEX winClass = make_wndclassex(m_hInstance, MainWndProc, m_main_class_name);
    ATOM atom = ::RegisterClassEx(&winClass);
    if (!atom){
      die("Error registering window class(main)");
    }

    m_main_hwnd = ::CreateWindowEx(0,
        m_main_class_name.c_str(), m_main_class_name.c_str(),
        0, 0, 0, 0, 0, NULL, NULL,
        m_hInstance, NULL);
    if (!m_main_hwnd){
      die("Error creating window(main)");
    }
  }
  void TilingWindowManager::init_statusline(){
    WNDCLASSEX winClass = make_wndclassex(m_hInstance, StatusLineWndProc, m_statusline_class_name);
    ATOM atom = ::RegisterClassEx(&winClass);
    if(!atom){
      die("Error registering window class(statusline)");
    }

    m_statusline_hwnd = make_toolwindow(m_hInstance, m_statusline_class_name);
    ::SetWindowPos(m_statusline_hwnd, HWND_TOPMOST,
        0, 0, get_statusline_width(), get_statusline_height(), SWP_NOACTIVATE);

    ::SetWindowLong(m_statusline_hwnd, GWL_EXSTYLE, ::GetWindowLong(m_statusline_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    ::SetLayeredWindowAttributes(m_statusline_hwnd, 0, 200, LWA_ALPHA);
  }
  void TilingWindowManager::init_border(){
    WNDCLASSEX winClass = make_wndclassex(m_hInstance, BorderWndProc, m_border_class_name);
    ATOM atom = ::RegisterClassEx(&winClass);
    if (!atom){
      die("Error registering window class(border)");
    }

    m_border_left_hwnd = make_toolwindow(m_hInstance, m_border_class_name);
    m_border_right_hwnd = make_toolwindow(m_hInstance, m_border_class_name);
    m_border_top_hwnd = make_toolwindow(m_hInstance, m_border_class_name);
    m_border_bottom_hwnd = make_toolwindow(m_hInstance, m_border_class_name);
  }
  bool TilingWindowManager::is_manageable(HWND const hwnd_){
    if(hwnd_ == NULL){
      return false;
    }

    HWND const parent = ::GetParent(hwnd_);
    int const style = ::GetWindowLong(hwnd_, GWL_STYLE);
    int const exstyle = ::GetWindowLong(hwnd_, GWL_EXSTYLE);
    bool const pok = (parent != 0 && is_manageable(parent));
    bool const istool = exstyle & WS_EX_TOOLWINDOW;
    bool const isapp = exstyle & WS_EX_APPWINDOW;

    std::string const classname = get_classname(hwnd_);
    if(classname == m_statusline_class_name){
      return false;
    }

    if (::GetWindowTextLength(hwnd_) == 0) {
      return false;
    }

    if (style & WS_DISABLED) {
      return false;
    }

    if((parent == 0 && ::IsWindowVisible(hwnd_)) || pok){
      if((!istool && parent == 0) || (istool && pok)){
        return true;
      }
      if(isapp && parent != 0){
        return true;
      }
    }

    return false;
  }
  void TilingWindowManager::regist_key(std::string key, void (Tile::TilingWindowManager::* f_)()){
    std::map<std::string, std::string> m = m_config->get_keys();
    auto it = m.find(key);
    if(it != std::end(m)){
      unsigned long MODKEY = MOD_ALT | MOD_CONTROL;
      if(0 < m[key].length()){
        unsigned char n = m[key].at(0);
        switch(n){
          case '!': n = '1'; MODKEY |= MOD_SHIFT; break;
          case '@': n = '2'; MODKEY |= MOD_SHIFT; break;
          case '#': n = '3'; MODKEY |= MOD_SHIFT; break;
          case '$': n = '4'; MODKEY |= MOD_SHIFT; break;
          case '%': n = '5'; MODKEY |= MOD_SHIFT; break;
          case '^': n = '6'; MODKEY |= MOD_SHIFT; break;
          case '&': n = '7'; MODKEY |= MOD_SHIFT; break;
          case '*': n = '8'; MODKEY |= MOD_SHIFT; break;
          case '(': n = '9'; MODKEY |= MOD_SHIFT; break;
          default:
                    if('A' <= n && n <= 'Z'){
                      MODKEY |= MOD_SHIFT;
                    }
                    else if('a' <= n && n <= 'z'){
                      n -= 0x20;
                    }
                    break;
        }
        m_keys.push_back(std::shared_ptr<Key>(new Key(m_main_hwnd, MODKEY, n, std::bind(f_, this))));
      }
    }
  }
  void TilingWindowManager::unmanage_all(){
    for(auto it = std::begin(m_workspaces); it < std::end(m_workspaces); it++){
      auto const hwnds = it->get_managed_hwnds();
      for(auto hwnd : hwnds){
        it->unmanage(hwnd);
        recovery.load(hwnd);
      }
    }
  }
  void TilingWindowManager::rescan(){
    unmanage_all();
    ::EnumWindows(scan, 0);
    arrange();
    try_focus_managed_window();
  }
  void TilingWindowManager::toggle_transparency_window(){
    HWND const foreground_hwnd = ::GetForegroundWindow();
    std::deque<HWND> const hwnds = m_workspace_it->get_managed_hwnds();
    auto const it = std::find(std::begin(hwnds), std::end(hwnds), foreground_hwnd);
    if(it != std::end(hwnds)){
      LONG const exstyle = ::GetWindowLong(foreground_hwnd, GWL_EXSTYLE);
      if(exstyle & WS_EX_LAYERED){
        ::SetWindowLong(foreground_hwnd, GWL_EXSTYLE, exstyle ^ WS_EX_LAYERED);
        ::SetLayeredWindowAttributes(foreground_hwnd, 0, 255, LWA_ALPHA);
      }
      else{
        ::SetWindowLong(foreground_hwnd, GWL_EXSTYLE, exstyle | WS_EX_LAYERED);
        ::SetLayeredWindowAttributes(foreground_hwnd, 0, 200, LWA_ALPHA);
      }
    }
  }
  void TilingWindowManager::swap_next(){
    auto const hwnds = m_workspace_it->get_managed_hwnds();
    HWND const foreground_hwnd = ::GetForegroundWindow();
    if(std::find(std::begin(hwnds), std::end(hwnds), foreground_hwnd) != std::end(hwnds)){
      for(auto it = std::begin(hwnds); it < std::end(hwnds);it++){
        if(*it == foreground_hwnd){
          it++;
          if(it == std::end(hwnds)){
            auto const top = hwnds.at(0);
            m_workspace_it->remanage_front(foreground_hwnd);
            m_workspace_it->remanage_back(top);
          }
          else{
            m_workspace_it->remanage_back(*it);
            m_workspace_it->remanage_back(foreground_hwnd);
          }
        }
        else{
          m_workspace_it->remanage_back(*it);
        }
      }
    }
  }
  void TilingWindowManager::swap_previous(){
    auto const hwnds = m_workspace_it->get_managed_hwnds();
    HWND const foreground_hwnd = ::GetForegroundWindow();
    if(std::find(std::begin(hwnds), std::end(hwnds), foreground_hwnd) != std::end(hwnds)){
      for(auto it = std::begin(hwnds); it < std::end(hwnds);it++){
        if(*it == foreground_hwnd){
          if(it == std::begin(hwnds)){
            auto const tail = hwnds.at(hwnds.size() - 1);
            m_workspace_it->remanage_back(foreground_hwnd);
            m_workspace_it->remanage_front(tail);
            break;
          }
          else{
            it--;
            m_workspace_it->remanage_back(foreground_hwnd);
            m_workspace_it->remanage_back(*it);
            it++;
          }
        }
        else{
          m_workspace_it->remanage_back(*it);
        }
      }
    }
  }
  void TilingWindowManager::run_process(){
    auto const path = m_config->get_run_process_path();
    if(path){
      if(exist_file(*path)){
        ::ShellExecute(NULL, NULL, path->c_str(), "", NULL, SW_SHOWDEFAULT);
      }
    }
  }
  void TilingWindowManager::exit_tile(){
    ::PostMessage(m_main_hwnd, WM_CLOSE, 0, 0);
  }
  void TilingWindowManager::kill_client(){
    HWND const foreground_hwnd = ::GetForegroundWindow();
    ::PostMessage(foreground_hwnd, WM_CLOSE, 0, 0);
  }
  void TilingWindowManager::next_layout(){
    m_layout_it++;
    if(m_layout_it == std::end(m_layouts)){
      m_layout_it = std::begin(m_layouts);
    }
    arrange();
    try_focus_managed_window();
  }
  void TilingWindowManager::next_focus(){
    bool is_next_focus = false;
    HWND const foreground_hwnd = ::GetForegroundWindow();
    if(0 < m_workspace_it->size()){
#ifdef DEBUG
      std::cout << "[TilingWindowManager::next_focus]" << std::endl;
      std::cout << m_workspace_it->size() << std::endl;
#endif
      ::SetForegroundWindow(m_workspace_it->at(0));
      for(auto hwnd : m_workspace_it->get_managed_hwnds()){
        if(foreground_hwnd == hwnd){
          is_next_focus = true;
        }
        else{
          if(is_next_focus){
            ::SetForegroundWindow(hwnd);
            break;
          }
        }
      }
    }
  }
  void TilingWindowManager::previous_focus(){
    bool is_next_focus = false;
    HWND const foreground_hwnd = ::GetForegroundWindow();
    if(0 < m_workspace_it->size()){
#ifdef DEBUG
      std::cout << "[TilingWindowManager::previous_focus]" << std::endl;
      std::cout << m_workspace_it->size() << std::endl;
#endif
      ::SetForegroundWindow(m_workspace_it->at(m_workspace_it->size() - 1));
      for(auto it = m_workspace_it->rbegin(); it < m_workspace_it->rend(); it++){
        if(foreground_hwnd == *it){
          is_next_focus = true;
        }
        else{
          if(is_next_focus){
            ::SetForegroundWindow(*it);
            break;
          }
        }
      }
    }
  }
  void TilingWindowManager::move_to_workspace_1(){
    move_to_workspace_of(0);
  }
  void TilingWindowManager::move_to_workspace_2(){
    move_to_workspace_of(1);
  }
  void TilingWindowManager::move_to_workspace_3(){
    move_to_workspace_of(2);
  }
  void TilingWindowManager::move_to_workspace_4(){
    move_to_workspace_of(3);
  }
  void TilingWindowManager::move_to_workspace_5(){
    move_to_workspace_of(4);
  }
  void TilingWindowManager::move_to_workspace_6(){
    move_to_workspace_of(5);
  }
  void TilingWindowManager::move_to_workspace_7(){
    move_to_workspace_of(6);
  }
  void TilingWindowManager::move_to_workspace_8(){
    move_to_workspace_of(7);
  }
  void TilingWindowManager::move_to_workspace_9(){
    move_to_workspace_of(8);
  }
  void TilingWindowManager::workspace_1(){
    workspace_of(0);
  }
  void TilingWindowManager::workspace_2(){
    workspace_of(1);
  }
  void TilingWindowManager::workspace_3(){
    workspace_of(2);
  }
  void TilingWindowManager::workspace_4(){
    workspace_of(3);
  }
  void TilingWindowManager::workspace_5(){
    workspace_of(4);
  }
  void TilingWindowManager::workspace_6(){
    workspace_of(5);
  }
  void TilingWindowManager::workspace_7(){
    workspace_of(6);
  }
  void TilingWindowManager::workspace_8(){
    workspace_of(7);
  }
  void TilingWindowManager::workspace_9(){
    workspace_of(8);
  }
  void TilingWindowManager::focus_window_to_master(){
    HWND const foreground_hwnd = ::GetForegroundWindow();
    if(0 < m_workspace_it->size()){
      m_workspace_it->remanage_front(foreground_hwnd);
      arrange();
      try_focus_managed_window();
    }
  }
  void TilingWindowManager::try_focus_managed_window(){
    HWND const foreground_hwnd = ::GetForegroundWindow();
    for(auto hwnd : m_workspace_it->get_managed_hwnds()){
      if(foreground_hwnd == hwnd){
        return;
      }
    }
    if(0 < m_workspace_it->size()){
      ::SetForegroundWindow(m_workspace_it->at(0));
      return;
    }
    ::SetForegroundWindow(::FindWindow("Progman", NULL));
  }
  void TilingWindowManager::move_to_workspace_of(unsigned int const i){
    HWND const hwnd = ::GetForegroundWindow();
    unsigned int n = 0;
    for(auto it = std::begin(m_workspaces); it < std::end(m_workspaces); it++){
      if(n == i && it != m_workspace_it){
        m_workspace_it->unmanage(hwnd);
        it->manage(hwnd, m_config->get_not_apply_style_to_classnames());
        arrange();
        try_focus_managed_window();
        ::ShowWindow(hwnd, SW_HIDE);
        break;
      }
      n++;
    }
  }
  void TilingWindowManager::workspace_of(unsigned int const i){
    unsigned int n = 0;
    if(i < m_workspaces.size()){
      for(auto it = std::begin(m_workspaces); it < std::end(m_workspaces); it++){
        if(n == i){
          if(m_workspace_it != it){
            for(auto hwnd : m_workspace_it->get_managed_hwnds()){
              ::ShowWindow(hwnd, SW_HIDE);
            }
            m_workspace_it = it;
            auto const hwnds = m_workspace_it->get_managed_hwnds();
            for(auto hwnd : hwnds){
              m_workspace_it->remanage_back(hwnd);
            }
            arrange();
            try_focus_managed_window();
            for(auto hwnd : m_workspace_it->get_managed_hwnds()){
              ::ShowWindow(hwnd, SW_SHOWNORMAL);
            }
          }
          break;
        }
        n++;
      }
    }
  }
  TilingWindowManager::TilingWindowManager(HINSTANCE const& hInstance_, std::string main_classname_, std::vector<Tile::Layout> layouts_, std::shared_ptr<ConfigReader> config_)
  : m_hInstance(hInstance_), m_config(config_){

    m_workspaces = {
      Tile::Workspace("1"),
      Tile::Workspace("2"),
      Tile::Workspace("3"),
      Tile::Workspace("4"),
      Tile::Workspace("5"),
      Tile::Workspace("6"),
      Tile::Workspace("7"),
      Tile::Workspace("8"),
      Tile::Workspace("9"),
    };
    m_workspace_it = std::begin(m_workspaces);

    m_layouts = layouts_;
    m_layout_it = std::begin(m_layouts);

    m_main_class_name = main_classname_;
    init_main();
    m_statusline_class_name = "TileStatusLine";
    init_statusline();
    m_border_class_name = "TileBorder";
    init_border();

    m_shellhookid = ::RegisterWindowMessage("SHELLHOOK");
  }
  TilingWindowManager::~TilingWindowManager(){
    unmanage_all();
    if (!m_main_hwnd){
      ::DestroyWindow(m_main_hwnd);
    }
    if (!m_statusline_hwnd){
      ::DestroyWindow(m_statusline_hwnd);
    }
    if (!m_border_left_hwnd){
      ::DestroyWindow(m_border_left_hwnd);
    }
    if (!m_border_right_hwnd){
      ::DestroyWindow(m_border_right_hwnd);
    }
    if (!m_border_top_hwnd){
      ::DestroyWindow(m_border_top_hwnd);
    }
    if (!m_border_bottom_hwnd){
      ::DestroyWindow(m_border_bottom_hwnd);
    }
    if (!m_hInstance){
      ::UnregisterClass(m_main_class_name.c_str(), m_hInstance);
      ::UnregisterClass(m_statusline_class_name.c_str(), m_hInstance);
      ::UnregisterClass(m_border_class_name.c_str(), m_hInstance);
    }
  }
  UINT const TilingWindowManager::start(){
    typedef BOOL (* RegisterShellHookWindowProc) (HWND);
    RegisterShellHookWindowProc RegisterShellHookWindow \
      = reinterpret_cast<RegisterShellHookWindowProc>(::GetProcAddress( ::GetModuleHandle("USER32.DLL"), "RegisterShellHookWindow"));
    if (!RegisterShellHookWindow){
      die("Could not find RegisterShellHookWindow");
    }

    RegisterShellHookWindow(m_main_hwnd);

    regist_key("exit_tile", &TilingWindowManager::exit_tile);
    regist_key("focus_window_to_master", &TilingWindowManager::focus_window_to_master);
    regist_key("kill_client", &TilingWindowManager::kill_client);
    regist_key("move_to_workspace_1", &TilingWindowManager::move_to_workspace_1);
    regist_key("move_to_workspace_2", &TilingWindowManager::move_to_workspace_2);
    regist_key("move_to_workspace_3", &TilingWindowManager::move_to_workspace_3);
    regist_key("move_to_workspace_4", &TilingWindowManager::move_to_workspace_4);
    regist_key("move_to_workspace_5", &TilingWindowManager::move_to_workspace_5);
    regist_key("move_to_workspace_6", &TilingWindowManager::move_to_workspace_6);
    regist_key("move_to_workspace_7", &TilingWindowManager::move_to_workspace_7);
    regist_key("move_to_workspace_8", &TilingWindowManager::move_to_workspace_8);
    regist_key("move_to_workspace_9", &TilingWindowManager::move_to_workspace_9);
    regist_key("next_focus", &TilingWindowManager::next_focus);
    regist_key("next_layout", &TilingWindowManager::next_layout);
    regist_key("previous_focus", &TilingWindowManager::previous_focus);
    regist_key("rescan", &TilingWindowManager::rescan);
    regist_key("toggle_transparency_window", &TilingWindowManager::toggle_transparency_window);
    regist_key("swap_next", &TilingWindowManager::swap_next);
    regist_key("swap_previous", &TilingWindowManager::swap_previous);
    regist_key("run_process", &TilingWindowManager::run_process);
    regist_key("workspace_1", &TilingWindowManager::workspace_1);
    regist_key("workspace_2", &TilingWindowManager::workspace_2);
    regist_key("workspace_3", &TilingWindowManager::workspace_3);
    regist_key("workspace_4", &TilingWindowManager::workspace_4);
    regist_key("workspace_5", &TilingWindowManager::workspace_5);
    regist_key("workspace_6", &TilingWindowManager::workspace_6);
    regist_key("workspace_7", &TilingWindowManager::workspace_7);
    regist_key("workspace_8", &TilingWindowManager::workspace_8);
    regist_key("workspace_9", &TilingWindowManager::workspace_9);

    ::EnumWindows(scan, 0);
    arrange();
    try_focus_managed_window();

    ::MSG msg;
    while (::GetMessage(&msg, NULL, 0, 0) > 0){
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
    }

    ::DeregisterShellHookWindow(m_main_hwnd);

    return msg.wParam;
  }
  void TilingWindowManager::arrange(){
    if(m_layout_it != std::end(m_layouts)){
      std::deque<HWND> hwnds;
      for(auto hwnd : m_workspace_it->get_managed_hwnds()){
        if(is_manageable(hwnd)){
          bool b = true;
          std::string const classname = get_classname(hwnd);
          for(auto c : m_config->get_ignore_classnames()){
            if(classname == c){
              b = false;
            }
          }
          if(b){
            hwnds.push_back(hwnd);
          }
        }
      }
      m_layout_it->arrange(hwnds);
    }
    if(is_manageable(::GetForegroundWindow())){
      unsigned int const n = 3;
      RECT rect;
      ::GetWindowRect(::GetForegroundWindow(), &rect);
      ::SetWindowPos(m_border_left_hwnd, HWND_TOPMOST,
          rect.left, rect.top, n, rect.bottom - rect.top, SWP_NOACTIVATE);
      ::SetWindowPos(m_border_right_hwnd, HWND_TOPMOST,
          rect.right - n, rect.top, n, rect.bottom - rect.top, SWP_NOACTIVATE);
      ::SetWindowPos(m_border_top_hwnd, HWND_TOPMOST,
          rect.left, rect.top, rect.right - rect.left, n, SWP_NOACTIVATE);
      ::SetWindowPos(m_border_bottom_hwnd, HWND_TOPMOST,
          rect.left, rect.bottom - n, rect.right - rect.left, n, SWP_NOACTIVATE);

      ::ShowWindow(m_border_top_hwnd, SW_SHOWNORMAL);
      ::ShowWindow(m_border_bottom_hwnd, SW_SHOWNORMAL);
      ::ShowWindow(m_border_left_hwnd, SW_SHOWNORMAL);
      ::ShowWindow(m_border_right_hwnd, SW_SHOWNORMAL);
    }
    else{
      ::ShowWindow(m_border_top_hwnd, SW_HIDE);
      ::ShowWindow(m_border_bottom_hwnd, SW_HIDE);
      ::ShowWindow(m_border_left_hwnd, SW_HIDE);
      ::ShowWindow(m_border_right_hwnd, SW_HIDE);
    }
  }
  void TilingWindowManager::manage(HWND hwnd_){
    if(is_manageable(hwnd_)){

#ifdef DEBUG
      HWND const parent = ::GetParent(hwnd_);
      std::string const classname = get_classname(hwnd_);

      std::cout << "[TilingWindowManager::manage]" << std::endl;
      std::cout << "hwnd:" << hwnd_ << std::endl;
      std::cout << "parent:" << parent << std::endl;
      std::cout << "classname:" << classname << std::endl;
      std::cout << std::endl;
#endif
      recovery.save(hwnd_);
      m_workspace_it->manage(hwnd_, m_config->get_not_apply_style_to_classnames());
    }
  }
  void TilingWindowManager::unmanage(HWND hwnd_){
    m_workspace_it->unmanage(hwnd_);
  }
  void TilingWindowManager::call_key_method(UINT const& i_){
    try{
      for(auto key : m_keys){
        if(key->hash() == i_){
          key->call();
        }
      }
    }
    catch(...){
      system_error("TilingWindowManager::call_key_method");
      exit_tile();
    }
  }
  void TilingWindowManager::redraw_statusline(){
    ::PostMessage(m_statusline_hwnd, WM_PAINT, 0, 0);
  }
  boost::optional<std::string> TilingWindowManager::get_layout_name() const{
    if(0 < m_layouts.size()){
      return m_layout_it->get_layout_name();
    }
    else{
      return boost::none;
    }
  }
  std::string TilingWindowManager::get_workspace_name() const{
    return m_workspace_it->get_workspace_name();
  }
  long TilingWindowManager::get_managed_window_size() const{
    return m_workspace_it->size();
  }
}

