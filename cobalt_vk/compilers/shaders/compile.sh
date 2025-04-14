#!/bin/zsh

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
# -o        : Specify the output path
# -i        : Specify targets for the compiler
# --version : Print the compiler version
args=("$@")

# Flag infos
compile_args=()
inputs=()
output=""

# Parse arguments
for ((i = 1; i < ${#args[@]}+1; i++)); do
    arg="${args[$i]}"

	# Handle current argument
    case $arg in
        --version)
			# Print the compiler version
			"$GLSLC" --version
			exit 0
			;;

        -o)
            ((i++))
			# Handle -o flag, expect next argument to be the output path
			if [ $i -le ${#args[@]} ] && [[ ! "${args[$i]}" =~ ^- ]]; then
				output=("${args[$i]}")
			else
				echo "Expected an argument for -o flag!"
				exit 1
			fi
			;;

        -i)
            ((i++))
			# Handle -i flag, expect multiple arguments for input paths
			while [ $i -le ${#args[@]} ] && [[ ! "${args[$i]}" =~ ^- ]]; do
				inputs+=("${args[$i]}")
				((i++))
			done
			((i--))

			if [ ${#inputs} -eq 0 ]; then
				echo "Expected at least one argument for -i flag!"
				exit 1
			fi
			;;

        *)
            # Handle other flags
			if [[ "${args[$i]}" =~ ^- ]]; then
				compile_args+=("${args[$i]}")
			else
				# If the argument is not a flag (starts with -), we don't recognize it
				echo "Unknown argument: ${args[$i]}"
				exit 1
			fi
			;;
    esac
done

# Create the output directory if it doesn't exist
mkdir -p ${output}

# Define input-output associative array (ONLY in zsh or bash 4+)
typeset -A shaders
for i in {1..${#inputs[@]}}; do
	# Get the output path and remove extra slashes ( | tr -s / )
	output_path="$output/${inputs[$i]:t}.spv"
	shaders[${inputs[$i]}]=$output_path
done

# Loop over key-value pairs and run the compiler
for src dst in ${(kv)shaders}; do
    # Prepare the command
    cli=("$src" "${compile_args[@]}" "-o" "$dst")

    # Print the final command to be executed
    echo "$GLSLC ${cli[@]}"

    # Run GLSLC with the arguments
    "$GLSLC" "${cli[@]}"
done
