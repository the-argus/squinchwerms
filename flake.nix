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

      vulkansdk = pkgs.callPackage ./vulkansdk.nix {};
    in {
      devShell =
        # gcc 15 segfaults when trying to print the diagnostic for this one error I had
        (pkgs.mkShell.override {stdenv = pkgs.gcc14Stdenv;})
        {
          packages = with pkgs; [
            (writeShellScriptBin "configure" "cmake --preset dev")
            (writeShellScriptBin "build" "cmake --build build-dev --parallel")
            (writeShellScriptBin "run" "build && gdb build-dev/squinchwerms")
            (writeShellScriptBin "frun" "build && ./build-dev/squinchwerms")
            (writeShellScriptBin "rd" "build && renderdoccmd capture -d . -c ./capture ./build-dev/squinchwerms")
            gdb
            valgrind
            mold
            cmake
            ninja
            pkg-config
            tracy
            libGL
            cmake
            renderdoc
            vulkansdk

            sdl3
            box2d
            glaze
            openssl # for glaze
            (imgui.override {
              IMGUI_BUILD_GLFW_BINDING = false;
              IMGUI_BUILD_SDL3_BINDING = true;
              IMGUI_BUILD_SDL3_RENDERER_BINDING = true;
              # IMGUI_BUILD_VULKAN_BINDING = true;
            })

            # libx11
            # libxrandr
            # libxinerama
            # libxcursor
            # libxi
            # libxext
            # libxft
          ];

          VULKAN_SDK = "${vulkansdk}";
          # VK_LAYER_PATH = "${vulkansdk}/share/vulkan/explicit_layer.d";
        };

      formatter = pkgs.alejandra;
    });
}
