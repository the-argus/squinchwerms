{
  description = "Squinchwerms";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    nixpkgs,
    flake-utils,
    ...
  }: let
    supportedSystems = let
      inherit (flake-utils.lib) system;
    in [
      system.aarch64-linux
      system.x86_64-linux
    ];
  in
    flake-utils.lib.eachSystem supportedSystems (system: let
      pkgs = import nixpkgs {inherit system;};
    in {
      devShell =
        pkgs.mkShell
        {
          packages =
            (with pkgs; [
              (writeShellScriptBin "build" "zig build -Dcpu=baseline")
              (writeShellScriptBin "run" "build && gdb zig-out/bin/squinchwerms")
              (writeShellScriptBin "frun" "build && zig build run -Dcpu=baseline")
              gdb
              valgrind
              pkg-config
              libGL
              cmake
              zig_0_13
            ])
            ++ (with pkgs.xorg; [
              libX11
              libXrandr
              libXinerama
              libXcursor
              libXi
            ]);
        };

      formatter = pkgs.alejandra;
    });
}
