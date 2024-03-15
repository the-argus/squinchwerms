const std = @import("std");
const builtin = @import("builtin");
const zcc = @import("compile_commands");
const app_name = "squinchwerms";

const flags = &[_][]const u8{
    "-DFMT_EXCEPTIONS=0",
    "-fno-exceptions",
    "-fno-rtti",
};

const cpp_sources = &[_][]const u8{
    "src/main.cpp",
};


pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    _ = target;
    const optimize = b.standardOptimizeOption(.{});
    _ = optimize;
    var ziglike: ?*std.Build.Dependency = null;
    _ = ziglike;
}
