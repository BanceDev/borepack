# Borepack

*put some description here*

## Features
- [ ] FBX Model Support
- [ ] BSP Map Rendering
*add more features here*

## Releases
- Binary builds are available from [releases](https://github.com/BanceDev/borepack/releases)

## Compiling
Clone the repo
```
git clone https://github.com/BanceDev/borepack.git
cd borepack
```

On Windows generate a Visual Studio project and then open the .sln file.
```
./premake5 vs2022
```

On Linux generate a GNU makefile and then compile
```
./premake5 gmake
make
```

### Linux Dependencies

```
sudo apt install build-essential pkg-config libgl1-mesa-dev libglfw3-dev
```

### Windows Dependencies

All Windows dependencies should be installed using precompiled binaries found in the ```/deps``` directory

## Contributing
- For bug reports and feature suggestions please use [issues](https://github.com/BanceDev/borepack/issues).
- If you wish to contribute code of your own please submit a [pull request](https://github.com/BanceDev/borepack/pulls).

## License

This software is licensed under the MIT License. For more details, see the [LICENSE](./LICENSE) file.
