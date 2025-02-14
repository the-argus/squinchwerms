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
};

const cpp_sources = &[_][]const u8{
    "src/main.cpp",
    "src/allo_impl.cpp",
    "src/level.cpp",
    "src/physics.cpp",
    "src/space.cpp",
    "src/shape.cpp",
    "src/body.cpp",
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
    const raylib = b.dependency("raylib", .{ .target = target, .optimize = optimize, .linux_display_backend = .X11 }).artifact("raylib");
    const chipmunk = b.dependency("chipmunk2d", .{ .target = target, .optimize = optimize }).artifact("chipmunk");

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
    exe.addIncludePath(okaylib.builder.path("include/"));
    exe.linkLibCpp();
    exe.linkLibrary(raylib);
    exe.linkLibrary(chipmunk);

    b.installArtifact(exe);

    const runstep = b.step("run", "Run the game");
    const runsubstep = b.addRunArtifact(exe);
    runstep.dependOn(&runsubstep.step);

    zcc.createStep(b, "cdb", try lsp_targets.toOwnedSlice());
}
