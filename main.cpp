#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <vector>
#include <thread>

// Prototypy tvých funkcí
void ModuloString(const std::string& URL, std::map<std::string, std::string> &query_,
                  std::string &path_);
void ModuloString_view(const std::string& URL, std::map<std::string, std::string> &query_,
                       std::string &path_);
void CompareString(const std::string& URL, std::map<std::string, std::string> &query_,
                   std::string &path_);
void CompareString_view(const std::string& URL, std::map<std::string, std::string> &query_,
                        std::string &path_);
std::vector<std::string> SplitRequestText(const std::string& to_split,
                                          const std::string& separator);


void logMapDifference(const std::map<std::string, std::string>& map1,
                      const std::map<std::string, std::string>& map2,
                      std::ofstream& logFile) {
  std::stringstream ss;
  ss << "Differences:\n";

  for (const auto& [key, value] : map1) {
    if (map2.find(key) == map2.end()) {
      ss << "Key " << key << " exists in first map but not in second.\n";
    } else if (map2.at(key) != value) {
      ss << "Key " << key << " differs: " << value << " (first) vs " << map2.at(key) << " (second).\n";
    }
  }

  for (const auto& [key, value] : map2) {
    if (map1.find(key) == map1.end()) {
      ss << "Key " << key << " exists in second map but not in first.\n";
    }
  }

  logFile << ss.str();
}

uint32_t repeat_test = 1000;

int main() {
  std::ofstream csvFile("results.csv");
  std::ofstream errorLog("errors.log");
  errorLog << "Error log\n\n";
  csvFile << "query_count,num_urls,test,ModuloString,ModuloString_view,"
             "CompareString,CompareString_view\n";

  std::vector<void (*)(const std::string&,std::map<std::string, std::string>&,
                       std::string&)> algorithms = {
      ModuloString, ModuloString_view, CompareString, CompareString_view};

  std::vector<int> num_urls_list;
  for (uint64_t i = 1; i <= 2501; i += 10) {
    num_urls_list.emplace_back(i);
  }

  std::vector<int> query_counts = {0, 3, 5, 8, 10};

  int iterator = 0;

  std::map<std::string, std::string>query_ref;
  std::map<std::string, std::string>query_test;
  std::string path_ref;
  std::string path_test;

  for (int test = 1; test <= repeat_test; ++test) {
    for (auto& query_count : query_counts) {
      for (auto& num_urls : num_urls_list) {
        std::vector<std::string> urls;

        query_ref.clear();
        path_test="";

        if (query_count != 0) {
          // Generate URLs first
          for (int url_count = 1; url_count <= num_urls; ++url_count) {
            path_ref = "http://example.com/path" + std::to_string(url_count);
            std::string url = path_ref + "?";

            // Generate random queries
            for (int i = 0; i < query_count; ++i) {
              url += "query" + std::to_string(i) + "=value" +
                     std::to_string(i) + "&";
              query_ref.emplace(std::string("query" + std::to_string(i)), std::string("value" + std::to_string(i)));
            }
            urls.push_back(url);
          }
        } else {
          for (int url_count = 1; url_count <= num_urls; ++url_count) {
            std::string url =
                "http://example.com/path" + std::to_string(url_count);
            urls.push_back(url);
          }
        }
        csvFile << query_count << ',' << num_urls << ',' << test;

        for (auto& algo : algorithms) {
          std::string algo_name;
          if (algo == ModuloString) {
            algo_name = "ModuloString";
          } else if (algo == ModuloString_view) {
            algo_name = "ModuloString_view";
          } else if (algo == CompareString) {
            algo_name = "CompareString";
          } else if (algo == CompareString_view) {
            algo_name = "CompareString_view";
          }
          auto start = std::chrono::high_resolution_clock::now();
          auto stop = std::chrono::high_resolution_clock::now();
          long long duration = 0;
          for (const auto& url : urls) {
            start = std::chrono::high_resolution_clock::now();
            try {
              algo(url, query_test, path_test);
            } catch (...) {
              std::cout << "error: " << algo_name << "\n";
            }
            stop = std::chrono::high_resolution_clock::now();
            duration +=
                std::chrono::duration_cast<std::chrono::microseconds>(stop -
                                                                      start)
                    .count();
            auto tmp_find = url.find("?");
            path_ref = url.substr(0,tmp_find);
            if (query_ref != query_test || path_ref != path_test) {
              errorLog << "Error in " << algo_name << " for URL: " << url << std::endl;
              if (path_ref != path_test) {
                errorLog << "Reference path: " << path_ref << "\tTest path: " << path_test << "\n";
              }
              if (query_ref != query_test) {
                logMapDifference(query_ref, query_test, errorLog);
              }
            }
          }


          csvFile << "," << duration;
        }

        csvFile << "\n";
        iterator++;
      }
    }
  }

  errorLog.close();
  csvFile.close();

  // Spustit Python skript
  std::system(
      "python3 -m venv venv && "
      "source venv/bin/activate && "
      "pip install pandas matplotlib seaborn && "
      "python3 ../plot.py && "
      "deactivate");

  return 0;
}

void ModuloString(const std::string& URL, std::map<std::string, std::string> &query_,
                  std::string &path_) {

  query_.clear();
  auto find = URL.find('?');
  if (find == std::string::npos) {
    path_ = URL;
    return;
  }
  path_ = URL.substr(0, find);
  auto pre_query = URL.substr(find + 1, URL.size());
  std::replace(pre_query.begin(), pre_query.end(), '?', '&');
  std::replace(pre_query.begin(), pre_query.end(), '=', '&');

  auto query = SplitRequestText(pre_query, "&");
  auto query_size = query.size();

  if (query_size % 2 != 0) {
    --query_size;
  }

  for (uint64_t i = 0; i < query_size; i += 2) {
    query_.emplace(query[i], query[i + 1]);
  }
}

void ModuloString_view(const std::string& URL, std::map<std::string, std::string> &query_,
                       std::string &path_) {
  query_.clear();
  auto find = URL.find('?');

  std::string_view URL_view = URL;
  std::string_view path_view;
  std::string_view pre_query_view;

  if (find == std::string::npos) {
    path_ = URL;
    return;
  }

  path_view = URL_view.substr(0, find);
  pre_query_view = URL_view.substr(find + 1);

  std::string pre_query(pre_query_view);
  std::replace(pre_query.begin(), pre_query.end(), '?', '&');
  std::replace(pre_query.begin(), pre_query.end(), '=', '&');

  auto query = SplitRequestText(pre_query, "&");
  auto query_size = query.size();

  if (query_size % 2 != 0) {
    --query_size;
  }

  for (uint64_t i = 0; i < query_size; i += 2) {
    query_.emplace(query[i], query[i + 1]);
  }

  path_ = std::string(path_view);
}

void CompareString(const std::string& URL, std::map<std::string, std::string> &query_,
                   std::string &path_) {
  query_.clear();
  auto find = URL.find('?');
  if (find == std::string::npos) {
    path_ = URL;
    return;
  }
  path_ = URL.substr(0, find);
  auto pre_query = URL.substr(find + 1, URL.size());
  std::replace(pre_query.begin(), pre_query.end(), '?', '&');
  std::replace(pre_query.begin(), pre_query.end(), '=', '&');

  auto query = SplitRequestText(pre_query, "&");
  auto query_size = query.size();
  for (uint64_t i = 0; i < query_size; i += 2) {
    if (i + 1 >= query_size) {
      break;
    }
    query_.emplace(query[i], query[i + 1]);
  }
}

void CompareString_view(const std::string& URL, std::map<std::string, std::string> &query_,
                        std::string &path_) {
  query_.clear();
  auto find = URL.find('?');

  std::string_view URL_view = URL;
  std::string_view path_view;
  std::string_view pre_query_view;

  if (find == std::string::npos) {
    path_ = URL;
    return;
  }

  path_view = URL_view.substr(0, find);

  pre_query_view = URL_view.substr(find + 1);

  std::string pre_query(pre_query_view);
  std::replace(pre_query.begin(), pre_query.end(), '?', '&');
  std::replace(pre_query.begin(), pre_query.end(), '=', '&');

  auto query = SplitRequestText(pre_query, "&");
  auto query_size = query.size();

  if (query_size % 2 != 0) {
    --query_size;
  }
  for (uint64_t i = 0; i < query_size; i += 2) {
    if (i + 1 >= query_size) {
      break;
    }
    query_.emplace(query[i], query[i + 1]);
  }

  path_ = std::string(path_view);
}

std::vector<std::string> SplitRequestText(const std::string& to_split,
                                          const std::string& separator) {
  uint64_t const separator_size = separator.size();
  std::vector<std::string> result;
  std::string_view view = to_split;
  size_t find = view.find(separator);
  while (find != std::string::npos) {
    result.emplace_back(view.substr(0, find));
    view.remove_prefix(find + separator_size);
    find = view.find(separator);
  }
  if (!view.empty()) {
    result.emplace_back(view);
  }
  return result;
}