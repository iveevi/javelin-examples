set -x -e

cd "$(cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P)"

cmake -B build .
cmake --build build -j 8

# Run all examples
# TODO: options to run with validation...
# TODO: produce fixed video and check with that each time... (MSE threshold)
./build/colored/colored resources/meshes/nefertiti.obj --frames 300 --auto
./build/normals/normals resources/meshes/nefertiti.obj --frames 300 --auto
./build/palette/palette resources/meshes/nefertiti.obj --frames 300 --auto
./build/meshlet/meshlet resources/meshes/nefertiti.obj --frames 300 --auto
./build/raytracing/raytracing resources/meshes/nefertiti.obj --frames 300 --auto
./build/pathtracing/pathtracing resources/meshes/nefertiti.obj --frames 300 --auto
./build/ngf/ngf resources/meshes/armadillo-ngf.bin --frames 300 --auto
./build/particles/particles --frames 300 --auto
./build/compute/compute --frames 300 --auto
./build/font/font --frames 300 --auto

# Generate PDF outputs for all .dot files in the output directory
find .javelin -name '*.dot' | while read -r dotfile; do
    pdffile="${dotfile%.dot}.pdf"
    dot -Tpdf "$dotfile" -o "$pdffile"
done
