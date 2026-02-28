{
  runCommandLocal,
  glslang,
  shaderc,
  vulkan-headers,
  vulkan-loader,
  vulkan-tools,
  vulkan-tools-lunarg,
  vulkan-validation-layers,
  ...
}: let
  packages = [
    glslang
    shaderc.dev
    shaderc.bin
    shaderc.lib
    shaderc.static
    vulkan-headers
    vulkan-loader
    vulkan-tools
    vulkan-tools-lunarg
    vulkan-validation-layers
  ];
in
  runCommandLocal "vulkansdk" {} ''
    mkdir -p $out/lib
    mkdir -p $out/bin
    mkdir -p $out/include
    mkdir -p $out/share

    # TODO: This seems like a common pattern, I'm sure there's a Nix Way to
    # recursively symlink files but I didn't bother figuring it out
    for pkg in ${builtins.concatStringsSep " " (map (pkg: "\"${pkg}\"") packages)}; do
      for inner in $pkg/lib/*; do
        ln -sf $inner $out/lib/
      done
      for inner in $pkg/bin/*; do
        ln -sf $inner $out/bin/
      done
      for inner in $pkg/share/*; do
        ln -sf $inner $out/share/
      done
      for inner in $pkg/include/*; do
        ln -sf $inner $out/include/
      done
    done
  ''
