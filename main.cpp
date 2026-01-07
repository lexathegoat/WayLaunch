#include "game.h"

int main(int argc, char *argv[]) {
  auto app = Gtk::Application::create("dev.launcher.games");
  return app->make_window_and_run<LauncherWindow>(argc, argv);
}
