echo "Building for Linux"
g++ -o shell main.cpp -lreadline -ldl
echo "Done Building"
echo "Starting Terminal"
./shell