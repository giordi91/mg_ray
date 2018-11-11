#pragma once
#undef  max
#include "middleware/json/json.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <DirectXMath.h>

inline bool file_exists(const std::string &name) {
  std::ifstream f(name.c_str());
  bool res = f.good();
  if (res) {
    return true;
  } else {
    std::cout << "ERROR file not found: " << name << std::endl;
    return false;
  }
}

inline nlohmann::json get_json_obj(std::string path) {

  bool res = file_exists(path);
  if (res) {
    // let s open the stream
    std::ifstream st(path);
    std::stringstream s_buffer;
    s_buffer << st.rdbuf();
    std::string s_buff = s_buffer.str();

    try {
      // try to parse
      nlohmann::json j_obj = nlohmann::json::parse(s_buff);
      return j_obj;
    } catch (...) {
      // if not lets throw an error
      std::cout << "ERROR, in parsing json file at path: \n"
                << path << std::endl;
      auto ex = std::current_exception();
      ex._RethrowException();
      return nlohmann::json();
    }
  } else {
    assert(0);
    return nlohmann::json();
  }
}

inline std::string get_file_name(std::string path) {
  // Remove directory if present.
  // Do this before extension removal incase directory has a period character.
  const size_t last_slash_idx = path.find_last_of("\\/");
  if (std::string::npos != last_slash_idx) {
    path.erase(0, last_slash_idx + 1);
  }

  // Remove extension if present.
  const size_t period_idx = path.rfind('.');
  if (std::string::npos != period_idx) {
    path.erase(period_idx);
  }
  return path;
}

// NOTE: requires c++17 filesystem
inline void list_files_in_folder(const char *folder_path,
                                 std::vector<std::string> &file_paths,
                                 std::string extension = "NONE") {
  bool should_filter = extension != "NONE";
  std::string _extension = "." + extension;
  auto program_p = std::experimental::filesystem::path(folder_path);
  auto dir_it = std::experimental::filesystem::directory_iterator(program_p);
  for (auto p : dir_it) {
    bool is_dir = std::experimental::filesystem::is_directory(p);
    if (!is_dir) {
      auto path = std::experimental::filesystem::path(p);

      if (should_filter && !(path.extension() == _extension)) {
        continue;
      }
      auto f_path = std::string(path.native().begin(), path.native().end());
      file_paths.push_back(f_path);
    }
  }
}

inline void load_file_into_string(const char *path, std::string &data) {
  // Read the Shader code from the file
  std::ifstream shaderStream(path, std::ios::in);
  if (shaderStream.is_open()) {
    std::string Line = "";
    while (getline(shaderStream, Line))
      data += "\n" + Line;
    shaderStream.close();
  } else {
    std::cout << "could not find file at path: " << path << std::endl;
  }
}

inline const std::string getFileName(const std::string& path)
{
      auto exp_path = std::experimental::filesystem::path(path);
	  return exp_path.filename().string();
}


template <typename T>
T get_value_if_in_json(const nlohmann::json &data, std::string key, T default_value) {
  if (data.find(key) != data.end()) {
    return data[key].get<T>();
  }
  return default_value;
}

template <>
inline DirectX::XMMATRIX get_value_if_in_json(const nlohmann::json &data, std::string key,
                                      DirectX::XMMATRIX default_value) {

  if (data.find(key) != data.end()) {
    auto &mat = data[key];
    return DirectX::XMMATRIX(
        mat[0].get<float>(), mat[1].get<float>(), mat[2].get<float>(),
        mat[3].get<float>(), mat[4].get<float>(), mat[5].get<float>(),
        mat[6].get<float>(), mat[7].get<float>(), mat[8].get<float>(),
        mat[9].get<float>(), mat[10].get<float>(), mat[11].get<float>(),
        mat[12].get<float>(), mat[13].get<float>(), mat[14].get<float>(),
        mat[15].get<float>());
  }
  return default_value;
}
/*
template <>
inline DirectX::XMFLOAT3 get_value_if_in_json(const nlohmann::json &data, std::string key,
                                      DirectX::XMFLOAT3 default_value) {
  if (data.find(key) != data.end()) {
    auto &vec = data[key];
    return DirectX::XMFLOAT3(vec[0].get<float>(), vec[1].get<float>(),
                     vec[2].get<float>());
  }
  return default_value;
}
template <>
inline DirectX::XMFLOAT4 get_value_if_in_json(const nlohmann::json &data, std::string key,
                                      DirectX::XMFLOAT4 default_value) {
  if (data.find(key) != data.end()) {
    auto &vec = data[key];
    return DirectX::XMFLOAT4(vec[0].get<float>(), vec[1].get<float>(),
                     vec[2].get<float>(),vec[3].get<float>() );
  }
  return default_value;
}

template <>
inline DirectX::XMVECTOR get_value_if_in_json(const nlohmann::json &data, std::string key,
                                      DirectX::XMVECTOR default_value) {
  if (data.find(key) != data.end()) {
    auto &vec = data[key];
    auto temp = DirectX::XMFLOAT4(vec[0].get<float>(), vec[1].get<float>(),
                     vec[2].get<float>(), vec[3].get<float>());
	return DirectX::XMLoadFloat4(&temp);
  }
  return default_value;
}

/*
template <>
inline glm::mat4 get_value_if_in_json(nlohmann::json &data, std::string key,
                                      glm::mat4 default_value) {
  if (data.find(key) != data.end()) {
    auto &mat = data[key];
    return glm::mat4(
        mat[0].get<float>(), mat[1].get<float>(), mat[2].get<float>(),
        mat[3].get<float>(), mat[4].get<float>(), mat[5].get<float>(),
        mat[6].get<float>(), mat[7].get<float>(), mat[8].get<float>(),
        mat[9].get<float>(), mat[10].get<float>(), mat[11].get<float>(),
        mat[12].get<float>(), mat[13].get<float>(), mat[14].get<float>(),
        mat[15].get<float>());
  }
  return default_value;
}
template <>
inline glm::vec3 get_value_if_in_json(nlohmann::json &data, std::string key,
                                      glm::vec3 default_value) {
  if (data.find(key) != data.end()) {
    auto &vec = data[key];
    return glm::vec3(vec[0].get<float>(), vec[1].get<float>(),
                     vec[2].get<float>());
  }
  return default_value;
}

template <>
inline glm::vec4 get_value_if_in_json(nlohmann::json &data, std::string key,
                                      glm::vec4 default_value) {
  if (data.find(key) != data.end()) {
    auto &vec = data[key];
    return glm::vec4(vec[0].get<float>(), vec[1].get<float>(),
                     vec[2].get<float>(), vec[3].get<float>());
  }
  return default_value;
}
template <>
inline glm::quat get_value_if_in_json(nlohmann::json &data, std::string key,
                                      glm::quat default_value) {
  if (data.find(key) != data.end()) {
    auto &vec = data[key];
    // need to be careful quaternion is initialize differntly
    // then how i export stuff, the glm quat is w,x,y,z
    // meanhwile i export x,y,z,w!
    return glm::quat(vec[3].get<float>(), vec[0].get<float>(),
                     vec[1].get<float>(), vec[2].get<float>());
  }
  return default_value;
}
*/
