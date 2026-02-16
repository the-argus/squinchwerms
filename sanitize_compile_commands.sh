#!/bin/sh

# this is for my local use because clangd doesnt understand these flags and
# seems to not respect a .clangd file which removes them

sed -i 's/-fdeps-format=.* //g' compile_commands.json
sed -i 's/-fmodule-mapper=.* //g' compile_commands.json
sed -i 's/-fmodules-ts //g' compile_commands.json
