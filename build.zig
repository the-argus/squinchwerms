const std = @import("std");
const builtin = @import("builtin");
const zcc = @import("compile_commands");
const app_name = "squinchwerms";

const universal_flags = &[_][]const u8{
    "-DCP_USE_DOUBLES=0",
    "-std=c11",
    "-Isrc/",
    "-Werror",
    "-Wall",
};

const c_sources = &[_][]const u8{
    "src/main.c",
    "src/level.c",
    "src/physics.c",
    "src/allocators/global.c",
    "src/structures/pointer_collection.c",
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

    const raylib = b.dependency("raylib", .{ .target = target, .optimize = optimize }).artifact("raylib");
    const chipmunk = b.dependency("chipmunk2d", .{ .target = target, .optimize = optimize }).artifact("chipmunk");

    // HACK: chipmunk include dirs dont appear in compile_commands.json unless I do this :/
    {
        for (chipmunk.installed_headers.items) |include_dir_step| {
            const path = b.pathJoin(&.{ include_dir_step.owner.install_prefix, "include" });
            defer b.allocator.free(path);
            try flags.append(b.fmt("-I{s}", .{path}));
        }
    }

    const final_flags = try flags.toOwnedSlice();

    const exe = b.addExecutable(.{
        .target = target,
        .optimize = optimize,
        .name = app_name,
    });
    try lsp_targets.append(exe);

    exe.addCSourceFiles(c_sources, final_flags);

    // link libraries
    exe.linkLibC();
    exe.linkLibrary(raylib);
    exe.linkLibrary(chipmunk);
    b.installArtifact(exe);

    const runstep = b.step("run", "Run the game");
    const runsubstep = b.addRunArtifact(exe);
    runstep.dependOn(&runsubstep.step);

    zcc.createStep(b, "cdb", try lsp_targets.toOwnedSlice());
}
