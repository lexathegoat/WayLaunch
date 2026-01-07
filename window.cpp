#include "game.h"
#include <algorithm>
#include <gtkmm.h>

class GameCard : public Gtk::Box {
public:
  GameCard(const Game &g) : Gtk::Box(Gtk::Orientation::VERTICAL), game(g) {
    set_margin(8);
    set_size_request(180, 240);
    add_css_class("card");

    // oyun resmi yerine gradient
    auto img = Gtk::make_managed<Gtk::Box>();
    img->set_size_request(180, 120);
    img->add_css_class("game-img");
    append(*img);

    // oyun ismi
    auto name_lbl = Gtk::make_managed<Gtk::Label>(game.name);
    name_lbl->set_wrap(true);
    name_lbl->set_max_width_chars(18);
    name_lbl->set_margin_top(8);
    name_lbl->set_margin_start(8);
    name_lbl->set_margin_end(8);
    append(*name_lbl);

    // platform
    auto platform_lbl = Gtk::make_managed<Gtk::Label>(game.platform);
    platform_lbl->add_css_class("platform");
    platform_lbl->set_margin_start(8);
    append(*platform_lbl);

    // play button
    auto btn = Gtk::make_managed<Gtk::Button>("Play");
    btn->set_margin(8);
    btn->signal_clicked().connect([this]() { launch_game(this->game); });
    append(*btn);
  }

private:
  Game game;
};

class LauncherWindow : public Gtk::Window {
public:
  LauncherWindow() {
    set_title("Game Launcher");
    set_default_size(1000, 700);

    setup_style();
    build_ui();

    library.scan_all();
    refresh_games();
  }

private:
  GameLibrary library;
  std::vector<Game> filtered;

  Gtk::Box vbox{Gtk::Orientation::VERTICAL};
  Gtk::Box toolbar{Gtk::Orientation::HORIZONTAL};
  Gtk::SearchEntry search;
  Gtk::DropDown platform_dd;
  Gtk::ScrolledWindow scroll;
  Gtk::FlowBox flow;

  std::string current_platform = "all";

  void setup_style() {
    auto css = Gtk::CssProvider::create();
    css->load_from_data(R"(
            window { background: #1a1b26; }
            
            .toolbar {
                background: #24283b;
                padding: 12px;
                border-bottom: 1px solid #414868;
            }
            
            searchentry {
                background: #1a1b26;
                color: #c0caf5;
                border-radius: 8px;
                min-width: 300px;
            }
            
            .card {
                background: #24283b;
                border-radius: 12px;
            }
            
            .card:hover {
                background: #292e42;
            }
            
            .game-img {
                background: linear-gradient(135deg, #7aa2f7 0%, #bb9af7 100%);
                border-radius: 12px 12px 0 0;
            }
            
            .platform {
                color: #7aa2f7;
                font-size: 11px;
                margin-bottom: 4px;
            }
            
            label {
                color: #c0caf5;
            }
            
            button {
                background: #7aa2f7;
                color: #1a1b26;
                border-radius: 6px;
                border: none;
                font-weight: 600;
            }
            
            button:hover {
                background: #9ca7db;
            }
        )");

    Gtk::StyleContext::add_provider_for_display(
        Gdk::Display::get_default(), css,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  }

  void build_ui() {
    set_child(vbox);

    // toolbar
    toolbar.add_css_class("toolbar");
    toolbar.set_spacing(12);

    search.set_placeholder_text("Search games...");
    search.signal_search_changed().connect([this]() { filter_games(); });
    toolbar.append(search);

    auto platforms = Gtk::StringList::create({"All", "Steam", "Epic", "GOG"});
    platform_dd.set_model(platforms);
    platform_dd.property_selected().signal_changed().connect([this]() {
      int idx = platform_dd.get_selected();
      if (idx == 0)
        current_platform = "all";
      else if (idx == 1)
        current_platform = "steam";
      else if (idx == 2)
        current_platform = "epic";
      else if (idx == 3)
        current_platform = "gog";
      filter_games();
    });
    toolbar.append(platform_dd);

    vbox.append(toolbar);

    // game grid
    flow.set_max_children_per_line(5);
    flow.set_selection_mode(Gtk::SelectionMode::NONE);
    scroll.set_child(flow);
    scroll.set_vexpand(true);
    vbox.append(scroll);
  }

  void filter_games() {
    filtered.clear();
    auto &all = library.get_games();

    std::string query = search.get_text();
    std::transform(query.begin(), query.end(), query.begin(), ::tolower);

    for (const auto &g : all) {
      if (current_platform != "all" && g.platform != current_platform)
        continue;

      if (!query.empty()) {
        std::string name = g.name;
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        if (name.find(query) == std::string::npos)
          continue;
      }

      filtered.push_back(g);
    }

    refresh_games();
  }

  void refresh_games() {
    while (auto child = flow.get_first_child()) {
      flow.remove(*child);
    }

    for (const auto &g : filtered) {
      auto card = Gtk::make_managed<GameCard>(g);
      flow.append(*card);
    }
  }
};
