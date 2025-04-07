{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = inputs@{
    self,
    nixpkgs,
    flake-utils,
    ...
  }:
  flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs {
        inherit system;
        config = { };
      };

      inherit (pkgs) lib;

      qt = pkgs.qt6;

      commonArgs = {
        nativeBuildInputs = [
          pkgs.cmake
          pkgs.ninja
          pkgs.pkg-config
          qt.wrapQtAppsHook
        ];
        buildInputs = with pkgs; [
          qt.qtbase
          pkgs.libgit2
          pkgs.fmt_11
        ];
      };

      # Read package version from CMakeLists
      version = lib.pipe ./CMakeLists.txt [
        builtins.readFile
        (builtins.match ".*\nset\\(version[[:space:]]+\"([\\.[:digit:]]+)\"\\)\n.*")
        builtins.head
      ];

      git-monitor = pkgs.stdenv.mkDerivation (commonArgs // {
        pname = "git-monitor";
        inherit version;
        src = self;
        meta = {
          description = "Monitor git repositories and let you know when you forget to push, pull, or commit";
          license = lib.licenses.gpl3;
          mainProgram = "git-monitor";
          inherit (qt.qtbase.meta) platforms;
        };
      });

    in {
      packages = {
        default = git-monitor;
        inherit git-monitor;
      };

      devShells.default = pkgs.mkShell (commonArgs // {
        packages = [
          qt.full
          qt.qtlanguageserver
          pkgs.qtcreator
          pkgs.gdb
        ];
      });
    }
  );
}
