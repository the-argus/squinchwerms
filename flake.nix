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
        pkgs.mkShell
        {
          packages = with pkgs; [
            (writeShellScriptBin "build" "zig build -Dcpu=baseline")
            (writeShellScriptBin "run" "build && gdb zig-out/bin/squinchwerms")
            (writeShellScriptBin "frun" "build && zig build run -Dcpu=baseline")
            (writeShellScriptBin "rd" "build && renderdoccmd capture -d . -c ./capture ./zig-out/bin/squinchwerms")
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
