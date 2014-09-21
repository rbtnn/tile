
#include "../common_headers.h"
#include "../common_functions.h"
#include "../wndproc_functions.h"
#include "./ConfigReader.h"

namespace Tile{
  ConfigReader::ConfigReader(){
    boost::property_tree::read_json("tile.json", m_pt);
  }
  boost::optional<std::string> ConfigReader::get_run_process_path() const{
    return m_pt.get_optional<std::string>("settings.RUN_PROCESS_PATH");
  }
  std::vector<std::string> ConfigReader::get_ignore_classnames() const{
    std::vector<std::string> xs;
    auto const children = m_pt.get_child_optional("settings.IGNORE_CLASSNAMES");
    if(children){
      for(auto const& x : *children){
        xs.push_back(x.second.get<std::string>(""));
      }
    }
    return xs;
  }
  std::vector<std::string> ConfigReader::get_not_apply_style_to_classnames() const{
    std::vector<std::string> xs;
    auto const children = m_pt.get_child_optional("settings.NOT_APPLY_STYLE_TO_CLASSNAMES");
    if(children){
      for(auto const& x : *children){
        xs.push_back(x.second.get<std::string>(""));
      }
    }
    return xs;
  }
  std::vector<std::string> ConfigReader::get_layout_method_names() const{
    std::vector<std::string> xs;
    auto const children = m_pt.get_child_optional("settings.LAYOUT_METHOD_NAMES");
    if(children){
      for(auto const& x : *children){
        xs.push_back(x.second.get<std::string>(""));
      }
    }
    return xs;
  }
  std::map<std::string, std::string> ConfigReader::get_keys() const{
    std::map<std::string, std::string> m;
    auto const children = m_pt.get_child_optional("keys");
    if(children){
      for(auto const& x : *children){
        m.insert(std::map<std::string, std::string>::value_type(
              x.first,
              x.second.get<std::string>("")
              ));
      }
    }
    return m;
  }
}

