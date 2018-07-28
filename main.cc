#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include <chrono>
#include <ctime>

#include "json.h"

using json = nlohmann::json;

struct Annotation {

};

class JSONParserExample {
 public:
  JSONParserExample(char* filename)
    : filename_(filename) {

  }

  void load() {
    std::ifstream f(filename_);
    f.seekg(0, std::ios::end);
    raw_json_.reserve(f.tellg());

    f.seekg(0, std::ios::beg);

    raw_json_.assign((std::istreambuf_iterator<char>(f)),
                    std::istreambuf_iterator<char>());
  }

  void parse() {
    char *source = reinterpret_cast<char*>(raw_json_.data());
    auto j = json::parse(source);
    std::cout << typeid(j).name() << std::endl;
    auto images = j.find("images");
    assert(images != j.end());
    for (auto& im : *images) {
      //auto filename = im.find("file_name");
      int id = im.find("id").value().get<int>();
      std::string filename = im.find("file_name").value().get<std::string>();
      image_label_pairs_.push_back(std::make_pair(filename, id));
      // std::cout << filename.value() << " - " << typeid(filename.value()).name() << "\n";
      // std::cout << id << " ";
      // std::cout <<  filename->is_array() << std::endl;
    }
  }

 private:
  std::string filename_;
  std::vector<char> raw_json_;
  std::vector<std::pair<std::string, int>> image_label_pairs_;
};

int main(int argc, char**argv)
{
  std::string program_name(argv[0]);
  if (argc < 2) {
    std::cout << "Usage: ./" << program_name << " [JSON_FILE]\n";
    return 1;
  }

  JSONParserExample json_parser(argv[1]);
  auto start = std::chrono::system_clock::now();
  json_parser.load();
  json_parser.parse();
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  std::cout << "Loaded " << argv[1] << " in " << elapsed_seconds.count() << "s\n";

  return 0;
}
