set -x -e

# Go to the project root (here)
cd "$(cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P)"

# Build all the examples
cmake -B build .
cmake --build build -j 8

# Run all examples
./build/ire-colored     resources/meshes/nefertiti.obj --frames 300 --auto
./build/ire-normals     resources/meshes/nefertiti.obj --frames 300 --auto
./build/ire-palette     resources/meshes/nefertiti.obj --frames 300 --auto
./build/ire-meshlet     resources/meshes/nefertiti.obj --frames 300 --auto
./build/ire-raytracing  resources/meshes/nefertiti.obj --frames 300 --auto
./build/ire-pathtracing resources/meshes/nefertiti.obj --frames 300 --auto
./build/ire-ngf         resources/meshes/armadillo-ngf.bin --frames 300 --auto
./build/ire-particles   --frames 300 --auto
./build/ire-compute     --frames 300 --auto
./build/ire-font        --frames 300 --auto

# Generate PDFs for all generated .dot files
find .javelin -name '*.dot' | while read -r dotfile; do
    pdffile="${dotfile%.dot}.pdf"
    dot -Tpdf "$dotfile" -o "$pdffile"
done
