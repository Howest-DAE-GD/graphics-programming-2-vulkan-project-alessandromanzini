#!/bin/zsh

# Ensure the output directory exists
mkdir -p build

# Compiler path
GLSLC="$VULKAN_SDK/bin/glslc"

# Catch arguments
# -w        : Suppress warnings
# -Werror   : Treat warnings as errors
# -g        : Generate debug information
# -c        : Only compile, do not link
# -E        : Only run the preprocessor
# -S        : Emit SPIR-V assembly instead of binary.
# -O        : Optimize the generated code for better performance
# -Os       : Optimize the generated code for smaller binary
# -O0       : Disable optimization
# --version : Print the compiler version
args=("$@")

# Declare an associative array (ONLY in zsh or bash 4+)
typeset -A shaders
shaders=(
    "shader.vert" "vert.spv"
    "shader.frag" "frag.spv"
)

# Loop over key-value pairs
for src dst in ${(kv)shaders}; do
    cli=($src $args "-o" "build/$dst")
    echo  $cli
    GLSLC $cli
done