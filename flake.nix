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
      system.aarch64-darwin
      system.x86_64-linux
    ];
  in
    flake-utils.lib.eachSystem supportedSystems (system: let
      pkgs = import nixpkgs {inherit system;};

      vulkansdk = pkgs.callPackage ./vendor/vulkansdk.nix {};
    in {
      devShell =
        # gcc 15 segfaults when trying to print the diagnostic for this one error I had
        (pkgs.mkShell.override {stdenv = pkgs.llvmPackages_21.stdenv;})
        {
          packages =
            (with pkgs; [
              clang-tools # to get wrapped variant of clang-scan-deps

              (writeShellScriptBin "configure" "cmake --preset dev")
              (writeShellScriptBin "build" "cmake --build build-dev --parallel")
              (writeShellScriptBin "run" "build && gdb build-dev/squinchwerms")
              (writeShellScriptBin "frun" "build && ./build-dev/squinchwerms")
              (writeShellScriptBin "rd" "build && renderdoccmd capture -d . -c ./capture ./build-dev/squinchwerms")
              cmake
              ninja
              pkg-config
              # tracy
              # renderdoc
              vulkansdk
            ])
            ++ pkgs.lib.optionals (system != flake-utils.lib.system.aarch64-darwin) (with pkgs; [
              mold
              gdb
              valgrind
              wayland
              wayland-protocols
              wayland-scanner

              libx11
              libxrandr
              libxinerama
              libxcursor
              libxi
              libxext
              libxft
              libxcb # vulkan + SDL uses vulkan-xcb
            ]);

          VULKAN_SDK = "${vulkansdk}";
          # VK_LAYER_PATH = "${vulkansdk}/share/vulkan/explicit_layer.d";
        };

      formatter = pkgs.alejandra;
    });
}
