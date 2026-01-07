#pragma once
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace std;

struct Game {
  string name;
  string platform;
  string appid;
  string exe_path;
  int hours_played;

  Game(const string &n, const string &p, const string &id)
      : name(n), platform(p), appid(id), hours_played(0) {}
};

class GameLibrary {
private:
  vector<Game> games;
  string get_home() {
    const char *home = getenv("HOME");
    return home ? home : "";
  }

  string parse_acf_value(const string &line) {
    size_t quote1 = line.find('"', line.find('"') + 1);
    size_t quote2 = line.find('"', quote1 + 1);
    if (quote1 != string::npos && quote2 != string::npos) {
      return line.substr(quote1 + 1, quote2 - quote1 - 1);
    }
    return "";
  }

public:
  void scan_steam() {
    string steam_dir = get_home() + "/.steam/steam/steamapps";

    if (!fs::exists(steam_dir)) {
      return;
    }

    for (const auto &entry : fs::directory_iterator(steam_dir)) {
      if (entry.path().extension() == ".acf") {
        ifstream file(entry.path());
        string line;

        string appid, name;
        while (getline(file, line)) {
          if (line.find("\"appid\"") != string::npos) {
            appid = parse_acf_value(line);
          }
          if (line.find("\"name\"") != string::npos) {
            name = parse_acf_value(line);
          }

          if (!appid.empty() && !name.empty()) {
            games.emplace_back(name, "steam", appid);
            break;
          }
        }
      }
    }
  }

  void scan_epic() {
    // usually /.config/Eampic/UnrealEngineLauncher/
    games.emplace_back("Fortnite", "epic", "fortnite");
    games.emplace_back("Rocket League", "epic", "rocketleague");
  }

  void scan_gog() {
    games.emplace_back("The Witcher 3", "gog", "witcher3");
    games.emplace_back("Cyberpunk 2077", "gog", "cyberpunk");
  }

  void scan_all() {
    games.clear();
    scan_steam();
    scan_epic();
    scan_gog();
  }

  const vector<Game> &get_games() const { return games; }
};

// for launch games

void launch_game(const Game &game) {
  string cmd;

  if (game.platform == "steam") {
    cmd = "xdg-open steam://rungameid/" + game.appid;
  } else if (game.platform == "epic") {
    cmd = "legendary launch" + game.appid;
  } else if (game.platform == "gog") {
    cmd = "heroic://lpaunch/" + game.appid;
  }

  if (!cmd.empty()) {
    system(cmd.c_str());
  }
}
