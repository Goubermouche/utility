## My personal, opinionated, utility library
> **This is still under development, and I change the interface/implementation of core classes pretty often**
This is 

This is my personal, opinionated library, which I use as a base for most of my toy projects. It's meant to be a replacement for the C++ STL, but it still relies on it (although I'm working on removing all dependencies from it and become STD-free).

## Bits
- [**Allocators**](./utility/allocators)
  - Pool allocator
- [**Algorithms**](./utility/algorithms)
  - Stable sort
- [**Containers**](./utility/containers)
  - Dynamic array
  - Dynamic string
  - Map
- [**Math**](./utility/math)
  - Vector
- [**System**](./utility/system)
  - Console/file interfaces
- [**Custom type names**]
  - This library attempts to provide alternatives to the C/C++ type names, which can be seen [here](./types.h). The custom type names can be used in the global namespace by using the `using namespace utility::types` directive. 
