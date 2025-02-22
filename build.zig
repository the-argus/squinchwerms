const std = @import("std");
const builtin = @import("builtin");
const zcc = @import("compile_commands");
const app_name = "squinchwerms";

const universal_flags = &[_][]const u8{
    "-DFMT_EXCEPTIONS=0",
    "-fno-exceptions",
    "-fno-rtti",
    "-DCP_USE_DOUBLES=0",
    "-std=c++20",
    "-Isrc/",
    "-DFMT_HEADER_ONLY",
    "-Werror",
    "-Wno-deprecated",
};

const cpp_sources = &[_][]const u8{
    "src/body.cpp",
    "src/main.cpp",
    "src/shape.cpp",
    "src/space.cpp",
    // "src/terrain.cpp",
    "src/vect.cpp",
    "src/natural_log/natural_log.cpp",
};

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // targets that should appear in compile_commands.json
    var lsp_targets = std.ArrayList(*std.Build.Step.Compile).init(b.allocator);
    defer lsp_targets.deinit();

    var flags = std.ArrayList([]const u8).init(b.allocator);
    defer flags.deinit();
    try flags.appendSlice(universal_flags);

    // import libraries
    const okaylib = b.dependency("okaylib", .{ .target = target, .optimize = optimize });
    const raylib = b.dependency("raylib", .{ .target = target, .optimize = optimize, .linux_display_backend = .X11 });
    const chipmunk = b.dependency("chipmunk2d", .{ .target = target, .optimize = optimize });

    // TODO: okaylib should propagate this to us but its not working, manually including it
    const fmt = b.dependency("fmt", .{});
    const fmt_include_path = b.pathJoin(&.{ fmt.builder.install_path, "include" });
    try flags.append(b.fmt("-I{s}", .{fmt_include_path}));

    const final_flags = try flags.toOwnedSlice();

    const exe = b.addExecutable(.{
        .target = target,
        .optimize = optimize,
        .name = app_name,
    });
    try lsp_targets.append(exe);

    exe.addCSourceFiles(.{
        .files = cpp_sources,
        .flags = final_flags,
    });

    // link libraries
    exe.step.dependOn(okaylib.builder.getInstallStep());
    exe.step.dependOn(fmt.builder.getInstallStep());
    exe.addIncludePath(okaylib.builder.path("include/"));
    exe.linkLibCpp();
    exe.linkLibrary(raylib.artifact("raylib"));
    exe.linkLibrary(chipmunk.artifact("chipmunk"));
    // raylib's include paths are wrong, they include the headers directly with -I/path/to/include/header.h instead of -I/path/to/include
    exe.addIncludePath(raylib.builder.path("src/"));

    b.installArtifact(exe);

    const runstep = b.step("run", "Run the game");
    const runsubstep = b.addRunArtifact(exe);
    runstep.dependOn(&runsubstep.step);

    zcc.createStep(b, "cdb", try lsp_targets.toOwnedSlice());
}
