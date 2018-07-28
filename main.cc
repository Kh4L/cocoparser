#include <cassert>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "json.h"

using json = nlohmann::json;

struct Annotation {
  Annotation(float x, float y, float w, float h, int category_id)
  : x(x), y(y), w(w), h(h), category_id(category_id){}
  float x;
  float y;
  float w;
  float h;
  int category_id;

  friend std::ostream& operator<<(std::ostream& os, Annotation& an);
};

std::ostream& operator<<(std::ostream& os, Annotation& an) {
  os << "Annotation(category_id=" << an.category_id
    << ",bbox = [" << an.x << "," << an.y
    << "," << an.w << "," << an.h << "])";
}

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
    auto start = std::chrono::system_clock::now();
    auto j = json::parse(source);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "Parsed in " << elapsed_seconds.count() << "s\n";
    // std::cout << typeid(j).name() << std::endl;
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

    start = std::chrono::system_clock::now();

    auto annotations = j.find("annotations");
    assert(annotations != j.end());
    int annotation_size = (*annotations).size();

    end = std::chrono::system_clock::now();
    elapsed_seconds = end-start;
    std::cout << "Loaded annotations in " << elapsed_seconds.count() << "s\n";

    for (auto& an : *annotations) {
      int image_id = an.find("image_id").value().get<int>();
      int category_id = an.find("category_id").value().get<int>();
      std::vector<float> bbox = an.find("bbox").value().get<std::vector<float>>();
      annotations_multimap_.insert(
          std::make_pair(image_id,
            Annotation(bbox[0], bbox[1], bbox[2], bbox[3], category_id)));
    }
  }

  std::multimap<int, Annotation>& GetAnnotationsMultimap() {
    return annotations_multimap_;
  }

 private:
  std::string filename_;
  std::vector<char> raw_json_;

  // multimap image_id -> annotations
  std::multimap<int, Annotation> annotations_multimap_;
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
  std::cout << "[Total]: " << argv[1] << " ran in " << elapsed_seconds.count() << "s\n";


  auto& amm = json_parser.GetAnnotationsMultimap();

  auto range = amm.equal_range(289343);

  for (auto it = range.first; it != range.second; ++it) {
    Annotation& an = it->second;
    std::cout << "Found Annotation for " << it->first << ": " << an << std::endl;
  }

  return 0;
}
